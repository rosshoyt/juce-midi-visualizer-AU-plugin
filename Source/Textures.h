/*
 ==============================================================================
 
 Ross Hoyt
 Textures.h
 ==============================================================================
 */

#pragma once
#include "GLUtils.h"

struct DemoTexture
{
    virtual ~DemoTexture() {}
    virtual bool applyTo (OpenGLTexture&) = 0;
    
    String name;
};

struct BuiltInTexture   : public DemoTexture
{
    BuiltInTexture (const char* nm, const void* imageData, size_t imageSize)
    : image (resizeImageToPowerOfTwo (ImageFileFormat::loadFrom (imageData, imageSize)))
    {
        name = nm;
    }
    
    Image image;
    
    bool applyTo (OpenGLTexture& texture) override
    {
        texture.loadImage (image);
        return false;
    }
};
