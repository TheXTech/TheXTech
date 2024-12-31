/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2025 Vitaly Novichkov <admin@wohlnet.ru>
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

#include "globals.h"

// Private Sub SpecialFrames() 'update frames for special things such as coins and kuribo's shoe
// update frames for special things such as coins and kuribo's shoe (pauses during FreezeNPCs)
extern void SpecialFrames();//PRIVATE

// updates CommonFrame and CommonFrame_Unpaused
inline void CommonFrames()
{
    CommonFrame++;

    if(LevelSelect || !FreezeNPCs)
        CommonFrame_NotFrozen++;
}

// NEW: update level frames that keep going during FreezeNPCs and pause
void LevelFramesAlways();

// NEW: update level frames that stop during FreezeNPCs (but continue during pause)
void LevelFramesNotFrozen();

// NEW: update world frames
void WorldFrames();

#endif // #ifndef GFX_SPECIAL_FRAMES_H
