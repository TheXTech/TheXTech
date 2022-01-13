/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2021 Vitaly Novichkov <admin@wohlnet.ru>
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
#ifndef LUNA_H
#define LUNA_H

#include <string>

struct SMBXHUDSettings
{
    bool              skip = false; // Skip the whole HUD drawing
    // WORLD_HUD_CONTROL overworldHudState;
    // bool              skipStarCount;
};

extern SMBXHUDSettings gSMBXHUDSettings;

extern bool gLunaEnabledGlobally;
extern bool gLunaEnabled;
extern bool gShowDemoCounter;
extern bool gEnableDemoCounter;
extern std::string gDemoCounterTitle;

extern void lunaReset();
extern void lunaLoad();
extern void lunaLoop();
extern void lunaRenderStart();
extern void lunaRenderHud();
extern void lunaRender(int screenZ);
extern void lunaRenderEnd();

#endif // LUNA_H
