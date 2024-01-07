/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2024 Vitaly Novichkov <admin@wohlnet.ru>
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
#ifndef GFX_KEYHOLE_H
#define GFX_KEYHOLE_H

#include "std_picture.h"

void RenderKeyhole(int Z);

void RenderTexturePlayer(int Z, double dst_x, double dst_y, double dst_w, double dst_h,
                         StdPicture& tex,
                         int src_x = 0, int src_y = 0,
                         XTColor color = XTColor());

void RenderTexturePlayerScale(int Z, double dst_x, double dst_y, double dst_w, double dst_h,
                         StdPicture& tex,
                         int src_x, int src_y, int src_w, int src_h,
                         XTColor color = XTColor());

#endif // GFX_KEYHOLE_H
