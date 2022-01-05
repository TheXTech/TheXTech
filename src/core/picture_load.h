/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2021 Vitaly Novichkov <admin@wohlnet.ru>
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
#ifndef STD_PICTURE_LOAD_H
#define STD_PICTURE_LOAD_H

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

    void clear()
    {
        raw.clear();
        rawMask.clear();
        lazyLoaded = false;
        isMaskPng = false;
    }
};

#endif // STD_PICTURE_LOAD_H
