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
#ifndef PICTURE_DATA_H
#define PICTURE_DATA_H

#ifdef __3DS__
#   define PICTURE_DATA_3DS
#   include "3ds/picture_data_3ds.h"
#elif defined(__WII__)
#   define PICTURE_DATA_WII
#   include "wii/picture_data_wii.h"
#elif defined(__16M__)
#   include "16m/picture_data_16m.h"
#elif defined(PGE_MIN_PORT) || defined(THEXTECH_CLI_BUILD)
#   define PICTURE_DATA_NULL
#   include "null/picture_data_null.h"
#else
#   define PICTURE_DATA_NORMAL
#   include "sdl/picture_data_sdl.h"
#endif

#endif // PICTURE_DATA_H
