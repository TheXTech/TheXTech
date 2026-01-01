/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2026 Vitaly Novichkov <admin@wohlnet.ru> and ds-sloth
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*

Includes material from qoi.h

Copyright (c) 2021, Dominic Szablewski - https://phoboslab.org
SPDX-License-Identifier: MIT

*/

#include <stdlib.h>
#include <string.h>

#include <Utils/files.h>

#include "graphics_funcs.h"
#include <Logger/logger.h>

#include <FreeImageLite.h>


#ifndef QOI_MALLOC
    #define QOI_MALLOC(sz) malloc(sz)
    #define QOI_FREE(p)    free(p)
#endif
#ifndef QOI_ZEROARR
    #define QOI_ZEROARR(a) memset((a),0,sizeof(a))
#endif

#define QOI_SRGB   0
#define QOI_LINEAR 1
#define BUF_SIZE 4096

typedef struct {
    unsigned int width;
    unsigned int height;
    unsigned char channels;
    unsigned char colorspace;
} qoi_desc;


#define QOI_OP_INDEX  0x00 /* 00xxxxxx */
#define QOI_OP_DIFF   0x40 /* 01xxxxxx */
#define QOI_OP_LUMA   0x80 /* 10xxxxxx */
#define QOI_OP_RUN    0xc0 /* 11xxxxxx */
#define QOI_OP_RGB    0xfe /* 11111110 */
#define QOI_OP_RGBA   0xff /* 11111111 */

#define QOI_MASK_2    0xc0 /* 11000000 */

#define QOI_COLOR_HASH(C) (C.rgba.r*3 + C.rgba.g*5 + C.rgba.b*7 + C.rgba.a*11)
#define QOI_COLOR_HASH_RGBQUAD(C) (C.rgbRed*3 + C.rgbGreen*5 + C.rgbBlue*7 + C.rgbReserved*11)
#define QOI_MAGIC \
    (((unsigned int)'q') << 24 | ((unsigned int)'o') << 16 | \
     ((unsigned int)'i') <<  8 | ((unsigned int)'f'))
#define QOI_HEADER_SIZE 14

/* 2GB is the max file size that this implementation can safely handle. We guard
against anything larger than that, assuming the worst case with 5 bytes per
pixel, rounded down to a nice clean value. 400 million pixels ought to be
enough for anybody. */
#define QOI_PIXELS_MAX ((unsigned int)400000000)

static const unsigned char qoi_padding[8] = {0,0,0,0,0,0,0,1};

static unsigned int qoi_read_32(const unsigned char*& read_ptr) {
    unsigned int a = *(read_ptr++);
    unsigned int b = *(read_ptr++);
    unsigned int c = *(read_ptr++);
    unsigned int d = *(read_ptr++);
    return a << 24 | b << 16 | c << 8 | d;
}

FIBITMAP *GraphicsHelps::loadQOI(const Files::Data &raw, bool &depthTestSupported)
{
    if(raw.size() < QOI_HEADER_SIZE + 8)
        return nullptr;

    FIBITMAP *dib = nullptr;
    const unsigned char *read_ptr = raw.begin();
    const unsigned char *read_bound = raw.end() - 8;

    // qoi_decode: local declarations
    qoi_desc _desc;
    qoi_desc* const desc = &_desc;

    // qoi_decode: header read
    unsigned int header_magic = qoi_read_32(read_ptr);
    desc->width = qoi_read_32(read_ptr);
    desc->height = qoi_read_32(read_ptr);
    desc->channels = *(read_ptr++);
    desc->colorspace = *(read_ptr++);

    if(desc->width == 0 || desc->height == 0 ||
       desc->channels < 3 || desc->channels > 4 ||
       desc->colorspace > 1 ||
       header_magic != QOI_MAGIC ||
       desc->height >= QOI_PIXELS_MAX / desc->width)
    {
        return nullptr;
    }

    // allocate bitmap
    dib = FreeImage_Allocate(desc->width, desc->height, 32);

    if(!dib)
        return nullptr;

    FreeImage_SetTransparent(dib, TRUE);

    // store whether the depth test is supported
    depthTestSupported = (desc->channels == 3);

    // start actually decoding
    int run = 0;

    RGBQUAD index[64];
    RGBQUAD px;

    QOI_ZEROARR(index);
    px.rgbRed = 0;
    px.rgbGreen = 0;
    px.rgbBlue = 0;
    px.rgbReserved = 255;

    RGBQUAD* pixels = reinterpret_cast<RGBQUAD*>(FreeImage_GetBits(dib));
    RGBQUAD* pixels_end = reinterpret_cast<RGBQUAD*>(FreeImage_GetBits(dib) + FreeImage_GetPitch(dib) * desc->height);
    RGBQUAD* pixels_risk = pixels_end - 0x40; // optimization: moment after which we need to check that runs won't overrun the pixel buffer

    while(pixels < pixels_end)
    {
        // if we're going to overrun bytes array, that's a bug!
        if(read_ptr >= read_bound)
            goto error;

        unsigned char b1 = *(read_ptr++);
        unsigned char fam = (b1 & QOI_MASK_2);

        if(fam == QOI_OP_RUN)
        {
            // QOI_OP_RUN
            if(b1 < QOI_OP_RGB)
            {
                run = (b1 & 0x3f) + 1; // add one because we will subtract one for this pixel below

                if(pixels > pixels_risk && pixels + run > pixels_end)
                    goto error;

                for(; run > 0; run--)
                    *(pixels++) = px;

                continue;
            }
            else if(b1 == QOI_OP_RGB)
            {
                px.rgbRed   = *(read_ptr++);
                px.rgbGreen = *(read_ptr++);
                px.rgbBlue  = *(read_ptr++);
            }
            // QOI_OP_RGBA
            else
            {
                px.rgbRed      = *(read_ptr++);
                px.rgbGreen    = *(read_ptr++);
                px.rgbBlue     = *(read_ptr++);
                px.rgbReserved = *(read_ptr++);
            }
        }
        else if(fam == QOI_OP_INDEX)
        {
            px = index[b1];
        }
        else if(fam == QOI_OP_DIFF)
        {
            px.rgbRed   += ((b1 >> 4) & 0x03) - 2;
            px.rgbGreen += ((b1 >> 2) & 0x03) - 2;
            px.rgbBlue  += ( b1       & 0x03) - 2;
        }
        else if(fam == QOI_OP_LUMA)
        {
            unsigned char b2 = *(read_ptr++);
            char vg = (char)(b1 & 0x3f) - 32;
            px.rgbRed   += vg - 8 + (char)((b2 >> 4) & 0x0f);
            px.rgbGreen += vg;
            px.rgbBlue  += vg - 8 +  (char)(b2       & 0x0f);
        }

        index[QOI_COLOR_HASH_RGBQUAD(px) & (64 - 1)] = px;

        *(pixels++) = px;
    }

    // if there aren't exactly 8 bytes (padding) left, or the padding doesn't match, that's a bug!
    if(read_ptr != read_bound || memcmp(read_ptr, qoi_padding, 8) != 0)
        goto error;

    return dib;

error:
    if(dib)
        FreeImage_Unload(dib);

    return nullptr;
}
