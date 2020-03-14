/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2020 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

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
