#ifndef BITMAP_H
#define BITMAP_H

#include "sprite.h"
#include "bitmapdata.h"
#include "graphicsbase.h"

class GBitmap : public GSprite
{
public:
    GBitmap(Application *application, GTextureRegion* bitmapdata) : GSprite(application)
	{
		//printf("Bitmap()\n");
		bitmapdata_ = bitmapdata;
		bitmapdata_->ref();

		texturebase_ = NULL;

		anchorx_ = 0;
		anchory_ = 0;
		dx_ = 0;
		dy_ = 0;

		setCoords();
        updateBounds();
	}

    GBitmap(Application *application, TextureBase* texturebase) : GSprite(application)
	{
		texturebase_ = texturebase;
		texturebase_->ref();

		bitmapdata_ = NULL;

		anchorx_ = 0;
		anchory_ = 0;
		dx_ = 0;
		dy_ = 0;

		setCoords();
        updateBounds();
    }

    virtual ~GBitmap()
	{
		//printf("~Bitmap()\n");
		if (bitmapdata_ != NULL)
			bitmapdata_->unref();

		if (texturebase_ != NULL)
			texturebase_->unref();
	}

	void setAnchorPoint(float x, float y);
	void getAnchorPoint(float* x, float* y) const;

    void setTextureRegion(GTextureRegion *bitmapdata);
    void setTexture(TextureBase *texturebase);

private:
    virtual void doDraw(const CurrentTransform&, float sx, float sy, float ex, float ey);

private:
	virtual void extraBounds(float* minx, float* miny, float* maxx, float* maxy) const;

	GTextureRegion* bitmapdata_;
	TextureBase* texturebase_;

	void setCoords();
	float anchorx_, anchory_;
	float dx_, dy_;

    void updateBounds();
    float minx_, miny_, maxx_, maxy_;

	GraphicsBase graphicsBase_;
};

#endif
