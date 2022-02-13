/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2022 Vitaly Novichkov <admin@wohlnet.ru>
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
#ifndef PLAYER_H
#define PLAYER_H

// Public Sub SetupPlayers() 'this set's the players values to their defaults and prepares them for playing a level
// this set's the players values to their defaults and prepares them for playing a level
void SetupPlayers();
// Public Sub UpdatePlayer() 'This is the main sub for the players
// This is the main sub for the players
void UpdatePlayer();
// Public Sub PlayerHurt(A As Integer) 'Player got hurt
// Player got hurt
void PlayerHurt(const int A);
// Public Sub PlayerDead(A As Integer) 'Set player up to die
// Set player up to die
void PlayerDead(const int A);
// Public Sub KillPlayer(A As Integer) 'Kill the player
// Kill the player
void KillPlayer(const int A);
// Public Function CheckDead() As Integer 'Check if players are dead and return which one, returns 0 if everyones dead
// Check if players are dead and return which one, returns 0 if everyones dead
int CheckDead();
// Public Function CheckLiving() As Integer 'Check if players are alive, returns the first player or 0 if everyones dead
// Check if players are alive, returns the first player or 0 if everyones dead
int CheckLiving();
int LivingPlayersLeft();
// Public Function LivingPlayers() As Boolean 'true if there are still living players
// true if there are still living players
bool LivingPlayers();
// Public Sub EveryonesDead() 'Handles the game when all players have died
// Handles the game when all players have died
void EveryonesDead();
void ProcessLastDead();
// Public Sub UnDuck(A As Integer) 'Un Duck the player
// Un Duck the player
void UnDuck(struct Player_t &p);
// Public Sub CheckSection(A As Integer)
void CheckSection(const int A);
// Public Sub PlayerFrame(A As Integer)
void PlayerFrame(const int A);
void PlayerFrame(struct Player_t &p);
// Public Sub UpdatePlayerBonus(A As Integer, B As Integer)
void UpdatePlayerBonus(const int A, const int B);
// Public Sub TailSwipe(plr As Integer, Optional bool As Boolean = False, Optional Stab As Boolean = False, Optional StabDir As Integer = 0)  'for whacking something with the tail
// for whacking something with the tail
void TailSwipe(const int plr, bool boo = false, bool Stab = false, int StabDir = 0);
// Public Sub YoshiHeight(A As Integer) 'fix the players height when on a yoshi
// fix the players height when on a yoshi
void YoshiHeight(const int A);
// Public Sub YoshiEat(A As Integer)
void YoshiEat(const int A);
// Public Sub YoshiSpit(A As Integer)
void YoshiSpit(const int A);
// Public Sub YoshiPound(A As Integer, C As Integer, Optional BreakBlocks As Boolean = False)
void YoshiPound(int A, int mount, bool BreakBlocks = false);


// NEW (but derived from existing code) forces player to jump out of mount as they do for AltJump, bypassing all checks.
void PlayerDismount(const int A);

// Public Sub SwapCoop()
void SwapCoop();
// Public Sub PlayerPush(A As Integer, HitSpot As Integer)
void PlayerPush(const int A, int HitSpot);
// Public Sub SizeCheck(A As Integer)
void SizeCheck(struct Player_t &p);
// Public Sub YoshiEatCode(A As Integer)
void YoshiEatCode(const int A);
// Public Sub StealBonus()
void StealBonus();
// Public Sub ClownCar()
void ClownCar();

// Private Sub WaterCheck(A As Integer)
void WaterCheck(const int A); //PRIVATE
// Private Sub Tanooki(A As Integer)
void Tanooki(const int A); //PRIVATE
// Private Sub PowerUps(A As Integer)
void PowerUps(const int A); //PRIVATE
// Private Sub SuperWarp(A As Integer)
void SuperWarp(const int A); //PRIVATE
// Private Sub PlayerCollide(A As Integer)
void PlayerCollide(const int A); //PRIVATE

// Public Sub PlayerGrabCode(A As Integer, Optional DontResetGrabTime As Boolean = False)
void PlayerGrabCode(const int A, bool DontResetGrabTime = false);
// Public Sub LinkFrame(A As Integer)
void LinkFrame(const int A);
void LinkFrame(struct Player_t &p);
// Private Sub PlayerEffects(A As Integer)
void PlayerEffects(const int A);

// main Drop/Add functions
void DropPlayer(const int A);
void AddPlayer(int Character);

// NEW but, when Die is false and FromBlock is true, identical to hitting character block.
void SwapCharacter(int A, int Character, bool Die = false, bool FromBlock = false);

// returns whether a player is allowed to swap characters
bool SwapCharAllowed();

#endif // PLAYER_H
