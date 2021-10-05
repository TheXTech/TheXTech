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
void PlayerHurt(int A);
// Public Sub PlayerDead(A As Integer) 'Set player up to die
// Set player up to die
void PlayerDead(int A);
// Public Sub KillPlayer(A As Integer) 'Kill the player
// Kill the player
void KillPlayer(int A);
// Public Function CheckDead() As Integer 'Check if players are dead and return which one, returns 0 if everyones dead
// Check if players are dead and return which one, returns 0 if everyones dead
int CheckDead();
// Public Function CheckLiving() As Integer 'Check if players are alive, returns the first player or 0 if everyones dead
// Check if players are alive, returns the first player or 0 if everyones dead
int CheckLiving();
// Public Function LivingPlayers() As Boolean 'true if there are still living players
// true if there are still living players
bool LivingPlayers();
// Public Sub EveryonesDead() 'Handles the game when all players have died
// Handles the game when all players have died
void EveryonesDead();
// Public Sub UnDuck(A As Integer) 'Un Duck the player
// Un Duck the player
void UnDuck(int A);
// Public Sub CheckSection(A As Integer)
void CheckSection(int A);
// Public Sub PlayerFrame(A As Integer)
void PlayerFrame(int A);
// Public Sub UpdatePlayerBonus(A As Integer, B As Integer)
void UpdatePlayerBonus(int A, int B);
// Public Sub TailSwipe(plr As Integer, Optional bool As Boolean = False, Optional Stab As Boolean = False, Optional StabDir As Integer = 0)  'for whacking something with the tail
// for whacking something with the tail
void TailSwipe(int plr, bool boo = false, bool Stab = false, int StabDir = 0);
// Public Sub YoshiHeight(A As Integer) 'fix the players height when on a yoshi
// fix the players height when on a yoshi
void YoshiHeight(int A);
// Public Sub YoshiEat(A As Integer)
void YoshiEat(int A);
// Public Sub YoshiSpit(A As Integer)
void YoshiSpit(int A);
// Public Sub YoshiPound(A As Integer, C As Integer, Optional BreakBlocks As Boolean = False)
void YoshiPound(int A, int mount, bool BreakBlocks = false);


// NEW (but derived from existing code) forces player to jump out of mount as they do for AltJump, bypassing all checks.
void PlayerDismount(int A);

// Public Sub SwapCoop()
void SwapCoop();
// Public Sub PlayerPush(A As Integer, HitSpot As Integer)
void PlayerPush(int A, int HitSpot);
// Public Sub SizeCheck(A As Integer)
void SizeCheck(int A);
// Public Sub YoshiEatCode(A As Integer)
void YoshiEatCode(int A);
// Public Sub StealBonus()
void StealBonus();
// Public Sub ClownCar()
void ClownCar();

// Private Sub WaterCheck(A As Integer)
void WaterCheck(int A); //PRIVATE
// Private Sub Tanooki(A As Integer)
void Tanooki(int A); //PRIVATE
// Private Sub PowerUps(A As Integer)
void PowerUps(int A); //PRIVATE
// Private Sub SuperWarp(A As Integer)
void SuperWarp(int A); //PRIVATE
// Private Sub PlayerCollide(A As Integer)
void PlayerCollide(int A); //PRIVATE

// Public Sub PlayerGrabCode(A As Integer, Optional DontResetGrabTime As Boolean = False)
void PlayerGrabCode(int A, bool DontResetGrabTime = false);
// Public Sub LinkFrame(A As Integer)
void LinkFrame(int A);
// Private Sub PlayerEffects(A As Integer)
void PlayerEffects(int A);

// main Drop/Add functions
void DropPlayer(int A);
void AddPlayer(int Character);

// NEW but, when Die is false, identical to player block.
void SwapCharacter(int A, int Character, bool Die = false, bool Block = false);

#endif // PLAYER_H
