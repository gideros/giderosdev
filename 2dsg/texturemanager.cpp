#include "texturemanager.h"
//#include <gtexture.h>
#include <gimage.h>
#include <gpath.h>
#include <vector>
#include <string.h>
#include <sys/stat.h>
#include <dib.h>
#include <glog.h>
#include "glcommon.h"
#include <application.h>

GLuint TextureData::id()
{
    return gid;
}

static void append(std::vector<char> &buffer, const void *data, size_t len)
{
    size_t s = buffer.size();
    buffer.resize(s + len);
    memcpy(&buffer[s], data, len);
}

static void append(std::vector<char> &buffer, const TextureParameters &parameters)
{
    append(buffer, &parameters.filter, sizeof(parameters.filter));
    append(buffer, &parameters.wrap, sizeof(parameters.wrap));
    append(buffer, &parameters.maketransparent, sizeof(parameters.maketransparent));
    append(buffer, &parameters.transparentcolor, sizeof(parameters.transparentcolor));
    append(buffer, &parameters.grayscale, sizeof(parameters.grayscale));
}

TextureManager::TextureManager(Application* application) :
    application_(application)
{
}

TextureManager::~TextureManager()
{
    /* TODO: delete textures */
}

TextureData *TextureManager::createTextureFromFile(const char *filename, const TextureParameters &parameters)
{
    int flags = gpath_getDriveFlags(gpath_getPathDrive(filename));

    std::vector<char> sig;
    if (flags & GPATH_RO)
    {
        append(sig, filename, strlen(filename) + 1);
        append(sig, parameters);
    }
    else
    {
        if (flags & GPATH_REAL)
        {
            struct stat s;
            stat(gpath_transform(filename), &s);

            append(sig, filename, strlen(filename) + 1);
            append(sig, parameters);
            append(sig, &s.st_mtime, sizeof(s.st_mtime));
        }
    }

    GLenum wrap = 0; // to avoid -Wmaybe-uninitialized warning
    switch (parameters.wrap)
    {
    case eClamp:
        wrap = GL_CLAMP_TO_EDGE;
        break;
    case eRepeat:
        wrap = GL_REPEAT;
        break;
    }

    GLenum filter = 0; // to avoid -Wmaybe-uninitialized warning
    switch (parameters.filter)
    {
    case eNearest:
        filter = GL_NEAREST;
        break;
    case eLinear:
        filter = GL_LINEAR;
        break;
    }

    GLenum format = 0; // to avoid -Wmaybe-uninitialized warning
    GLenum type = 0; // to avoid -Wmaybe-uninitialized warning
    switch (parameters.format)
    {
    case eRGBA8888:
        format = GL_RGBA;
        type = GL_UNSIGNED_BYTE;
        break;
    case eRGB888:
        format = GL_RGB;
        type = GL_UNSIGNED_BYTE;
        break;
    case eRGB565:
        format = GL_RGB;
        type = GL_UNSIGNED_SHORT_5_6_5;
        break;
    case eRGBA4444:
        format = GL_RGBA;
        type = GL_UNSIGNED_SHORT_4_4_4_4;
        break;
    case eRGBA5551:
        format = GL_RGBA;
        type = GL_UNSIGNED_SHORT_5_5_5_1;
        break;
    }

    if (!sig.empty())
    {
        std::map<std::vector<char>, TextureData*>::iterator iter = textureCache_.find(sig);

        if (iter != textureCache_.end())
        {
            iter->second->refcount++;
            return iter->second;
        }
    }

    Dib dib(application_, filename, true, true, parameters.maketransparent, parameters.transparentcolor);

    if (parameters.grayscale)
        dib.convertGrayscale();

#ifndef PREMULTIPLIED_ALPHA
#error PREMULTIPLIED_ALPHA is not defined
#endif

#if PREMULTIPLIED_ALPHA
    dib.premultiplyAlpha();
#endif

    GLuint gid;
    glGenTextures(1, &gid);
    glBindTexture(GL_TEXTURE_2D, gid);

    switch (parameters.format)
    {
    case eRGBA8888:
        glTexImage2D(GL_TEXTURE_2D, 0, format, dib.width(), dib.height(), 0, format, type, dib.data());
        break;
    case eRGB888:
    {
        std::vector<unsigned char> data = dib.to888();
        glTexImage2D(GL_TEXTURE_2D, 0, format, dib.width(), dib.height(), 0, format, type, &data[0]);
        break;
    }
    case eRGB565:
    {
        std::vector<unsigned short> data = dib.to565();
        glTexImage2D(GL_TEXTURE_2D, 0, format, dib.width(), dib.height(), 0, format, type, &data[0]);
        break;
    }
    case eRGBA4444:
    {
        std::vector<unsigned short> data = dib.to4444();
        glTexImage2D(GL_TEXTURE_2D, 0, format, dib.width(), dib.height(), 0, format, type, &data[0]);
        break;
    }
    case eRGBA5551:
    {
        std::vector<unsigned short> data = dib.to5551();
        glTexImage2D(GL_TEXTURE_2D, 0, format, dib.width(), dib.height(), 0, format, type, &data[0]);
        break;
    }
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);

    TextureData *data = new TextureData;

    data->gid = gid;
    data->parameters = parameters;
    data->width = dib.originalWidth();
    data->height = dib.originalHeight();
    data->exwidth = dib.width();
    data->exheight = dib.height();
    data->baseWidth = dib.baseOriginalWidth();
    data->baseHeight = dib.baseOriginalHeight();

    data->refcount = 1;
    data->sig = sig;
    textureCache_[sig] = data;

    return data;
}

TextureData *TextureManager::createTextureFromDib(const Dib &dib, const TextureParameters &parameters)
{
#if 0
    int wrap = 0;
    switch (parameters.wrap)
    {
    case eClamp:
        wrap = GTEXTURE_CLAMP;
        break;
    case eRepeat:
        wrap = GTEXTURE_REPEAT;
        break;
    }

    int filter = 0;
    switch (parameters.filter)
    {
    case eNearest:
        filter = GTEXTURE_NEAREST;
        break;
    case eLinear:
        filter = GTEXTURE_LINEAR;
        break;
    }

    int format = 0;
    int type = 0;
    switch (parameters.format)
    {
    case eRGBA8888:
        format = GTEXTURE_RGBA;
        type = GTEXTURE_UNSIGNED_BYTE;
        break;
    case eRGB888:
        format = GTEXTURE_RGB;
        type = GTEXTURE_UNSIGNED_BYTE;
        break;
    case eRGB565:
        format = GTEXTURE_RGB;
        type = GTEXTURE_UNSIGNED_SHORT_5_6_5;
        break;
    case eRGBA4444:
        format = GTEXTURE_RGBA;
        type = GTEXTURE_UNSIGNED_SHORT_4_4_4_4;
        break;
    case eRGBA5551:
        format = GTEXTURE_RGBA;
        type = GTEXTURE_UNSIGNED_SHORT_5_5_5_1;
        break;
    }


    Dib dib2 = dib;

    if (parameters.grayscale)
        dib2.convertGrayscale();

#if PREMULTIPLIED_ALPHA
    dib2.premultiplyAlpha();
#endif


    g_id gid = 0;
    switch (parameters.format)
    {
    case eRGBA8888:
        gid = gtexture_create(dib.width(), dib.height(), format, type, wrap, filter, dib2.data(), NULL, 0);
        break;
    case eRGB888:
    {
        std::vector<unsigned char> data = dib2.to888();
        gid = gtexture_create(dib.width(), dib.height(), format, type, wrap, filter, &data[0], NULL, 0);
        break;
    }
    case eRGB565:
    {
        std::vector<unsigned short> data = dib2.to565();
        gid = gtexture_create(dib.width(), dib.height(), format, type, wrap, filter, &data[0], NULL, 0);
        break;
    }
    case eRGBA4444:
    {
        std::vector<unsigned short> data = dib2.to4444();
        gid = gtexture_create(dib.width(), dib.height(), format, type, wrap, filter, &data[0], NULL, 0);
        break;
    }
    case eRGBA5551:
    {
        std::vector<unsigned short> data = dib2.to5551();
        gid = gtexture_create(dib.width(), dib.height(), format, type, wrap, filter, &data[0], NULL, 0);
        break;
    }
    }

    TextureData* data = new TextureData;

    data->gid = gid;
    data->parameters = parameters;
    data->width = dib.originalWidth();
    data->height = dib.originalHeight();
    data->exwidth = dib.width();
    data->exheight = dib.height();
    data->baseWidth = dib.baseOriginalWidth();
    data->baseHeight = dib.baseOriginalHeight();

    return data;
#endif
    return NULL;
}

static unsigned int nextpow2(unsigned int v)
{
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;

    return v;
}

TextureData* TextureManager::createRenderTarget(int w, int h, const TextureParameters& parameters)
{
#if 0
    int wrap = 0;
    switch (parameters.wrap)
    {
    case eClamp:
        wrap = GTEXTURE_CLAMP;
        break;
    case eRepeat:
        wrap = GTEXTURE_REPEAT;
        break;
    }

    int filter = 0;
    switch (parameters.filter)
    {
    case eNearest:
        filter = GTEXTURE_NEAREST;
        break;
    case eLinear:
        filter = GTEXTURE_LINEAR;
        break;
    }

    float scalex = application_->getLogicalScaleX();
    float scaley = application_->getLogicalScaleY();

    int baseWidth = w;
    int baseHeight = h;
    int width = (int)floor(baseWidth * scalex + 0.5);
    int height = (int)floor(baseHeight * scaley + 0.5);
    int exwidth = nextpow2(width);
    int exheight = nextpow2(height);

    g_id gid = gtexture_RenderTargetCreate(exwidth, exheight, wrap, filter);

    TextureData *data = new TextureData;

    data->gid = gid;
    data->parameters = parameters;
    data->width = width;
    data->height = height;
    data->exwidth = exwidth;
    data->exheight = exheight;
    data->baseWidth = baseWidth;
    data->baseHeight = baseHeight;

    return data;
#endif
    return NULL;
}

void TextureManager::destroyTexture(TextureData *texture)
{
    if (--texture->refcount == 0)
    {
        textureCache_.erase(texture->sig);
        glDeleteTextures(1, &texture->gid);
        delete texture;
    }
}

