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
#ifndef CHANGE_RES_H
#define CHANGE_RES_H

//void GetCurrentRes(); // Useless
void SetOrigRes();
void ChangeRes(int ScreenX, int ScreenY, int ScreenColor, int ScreenFreq);

// New: update the internal game resolution and scaling based on game window size and preferences
// Calls XRender::updateViewport on completion
void UpdateInternalRes();

// New: update the window size based on internal resolution and scaling factor
// Only active for windowed mode with 0.5x, 1x, or 2x scaling
void UpdateWindowRes();

#endif // CHANGE_RES_H
