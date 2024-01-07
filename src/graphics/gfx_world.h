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
#ifndef GFX_WORLD_H
#define GFX_WORLD_H

struct vScreen_t;
struct Player_t;

//! Checks whether the world map frame assets exist and are valid
bool worldHasFrameAssets();

//! Get the vscreen for a world player, with world section bounds checking
void GetvScreenWorld(vScreen_t& vscreen);

//! Draws world map frame around vscreen in its screen
void DrawWorldMapFrame(const vScreen_t& vscreen);

//! Draws a player for the world map HUD, at coordinates X (left) and Y (bottom)
void DrawPlayerWorld(Player_t& p, int X, int Y);

#endif // GFX_WORLD_H
