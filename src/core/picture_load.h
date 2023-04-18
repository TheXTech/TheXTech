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

#ifdef __WII__
#   define PICTURE_LOAD_WII
#   include "core/wii/picture_load_wii.h"
#elif defined(__16M__)
#   define PICTURE_LOAD_16M
#   include "core/16m/picture_load_16m.h"
#elif defined(__3DS__)
#   define PICTURE_LOAD_3DS
#   include "core/3ds/picture_load_3ds.h"
#elif defined(PGE_MIN_PORT) || defined(THEXTECH_CLI_BUILD)
#   define PICTURE_LOAD_NULL
#   include "core/null/picture_load_null.h"
#else
#   define PICTURE_LOAD_NORMAL
#   include "core/base/picture_load_base.h"
#endif
