/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2022 Vitaly Novichkov <admin@wohlnet.ru>
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

#ifndef PICTURE_DATA_WII_H
#define PICTURE_DATA_WII_H

#include <cstdint>
#include <gccore.h>

#define X_IMG_EXT ".tpl"

/*!
 * \brief Platform specific picture data. Fields should not be used directly
 */
struct StdPictureData
{

    bool texture_file_init[3] = {false, false, false};
    bool texture_init[3] = {false, false, false};
    TPLFile texture_file[3];
    GXTexObj texture[3];
    uint16_t tex_w[3];
    uint16_t tex_h[3];

    inline bool hasTexture()
    {
        return texture_init[0];
    }

    inline void destroy()
    {
        for(int i = 0; i < 3; i++)
        {
            if(texture_file_init[i])
                TPL_CloseTPLFile(&texture_file[i]);

            texture_file_init[i] = false;
            texture_init[i] = false;
        }
    }

};

#endif // PICTURE_DATA_NULL_H
