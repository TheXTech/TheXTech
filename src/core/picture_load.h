/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2023 Vitaly Novichkov <admin@wohlnet.ru>
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

#pragma once

#ifdef __WII__
#   define PICTURE_LOAD_WII
#   include "core/wii/picture_load.h"
#elif defined(__16M__)
#   define PICTURE_LOAD_16M
#   include "core/16m/picture_load_16m.h"
#elif defined(__3DS__) || defined(PGE_MIN_PORT) || defined(THEXTECH_CLI_BUILD)
#   define PICTURE_LOAD_3DS
#   include "core/3ds/picture_load.h"
#endif

// each of the above defines STD_PICTURE_LOAD_H so they prevent the rest from running

#ifndef STD_PICTURE_LOAD_H
#define STD_PICTURE_LOAD_H

#define PICTURE_LOAD_NORMAL

#include <cstdint>
#include <vector>

/*!
 * \brief Generic image loading store.
 *
 * If needed somehing unusual, please define alternative structure instead of this
 */
struct StdPictureLoad
{
    //! Is this a lazy-loaded texture?
    bool lazyLoaded = false;
    //! Original compressed data of the front image
    std::vector<char> raw;
    //! Original compressed data of the mask image (if presented)
    std::vector<char> rawMask;
    //! Was mask restored from the PNG at default graphics?
    bool isMaskPng = false;

    // Original size (if texture got scaled while loading)
    //! Original width
    int w_orig = 0;
    //! Original height
    int h_orig = 0;

    // Difference between original and initial size
    //! Width scale factor
    float w_scale = 1.0f;
    //! Height scale factor
    float h_scale = 1.0f;

    // Transparent color for BMP and JPEG
    bool     colorKey = false;
    uint8_t  keyRgb[3] = {0 /*R*/, 0 /*G*/, 0 /*B*/};

    /*!
     * \brief Clear all held data
     *
     * Must be called by renderer backend after texture deletion
     */
    void clear()
    {
        raw.clear();
        rawMask.clear();
        lazyLoaded = false;
        isMaskPng = false;
        w_orig = 0;
        h_orig = 0;
        w_scale = 1.f;
        h_scale = 1.f;
        colorKey = false;
        keyRgb[0] = 0;
        keyRgb[1] = 0;
        keyRgb[2] = 0;
    }
};

#endif // STD_PICTURE_LOAD_H
