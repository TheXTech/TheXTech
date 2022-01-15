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
#ifndef LunaPlayer_HHH
#define LunaPlayer_HHH

#include "lunadefs.h"

struct Player_t;

namespace PlayerF
{

Player_t *Get(int num);

// PLAYER MANAGEMENT

void MemSet(int offset, double value, OPTYPE operation, FIELDTYPE ftype);

// PLAYER BUTTONS
bool PressingDown(Player_t* player);
bool PressingUp(Player_t* player);
bool PressingLeft(Player_t* player);
bool PressingRight(Player_t* player);
bool PressingJump(Player_t* player);
bool PressingRun(Player_t* player);
bool PressingSEL(Player_t* pPlayer);

// CYCLE PLAYER
void CycleRight(Player_t* player);	// Changes player identity to the next character, or around to Demo
void CycleLeft(Player_t* player);	// Changes player identity to the previous character, or around to Sheath

// PLAYER STATES
bool IsHoldingSpriteType(Player_t* player, int NPC_ID);

// FILTER FUNCS
void FilterToFire(Player_t *player);
void FilterToBig(Player_t *player);
void FilterToSmall(Player_t *player);
void FilterReservePowerup(Player_t *player);
void FilterMount(Player_t *player);

void InfiniteFlying(int player);

bool UsesHearts(Player_t *p);

// GET SCREEN POSITION
LunaRect GetScreenPosition(Player_t *player);

} // PlayerF

#endif // LunaPlayer_HHH
