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

#ifndef PLAYER_UPDATE_PRIV_H
#define PLAYER_UPDATE_PRIV_H

#include "numeric_types.h"

struct Location_t;
struct Player_t;

void p_PlayerMakeFlySparkle(const Location_t& loc, int Frame = 1);

// most of these routines were originally part of UpdatePlayer
void PlayerNPCLogic(int A, bool& tempSpring, bool& tempShell, int& MessageNPC, const bool movingBlock, const int floorBlock, const tempf_t oldSpeedY);

void PlayerBlockLogic(int A, int& floorBlock, bool& movingBlock, bool& DontResetGrabTime, tempf_t cursed_value_C, const int oldStandingOnNpc);

bool p_PlayerTouchVine(Player_t& p, num_t vine_top, int VineNPC, int VineBGO);
void PlayerVineLogic(int A);
bool PlayerFairyOnVine(int A);
void PlayerVineMovement(int A);

void PlayerSharedScreenLogic(int A);
void PlayerLevelBoundsLogic(int A);
void PlayerEffectWings(int A);

void PlayerFairyTimerUpdate(int A);
void PlayerFairyMovementX(int A);
void PlayerFairyMovementY(int A);

void PlayerPinchedTimerUpdate(int A);
void PlayerPinchedDeathCheck(int A);

void PlayerMovementX(int A, tempf_t& cursed_value_C);
void PlayerSlideMovementX(int A);
void PlayerCockpitMovementX(int A);
void PlayerMovementY(int A);
void PlayerSwimMovementY(int A);
void PlayerAquaticSwimMovement(int A);

void PlayerPoundLogic(int A);
void PlayerShootChar5Beam(int A);
void PlayerThrowBomb(int A); // (from PowerUps)

void PlayerChar5Logic(int A);
void PlayerChar5StabLogic(int A); // (from PowerUps)

void UpdatePlayerTimeToLive(int A);
void UpdatePlayerDead(int A);

void PlayerVehicleDismountCheck(int A);

void PlayerEffectWarpPipe(int A);
void PlayerEffectWarpDoor(int A);
void PlayerEffectWarpWait(int A);

void PlayerMazeZoneMovement(int A);
void PlayerFlagSlideMovement(int A);
void PlayerThrowItemMaze(const Player_t& p, Location_t& loc, uint8_t& maze_status);
void PlayerThrownNpcMazeCheck(const Player_t& p, NPC_t& npc);

#endif // PLAYER_UPDATE_PRIV_H
