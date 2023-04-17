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

#ifndef STD_PICTURE_LOAD_3DS_H
#define STD_PICTURE_LOAD_3DS_H

#include <string>

/*!
 * \brief Generic image loading store.
 *
 * If needed somehing unusual, please define alternative structure instead of this
 */
struct StdPictureLoad
{
    //! Is this a lazy-loaded texture?
    bool lazyLoaded = false;

    //! Path to find image
    std::string path = "";

    //! Path to find mask (if any)
    std::string mask_path = "";

    // Transparent color for BMP and JPEG
    bool     colorKey = false;
    uint8_t  keyRgb[3] = {0 /*R*/, 0 /*G*/, 0 /*B*/};

    inline bool canLoad() const
    {
        return lazyLoaded;
    }
};

#endif // #ifndef STD_PICTURE_LOAD_3DS_H
