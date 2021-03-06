#include "ttfont.h"
#include <gfile.h>
#include <gstdio.h>
#include "giderosexception.h"
#include "gstatus.h"
#include "application.h"
#include "ftlibrarysingleton.h"

#include <ft2build.h>
#include FT_OUTLINE_H

#include <utf8.h>

static unsigned long read(	FT_Stream stream,
							unsigned long offset,
							unsigned char* buffer,
							unsigned long count)
{
	G_FILE* fis = (G_FILE*)stream->descriptor.pointer;
	g_fseek(fis, offset, SEEK_SET);
	if (count == 0)
		return 0;
	return g_fread(buffer, 1, count, fis);
}

static void close(FT_Stream stream)
{
	G_FILE* fis = (G_FILE*)stream->descriptor.pointer;
	g_fclose(fis);
}

TTFont::TTFont(Application *application, const char *filename, float size, bool smoothing, GStatus *status) : FontBase(application)
{
    try
    {
        constructor(filename, size, smoothing);
    }
    catch (GiderosException &e)
    {
        if (status)
            *status = e.status();
    }
}

void TTFont::constructor(const char *filename, float size, bool smoothing)
{
	face_ = NULL;

	G_FILE* fis = g_fopen(filename, "rb");
	if (fis == NULL)
	{
		throw GiderosException(GStatus(6000, filename));		// Error #6000: %s: No such file or directory.
		return;
	}

	memset(&stream_, 0, sizeof(stream_));

	g_fseek(fis, 0, SEEK_END);
	stream_.size = g_ftell(fis);
	g_fseek(fis, 0, SEEK_SET);
	stream_.descriptor.pointer = fis;
	stream_.read = read;
	stream_.close = close;

	FT_Open_Args args;
	memset(&args, 0, sizeof(args));
	args.flags = FT_OPEN_STREAM;
	args.stream = &stream_;

	if (FT_Open_Face(FT_Library_Singleton::instance(), &args, 0, &face_))
		throw GiderosException(GStatus(6012, filename));		// Error #6012: %s: Error while reading font file.

    float scalex = application_->getLogicalScaleX();
    float scaley = application_->getLogicalScaleY();

	const int RESOLUTION = 72;
	if (FT_Set_Char_Size(face_, 0L, (int)floor(size * 64 + 0.5f), (int)floor(RESOLUTION * scalex + 0.5f), (int)floor(RESOLUTION * scaley + 0.5f)))
	{
		FT_Done_Face(face_);
		face_ = NULL;
        throw GiderosException(GStatus(6017, filename));		// Error #6017: Invalid font size.
	}

	ascender_ = face_->size->metrics.ascender >> 6;
	height_ = face_->size->metrics.height >> 6;

    smoothing_ = smoothing;
}

TTFont::~TTFont()
{
	if (face_)
		FT_Done_Face(face_);
}

void TTFont::getBounds(const wchar32_t *text, float letterSpacing, int *pminx, int *pminy, int *pmaxx, int *pmaxy) const
{
    float scalex = application_->getLogicalScaleX();

    int minx = 0x7fffffff;
    int miny = 0x7fffffff;
    int maxx = -0x7fffffff;
    int maxy = -0x7fffffff;

    int size = 0;
    for (const wchar32_t *t = text; *t; ++t, ++size)
        ;

    int x = 0, y = 0;
    FT_UInt prev = 0;
    for (int i = 0; i < size; ++i)
	{
        FT_UInt glyphIndex = FT_Get_Char_Index(face_, text[i]);
        if (glyphIndex == 0)
            continue;

        if (FT_Load_Glyph(face_, glyphIndex, FT_LOAD_DEFAULT))
			continue;

        int top, left, width, height;
        if (face_->glyph->format == FT_GLYPH_FORMAT_OUTLINE)
        {
            FT_BBox bbox;
            FT_Outline_Get_CBox(&face_->glyph->outline, &bbox);

            bbox.xMin &= ~63;
            bbox.yMin &= ~63;
            bbox.xMax  = (bbox.xMax + 63) & ~63;
            bbox.yMax  = (bbox.yMax + 63) & ~63;

            width  = (bbox.xMax - bbox.xMin) >> 6;
            height = (bbox.yMax - bbox.yMin) >> 6;
            top = bbox.yMax >> 6;
            left = bbox.xMin >> 6;
        }
        else if (face_->glyph->format == FT_GLYPH_FORMAT_BITMAP)
        {
            width = face_->glyph->bitmap.width;
            height = face_->glyph->bitmap.rows;
            top = face_->glyph->bitmap_top;
            left = face_->glyph->bitmap_left;
        }
        else
            continue;

        x += kerning(prev, glyphIndex) >> 6;
        prev = glyphIndex;

        int xo = x + left;
        int yo = y - top;

        minx = std::min(minx, xo);
        miny = std::min(miny, yo);
        maxx = std::max(maxx, xo + width);
        maxy = std::max(maxy, yo + height);

        x += face_->glyph->advance.x >> 6;

        x += (int)(letterSpacing * scalex);
	}

	if (pminx)
		*pminx = minx;
	if (pminy)
		*pminy = miny;
	if (pmaxx)
		*pmaxx = maxx;
	if (pmaxy)
		*pmaxy = maxy;
}

Dib TTFont::renderFont(const wchar32_t *text, float letterSpacing, int *pminx, int *pminy, int *pmaxx, int *pmaxy) const
{
    float scalex = application_->getLogicalScaleX();

    int minx, miny, maxx, maxy;
	getBounds(text, letterSpacing, &minx, &miny, &maxx, &maxy);

    Dib dib(application_, (maxx - minx) + 2, (maxy - miny) + 2, true);
	unsigned char rgba[] = {255, 255, 255, 0};
	dib.fill(rgba);

    int size = 0;
    for (const wchar32_t *t = text; *t; ++t, ++size)
        ;

    int x = 1, y = 1;
    FT_UInt prev = 0;
    for (int i = 0; i < size; ++i)
	{
        FT_UInt glyphIndex = FT_Get_Char_Index(face_, text[i]);
        if (glyphIndex == 0)
            continue;

        if (FT_Load_Glyph(face_, glyphIndex, FT_LOAD_DEFAULT))
			continue;

        int top, left, width, height;
        if (face_->glyph->format == FT_GLYPH_FORMAT_OUTLINE)
        {
            FT_BBox bbox;
            FT_Outline_Get_CBox(&face_->glyph->outline, &bbox);

            bbox.xMin &= ~63;
            bbox.yMin &= ~63;
            bbox.xMax  = (bbox.xMax + 63) & ~63;
            bbox.yMax  = (bbox.yMax + 63) & ~63;

            width  = (bbox.xMax - bbox.xMin) >> 6;
            height = (bbox.yMax - bbox.yMin) >> 6;
            top = bbox.yMax >> 6;
            left = bbox.xMin >> 6;
        }
        else if (face_->glyph->format == FT_GLYPH_FORMAT_BITMAP)
        {
            width = face_->glyph->bitmap.width;
            height = face_->glyph->bitmap.rows;
            top = face_->glyph->bitmap_top;
            left = face_->glyph->bitmap_left;
        }
        else
            continue;

        if (FT_Render_Glyph(face_->glyph, FT_RENDER_MODE_NORMAL))
			continue;

        FT_Bitmap &bitmap = face_->glyph->bitmap;

        width = std::min(width, bitmap.width);
        height = std::min(height, bitmap.rows);

        x += kerning(prev, glyphIndex) >> 6;
        prev = glyphIndex;

        int xo = x + left;
        int yo = y - top;

		for (int y = 0; y < height; ++y)
			for (int x = 0; x < width; ++x)
			{
				int index = x + y * bitmap.pitch;
				int c = bitmap.buffer[index];

				unsigned char a = dib.getAlpha(xo + x - minx, yo + y - miny);
				dib.setAlpha(xo + x - minx, yo + y - miny, std::min(255, c + a));
			}

        x += face_->glyph->advance.x >> 6;

        x += (int)(letterSpacing * scalex);
    }

	if (pminx)
		*pminx = minx;
	if (pminy)
		*pminy = miny;
	if (pmaxx)
		*pmaxx = maxx;
	if (pmaxy)
		*pmaxy = maxy;

	return dib;
}

void TTFont::getBounds(const char *text, float letterSpacing, float *pminx, float *pminy, float *pmaxx, float *pmaxy) const
{
    std::vector<wchar32_t> wtext;
    size_t len = utf8_to_wchar(text, strlen(text), NULL, 0, 0);
    if (len != 0)
    {
        wtext.resize(len);
        utf8_to_wchar(text, strlen(text), &wtext[0], len, 0);
    }
    wtext.push_back(0);

    int minx, miny, maxx, maxy;
    getBounds(&wtext[0], letterSpacing, &minx, &miny, &maxx, &maxy);

    float scalex = application_->getLogicalScaleX();
    float scaley = application_->getLogicalScaleY();

    if (pminx)
        *pminx = minx / scalex;
    if (pminy)
        *pminy = miny / scaley;
    if (pmaxx)
        *pmaxx = maxx / scalex;
    if (pmaxy)
        *pmaxy = maxy / scaley;
}

float TTFont::getAdvanceX(const char *text, float letterSpacing, int size) const
{
    float scalex = application_->getLogicalScaleX();

    std::vector<wchar32_t> wtext;
    size_t len = utf8_to_wchar(text, strlen(text), NULL, 0, 0);
    if (len != 0)
    {
        wtext.resize(len);
        utf8_to_wchar(text, strlen(text), &wtext[0], len, 0);
    }

    if (size < 0 || size > wtext.size())
        size = wtext.size();

    wtext.push_back(0);

    int x = 0;
    FT_UInt prev = 0;
    for (int i = 0; i < size; ++i)
    {
        FT_UInt glyphIndex = FT_Get_Char_Index(face_, text[i]);
        if (glyphIndex == 0)
            continue;

        if (FT_Load_Glyph(face_, glyphIndex, FT_LOAD_DEFAULT))
            continue;

        x += kerning(prev, glyphIndex) >> 6;
        prev = glyphIndex;

        x += face_->glyph->advance.x >> 6;

        x += (int)(letterSpacing * scalex);
    }

    x += kerning(prev, FT_Get_Char_Index(face_, text[size])) >> 6;

    return x / scalex;
}

int TTFont::kerning(FT_UInt left, FT_UInt right) const
{
    if (FT_HAS_KERNING(face_))
    {
        FT_Vector delta;
        FT_Get_Kerning(face_, left, right, FT_KERNING_DEFAULT, &delta);
        return delta.x;
    }

    return 0;
}

float TTFont::getAscender() const
{
    float scaley = application_->getLogicalScaleY();
    return ascender_ / scaley;
}

float TTFont::getLineHeight() const
{
    float scaley = application_->getLogicalScaleY();
    return height_ / scaley;
}
