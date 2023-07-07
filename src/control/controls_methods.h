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

#ifndef CONTROLS_METHODS_H
#define CONTROLS_METHODS_H

#if   defined(__3DS__)

#include "control/input_3ds.h"

#elif defined(__WII__)

#include "control/input_wii.h"

#elif defined(__16M__)

#include "control/input_16m.h"

#elif !defined(THEXTECH_NO_SDL_BUILD) && !defined(THEXTECH_CLI_BUILD)

#include "control/keyboard.h"
#include "control/joystick.h"
#include "control/touchscreen.h"

#endif

#include "control/duplicate.h"

#endif // #ifndef CONTROLS_METHODS_H
