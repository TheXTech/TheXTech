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

#ifndef ENUMNPCEFFECT_HHH
#define ENUMNPCEFFECT_HHH

#include "global_constants.h"

enum NPCEffect : vbint_t
{
    NPCEFF_NORMAL = 0,
    NPCEFF_EMERGE_UP = 1,
    NPCEFF_DROP_ITEM = 2,
    NPCEFF_EMERGE_DOWN = 3,
    NPCEFF_WARP = 4,
    NPCEFF_PET_TONGUE = 5,
    NPCEFF_PET_INSIDE = 6,
    NPCEFF_WAITING = 8,
    NPCEFF_ENCASED = 208,
};


#endif // ENUMNPCEFFECT_HHH
