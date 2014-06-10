#pragma once

#include "texturebase.h"
#include "glcommon.h"

class Application;
class GSprite;

class GRenderTarget : public GTextureBase
{
public:
    GRenderTarget(Application *application, int width, int height, Filter filter);
    virtual ~GRenderTarget();

    void clear(unsigned int color, float a);

    void draw(const GSprite *sprite);

private:
    g_id tempTexture_;
    static int qualcommFix_;
};
