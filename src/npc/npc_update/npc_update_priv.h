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

#ifndef NPC_UPDATE_PRIV_H
#define NPC_UPDATE_PRIV_H

#include "numeric_types.h"

struct NPC_t;

// most of these routines were originally part of UpdateNPCs
void NPCMovementLogic(int A, numf_t& speedVar);

void NPCSectionWrap(NPC_t& npc);

void NPCBlockLogic(int A, num_t& tempHit, int& tempHitBlock, numf_t& tempSpeedA, const int numTempBlock, const numf_t speedVar);

void NPCCollide(int A);

void NPCCollideHeld(int A);

void NPCWalkingLogic(int A, const num_t tempHit, const int tempHitBlock, numf_t tempSpeedA);

void NPCEffects(int A);

void NPCSpecialMaybeHeld(int A);

#endif // NPC_UPDATE_PRIV_H
