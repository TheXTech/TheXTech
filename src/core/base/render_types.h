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
#ifndef ABTRACTRENDERTYPES_T_H
#define ABTRACTRENDERTYPES_T_H

enum RendererFlip_t
{
    X_FLIP_NONE       = 0x00000000,    /**< Do not flip */
    X_FLIP_HORIZONTAL = 0x00000001,    /**< flip horizontally */
    X_FLIP_VERTICAL   = 0x00000002     /**< flip vertically */
};

struct FPoint_t
{
    float x;
    float y;
};

#endif // ABTRACTRENDERTYPES_T_H
