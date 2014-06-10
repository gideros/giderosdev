#ifndef TEXTUREBASE_H
#define TEXTUREBASE_H

#include "texturemanager.h"
#include "refptr.h"

class GTextureBase : public GReferenced
{
public:
	float sizescalex;
	float sizescaley;
	float uvscalex;
	float uvscaley;

	TextureData* data;

protected:
    GTextureBase(Application* application);
    GTextureBase(Application* application,
                const char* filename, Filter filter, Wrap wrap, Format format,
				bool maketransparent = false, unsigned int transparentcolor = 0x00000000);

    virtual ~GTextureBase();

private:
	Application* application_;
};



#endif
