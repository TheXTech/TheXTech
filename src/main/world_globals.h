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
#ifndef WORLD_GLOBALS_H
#define WORLD_GLOBALS_H

#include "../screen_fader.h"

//! Holds the screen overlay for the world map
extern ScreenFader g_worldScreenFader;

//! Multiplier for world map qScreen
extern double g_worldCamSpeed;

//! Play sound if world map qScreen stays active next frame
extern bool g_worldPlayCamSound;

//! NEW: set the world player's section variable based on its position
extern void worldCheckSection(WorldPlayer_t& wp);

//! NEW: reset the world players' section variables without invoking qScreen
extern void worldResetSection();

extern void worldWaitForFade(int waitTicks = -1);

extern bool worldHasFrameAssets();

#endif // WORLD_GLOBALS_H
