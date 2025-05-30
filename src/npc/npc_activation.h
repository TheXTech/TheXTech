/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2025 Vitaly Novichkov <admin@wohlnet.ru>
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
#ifndef NPC_ACTIVATION_H
#define NPC_ACTIVATION_H

#include "globals.h"

bool NPC_MustBeCanonical(NPCRef_t n);

// four possible render outcomes for an NPC: ignore (hide with no effect), render normally, hide with smoke effect, or shade (the default)
bool NPC_InactiveIgnore(const NPC_t& n);

bool NPC_InactiveRender(const NPC_t& n);

bool NPC_InactiveSmoke(const NPC_t& n);

void NPC_ConstructCanonicalSet();

#endif // #ifndef NPC_ACTIVATION_H
