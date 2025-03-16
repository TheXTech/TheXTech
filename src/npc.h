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
#ifndef NPC_H
#define NPC_H

#include "location.h"
#include "global_constants.h"

enum NPCID : vbint_t;
struct NPC_t;
struct Player_t;

// Returns true if the game has been paused
bool UpdateNPCs();
// Public Sub DropBonus(A As Integer) 'Drops a bonus item that was held by the player
// Drops a bonus item that was held by the player
void DropBonus(int A);

// EXTRA: After adding one new star, hide all over-star BGOs
void CheckAfterStarTake(bool many = false);

// EXTRA: all the logic for collecting a medal NPC
void CollectMedal(const NPC_t& medal);
// Public Sub TouchBonus(A As Integer, B As Integer) 'Code for dealing with bonus and player
// Code for dealing with bonus and player
void TouchBonus(int A, int B);
// Public Sub NPCHit(A As Integer, B As Integer, Optional C As Integer = 0) 'For NPCs that were hit
// For NPCs that were hit
void NPCHit(int A, int B, int C = 0);
// Public Sub KillNPC(A As Integer, B As Integer) 'Handles NPC deaths and death effects
// Handles NPC deaths and death effects
// Returns true if this should be resumed later
bool KillNPC(int A, int B);
// Public Sub CheckSectionNPC(A As Integer) 'find out what section the NPC is in
// find out what section the NPC is in
void CheckSectionNPC(int A);
// Public Sub Deactivate(A As Integer) 'deactive and reset the NPC when it goes offscreen
// deactive and reset the NPC when it goes offscreen
void Deactivate(int A);
// Public Sub Bomb(Location As Location, Game As Integer, Optional ImmunePlayer As Integer = 0) 'for bomb explosions
// for bomb explosions
void Bomb(Location_t Location, int Game, int ImmunePlayer = 0);
// Public Sub DropNPC(A As Integer, NPCType As Integer)  'Drops an NPC from the screen
// Drops an NPC from the screen (dead code, removed)
// void DropNPC(int A, int NPCType);
// Public Sub TurnNPCsIntoCoins() 'turns some NPCs into coins when the player reaches the level exit
// turns some NPCs into coins when the player reaches the level exit
void TurnNPCsIntoCoins();
// Public Sub NPCFrames(A As Integer) 'updates the NPCs graphics
// updates the NPCs graphics
void NPCFrames(int A);
// Public Sub SkullRide(A As Integer)
void SkullRide(int A, bool reEnable = false, const Location_t *alignAt = nullptr);
// Public Sub NPCSpecial(A As Integer)
// called before movement is applied
void NPCSpecial(int A);
// Public Sub SpecialNPC(A As Integer)
// called after movement is applied
void SpecialNPC(int A);
// Public Sub CharStuff(Optional WhatNPC As Integer = 0, Optional CheckEggs As Boolean = False)
void CharStuff(int WhatNPC = 0, bool CheckEggs = false);
// Public Function RandomBonus()
NPCID RandomBonus();

// totally new function, used in the Raft logic
bool npcHasFloor(const struct NPC_t &npc);

// totally new function, used for compatibility (in compat mode, horizontal distance; in modern mode, squared Euclidean distance)
double NPCPlayerTargetDist(const NPC_t& npc, const Player_t& player);

// totally new function covering old logic. returns nearest player (using NPCPlayerTargetDist) that is not Dead and is in NPC's section.
int NPCTargetPlayer(const NPC_t& npc);

// totally new function covering old logic. checks nearest player (using NPCTargetPlayer) then faces it.
int NPCFaceNearestPlayer(NPC_t& npc, bool old_version = false);

#endif // NPC_H
