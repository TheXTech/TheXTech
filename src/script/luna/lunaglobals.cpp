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

#include "luna.h"

SMBXHUDSettings gSMBXHUDSettings;

bool gLunaEnabledGlobally = true;
bool gLunaEnabled = true;
bool gShowDemoCounter = true;
bool gEnableDemoCounter = true;
bool gEnableDemoCounterByLC = false;
std::string gDemoCounterTitle = "FAILS";
std::string gDemoCounterTitleDefault = "FAILS";


/* Fallback dummy calls for the case when Luna Autocode has been disabled */

#ifndef THEXTECH_ENABLE_LUNA_AUTOCODE

void lunaReset()
{}

void lunaLoad()
{}

void lunaLoop()
{}

void lunaRenderStart()
{}

void lunaRenderHud()
{}

void lunaRender(int)
{}

void lunaRenderEnd()
{}

#endif
