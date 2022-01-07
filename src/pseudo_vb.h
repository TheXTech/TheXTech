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
#ifndef PSEUDO_VB_H
#define PSEUDO_VB_H

#include <SDL2/SDL_scancode.h>

#define False false
#define True true
#define If if(
#define Then ) {
#define End }

#define CDbl(x) static_cast<double>(x)
#define CSng(x) static_cast<float>(x)
#define CBool(x) static_cast<bool>(x)
#define CInt(x) static_cast<int>(x)

const SDL_Scancode vbKeyEscape = SDL_SCANCODE_ESCAPE;
const SDL_Scancode vbKeySpace = SDL_SCANCODE_SPACE;
const SDL_Scancode vbKeyReturn = SDL_SCANCODE_RETURN;
const SDL_Scancode vbKeyDown = SDL_SCANCODE_DOWN;
const SDL_Scancode vbKeyLeft = SDL_SCANCODE_LEFT;
const SDL_Scancode vbKeyUp = SDL_SCANCODE_UP;
const SDL_Scancode vbKeyRight = SDL_SCANCODE_RIGHT;
const SDL_Scancode vbKeyShift = SDL_SCANCODE_LSHIFT;
const SDL_Scancode vbKeyZ = SDL_SCANCODE_Z;
const SDL_Scancode vbKeyX = SDL_SCANCODE_X;
const SDL_Scancode vbKeyA = SDL_SCANCODE_A;
const SDL_Scancode vbKeyS = SDL_SCANCODE_S;
const SDL_Scancode vbKeyPageUp = SDL_SCANCODE_PAGEUP;
const SDL_Scancode vbKeyPageDown = SDL_SCANCODE_PAGEDOWN;

#endif // PSEUDO_VB_H
