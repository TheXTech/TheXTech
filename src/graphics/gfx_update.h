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
#ifndef GFX_UPDATE_H
#define GFX_UPDATE_H

extern bool g_drawBlocks_valid[2];
extern bool g_drawBGOs_valid[2];

extern double g_drawBlocks_invalidate_rate;
extern double g_drawBGOs_invalidate_rate;

inline void invalidateDrawBlocks()
{
    g_drawBlocks_valid[0] = false;
    g_drawBlocks_valid[1] = false;
}

inline void invalidateDrawBGOs()
{
    g_drawBGOs_valid[0] = false;
    g_drawBGOs_valid[1] = false;
}

#endif // #ifdef GFX_UPDATE_H
