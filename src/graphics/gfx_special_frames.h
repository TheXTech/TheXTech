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
#ifndef GFX_SPECIAL_FRAMES_H
#define GFX_SPECIAL_FRAMES_H

// Private Sub SpecialFrames() 'update frames for special things such as coins and kuribo's shoe
extern void SpecialFrames();//PRIVATE
// update frames for special things such as coins and kuribo's shoe

// NEW: update level frames that keep going during
void LevelFramesAlways();

// NEW: update level frames that stop during FreezeNPCs
void LevelFramesNotFrozen();

#endif // #ifndef GFX_SPECIAL_FRAMES_H
