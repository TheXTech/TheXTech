/*
 * Bitmask to RGBA converter: allows to convert graphics with a front (image with
 * a black background) and mask (a dark shape of image on a white background)
 * into RGBA with a transparent background
 *
 * Copyright (c) 2020-2023 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies
 * or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
 * FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <FreeImageLite.h>

#include "bitmask2rgba.h"


void bitmask_to_rgba(FIBITMAP *front, FIBITMAP *mask)
{
    unsigned int x, y, ym, img_w, img_h, mask_w, mask_h, img_pitch, mask_pitch;

    BYTE *img_bits, *mask_bits, *FPixP, *SPixP;
    RGBQUAD Npix = {0x00, 0x00, 0x00, 0xFF};   /* Destination pixel color */
    BYTE Wpix[] = {0xFF, 0xFF, 0xFF, 0xFF};   /* Dummy white pixel */
    unsigned short newAlpha = 0xFF; /* Calculated destination alpha-value*/

    BOOL endOfY = FALSE;

    if(!mask)
        return; /* Nothing to do */

    img_w  = FreeImage_GetWidth(front);
    img_h  = FreeImage_GetHeight(front);
    img_pitch = FreeImage_GetPitch(front);
    mask_w = FreeImage_GetWidth(mask);
    mask_h = FreeImage_GetHeight(mask);
    mask_pitch = FreeImage_GetPitch(mask);

    img_bits  = FreeImage_GetBits(front);
    mask_bits = FreeImage_GetBits(mask);
    FPixP = img_bits;
    SPixP = mask_bits;

    ym = mask_h - 1;
    y = img_h - 1;

    while(1)
    {
        FPixP = img_bits + (img_pitch * y);
        if(!endOfY)
            SPixP = mask_bits + (mask_pitch * ym);

        for(x = 0; (x < img_w); x++)
        {
            Npix.rgbBlue = ((SPixP[FI_RGBA_BLUE] & 0x7F) | FPixP[FI_RGBA_BLUE]);
            Npix.rgbGreen = ((SPixP[FI_RGBA_GREEN] & 0x7F) | FPixP[FI_RGBA_GREEN]);
            Npix.rgbRed = ((SPixP[FI_RGBA_RED] & 0x7F) | FPixP[FI_RGBA_RED]);
            newAlpha = 255 - (((unsigned short)(SPixP[FI_RGBA_RED]) +
                               (unsigned short)(SPixP[FI_RGBA_GREEN]) +
                               (unsigned short)(SPixP[FI_RGBA_BLUE])) / 3);

            if((SPixP[FI_RGBA_RED] > 240u) //is almost White
               && (SPixP[FI_RGBA_GREEN] > 240u)
               && (SPixP[FI_RGBA_BLUE] > 240u))
                newAlpha = 0;

            newAlpha += (((unsigned short)(FPixP[FI_RGBA_RED]) +
                          (unsigned short)(FPixP[FI_RGBA_GREEN]) +
                          (unsigned short)(FPixP[FI_RGBA_BLUE])) / 3);

            if(newAlpha > 255)
                newAlpha = 255;

            FPixP[FI_RGBA_BLUE]  = Npix.rgbBlue;
            FPixP[FI_RGBA_GREEN] = Npix.rgbGreen;
            FPixP[FI_RGBA_RED]   = Npix.rgbRed;
            FPixP[FI_RGBA_ALPHA] = (BYTE)(newAlpha);
            FPixP += 4;

            if(x >= mask_w - 1 || endOfY)
                SPixP = Wpix;
            else
                SPixP += 4;
        }

        if(ym == 0)
        {
            endOfY = TRUE;
            SPixP = Wpix;
        }
        else
            ym--;

        if(y == 0)
            break;
        y--;
    }
}

void bitmask_get_mask_from_rgba(FIBITMAP *image, FIBITMAP **outmask)
{
    unsigned int img_w, img_h, x, y;
    RGBQUAD Fpix;
    RGBQUAD Npix = {0x0, 0x0, 0x0, 0xFF};
    BYTE gray;

    if(!image)
        *outmask = NULL;

    img_w = FreeImage_GetWidth(image);
    img_h = FreeImage_GetHeight(image);

    *outmask = FreeImage_AllocateT(FIT_BITMAP,
                                   (int)(img_w), (int)(img_h),
                                   (int)(FreeImage_GetBPP(image)),
                                   FreeImage_GetRedMask(image),
                                   FreeImage_GetGreenMask(image),
                                   FreeImage_GetBlueMask(image));

    for(y = 0; (y < img_h); y++)
    {
        for(x = 0; (x < img_w); x++)
        {
            FreeImage_GetPixelColor(image, x, y, &Fpix);

            gray = (255 - Fpix.rgbReserved);
            Npix.rgbRed  = gray;
            Npix.rgbGreen = gray;
            Npix.rgbBlue = gray;
            Npix.rgbReserved = 0xFF;
            FreeImage_SetPixelColor(*outmask,  x, y, &Npix);
        }
    }
}
