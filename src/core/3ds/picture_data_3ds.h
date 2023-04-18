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

#ifndef PICTURE_DATA_3DS_H
#define PICTURE_DATA_3DS_H

#include <cstdint>
#include <citro2d.h>

#define X_IMG_EXT ".t3x"

struct StdPicture;

/*!
 * \brief Platform specific picture data. Fields should not be used directly
 */
struct StdPictureData
{

    //! The previous texture in the render chain (nullptr if this is the tail or unloaded)
    StdPicture* last_texture = nullptr;

    //! The next texture in the render chain (nullptr if this is the head or unloaded)
    StdPicture* next_texture = nullptr;

    //! The last frame that the texture was rendered (not accessed if not in the render chain)
    uint32_t last_draw_frame = 0;

    //! Loaded texture data
    C2D_SpriteSheet texture[6] = {nullptr};
    C2D_Image image[6];

    inline bool hasTexture()
    {
        return texture[0] != nullptr;
    }

};

#endif // PICTURE_DATA_3DS_H
