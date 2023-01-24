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

#ifndef STD_PICTURE_LOAD_H
#define STD_PICTURE_LOAD_H

#include <string>

#include "core/16m/packloader.h"

struct StdPicture;

/*!
 * \brief Generic image loading store.
 *
 * If needed somehing unusual, please define alternative structure instead of this
 */
struct StdPictureLoad
{
    //! Is this a lazy-loaded texture?
    bool lazyLoaded = false;

    //! Pack index (PACK_NONE if not a pack)
    PackLoader::packref_t pack;
    uint32_t pack_offset = 0;

    //! The previous texture in the render chain (nullptr if this is the tail or unloaded)
    StdPicture* last_texture = nullptr;

    //! The next texture in the render chain (nullptr if this is the head or unloaded)
    StdPicture* next_texture = nullptr;

    //! The last frame that the texture was rendered (not accessed if not in the render chain)
    uint32_t last_draw_frame;

    //! Path to find image (could be a pack)
    std::string path = "";

    inline void clear()
    {
        pack = PackLoader::PACK_NONE;
        pack_offset = 0;
    }
};

#endif // #ifndef STD_PICTURE_LOAD_H
