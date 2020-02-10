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
void NPCHit(int A, int B, int C);
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
void Bomb(Location location, int Game, int ImmunePlayer);
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
