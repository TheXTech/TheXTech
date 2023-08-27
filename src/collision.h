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
#ifndef COLLISION_H
#define COLLISION_H

#include "location.h"

struct NPC_t;

enum CollisionSpot
{
    COLLISION_NONE = 0,
    COLLISION_TOP = 1,
    COLLISION_RIGHT = 2,
    COLLISION_BOTTOM = 3,
    COLLISION_LEFT = 4,
    COLLISION_CENTER = 5
};

// Public Function CheckCollision(Loc1 As Location, Loc2 As Location) As Boolean 'Normal collisions
// Normal collisions
bool CheckCollision(const Location_t &Loc1, const Location_t &Loc2);
bool CheckCollision(const SpeedlessLocation_t &Loc1, const Location_t &Loc2);
bool CheckCollision(const Location_t &Loc1, const SpeedlessLocation_t &Loc2);
bool CheckCollision(const SpeedlessLocation_t &Loc1, const SpeedlessLocation_t &Loc2);
// Intersect collision
bool CheckCollisionIntersect(const Location_t &Loc1, const Location_t &Loc2);
// Public Function n00bCollision(Loc1 As Location, Loc2 As Location) As Boolean 'Make the game easier for the people who whine about the detection being 'off'
// Make the game easier for the people who whine about the detection being 'off'
bool n00bCollision(const Location_t &Loc1, const Location_t &Loc2);
// Public Function NPCStartCollision(Loc1 As Location, Loc2 As Location) As Boolean 'Used when a NPC is activated to see if it should spawn
// Used when a NPC is activated to see if it should spawn
bool NPCStartCollision(const Location_t &Loc1, const Location_t &Loc2);
// Public Function WarpCollision(Loc1 As Location, A As Integer) As Boolean  'Warp point collisions
// Warp point collisions
bool WarpCollision(const Location_t &Loc1, const SpeedlessLocation_t &entrance, int direction);
// Public Function FindCollision(Loc1 As Location, Loc2 As Location) As Integer 'Whats side the collision happened
// Whats side the collision happened
int FindCollision(const Location_t &Loc1, const Location_t &Loc2);
// Public Function FindCollisionBelt(Loc1 As Location, Loc2 As Location, BeltSpeed As Single) As Integer  'Whats side the collision happened for belts
// Whats side the collision happened for belts
int FindCollisionBelt(const Location_t &Loc1, const Location_t &Loc2, float BeltSpeed);
// Public Function NPCFindCollision(Loc1 As Location, Loc2 As Location) As Integer 'Whats side the collision happened for NPCs
// Whats side the collision happened for NPCs
int NPCFindCollision(const Location_t &Loc1, const Location_t &Loc2);
// Public Function EasyModeCollision(Loc1 As Location, Loc2 As Location, Optional StandOn As Boolean = False) As Integer  'Easy mode collision for jumping on NPCs
// Easy mode collision for jumping on NPCs
int EasyModeCollision(const Location_t &Loc1, const Location_t &Loc2, bool StandOn);
// Public Function BootCollision(Loc1 As Location, Loc2 As Location, Optional StandOn As Boolean = False) As Integer 'Easy mode collision for jumping on NPCs while on yoshi/boot
// Easy mode collision for jumping on NPCs while on yoshi/boot
int BootCollision(const Location_t &Loc1, const Location_t &Loc2, bool StandOn);
// Public Function CursorCollision(Loc1 As Location, Loc2 As Location) As Boolean 'Cursor collision
// Cursor collision
bool CursorCollision(const Location_t &Loc1, const Location_t &Loc2);
bool CursorCollision(const Location_t &Loc1, const SpeedlessLocation_t &Loc2);
// Public Function ShakeCollision(Loc1 As Location, Loc2 As Location, ShakeY3 As Integer) As Boolean 'Shakey block collision
// Shakey block collision
bool ShakeCollision(const Location_t &Loc1, const Location_t &Loc2, int ShakeY3);
// Public Function vScreenCollision(A As Integer, Loc2 As Location) As Boolean  'vScreen collisions
// vScreen collisions
bool vScreenCollision(int A, const Location_t &Loc2);
bool vScreenCollision(int A, const SpeedlessLocation_t &Loc2);
// Public Function vScreenCollision2(A As Integer, Loc2 As Location) As Boolean  'vScreen collisions 2
// vScreen collisions 2
bool vScreenCollision2(int A, const Location_t &Loc2);
bool vScreenCollision2(int A, const SpeedlessLocation_t &Loc2);
// Public Function WalkingCollision(Loc1 As Location, Loc2 As Location) As Boolean 'Collision detection for blocks. Prevents walking on walls.
// Collision detection for blocks. Prevents walking on walls.
bool WalkingCollision(const Location_t &Loc1, const Location_t &Loc2);
// Public Function WalkingCollision2(Loc1 As Location, Loc2 As Location) As Boolean 'Collision detection for blocks. Lets NPCs fall through cracks.
// Collision detection for blocks. Lets NPCs fall through cracks.
bool WalkingCollision2(const Location_t &Loc1, const Location_t &Loc2);
// Public Function WalkingCollision3(Loc1 As Location, Loc2 As Location, BeltSpeed As Single) As Boolean 'Factors in beltspeed
// Factors in beltspeed
bool WalkingCollision3(const Location_t &Loc1, const Location_t &Loc2, float BeltSpeed);
// Public Function FindRunningCollision(Loc1 As Location, Loc2 As Location) As Integer 'Helps the player to walk over 1 unit cracks
// Helps the player to walk over 1 unit cracks
int FindRunningCollision(const Location_t &Loc1, const Location_t &Loc2);
// Public Function ShouldTurnAround(Loc1 As Location, Loc2 As Location, Direction As Single) As Boolean  'Determines if an NPC should turnaround
// Determines if an NPC should turnaround
bool ShouldTurnAround(const Location_t &Loc1, const Location_t &Loc2, float Direction);
// Public Function CanComeOut(Loc1 As Location, Loc2 As Location) As Boolean  'Determines if an NPC can come out of a pipe
// Determines if an NPC can come out of a pipe
bool CanComeOut(const Location_t &Loc1, const Location_t &Loc2);
// Public Function CheckHitSpot1(Loc1 As Location, Loc2 As Location) As Boolean  'Fixes NPCs sinking through the ground
// Fixes NPCs sinking through the ground
bool CheckHitSpot1(const Location_t &Loc1, const Location_t &Loc2);

bool CompareWalkBlock(int oldBlockIdx, int newBlockIdx, const Location_t &referenceLoc);

void CompareNpcWalkBlock(int &tempHitBlock, int &tempHitOldBlock,
                         double &tempHit,   double &tempHitOld,
                         int &tempHitIsSlope, NPC_t *npc);

bool SectionCollision(const int section, const Location_t &loc);

#endif // COLLISION_H
