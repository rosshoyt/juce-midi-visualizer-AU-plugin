/*
 ==============================================================================
 
 Utility.h
 Created: 8 Dec 2019 11:50:48pm
 Author:  Ross Hoyt
 
 ==============================================================================
 */

#pragma once

static Image resizeImageToPowerOfTwo (Image image)
{
    if (! (isPowerOfTwo (image.getWidth()) && isPowerOfTwo (image.getHeight())))
        return image.rescaled (jmin (1024, nextPowerOfTwo (image.getWidth())),
                               jmin (1024, nextPowerOfTwo (image.getHeight())));
    
    return image;
}
