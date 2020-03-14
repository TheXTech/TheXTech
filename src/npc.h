/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2020 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef NPC_H
#define NPC_H

#include "location.h"

// Public Sub UpdateNPCs()
void UpdateNPCs();
// Public Sub DropBonus(A As Integer) 'Drops a bonus item that was held by the player
// Drops a bonus item that was held by the player
void DropBonus(int A);
// Public Sub TouchBonus(A As Integer, B As Integer) 'Code for dealing with bonus and player
// Code for dealing with bonus and player
void TouchBonus(int A, int B);
// Public Sub NPCHit(A As Integer, B As Integer, Optional C As Integer = 0) 'For NPCs that were hit
// For NPCs that were hit
void NPCHit(int A, int B, int C = 0);
// Public Sub KillNPC(A As Integer, B As Integer) 'Handles NPC deaths and death effects
// Handles NPC deaths and death effects
void KillNPC(int A, int B);
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
// Drops an NPC from the screen
void DropNPC(int A, int NPCType);
// Public Sub TurnNPCsIntoCoins() 'turns some NPCs into coins when the player reaches the level exit
// turns some NPCs into coins when the player reaches the level exit
void TurnNPCsIntoCoins();
// Public Sub NPCFrames(A As Integer) 'updates the NPCs graphics
// updates the NPCs graphics
void NPCFrames(int A);
// Public Sub SkullRide(A As Integer)
void SkullRide(int A);
// Public Sub NPCSpecial(A As Integer)
void NPCSpecial(int A);
// Public Sub SpecialNPC(A As Integer)
void SpecialNPC(int A);
// Public Sub CharStuff(Optional WhatNPC As Integer = 0, Optional CheckEggs As Boolean = False)
void CharStuff(int WhatNPC = 0, bool CheckEggs = false);
// Public Function RandomBonus()
int RandomBonus();

#endif // NPC_H
