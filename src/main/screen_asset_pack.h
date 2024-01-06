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

#ifndef SCREEN_ASSET_PACK_H
#define SCREEN_ASSET_PACK_H

#include "xt_color.h"

// The asset pack screen is special, because after exiting from it with a success, EVERYTHING gets reloaded. So, it gets its own main loop.

namespace ScreenAssetPack
{

extern bool g_LoopActive;

// this method is public so that it is possible to fade in the asset pack screen from the main menu
void DrawBackground(double fade);

void Loop();

} // namespace ScreenAssetPack

#endif // #ifndef SCREEN_ASSET_PACK_H
