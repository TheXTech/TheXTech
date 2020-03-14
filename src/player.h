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
void YoshiPound(int A, int C, bool BreakBlocks = false);
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


#endif // PLAYER_H
