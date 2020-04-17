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

#include <cmath>
#include <ctime>
#include <Utils/maths.h>
#include <fmt_format_ne.h>
#include <pge_delay.h>

#include "globals.h"
#include "player.h"
#include "graphics.h"
#include "collision.h"
#include "npc.h"
#include "sound.h"
#include "game_main.h"
#include "effect.h"
#include "blocks.h"
#include "editor.h"
#include "layers.h"
#include "main/level_file.h"



void WaterCheck(int A);
// Private Sub Tanooki(A As Integer)
void Tanooki(int A);
// Private Sub PowerUps(A As Integer)
void PowerUps(int A);
// Private Sub SuperWarp(A As Integer)
void SuperWarp(int A);
// Private Sub PlayerCollide(A As Integer)
void PlayerCollide(int A);
// Private Sub PlayerEffects(A As Integer)
void PlayerEffects(int A);

void SetupPlayers()
{
    Location_t tempLocation;
//    Controls_t blankControls;
    int A = 0;
    int B = 0;
    int C = 0;
    FreezeNPCs = false;
    qScreen = false;
    ForcedControls = false;
    // online stuff
    //    if(nPlay.Online == true)
    //    {
    //        for(A = 0; A <= 15; A++)
    //        {
    //            nPlay.Player[A].Controls = blankControls;
    //            nPlay.MyControls = blankControls;
    //        }
    //    }
    //    if(nPlay.Online == true)
    //    {
    //        if(nPlay.Mode == 1)
    //            nPlay.MySlot = 0;
    //        else
    //        {
    //            for(A = 1; A <= 15; A++)
    //            {
    //                if(nPlay.Player[A].IsMe)
    //                {
    //                    nPlay.MySlot = A;
    //                    break;
    //                }
    //            }
    //        }
    //    }

    // battle mode
    if(BattleMode == true)
    {
        for(A = 1; A <= numPlayers; A++)
        {
            Player[A].State = 2;
            Player[A].Hearts = 2;
        }
        pLogDebug("Clear check-points at Battle Mode begining");
        Checkpoint.clear();
        CheckpointsList.clear();
    }
    else
    {
        BattleIntro = 0;
        BattleOutro = 0;
    }


    for(int numPlayersMax = numPlayers, A = 1; A <= numPlayersMax; A++) // set up players
    {
        if(Player[A].Character == 0) // player has no character
        {
            Player[A].Character = 1; // Sets as Mario
            if(numPlayers == 2 && A == 2 /*&& nPlay.Online == false*/) // Sets as Luigi
                Player[A].Character = 2;
        }
        //        if(nPlay.Online == true) // online stuff
        //        {
        //            Player[A].State = 2; // Super mario
        //            Player[A].Mount = 0;
        //            if(A == nPlay.MySlot + 1)
        //            {
        //                if(frmNetplay::optPlayer(2).Value == true)
        //                    Player[A].Character = 2;
        //                else if(frmNetplay::optPlayer(3).Value == true)
        //                    Player[A].Character = 3;
        //                else if(frmNetplay::optPlayer(4).Value == true)
        //                    Player[A].Character = 4;
        //                else
        //                    Player[A].Character = 1;
        //            }
        //        }
        if(Player[A].State == 0) // if no state it defaults to small mario
            Player[A].State = 1;
        // box to hearts

        if(Player[A].Character == 3 || Player[A].Character == 4 || Player[A].Character == 5) // Peach and Toad
        {
            if(Player[A].Hearts <= 0)
                Player[A].Hearts = 1;

            // power up limiter
            // If (.Character = 3 Or .Character = 4) And .State > 3 And .State <> 7 Then .State = 2

            if(Player[A].Hearts <= 1 && Player[A].State > 1 && Player[A].Character != 5)
                Player[A].Hearts = 2;
            if(Player[A].HeldBonus > 0)
            {
                Player[A].Hearts = Player[A].Hearts + 1;
                Player[A].HeldBonus = 0;
            }
            if(Player[A].State == 1 && Player[A].Hearts > 1)
                Player[A].State = 2;
            if(Player[A].Hearts > 3)
                Player[A].Hearts = 3;
            if(Player[A].Mount == 3)
                Player[A].Mount = 0;
        }
        else // Mario and Luigi
        {
            if(Player[A].Hearts == 3 && Player[A].HeldBonus == 0)
                Player[A].HeldBonus = 9;
            Player[A].Hearts = 0;
        }
        if(Player[A].Character == 5)
            Player[A].Mount = 0;

        Player[A].Direction = 1; // Moved from below to here
        Player[A].Location.Width = Physics.PlayerWidth[Player[A].Character][Player[A].State]; // set height
        Player[A].Location.Height = Physics.PlayerHeight[Player[A].Character][Player[A].State]; // set width
        if(Player[A].State == 1 && Player[A].Mount == 1) // if small and in a shoe then set the height to super mario
            Player[A].Location.Height = Physics.PlayerHeight[1][2];
        if(numPlayers == 2 && A == 2)
            B = 2;
        else
            B = 1;
        if(A == 2 && PlayerStart[B].X == 0.0 && PlayerStart[B].Y == 0.0)
        {
            Player[A].Location.X = PlayerStart[1].X + PlayerStart[1].Width * 0.5 - Player[A].Location.Width * 0.5;
            Player[A].Location.Y = PlayerStart[1].Y + PlayerStart[1].Height - Player[A].Location.Height; // - 2
            Player[A].Direction = PlayerStart[1].Direction; // manually defined direction of player
        }
        else
        {
            Player[A].Location.X = PlayerStart[B].X + PlayerStart[B].Width * 0.5 - Player[A].Location.Width * 0.5;
            Player[A].Location.Y = PlayerStart[B].Y + PlayerStart[B].Height - Player[A].Location.Height; // - 2
            Player[A].Direction = PlayerStart[B].Direction; // manually defined direction of player
        }

        Player[A].CanGrabNPCs = GrabAll;

        // reset all variables
        if(Player[A].Mount == 2)
            Player[A].Mount = 0;
        if(Player[A].Character >= 3 && Player[A].Mount == 3)
            Player[A].Mount = 0;

        Player[A].Slippy = false;
        Player[A].DoubleJump = false;
        Player[A].FlySparks = false;
        Player[A].Quicksand = 0;
        Player[A].Bombs = 0;
        Player[A].Wet = 0;
        Player[A].ShellSurf = false;
        Player[A].WetFrame = false;
        Player[A].Slide = false;
        Player[A].Vine = 0;
        Player[A].VineNPC = 0.0;
        Player[A].VineBGO = 0.0;
        Player[A].Fairy = false;
        Player[A].GrabSpeed = 0;
        Player[A].GrabTime = 0;
        Player[A].SwordPoke = 0;
        Player[A].FireBallCD2 = 0;
        Player[A].SpinJump = false;
        Player[A].Stoned = false;
        Player[A].Slope = 0;
        Player[A].SpinFireDir = 0;
        Player[A].SpinFrame = 0;
        Player[A].YoshiNPC = 0;
        Player[A].YoshiPlayer = 0;
        Player[A].YoshiRed = false;
        Player[A].YoshiBlue = false;
        Player[A].YoshiYellow = false;
        Player[A].YoshiBFrame = 0;
        Player[A].YoshiBFrameCount = 0;
        Player[A].YoshiTFrame = 0;
        Player[A].YoshiTFrameCount = 0;
        Player[A].CanFly = false;
        Player[A].CanFly2 = false;
        Player[A].RunCount = 0;
        Player[A].FlyCount = 0;
        Player[A].ForceHitSpot3 = false;
        Player[A].StandUp = false;
        Player[A].StandUp2 = false;
        Player[A].TailCount = 0;
        Player[A].HasKey = false;
        Player[A].TimeToLive = 0;
        Player[A].Warp = 0;
        Player[A].WarpCD = 0;
        Player[A].CanPound = false;
        Player[A].GroundPound = false;
        Player[A].GroundPound2 = false;
        Player[A].Duck = false;
        Player[A].MountSpecial = 0;
        Player[A].YoshiTongueLength = 0;

//        Player[A].Direction = 1; // Moved to above
        Player[A].Location.SpeedX = 0;
        Player[A].Location.SpeedY = 2;
        Player[A].Frame = 1;
        Player[A].FrameCount = 0;
        Player[A].NPCPinched = 0;
        Player[A].Pinched1 = 0;
        Player[A].Pinched2 = 0;
        Player[A].Pinched3 = 0;
        Player[A].StandingOnNPC = 0;
        Player[A].StandingOnTempNPC = 0;
        Player[A].Pinched4 = 0;
        Player[A].HoldingNPC = 0;
        Player[A].Dead = false;
        //        if(nPlay.Online == true && nPlay.Mode == 0)
        //        {
        //            if(nPlay.Player[A - 1].Active == false)
        //                Player[A].Dead = true;
        //        }
        Player[A].TimeToLive = 0;
        Player[A].Bumped = 0;
        Player[A].Bumped2 = 0;
        Player[A].Effect = 0;
        Player[A].Effect2 = 0;
        Player[A].Immune = 0;
        Player[A].Immune2 = false;
        Player[A].Jump = 0;
        Player[A].Frame = 1;
        Player[A].FrameCount = 0;
        Player[A].RunRelease = false;
        Player[A].FloatTime = 0;
        Player[A].CanFloat = false;

        if(Player[A].Character == 3)
            Player[A].CanFloat = true;

        if(Player[A].Character == 3 || Player[A].Character == 4)
        {
            if(Player[A].State == 1)
                Player[A].Hearts = 1;
            if(Player[A].State > 1 && Player[A].Hearts < 2)
                Player[A].Hearts = 2;
        }

        if(numPlayers > 2 && !GameMenu) // online stuff
        {
            /*if(nPlay.Online == true)
            {
                Player[A].Location = Player[1].Location;
                Player[A].Location.X = Player[A].Location.X + A * 32 - 32;
            }
            else*/
            if(GameOutro)
            {
                Player[A].Location = Player[1].Location;
                Player[A].Location.X = Player[A].Location.X + A * 52 - 52;
            }
            else
            {
                Player[A].Location = Player[1].Location;
                Player[A].Location.SpeedY = dRand() * -12 - 6;
            }
        }
        Player[A].Section = -1;
        CheckSection(A); // find the section the player is in
        if(Player[A].Section == -1)
        {
            Player[A].Section = 0;
            CheckSection(A);
        }

        // Set player's direction to left automatically when a start point is located at right side of the section
        if(Player[A].Location.X + Player[A].Location.Width / 2.0 > level[Player[A].Section].X + (level[Player[A].Section].Width - level[Player[A].Section].X) / 2)
            Player[A].Direction = -1;

        //        if(nPlay.Online == true && A <= 15)
        //        {
        //            if(nPlay.Player[A - 1].Active == false && A != 1)
        //                Player[A].Dead = true;
        //        }
        SizeCheck(A);
    }
    //    if(nPlay.Online == true)
    //    {
    //        Netplay::sendData "1d" + (nPlay.MySlot + 1) + "|" + Player[nPlay.MySlot + 1].Character + "|" + Player[nPlay.MySlot + 1].State + LB + Netplay::PutPlayerLoc(nPlay.MySlot + 1);
    //        StartMusic Player[nPlay.MySlot + 1].Section;
    //    }
    SetupScreens(); // setup the screen depending on how many players there are


    if(Checkpoint == FullFileName && !Checkpoint.empty()) // if this level has a checkpoint the put the player in the correct position
    {
        pLogDebug("Trying to restore %zu checkpoints...", CheckpointsList.size());
        for(int cpId = 0; cpId < int(CheckpointsList.size()); cpId++)
        {
            auto &cp = CheckpointsList[size_t(cpId)];

            for(int numNPCsMax = numNPCs, A = 1; A <= numNPCsMax; A++)
            {
                if(NPC[A].Type != 192)
                    continue;

                if(cp.id != Maths::iRound(NPC[A].Special))
                    continue;

                NPC[A].Killed = 9;

                // found a last id, leave player here
                if(cpId == int(CheckpointsList.size() - 1))
                {
                    tempLocation = NPC[A].Location;
                    tempLocation.Height = 600;


                    C = 0;
                    for(B = 1; B <= numBlock; B++)
                    {
                        if(CheckCollision(tempLocation, Block[B].Location) == true)
                        {
                            if(C == 0)
                                C = B;
                            else
                            {
                                if(Block[B].Location.Y < Block[C].Location.Y)
                                    C = B;
                            }
                        }
                    }

                    for(B = 1; B <= numPlayers; B++)
                    {
                        Player[B].Location.Y = Block[C].Location.Y - Player[B].Location.Height;
                        Player[B].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - Player[B].Location.Width / 2.0;
                        CheckSection(B);
                        pLogDebug("Restore player %d at checkpoint ID=%d by X=%g, Y=%g",
                                  B, cp.id, Player[B].Location.X, Player[B].Location.Y);
                    }

                    if(numPlayers > 1)
                    {
                        Player[1].Location.X = Player[1].Location.X - 16;
                        Player[2].Location.X = Player[2].Location.X + 16;
                    }
                    break;// Stop to find NPCs
                }
            }// for NPCs
        } // for Check points
    }
    // if not in the level for the checkpoint, blank the checkpoint
    else if(StartLevel != FileNameFull)
    {
        pLogDebug("Clear check-points at SetupPlayers()");
        Checkpoint.clear();
        CheckpointsList.clear();
    }
}

void PlayerHurt(int A)
{
    if(GodMode == true || GameOutro == true || BattleOutro > 0)
            return;
    Location_t tempLocation;
    int B = 0;

    if(Player[A].Dead == true || Player[A].TimeToLive > 0 || Player[A].Stoned == true || Player[A].Immune > 0 || Player[A].Effect > 0)
        return;
//    if(nPlay.Online == true) // netplay stuffs
//    {
//        if(nPlay.Allow == false && A != nPlay.MySlot + 1)
//            return;
//        if(A == nPlay.MySlot + 1)
//            Netplay::sendData Netplay::PutPlayerLoc(nPlay.MySlot) + "1a" + std::to_string(A) + "|" + Player[A].State + LB;
//    }
    Player[A].DoubleJump = false;
    Player[A].GrabSpeed = 0;
    Player[A].GrabTime = 0;
    Player[A].Slide = false;
    Player[A].SlideKill = false;
    Player[A].CanFly = false;
    Player[A].CanFly2 = false;
    Player[A].FlyCount = 0;
    Player[A].RunCount = 0;
    if(Player[A].Fairy == true)
    {
        PlaySound(87);
        Player[A].Immune = 30;
        Player[A].Effect = 8;
        Player[A].Effect2 = 4;
        Player[A].Fairy = false;
        Player[A].FairyTime = 0;
        SizeCheck(B);
        NewEffect(63, Player[A].Location);
        if(Player[A].Character == 5)
        {
            Player[A].FrameCount = -10;
            Player[A].Location.SpeedX = 3 * -Player[A].Direction;
            Player[A].Location.SpeedY = -7.01;
            Player[A].StandingOnNPC = 0;
            Player[A].FireBallCD = 20;
            PlaySound(78);
        }
        return;
    }
    if(GameMenu == true)
    {
        if(Player[A].State > 1)
            Player[A].Hearts = 2;
        else
            Player[A].Hearts = 1;
    }

    if(NPC[Player[A].HoldingNPC].Type == 13)
        Player[A].HoldingNPC = 0;

    if(LevelMacro == 0)
    {
        if(Player[A].Immune == 0)
        {
            if(Player[A].Mount == 1)
            {
                Player[A].Mount = 0;
                PlaySound(35);
                UnDuck(A);
                tempLocation = Player[A].Location;
                tempLocation.SpeedX = 5 * -Player[A].Direction;
                if(Player[A].MountType == 1)
                    NewEffect(26, tempLocation);
                else if(Player[A].MountType == 2)
                    NewEffect(101, tempLocation);
                else
                    NewEffect(102, tempLocation);
                Player[A].Location.Y = Player[A].Location.Y + Player[A].Location.Height;
                Player[A].Location.Height = Physics.PlayerHeight[Player[A].Character][Player[A].State];
                Player[A].Location.Y = Player[A].Location.Y - Player[A].Location.Height;
                Player[A].Immune = 150;
                Player[A].Immune2 = true;
            }
            else if(Player[A].Mount == 3)
            {
                UnDuck(A);
                PlaySound(49);
                Player[A].Immune = 100;
                Player[A].Immune2 = true;
                Player[A].CanJump = false;
                Player[A].Location.SpeedX = 0;
                if(Player[A].Location.SpeedY > Physics.PlayerJumpVelocity)
                    Player[A].Location.SpeedY = Physics.PlayerJumpVelocity;
                Player[A].Jump = 0;
                Player[A].Mount = 0;
                Player[A].YoshiBlue = false;
                Player[A].YoshiRed = false;
                Player[A].GroundPound = false;
                Player[A].GroundPound2 = false;
                Player[A].YoshiYellow = false;
                Player[A].Dismount = Player[A].Immune;
                numNPCs++;
                NPC[numNPCs] = NPC_t();
                if(Player[A].YoshiNPC > 0 || Player[A].YoshiPlayer > 0)
                {
                    YoshiSpit(A);
                }
                // If ShadowMode = True Then .Shadow = True
                NPC[numNPCs].Direction = Player[A].Direction;
                NPC[numNPCs].Active = true;
                NPC[numNPCs].TimeLeft = 100;
                if(Player[A].MountType == 1)
                    NPC[numNPCs].Type = 95;
                else if(Player[A].MountType == 2)
                    NPC[numNPCs].Type = 98;
                else if(Player[A].MountType == 3)
                    NPC[numNPCs].Type = 99;
                else if(Player[A].MountType == 4)
                    NPC[numNPCs].Type = 100;
                else if(Player[A].MountType == 5)
                    NPC[numNPCs].Type = 148;
                else if(Player[A].MountType == 6)
                    NPC[numNPCs].Type = 149;
                else if(Player[A].MountType == 7)
                    NPC[numNPCs].Type = 150;
                else if(Player[A].MountType == 8)
                    NPC[numNPCs].Type = 228;
                NPC[numNPCs].Special = 1;
                NPC[numNPCs].Location.Height = 32;
                NPC[numNPCs].Location.Width = 32;
                NPC[numNPCs].Location.Y = Player[A].Location.Y + Player[A].Location.Height - 33;
                NPC[numNPCs].Location.X = static_cast<int>(floor(static_cast<double>(Player[A].Location.X + Player[A].Location.Width / 2.0 - 16)));
                NPC[numNPCs].Location.SpeedY = 0.5;
                NPC[numNPCs].Location.SpeedX = 0;
                NPC[numNPCs].CantHurt = 10;
                NPC[numNPCs].CantHurtPlayer = A;
                Player[A].Location.Height = Physics.PlayerHeight[Player[A].Character][Player[A].State];
            }
            else
            {
                if(Player[A].Character == 3 || Player[A].Character == 4)
                {
                    if(Player[A].Hearts == 3 && (Player[A].State == 2 || Player[A].State == 4 || Player[A].State == 5 || Player[A].State == 6))
                    {
                        Player[A].State = 2;
                        Player[A].Immune = 150;
                        Player[A].Immune2 = true;
                        Player[A].Hearts = Player[A].Hearts - 1;
                        PlaySound(76);
                        return;
                    }
                    else
                    {
                        Player[A].Hearts = Player[A].Hearts - 1;
                        if(Player[A].Hearts == 0)
                            Player[A].State = 1;
                        else if(Player[A].State == 3 && Player[A].Hearts == 2)
                        {
                            Player[A].Effect = 227;
                            Player[A].Effect2 = 0;
                            PlaySound(5);
                            return;
                        }
                        else if(Player[A].State == 7 && Player[A].Hearts == 2)
                        {
                            Player[A].Effect = 228;
                            Player[A].Effect2 = 0;
                            PlaySound(5);
                            return;
                        }
                        else
                            Player[A].State = 2;
                    }
                }
                else if(Player[A].Character == 5)
                {
                    Player[A].Hearts = Player[A].Hearts - 1;
                    if(Player[A].Hearts > 0)
                    {
                        if(Player[A].Hearts == 1)
                            Player[A].State = 1;
                        else
                            Player[A].State = 2;
                        if(Player[A].State < 1)
                            Player[A].State = 1;
                        if(Player[A].Mount == 0)
                        {
                            Player[A].FrameCount = -10;
                            Player[A].Location.SpeedX = 3 * -Player[A].Direction;
                            Player[A].Location.SpeedY = -7.01;
                            Player[A].FireBallCD = 30;
                            Player[A].SwordPoke = 0;
                        }
                        Player[A].Immune = 150;
                        Player[A].Immune2 = true;
                        PlaySound(78);
                        return;
                    }
                }
                if(Player[A].State > 1)
                {
                    PlaySound(5);
                    Player[A].StateNPC = 0;
                    Player[A].Effect = 2;
                }
                else
                {
                    PlayerDead(A);
                    Player[A].HoldingNPC = 0;
                    if(Player[A].Mount == 2)
                    {
                        Player[A].Mount = 0;
                        numNPCs++;
                        NPC[numNPCs] = NPC_t();
                        NPC[numNPCs].Direction = Player[A].Direction;
                        if(NPC[numNPCs].Direction == 1)
                            NPC[numNPCs].Frame = 4;
                        NPC[numNPCs].Frame = NPC[numNPCs].Frame + SpecialFrame[2];
                        NPC[numNPCs].Active = true;
                        NPC[numNPCs].TimeLeft = 100;
                        NPC[numNPCs].Type = 56;
                        NPC[numNPCs].Location.Height = 128;
                        NPC[numNPCs].Location.Width = 128;
                        NPC[numNPCs].Location.Y = static_cast<int>(floor(static_cast<double>(Player[A].Location.Y)));
                        NPC[numNPCs].Location.X = static_cast<int>(floor(static_cast<double>(Player[A].Location.X)));
                        NPC[numNPCs].Location.SpeedY = 0;
                        NPC[numNPCs].Location.SpeedX = 0;
                        NPC[numNPCs].CantHurt = 10;
                        NPC[numNPCs].CantHurtPlayer = A;
                        Player[A].Location.Height = Physics.PlayerHeight[Player[A].Character][Player[A].State];
                        Player[A].Location.Width = Physics.PlayerWidth[Player[A].Character][Player[A].State];
                        Player[A].Location.X = Player[A].Location.X + 64 - Physics.PlayerWidth[Player[A].Character][Player[A].State] / 2;
                        Player[A].ForceHitSpot3 = true;
                        Player[A].Location.Y = NPC[numNPCs].Location.Y - Player[A].Location.Height;

                        for(B = 1; B <= numNPCs; B++)
                        {
                            if(NPC[B].standingOnPlayer == A)
                            {
                                NPC[B].standingOnPlayer = 0;
                                NPC[B].Location.SpeedY = 0;
                                NPC[B].Location.Y = NPC[numNPCs].Location.Y - 0.1 - NPC[B].standingOnPlayerY;
                                NPC[B].standingOnPlayerY = 0;
                                if(NPC[B].Type == 22)
                                    NPC[B].Special = 0;
                                if(NPC[B].Type == 50)
                                {
                                    NPC[B].Killed = 9;
                                    NPC[B].Special = 0;
                                }
                                else if(NPC[B].Type == 49)
                                    NPC[B].Special = 0;
                            }
                        }
                    }
                }
            }
        }
    }
}

void PlayerDead(int A)
{
    bool tempBool = false;
    int B = 0;

//    if(nPlay.Online == true) // netplay stuffs
//    {
//        if(nPlay.Allow == false && A != nPlay.MySlot + 1)
//            return;
//        if(A == nPlay.MySlot + 1)
//            Netplay::sendData Netplay::PutPlayerLoc(nPlay.MySlot) + "1b" + std::to_string(A) + LB;
//    }

    if(Player[A].Character == 5)
        PlaySound(80);
    else
    {
        if(BattleMode == true)
            PlaySound(54);
        else if(numPlayers > 2)
        {
            for(B = 1; B <= numPlayers; B++)
            {
                if(Player[B].Dead == false && Player[B].TimeToLive == 0 && A != B)
                    tempBool = true;
            }
            if(tempBool == true)
                PlaySound(54);
            else
                PlaySound(8);
        }
        else
            PlaySound(8);
    }
    if(Player[A].YoshiNPC > 0 || Player[A].YoshiPlayer > 0)
    {
        YoshiSpit(A);
    }
    Player[A].Location.SpeedX = 0;
    Player[A].Location.SpeedY = 0;
    Player[A].Hearts = 0;
    Player[A].Wet = 0;
    Player[A].WetFrame = 0;
    Player[A].Quicksand = 0;
    Player[A].Effect = 0;
    Player[A].Effect2 = 0;
    Player[A].Fairy = false;
    if(Player[A].Mount == 2)
    {
        numNPCs++;
        NPC[numNPCs] = NPC_t();
        NPC[numNPCs].Direction = Player[A].Direction;
        if(Maths::iRound(NPC[numNPCs].Direction) == 1)
            NPC[numNPCs].Frame = 4;
        NPC[numNPCs].Frame = NPC[numNPCs].Frame + SpecialFrame[2];
        NPC[numNPCs].Active = true;
        NPC[numNPCs].TimeLeft = 100;
        NPC[numNPCs].Type = 56;
        NPC[numNPCs].Location.Height = 128;
        NPC[numNPCs].Location.Width = 128;
        NPC[numNPCs].Location.Y = static_cast<int>(floor(static_cast<double>(Player[A].Location.Y)));
        NPC[numNPCs].Location.X = static_cast<int>(floor(static_cast<double>(Player[A].Location.X)));
        NPC[numNPCs].Location.SpeedY = 0;
        NPC[numNPCs].Location.SpeedX = 0;
        NPC[numNPCs].CantHurt = 10;
        NPC[numNPCs].CantHurtPlayer = A;
        Player[A].Mount = 0;
        Player[A].Location.Y = Player[A].Location.Y - 32;
        Player[A].Location.Height = 32;
        SizeCheck(A);
    }
    Player[A].Mount = 0;
    Player[A].State = 1;
    Player[A].HoldingNPC = 0;
    Player[A].GroundPound = false;
    Player[A].GroundPound2 = false;
    if(Player[A].Character == 1)
        NewEffect(3, Player[A].Location, 1, 0, ShadowMode);
    else if(Player[A].Character == 2)
        NewEffect(5, Player[A].Location, 1, 0, ShadowMode);
    else if(Player[A].Character == 3)
        NewEffect(129, Player[A].Location, 1, 0, ShadowMode);
    else if(Player[A].Character == 4)
        NewEffect(130, Player[A].Location, 1, 0, ShadowMode);
    else if(Player[A].Character == 5)
    {
        NewEffect(134, Player[A].Location, static_cast<float>(Player[A].Direction), 0, ShadowMode);
        Effect[numEffects].Location.SpeedX = 2 * -Player[A].Direction;
    }
    Player[A].TimeToLive = 1;
    if(CheckLiving() == 0 && GameMenu == false && BattleMode == false)
    {
        StopMusic();
        FreezeNPCs = false;
    }
    if(A == SingleCoop)
        SwapCoop();
}

void KillPlayer(int A)
{
    Location_t tempLocation;
    Player[A].Location.SpeedX = 0;
    Player[A].Location.SpeedY = 0;
    Player[A].State = 1;
    Player[A].Stoned = false;
    Player[A].Pinched1 = 0;
    Player[A].Pinched2 = 0;
    Player[A].Pinched3 = 0;
    Player[A].Pinched4 = 0;
    Player[A].NPCPinched = 0;
    Player[A].TimeToLive = 0;
    Player[A].Direction = 1;
    Player[A].Frame = 1;
    Player[A].Mount = 0;
    Player[A].Dead = true;
    Player[A].Location.X = 0;
    Player[A].Location.Y = 0;
    Player[A].Location.Width = Physics.PlayerWidth[Player[A].Character][Player[A].State];
    Player[A].Location.Height = Physics.PlayerHeight[Player[A].Character][Player[A].State];

    if(Player[A].HoldingNPC > 0)
    {
        if(NPC[Player[A].HoldingNPC].Type == 272)
            NPC[Player[A].HoldingNPC].Projectile = true;
    }

    Player[A].HoldingNPC = 0;
    if(BattleMode == true)
    {
        if(BattleLives[A] <= 0)
        {
            if(BattleOutro == 0)
            {
                BattleOutro = 1;
                PlaySound(52);
                StopMusic();
            }
            if(BattleWinner == 0)
            {
                if(A == 1)
                    BattleWinner = 2;
                else
                    BattleWinner = 1;
            }
        }
        if(A == BattleWinner || BattleWinner == 0)
        {
            if(BattleLives[A] > 0)
                BattleLives[A] = BattleLives[A] - 1;
            PlaySound(34);
            Player[A].Frame = 1;
            Player[A].Location.SpeedX = 0;
            Player[A].Location.SpeedY = 0;
            Player[A].Mount = 0;
            Player[A].State = 2;
            Player[A].Hearts = 2;
            Player[A].Effect = 0;
            Player[A].Location.Width = Physics.PlayerWidth[Player[A].Character][Player[A].State];
            Player[A].Location.Height = Physics.PlayerHeight[Player[A].Character][Player[A].State];
            Player[A].Location.X = PlayerStart[A].X + PlayerStart[A].Width * 0.5 - Player[A].Location.Width * 0.5;
            Player[A].Location.Y = PlayerStart[A].Y + PlayerStart[A].Height - Player[A].Location.Height;
            Player[A].Direction = 1;
            Player[A].Dead = false;
            CheckSection(A);
            if(Player[A].Location.X + Player[A].Location.Width / 2.0 > level[Player[A].Section].X + (level[Player[A].Section].Width - level[Player[A].Section].X) / 2)
                Player[A].Direction = -1;
            Player[A].Immune = 300;
            tempLocation.Width = 32;
            tempLocation.Height = 32;
            tempLocation.X = Player[A].Location.X + Player[A].Location.Width / 2.0 - tempLocation.Width / 2.0;
            tempLocation.Y = Player[A].Location.Y + Player[A].Location.Height / 2.0 - tempLocation.Height / 2.0;
            NewEffect(131, tempLocation);
        }
    }
}

int CheckDead()
{
    int A = 0;
    for(A = 1; A <= numPlayers; A++)
    {
        if(Player[A].Dead == true && Player[A].State > 0 && Player[A].Character > 0)
        {
//            if(nPlay.Online == false)
            return A;
//            else
//            {
//                if(nPlay.Player[A - 1].Active == true || A == 1)
//                    return A;
//            }
        }
    }
    return 0;
}

int CheckLiving()
{
    int A = 0;
    for(A = 1; A <= numPlayers; A++)
    {
        if(Player[A].Dead == false && Player[A].TimeToLive == 0)
            return A;
    }
    return 0;
}

bool LivingPlayers()
{
    bool tempLivingPlayers = false;
    int A = 0;
    for(A = 1; A <= numPlayers; A++)
    {
        if(Player[A].Dead == false)
            tempLivingPlayers = true;
    }
    return tempLivingPlayers;
}

void EveryonesDead()
{
//    int A = 0; // UNUSED
    if(BattleMode == true)
        return;
    LevelMacro = 0;
    FreezeNPCs = false;
    StopMusic();
    frmMain.clearBuffer();
    frmMain.repaint();
//    if(MagicHand == true)
//        BitBlt frmLevelWindow::vScreen[1].hdc, 0, 0, frmLevelWindow::vScreen[1].ScaleWidth, frmLevelWindow::vScreen[1].ScaleHeight, 0, 0, 0, vbWhiteness;

    PGE_Delay(500);

    Lives--;
    if(Lives >= 0.f)
    {
        LevelMacro = 0;
        LevelMacroCounter = 0;

        ClearLevel();
        if(RestartLevel)
        {
            OpenLevel(FullFileName);
            LevelSelect = false;
            SetupPlayers();
        }
        else
            LevelSelect = true;
    }
    else // no more lives
    {
// GAME OVER
        Lives = 3;
        Coins = 0;
        Score = 0;
        SaveGame();
        LevelMacro = 0;
        LevelMacroCounter = 0;
        ClearLevel();
        LevelSelect = true;
        GameMenu = true;
        MenuMode = 0;
        MenuCursor = 0;
    }
    DoEvents();
}

void UnDuck(int A)
{
    if(Player[A].Duck == true && Player[A].GrabTime == 0) // Player stands up
    {
        if(Player[A].Location.SpeedY != 0.0) // Fixes a block collision bug
            Player[A].StandUp = true;
        Player[A].StandUp2 = true;
        Player[A].Frame = 1;
        Player[A].FrameCount = 0;
        Player[A].Duck = false;
        if(Player[A].Mount == 3)
        {
            Player[A].Location.Y = Player[A].Location.Y + Player[A].Location.Height;
            if(Player[A].State == 1)
                Player[A].Location.Height = 54;
            else
                Player[A].Location.Height = 60;
            Player[A].Location.Y = Player[A].Location.Y - Player[A].Location.Height;
        }
        else
        {
            if(Player[A].State == 1 && Player[A].Mount == 1)
            {
                Player[A].Location.Height = Physics.PlayerHeight[1][2];
                Player[A].Location.Y = Player[A].Location.Y - Physics.PlayerHeight[1][2] + Physics.PlayerDuckHeight[1][2];
            }
            else
            {
                Player[A].Location.Y = Player[A].Location.Y + Player[A].Location.Height;
                Player[A].Location.Height = Physics.PlayerHeight[Player[A].Character][Player[A].State];
                Player[A].Location.Y = Player[A].Location.Y - Player[A].Location.Height;
            }
        }
        SizeCheck(A);
    }
}

void CheckSection(int A)
{
    // finds out what section the player is in and handles the music for section changes
    int B = 0;
    int C = 0;
    int oldSection = 0;
    bool foundSection = false;

    if(LevelSelect)
        return;

    oldSection = Player[A].Section;

    for(B = 0; B <= numSections; B++)
    {
        if(Player[A].Location.X + Player[A].Location.Width >= level[B].X)
        {
            if(Player[A].Location.X <= level[B].Width)
            {
                if(Player[A].Location.Y + Player[A].Location.Height >= level[B].Y)
                {
                    if(Player[A].Location.Y <= level[B].Height)
                    {
                        foundSection = true;
                        if(oldSection != B /*&& (nPlay.Online == false || nPlay.MySlot == A - 1)*/)
                        {
                            ClearBuffer = true;
                            Player[A].Section = B;
//                            if(nPlay.Online == true)
//                            {
//                                if(nPlay.MySlot == A - 1)
//                                    Netplay::sendData "1e" + std::to_string(A) + "|" + Player[A].Section;
//                                else
//                                    return;
//                            }
                            if(curMusic >= 0 && GameMenu == false) // Dont interupt boss / switch music
                            {

                                if(!(curMusic == bgMusic[B]))
                                {
                                    StartMusic(B);
                                }
                                else if(bgMusic[B] == 24)
                                {
                                    if(oldSection >= 0)
                                    {
                                        if(CustomMusic[oldSection] != CustomMusic[Player[A].Section])
                                        {
                                            StartMusic(B);
                                        }
                                    }
                                }
                            }
                            break;
                        }
                    }
                }
            }
        }
    }

    if(!foundSection)
    {
        for(B = 0; B <= numSections; B++)
        {
            if(Player[A].Location.X + Player[A].Location.Width >= LevelREAL[B].X)
            {
                if(Player[A].Location.X <= LevelREAL[B].Width)
                {
                    if(Player[A].Location.Y + Player[A].Location.Height >= LevelREAL[B].Y)
                    {
                        if(Player[A].Location.Y <= LevelREAL[B].Height)
                        {
                            Player[A].Section = B;
//                            if(nPlay.Online == true)
//                            {
//                                if(nPlay.MySlot == A - 1)
//                                    Netplay::sendData "1e" + std::to_string(A) + "|" + Player[A].Section;
//                                else
//                                    return;
//                            }
                            if(oldSection != B)
                            {
                                ClearBuffer = true;
                                if(curMusic != 6 && curMusic >= 0 && curMusic != 15) // Dont interupt boss / switch music
                                {
                                    if(!(curMusic == bgMusic[B]))
                                    {
                                        StartMusic(B);
                                    }
                                    else if(bgMusic[B] == 24)
                                    {
                                        if(CustomMusic[B] != CustomMusic[Player[A].Section])
                                        {
                                            StartMusic(B);
                                        }
                                    }
                                }
                            }
                            for(C = 1; C <= numPlayers; C++)
                            {
                                if(Player[C].Section == Player[A].Section && C != A)
                                {
                                    Player[A].Location.X = Player[C].Location.X + Player[C].Location.Width / 2.0 - Player[A].Location.Width / 2.0;
                                    Player[A].Location.Y = Player[C].Location.Y + Player[C].Location.Height - Player[A].Location.Height - 0.01;
                                    break;
                                }
                            }
                            break;
                        }
                    }
                }
            }
        }
    }
}

void PlayerFrame(int A)
{
// updates the players GFX
    Location_t tempLocation;

// cause the flicker when he is immune
    if(Player[A].Effect != 9)
    {
        if(Player[A].Immune > 0)
        {
            Player[A].Immune = Player[A].Immune - 1;
            if(Player[A].Immune % 3 == 0)
            {
                if(Player[A].Immune2 == false)
                    Player[A].Immune2 = true;
                else
                    Player[A].Immune2 = false;
            }
        }
        else
            Player[A].Immune2 = false;
    }

// find frames for link
    if(Player[A].Character == 5)
    {
        LinkFrame(A);
        return;
    }

// for the grab animation when picking something up from the top
    if(Player[A].GrabTime > 0)
    {
        Player[A].FrameCount = Player[A].FrameCount + 1;
        if(Player[A].FrameCount <= 6)
            Player[A].Frame = 23;
        else if(Player[A].FrameCount <= 12)
            Player[A].Frame = 22;
        else
        {
            Player[A].FrameCount = 0;
            Player[A].Frame = 23;
        }
        return;
    }

// statue frames
    if(Player[A].Stoned)
    {
        Player[A].Frame = 0;
        Player[A].FrameCount = 0;
        if(Player[A].Location.SpeedX != 0.0)
        {
            if(Player[A].Location.SpeedY == 0.0 || Player[A].Slope > 0 || Player[A].StandingOnNPC != 0)
            {
                if(Player[A].SlideCounter <= 0)
                {
                    Player[A].SlideCounter = 2 + dRand() * 2;
                    tempLocation.Y = Player[A].Location.Y + Player[A].Location.Height - 5;
                    tempLocation.X = Player[A].Location.X + Player[A].Location.Width / 2.0 - 4;
                    NewEffect(74, tempLocation, 1, 0, ShadowMode);
                }
            }
        }
        return;
    }

// sliding frames
    if(Player[A].Slide && (Player[A].Character == 1 || Player[A].Character == 2))
    {
        if(Player[A].Location.SpeedX != 0.0)
        {
            if(Player[A].Location.SpeedY == 0.0 || Player[A].Slope > 0 || Player[A].StandingOnNPC != 0)
            {
                if(Player[A].SlideCounter <= 0 && Player[A].SlideKill == true)
                {
                    Player[A].SlideCounter = 2 + dRand() * 2;
                    tempLocation.Y = Player[A].Location.Y + Player[A].Location.Height - 4;
                    if(Player[A].Location.SpeedX < 0)
                        tempLocation.X = Player[A].Location.X + Player[A].Location.Width / 2.0 - 4 + 6;
                    else
                        tempLocation.X = Player[A].Location.X + Player[A].Location.Width / 2.0 - 4 - 6;
                    NewEffect(74, tempLocation, 1, 0, ShadowMode);
                }
            }
        }
        Player[A].Frame = 24;
        return;
    }

// climbing a vine/ladder
    if(Player[A].Vine > 0)
    {
        bool doesPlayerMoves = false;

        if(Player[A].VineBGO > 0)
        {
            doesPlayerMoves = !fEqual(Player[A].Location.SpeedX,  Background[Player[A].VineBGO].Location.SpeedX) ||
                               Player[A].Location.SpeedY < Background[Player[A].VineBGO].Location.SpeedY - 0.1;
        }
        else
        {
            doesPlayerMoves = !fEqual(Player[A].Location.SpeedX,  NPC[Player[A].VineNPC].Location.SpeedX) ||
                               Player[A].Location.SpeedY < NPC[Player[A].VineNPC].Location.SpeedY - 0.1;
        }

        if(doesPlayerMoves) // Or .Location.SpeedY > 0.1 Then
        {
            Player[A].FrameCount += 1;
            if(Player[A].FrameCount >= 8)
            {
                Player[A].Frame += 1;
                Player[A].FrameCount = 0;
            }
            PlaySound(71);
        }

        if(Player[A].Frame < 25)
            Player[A].Frame = 26;
        else if(Player[A].Frame > 26)
            Player[A].Frame = 25;

        return;
    }

// this finds the players direction
    if(!LevelSelect && Player[A].Effect != 3)
    {
        if(!(Player[A].Mount == 3 && Player[A].MountSpecial > 0))
        {
            if(Player[A].Controls.Left == true)
                Player[A].Direction = -1;
            if(Player[A].Controls.Right == true)
                Player[A].Direction = 1;
        }
    }

    if(Player[A].Driving == true && Player[A].StandingOnNPC > 0)
        Player[A].Direction = NPC[Player[A].StandingOnNPC].DefaultDirection;

    if(Player[A].Fairy == true)
        return;

// ducking and holding
    if(Player[A].HoldingNPC > 0 && Player[A].Duck == true)
    {
        Player[A].Frame = 27;
        return;
    }

    Player[A].MountOffsetY = 0;

// for the spinjump/shellsurf
    if((Player[A].SpinJump || Player[A].ShellSurf) && Player[A].Mount == 0)
    {
        if(Player[A].SpinFrame < 4 || Player[A].SpinFrame >= 9)
            Player[A].Direction = -1;
        else
            Player[A].Direction = 1;
        if(Player[A].ShellSurf)
        {
            if(NPC[Player[A].StandingOnNPC].Location.SpeedX > 0)
                Player[A].Direction = -Player[A].Direction;
        }
        Player[A].SpinFrame = Player[A].SpinFrame + 1;
        if(Player[A].SpinFrame < 0)
            Player[A].SpinFrame = 14;
        if(Player[A].SpinFrame < 3)
        {
            Player[A].Frame = 1;
            if(Player[A].HoldingNPC > 0)
            {
                if(Player[A].State == 1)
                    Player[A].Frame = 5;
                else
                    Player[A].Frame = 8;
            }
            if(Player[A].State == 4 || Player[A].State == 5)
                Player[A].Frame = 12;
        }
        else if(Player[A].SpinFrame < 6)
            Player[A].Frame = 13;
        else if(Player[A].SpinFrame < 9)
        {
            Player[A].Frame = 1;
            if(Player[A].HoldingNPC > 0)
            {
                if(Player[A].State == 1)
                    Player[A].Frame = 5;
                else
                    Player[A].Frame = 8;
            }
            if(Player[A].State == 4 || Player[A].State == 5)
                Player[A].Frame = 12;
        }
        else if(Player[A].SpinFrame < 12 - 1)
            Player[A].Frame = 15;
        else
        {
            Player[A].Frame = 15;
            Player[A].SpinFrame = -1;
        }
    }
    else
    {
        if(Player[A].State == 1 && (Player[A].Character == 1 || Player[A].Character == 2)) // Small Mario & Luigi
        {
            if(Player[A].HoldingNPC == 0) // not holding anything
            {
                if(Player[A].WetFrame == true && Player[A].Location.SpeedY != 0 && Player[A].Slope == 0 && Player[A].StandingOnNPC == 0 && Player[A].Duck == false && Player[A].Quicksand == 0) // swimming
                {
                    if(Player[A].Location.SpeedY < 0 || Player[A].Frame == 42 || Player[A].Frame == 43)
                    {
                        if(Player[A].Frame != 40 && Player[A].Frame != 42 && Player[A].Frame != 43)
                            Player[A].FrameCount = 6;
                        Player[A].FrameCount = Player[A].FrameCount + 1;
                        if(Player[A].FrameCount < 6)
                            Player[A].Frame = 40;
                        else if(Player[A].FrameCount < 12)
                            Player[A].Frame = 42;
                        else if(Player[A].FrameCount < 18)
                            Player[A].Frame = 43;
                        else
                        {
                            Player[A].Frame = 43;
                            Player[A].FrameCount = 0;
                        }
                    }
                    else
                    {
                        Player[A].FrameCount = Player[A].FrameCount + 1;
                        if(Player[A].FrameCount < 10)
                            Player[A].Frame = 40;
                        else if(Player[A].FrameCount < 20)
                            Player[A].Frame = 41;
                        else
                        {
                            Player[A].Frame = 41;
                            Player[A].FrameCount = 0;
                        }
                    }
                }
                else // not swimming
                {
                    if(Player[A].Location.SpeedY == 0 || Player[A].StandingOnNPC != 0 || Player[A].Slope > 0 || (Player[A].Location.SpeedY > 0 && Player[A].Quicksand > 0))
                    {
                        if(Player[A].Location.SpeedX > 0 && (Player[A].Controls.Left == true || (Player[A].Direction == -1 && Player[A].Bumped == true)) && Player[A].Effect == 0 && Player[A].Quicksand == 0)
                        {
                            if(LevelSelect == false)
                            {
                                if(!(Player[A].Mount == 2) && Player[A].WetFrame == false && Player[A].Duck == false)
                                {
                                    PlaySound(10);
                                    if(Player[A].SlideCounter <= 0)
                                    {
                                        Player[A].SlideCounter = 2 + dRand() * 2;
                                        tempLocation.Y = Player[A].Location.Y + Player[A].Location.Height - 5;
                                        tempLocation.X = Player[A].Location.X + Player[A].Location.Width / 2.0 - 4 + 8 * -Player[A].Direction;
                                        NewEffect(74, tempLocation, 1, 0, ShadowMode);
                                    }
                                }
                                Player[A].Frame = 4;
                            }
                        }
                        else if(Player[A].Location.SpeedX < 0 && (Player[A].Controls.Right == true || (Player[A].Direction == 1 && Player[A].Bumped == true)) && Player[A].Effect == 0 && Player[A].Quicksand == 0)
                        {
                            if(LevelSelect == false)
                            {
                                if(!(Player[A].Mount == 2) && Player[A].WetFrame == false && Player[A].Duck == false)
                                {
                                    PlaySound(10);
                                    if(Player[A].SlideCounter <= 0)
                                    {
                                        Player[A].SlideCounter = 2 + iRand() % 2;
                                        tempLocation.Y = Player[A].Location.Y + Player[A].Location.Height - 5;
                                        tempLocation.X = Player[A].Location.X + Player[A].Location.Width / 2.0 - 4 + 8 * -Player[A].Direction;
                                        NewEffect(74, tempLocation, 1, 0, ShadowMode);
                                    }
                                }
                                Player[A].Frame = 4;
                            }
                        }
                        else
                        {
                            if(Player[A].Location.SpeedX != 0 && !(Player[A].Slippy == true && Player[A].Controls.Left == false && Player[A].Controls.Right == false))
                            {
                                Player[A].FrameCount = Player[A].FrameCount + 1;
                                if(Player[A].Location.SpeedX > Physics.PlayerWalkSpeed - 1.5 || Player[A].Location.SpeedX < -Physics.PlayerWalkSpeed + 1.5)
                                    Player[A].FrameCount = Player[A].FrameCount + 1;
                                if(Player[A].Location.SpeedX > Physics.PlayerWalkSpeed || Player[A].Location.SpeedX < -Physics.PlayerWalkSpeed)
                                    Player[A].FrameCount = Player[A].FrameCount + 1;
                                if(Player[A].Location.SpeedX > Physics.PlayerWalkSpeed + 1 || Player[A].Location.SpeedX < -Physics.PlayerWalkSpeed - 1)
                                    Player[A].FrameCount = Player[A].FrameCount + 1;
                                if(Player[A].Location.SpeedX > Physics.PlayerWalkSpeed + 2 || Player[A].Location.SpeedX < -Physics.PlayerWalkSpeed - 2)
                                    Player[A].FrameCount = Player[A].FrameCount + 1;
                                if(Player[A].FrameCount >= 10)
                                {
                                Player[A].FrameCount = 0;
                                    if(Player[A].Frame == 1)
                                        Player[A].Frame = 2;
                                    else
                                        Player[A].Frame = 1;
                                }
                            }
                            else
                            {
                                Player[A].Frame = 1;
                                Player[A].FrameCount = 0;
                            }
                        }
                    }
                    else
                        Player[A].Frame = 3;
                }
            }
            else
            {
                if(Player[A].Location.SpeedY == 0 || Player[A].StandingOnNPC != 0 || Player[A].Slope > 0)
                {
                    if(Player[A].Mount != 2 && ((Player[A].Controls.Left == true && Player[A].Location.SpeedX > 0) || (Player[A].Controls.Right == true && Player[A].Location.SpeedX < 0)) && Player[A].Effect == 0 && Player[A].Duck == false)
                    {
                        PlaySound(10);
                        if(Player[A].SlideCounter <= 0)
                        {
                            Player[A].SlideCounter = 2 + iRand() % 2;
                            tempLocation.Y = Player[A].Location.Y + Player[A].Location.Height - 5;
                            tempLocation.X = Player[A].Location.X + Player[A].Location.Width / 2.0 - 4 + 10 * -Player[A].Direction;
                            NewEffect(74, tempLocation, 1, 0, ShadowMode);
                        }
                    }
                    if(Player[A].Location.SpeedX != 0)
                    {
                        Player[A].FrameCount = Player[A].FrameCount + 2;
                        if(Player[A].Location.SpeedX > Physics.PlayerWalkSpeed || Player[A].Location.SpeedX < -Physics.PlayerWalkSpeed)
                            Player[A].FrameCount = Player[A].FrameCount + 3;
                        if(Player[A].FrameCount >= 10)
                        {
                        Player[A].FrameCount = 0;
                            if(Player[A].Frame == 5)
                                Player[A].Frame = 6;
                            else
                                Player[A].Frame = 5;
                        }
                    }
                    else
                    {
                        Player[A].Frame = 5;
                        Player[A].FrameCount = 0;
                    }
                }
                else
                    Player[A].Frame = 6;
            }
        }
        else if(Player[A].FrameCount >= 100 && Player[A].FrameCount <= 118 && (Player[A].State == 3 || Player[A].State == 6 || Player[A].State == 7)) // Fire Mario and Luigi
        {
            if(Player[A].Duck == true)
            {
                Player[A].FrameCount = 0;
                Player[A].Frame = 7;
            }
            else
            {
                if(Player[A].FrameCount <= 106)
                {
                    Player[A].Frame = 11;
                    if(Player[A].WetFrame == true && Player[A].Quicksand == 0 && Player[A].Location.SpeedY != 0 && Player[A].Slope == 0 && Player[A].StandingOnNPC == 0 && Player[A].Character <= 2)
                        Player[A].Frame = 43;
                }
                else if(Player[A].FrameCount <= 112)
                {
                    Player[A].Frame = 12;
                    if(Player[A].WetFrame == true && Player[A].Quicksand == 0 && Player[A].Location.SpeedY != 0 && Player[A].Slope == 0 && Player[A].StandingOnNPC == 0 && Player[A].Character <= 2)
                        Player[A].Frame = 44;
                }
                else
                {
                    Player[A].Frame = 11;
                    if(Player[A].WetFrame == true && Player[A].Quicksand == 0 && Player[A].Location.SpeedY != 0 && Player[A].Slope == 0 && Player[A].StandingOnNPC == 0 && Player[A].Character <= 2)
                        Player[A].Frame = 43;
                }
                Player[A].FrameCount = Player[A].FrameCount + 1;
                if(FlameThrower == true)
                    Player[A].FrameCount = Player[A].FrameCount + 2;
                if(Player[A].FrameCount > 118)
                    Player[A].FrameCount = 0;
            }
        }
        else if(Player[A].TailCount > 0) // Racoon Mario
        {
            if(Player[A].TailCount < 5 || Player[A].TailCount >= 20)
                Player[A].Frame = 12;
            else if(Player[A].TailCount < 10)
                Player[A].Frame = 15;
            else if(Player[A].TailCount < 15)
                Player[A].Frame = 14;
            else
                Player[A].Frame = 13;
        }
        else // Large Mario, Luigi, and Peach
        {
            if(Player[A].HoldingNPC == 0 || (Player[A].Effect == 3 && Player[A].Character >= 3))
            {
                if(Player[A].WetFrame == true && Player[A].Location.SpeedY != 0 && Player[A].Slope == 0 && Player[A].StandingOnNPC == 0 && Player[A].Duck == false && Player[A].Quicksand == 0)
                {
                    if(Player[A].Location.SpeedY < 0 || Player[A].Frame == 43 || Player[A].Frame == 44)
                    {
                        if(Player[A].Character <= 2)
                        {
                            if(Player[A].Frame != 40 && Player[A].Frame != 43 && Player[A].Frame != 44)
                                Player[A].FrameCount = 6;
                        }

                        Player[A].FrameCount = Player[A].FrameCount + 1;
                        if(Player[A].FrameCount < 6)
                            Player[A].Frame = 40;
                        else if(Player[A].FrameCount < 12)
                            Player[A].Frame = 43;
                        else if(Player[A].FrameCount < 18)
                            Player[A].Frame = 44;
                        else
                        {
                            Player[A].Frame = 44;
                            Player[A].FrameCount = 0;
                        }
                    }
                    else
                    {
                        Player[A].FrameCount = Player[A].FrameCount + 1;
                        if(Player[A].FrameCount < 10)
                            Player[A].Frame = 40;
                        else if(Player[A].FrameCount < 20)
                            Player[A].Frame = 41;
                        else if(Player[A].FrameCount < 30)
                            Player[A].Frame = 42;
                        else if(Player[A].FrameCount < 40)
                            Player[A].Frame = 41;
                        else
                        {
                            Player[A].Frame = 41;
                            Player[A].FrameCount = 0;
                        }
                    }

                    if(Player[A].Character >= 3)
                    {
                        if(Player[A].Frame == 43)
                            Player[A].Frame = 1;
                        else if(Player[A].Frame == 44)
                            Player[A].Frame = 2;
                        else
                            Player[A].Frame = 5;

                    }

                }
                else
                {
                    if(Player[A].Location.SpeedY == 0 || Player[A].StandingOnNPC != 0 || Player[A].Slope > 0 || (Player[A].Quicksand > 0 && Player[A].Location.SpeedY > 0))
                    {
                        if(Player[A].Location.SpeedX > 0 && (Player[A].Controls.Left == true || (Player[A].Direction == -1 && Player[A].Bumped == true)) && Player[A].Effect == 0 && Player[A].Duck == false && Player[A].Quicksand == 0)
                        {
                            if(LevelSelect == false)
                            {
                                if(!(Player[A].Mount == 2) && Player[A].Wet == 0)
                                {
                                    PlaySound(10);
                                    if(Player[A].SlideCounter <= 0)
                                    {
                                        Player[A].SlideCounter = 2 + iRand() % 2;
                                        tempLocation.Y = Player[A].Location.Y + Player[A].Location.Height - 5;
                                        tempLocation.X = Player[A].Location.X + Player[A].Location.Width / 2.0 - 4 + 6 * -Player[A].Direction;
                                        NewEffect(74, tempLocation, 1, 0, ShadowMode);
                                    }
                                }
                                Player[A].Frame = 6;
                            }
                        }
                        else if(Player[A].Location.SpeedX < 0 && (Player[A].Controls.Right == true || (Player[A].Direction == 1 && Player[A].Bumped == true)) && Player[A].Effect == 0 && Player[A].Duck == false && Player[A].Quicksand == 0)
                        {
                            if(LevelSelect == false)
                            {
                                if(!(Player[A].Mount == 2) && Player[A].Wet == 0)
                                {
                                    PlaySound(10);
                                    if(Player[A].SlideCounter <= 0)
                                    {
                                        Player[A].SlideCounter = 2 + iRand() % 2;
                                        tempLocation.Y = Player[A].Location.Y + Player[A].Location.Height - 5;
                                        tempLocation.X = Player[A].Location.X + Player[A].Location.Width / 2.0 - 4 + 10 * -Player[A].Direction;
                                        NewEffect(74, tempLocation, 1, 0, ShadowMode);
                                    }
                                }
                                Player[A].Frame = 6;
                            }
                        }
                        else
                        {
                            if(Player[A].Location.SpeedX != 0 && !(Player[A].Slippy == true && Player[A].Controls.Left == false && Player[A].Controls.Right == false))
                            {
                                Player[A].FrameCount = Player[A].FrameCount + 1;
                                if(Player[A].Location.SpeedX >= Physics.PlayerWalkSpeed || Player[A].Location.SpeedX <= -Physics.PlayerWalkSpeed)
                                    Player[A].FrameCount = Player[A].FrameCount + 1;
                                if(Player[A].Location.SpeedX > Physics.PlayerWalkSpeed + 1.5 || Player[A].Location.SpeedX < -Physics.PlayerWalkSpeed - 1.5)
                                    Player[A].FrameCount = Player[A].FrameCount + 1;
                                if(Player[A].FrameCount >= 5 && Player[A].FrameCount < 10)
                                {
                                    if(Player[A].CanFly == true && Player[A].Character != 3)
                                        Player[A].Frame = 16;
                                    else
                                        Player[A].Frame = 1;
                                }
                                else if(Player[A].FrameCount >= 10 && Player[A].FrameCount < 15)
                                {
                                    if(Player[A].CanFly == true && Player[A].Character != 3)
                                        Player[A].Frame = 17;
                                    else
                                        Player[A].Frame = 2;
                                }
                                else if(Player[A].FrameCount >= 15 && Player[A].FrameCount < 20)
                                {
                                    if(Player[A].CanFly == true && Player[A].Character != 3)
                                        Player[A].Frame = 18;
                                    else
                                        Player[A].Frame = 3;
                                }
                                else if(Player[A].FrameCount >= 20)
                                {
                                    Player[A].FrameCount = Player[A].FrameCount - 20;
                                    if(Player[A].CanFly == true && Player[A].Character != 3)
                                        Player[A].Frame = 17;
                                    else
                                        Player[A].Frame = 2;
                                }
                            }
                            else
                            {
                                Player[A].Frame = 1;
                                Player[A].FrameCount = 0;
                            }
                        }
                    }
                    else
                    {
                        if(Player[A].CanFly2 == true)
                        {
                            if(Player[A].Controls.Jump == false && Player[A].Controls.AltJump == false)
                            {
                                if(Player[A].Location.SpeedY < 0)
                                    Player[A].Frame = 19;
                                else
                                    Player[A].Frame = 21;
                            }
                            else
                            {
                                Player[A].FrameCount = Player[A].FrameCount + 1;
                                if(!(Player[A].Frame == 19 || Player[A].Frame == 20 || Player[A].Frame == 21))
                                    Player[A].Frame = 19;
                                if(Player[A].FrameCount >= 5)
                                {
                                    Player[A].FrameCount = 0;
                                    if(Player[A].Frame == 19)
                                        Player[A].Frame = 20;
                                    else if(Player[A].Frame == 20)
                                        Player[A].Frame = 21;
                                    else
                                        Player[A].Frame = 19;
                                }
                            }
                        }
                        else
                        {
                            if(Player[A].Location.SpeedY < 0)
                                Player[A].Frame = 4;
                            else
                            {
                                if((Player[A].State == 4 || Player[A].State == 5) && Player[A].Controls.Jump == true && !(Player[A].Character == 3 || Player[A].Character == 4))
                                {
                                    Player[A].FrameCount = Player[A].FrameCount + 1;
                                    if(!(Player[A].Frame == 3 || Player[A].Frame == 5 || Player[A].Frame == 11))
                                        Player[A].Frame = 11;
                                    if(Player[A].FrameCount >= 5)
                                    {
                                        Player[A].FrameCount = 0;
                                        if(Player[A].Frame == 11)
                                            Player[A].Frame = 3;
                                        else if(Player[A].Frame == 3)
                                            Player[A].Frame = 5;
                                        else
                                            Player[A].Frame = 11;
                                    }
                                }
                                else
                                    Player[A].Frame = 5;
                            }
                        }
                    }
                    if(Player[A].Duck == true)
                        Player[A].Frame = 7;
                }
            }
            else
            {
                if(Player[A].Location.SpeedY == 0 || Player[A].StandingOnNPC != 0 || Player[A].Slope > 0)
                {
                    if(Player[A].Mount != 2 && ((Player[A].Controls.Left == true && Player[A].Location.SpeedX > 0) || (Player[A].Controls.Right == true && Player[A].Location.SpeedX < 0)) && Player[A].Effect == 0 && Player[A].Duck == false)
                    {
                        PlaySound(10);
                        if(Player[A].SlideCounter <= 0)
                        {
                            Player[A].SlideCounter = 2 + iRand() % 2;
                            tempLocation.Y = Player[A].Location.Y + Player[A].Location.Height - 5;
                            tempLocation.X = Player[A].Location.X + Player[A].Location.Width / 2.0 - 4 + 10 * -Player[A].Direction;
                            NewEffect(74, tempLocation, 1, 0, ShadowMode);
                        }
                    }
                    if(Player[A].Location.SpeedX != 0)
                    {
                        Player[A].FrameCount = Player[A].FrameCount + 1;
                        if(Player[A].Location.SpeedX > Physics.PlayerWalkSpeed || Player[A].Location.SpeedX < -Physics.PlayerWalkSpeed)
                            Player[A].FrameCount = Player[A].FrameCount + 1;
                        if(Player[A].FrameCount >= 5 && Player[A].FrameCount < 10)
                            Player[A].Frame = 8;
                        else if(Player[A].FrameCount >= 10 && Player[A].FrameCount < 15)
                            Player[A].Frame = 9;
                        else if(Player[A].FrameCount >= 15 && Player[A].FrameCount < 20)
                            Player[A].Frame = 10;
                        else if(Player[A].FrameCount >= 20)
                        {
                            Player[A].FrameCount = 0;
                            Player[A].Frame = 9;
                        }
                    }
                    else
                    {
                        Player[A].Frame = 8;
                        Player[A].FrameCount = 0;
                    }
                }
                else
                {
                    Player[A].Frame = 10;
                    if(Player[A].Character == 3)
                        Player[A].Frame = 9;
                }
            }
        }
        if(Player[A].Mount == 1) // Goomba's Shoe
        {
            Player[A].MountOffsetY = -Player[A].Location.SpeedY / 2;
            if(Player[A].Duck == true || Player[A].StandingOnNPC != 0)
                Player[A].MountOffsetY = 0;
            if(Player[A].Direction == 1)
                Player[A].MountFrame = 2 + SpecialFrame[1];
            else
                Player[A].MountFrame = 0 + SpecialFrame[1];
            Player[A].Frame = 1;
        }
        else if(Player[A].Mount == 2) // Koopa Clown Car
        {
            Player[A].Frame = 1;
            Player[A].MountFrame = SpecialFrame[2];
            if(Player[A].Direction == 1)
                Player[A].MountFrame = Player[A].MountFrame + 4;
        }
        else if(Player[A].Mount == 3) // Green Yoshi
        {
            Player[A].YoshiBY = 42;
            Player[A].YoshiBX = 0;
            Player[A].YoshiTY = 10;
            Player[A].YoshiTX = 20;
            Player[A].Frame = 30;
            Player[A].YoshiBFrame = 0;
            Player[A].YoshiTFrame = 0;
            Player[A].MountOffsetY = 0;
            if(Player[A].Location.SpeedY < 0 && Player[A].StandingOnNPC == 0 && Player[A].Slope == 0)
            {
                Player[A].YoshiBFrame = 3;
                Player[A].YoshiTFrame = 2;
            }
            else if(Player[A].Location.SpeedY > 0 && Player[A].StandingOnNPC == 0 && Player[A].Slope == 0)
            {
                Player[A].YoshiBFrame = 2;
                Player[A].YoshiTFrame = 0;
            }
            else
            {
                if(Player[A].Location.SpeedX != 0.0)
                {
                    if(Player[A].Effect == 0)
                        Player[A].YoshiBFrameCount = Player[A].YoshiBFrameCount + 1 + (std::abs(Player[A].Location.SpeedX * 0.7));
                    if((Player[A].Direction == -1 && Player[A].Location.SpeedX > 0) || (Player[A].Direction == 1 && Player[A].Location.SpeedX < 0))
                        Player[A].YoshiBFrameCount = 24;
                    if(Player[A].YoshiBFrameCount < 0)
                        Player[A].YoshiBFrameCount = 0;
                    if(Player[A].YoshiBFrameCount > 32)
                    {
                        Player[A].YoshiBFrame = 0;
                        Player[A].YoshiBFrameCount = 0;
                    }
                    else if(Player[A].YoshiBFrameCount > 24)
                    {
                        Player[A].YoshiBFrame = 1;
                        Player[A].YoshiTX = Player[A].YoshiTX - 1;
                        Player[A].YoshiTY = Player[A].YoshiTY + 2;
                        Player[A].YoshiBY = Player[A].YoshiBY + 1;
                        Player[A].MountOffsetY = Player[A].MountOffsetY + 1;
                    }
                    else if(Player[A].YoshiBFrameCount > 16)
                    {
                        Player[A].YoshiBFrame = 2;
                        Player[A].YoshiTX = Player[A].YoshiTX - 2;
                        Player[A].YoshiTY = Player[A].YoshiTY + 4;
                        Player[A].YoshiBY = Player[A].YoshiBY + 2;
                        Player[A].MountOffsetY = Player[A].MountOffsetY + 2;
                    }
                    else if(Player[A].YoshiBFrameCount > 8)
                    {
                        Player[A].YoshiBFrame = 1;
                        Player[A].YoshiTX = Player[A].YoshiTX - 1;
                        Player[A].YoshiTY = Player[A].YoshiTY + 2;
                        Player[A].YoshiBY = Player[A].YoshiBY + 1;
                        Player[A].MountOffsetY = Player[A].MountOffsetY + 1;
                    }
                    else
                        Player[A].YoshiBFrame = 0;
                }
                else
                    Player[A].YoshiBFrameCount = 0;
            }
            if(Player[A].MountSpecial == 1)
            {
                if(Player[A].Controls.Up == true || (Player[A].StandingOnNPC == 0 && Player[A].Location.SpeedY != 0.0 && Player[A].Slope == 0 && Player[A].Controls.Down == false))
                {
                    // .YoshiBFrame = 0
                    Player[A].YoshiTFrame = 3;
                    Player[A].MountOffsetY = Player[A].MountOffsetY;
                    Player[A].YoshiTongue.Y = Player[A].YoshiTongue.Y + Player[A].MountOffsetY;
                }
                else
                {
                    // defaults
                    Player[A].YoshiBY = 42;
                    Player[A].YoshiBX = 0;
                    Player[A].YoshiTY = 10;
                    Player[A].YoshiTX = 20;
                    Player[A].YoshiBFrame = 5;
                    Player[A].YoshiTFrame = 4;
                    Player[A].YoshiBY = Player[A].YoshiBY + 8;
                    Player[A].YoshiTY = Player[A].YoshiTY + 24;
                    Player[A].YoshiTX = Player[A].YoshiTX + 12;
                    Player[A].MountOffsetY = 0;
                    Player[A].MountOffsetY = Player[A].MountOffsetY + 8;
                }
            }
            if(Player[A].Duck == true)
            {
                Player[A].Frame = 31;
                if(Player[A].MountSpecial == 0)
                    Player[A].YoshiBFrame = 6;
                Player[A].YoshiBFrameCount = 0;
            }
            if(Player[A].YoshiTFrameCount > 0)
            {
                if(Player[A].YoshiNPC == 0 && Player[A].YoshiPlayer == 0)
                    Player[A].YoshiTFrameCount = Player[A].YoshiTFrameCount + 1;
                if(Player[A].YoshiTFrameCount < 10)
                    Player[A].YoshiTFrame = 1;
                else if(Player[A].YoshiTFrameCount < 20)
                    Player[A].YoshiTFrame = 2;
                else
                    Player[A].YoshiTFrameCount = 0;
            }
            else if(Player[A].YoshiTFrameCount < 0)
            {
                Player[A].YoshiTFrameCount = Player[A].YoshiTFrameCount - 1;
                if(Player[A].YoshiTFrameCount > -10)
                    Player[A].YoshiTFrame = 3;
                else
                    Player[A].YoshiTFrameCount = 0;
            }
            if(Player[A].Direction == 1)
            {
                Player[A].YoshiTFrame = Player[A].YoshiTFrame + 5;
                Player[A].YoshiBFrame = Player[A].YoshiBFrame + 7;
            }
            else
            {
                Player[A].YoshiBX = -Player[A].YoshiBX;
                Player[A].YoshiTX = -Player[A].YoshiTX;
            }
            if(Player[A].Duck == false || Player[A].MountSpecial > 0)
            {
                Player[A].MountOffsetY = Player[A].MountOffsetY - (72 - Player[A].Location.Height);
                Player[A].YoshiBY = Player[A].YoshiBY - (72 - Player[A].Location.Height);
                Player[A].YoshiTY = Player[A].YoshiTY - (72 - Player[A].Location.Height);
            }
            else
            {
                Player[A].MountOffsetY = Player[A].MountOffsetY - (64 - Player[A].Location.Height);
                Player[A].YoshiBY = Player[A].YoshiBY - (64 - Player[A].Location.Height);
                Player[A].YoshiTY = Player[A].YoshiTY - (64 - Player[A].Location.Height);
            }
            Player[A].YoshiBX = Player[A].YoshiBX - 4;
            Player[A].YoshiTX = Player[A].YoshiTX - 4;
            if(Player[A].YoshiBlue == true)
            {
                if(Player[A].Location.SpeedY == 0 || Player[A].StandingOnNPC != 0)
                    Player[A].YoshiWingsFrame = 1;
                else if(Player[A].Location.SpeedY < 0)
                {
                    Player[A].YoshiWingsFrameCount = Player[A].YoshiWingsFrameCount + 1;
                    if(Player[A].YoshiWingsFrameCount < 6)
                        Player[A].YoshiWingsFrame = 1;
                    else if(Player[A].YoshiWingsFrameCount < 12)
                        Player[A].YoshiWingsFrame = 0;
                    else
                    {
                        Player[A].YoshiWingsFrameCount = 0;
                        Player[A].YoshiWingsFrame = 0;
                    }
                }
                else
                {
                    Player[A].YoshiWingsFrameCount = Player[A].YoshiWingsFrameCount + 1;
                    if(Player[A].YoshiWingsFrameCount < 12)
                        Player[A].YoshiWingsFrame = 1;
                    else if(Player[A].YoshiWingsFrameCount < 24)
                        Player[A].YoshiWingsFrame = 0;
                    else
                    {
                        Player[A].YoshiWingsFrameCount = 0;
                        Player[A].YoshiWingsFrame = 0;
                    }
                }
                if(Player[A].GroundPound == true)
                    Player[A].YoshiWingsFrame = 0;
                if(Player[A].Direction == 1)
                    Player[A].YoshiWingsFrame = Player[A].YoshiWingsFrame + 2;
            }
        }
    }
    if(Player[A].Mount == 1 && Player[A].MountType == 3)
    {
        if(Player[A].Location.SpeedY == 0.0 || Player[A].StandingOnNPC != 0)
            Player[A].YoshiWingsFrame = 1;
        else if(Player[A].Location.SpeedY < 0)
        {
            Player[A].YoshiWingsFrameCount = Player[A].YoshiWingsFrameCount + 1;
            if(Player[A].YoshiWingsFrameCount < 6)
                Player[A].YoshiWingsFrame = 1;
            else if(Player[A].YoshiWingsFrameCount < 12)
                Player[A].YoshiWingsFrame = 0;
            else
            {
                Player[A].YoshiWingsFrameCount = 0;
                Player[A].YoshiWingsFrame = 0;
            }
        }
        else
        {
            Player[A].YoshiWingsFrameCount = Player[A].YoshiWingsFrameCount + 1;
            if(Player[A].YoshiWingsFrameCount < 12)
                Player[A].YoshiWingsFrame = 1;
            else if(Player[A].YoshiWingsFrameCount < 24)
                Player[A].YoshiWingsFrame = 0;
            else
            {
                Player[A].YoshiWingsFrameCount = 0;
                Player[A].YoshiWingsFrame = 0;
            }
        }
        if(Player[A].GroundPound == true)
            Player[A].YoshiWingsFrame = 0;
        if(Player[A].Direction == 1)
            Player[A].YoshiWingsFrame = Player[A].YoshiWingsFrame + 2;
    }
}

void UpdatePlayerBonus(int A, int B)
{
    // 1 player growing
    // 4 fire flower
    // 5 leaf
    if(!(Player[A].State == 1) || (Player[A].Effect == 1 || Player[A].Effect == 4 || Player[A].Effect == 5))
    {
        if(B == 9 || B == 185 || B == 184 || B == 250)
        {
            if(Player[A].HeldBonus == 0)
                Player[A].HeldBonus = B;
        }
        else if((Player[A].State == 2 || Player[A].Effect == 1) && !(Player[A].Effect == 4 || Player[A].Effect == 5))
        {
            if(Player[A].HeldBonus == 0)
            {
                if(Player[A].StateNPC == 184)
                    Player[A].HeldBonus = 184;
                else if(Player[A].StateNPC == 185)
                    Player[A].HeldBonus = 185;
                else
                    Player[A].HeldBonus = 9;
            }
        }
        else
        {
            if(Player[A].State == 3 || Player[A].Effect == 4)
            {
                if(Player[A].StateNPC == 183)
                    Player[A].HeldBonus = 183;
                else if(Player[A].StateNPC == 182)
                    Player[A].HeldBonus = 182;
                else
                    Player[A].HeldBonus = 14;
            }
            if(Player[A].State == 4 || Player[A].Effect == 5)
                Player[A].HeldBonus = 34;
            if(Player[A].State == 5 || Player[A].Effect == 11)
                Player[A].HeldBonus = 169;
            if(Player[A].State == 6 || Player[A].Effect == 12)
                Player[A].HeldBonus = 170;
            if(Player[A].State == 7 || Player[A].Effect == 41)
            {
                if(Player[A].StateNPC == 277)
                    Player[A].HeldBonus = 277;
                else
                    Player[A].HeldBonus = 264;
            }
        }
    }
    if(Player[A].Character == 3 || Player[A].Character == 4 || Player[A].Character == 5)
        Player[A].HeldBonus = 0;
}

void TailSwipe(int plr, bool boo, bool Stab, int StabDir)
{
    Location_t tailLoc;
    Location_t tempLoc;
    Location_t stabLoc;
    NPC_t oldNPC;
    int A = 0;
    long long B = 0;
    int C = 0;
    int fBlock = 0;
    int lBlock = 0;
    if(Stab == true)
    {
        if(Player[plr].Duck == false)
        {
            if(StabDir == 1)
            {
                tailLoc.Width = 6;
                tailLoc.Height = 14;
                tailLoc.Y = Player[plr].Location.Y - tailLoc.Height;
                if(Player[plr].Direction == 1)
                    tailLoc.X = Player[plr].Location.X + Player[plr].Location.Width - 4;
                else
                    tailLoc.X = Player[plr].Location.X - tailLoc.Width + 4;
            }
            else if(StabDir == 2)
            {
                // tailLoc.Width = 8
                tailLoc.Height = 8;
                if(Player[plr].Location.SpeedY >= 10)
                    tailLoc.Height = 12;
                else if(Player[plr].Location.SpeedY >= 8)
                    tailLoc.Height = 10;
                tailLoc.Y = Player[plr].Location.Y + Player[plr].Location.Height;
                // tailLoc.X = .Location.X + .Location.Width / 2 - tailLoc.Width / 2 + (2 * .Direction)
                tailLoc.Width = Player[plr].Location.Width - 2;
                tailLoc.X = Player[plr].Location.X + 1;
            }
            else
            {
                tailLoc.Width = 38;
                tailLoc.Height = 6;
                tailLoc.Y = Player[plr].Location.Y + Player[plr].Location.Height - 42;
                if(Player[plr].Direction == 1)
                    tailLoc.X = Player[plr].Location.X + Player[plr].Location.Width;
                else
                    tailLoc.X = Player[plr].Location.X - tailLoc.Width;
            }
        }
        else
        {
            tailLoc.Width = 38;
            tailLoc.Height = 8;
            tailLoc.Y = Player[plr].Location.Y + Player[plr].Location.Height - 22;
            if(Player[plr].Direction == 1)
                tailLoc.X = Player[plr].Location.X + Player[plr].Location.Width;
            else
                tailLoc.X = Player[plr].Location.X - tailLoc.Width;
        }
    }
    else
    {
        tailLoc.Width = 18;
        tailLoc.Height = 12;
        tailLoc.Y = Player[plr].Location.Y + Player[plr].Location.Height - 26;
        if(Player[plr].Direction == 1)
            tailLoc.X = Player[plr].Location.X + Player[plr].Location.Width;
        else
            tailLoc.X = Player[plr].Location.X - tailLoc.Width;
    }
    if(Player[plr].Character == 4) // move tail down for toad
        tailLoc.Y = tailLoc.Y + 4;
    if(boo == true) // the bool flag means hit a block
    {
        fBlock = FirstBlock[(tailLoc.X / 32) - 1];
        lBlock = LastBlock[((tailLoc.X + tailLoc.Width) / 32.0) + 1];
        for(A = fBlock; A <= lBlock; A++)
        {
            if(!BlockIsSizable[Block[A].Type] && Block[A].Hidden == false && (Block[A].Type != 293 || Stab == true) && Block[A].Invis == false && BlockNoClipping[Block[A].Type] == false)
            {
                if(CheckCollision(tailLoc, Block[A].Location) == true)
                {
                    if(Block[A].ShakeY == 0 && Block[A].ShakeY2 == 0 && Block[A].ShakeY3 == 0)
                    {
                        if(Block[A].Special > 0 || Block[A].Type == 55 || Block[A].Type == 159 || Block[A].Type == 90)
                            PlaySound(3);
//                        if(nPlay.Online == true && plr - 1 == nPlay.MySlot)
//                            Netplay::sendData Netplay::PutPlayerLoc(nPlay.MySlot) + "1g" + std::to_string(plr) + "|" + Player[plr].TailCount - 1;
                        UpdateGraphics(true);
                        if(StabDir == 2)
                        {
                            BlockHit(A, true, plr);
                        }
                        else
                        {
                            BlockHit(A, false, plr);
                        }
                        BlockHitHard(A);
                        if(Stab == false)
                        {
                            if(Block[A].ShakeY != 0)
                            {
                                tempLoc.X = (Block[A].Location.X + tailLoc.X + (Block[A].Location.Width + tailLoc.Width) / 2.0) / 2 - 16;
                                tempLoc.Y = (Block[A].Location.Y + tailLoc.Y + (Block[A].Location.Height + tailLoc.Height) / 2.0) / 2 - 16;
                                NewEffect(73, tempLoc);
                            }
                            break;
                        }
                        else
                        {
                            if(StabDir == 2)
                            {
                                if(Block[A].Type == 293 || Block[A].Type == 370 || Block[A].ShakeY != 0 || Block[A].ShakeY2 != 0 || Block[A].ShakeY3 != 0 || Block[A].Hidden == true || BlockHurts[Block[A].Type])
                                {
                                    if(BlockHurts[Block[A].Type])
                                        PlaySound(24);
                                    Player[plr].Location.Y = Player[plr].Location.Y - 0.1;
                                    Player[plr].Location.SpeedY = Physics.PlayerJumpVelocity;
                                    Player[plr].StandingOnNPC = 0;
                                    if(Player[plr].Controls.Jump == true || Player[plr].Controls.AltJump == true)
                                        Player[plr].Jump = 10;
                                }
                            }
                            if(Block[A].Type == 370)
                            {
                                PlaySound(88);
                                Block[A].Hidden = true;
                                Block[A].Layer = "Destroyed Blocks";
                                NewEffect(10, Block[A].Location);
                                Effect[numEffects].Location.SpeedY = -2;
                            }
                            if(Block[A].Type == 457 && Player[plr].State == 6)
                            {
                                KillBlock(A);
                            }

                        }
                    }
                }
            }
        }
    }
    for(int numNPCsMax5 = numNPCs, A = 1; A <= numNPCsMax5; A++)
    {
        if(NPC[A].Active == true && NPC[A].Effect == 0 && !(NPCIsAnExit[NPC[A].Type] || (NPCIsACoin[NPC[A].Type] && Stab == false)) && NPC[A].CantHurtPlayer != plr && !(Player[plr].StandingOnNPC == A && Player[plr].ShellSurf == true))
        {
            if(NPC[A].Type != 13 && NPC[A].Type != 265 && !(NPC[A].Type == 17 && NPC[A].Projectile == true) && NPC[A].Type != 108 && NPC[A].Type != 197 && NPC[A].Type != 192)
            {
                stabLoc = NPC[A].Location;
                if(NPCHeightGFX[NPC[A].Type] > NPC[A].Location.Height && NPC[A].Type != 8 && NPC[A].Type != 15 && NPC[A].Type != 205 && NPC[A].Type != 9 && NPC[A].Type != 51 && NPC[A].Type != 52 && NPC[A].Type != 74 && NPC[A].Type != 93 && NPC[A].Type != 245)
                {
                    stabLoc.Y = stabLoc.Y + stabLoc.Height;
                    stabLoc.Height = NPCHeightGFX[NPC[A].Type];
                    stabLoc.Y = stabLoc.Y - stabLoc.Height;
                }
                if(NPC[A].Type == 91 && Stab == true)
                    stabLoc.Y = stabLoc.Y - stabLoc.Height;
                if(CheckCollision(tailLoc, stabLoc) == true && NPC[A].Killed == 0 && NPC[A].TailCD == 0 && !(StabDir != 0 && NPC[A].Type == 91))
                {
                    oldNPC = NPC[A];
                    if(Stab == true)
                    {
                        B = NPC[A].Damage;
                        C = NPC[A].Type;
                        if(StabDir == 2 && ((NPC[A].Type >= 154 && NPC[A].Type <= 157) || NPC[A].Type == 26 || NPC[A].Type == 32 || NPC[A].Type == 238 || NPC[A].Type == 241))
                        {
                        }
                        else
                        {
                            if(NPC[A].Type == 45 && StabDir != 0)
                            {
                                NPC[A].Special = 1;
                                NPC[A].Projectile = true;
                                NPCHit(A, 3, A);
                                Player[plr].Location.SpeedY = Physics.PlayerJumpVelocity;
                                Player[plr].StandingOnNPC = 0;
                                if(Player[plr].Controls.Jump == true || Player[plr].Controls.AltJump == true)
                                    Player[plr].Jump = 10;
                            }
                            else
                            {
                                NPCHit(A, 10, plr);
                            }
                            if(StabDir == 2 && (NPC[A].Killed == 10 || NPC[A].Damage != B || NPC[A].Type != C))
                            {
                                Player[plr].Location.SpeedY = Physics.PlayerJumpVelocity;
                                Player[plr].StandingOnNPC = 0;
                                if(Player[plr].Controls.Jump == true || Player[plr].Controls.AltJump == true)
                                    Player[plr].Jump = 10;
                            }
                        }
                    }
                    else
                    {
                        NPCHit( A, 7, plr);
                        if(NPC[A].Killed > 0 || NPC[A].Type != oldNPC.Type || NPC[A].Projectile != oldNPC.Projectile || (NPC[A].Location.SpeedY != oldNPC.Location.SpeedY))
                        {
//                            if(nPlay.Online == true && plr - 1 == nPlay.MySlot)
//                                Netplay::sendData Netplay::PutPlayerLoc(nPlay.MySlot) + "1g" + std::to_string(plr) + "|" + Player[plr].TailCount - 1;
                            tempLoc.X = (NPC[A].Location.X + tailLoc.X + (NPC[A].Location.Width + tailLoc.Width) / 2.0) / 2 - 16;
                            tempLoc.Y = (NPC[A].Location.Y + tailLoc.Y + (NPC[A].Location.Height + tailLoc.Height) / 2.0) / 2 - 16;
                            NPC[A].BattleOwner = plr;
                            NewEffect(73, tempLoc);
                        }
                    }
                    NPC[A].TailCD = 8;
                }
            }
        }
    }

    if(BattleMode == true)
    {
        for(A = 1; A <= numPlayers; A++)
        {
            if(A != plr)
            {
                stabLoc = Player[A].Location;
                if(CheckCollision(tailLoc, stabLoc) == true && Player[A].Effect == 0 && Player[A].Immune == 0 && Player[A].Dead == false && Player[A].TimeToLive == 0)
                {
                    if(Stab == true)
                    {
                        if(StabDir == 2)
                        {
                            Player[plr].Location.SpeedY = Physics.PlayerJumpVelocity;
                            Player[plr].StandingOnNPC = 0;
                            if(Player[plr].Controls.Jump == true || Player[plr].Controls.AltJump == true)
                                Player[plr].Jump = 10;
                        }
                        PlayerHurt(A);
                        PlaySound(89);
                    }
                    else
                    {
                        Player[A].Location.SpeedX = 6 * Player[plr].Direction;
                        Player[A].Location.SpeedY = -5;
                        PlaySound(2);
                    }
                }
            }
        }
    }

    if(Stab == false)
    {
        if(((Player[plr].TailCount) % 10 == 0 && Player[plr].SpinJump == false) || ((Player[plr].TailCount) % 5 == 0 && Player[plr].SpinJump == true))
        {
            NewEffect (80, newLoc(tailLoc.X + (dRand() * tailLoc.Width) - 4, tailLoc.Y + (dRand() * tailLoc.Height)), 1, 0, ShadowMode);
            Effect[numEffects].Location.SpeedX = (0.5 + dRand() * 1) * Player[plr].Direction;
            Effect[numEffects].Location.SpeedY = dRand() * 1 - 0.5;
        }
    }
}

void YoshiHeight(int A)
{
    if(Player[A].Mount == 3)
    {
        Player[A].Location.Y = Player[A].Location.Y + Player[A].Location.Height;
        if(Player[A].State == 1)
            Player[A].Location.Height = 54;
        else
            Player[A].Location.Height = 60;
        Player[A].Location.Y = Player[A].Location.Y - Player[A].Location.Height;
    }
}

void YoshiEat(int A)
{
    int B = 0;
    Location_t tempLocation;
    for(B = 1; B <= numPlayers; B++)
    {
        if(B != A && Player[B].Effect == 0 && Player[B].Dead == false && Player[B].TimeToLive == 0 && Player[B].Mount == 0)
        {
            if(CheckCollision(Player[A].YoshiTongue, Player[B].Location) == true)
            {
                Player[A].YoshiPlayer = B;
                Player[B].HoldingNPC = 0;
                return;
            }
        }
    }
    for(int numNPCsMax6 = numNPCs, B = 1; B <= numNPCsMax6; B++)
    {
        if(((NPCIsACoin[NPC[B].Type] && NPC[B].Special == 1) || NPCNoYoshi[NPC[B].Type] == false) && NPC[B].Active == true && ((NPCIsACoin[NPC[B].Type] == false || NPC[B].Special == 1) || NPC[B].Type == 103) && NPCIsAnExit[NPC[B].Type] == false && NPC[B].Generator == false && NPC[B].Inert == false && NPCIsYoshi[NPC[B].Type] == false && NPC[B].Effect != 5 && NPC[B].Immune == 0 && NPC[B].Type != 91 && !(NPC[B].Projectile == true && NPC[B].Type == 17) && NPC[B].HoldingPlayer == 0)
        {
            tempLocation = NPC[B].Location;
            if(NPC[B].Type == 91)
                tempLocation.Y = NPC[B].Location.Y - 16;
            if(CheckCollision(Player[A].YoshiTongue, tempLocation))
            {
                if(NPC[B].Type == 91)
                {
                    if(NPCNoYoshi[NPC[B].Special] == false)
                    {
                        PlaySound(23);
                        NPC[B].Generator = false;
                        NPC[B].Frame = 0;
                        NPC[B].Type = NPC[B].Special;
                        NPC[B].Special = 0;
                        if(NPCIsYoshi[NPC[B].Type])
                        {
                            NPC[B].Special = NPC[B].Type;
                            NPC[B].Type = 96;
                        }
                        NPC[B].Location.Height = NPCHeight[NPC[B].Type];
                        NPC[B].Location.Width = NPCWidth[NPC[B].Type];
                        if(!(NPC[B].Type == 21 || NPC[B].Type == 22 || NPC[B].Type == 26 || NPC[B].Type == 31 || NPC[B].Type == 32 || NPC[B].Type == 35 || NPC[B].Type == 49 || NPCIsAnExit[NPC[B].Type]))
                            NPC[B].DefaultType = 0;
                        NPC[B].Effect = 5;
                        NPC[B].Effect2 = A;
                        Player[A].YoshiNPC = B;
                    }
                }
                else if(NPC[B].Type == 283)
                {
                    NPCHit(B, 3, B);
                }
                else
                {
                    NPC[B].Effect = 5;
                    NPC[B].Effect2 = A;
                    NPC[B].Location.Height = NPCHeight[NPC[B].Type];
                    Player[A].YoshiNPC = B;
                }
                if(NPC[B].Type == 147)
                {
                    NPC[B].Type = 139 + (iRand() % 9);
                    if(NPC[B].Type == 147)
                        NPC[B].Type = 92;
                    NPC[B].Location.X = NPC[B].Location.X + NPC[B].Location.Width / 2.0;
                    NPC[B].Location.Y = NPC[B].Location.Y + NPC[B].Location.Height / 2.0;
                    NPC[B].Location.Width = NPCWidth[NPC[B].Type];
                    NPC[B].Location.Height = NPCHeight[NPC[B].Type];
                    NPC[B].Location.X = NPC[B].Location.X - NPC[B].Location.Width / 2.0;
                    NPC[B].Location.Y = NPC[B].Location.Y - NPC[B].Location.Height / 2.0;
                }
                break;
            }
        }
    }
}

void YoshiSpit(int A)
{
    int B = 0;
//    if(nPlay.Online == true && A - 1 == nPlay.MySlot)
//        Netplay::sendData Netplay::PutPlayerControls(nPlay.MySlot);
    Player[A].YoshiTFrameCount = -1;
    if(Player[A].YoshiPlayer > 0)
    {
        Player[Player[A].YoshiPlayer].Section = Player[A].Section;
        Player[Player[A].YoshiPlayer].Effect = 0;
        Player[Player[A].YoshiPlayer].Effect2 = 0;
        Player[Player[A].YoshiPlayer].Slide = true;
        if(Player[Player[A].YoshiPlayer].State > 1)
            Player[Player[A].YoshiPlayer].Location.Height = Physics.PlayerDuckHeight[Player[Player[A].YoshiPlayer].Character][Player[Player[A].YoshiPlayer].State];
            // Player(.YoshiPlayer).Location.Y = Player(.YoshiPlayer).Location.Y - Physics.PlayerDuckHeight(Player(.YoshiPlayer).Character, Player(.YoshiPlayer).State) + Physics.PlayerHeight(Player(.YoshiPlayer).Character, Player(.YoshiPlayer).State)
            // Player(.YoshiPlayer).Duck = True
        if(Player[A].Controls.Down == true)
        {
            Player[Player[A].YoshiPlayer].Location.X = Player[A].Location.X + Player[A].YoshiTX + Player[Player[A].YoshiPlayer].Location.Width * Player[A].Direction;
            Player[Player[A].YoshiPlayer].Location.X = Player[Player[A].YoshiPlayer].Location.X + 5;
            Player[Player[A].YoshiPlayer].Location.Y = Player[A].Location.Y + Player[A].Location.Height - Player[Player[A].YoshiPlayer].Location.Height;
            Player[Player[A].YoshiPlayer].Location.SpeedX = 0 + Player[A].Location.SpeedX * 0.3;
            Player[Player[A].YoshiPlayer].Location.SpeedY = 1 + Player[A].Location.SpeedY * 0.3;
        }
        else
        {
            Player[Player[A].YoshiPlayer].Location.X = Player[A].Location.X + Player[A].YoshiTX + Player[Player[A].YoshiPlayer].Location.Width * Player[A].Direction;
            Player[Player[A].YoshiPlayer].Location.X = Player[Player[A].YoshiPlayer].Location.X + 5;
            Player[Player[A].YoshiPlayer].Location.Y = Player[A].Location.Y + 1;
            Player[Player[A].YoshiPlayer].Location.SpeedX = 7 * Player[A].Direction + Player[A].Location.SpeedX * 0.3;
            Player[Player[A].YoshiPlayer].Location.SpeedY = -3 + Player[A].Location.SpeedY * 0.3;
        }
        Player[Player[A].YoshiPlayer].Direction = -Player[A].Direction;
        Player[Player[A].YoshiPlayer].Bumped = true;
        if(Player[A].Direction == 1)
        {
            PlayerPush(Player[A].YoshiPlayer, 2);
        }
        else
        {
            PlayerPush(Player[A].YoshiPlayer, 4);
        }
        Player[A].YoshiPlayer = 0;
        PlaySound(38);
    }
    else
    {
        NPC[Player[A].YoshiNPC].RealSpeedX = 0;
        if(NPCIsAShell[NPC[Player[A].YoshiNPC].Type])
            NPC[Player[A].YoshiNPC].Special = 0;
        if((NPCIsAShell[NPC[Player[A].YoshiNPC].Type] || NPCIsABot[NPC[Player[A].YoshiNPC].Type] || NPC[Player[A].YoshiNPC].Type == 194) && !(NPC[Player[A].YoshiNPC].Type == 24) && Player[A].YoshiRed == true)
        {
            NPC[Player[A].YoshiNPC].Killed = 9;
            PlaySound(42);
            for(B = 1; B <= 3; B++)
            {
                numNPCs++;
                NPC[numNPCs] = NPC_t();
                NPC[numNPCs].Direction = Player[A].Direction;
                NPC[numNPCs].Type = 108;
                NPC[numNPCs].Frame = EditorNPCFrame(NPC[numNPCs].Type, NPC[numNPCs].Direction);
                NPC[numNPCs].Active = true;
                NPC[numNPCs].Section = Player[A].Section;
                NPC[numNPCs].TimeLeft = 100;
                NPC[numNPCs].Effect = 0;
                NPC[numNPCs].Location.X = Player[A].Location.X + Player[A].YoshiTX + 32 * Player[A].Direction;
                NPC[numNPCs].Location.Y = Player[A].Location.Y + Player[A].YoshiTY;
                NPC[numNPCs].Location.Width = 32;
                NPC[numNPCs].Location.Height = 32;
                if(B == 1)
                {
                    NPC[numNPCs].Location.SpeedY = -0.8;
                    NPC[numNPCs].Location.SpeedX = 5 * Player[A].Direction;
                }
                else if(B == 2)
                {
                    NPC[numNPCs].Location.SpeedY = 0;
                    NPC[numNPCs].Location.SpeedX = 5.5 * Player[A].Direction;
                }
                else
                {
                    NPC[numNPCs].Location.SpeedY = 0.8;
                    NPC[numNPCs].Location.SpeedX = 5 * Player[A].Direction;
                }
            }
        }
        else
        {
            NPC[Player[A].YoshiNPC].Direction = Player[A].Direction;
            NPC[Player[A].YoshiNPC].Frame = 0;
            NPC[Player[A].YoshiNPC].WallDeath = 5;
            NPC[Player[A].YoshiNPC].FrameCount = 0;
            NPC[Player[A].YoshiNPC].Frame = EditorNPCFrame(NPC[Player[A].YoshiNPC].Type, NPC[Player[A].YoshiNPC].Direction);
            NPC[Player[A].YoshiNPC].Active = true;
            NPC[Player[A].YoshiNPC].Section = Player[A].Section;
            NPC[Player[A].YoshiNPC].TimeLeft = 100;
            NPC[Player[A].YoshiNPC].Effect = 0;
            NPC[Player[A].YoshiNPC].Effect2 = 0;
            NPC[Player[A].YoshiNPC].Location.X = Player[A].Location.X + Player[A].YoshiTX + 32 * Player[A].Direction;
            NPC[Player[A].YoshiNPC].Location.Y = Player[A].Location.Y + Player[A].YoshiTY;
            if(Player[A].Duck == true)
                NPC[Player[A].YoshiNPC].Location.Y = NPC[Player[A].YoshiNPC].Location.Y - 8;
            NPC[Player[A].YoshiNPC].Location.Y = NPC[Player[A].YoshiNPC].Location.Y - 2;
            NPC[Player[A].YoshiNPC].Location.SpeedX = 0;
            NPC[Player[A].YoshiNPC].Location.SpeedY = 0;



            if(NPC[Player[A].YoshiNPC].Type == 45)
                NPC[Player[A].YoshiNPC].Special = 1;
            PlaySound(38);
            if(Player[A].Controls.Down == false || (Player[A].Location.SpeedY != 0 && Player[A].StandingOnNPC == 0 && Player[A].Slope == 0))
            {
                if(NPCIsAShell[NPC[Player[A].YoshiNPC].Type])
                {
                    SoundPause[9] = 2;
                    // NPCHit .YoshiNPC, 1, A
                    NPC[Player[A].YoshiNPC].Location.SpeedX = Physics.NPCShellSpeed * Player[A].Direction;
                }
                else if(NPC[Player[A].YoshiNPC].Type == 45)
                    NPC[Player[A].YoshiNPC].Location.SpeedX = Physics.NPCShellSpeed * Player[A].Direction;
                else
                {
                    NPC[Player[A].YoshiNPC].Projectile = true;
                    NPC[Player[A].YoshiNPC].Location.SpeedX = 7 * Player[A].Direction;
                    NPC[Player[A].YoshiNPC].Location.SpeedY = -1.3;
                }
            }
            if(NPC[Player[A].YoshiNPC].Type == 237)
            {
                NPC[Player[A].YoshiNPC].Direction = Player[A].Direction;
                NPC[Player[A].YoshiNPC].Projectile = true;
                NPC[Player[A].YoshiNPC].Location.SpeedX = Physics.NPCShellSpeed * Player[A].Direction * 0.6 + Player[A].Location.SpeedX * 0.4;
                NPC[Player[A].YoshiNPC].TurnAround = false;
            }
        }
    }
    Player[A].FireBallCD = 20;
    Player[A].YoshiNPC = 0;
    Player[A].YoshiRed = false;
    if(Player[A].YoshiBlue == true)
    {
        Player[A].CanFly = false;
        Player[A].CanFly2 = false;
    }
    Player[A].YoshiBlue = false;
    Player[A].YoshiYellow = false;
}

void YoshiPound(int A, int /*C*/, bool BreakBlocks)
{
    int B = 0;
    Location_t tempLocation;
    Location_t tempLocation2;
    if(Player[A].Location.SpeedY > 3)
    {

        tempLocation.Width = 128;
        tempLocation.X = Player[A].Location.X + Player[A].Location.Width / 2.0 - tempLocation.Width / 2.0;
        tempLocation.Height = 32;
        tempLocation.Y = Player[A].Location.Y + Player[A].Location.Height - 16;

        for(int numNPCsMax7 = numNPCs, B = 1; B <= numNPCsMax7; B++)
        {
            if(NPC[B].Hidden == false && NPC[B].Active == true && NPC[B].Effect == 0)
            {
                tempLocation2 = NPC[B].Location;
                tempLocation2.Y = tempLocation2.Y + tempLocation2.Height - 4;
                tempLocation2.Height = 8;
                if(CheckCollision(tempLocation, tempLocation2) == true)
                {
                    Block[0].Location.Y = NPC[B].Location.Y + NPC[B].Location.Height;
                    NPCHit(B, 2, 0);
                }
            }
        }


        if(BreakBlocks == true)
        {
            for(B = 1; B <= numBlock; B++)
            {
                if(Block[B].Hidden == false && Block[B].Invis == false && BlockNoClipping[Block[B].Type] == false && BlockIsSizable[Block[B].Type] == false)
                {
                    if(CheckCollision(Player[A].Location, Block[B].Location) == true)
                    {
                        BlockHit(B, true, A);
                        BlockHitHard(B);
                    }
                }
            }
        }
        tempLocation.Width = 32;
        tempLocation.Height = 32;
        tempLocation.Y = Player[A].Location.Y + Player[A].Location.Height - 16;
        tempLocation.X = Player[A].Location.X + Player[A].Location.Width / 2.0 - 16 - 16;
        NewEffect(10, tempLocation);
        Effect[numEffects].Location.SpeedX = -2;
        tempLocation.X = Player[A].Location.X + Player[A].Location.Width / 2.0 - 16 + 16;
        NewEffect(10, tempLocation);
        Effect[numEffects].Location.SpeedX = 2;
        PlaySound(37);
    }
}

void SwapCoop()
{
    if(SingleCoop == 1)
    {
        if(Player[2].Dead == true || Player[2].TimeToLive > 0)
            return;
        SingleCoop = 2;
    }
    else
    {
        if(Player[1].Dead == true || Player[1].TimeToLive > 0)
            return;
        SingleCoop = 1;
    }
    Player[1].DropRelease = false;
    Player[1].Controls.Drop = true;
    Player[2].DropRelease = false;
    Player[2].Controls.Drop = true;
    PlaySound(13);

    Player[SingleCoop].Immune = 50;
    if(curMusic >= 0 && curMusic != bgMusic[Player[SingleCoop].Section])
    {
        StopMusic();
        StartMusic(Player[SingleCoop].Section);
    }
}

void PlayerPush(int A, int HitSpot)
{
    Location_t tempLocation;
    double fBlock = 0;
    double lBlock = 0;
    int B = 0;
    if(ShadowMode == true)
        return;
    fBlock = FirstBlock[(Player[A].Location.X / 32) - 1];
    lBlock = LastBlock[((Player[A].Location.X + Player[A].Location.Width) / 32.0) + 1];
    for(B = int(fBlock); B <= lBlock; B++)
    {
        if(Block[B].Hidden == false)
        {
            if(BlockIsSizable[Block[B].Type] == false)
            {
                if(BlockSlope[Block[B].Type] == 0 && BlockSlope2[Block[B].Type] == 0)
                {
                    tempLocation = Player[A].Location;
                    tempLocation.Height = tempLocation.Height - 1;
                    if(CheckCollision(tempLocation, Block[B].Location) == true)
                    {
                        if(BlockOnlyHitspot1[Block[B].Type] == false)
                        {
                            if(BlockNoClipping[Block[B].Type] == false)
                            {
                                if(HitSpot == 2)
                                    Player[A].Location.X = Block[B].Location.X - Player[A].Location.Height - 0.01;
                                else if(HitSpot == 3)
                                    Player[A].Location.Y = Block[B].Location.Y + Block[B].Location.Height + 0.01;
                                else if(HitSpot == 4)
                                    Player[A].Location.X = Block[B].Location.X + Block[B].Location.Width + 0.01;
                            }
                        }
                    }
                }
            }
        }
    }
}

void SizeCheck(int A)
{
//On Error Resume Next

// player size fix
// height
    if(Player[A].State == 0)
        Player[A].State = 1;
    if(Player[A].Character == 0)
        Player[A].Character = 1;
    if(Player[A].Fairy == true)
    {
        if(Player[A].Duck == true)
        {
            UnDuck(A);
        }
        if(Player[A].Location.Width != 22.0)
        {
            Player[A].Location.X = Player[A].Location.X + Player[A].Location.Width / 2.0 - 11;
            Player[A].Location.Width = 22;
        }
        if(Player[A].Location.Height != 26.0)
        {
            Player[A].Location.Y = Player[A].Location.Y + Player[A].Location.Height - 26;
            Player[A].Location.Height = 26;
        }
    }
    else if(Player[A].Mount == 0)
    {
        if(Player[A].Duck == false)
        {
            if(Player[A].Location.Height != Physics.PlayerHeight[Player[A].Character][Player[A].State])
            {
                Player[A].Location.Y = Player[A].Location.Y + Player[A].Location.Height;
                Player[A].Location.Height = Physics.PlayerHeight[Player[A].Character][Player[A].State];
                Player[A].Location.Y = Player[A].Location.Y - Player[A].Location.Height;
            }
        }
        else
        {
            if(Player[A].Location.Height != Physics.PlayerDuckHeight[Player[A].Character][Player[A].State])
            {
                Player[A].Location.Y = Player[A].Location.Y + Player[A].Location.Height;
                Player[A].Location.Height = Physics.PlayerDuckHeight[Player[A].Character][Player[A].State];
                Player[A].Location.Y = Player[A].Location.Y - Player[A].Location.Height;
            }
        }
    }
    else if(Player[A].Mount == 1)
    {
        if(Player[A].Duck == true)
        {
            if(Player[A].Location.Height != Physics.PlayerDuckHeight[Player[A].Character][2])
            {
                Player[A].Location.Y = Player[A].Location.Y + Player[A].Location.Height;
                Player[A].Location.Height = Physics.PlayerDuckHeight[Player[A].Character][2];
                Player[A].Location.Y = Player[A].Location.Y - Player[A].Location.Height;
            }
        }
        else if(Player[A].Character == 2 && Player[A].State > 1)
        {
            if(Player[A].Location.Height != Physics.PlayerHeight[1][2])
            {
                Player[A].Location.Y = Player[A].Location.Y + Player[A].Location.Height;
                Player[A].Location.Height = Physics.PlayerHeight[Player[A].Character][Player[A].State];
                Player[A].Location.Y = Player[A].Location.Y - Player[A].Location.Height;
            }
        }
        else
        {
            if(Player[A].Location.Height != Physics.PlayerHeight[1][2])
            {
                Player[A].Location.Y = Player[A].Location.Y + Player[A].Location.Height;
                Player[A].Location.Height = Physics.PlayerHeight[1][2];
                Player[A].Location.Y = Player[A].Location.Y - Player[A].Location.Height;
            }
        }
    }
    else if(Player[A].Mount == 2)
    {
        if(Player[A].Location.Height != 128)
        {
            Player[A].Location.Y = Player[A].Location.Y + Player[A].Location.Height;
            Player[A].Location.Height = 128;
            Player[A].Location.Y = Player[A].Location.Y - Player[A].Location.Height;
        }
    }
    else if(Player[A].Mount == 3)
    {
        if(Player[A].Duck == false)
        {
            if(Player[A].State == 1)
            {
                if(Player[A].Location.Height != Physics.PlayerHeight[1][2])
                {
                    Player[A].Location.Y = Player[A].Location.Y + Player[A].Location.Height;
                    Player[A].Location.Height = Physics.PlayerHeight[1][2];
                    Player[A].Location.Y = Player[A].Location.Y - Player[A].Location.Height;
                }
            }
            else
            {
                if(Player[A].Location.Height != Physics.PlayerHeight[2][2])
                {
                    Player[A].Location.Y = Player[A].Location.Y + Player[A].Location.Height;
                    Player[A].Location.Height = Physics.PlayerHeight[2][2];
                    Player[A].Location.Y = Player[A].Location.Y - Player[A].Location.Height;
                }
            }
        }
        else
        {
            if(Player[A].Location.Height != 31)
            {
                Player[A].Location.Y = Player[A].Location.Y + Player[A].Location.Height;
                Player[A].Location.Height = 31;
                Player[A].Location.Y = Player[A].Location.Y - Player[A].Location.Height;
            }
        }
    }
// width
    if(Player[A].Mount == 2)
    {
        if(Player[A].Location.Width != 127.9)
        {
            Player[A].Location.X = Player[A].Location.X + Player[A].Location.Width / 2.0;
            Player[A].Location.Width = 127.9;
            Player[A].Location.X = Player[A].Location.X - Player[A].Location.Width / 2.0;
        }
    }
    else
    {
        if(Player[A].Location.Width != Physics.PlayerWidth[Player[A].Character][Player[A].State])
        {
            Player[A].Location.X = Player[A].Location.X + Player[A].Location.Width / 2.0;
            Player[A].Location.Width = Physics.PlayerWidth[Player[A].Character][Player[A].State];
            Player[A].Location.X = Player[A].Location.X - Player[A].Location.Width / 2.0;
        }
    }
}

void YoshiEatCode(int A)
{
    int B = 0;
    Location_t tempLocation;

    if(Player[A].Mount == 3 && Player[A].Fairy == false)
    {
    // Shell Colors
        if(Player[A].MountSpecial == 0)
        {
            if(Player[A].YoshiNPC > 0)
            {
                if(NPC[Player[A].YoshiNPC].Type == 7 || NPC[Player[A].YoshiNPC].Type == 6)
                    Player[A].YoshiRed = true;
                if(NPC[Player[A].YoshiNPC].Type == 110 || NPC[Player[A].YoshiNPC].Type == 114 || NPC[Player[A].YoshiNPC].Type == 128)
                    Player[A].YoshiRed = true;
                if(NPC[Player[A].YoshiNPC].Type == 110 || NPC[Player[A].YoshiNPC].Type == 114 || NPC[Player[A].YoshiNPC].Type == 128)
                    Player[A].YoshiRed = true;
                if(NPC[Player[A].YoshiNPC].Type == 174 || NPC[Player[A].YoshiNPC].Type == 175 || NPC[Player[A].YoshiNPC].Type == 177 || NPC[Player[A].YoshiNPC].Type == 194)
                    Player[A].YoshiRed = true;
                if(NPC[Player[A].YoshiNPC].Type == 111 || NPC[Player[A].YoshiNPC].Type == 115 || NPC[Player[A].YoshiNPC].Type == 194 || NPC[Player[A].YoshiNPC].Type == 195)
                {
                    if(Player[A].YoshiBlue == false)
                    {
                        Player[A].CanFly2 = true;
                        Player[A].CanFly2 = true;
                        Player[A].FlyCount = 300;
                    }
                    Player[A].YoshiBlue = true;
                }
                if(NPC[Player[A].YoshiNPC].Type == 112 || NPC[Player[A].YoshiNPC].Type == 116 || NPC[Player[A].YoshiNPC].Type == 127 || NPC[Player[A].YoshiNPC].Type == 194 || NPC[Player[A].YoshiNPC].Type == 195)
                    Player[A].YoshiYellow = true;
            }
            if(Player[A].YoshiNPC > 0 || Player[A].YoshiPlayer > 0)
            {
                if(Player[A].MountType == 2 || Player[A].MountType == 5)
                {
                    if(Player[A].YoshiBlue == false)
                    {
                        Player[A].CanFly2 = true;
                        Player[A].CanFly2 = true;
                        Player[A].FlyCount = 300;
                    }
                    Player[A].YoshiBlue = true;
                }
                if(Player[A].MountType == 3 || Player[A].MountType == 5)
                    Player[A].YoshiYellow = true;
                if(Player[A].MountType == 4 || Player[A].MountType == 5)
                    Player[A].YoshiRed = 4;
            }
        }

        if(Player[A].YoshiBlue == true)
        {
            Player[A].CanFly = true;
            Player[A].RunCount = 1000;
        }
        if(Player[A].MountSpecial == 0)
        {
            if(NPC[Player[A].YoshiNPC].Type == 31) // key check
            {
                for(B = 1; B <= numBackground; B++)
                {
                    if(Background[B].Type == 35)
                    {
                        tempLocation = Background[B].Location;
                        tempLocation.Width = 16;
                        tempLocation.X = tempLocation.X + 8;
                        tempLocation.Height = 26;
                        tempLocation.Y = tempLocation.Y + 2;
                        if(CheckCollision(Player[A].Location, tempLocation) == true)
                        {
                            PlaySound(31);
                            StopMusic();
                            LevelMacro = 3;
                            break;
                        }
                    }
                }
            }
            else if(NPC[Player[A].YoshiNPC].Type == 45)
                NPC[Player[A].YoshiNPC].Special = 1;
            if(Player[A].FireBallCD > 0)
                Player[A].FireBallCD = Player[A].FireBallCD - 1;
            if(Player[A].Controls.Run == true)
            {
                if(Player[A].RunRelease == true)
                {
                    if(Player[A].YoshiNPC == 0 && Player[A].YoshiPlayer == 0)
                    {
                        if(Player[A].FireBallCD == 0)
                        {
                            Player[A].MountSpecial = 1;
                            Player[A].YoshiTongueLength = 0;
                            Player[A].YoshiTonugeBool = false;
                            PlaySound(50);
                        }
                    }
                    else
                    {
                        YoshiSpit(A);
                    }
                }
            }
        }
        if(Player[A].MountSpecial != 0)
        {
            Player[A].YoshiTFrameCount = 0;
            if(Player[A].YoshiNPC > 0 || Player[A].YoshiPlayer > 0)
                Player[A].YoshiTonugeBool = true;
            if(Player[A].YoshiTonugeBool == false)
            {
                if(Player[A].MountType <= 4)
                {
                    if(Player[A].YoshiTongueLength < 64 * 0.7)
                        Player[A].YoshiTongueLength = Player[A].YoshiTongueLength + 6;
                    else
                        Player[A].YoshiTongueLength = Player[A].YoshiTongueLength + 3;
                }
                else
                {
                    if(Player[A].YoshiTongueLength < 80 * 0.7)
                        Player[A].YoshiTongueLength = Player[A].YoshiTongueLength + 7.5;
                    else
                        Player[A].YoshiTongueLength = Player[A].YoshiTongueLength + 3.75;
                }

                if(Player[A].YoshiTongueLength >= 64 && Player[A].MountType <= 4)
                    Player[A].YoshiTonugeBool = true;
                else if(Player[A].YoshiTongueLength >= 80)
                    Player[A].YoshiTonugeBool = true;
            }
            else
            {
                if(Player[A].MountType <= 4)
                {
                    if(Player[A].YoshiTongueLength < 64 * 0.7)
                        Player[A].YoshiTongueLength = Player[A].YoshiTongueLength - 6;
                    else
                        Player[A].YoshiTongueLength = Player[A].YoshiTongueLength - 3;
                }
                else
                {
                    if(Player[A].YoshiTongueLength < 80 * 0.7)
                        Player[A].YoshiTongueLength = Player[A].YoshiTongueLength - 7.5;
                    else
                        Player[A].YoshiTongueLength = Player[A].YoshiTongueLength - 3.75;
                }
                if(Player[A].YoshiTongueLength <= -8)
                {
                    Player[A].YoshiTongueLength = 0;
                    Player[A].YoshiTonugeBool = false;
                    Player[A].MountSpecial = 0;
                }
            }
            Player[A].YoshiTongue.Height = 12;
            Player[A].YoshiTongue.Width = 16;
            Player[A].YoshiTongueX = Player[A].Location.X + Player[A].Location.Width / 2.0;
            if(Player[A].Controls.Up == true || (Player[A].StandingOnNPC == 0 && Player[A].Slope == 0 && Player[A].Location.SpeedY != 0 && Player[A].Controls.Down == false))
            {
                Player[A].YoshiTongueX = Player[A].YoshiTongueX + Player[A].Direction * (22);
                Player[A].YoshiTongue.Y = Player[A].Location.Y + 8 + (Player[A].Location.Height - 54);
                Player[A].YoshiTongue.X = Player[A].YoshiTongueX + Player[A].YoshiTongueLength * Player[A].Direction;
            }
            else
            {
                Player[A].YoshiTongueX = Player[A].YoshiTongueX + Player[A].Direction * (34);
                Player[A].YoshiTongue.Y = Player[A].Location.Y + 30 + (Player[A].Location.Height - 54);
                Player[A].YoshiTongue.X = Player[A].YoshiTongueX + Player[A].YoshiTongueLength * Player[A].Direction;
            }
            if(Player[A].Direction == -1)
                Player[A].YoshiTongue.X = Player[A].YoshiTongue.X - 16;
            if(Player[A].YoshiNPC == 0 && Player[A].YoshiPlayer == 0)
            {
                YoshiEat(A);
            }
            if(Player[A].YoshiNPC > 0)
            {
                NPC[Player[A].YoshiNPC].Effect2 = A;
                NPC[Player[A].YoshiNPC].Effect3 = 5;
                if(Player[A].YoshiTonugeBool == false)
                    Player[A].YoshiTonugeBool = true;
                NPC[Player[A].YoshiNPC].Location.X = Player[A].YoshiTongue.X - NPC[Player[A].YoshiNPC].Location.Width / 2.0 + 8 + 4 * Player[A].Direction;
                NPC[Player[A].YoshiNPC].Location.Y = Player[A].YoshiTongue.Y - NPC[Player[A].YoshiNPC].Location.Height / 2.0 + 6;
            }
            if(Player[A].YoshiPlayer > 0)
            {
                Player[Player[A].YoshiPlayer].Effect = 9;
                Player[Player[A].YoshiPlayer].Effect2 = A;
                Player[Player[A].YoshiPlayer].Location.X = Player[A].YoshiTongue.X + Player[A].YoshiTongue.Width / 2.0 - Player[Player[A].YoshiPlayer].Location.Width / 2.0;
                Player[Player[A].YoshiPlayer].Location.Y = Player[A].YoshiTongue.Y + Player[A].YoshiTongue.Height / 2.0 - Player[Player[A].YoshiPlayer].Location.Height / 2.0;
                if(Player[Player[A].YoshiPlayer].Location.Y + Player[Player[A].YoshiPlayer].Location.Height > Player[A].Location.Y + Player[A].Location.Height)
                    Player[Player[A].YoshiPlayer].Location.Y = Player[A].Location.Y + Player[A].Location.Height - Player[Player[A].YoshiPlayer].Location.Height;
            }
        }
        if(Player[A].MountSpecial == 0 && Player[A].YoshiNPC > 0)
        {
            Player[A].YoshiTFrameCount = 1;
            if(NPC[Player[A].YoshiNPC].Type == 4 || NPC[Player[A].YoshiNPC].Type == 76)
                NPC[Player[A].YoshiNPC].Type = 5;
            else if(NPC[Player[A].YoshiNPC].Type == 6 || NPC[Player[A].YoshiNPC].Type == 161)
                NPC[Player[A].YoshiNPC].Type = 7;
            else if(NPC[Player[A].YoshiNPC].Type == 23)
                NPC[Player[A].YoshiNPC].Type = 24;
            else if(NPC[Player[A].YoshiNPC].Type == 72)
                NPC[Player[A].YoshiNPC].Type = 73;
            else if(NPC[Player[A].YoshiNPC].Type >= 109 && NPC[Player[A].YoshiNPC].Type <= 112)
                NPC[Player[A].YoshiNPC].Type = NPC[Player[A].YoshiNPC].Type + 4;
            else if(NPC[Player[A].YoshiNPC].Type >= 121 && NPC[Player[A].YoshiNPC].Type <= 124)
            {
                NPC[Player[A].YoshiNPC].Type = NPC[Player[A].YoshiNPC].Type - 8;
                NPC[Player[A].YoshiNPC].Special = 0;
            }
            else if(NPC[Player[A].YoshiNPC].Type == 173 || NPC[Player[A].YoshiNPC].Type == 176)
            {
                NPC[Player[A].YoshiNPC].Type = 172;
                NPC[Player[A].YoshiNPC].Location.Height = 28;
            }
            else if(NPC[Player[A].YoshiNPC].Type == 175 || NPC[Player[A].YoshiNPC].Type == 177)
            {
                NPC[Player[A].YoshiNPC].Type = 174;
                NPC[Player[A].YoshiNPC].Location.Height = 28;
            }
            NPC[Player[A].YoshiNPC].Location.Height = NPCHeight[NPC[Player[A].YoshiNPC].Type];
            if((NPCIsGrabbable[NPC[Player[A].YoshiNPC].Type] == true || NPCIsAShell[NPC[Player[A].YoshiNPC].Type] == true || NPC[Player[A].YoshiNPC].Type == 40 || NPCIsABot[NPC[Player[A].YoshiNPC].Type] || NPC[Player[A].YoshiNPC].Type == 194 || NPC[Player[A].YoshiNPC].Type == 135 || NPC[Player[A].YoshiNPC].Type == 136 || NPC[Player[A].YoshiNPC].Type == 137) && (NPC[Player[A].YoshiNPC].Type != 166))
            {
                if(NPC[Player[A].YoshiNPC].Type == 135)
                    NPC[Player[A].YoshiNPC].Special = 450;
                if(NPC[Player[A].YoshiNPC].Type == 134)
                    NPC[Player[A].YoshiNPC].Special = 250;
                if(NPC[Player[A].YoshiNPC].Type == 136 || NPC[Player[A].YoshiNPC].Type == 137)
                {
                    NPC[Player[A].YoshiNPC].Special = 250;
                    NPC[Player[A].YoshiNPC].Type = 137;
                    NPC[Player[A].YoshiNPC].Location.Height = NPCHeight[NPC[Player[A].YoshiNPC].Type];
                }
                NPC[Player[A].YoshiNPC].Effect = 6;
                NPC[Player[A].YoshiNPC].Effect2 = A;
                NPC[Player[A].YoshiNPC].Active = false;
                if(NPC[Player[A].YoshiNPC].Type == 49)
                {
                    NPC[Player[A].YoshiNPC].Special = 0;
                    NPC[Player[A].YoshiNPC].Special2 = 0;
                }
            }
            else if(Player[A].MountType == 7 && NPCIsABonus[NPC[Player[A].YoshiNPC].Type] == false)
            {
                B = (iRand() % 9);
                NPC[Player[A].YoshiNPC].Type = 139 + B;
                if(NPC[Player[A].YoshiNPC].Type == 147)
                    NPC[Player[A].YoshiNPC].Type = 92;
                NPC[Player[A].YoshiNPC].Location.X = NPC[Player[A].YoshiNPC].Location.X + NPC[Player[A].YoshiNPC].Location.Width / 2.0;
                NPC[Player[A].YoshiNPC].Location.Y = NPC[Player[A].YoshiNPC].Location.Y + NPC[Player[A].YoshiNPC].Location.Height / 2.0;
                NPC[Player[A].YoshiNPC].Location.Width = NPCWidth[NPC[Player[A].YoshiNPC].Type];
                NPC[Player[A].YoshiNPC].Location.Height = NPCHeight[NPC[Player[A].YoshiNPC].Type];
                NPC[Player[A].YoshiNPC].Location.X = NPC[Player[A].YoshiNPC].Location.X - NPC[Player[A].YoshiNPC].Location.Width / 2.0;
                NPC[Player[A].YoshiNPC].Location.Y = NPC[Player[A].YoshiNPC].Location.Y - NPC[Player[A].YoshiNPC].Location.Height / 2.0;
                NPC[Player[A].YoshiNPC].Effect = 6;
                NPC[Player[A].YoshiNPC].Effect2 = A;
                NPC[Player[A].YoshiNPC].Active = false;
            }
            else if(Player[A].MountType == 8 && NPCIsABonus[NPC[Player[A].YoshiNPC].Type] == false)
            {
                NPC[Player[A].YoshiNPC].Type = 237;
                NPC[Player[A].YoshiNPC].Location.X = NPC[Player[A].YoshiNPC].Location.X + NPC[Player[A].YoshiNPC].Location.Width / 2.0;
                NPC[Player[A].YoshiNPC].Location.Y = NPC[Player[A].YoshiNPC].Location.Y + NPC[Player[A].YoshiNPC].Location.Height / 2.0;
                NPC[Player[A].YoshiNPC].Location.Width = NPCWidth[NPC[Player[A].YoshiNPC].Type];
                NPC[Player[A].YoshiNPC].Location.Height = NPCHeight[NPC[Player[A].YoshiNPC].Type];
                NPC[Player[A].YoshiNPC].Location.X = NPC[Player[A].YoshiNPC].Location.X - NPC[Player[A].YoshiNPC].Location.Width / 2.0;
                NPC[Player[A].YoshiNPC].Location.Y = NPC[Player[A].YoshiNPC].Location.Y - NPC[Player[A].YoshiNPC].Location.Height / 2.0;
                NPC[Player[A].YoshiNPC].Effect = 6;
                NPC[Player[A].YoshiNPC].Effect2 = A;
                NPC[Player[A].YoshiNPC].Active = false;
            }
            else
            {
                if(NPCIsABonus[NPC[Player[A].YoshiNPC].Type])
                {
                    TouchBonus(A, Player[A].YoshiNPC);
                    Player[A].YoshiNPC = 0;
                }
                else
                {
                    MoreScore(NPCScore[NPC[Player[A].YoshiNPC].Type], NPC[Player[A].YoshiNPC].Location, Player[A].Multiplier);
                    NPC[Player[A].YoshiNPC].Killed = 9;
                    Player[A].YoshiNPC = 0;
                    Player[A].FireBallCD = 30;
                    Coins = Coins + 1;
                    if(Coins >= 100)
                    {
                        if(Lives < 99)
                        {
                            Lives = Lives + 1;
                            PlaySound(15);
                            Coins = Coins - 100;
                        }
                        else
                            Coins = 99;
                    }
                    PlaySound(55);
                }
            }
        }
        else if(Player[A].MountSpecial == 0 && Player[A].YoshiPlayer > 0)
        {
            Player[Player[A].YoshiPlayer].Effect = 10;
            Player[Player[A].YoshiPlayer].Effect2 = A;
            Player[Player[A].YoshiPlayer].Location.X = Player[A].Location.X + Player[A].Location.Width / 2.0 - Player[Player[A].YoshiPlayer].Location.Width / 2.0;
            Player[Player[A].YoshiPlayer].Location.Y = Player[A].Location.Y + Player[A].Location.Height / 2.0 - Player[Player[A].YoshiPlayer].Location.Height / 2.0;
            Player[A].YoshiTFrameCount = 1;
        }
    }
}

void StealBonus()
{
    int A = 0;
    int B = 0;
    int C = 0;
    UNUSED(C);
//    Location_t tempLocation;

    // dead players steal life
    if(BattleMode == true)
        return;

    if(numPlayers == 2 /*&& nPlay.Online == false*/)
    {
        if((Player[1].Dead == true || Player[1].TimeToLive > 0) && (Player[2].Dead == true || Player[2].TimeToLive > 0))
            return;
        for(A = 1; A <= numPlayers; A++)
        {
            if(Player[A].Dead == true)
            {
                if(A == 1)
                {
                    B = 2;
                    C = 40;
                }
                else
                {
                    B = 1;
                    C = -40;
                }

                if(Lives > 0 && LevelMacro == 0)
                {
                    if(Player[A].Controls.Jump == true || Player[A].Controls.Run == true)
                    {
                        Lives = Lives - 1;
                        if(B == 1)
                            C = -40;
                        if(B == 2)
                            C = 40;
                        Player[A].State = 1;
                        Player[A].Hearts = 1;
                        Player[A].Location.Width = Physics.PlayerWidth[Player[A].Character][Player[A].State];
                        Player[A].Location.Height = Physics.PlayerHeight[Player[A].Character][Player[A].State];
                        Player[A].Frame = 1;
                        Player[A].Direction = Player[B].Direction;
                        Player[A].Dead = false;
                        Player[A].Location.SpeedX = 0;
                        Player[A].Location.SpeedY = 0;
                        Player[A].Effect = 6;
                        Player[A].Effect2 = Player[B].Location.Y + Player[B].Location.Height - Player[A].Location.Height;
                        if(Player[B].Mount == 2)
                            Player[A].Effect2 = Player[B].Location.Y - Player[A].Location.Height;
                        Player[A].Location.Y = -vScreenY[1] - Player[A].Location.Height;
                        Player[A].Location.X = Player[B].Location.X + Player[B].Location.Width / 2.0 - Player[A].Location.Width / 2.0;
                        PlaySound(11);
                    }
                }
            }
        }
    }
}

void ClownCar()
{
    // for when the player is in the clown car
    int A = 0;
    int B = 0;
//    int C = 0;
    NPC_t blankNPC;
    bool tempBool = false;
    Location_t tempLocation;

    for(A = 1; A <= numPlayers; A++) // Code for running the Koopa Clown Car
    {
        if(numPlayers > 2 && GameMenu == false && LevelMacro == 0 /*&& nPlay.Online == false*/)
            Player[A].Controls = Player[1].Controls;
        if(Player[A].Mount == 2 && Player[A].Dead == false && Player[A].TimeToLive == 0)
        {
            if(Player[A].Effect == 0)
            {
                if(Player[A].Controls.Left == true)
                {
                    Player[A].Location.SpeedX = Player[A].Location.SpeedX - 0.1;
                    if(Player[A].Location.SpeedX > 0)
                        Player[A].Location.SpeedX = Player[A].Location.SpeedX - 0.15;
                }
                else if(Player[A].Controls.Right == true)
                {
                    Player[A].Location.SpeedX = Player[A].Location.SpeedX + 0.1;
                    if(Player[A].Location.SpeedX < 0)
                        Player[A].Location.SpeedX = Player[A].Location.SpeedX + 0.15;
                }
                else
                {
                    if(Player[A].Location.SpeedX > 0.2)
                        Player[A].Location.SpeedX = Player[A].Location.SpeedX - 0.05;
                    else if(Player[A].Location.SpeedX < -0.2)
                        Player[A].Location.SpeedX = Player[A].Location.SpeedX + 0.05;
                    else
                        Player[A].Location.SpeedX = 0;
                }
                if(Player[A].Controls.Up == true)
                {
                    Player[A].Location.SpeedY = Player[A].Location.SpeedY - 0.1;
                    if(Player[A].Location.SpeedY > 0)
                        Player[A].Location.SpeedY = Player[A].Location.SpeedY - 0.2;
                }
                else if(Player[A].Controls.Down == true)
                {
                    Player[A].Location.SpeedY = Player[A].Location.SpeedY + 0.2;
                    if(Player[A].Location.SpeedY < 0)
                        Player[A].Location.SpeedY = Player[A].Location.SpeedY + 0.2;
                }
                else
                {
                    if(Player[A].Location.SpeedY > 0.1)
                        Player[A].Location.SpeedY = Player[A].Location.SpeedY - 0.1;
                    else if(Player[A].Location.SpeedY < -0.1)
                        Player[A].Location.SpeedY = Player[A].Location.SpeedY + 0.1;
                    else
                        Player[A].Location.SpeedY = 0;
                }
                if(Player[A].Location.SpeedX > 4)
                    Player[A].Location.SpeedX = 4;
                else if(Player[A].Location.SpeedX < -4)
                    Player[A].Location.SpeedX = -4;
                if(Player[A].Location.SpeedY > 10)
                    Player[A].Location.SpeedY = 10;
                else if(Player[A].Location.SpeedY < -4)
                    Player[A].Location.SpeedY = -4;
            }
            numNPCs++;
            NPC[numNPCs] = NPC_t();
            NPC[numNPCs].playerTemp = true;
            NPC[numNPCs].Type = 56;
            NPC[numNPCs].Active = true;
            NPC[numNPCs].TimeLeft = 100;
            NPC[numNPCs].Location = Player[A].Location;

            if(Player[A].Effect != 0)
            {
                NPC[numNPCs].Location.SpeedX = 0;
                NPC[numNPCs].Location.SpeedY = 0;
            }
            NPC[numNPCs].Location.Y = NPC[numNPCs].Location.Y + NPC[numNPCs].Location.SpeedY;
            NPC[numNPCs].Location.X = NPC[numNPCs].Location.X + NPC[numNPCs].Location.SpeedX;
            NPC[numNPCs].Section = Player[A].Section;
            for(B = 1; B <= numPlayers; B++)
            {
                if(Player[B].StandingOnTempNPC == 56)
                {
                    Player[B].StandingOnNPC = numNPCs;
                    Player[B].Location.X = Player[B].Location.X + double(Player[A].mountBump);
                    if(Player[B].Effect != 0)
                    {
                        Player[B].Location.Y = Player[A].Location.Y - Player[B].Location.Height;
                        Player[B].Location.X = Player[B].Location.X + Player[A].Location.SpeedX;
                    }
                }
            }

            for(int numNPCsMax8 = numNPCs, B = 1; B <= numNPCsMax8; B++)
            {
                if(NPC[B].standingOnPlayer == A && NPC[B].Type != 50)
                {
                    if(Player[A].Effect == 0)
                        NPC[B].Location.X = NPC[B].Location.X + Player[A].Location.SpeedX + double(Player[A].mountBump);
                    NPC[B].TimeLeft = 100;
                    NPC[B].Location.SpeedY = Player[A].Location.SpeedY;
                    NPC[B].Location.SpeedX = 0;
                    if(Player[A].Effect != 0)
                        NPC[B].Location.SpeedY = 0;
                    NPC[B].Location.Y = Player[A].Location.Y + NPC[B].Location.SpeedY + 0.1 - NPC[B].standingOnPlayerY;
                    if(Player[A].Controls.Run == true)
                    {
                        if(NPC[B].Type == 49)
                        {
                            if(NPC[B].Special == 0.0)
                            {
                                NPC[B].Special = 1;
                                numNPCs++;
                                NPC[numNPCs] = NPC_t();
                                NPC[B].Special2 = numNPCs;
                                NPC[numNPCs].Active = true;
                                NPC[numNPCs].Section = Player[A].Section;
                                NPC[numNPCs].TimeLeft = 100;
                                NPC[numNPCs].Type = 50;
                                NPC[numNPCs].Location.Height = 32;
                                NPC[numNPCs].Location.Width = 48;
                                NPC[numNPCs].Special = A;
                                NPC[numNPCs].Special2 = B;
                                NPC[numNPCs].Direction = NPC[B].Direction;
                                if(Maths::iRound(NPC[numNPCs].Direction) == 1)
                                    NPC[numNPCs].Frame = 2;
                            }
                            for(int numNPCsMax9 = numNPCs, C = 1; C <= numNPCsMax9; C++)
                            {
                                if(NPC[C].Type == 50 && Maths::iRound(NPC[C].Special) == A && Maths::iRound(NPC[C].Special2) == B)
                                {
                                    NPC[C].standingOnPlayer = A;
                                    NPC[C].Projectile = true;
                                    NPC[C].Direction = NPC[B].Direction;
                                    if(NPC[C].Direction > 0)
                                        NPC[C].Location.X = NPC[B].Location.X + 32;
                                    else
                                        NPC[C].Location.X = NPC[B].Location.X - NPC[C].Location.Width;
                                    NPC[C].Location.Y = NPC[B].Location.Y;
                                    NPC[C].TimeLeft = 100;
                                    break;
                                }
                            }
                        }
                    }
                    tempBool = false;
                    tempLocation = NPC[B].Location;
                    tempLocation.Y = tempLocation.Y + tempLocation.Height + 0.1;
                    tempLocation.X = tempLocation.X + 0.5;
                    tempLocation.Width = tempLocation.Width - 1;
                    tempLocation.Height = 1;
                    for(int numNPCsMax10 = numNPCs, C = 1; C <= numNPCsMax10; C++)
                    {
                        if(B != C && (NPC[C].standingOnPlayer == A || NPC[C].playerTemp == true))
                        {
                            if(CheckCollision(tempLocation, NPC[C].Location) == true)
                                tempBool = true;
                        }
                    }
                    if(tempBool == false)
                    {
                        NPC[B].standingOnPlayer = 0;
                        NPC[B].standingOnPlayerY = 0;
                    }
                    else
                        NPC[B].Location.SpeedX = 0;
                }
            }
        }
    }
}

void WaterCheck(int A)
{
    Location_t tempLocation;
    int B = 0;

    if(Player[A].Wet > 0)
    {
        Player[A].Wet = Player[A].Wet - 1;
        Player[A].Multiplier = 0;
    }

    if(Player[A].Quicksand > 0)
    {
        Player[A].Quicksand = Player[A].Quicksand - 1;
        if(Player[A].Quicksand == 0)
            Player[A].WetFrame = false;
    }

    if(UnderWater[Player[A].Section])
        Player[A].Wet = 2;

    if(Player[A].Wet > 0)
    {
        Player[A].SpinJump = false;
        Player[A].WetFrame = true;
        Player[A].Slide = false;
    }
    else if(Player[A].WetFrame)
    {
        if(Player[A].Location.SpeedY >= 3.1 || Player[A].Location.SpeedY <= -3.1)
        {
            Player[A].WetFrame = false;
            tempLocation.Width = 32;
            tempLocation.Height = 32;
            tempLocation.X = Player[A].Location.X + Player[A].Location.Width / 2.0 - tempLocation.Width / 2.0;
            tempLocation.Y = Player[A].Location.Y + Player[A].Location.Height - tempLocation.Height;
            NewEffect(114, tempLocation);
        }
    }

    for(int numWaterMax = numWater, B = 1; B <= numWaterMax; B++)
    {
        if(Water[B].Hidden == false)
        {
            if(CheckCollision(Player[A].Location, Water[B].Location) == true)
            {
                if(Player[A].Wet == 0 && Player[A].Mount != 2)
                {
                    Player[A].FlyCount = 0;
                    Player[A].CanFly = false;
                    Player[A].CanFly2 = false;
                    if(Player[A].Controls.Jump == false && Player[A].Controls.AltJump == false)
                        Player[A].CanJump = true;
                    Player[A].SwimCount = 0;
                    if(Player[A].Location.SpeedY > 0.5)
                        Player[A].Location.SpeedY = 0.5;
                    if(Player[A].Location.SpeedY < -1.5)
                        Player[A].Location.SpeedY = -1.5;
                    if(Player[A].WetFrame == false)
                    {
                        if(Player[A].Location.SpeedX > 0.5)
                            Player[A].Location.SpeedX = 0.5;
                        if(Player[A].Location.SpeedX < -0.5)
                            Player[A].Location.SpeedX = -0.5;
                    }
                    if(Player[A].Location.SpeedY > 0 && Player[A].WetFrame == false)
                    {
                        tempLocation.Width = 32;
                        tempLocation.Height = 32;
                        tempLocation.X = Player[A].Location.X + Player[A].Location.Width / 2.0 - tempLocation.Width / 2.0;
                        tempLocation.Y = Player[A].Location.Y + Player[A].Location.Height - tempLocation.Height;
                        NewEffect(114, tempLocation);
                    }
                }
                Player[A].Wet = 2;
                Player[A].SpinJump = false;
                if(Water[B].Quicksand == true)
                    Player[A].Quicksand = 3;
            }
        }
    }

    if(Player[A].Mount == 2)
    {
        Player[A].Wet = 0;
        Player[A].WetFrame = 0;
    }

    if(Player[A].Wet == 1)
    {
        if(Player[A].Location.SpeedY < 0 && (Player[A].Controls.AltJump || Player[A].Controls.Jump) && !Player[A].Controls.Down)
        {
            Player[A].Jump = 12;
            Player[A].Location.SpeedY = double(Physics.PlayerJumpVelocity);
        }
    }
    else if(Player[A].Wet == 2 && Player[A].Quicksand == 0)
    {
        if(dRand() * 100.0 > 97.0)
        {
            if(Player[A].Direction == 1)
                tempLocation = newLoc(Player[A].Location.X + Player[A].Location.Width - dRand() * 8, Player[A].Location.Y + 4 + dRand() * 8, 8, 8);
            else
                tempLocation = newLoc(Player[A].Location.X - 8 + dRand() * 8, Player[A].Location.Y + 4 + dRand() * 8, 8, 8);
            if(!UnderWater[Player[A].Section])
            {
                for(B = 1; B <= numWater; B++)
                {
                    if(CheckCollision(Water[B].Location, tempLocation))
                    {
                        NewEffect(113, tempLocation, 1, 0, ShadowMode);
                        break;
                    }
                }
            }
            else
                NewEffect(113, tempLocation, 1, 1, ShadowMode);
        }
    }
}

void Tanooki(int A)
{
    if(Player[A].Fairy)
           return;
// tanooki
    if(Player[A].Stoned && Player[A].Controls.Down && Player[A].StandingOnNPC == 0)
    {
        Player[A].Location.SpeedX = Player[A].Location.SpeedX * 0.8;
        if(Player[A].Location.SpeedX >= -0.5 && Player[A].Location.SpeedX <= 0.5)
            Player[A].Location.SpeedX = 0;
        if(Player[A].Location.SpeedY < 8)
            Player[A].Location.SpeedY = Player[A].Location.SpeedY + 0.25;
    }

    if(Player[A].StonedCD == 0)
    {
        // If .Mount = 0 And .State = 5 And .Controls.Run = True And .Controls.Down = True Then
        if(Player[A].Mount == 0 && Player[A].State == 5 && Player[A].Controls.AltRun && Player[A].Bombs == 0)
        {
            if(!Player[A].Stoned)
                Player[A].Effect = 500;
        }
        else if(Player[A].Stoned)
            Player[A].Effect = 500;
    }
    else
        Player[A].StonedCD = Player[A].StonedCD - 1;

    if(Player[A].Stoned)
    {
        Player[A].StonedTime = Player[A].StonedTime + 1;
        if(Player[A].StonedTime >= 240)
        {
            Player[A].Effect = 500;
            Player[A].StonedCD = 60;
        }
        else if(Player[A].StonedTime >= 180)
        {
            Player[A].Immune = Player[A].Immune + 1;
            if(Player[A].Immune % 3 == 0)
            {
                Player[A].Immune2 = !Player[A].Immune2;
//                if(Player[A].Immune2 == true)
//                    Player[A].Immune2 = false;
//                else
//                    Player[A].Immune2 = true;
            }
        }
    }
}

void PowerUps(int A)
{
    bool BoomOut = false;
    //int B = 0;

    if(Player[A].Fairy)
    {
        Player[A].SwordPoke = 0;
        Player[A].FireBallCD = 0;
        Player[A].FireBallCD2 = 0;
        Player[A].TailCount = 0;
        return;
    }


    if(Player[A].State == 6 && Player[A].Character == 4 && Player[A].Controls.Run == true && Player[A].RunRelease == true)
    {
        for(int numNPCsMax11 = numNPCs, B = 1; B <= numNPCsMax11; B++)
        {
            if(NPC[B].Active)
            {
                if(NPC[B].Type == 292)
                {
                    if(Maths::iRound(NPC[B].Special5) == A)
                        BoomOut = true;
                }
            }
        }
    }

// Hammer Throw Code
        if(!Player[A].Slide && Player[A].Vine == 0 && Player[A].State == 6 && Player[A].Duck == false && Player[A].Mount != 2 && Player[A].Mount != 3 && Player[A].HoldingNPC <= 0 && Player[A].Character != 5)
        {
            if(Player[A].Controls.Run == true && Player[A].SpinJump == false && Player[A].FireBallCD <= 0 && BoomOut == false)
            {
                if(Player[A].RunRelease == true || FlameThrower == true)
                {
                    if(numNPCs < maxNPCs - 100)
                    {
//                        if(nPlay.Online == true && A - 1 == nPlay.MySlot)
//                            Netplay::sendData Netplay::PutPlayerControls(nPlay.MySlot) + "1f" + std::to_string(A) + "|" + Player[A].FireBallCD - 1;
                        Player[A].FrameCount = 110;
                        Player[A].FireBallCD = 25;
                        numNPCs++;
                        NPC[numNPCs] = NPC_t();
                        if(ShadowMode == true)
                            NPC[numNPCs].Shadow = true;
                        NPC[numNPCs].Type = 171;
                        if(Player[A].Character == 3)
                        {
                            Player[A].FireBallCD = 45;
                            NPC[numNPCs].Type = 291;
                            if(Player[A].Controls.AltRun == true && Player[A].Mount == 0)
                            {
                                NPC[numNPCs].HoldingPlayer = A;
                                Player[A].HoldingNPC = numNPCs;
                                PlaySound(73);
                            }
                            else
                                PlaySound(75);
                        }
                        else if(Player[A].Character == 4)
                        {

                            Player[A].FireBallCD = 0;
                            if(FlameThrower == true)
                                Player[A].FireBallCD = 40;
                            NPC[numNPCs].Type = 292;
                            NPC[numNPCs].Special5 = A;
                            NPC[numNPCs].Special6 = Player[A].Direction;
                            PlaySound(75);
                        }
                        else
                            PlaySound(18);

                        NPC[numNPCs].Projectile = true;
                        NPC[numNPCs].Location.Height = NPCHeight[NPC[numNPCs].Type];
                        NPC[numNPCs].Location.Width = NPCWidth[NPC[numNPCs].Type];
                        NPC[numNPCs].Location.X = Player[A].Location.X + Physics.PlayerGrabSpotX[Player[A].Character][Player[A].State] * Player[A].Direction;
                        NPC[numNPCs].Location.Y = Player[A].Location.Y + Physics.PlayerGrabSpotY[Player[A].Character][Player[A].State];
                        NPC[numNPCs].Active = true;
                        NPC[numNPCs].TimeLeft = 100;
                        NPC[numNPCs].Location.SpeedY = 20;
                        NPC[numNPCs].CantHurt = 100;
                        NPC[numNPCs].CantHurtPlayer = A;
                        if(Player[A].Controls.Up == true)
                        {
                            NPC[numNPCs].Location.SpeedX = 2 * Player[A].Direction + Player[A].Location.SpeedX * 0.9;
                            if(Player[A].StandingOnNPC == 0)
                                NPC[numNPCs].Location.SpeedY = -8 + Player[A].Location.SpeedY * 0.3;
                            else
                                NPC[numNPCs].Location.SpeedY = -8 + NPC[Player[A].StandingOnNPC].Location.SpeedY * 0.3;
                            NPC[numNPCs].Location.Y = NPC[numNPCs].Location.Y - 24;
                            NPC[numNPCs].Location.X = NPC[numNPCs].Location.X - 6 * Player[A].Direction;
                            if(Player[A].Character == 3)
                            {
                                NPC[numNPCs].Location.SpeedY = NPC[numNPCs].Location.SpeedY + 1;
                                NPC[numNPCs].Location.SpeedX = NPC[numNPCs].Location.SpeedX * 1.5;
                            }
                            else if(Player[A].Character == 4)
                            {
                                NPC[numNPCs].Location.SpeedY = -8;
                                NPC[numNPCs].Location.SpeedX = 12 * Player[A].Direction + Player[A].Location.SpeedX;
                            }
                        }
                        else
                        {
                            NPC[numNPCs].Location.SpeedX = 4 * Player[A].Direction + Player[A].Location.SpeedX * 0.9;
                            if(Player[A].StandingOnNPC == 0)
                                NPC[numNPCs].Location.SpeedY = -5 + Player[A].Location.SpeedY * 0.3;
                            else
                                NPC[numNPCs].Location.SpeedY = -5 + NPC[Player[A].StandingOnNPC].Location.SpeedY * 0.3;
                            if(Player[A].Character == 3)
                                NPC[numNPCs].Location.SpeedY = NPC[numNPCs].Location.SpeedY + 1;
                            else if(Player[A].Character == 4)
                            {
                                NPC[numNPCs].Location.SpeedY = -5;
                                NPC[numNPCs].Location.SpeedX = 10 * Player[A].Direction + Player[A].Location.SpeedX;
                                NPC[numNPCs].Location.Y = NPC[numNPCs].Location.Y - 12;
                            }
                        }
                        if(Player[A].Character == 4)
                            NPC[numNPCs].Location.X = Player[A].Location.X + Player[A].Location.Width / 2.0 - NPC[numNPCs].Location.Width / 2.0;
                        CheckSectionNPC(numNPCs);
                    }
                }
            }
        }




// Fire Mario / Luigi code ---- FIRE FLOWER ACTION BALLS OF DOOM
        if(Player[A].Slide == false && Player[A].Vine == 0 && (Player[A].State == 3 || Player[A].State == 7) && Player[A].Duck == false && Player[A].Mount != 2 && Player[A].Mount != 3 && Player[A].HoldingNPC <= 0 && Player[A].Character != 5)
        {
            if(((Player[A].Controls.Run == true && Player[A].SpinJump == false) || (Player[A].SpinJump == true && Player[A].Direction != Player[A].SpinFireDir)) && Player[A].FireBallCD <= 0)
            {
                if((Player[A].RunRelease == true || Player[A].SpinJump == true) || (FlameThrower == true && Player[A].HoldingNPC <= 0))
                {
                    if(Player[A].SpinJump == true)
                        Player[A].SpinFireDir = Player[A].Direction;
                    if(numNPCs < maxNPCs - 100)
                    {
//                        if(nPlay.Online == true && A - 1 == nPlay.MySlot)
//                            Netplay::sendData Netplay::PutPlayerControls(nPlay.MySlot) + "1f" + std::to_string(A) + "|" + Player[A].FireBallCD - 1;
                        if(Player[A].SpinJump == false)
                            Player[A].FrameCount = 110;
                        numNPCs++;
                        NPC[numNPCs] = NPC_t();
                        if(ShadowMode == true)
                            NPC[numNPCs].Shadow = true;
                        NPC[numNPCs].Type = 13;
                        if(Player[A].State == 7)
                            NPC[numNPCs].Type = 265;
                        NPC[numNPCs].Projectile = true;
                        NPC[numNPCs].Location.Height = NPCHeight[NPC[numNPCs].Type];
                        NPC[numNPCs].Location.Width = NPCWidth[NPC[numNPCs].Type];
                        NPC[numNPCs].Location.X = Player[A].Location.X + Physics.PlayerGrabSpotX[Player[A].Character][Player[A].State] * Player[A].Direction + 4;
                        NPC[numNPCs].Location.Y = Player[A].Location.Y + Physics.PlayerGrabSpotY[Player[A].Character][Player[A].State];
                        NPC[numNPCs].Active = true;
                        NPC[numNPCs].TimeLeft = 100;
                        NPC[numNPCs].Location.SpeedY = 20;
                        NPC[numNPCs].CantHurt = 100;
                        NPC[numNPCs].CantHurtPlayer = A;
                        NPC[numNPCs].Special = Player[A].Character;
                        if(Player[A].State == 7)
                            NPC[numNPCs].Special = 1;
                        if((Player[A].Character == 3 || Player[A].Character == 4) && Player[A].Mount == 0 && Player[A].Controls.AltRun == true) // peach holds fireballs
                        {
                            Player[A].HoldingNPC = numNPCs;
                            NPC[numNPCs].HoldingPlayer = A;
                        }
                        if(Maths::iRound(NPC[numNPCs].Special) == 2)
                            NPC[numNPCs].Frame = 4;
                        if(Maths::iRound(NPC[numNPCs].Special) == 3)
                            NPC[numNPCs].Frame = 8;
                        if(Maths::iRound(NPC[numNPCs].Special) == 4)
                            NPC[numNPCs].Frame = 12;
                        CheckSectionNPC(numNPCs);
                        Player[A].FireBallCD = 30;
                        if(Player[A].Character == 2)
                            Player[A].FireBallCD = 35;
                        if(Player[A].Character == 3)
                            Player[A].FireBallCD = 40;
                        if(Player[A].Character == 4)
                            Player[A].FireBallCD = 25;
                        NPC[numNPCs].Location.SpeedX = 5 * Player[A].Direction + (Player[A].Location.SpeedX / 3.5);

                        if(Player[A].State == 7)
                        {
                            NPC[numNPCs].Location.SpeedY = 5;

                            if(Player[A].Controls.Up == true)
                            {
                                if(Player[A].StandingOnNPC != 0)
                                    NPC[numNPCs].Location.SpeedY = -8 + NPC[Player[A].StandingOnNPC].Location.SpeedY * 0.1;
                                else
                                    NPC[numNPCs].Location.SpeedY = -8 + Player[A].Location.SpeedY * 0.1;
                                NPC[numNPCs].Location.SpeedX = NPC[numNPCs].Location.SpeedX * 0.9;
                            }
                            if(FlameThrower == true)
                            {
                                NPC[numNPCs].Location.SpeedX = NPC[numNPCs].Location.SpeedX * 1.5;
                                NPC[numNPCs].Location.SpeedY = NPC[numNPCs].Location.SpeedY * 1.5;
                            }
                            if(Player[A].StandingOnNPC != 0)
                                NPC[numNPCs].Location.SpeedX = 5 * Player[A].Direction + (Player[A].Location.SpeedX / 3.5) + NPC[Player[A].StandingOnNPC].Location.SpeedX / 3.5;
                            PlaySound(18);
                            NPC[numNPCs].Location.SpeedX = NPC[numNPCs].Location.SpeedX * 0.8;
                        }
                        else
                        {
                            if(Maths::iRound(NPC[numNPCs].Special) == 2)
                                NPC[numNPCs].Location.SpeedX = NPC[numNPCs].Location.SpeedX * 0.85;

                            if(Player[A].Controls.Up == true)
                            {
                                if(Player[A].StandingOnNPC != 0)
                                    NPC[numNPCs].Location.SpeedY = -6 + NPC[Player[A].StandingOnNPC].Location.SpeedY * 0.1;
                                else
                                    NPC[numNPCs].Location.SpeedY = -6 + Player[A].Location.SpeedY * 0.1;
                                NPC[numNPCs].Location.SpeedX = NPC[numNPCs].Location.SpeedX * 0.9;
                            }

                            if(FlameThrower)
                            {
                                NPC[numNPCs].Location.SpeedX = NPC[numNPCs].Location.SpeedX * 1.5;
                                NPC[numNPCs].Location.SpeedY = NPC[numNPCs].Location.SpeedY * 1.5;
                            }

                            if(Player[A].StandingOnNPC != 0)
                                NPC[numNPCs].Location.SpeedX = 5 * Player[A].Direction + (Player[A].Location.SpeedX / 3.5) + NPC[Player[A].StandingOnNPC].Location.SpeedX / 3.5;
                            PlaySound(18);
                        }
                    }
                }
            }
        }
// RacoonMario
        if(Player[A].Slide == false && Player[A].Vine == 0 && (Player[A].State == 4 || Player[A].State == 5) && Player[A].Duck == false && Player[A].HoldingNPC == 0 && Player[A].Mount != 2 && Player[A].Stoned == false && Player[A].Effect == 0 && Player[A].Character != 5)
        {
             if(Player[A].Controls.Run == true || Player[A].SpinJump == true)
             {
                if(Player[A].TailCount == 0 || Player[A].TailCount >= 12)
                {
                    if(Player[A].RunRelease == true || Player[A].SpinJump == true)
                    {
                        Player[A].TailCount = 1;
                        if(Player[A].SpinJump == false)
                            PlaySound(33);
                    }
                }
             }
        }
        if(Player[A].TailCount > 0)
        {
            Player[A].TailCount = Player[A].TailCount + 1;
            if(Player[A].TailCount == 25)
                Player[A].TailCount = 0;
            if(Player[A].TailCount % 7 == 0 || (Player[A].SpinJump == true && Player[A].TailCount) % 2 == 0)
            {
                TailSwipe(A, true);
            }
            else
            {
                TailSwipe(A);
            }
            if(Player[A].HoldingNPC > 0)
                Player[A].TailCount = 0;
        }


// link stab
    if(Player[A].Character == 5 && Player[A].Vine == 0 && Player[A].Mount == 0 && Player[A].Stoned == false && Player[A].FireBallCD == 0)
    {
        if(Player[A].Bombs > 0 && Player[A].Controls.AltRun == true && Player[A].RunRelease == true)
        {
            Player[A].FireBallCD = 10;
            Player[A].Bombs = Player[A].Bombs - 1;
            numNPCs++;
            NPC[numNPCs] = NPC_t();
            NPC[numNPCs].Active = true;
            NPC[numNPCs].TimeLeft = Physics.NPCTimeOffScreen;
            NPC[numNPCs].Section = Player[A].Section;
            NPC[numNPCs].Type = 134;
            NPC[numNPCs].Location.Width = NPCWidth[NPC[numNPCs].Type];
            NPC[numNPCs].Location.Height = NPCHeight[NPC[numNPCs].Type];
            NPC[numNPCs].CantHurtPlayer = A;
            NPC[numNPCs].CantHurt = 1000;
            if(Player[A].Duck == true && (Player[A].Location.SpeedY == 0 || Player[A].Slope > 0 || Player[A].StandingOnNPC != 0))
            {
                NPC[numNPCs].Location.X = Player[A].Location.X + Player[A].Location.Width / 2.0 - NPC[numNPCs].Location.Width / 2.0;
                NPC[numNPCs].Location.Y = Player[A].Location.Y + Player[A].Location.Height - NPC[numNPCs].Location.Height;
                NPC[numNPCs].Location.SpeedX = 0;
                NPC[numNPCs].Location.SpeedY = 0;
                PlaySound(23);
            }
            else
            {
                NPC[numNPCs].Location.X = Player[A].Location.X + Player[A].Location.Width / 2.0 - NPC[numNPCs].Location.Width / 2.0;
                NPC[numNPCs].Location.Y = Player[A].Location.Y;
                NPC[numNPCs].Location.SpeedX = 5 * Player[A].Direction;
                NPC[numNPCs].Location.SpeedY = -6;
                NPC[numNPCs].Projectile = true;
                if(Player[A].Location.SpeedY == 0 || Player[A].Slope > 0 || Player[A].StandingOnNPC != 0)
                    Player[A].SwordPoke = -10;
                PlaySound(75);
            }


        }
        else if(Player[A].FireBallCD == 0 && Player[A].Controls.Run == true && Player[A].RunRelease == true)
        {
            Player[A].FireBallCD = 20;
            if(Player[A].Location.SpeedY != Physics.PlayerGravity && Player[A].StandingOnNPC == 0 && Player[A].Slope == 0) // Link ducks when jumping
            {
                if(Player[A].Wet == 0 && Player[A].WetFrame == false)
                {
                    if(Player[A].Controls.Down == true && Player[A].Duck == false && Player[A].Mount == 0)
                    {
                        Player[A].Duck = true;
                        Player[A].Location.Y = Player[A].Location.Y + Player[A].Location.Height;
                        Player[A].Location.Height = Physics.PlayerDuckHeight[Player[A].Character][Player[A].State];
                        Player[A].Location.Y = Player[A].Location.Y - Player[A].Location.Height;
                    }
                    else if(Player[A].Controls.Down == false && Player[A].Duck == true)
                    {
                        UnDuck(A);
                    }
                }
            }
            if(Player[A].Duck == true)
                Player[A].SwordPoke = 1;
            else
                Player[A].SwordPoke = -1;
        }
        else if(Player[A].Controls.Up == true && Player[A].Location.SpeedY < 0 && Player[A].Duck == false && Player[A].SwordPoke == 0) // Link stabs up
        {
            if(Player[A].WetFrame == false && Player[A].Frame == 10)
            {
                TailSwipe(A, true, true, 1);
            }
        }
        else if(Player[A].Controls.Down == true && (Player[A].Location.SpeedY > 0 && Player[A].StandingOnNPC == 0 && Player[A].Slope == 0) && Player[A].Duck == false && Player[A].SwordPoke == 0) // Link stabs down
        {
            if(Player[A].WetFrame == false && Player[A].Frame == 9)
            {
                TailSwipe(A, true, true, 2);
            }
        }
    }


// cooldown timer
    Player[A].FireBallCD2 = Player[A].FireBallCD2 - 1;
    if(Player[A].FireBallCD2 < 0)
        Player[A].FireBallCD2 = 0;

    if(!(Player[A].Character == 3 && NPC[Player[A].HoldingNPC].Type == 13))
    {
        Player[A].FireBallCD = Player[A].FireBallCD - 1;
        if(FlameThrower == true)
            Player[A].FireBallCD = Player[A].FireBallCD - 3;
        if(Player[A].FireBallCD < 0)
            Player[A].FireBallCD = 0;
    }
}

void SuperWarp(int A)
{
    bool canWarp = false;

    auto &plr = Player[A];

    if(plr.WarpCD <= 0 && plr.Mount != 2 && !plr.GroundPound && !plr.GroundPound2)
    {
        for(int B = 1; B <= numWarps; B++)
        {
            auto &warp = Warp[B];
            if(CheckCollision(plr.Location, warp.Entrance) && !warp.Hidden)
            {
                plr.ShowWarp = B;
                canWarp = false;

                if(warp.Effect == 3) // Portal
                    canWarp = true;
                else if(warp.Direction == 1 && plr.Controls.Up) // Pipe
                {
                    if(WarpCollision(plr.Location, B))
                        canWarp = true;
                }
                else if(warp.Direction == 2 && plr.Controls.Left)
                {
                    if(WarpCollision(plr.Location, B))
                        canWarp = true;
                }
                else if(warp.Direction == 3 && plr.Controls.Down)
                {
                    if(WarpCollision(plr.Location, B))
                        canWarp = true;
                }
                else if(warp.Direction == 4 && plr.Controls.Right)
                {
                    if(WarpCollision(plr.Location, B))
                        canWarp = true;
                }
                // NOTE: Would be correct to move this up, but leave this here for a compatibility to keep the same behavior
                else if(warp.Effect == 0) // Instant
                    canWarp = true;

                if(warp.LevelEnt)
                    canWarp = false;

                if(warp.Stars > numStars && canWarp)
                {
                    if(warp.StarsMsg.empty())
                    {
                        if(warp.Stars == 1)
                            MessageText = "You need 1 star to enter.";
                        else
                            MessageText = fmt::format_ne("You need {0} stars to enter.", warp.Stars);
                    } else {
                        MessageText = warp.StarsMsg;
                    }
                    PauseGame(A);
                    MessageText = "";
                    canWarp = false;
                }

                if(canWarp)
                {
                    plr.Slide = false;

                    if(warp.Effect != 3)
                        plr.Stoned = false;

                    if(warp.Locked)
                    {
                        if(plr.HoldingNPC > 0 && NPC[plr.HoldingNPC].Type == 31)
                        {
                            NPC[plr.HoldingNPC].Killed = 9;
                            NewEffect(10, NPC[plr.HoldingNPC].Location);
                            warp.Locked = false;
                            int allBGOs = numBackground + numLocked;
                            for(int C = numBackground; C <= allBGOs; C++)
                            {
                                if(Background[C].Type == 98)
                                {
                                    if(CheckCollision(warp.Entrance, Background[C].Location))
                                    {
                                        Background[C].Layer.clear();
                                        Background[C].Hidden = true;
                                    }
                                }
                            }
                        }
                        else if(plr.Mount == 3 && plr.YoshiNPC > 0 && NPC[plr.YoshiNPC].Type == 31)
                        {
                            NPC[plr.YoshiNPC].Killed = 9;
                            plr.YoshiNPC = 0;
                            warp.Locked = false;
                            int allBGOs = numBackground + numLocked;
                            for(int C = numBackground; C <= allBGOs; C++)
                            {
                                if(Background[C].Type == 98)
                                {
                                    if(CheckCollision(warp.Entrance, Background[C].Location))
                                    {
                                        Background[C].Layer.clear();
                                        Background[C].Hidden = true;
                                    }
                                }
                            }
                        }
                        else if(plr.HasKey)
                        {
                            plr.HasKey = false;
                            warp.Locked = false;
                            int allBGOs = numBackground + numLocked;
                            for(int C = numBackground; C <= allBGOs; C++)
                            {
                                if(Background[C].Type == 98)
                                {
                                    if(CheckCollision(warp.Entrance, Background[C].Location))
                                    {
                                        Background[C].Layer.clear();
                                        Background[C].Hidden = true;
                                    }
                                }
                            }
                        }
                        else
                            canWarp = false;
                    }
                }

                if(canWarp)
                {
                    UnDuck(A);
                    plr.YoshiTongueLength = 0;
                    plr.MountSpecial = 0;
                    plr.FrameCount = 0;
                    plr.TailCount = 0;
                    plr.CanFly = false;
                    plr.CanFly2 = false;
                    plr.RunCount = 0;

                    if(warp.NoYoshi && plr.YoshiPlayer > 0)
                    {
                        YoshiSpit(A);
                    }

                    if(!warp.WarpNPC || (plr.Mount == 3 && (plr.YoshiNPC != 0 || plr.YoshiPlayer != 0) && warp.NoYoshi))
                    {
                        if(plr.HoldingNPC > 0)
                        {
                            if(NPC[plr.HoldingNPC].Type == 29)
                            {
                                NPCHit(plr.HoldingNPC, 3, plr.HoldingNPC);
                            }
                        }
                        if(plr.Character == 3 ||
                          (plr.Character == 4 && warp.Effect == 1 && warp.Direction == 1))
                            NPC[plr.HoldingNPC].Location.Y = warp.Entrance.Y;
                        plr.HoldingNPC = 0;
                        if(plr.YoshiNPC > 0)
                        {
                            YoshiSpit(A);
                        }
                    }

                    if(plr.HoldingNPC > 0)
                    {
                        if(NPC[plr.HoldingNPC].Type == 263) // can't bring ice through warps
                        {
                            NPC[plr.HoldingNPC].HoldingPlayer = 0;
                            plr.HoldingNPC = 0;
                        }
                    }

                    plr.StandingOnNPC = 0;
                    if(warp.Effect != 3) // Don't zero speed when passing a portal warp
                    {
                        plr.Location.SpeedX = 0;
                        plr.Location.SpeedY = 0;
                    }

                    if(!warp.eventEnter.empty())
                        ProcEvent(warp.eventEnter);

                    if(warp.Effect == 0 || warp.Effect == 3)
                    {
                        plr.Location.X = warp.Exit.X + warp.Exit.Width / 2.0 - plr.Location.Width / 2.0;
                        plr.Location.Y = warp.Exit.Y + warp.Exit.Height - plr.Location.Height - 0.1;
                        CheckSection(A);
                        plr.WarpCD = (warp.Effect == 3) ? 10 : 50;
                        break;
                    }
                    else if(warp.Effect == 1)
                    {
                        PlaySound(17);
                        plr.Effect = 3;
                        plr.Warp = B;
//                        if(nPlay.Online == true && A == nPlay.MySlot + 1)
//                            Netplay::sendData Netplay::PutPlayerLoc(nPlay.MySlot) + "1j" + std::to_string(A) + "|" + plr.Warp + LB;
                    }
                    else if(warp.Effect == 2)
                    {
                        PlaySound(46);
                        plr.Effect = 7;
                        plr.Warp = B;
//                        if(nPlay.Online == true && A == nPlay.MySlot + 1)
//                            Netplay::sendData Netplay::PutPlayerLoc(nPlay.MySlot) + "1j" + std::to_string(A) + "|" + plr.Warp + LB;
                        plr.Location.X = Warp[plr.Warp].Entrance.X + Warp[plr.Warp].Entrance.Width / 2.0 - plr.Location.Width / 2.0;
                        plr.Location.Y = Warp[plr.Warp].Entrance.Y + Warp[plr.Warp].Entrance.Height - plr.Location.Height;
                        for(int C = 1; C <= numBackground; C++)
                        {
                            if((CheckCollision(warp.Entrance, Background[C].Location) | CheckCollision(warp.Exit, Background[C].Location)) != 0)
                            {
                                if(Background[C].Type == 88)
                                    NewEffect(54, Background[C].Location);
                                else if(Background[C].Type == 87)
                                    NewEffect(55, Background[C].Location);
                                else if(Background[C].Type == 107)
                                    NewEffect(59, Background[C].Location);
                                else if(Background[C].Type == 141)
                                {
                                    Location_t bLoc = Background[C].Location;
                                    bLoc.X = bLoc.X + bLoc.Width / 2.0;
                                    bLoc.Width = 104;
                                    bLoc.X = bLoc.X - bLoc.Width / 2.0;
                                    NewEffect(103, bLoc);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if(plr.Mount != 2)
        plr.WarpCD--;
}

void PlayerCollide(int A)
{
    int B = 0;
    Location_t tempLocation;
    Location_t tempLocation3;
    int HitSpot = 0;

// Check player collisions
    for(B = 1; B <= numPlayers; B++)
    {
        if(B != A && Player[B].Dead == false && Player[B].TimeToLive == 0 && (Player[B].Effect == 0 || Player[B].Effect == 3) && !(Player[B].Mount == 2 || Player[A].Mount == 2) && (BattleMode == false || (Player[A].Immune == 0 && Player[B].Immune == 0)))
        {
            tempLocation = Player[A].Location;
            if(Player[A].StandingOnNPC != 0 && FreezeNPCs == false)
                tempLocation.SpeedY = NPC[Player[A].StandingOnNPC].Location.SpeedY;
            tempLocation3 = Player[B].Location;
            if(Player[B].StandingOnNPC != 0 && FreezeNPCs == false)
                tempLocation3.SpeedY = NPC[Player[B].StandingOnNPC].Location.SpeedY;
            if(CheckCollision(tempLocation, tempLocation3) == true)
            {
                HitSpot = FindCollision(tempLocation, tempLocation3);
                if(HitSpot == 5)
                {
                    if(Player[A].StandUp2 == true && Player[A].Location.Y > Player[B].Location.Y)
                        HitSpot = 3;
                    else if(Player[B].StandUp2 == true && Player[A].Location.Y < Player[B].Location.Y)
                        HitSpot = 1;
                }
                if(BattleMode == true)
                {
                    if(Player[B].SlideKill == true)
                    {
                        HitSpot = 0;
                        PlayerHurt(A);
                    }
                    if(Player[A].SlideKill == true)
                    {
                        HitSpot = 0;
                        PlayerHurt(B);
                    }
                    if(Player[A].Stoned == true && (Player[A].Location.SpeedX > 3 || Player[A].Location.SpeedX < -3 || HitSpot == 1))
                    {
                        PlayerHurt(B);
                    }
                    else if(Player[B].Stoned == true && (Player[B].Location.SpeedX > 3 || Player[B].Location.SpeedX < -3 || HitSpot == 3))
                    {
                        PlayerHurt(A);
                    }
                }

                if(Player[A].Stoned == true || Player[B].Stoned == true)
                    HitSpot = 0;
                if(HitSpot == 2 || HitSpot == 4)
                {
                    if(numPlayers < 3 /*|| nPlay.Online == true*/)
                        PlaySound(10);
                    tempLocation = Player[A].Location;
                    Player[A].Location.SpeedX = Player[B].Location.SpeedX;
                    Player[B].Location.SpeedX = tempLocation.SpeedX;
                    Player[A].Bumped = true;
                    Player[B].Bumped = true;
                }
                else if(HitSpot == 1)
                {
                    if(numPlayers < 3 /*|| nPlay.Online == true*/)
                        PlaySound(2);
                    Player[A].Location.Y = Player[B].Location.Y - Player[A].Location.Height - 0.1;
                    PlayerPush(A, 3);
                    Player[A].Location.SpeedY = Physics.PlayerJumpVelocity;
                    Player[A].Jump = Physics.PlayerHeadJumpHeight;
                    if(Player[A].Character == 2)
                        Player[A].Jump = Player[A].Jump + 3;
                    if(Player[A].SpinJump == true)
                        Player[A].Jump = Player[A].Jump - 6;
                    Player[B].Jump = 0;
                    if(Player[B].Location.SpeedY <= 0)
                        Player[B].Location.SpeedY = 0.1;
                    Player[B].CanJump = false;
                    NewEffect(75, newLoc(Player[A].Location.X + Player[A].Location.Width / 2.0 - 16, Player[A].Location.Y + Player[A].Location.Height - 16));
                }
                else if(HitSpot == 3)
                {
                    if(numPlayers < 3/* || nPlay.Online == true*/)
                        PlaySound(2);
                    Player[B].Location.Y = Player[A].Location.Y - Player[B].Location.Height - 0.1;
                    PlayerPush(B, 3);
                    Player[B].Location.SpeedY = Physics.PlayerJumpVelocity;
                    Player[B].Jump = Physics.PlayerHeadJumpHeight;
                    if(Player[B].Character == 2)
                        Player[A].Jump = Player[A].Jump + 3;
                    if(Player[A].SpinJump == true)
                        Player[A].Jump = Player[A].Jump - 6;
                    Player[A].Jump = 0;
                    if(Player[A].Location.SpeedY <= 0)
                        Player[A].Location.SpeedY = 0.1;
                    Player[A].CanJump = false;
                    NewEffect(75, newLoc(Player[B].Location.X + Player[B].Location.Width / 2.0 - 16, Player[B].Location.Y + Player[B].Location.Height - 16));
                }
                else if(HitSpot == 5)
                {
                    if(Player[A].Location.X + Player[A].Location.Width / 2.0 > Player[B].Location.X + Player[B].Location.Width / 2.0)
                    {
                        Player[A].Bumped2 = 1;
                        Player[B].Bumped2 = -1;
                    }
                    else if(Player[A].Location.X + Player[A].Location.Width / 2.0 < Player[B].Location.X + Player[B].Location.Width / 2.0)
                    {
                        Player[A].Bumped2 = -1;
                        Player[B].Bumped2 = 1;
                    }
                    else if(iRand() % 2 == 1)
                    {
                        Player[A].Bumped2 = -1;
                        Player[B].Bumped2 = 1;
                    }
                    else
                    {
                        Player[A].Bumped2 = 1;
                        Player[B].Bumped2 = -1;
                    }
                    // If Player(A).Bumped2 < -1 Then Player(A).Bumped2 = -1 - Rnd
                    // If Player(A).Bumped2 > 1 Then Player(A).Bumped2 = 1 + Rnd
                    // If Player(B).Bumped2 < -1 Then Player(B).Bumped2 = -1 - Rnd
                    // If Player(B).Bumped2 > 1 Then Player(B).Bumped2 = 1 + Rnd
                }
                if(BattleMode == true)
                {
                    if(HitSpot == 1 && Player[A].Mount == 1)
                    {
                        PlayerHurt(B);
                    }
                    else if(HitSpot == 3 && Player[B].Mount == 1)
                    {
                        PlayerHurt(A);
                    }
                }
            }
        }
    }
}

void PlayerGrabCode(int A, bool DontResetGrabTime)
{
    // this code handles all the grab related stuff
    // for grabbing something while walking into it, refer to the NPC collision code in sub UpdatePlayer()
    Location_t tempLocation;
    int LayerNPC = 0;
    int B = 0;
    int C = 0;
    bool tempBool = false;
    double lyrX = 0;
    double lyrY = 0;

    if(Player[A].StandingOnNPC != 0 && Player[A].HoldingNPC == 0)
    {
        if(NPCGrabFromTop[NPC[Player[A].StandingOnNPC].Type] == true)
        {
            if(((Player[A].Controls.Run == true && Player[A].Controls.Down == true) || ((Player[A].Controls.Down == true || Player[A].Controls.Run == true) && Player[A].GrabTime > 0)) && (Player[A].RunRelease == true || Player[A].GrabTime > 0) && Player[A].TailCount == 0)
            {
                if((Player[A].GrabTime >= 12 && Player[A].Character < 3) || (Player[A].GrabTime >= 16 && Player[A].Character == 3) || (Player[A].GrabTime >= 8 && Player[A].Character == 4))
                {
                    Player[A].Location.SpeedX = Player[A].GrabSpeed;
                    Player[A].GrabSpeed = 0;
                    Player[A].GrabTime = 0;
                    Player[A].TailCount = 0;
                    if(Player[A].Character == 1 || Player[A].Character == 2)
                    {
                        UnDuck(A);
                    }
                    Player[A].HoldingNPC = Player[A].StandingOnNPC;
                    Player[A].Location.SpeedY = NPC[Player[A].StandingOnNPC].Location.SpeedY;
                    if(Player[A].Location.SpeedY == 0)
                        Player[A].Location.SpeedY = 0.01;
                    Player[A].CanJump = false;
                    if(NPCIsAShell[NPC[Player[A].StandingOnNPC].Type] == true)
                        Player[A].Location.SpeedX = NPC[Player[A].StandingOnNPC].Location.SpeedX;
                    NPC[Player[A].StandingOnNPC].HoldingPlayer = A;
                    NPC[Player[A].StandingOnNPC].CantHurt = Physics.NPCCanHurtWait;
                    NPC[Player[A].StandingOnNPC].CantHurtPlayer = A;
                    NPC[Player[A].StandingOnNPC].Direction = Player[A].Direction;
                    NPCFrames(Player[A].StandingOnNPC);
                    if(NPC[Player[A].StandingOnNPC].Type == 91)
                    {
                        Player[A].Location.SpeedX = Player[A].Location.SpeedX + NPC[Player[A].StandingOnNPC].Location.SpeedX;
                        NPC[Player[A].StandingOnNPC].Direction = Player[A].Direction;
                        NPC[Player[A].StandingOnNPC].Generator = false;
                        NPC[Player[A].StandingOnNPC].Frame = 0;
                        NPC[Player[A].StandingOnNPC].Frame = EditorNPCFrame(NPC[Player[A].StandingOnNPC].Type, NPC[Player[A].StandingOnNPC].Direction);
                        NPC[Player[A].StandingOnNPC].Type = NPC[Player[A].StandingOnNPC].Special;
                        if(NPC[Player[A].StandingOnNPC].Type == 287)
                        {
                            NPC[Player[A].StandingOnNPC].Type = RandomBonus();
                            NPC[Player[A].StandingOnNPC].DefaultSpecial = NPC[Player[A].StandingOnNPC].Type;
                        }
                        CharStuff(Player[A].StandingOnNPC);
                        NPC[Player[A].StandingOnNPC].Special = 0;
                        if(NPCIsYoshi[NPC[Player[A].StandingOnNPC].Type])
                        {
                            NPC[Player[A].StandingOnNPC].Special = NPC[Player[A].StandingOnNPC].Type;
                            NPC[Player[A].StandingOnNPC].Type = 96;
                        }
                        if(!(NPC[Player[A].StandingOnNPC].Type == 21 || NPC[Player[A].StandingOnNPC].Type == 22 || NPC[Player[A].StandingOnNPC].Type == 26 || NPC[Player[A].StandingOnNPC].Type == 31 || NPC[Player[A].StandingOnNPC].Type == 32 || NPC[Player[A].StandingOnNPC].Type == 35 || NPC[Player[A].StandingOnNPC].Type == 191 || NPC[Player[A].StandingOnNPC].Type == 193 || NPC[Player[A].StandingOnNPC].Type == 49 || NPCIsAnExit[NPC[Player[A].StandingOnNPC].Type]))
                        {
                            if(BattleMode == false)
                                NPC[Player[A].StandingOnNPC].DefaultType = 0;
                        }
                        NPC[Player[A].StandingOnNPC].Location.Height = NPCHeight[NPC[Player[A].StandingOnNPC].Type];
                        NPC[Player[A].StandingOnNPC].Location.Width = NPCWidth[NPC[Player[A].StandingOnNPC].Type];
                        if(NPC[Player[A].StandingOnNPC].Type == 147)
                        {
                            B = (iRand() % 9);
                            NPC[Player[A].StandingOnNPC].Type = 139 + B;
                            if(NPC[Player[A].StandingOnNPC].Type == 147)
                                NPC[Player[A].StandingOnNPC].Type = 92;
                            NPC[Player[A].StandingOnNPC].Location.X = NPC[Player[A].StandingOnNPC].Location.X + NPC[Player[A].StandingOnNPC].Location.Width / 2.0;
                            NPC[Player[A].StandingOnNPC].Location.Y = NPC[Player[A].StandingOnNPC].Location.Y + NPC[Player[A].StandingOnNPC].Location.Height / 2.0;
                            NPC[Player[A].StandingOnNPC].Location.Width = NPCWidth[NPC[Player[A].StandingOnNPC].Type];
                            NPC[Player[A].StandingOnNPC].Location.Height = NPCHeight[NPC[Player[A].StandingOnNPC].Type];
                            NPC[Player[A].StandingOnNPC].Location.X = NPC[Player[A].StandingOnNPC].Location.X - NPC[Player[A].StandingOnNPC].Location.Width / 2.0;
                            NPC[Player[A].StandingOnNPC].Location.Y = NPC[Player[A].StandingOnNPC].Location.Y - NPC[Player[A].StandingOnNPC].Location.Height / 2.0;
                        }
                        NPCFrames(Player[A].StandingOnNPC);
                        Player[A].StandingOnNPC = 0;
                    }
                }
                else
                {
                    if(Player[A].GrabTime == 0)
                    {
                        if(NPC[Player[A].StandingOnNPC].Type == 91 || NPCIsVeggie[NPC[Player[A].StandingOnNPC].Type] == true)
                            PlaySound(73);
                        else
                            PlaySound(23);
                        Player[A].FrameCount = 0;
                        Player[A].GrabSpeed = Player[A].Location.SpeedX;
                    }
                    Player[A].Location.SpeedX = 0;
                    Player[A].GrabTime = Player[A].GrabTime + 1;
                    Player[A].Slide = false;
                }
            }
            else
                Player[A].GrabTime = 0;
        }
        else
            Player[A].GrabTime = 0;
    }
    else if(DontResetGrabTime == false)
        Player[A].GrabTime = 0;
    if(Player[A].HoldingNPC > numNPCs) // Can't hold an NPC that is dead
        Player[A].HoldingNPC = 0;
    if(Player[A].HoldingNPC > 0)
    {
        lyrX = NPC[Player[A].HoldingNPC].Location.X;
        lyrY = NPC[Player[A].HoldingNPC].Location.Y;
        LayerNPC = Player[A].HoldingNPC;
        if(NPC[Player[A].HoldingNPC].Type == 278 || NPC[Player[A].HoldingNPC].Type == 279)
        {
            if(Player[A].Jump == 0)
                NPC[Player[A].HoldingNPC].Special = 1;
            else if(Player[A].Jump > 0 && NPC[Player[A].HoldingNPC].Special == 1)
            {
                Player[A].Jump = Player[A].Jump * 1.5;
                NPC[Player[A].HoldingNPC].Special = 0;
            }
            if(NPC[Player[A].HoldingNPC].Type == 279)
            {
                NPC[Player[A].HoldingNPC].Special2 = NPC[Player[A].HoldingNPC].Special2 + 1;
                if(Player[A].SpinJump == true)
                {
                    if(NPC[Player[A].HoldingNPC].Special3 == 0)
                        NPC[Player[A].HoldingNPC].Special3 = Player[A].Direction;
                    else if(NPC[Player[A].HoldingNPC].Special3 == -Player[A].Direction && NPC[Player[A].HoldingNPC].Special2 >= 25)
                    {
                        NPC[Player[A].HoldingNPC].Special3 = Player[A].Direction;
                        NPC[Player[A].HoldingNPC].Special2 = 100;
                    }
                    else if(NPC[Player[A].HoldingNPC].Special2 >= 25)
                        NPC[Player[A].HoldingNPC].Special2 = 25;

                }


                if(NPC[Player[A].HoldingNPC].Special2 == 20 || NPC[Player[A].HoldingNPC].Special2 == 40 || NPC[Player[A].HoldingNPC].Special2 == 60 || NPC[Player[A].HoldingNPC].Special2 == 80)
                {
                    if(NPC[Player[A].HoldingNPC].Special2 == 20 || NPC[Player[A].HoldingNPC].Special2 == 60)
                        B = 1;
                    else if(NPC[Player[A].HoldingNPC].Special2 == 40)
                        B = 2;
                    else if(NPC[Player[A].HoldingNPC].Special2 == 80)
                    {
                        B = 3;
                        NPC[Player[A].HoldingNPC].Special2 = 0;
                    }

                        PlaySound(82);

                    // For B = 1 To 3
                        numNPCs++;
                        NPC[numNPCs] = NPC_t();
                        NPC[numNPCs].CantHurt = 10000;
                        NPC[numNPCs].CantHurtPlayer = A;
                        NPC[numNPCs].BattleOwner = A;
                        NPC[numNPCs].Type = 108;
                        NPC[numNPCs].Location.Width = NPCWidth[108];
                        NPC[numNPCs].Location.Height = NPCHeight[108];
                        NPC[numNPCs].Active = true;
                        NPC[numNPCs].TimeLeft = NPC[Player[A].HoldingNPC].TimeLeft;
                        NPC[numNPCs].Layer = "Spawned NPCs";
                        NPC[numNPCs].Location.Y = NPC[Player[A].HoldingNPC].Location.Y + NPC[Player[A].HoldingNPC].Location.Height - NPC[numNPCs].Location.Height;
                        NPC[numNPCs].Direction = Player[A].Direction;
                        if(NPC[numNPCs].Direction == 1)
                            NPC[numNPCs].Location.X = NPC[Player[A].HoldingNPC].Location.X + NPC[Player[A].HoldingNPC].Location.Width * 2 - 8;
                        else
                            NPC[numNPCs].Location.X = NPC[Player[A].HoldingNPC].Location.X - NPC[numNPCs].Location.Width - NPC[Player[A].HoldingNPC].Location.Width + 8;

                        if(B == 1)
                            NPC[numNPCs].Location.SpeedX = 7 * NPC[numNPCs].Direction + (Player[A].Location.SpeedX / 3.5);
                        else if(B == 2)
                        {
                            NPC[numNPCs].Location.SpeedX = 6.5 * NPC[numNPCs].Direction + (Player[A].Location.SpeedX / 3.5);
                            NPC[numNPCs].Location.SpeedY = -1.5;
                        }
                        else
                        {
                            NPC[numNPCs].Location.SpeedX = 6.5 * NPC[numNPCs].Direction + (Player[A].Location.SpeedX / 3.5);
                            NPC[numNPCs].Location.SpeedY = 1.5;
                        }

                        NPC[numNPCs].Projectile = true;
                        NPC[numNPCs].Frame = EditorNPCFrame(NPC[numNPCs].Type, NPC[numNPCs].Direction);
                    // Next B
                }
            }
        }

        if(NPC[Player[A].HoldingNPC].Type == 263)
            NPC[Player[A].HoldingNPC].Special3 = 0;
        NPC[Player[A].HoldingNPC].TimeLeft = Physics.NPCTimeOffScreen;
        NPC[Player[A].HoldingNPC].Effect = 0;
        NPC[Player[A].HoldingNPC].CantHurt = Physics.NPCCanHurtWait;
        NPC[Player[A].HoldingNPC].CantHurtPlayer = A;
        if(NPCIsVeggie[NPC[Player[A].HoldingNPC].Type])
            NPC[Player[A].HoldingNPC].CantHurt = 1000;
        if(Player[A].Controls.Run == true || Player[A].ForceHold > 0)
        {

        // hold above head
            if(Player[A].Character == 3 || Player[A].Character == 4 || (Player[A].Duck == true))
            {
                NPC[Player[A].HoldingNPC].Bouce = true;
                NPC[Player[A].HoldingNPC].Location.X = Player[A].Location.X + Player[A].Location.Width / 2.0 - NPC[Player[A].HoldingNPC].Location.Width / 2.0;
                if(Player[A].Character == 3) // princess peach
                {
                    if(Player[A].State == 1)
                        NPC[Player[A].HoldingNPC].Location.Y = Player[A].Location.Y - NPC[Player[A].HoldingNPC].Location.Height;
                    else
                        NPC[Player[A].HoldingNPC].Location.Y = Player[A].Location.Y - NPC[Player[A].HoldingNPC].Location.Height + 6;
                }
                else // toad
                {
                    if(Player[A].State == 1)
                        NPC[Player[A].HoldingNPC].Location.Y = Player[A].Location.Y - NPC[Player[A].HoldingNPC].Location.Height + 6;
                    else
                    {
                        if(NPC[Player[A].HoldingNPC].Type == 13 || NPC[Player[A].HoldingNPC].Type == 265)
                        {
                            NPC[Player[A].HoldingNPC].Location.X = Player[A].Location.X + Player[A].Location.Width / 2.0 - NPC[Player[A].HoldingNPC].Location.Width / 2.0 + dRand() * 4 - 2;
                            NPC[Player[A].HoldingNPC].Location.Y = Player[A].Location.Y - NPC[Player[A].HoldingNPC].Location.Height - 4 + dRand() * 4 - 2;
                        }
                        else
                            NPC[Player[A].HoldingNPC].Location.Y = Player[A].Location.Y - NPC[Player[A].HoldingNPC].Location.Height + 10;
                    }
                }
            }
            else
            {
                if(Player[A].Direction > 0)
                    NPC[Player[A].HoldingNPC].Location.X = Player[A].Location.X + Physics.PlayerGrabSpotX[Player[A].Character][Player[A].State];
                else
                    NPC[Player[A].HoldingNPC].Location.X = Player[A].Location.X + Player[A].Location.Width - Physics.PlayerGrabSpotX[Player[A].Character][Player[A].State] - NPC[Player[A].HoldingNPC].Location.Width;
                NPC[Player[A].HoldingNPC].Location.Y = Player[A].Location.Y + Physics.PlayerGrabSpotY[Player[A].Character][Player[A].State] + 32 - NPC[Player[A].HoldingNPC].Location.Height;
            }
            if(NPC[Player[A].HoldingNPC].Type == 49 && FreezeNPCs == false)
            {
                if(NPC[Player[A].HoldingNPC].Special == 0.0)
                {
                    NPC[Player[A].HoldingNPC].Special = 1;
                    NPC[Player[A].HoldingNPC].Special2 = numNPCs + 1;
                    numNPCs++;
                    NPC[numNPCs] = NPC_t();
                    NPC[numNPCs].Active = true;
                    NPC[numNPCs].Section = Player[A].Section;
                    NPC[numNPCs].TimeLeft = 100;
                    NPC[numNPCs].Type = 50;
                    NPC[numNPCs].Location.Height = 32;
                    NPC[numNPCs].Location.Width = 48;
                    NPC[numNPCs].Special = A;
                    if(Player[A].Direction > 0)
                        NPC[numNPCs].Frame = 2;
                }
                for(B = 1; B <= numNPCs; B++)
                {
                    if(NPC[B].Type == 50 && NPC[B].Special == A)
                    {
                        NPC[B].CantHurt = 10;
                        NPC[B].CantHurtPlayer = A;
                        NPC[B].Projectile = true;
                        NPC[B].Direction = Player[A].Direction;
                        NPC[B].TimeLeft = 100;
                        if(Player[A].Direction > 0)
                            NPC[B].Location.X = NPC[Player[A].HoldingNPC].Location.X + 32;
                        else
                            NPC[B].Location.X = NPC[Player[A].HoldingNPC].Location.X - NPC[B].Location.Width;
                        NPC[B].Location.Y = NPC[Player[A].HoldingNPC].Location.Y;
                    }
                }
            }
        }
        else
        {
            NPC[Player[A].HoldingNPC].Location.SpeedX = 0;
            NPC[Player[A].HoldingNPC].Location.SpeedY = 0;
            NPC[Player[A].HoldingNPC].WallDeath = 5;
            if(NPC[Player[A].HoldingNPC].Type == 29)
            {
                NPCHit(Player[A].HoldingNPC, 3, Player[A].HoldingNPC);
            }
            if(NPCIsACoin[NPC[Player[A].HoldingNPC].Type] && Player[A].Controls.Down == false) // Smoke effect for coins
                NewEffect(10, NPC[Player[A].HoldingNPC].Location);
            if(Player[A].Controls.Up == true && !NPCIsACoin[NPC[Player[A].HoldingNPC].Type] && !(NPC[Player[A].HoldingNPC].Type == 17)) // Throw the npc up
            {
                if(NPCIsAShell[NPC[Player[A].HoldingNPC].Type] || NPC[Player[A].HoldingNPC].Type == 45 || NPC[Player[A].HoldingNPC].Type == 263)
                {
                    if(Player[A].Controls.Left == true || Player[A].Controls.Right == true) // Up and forward
                    {
                        NPC[Player[A].HoldingNPC].Location.SpeedX = Physics.NPCShellSpeed * Player[A].Direction;
                        NPC[Player[A].HoldingNPC].Location.SpeedY = -7;
                        tempLocation.Height = 0;
                        tempLocation.Width = 0;
                        tempLocation.Y = (Player[A].Location.Y + NPC[Player[A].HoldingNPC].Location.Y * 4) / 5;
                        tempLocation.X = (Player[A].Location.X + NPC[Player[A].HoldingNPC].Location.X * 4) / 5;
                        if(NPC[Player[A].HoldingNPC].Type != 263)
                            NewEffect(132, tempLocation);
                    }
                    else
                    {
                        NPC[Player[A].HoldingNPC].Location.SpeedY = -Physics.NPCShellSpeedY;
                        tempLocation.Height = 0;
                        tempLocation.Width = 0;
                        tempLocation.Y = (Player[A].Location.Y + NPC[Player[A].HoldingNPC].Location.Y * 4) / 5;
                        tempLocation.X = (Player[A].Location.X + NPC[Player[A].HoldingNPC].Location.X * 4) / 5;
                        if(NPC[Player[A].HoldingNPC].Type != 263)
                            NewEffect(132, tempLocation);
                    }
                }
                else
                {
                    if(Player[A].Controls.Left == true || Player[A].Controls.Right == true) // Up and forward
                    {
                        if(Player[A].Character == 3 || Player[A].Character == 4)
                        {
                            NPC[Player[A].HoldingNPC].Location.SpeedX = 5 * Player[A].Direction;
                            NPC[Player[A].HoldingNPC].Location.SpeedY = -6;
                        }
                        else
                        {
                            NPC[Player[A].HoldingNPC].Location.SpeedY = -8;
                            NPC[Player[A].HoldingNPC].Location.SpeedX = 3 * Player[A].Direction;
                        }
                    }
                    else
                    {
                        NPC[Player[A].HoldingNPC].Location.SpeedY = -10;
                        if(Player[A].Character == 3) // peach
                            NPC[Player[A].HoldingNPC].Location.SpeedY = -9;
                    }
                }
                if(NPCIsVeggie[NPC[Player[A].HoldingNPC].Type] == true || NPC[Player[A].HoldingNPC].Type == 19 || NPC[Player[A].HoldingNPC].Type == 20 || NPC[Player[A].HoldingNPC].Type == 25 || NPC[Player[A].HoldingNPC].Type == 129 || NPC[Player[A].HoldingNPC].Type == 130 || NPC[Player[A].HoldingNPC].Type == 131 || NPC[Player[A].HoldingNPC].Type == 132 || NPC[Player[A].HoldingNPC].Type == 134 || NPC[Player[A].HoldingNPC].Type == 135 || NPC[Player[A].HoldingNPC].Type == 154 || NPC[Player[A].HoldingNPC].Type == 155 || NPC[Player[A].HoldingNPC].Type == 156 || NPC[Player[A].HoldingNPC].Type == 157 || NPC[Player[A].HoldingNPC].Type == 40 || NPC[Player[A].HoldingNPC].Type == 240 || NPC[Player[A].HoldingNPC].Type == 13 || NPC[Player[A].HoldingNPC].Type == 265 || NPC[Player[A].HoldingNPC].Type == 288 || NPC[Player[A].HoldingNPC].Type == 291)
                    PlaySound(75);
                else
                    PlaySound(9);
                NPC[Player[A].HoldingNPC].Projectile = true;
            }
            else if(Player[A].Controls.Down == true && NPC[Player[A].HoldingNPC].Type != 17) // Drop
            {
                tempBool = false;
                if((Player[A].Direction == 1 && Player[A].Location.SpeedX > 3) || (Player[A].Direction == -1 && Player[A].Location.SpeedX < -3))
                    tempBool = true;
                if(tempBool == true && NPC[Player[A].HoldingNPC].Type == 195)
                {
                    Player[A].Location.SpeedX = 0;
                    NPC[Player[A].HoldingNPC].Location.SpeedX = Physics.NPCShellSpeed * Player[A].Direction;
                    NPC[Player[A].HoldingNPC].Projectile = true;
                    NPC[Player[A].HoldingNPC].CantHurt = 0;
                    NPC[Player[A].HoldingNPC].CantHurtPlayer = 0;
                    NPC[Player[A].HoldingNPC].HoldingPlayer = 0;
                    PlaySound(9);
                    NewEffect(73, newLoc(NPC[Player[A].HoldingNPC].Location.X, NPC[Player[A].HoldingNPC].Location.Y + NPC[Player[A].HoldingNPC].Location.Height - 16));
                    NPC[Player[A].HoldingNPC].Location.X = Player[A].Location.X + Player[A].Location.Width / 2.0 - NPC[Player[A].HoldingNPC].Location.Width / 2.0;
                    NPC[Player[A].HoldingNPC].Location.Y = Player[A].Location.Y + Player[A].Location.Height - NPC[Player[A].HoldingNPC].Location.Height;
                    Player[A].Location.Y = NPC[Player[A].HoldingNPC].Location.Y - Player[A].Location.Height;
                    NPC[Player[A].HoldingNPC].Location.SpeedY = Player[A].Location.SpeedY;
                    Player[A].StandingOnNPC = Player[A].HoldingNPC;
                    Player[A].HoldingNPC = 0;
                    Player[A].ShellSurf = true;
                    Player[A].Jump = 0;
                    Player[A].Location.SpeedY = 10;
                }
                else
                {
                    if(Player[A].Direction == 1)
                        NPC[Player[A].HoldingNPC].Location.X = Player[A].Location.X + Player[A].Location.Width + 0.1;
                    else
                        NPC[Player[A].HoldingNPC].Location.X = Player[A].Location.X - NPC[Player[A].HoldingNPC].Location.Width - 0.1;
                    NPC[Player[A].HoldingNPC].Projectile = false;
                    if(NPC[Player[A].HoldingNPC].Type == 272)
                        NPC[Player[A].HoldingNPC].Projectile = true;
                    if(Player[A].StandingOnNPC != 0)
                        NPC[Player[A].HoldingNPC].Location.Y = NPC[Player[A].HoldingNPC].Location.Y + NPC[Player[A].StandingOnNPC].Location.SpeedY;
                }
                if(NPC[Player[A].HoldingNPC].Type == 13 || NPC[Player[A].HoldingNPC].Type == 265 || NPC[Player[A].HoldingNPC].Type == 291)
                {
                    NPC[Player[A].HoldingNPC].Location.X = Player[A].Location.X + Player[A].Location.Width / 2.0 - NPC[Player[A].HoldingNPC].Location.Width / 2.0;
                    if(Player[A].State == 1)
                        NPC[Player[A].HoldingNPC].Location.Y = Player[A].Location.Y - NPC[Player[A].HoldingNPC].Location.Height;
                    else
                        NPC[Player[A].HoldingNPC].Location.Y = Player[A].Location.Y - NPC[Player[A].HoldingNPC].Location.Height + 6;
                    PlaySound(75);
                    NPC[Player[A].HoldingNPC].Location.SpeedX = 0;
                    NPC[Player[A].HoldingNPC].Location.SpeedY = 20;
                }
            }
            else if(!NPCIsAShell[NPC[Player[A].HoldingNPC].Type] &&
                    NPC[Player[A].HoldingNPC].Type != 45 &&
                    NPC[Player[A].HoldingNPC].Type != 263 &&
                    !NPCIsACoin[NPC[Player[A].HoldingNPC].Type]) // if not a shell or a coin the kick it up and forward
            {
            // peach
                if(Player[A].Character == 3)
                {
                    if(NPC[Player[A].HoldingNPC].Type == 13 || NPC[Player[A].HoldingNPC].Type == 265 || (Player[A].Location.SpeedY != 0 && Player[A].StandingOnNPC == 0 && Player[A].Slope == 0))
                    {
                        NPC[Player[A].HoldingNPC].Location.SpeedX = 5 * Player[A].Direction + Player[A].Location.SpeedX * 0.3;
                        NPC[Player[A].HoldingNPC].Location.SpeedY = 3;
                    }
                    else
                    {
                        NPC[Player[A].HoldingNPC].Location.SpeedX = 5 * Player[A].Direction + Player[A].Location.SpeedX * 0.3;
                        NPC[Player[A].HoldingNPC].Location.SpeedY = 0;
                    }
            // toad
                }
                else if(Player[A].Character == 4)
                {
                    if(NPC[Player[A].HoldingNPC].Type == 13 || NPC[Player[A].HoldingNPC].Type == 265 || (Player[A].Location.SpeedY != 0 && Player[A].StandingOnNPC == 0 && Player[A].Slope == 0))
                    {
                        NPC[Player[A].HoldingNPC].Location.SpeedX = 6 * Player[A].Direction + Player[A].Location.SpeedX * 0.4;
                        NPC[Player[A].HoldingNPC].Location.SpeedY = 3.5;
                    }
                    else
                    {
                        NPC[Player[A].HoldingNPC].Location.SpeedX = 6 * Player[A].Direction + Player[A].Location.SpeedX * 0.4;
                        NPC[Player[A].HoldingNPC].Location.SpeedY = 0;
                        NPC[Player[A].HoldingNPC].CantHurt = NPC[Player[A].HoldingNPC].CantHurt * 2;
                    }
                }
                else
                {
                    NPC[Player[A].HoldingNPC].Location.SpeedX = 5 * Player[A].Direction;
                    NPC[Player[A].HoldingNPC].Location.SpeedY = -6;
                }
                NPC[Player[A].HoldingNPC].Projectile = true;
                if(NPCIsVeggie[NPC[Player[A].HoldingNPC].Type] == true || NPC[Player[A].HoldingNPC].Type == 19 || NPC[Player[A].HoldingNPC].Type == 20 || NPC[Player[A].HoldingNPC].Type == 25 || NPC[Player[A].HoldingNPC].Type == 129 || NPC[Player[A].HoldingNPC].Type == 130 || NPC[Player[A].HoldingNPC].Type == 131 || NPC[Player[A].HoldingNPC].Type == 132 || NPC[Player[A].HoldingNPC].Type == 134 || NPC[Player[A].HoldingNPC].Type == 135 || NPC[Player[A].HoldingNPC].Type == 154 || NPC[Player[A].HoldingNPC].Type == 155 || NPC[Player[A].HoldingNPC].Type == 156 || NPC[Player[A].HoldingNPC].Type == 157 || NPC[Player[A].HoldingNPC].Type == 40 || NPC[Player[A].HoldingNPC].Type == 240 || NPC[Player[A].HoldingNPC].Type == 13 || NPC[Player[A].HoldingNPC].Type == 265 || NPC[Player[A].HoldingNPC].Type == 288 || NPC[Player[A].HoldingNPC].Type == 291)
                    PlaySound(75);
                else if(NPC[Player[A].HoldingNPC].Type == 17)
                    PlaySound(22);
                else
                    PlaySound(9);
            }
            else if(NPCIsAShell[NPC[Player[A].HoldingNPC].Type] == true)
            {
                NPC[Player[A].HoldingNPC].Location.SpeedY = 0;
                NPC[Player[A].HoldingNPC].Location.SpeedX = 0;
                NPC[Player[A].HoldingNPC].HoldingPlayer = 0;
                NPC[Player[A].HoldingNPC].CantHurt = 0;
                NPC[Player[A].HoldingNPC].CantHurtPlayer = 0;
                NPC[Player[A].HoldingNPC].Projectile = false;
                NPCHit(Player[A].HoldingNPC, 1, A);
                tempLocation.Height = 0;
                tempLocation.Width = 0;
                tempLocation.Y = (Player[A].Location.Y + NPC[Player[A].HoldingNPC].Location.Y * 4) / 5;
                tempLocation.X = (Player[A].Location.X + NPC[Player[A].HoldingNPC].Location.X * 4) / 5;
                NewEffect(132, tempLocation);
            }
            else if(NPC[Player[A].HoldingNPC].Type == 263)
            {
                 PlaySound(9);
                NPC[Player[A].HoldingNPC].Location.SpeedX = Physics.NPCShellSpeed * Player[A].Direction;
                NPC[Player[A].HoldingNPC].CantHurt = Physics.NPCCanHurtWait;
                NPC[Player[A].HoldingNPC].CantHurtPlayer = A;
                NPC[Player[A].HoldingNPC].Projectile = true;
            }
            if(NPC[Player[A].HoldingNPC].Type == 134 && NPC[Player[A].HoldingNPC].Location.SpeedX != 0)
            {
                NPC[Player[A].HoldingNPC].Location.SpeedX = NPC[Player[A].HoldingNPC].Location.SpeedX + Player[A].Location.SpeedX * 0.5;
                if(Player[A].StandingOnNPC != 0)
                    NPC[Player[A].HoldingNPC].Location.SpeedX = NPC[Player[A].HoldingNPC].Location.SpeedX + NPC[Player[A].StandingOnNPC].Location.SpeedX;
            }
            if(NPC[Player[A].HoldingNPC].Type == 13 && NPC[Player[A].HoldingNPC].Special == 4) // give toad fireballs a little spunk
            {
                if(NPC[Player[A].HoldingNPC].Location.SpeedY < 0)
                    NPC[Player[A].HoldingNPC].Location.SpeedY = NPC[Player[A].HoldingNPC].Location.SpeedY * 1.1;
            }
            if(NPC[Player[A].HoldingNPC].Type == 291)
            {
                if(Player[A].Location.SpeedX != 0 && NPC[Player[A].HoldingNPC].Location.SpeedX != 0)
                    NPC[Player[A].HoldingNPC].Location.SpeedX = NPC[Player[A].HoldingNPC].Location.SpeedX + Player[A].Location.SpeedX * 0.5;
            }

        if(NPC[Player[A].HoldingNPC].Type == 292)
        {
            NPC[Player[A].HoldingNPC].Special5 = A;
            NPC[Player[A].HoldingNPC].Special6 = Player[A].Direction;
            NPC[Player[A].HoldingNPC].Location.SpeedY = -8;
            NPC[Player[A].HoldingNPC].Location.SpeedX = 12 * Player[A].Direction + Player[A].Location.SpeedX;
            NPC[Player[A].HoldingNPC].Projectile = true;
        }


            NPC[Player[A].HoldingNPC].HoldingPlayer = 0;
            Player[A].HoldingNPC = 0;
        }
    }

    if(LayerNPC > 0)
    {
        for(B = 1; B <= maxLayers; B++)
        {
            if(!Layer[B].Name.empty())
            {
                if(Layer[B].Name == NPC[LayerNPC].AttLayer)
                {
                    if(NPC[LayerNPC].Location.X - lyrX == 0.0 && NPC[LayerNPC].Location.Y - lyrY == 0.0)
                    {
                        if(Layer[B].SpeedX != 0.0f || Layer[B].SpeedY != 0.0f)
                        {
                            Layer[B].EffectStop = true;
                            Layer[B].SpeedX = 0;
                            Layer[B].SpeedY = 0;
                            for(C = 1; C <= numBlock; C++)
                            {
                                if(Block[C].Layer == Layer[B].Name)
                                {
                                    Block[C].Location.SpeedX = Layer[B].SpeedX;
                                    Block[C].Location.SpeedY = Layer[B].SpeedY;
                                }
                            }
                            for(C = 1; C <= numNPCs; C++)
                            {
                                if(NPC[C].Layer == Layer[B].Name)
                                {
                                    if(NPCIsAVine[NPC[C].Type] || NPC[C].Type == 91)
                                    {
                                        NPC[C].Location.SpeedX = 0;
                                        NPC[C].Location.SpeedY = 0;
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        Layer[B].EffectStop = false;
                        Layer[B].SpeedX = NPC[LayerNPC].Location.X - lyrX;
                        Layer[B].SpeedY = NPC[LayerNPC].Location.Y - lyrY;
                    }
                }
            }
            else
                break;
        }
    }
}

void LinkFrame(int A)
{
    Location_t tempLocation;

    // Hurt frame
    if(Player[A].FrameCount == -10)
    {
        if(Player[A].SwordPoke == 0)
        {
            if(Player[A].Location.SpeedY == 0.0 ||
               Player[A].StandingOnNPC != 0 ||
               Player[A].Slope != 0 || Player[A].Wet > 0 ||
               Player[A].Immune == 0) // Hurt Frame
               Player[A].FrameCount = 0;
            else
            {
                Player[A].Frame = 11;
                return;
            }
        }
        else
            Player[A].FrameCount = 0;
    }
    if(Player[A].Stoned == true)
    {
        Player[A].Frame = 12;
        if(Player[A].Location.SpeedX != 0.0)
        {
            if(Player[A].Location.SpeedY == 0.0 || Player[A].Slope > 0 || Player[A].StandingOnNPC != 0)
            {
                if(Player[A].SlideCounter <= 0)
                {
                    Player[A].SlideCounter = 2 + dRand() * 2;
                    tempLocation.Y = Player[A].Location.Y + Player[A].Location.Height - 5;
                    tempLocation.X = Player[A].Location.X + Player[A].Location.Width / 2.0 - 4;
                    NewEffect(74, tempLocation, 1, 0, ShadowMode);
                }
            }
        }
        return;
    }
    if(LevelSelect == false && Player[A].Effect == 0 && Player[A].FireBallCD == 0)
    {
        if(Player[A].Controls.Left == true)
            Player[A].Direction = -1;
        if(Player[A].Controls.Right == true)
            Player[A].Direction = 1;
    }
    if(Player[A].Fairy == true)
        return;
    if(Player[A].SwordPoke < 0) // Drawing back
    {
        if(Player[A].Duck == false)
            Player[A].Frame = 6;
        else
            Player[A].Frame = 8;
    }
    else if(Player[A].SwordPoke > 0) // Stabbing
    {
        if(Player[A].Duck == false)
            Player[A].Frame = 7;
        else
            Player[A].Frame = 8;
    }
    else if(Player[A].Mount == 2) // Clown Car
    {
        Player[A].Frame = 1;
        Player[A].MountFrame = SpecialFrame[2];
        if(Player[A].Direction == 1)
            Player[A].MountFrame = Player[A].MountFrame + 4;
    }
    else if(Player[A].Duck == true) // Ducking
        Player[A].Frame = 5;
    else if(Player[A].WetFrame == true && Player[A].Location.SpeedY != 0.0 && Player[A].Slope == 0 && Player[A].StandingOnNPC == 0 && Player[A].Duck == false && Player[A].Quicksand == 0) // Link is swimming
    {
        if(Player[A].Location.SpeedY < 0.5 || Player[A].Frame != 3)
        {
            if(Player[A].Frame != 1 && Player[A].Frame != 2 && Player[A].Frame != 3 && Player[A].Frame != 4)
                Player[A].FrameCount = 6;
            Player[A].FrameCount = Player[A].FrameCount + 1;
            if(Player[A].FrameCount < 6)
                Player[A].Frame = 3;
            else if(Player[A].FrameCount < 12)
                Player[A].Frame = 2;
            else if(Player[A].FrameCount < 18)
                Player[A].Frame = 3;
            else if(Player[A].FrameCount < 24)
                Player[A].Frame = 1;
            else
            {
                Player[A].Frame = 3;
                Player[A].FrameCount = 0;
            }
        }
        else
            Player[A].Frame = 3;
    }
    else if(Player[A].Location.SpeedY != 0.0 && Player[A].StandingOnNPC == 0 && Player[A].Slope == 0 && !(Player[A].Quicksand > 0 && Player[A].Location.SpeedY > 0)) // Jumping/falling
    {
        if(Player[A].Location.SpeedY < 0)
        {
            if(Player[A].Controls.Up == true)
                Player[A].Frame = 10;
            else
                Player[A].Frame = 5;
        }
        else
        {
            if(Player[A].Controls.Down == true)
                Player[A].Frame = 9;
            else
                Player[A].Frame = 3;
        }
    }
    else if(Player[A].Location.SpeedX == 0.0 || (Player[A].Slippy == true && Player[A].Controls.Left == false && Player[A].Controls.Right == false)) // Standing
        Player[A].Frame = 1;
    else // Running
    {
        Player[A].FrameCount = Player[A].FrameCount + 1;
        if(Player[A].Location.SpeedX > Physics.PlayerWalkSpeed - 1.5 || Player[A].Location.SpeedX < -Physics.PlayerWalkSpeed + 1.5)
            Player[A].FrameCount = Player[A].FrameCount + 1;
        if(Player[A].Location.SpeedX > Physics.PlayerWalkSpeed || Player[A].Location.SpeedX < -Physics.PlayerWalkSpeed)
            Player[A].FrameCount = Player[A].FrameCount + 1;
        if(Player[A].Location.SpeedX > Physics.PlayerWalkSpeed + 1 || Player[A].Location.SpeedX < -Physics.PlayerWalkSpeed - 1)
            Player[A].FrameCount = Player[A].FrameCount + 1;
        if(Player[A].Location.SpeedX > Physics.PlayerWalkSpeed + 2 || Player[A].Location.SpeedX < -Physics.PlayerWalkSpeed - 2)
            Player[A].FrameCount = Player[A].FrameCount + 1;
        if(Player[A].FrameCount >= 8)
        {
            Player[A].FrameCount = 0;
            Player[A].Frame = Player[A].Frame - 1;
        }
        if(Player[A].Frame <= 0)
            Player[A].Frame = 4;
        else if(Player[A].Frame >= 5)
            Player[A].Frame = 1;
        if(Player[A].Location.SpeedX >= Physics.PlayerRunSpeed * 0.9 || Player[A].Location.SpeedX <= -Physics.PlayerRunSpeed * 0.9)
        {
            if(Player[A].SlideCounter <= 0)
            {
                PlaySound(86);
                Player[A].SlideCounter = 2 + dRand() * 2;
                tempLocation.Y = Player[A].Location.Y + Player[A].Location.Height - 4;
                if(Player[A].Location.SpeedX < 0)
                    tempLocation.X = Player[A].Location.X + Player[A].Location.Width / 2.0 - 6 - 4;
                else
                    tempLocation.X = Player[A].Location.X + Player[A].Location.Width / 2.0 + 6 - 4;
                NewEffect(74, tempLocation, 1, 0, ShadowMode);
            }
        }
    }
}

void PlayerEffects(int A)
{
    int B = 0;
    float C = 0;
    float D = 0;
    bool tempBool = false;
    Location_t tempLocation;
    if(Player[A].Effect != 8 && Player[A].Fairy == true)
    {
        Player[A].Fairy = false;
        SizeCheck(A);
    }
    Player[A].TailCount = 0;
    Player[A].Pinched1 = 0;
    Player[A].Pinched2 = 0;
    Player[A].Pinched3 = 0;
    Player[A].Pinched4 = 0;
    Player[A].NPCPinched = 0;
    Player[A].SwordPoke = 0;
    if(Player[A].YoshiBlue == false && Player[A].Effect != 500)
    {
        Player[A].CanFly = false;
        Player[A].CanFly2 = false;
        Player[A].RunCount = 0;
    }
    Player[A].Immune2 = false;
    if(Player[A].Effect == 1) // Player growing effect
    {

        Player[A].Frame = 1;
        Player[A].Effect2 = Player[A].Effect2 + 1;
        if(Player[A].Effect2 / 5 == static_cast<int>(floor(static_cast<double>(Player[A].Effect2 / 5))))
        {
            if(Player[A].State == 1)
            {
                Player[A].State = 2;
                if(Player[A].Mount == 0)
                {
                    Player[A].Location.X = Player[A].Location.X - Physics.PlayerWidth[Player[A].Character][2] * 0.5 + Physics.PlayerWidth[Player[A].Character][1] * 0.5;
                    Player[A].Location.Y = Player[A].Location.Y - Physics.PlayerHeight[Player[A].Character][2] + Physics.PlayerHeight[Player[A].Character][1];
                    Player[A].Location.Width = Physics.PlayerWidth[Player[A].Character][Player[A].State];
                    Player[A].Location.Height = Physics.PlayerHeight[Player[A].Character][Player[A].State];
                }
                else if(Player[A].Mount == 3)
                {
                    YoshiHeight(A);
                }
                else if(Player[A].Character == 2 && Player[A].Mount != 2)
                {
                    Player[A].Location.Y = Player[A].Location.Y - Physics.PlayerHeight[2][2] + Physics.PlayerHeight[1][2];
                    Player[A].Location.Height = Physics.PlayerHeight[Player[A].Character][Player[A].State];
                }
            }
            else
            {
                Player[A].State = 1;
                if(Player[A].Mount == 0)
                {
                    Player[A].Location.X = Player[A].Location.X - Physics.PlayerWidth[Player[A].Character][1] * 0.5 + Physics.PlayerWidth[Player[A].Character][2] * 0.5;
                    Player[A].Location.Y = Player[A].Location.Y - Physics.PlayerHeight[Player[A].Character][1] + Physics.PlayerHeight[Player[A].Character][2];
                    Player[A].Location.Width = Physics.PlayerWidth[Player[A].Character][Player[A].State];
                    Player[A].Location.Height = Physics.PlayerHeight[Player[A].Character][1];
                }
                else if(Player[A].Mount == 3)
                {
                    YoshiHeight(A);
                }
                else if(Player[A].Character == 2 && Player[A].Mount != 2)
                {
                    Player[A].Location.Y = Player[A].Location.Y - Physics.PlayerHeight[1][2] + Physics.PlayerHeight[2][2];
                    Player[A].Location.Height = Physics.PlayerHeight[1][2];
                }
            }
        }
        if(Player[A].Effect2 >= 50 && Player[A].State == 2)
        {
            Player[A].Immune = Player[A].Immune + 50;
            Player[A].Immune2 = true;
            Player[A].Effect = 0;
            Player[A].Effect2 = 0;
            Player[A].StandUp = true;
        }
    }
    else if(Player[A].Effect == 2) // Player shrinking effect
    {
        if(Player[A].Duck == true)
        {
            Player[A].StandUp = true; // Fixes a block collision bug
            Player[A].Duck = false;
            Player[A].Location.Height = Physics.PlayerHeight[Player[A].Character][Player[A].State];
            Player[A].Location.Y = Player[A].Location.Y - Physics.PlayerHeight[Player[A].Character][Player[A].State] + Physics.PlayerDuckHeight[Player[A].Character][Player[A].State];
        }
        Player[A].Frame = 1;
        Player[A].Effect2 = Player[A].Effect2 + 1;
        if(Player[A].Effect2 / 5 == static_cast<int>(floor(static_cast<double>(Player[A].Effect2 / 5))))
        {
            if(Player[A].State == 1)
            {
                Player[A].State = 2;
                if(Player[A].Mount == 3)
                {
                    YoshiHeight(A);
                }
                else if(!(Player[A].Mount == 2))
                {
                    Player[A].Location.X = Player[A].Location.X - Physics.PlayerWidth[Player[A].Character][2] * 0.5 + Physics.PlayerWidth[Player[A].Character][1] * 0.5;
                    Player[A].Location.Y = Player[A].Location.Y - Physics.PlayerHeight[Player[A].Character][2] + Physics.PlayerHeight[Player[A].Character][1];
                    Player[A].Location.Width = Physics.PlayerWidth[Player[A].Character][Player[A].State];
                    Player[A].Location.Height = Physics.PlayerHeight[Player[A].Character][Player[A].State];
                }
            }
            else
            {
                Player[A].State = 1;
                if(Player[A].Mount == 3)
                {
                    YoshiHeight(A);
                }
                else if(!(Player[A].Mount == 2))
                {
                    Player[A].Location.X = Player[A].Location.X - Physics.PlayerWidth[Player[A].Character][1] * 0.5 + Physics.PlayerWidth[Player[A].Character][2] * 0.5;
                    Player[A].Location.Y = Player[A].Location.Y - Physics.PlayerHeight[Player[A].Character][1] + Physics.PlayerHeight[Player[A].Character][2];
                    Player[A].Location.Width = Physics.PlayerWidth[Player[A].Character][Player[A].State];
                    Player[A].Location.Height = Physics.PlayerHeight[Player[A].Character][1];
                }
            }
        }
        if(Player[A].Effect2 >= 50)
        {
            if(Player[A].State == 2)
            {
                Player[A].State = 1;
                if(!(Player[A].Mount == 2))
                {
                    Player[A].Location.X = Player[A].Location.X - Physics.PlayerWidth[Player[A].Character][1] * 0.5 + Physics.PlayerWidth[Player[A].Character][2] * 0.5;
                    Player[A].Location.Y = Player[A].Location.Y - Physics.PlayerHeight[Player[A].Character][1] + Physics.PlayerHeight[Player[A].Character][2];
                    Player[A].Location.Width = Physics.PlayerWidth[Player[A].Character][Player[A].State];
                    Player[A].Location.Height = Physics.PlayerHeight[Player[A].Character][Player[A].State];
                }
            }
            Player[A].Immune = 150;
            Player[A].Immune2 = true;
            Player[A].Effect = 0;
            Player[A].Effect2 = 0;
            // If numPlayers <= 2 Then DropBonus A
        }
    }
    else if(Player[A].Effect == 227) // Player losing firepower
    {
        if(Player[A].Duck == true)
        {
            Player[A].StandUp = true; // Fixes a block collision bug
            Player[A].Duck = false;
            Player[A].Location.Height = Physics.PlayerHeight[Player[A].Character][Player[A].State];
            Player[A].Location.Y = Player[A].Location.Y - Physics.PlayerHeight[Player[A].Character][Player[A].State] + Physics.PlayerDuckHeight[Player[A].Character][Player[A].State];
        }
        Player[A].Frame = 1;
        Player[A].Effect2 = Player[A].Effect2 + 1;
        if(Player[A].Effect2 / 5 == static_cast<int>(floor(static_cast<double>(Player[A].Effect2 / 5))))
        {
            if(Player[A].State == 2)
                Player[A].State = 3;
            else
                Player[A].State = 2;
        }
        if(Player[A].Effect2 >= 50)
        {
            if(Player[A].State == 3)
                Player[A].State = 2;
            Player[A].Immune = 150;
            Player[A].Immune2 = true;
            Player[A].Effect = 0;
            Player[A].Effect2 = 0;
            // If numPlayers <= 2 Then DropBonus A
        }
    }
    else if(Player[A].Effect == 228) // Player losing icepower
    {
        if(Player[A].Duck)
        {
            Player[A].StandUp = true; // Fixes a block collision bug
            Player[A].Duck = false;
            Player[A].Location.Height = Physics.PlayerHeight[Player[A].Character][Player[A].State];
            Player[A].Location.Y = Player[A].Location.Y - Physics.PlayerHeight[Player[A].Character][Player[A].State] + Physics.PlayerDuckHeight[Player[A].Character][Player[A].State];
        }

        Player[A].Frame = 1;
        Player[A].Effect2 = Player[A].Effect2 + 1;

        if(fEqual(Player[A].Effect2 / 5, std::floor(Player[A].Effect2 / 5)))
        {
            if(Player[A].State == 2)
                Player[A].State = 7;
            else
                Player[A].State = 2;
        }

        if(Player[A].Effect2 >= 50)
        {
            if(Player[A].State == 7)
                Player[A].State = 2;
            Player[A].Immune = 150;
            Player[A].Immune2 = true;
            Player[A].Effect = 0;
            Player[A].Effect2 = 0;
            // If numPlayers <= 2 Then DropBonus A
        }
    }
    else if(Player[A].Effect == 3) // Warp effect
    {
        Player[A].SpinJump = false;
        Player[A].TailCount = 0;
        Player[A].Location.SpeedY = 0;
        if(Player[A].Effect2 == 0.0)
        {
            if(Warp[Player[A].Warp].Direction == 3)
            {
                Player[A].Location.Y = Player[A].Location.Y + 1;
                Player[A].Location.X = Warp[Player[A].Warp].Entrance.X + Warp[Player[A].Warp].Entrance.Width / 2.0 - Player[A].Location.Width / 2.0;
                if(Player[A].Location.Y > Warp[Player[A].Warp].Entrance.Y + Warp[Player[A].Warp].Entrance.Height + 8)
                    Player[A].Effect2 = 1;
                if(Player[A].Mount == 0)
                    Player[A].Frame = 15;
                if(Player[A].HoldingNPC > 0)
                {
                    NPC[Player[A].HoldingNPC].Location.Y = Player[A].Location.Y + Physics.PlayerGrabSpotY[Player[A].Character][Player[A].State] + 32 - NPC[Player[A].HoldingNPC].Location.Height;
                    NPC[Player[A].HoldingNPC].Location.X = Player[A].Location.X + Player[A].Location.Width / 2.0 - NPC[Player[A].HoldingNPC].Location.Width / 2.0;
                }
            }
            else if(Warp[Player[A].Warp].Direction == 1)
            {
                Player[A].Location.Y = Player[A].Location.Y - 1;
                Player[A].Location.X = Warp[Player[A].Warp].Entrance.X + Warp[Player[A].Warp].Entrance.Width / 2.0 - Player[A].Location.Width / 2.0;
                if(Player[A].Location.Y + Player[A].Location.Height + 8 < Warp[Player[A].Warp].Entrance.Y)
                    Player[A].Effect2 = 1;
                if(Player[A].HoldingNPC > 0)
                {
                    NPC[Player[A].HoldingNPC].Location.Y = Player[A].Location.Y + Physics.PlayerGrabSpotY[Player[A].Character][Player[A].State] + 32 - NPC[Player[A].HoldingNPC].Location.Height;
                    NPC[Player[A].HoldingNPC].Location.X = Player[A].Location.X + Player[A].Location.Width / 2.0 - NPC[Player[A].HoldingNPC].Location.Width / 2.0;
                }
                if(Player[A].Mount == 0)
                    Player[A].Frame = 15;
            }
            else if(Warp[Player[A].Warp].Direction == 2)
            {
                if(Player[A].Mount == 3)
                {
                    Player[A].Duck = true;
                    Player[A].Location.Height = 30;
                }
                Player[A].Direction = -1;
                Player[A].Location.Y = Warp[Player[A].Warp].Entrance.Y + Warp[Player[A].Warp].Entrance.Height - Player[A].Location.Height - 2;
                Player[A].Location.X = Player[A].Location.X - 0.5;
                if(Player[A].Location.X + Player[A].Location.Width + 8 < Warp[Player[A].Warp].Entrance.X)
                    Player[A].Effect2 = 1;
                if(Player[A].HoldingNPC > 0)
                {
                    NPC[Player[A].HoldingNPC].Location.Y = Player[A].Location.Y + Physics.PlayerGrabSpotY[Player[A].Character][Player[A].State] + 32 - NPC[Player[A].HoldingNPC].Location.Height;
                    if(Player[A].Direction > 0)
                        NPC[Player[A].HoldingNPC].Location.X = Player[A].Location.X + Physics.PlayerGrabSpotX[Player[A].Character][Player[A].State];
                    else
                        NPC[Player[A].HoldingNPC].Location.X = Player[A].Location.X + Player[A].Location.Width - Physics.PlayerGrabSpotX[Player[A].Character][Player[A].State] - NPC[Player[A].HoldingNPC].Location.Width;
                }
                Player[A].Location.SpeedX = -0.5;
                PlayerFrame(A);
                Player[A].Location.SpeedX = 0;
            }
            else if(Warp[Player[A].Warp].Direction == 4)
            {
                if(Player[A].Mount == 3)
                {
                    Player[A].Duck = true;
                    Player[A].Location.Height = 30;
                }
                Player[A].Direction = 1;
                Player[A].Location.Y = Warp[Player[A].Warp].Entrance.Y + Warp[Player[A].Warp].Entrance.Height - Player[A].Location.Height - 2;
                Player[A].Location.X = Player[A].Location.X + 0.5;
                if(Player[A].Location.X > Warp[Player[A].Warp].Entrance.X + Warp[Player[A].Warp].Entrance.Width + 8)
                    Player[A].Effect2 = 1;
                if(Player[A].HoldingNPC > 0)
                {
                    NPC[Player[A].HoldingNPC].Location.Y = Player[A].Location.Y + Physics.PlayerGrabSpotY[Player[A].Character][Player[A].State] + 32 - NPC[Player[A].HoldingNPC].Location.Height;
                    if(Player[A].Direction > 0)
                        NPC[Player[A].HoldingNPC].Location.X = Player[A].Location.X + Physics.PlayerGrabSpotX[Player[A].Character][Player[A].State];
                    else
                        NPC[Player[A].HoldingNPC].Location.X = Player[A].Location.X + Player[A].Location.Width - Physics.PlayerGrabSpotX[Player[A].Character][Player[A].State] - NPC[Player[A].HoldingNPC].Location.Width;
                }
                Player[A].Location.SpeedX = 0.5;
                PlayerFrame(A);
                Player[A].Location.SpeedX = 0;
            }
        }
        else if(fEqual(Player[A].Effect2, 1))
        {
            if(Warp[Player[A].Warp].NoYoshi == true)
            {
                if(OwedMount[A] == 0 && Player[A].Mount > 0 && Player[A].Mount != 2)
                {
                    OwedMount[A] = Player[A].Mount;
                    OwedMountType[A] = Player[A].MountType;
                }
                UnDuck(A);
                Player[A].Mount = 0;
                Player[A].MountType = 0;
                Player[A].MountOffsetY = 0;
                SizeCheck(A);
            }

            if(Warp[Player[A].Warp].Direction2 == 1)
            {
                Player[A].Location.X = Warp[Player[A].Warp].Exit.X + Warp[Player[A].Warp].Exit.Width / 2.0 - Player[A].Location.Width / 2.0;
                Player[A].Location.Y = Warp[Player[A].Warp].Exit.Y - Player[A].Location.Height - 8;
                if(Player[A].Mount == 0)
                    Player[A].Frame = 15;
                if(Player[A].HoldingNPC > 0)
                {
                    NPC[Player[A].HoldingNPC].Location.Y = Player[A].Location.Y + Physics.PlayerGrabSpotY[Player[A].Character][Player[A].State] + 32 - NPC[Player[A].HoldingNPC].Location.Height;
                    NPC[Player[A].HoldingNPC].Location.X = Player[A].Location.X + Player[A].Location.Width / 2.0 - NPC[Player[A].HoldingNPC].Location.Width / 2.0;
                }
            }
            else if(Warp[Player[A].Warp].Direction2 == 3)
            {
                Player[A].Location.X = Warp[Player[A].Warp].Exit.X + Warp[Player[A].Warp].Exit.Width / 2.0 - Player[A].Location.Width / 2.0;
                Player[A].Location.Y = Warp[Player[A].Warp].Exit.Y + Warp[Player[A].Warp].Exit.Height + 8;
                if(Player[A].Mount == 0)
                    Player[A].Frame = 15;
                if(Player[A].HoldingNPC > 0)
                {
                    NPC[Player[A].HoldingNPC].Location.Y = Player[A].Location.Y + Physics.PlayerGrabSpotY[Player[A].Character][Player[A].State] + 32 - NPC[Player[A].HoldingNPC].Location.Height;
                    NPC[Player[A].HoldingNPC].Location.X = Player[A].Location.X + Player[A].Location.Width / 2.0 - NPC[Player[A].HoldingNPC].Location.Width / 2.0;
                }
            }
            else if(Warp[Player[A].Warp].Direction2 == 2)
            {
                if(Player[A].Mount == 3)
                {
                    Player[A].Duck = true;
                    Player[A].Location.Height = 30;
                }
                Player[A].Location.X = Warp[Player[A].Warp].Exit.X - Player[A].Location.Width - 8;
                Player[A].Location.Y = Warp[Player[A].Warp].Exit.Y + Warp[Player[A].Warp].Exit.Height - Player[A].Location.Height - 2;
                if(Player[A].Mount == 0)
                    Player[A].Frame = 1;
                Player[A].Direction = 1;
                if(Player[A].HoldingNPC > 0)
                {
                    if(Player[A].State == 1)
                        Player[A].Frame = 5;
                    else
                        Player[A].Frame = 8;
                    NPC[Player[A].HoldingNPC].Location.Y = Player[A].Location.Y + Physics.PlayerGrabSpotY[Player[A].Character][Player[A].State] + 32 - NPC[Player[A].HoldingNPC].Location.Height;
                    Player[A].Direction = -1;
                    if(Player[A].Direction > 0)
                        NPC[Player[A].HoldingNPC].Location.X = Player[A].Location.X + Physics.PlayerGrabSpotX[Player[A].Character][Player[A].State];
                    else
                        NPC[Player[A].HoldingNPC].Location.X = Player[A].Location.X + Player[A].Location.Width - Physics.PlayerGrabSpotX[Player[A].Character][Player[A].State] - NPC[Player[A].HoldingNPC].Location.Width;
                }
            }
            else if(Warp[Player[A].Warp].Direction2 == 4)
            {
                if(Player[A].Mount == 3)
                {
                    Player[A].Duck = true;
                    Player[A].Location.Height = 30;
                }
                Player[A].Location.X = Warp[Player[A].Warp].Exit.X + Warp[Player[A].Warp].Exit.Width + 8;
                Player[A].Location.Y = Warp[Player[A].Warp].Exit.Y + Warp[Player[A].Warp].Exit.Height - Player[A].Location.Height - 2;
                if(Player[A].Mount == 0)
                    Player[A].Frame = 1;
                Player[A].Direction = -1;
                if(Player[A].HoldingNPC > 0)
                {
                    if(Player[A].State == 1)
                        Player[A].Frame = 5;
                    else
                        Player[A].Frame = 8;
                    Player[A].Direction = 1;
                    NPC[Player[A].HoldingNPC].Location.Y = Player[A].Location.Y + Physics.PlayerGrabSpotY[Player[A].Character][Player[A].State] + 32 - NPC[Player[A].HoldingNPC].Location.Height;
                    if(Player[A].Direction > 0)
                        NPC[Player[A].HoldingNPC].Location.X = Player[A].Location.X + Physics.PlayerGrabSpotX[Player[A].Character][Player[A].State];
                    else
                        NPC[Player[A].HoldingNPC].Location.X = Player[A].Location.X + Player[A].Location.Width - Physics.PlayerGrabSpotX[Player[A].Character][Player[A].State] - NPC[Player[A].HoldingNPC].Location.Width;
                }
            }

            Player[A].Effect2 = 100;
            if(Player[A].Duck)
            {
                if(Warp[Player[A].Warp].Direction2 == 1 || Warp[Player[A].Warp].Direction2 == 3)
                {
                    UnDuck(A);
                }
            }

            CheckSection(A);
            if(Player[A].HoldingNPC > 0)
            {
                CheckSectionNPC(Player[A].HoldingNPC);
            }

            if(numPlayers > 2/* && nPlay.Online == false*/)
            {
                for(B = 1; B <= numPlayers; B++)
                {
                    if(B != A)
                    {
                        if(Warp[Player[A].Warp].Direction2 != 3)
                            Player[B].Location.Y = Player[A].Location.Y + Player[A].Location.Height - Player[B].Location.Height;
                        else
                            Player[B].Location.Y = Player[A].Location.Y;
                        Player[B].Location.X = Player[A].Location.X + Player[A].Location.Width / 2.0 - Player[B].Location.Width / 2.0;
                        Player[B].Location.SpeedY = dRand() * 24 - 12;
                        Player[B].Effect = 8;
                        Player[B].Effect2 = 0;
                        CheckSection(B);
                        if(Player[B].HoldingNPC > 0)
                        {
                            CheckSectionNPC(Player[B].HoldingNPC);
                        }
                    }
                }
            }

            if(!Warp[Player[A].Warp].level.empty())
            {
                GoToLevel = Warp[Player[A].Warp].level;
                GoToLevelNoGameThing = Warp[Player[A].Warp].noEntranceScene;
                Player[A].Effect = 8;
                Player[A].Effect2 = 2970;
                ReturnWarp = Player[A].Warp;
                if(IsEpisodeIntro && NoMap)
                    ReturnWarpSaved = ReturnWarp;
                StartWarp = Warp[Player[A].Warp].LevelWarp;
            }
            else if(Warp[Player[A].Warp].MapWarp == true)
            {
                Player[A].Effect = 8;
                Player[A].Effect2 = 2970;
            }
        }
        else if(Player[A].Effect2 >= 100)
        {
            Player[A].Effect2 = Player[A].Effect2 + 1;
            if(Player[A].Effect2 >= 110)
            {
                Player[A].Effect2 = 2;
                PlaySound(17);
            }
        }
        else if(fEqual(Player[A].Effect2, 2))
        {
            if(Warp[Player[A].Warp].Direction2 == 1)
            {
                Player[A].Location.Y = Player[A].Location.Y + 1;
                if(Player[A].Location.Y >= Warp[Player[A].Warp].Exit.Y)
                    Player[A].Effect2 = 3;
                if(Player[A].HoldingNPC > 0)
                {
                    NPC[Player[A].HoldingNPC].Location.Y = Player[A].Location.Y + Physics.PlayerGrabSpotY[Player[A].Character][Player[A].State] + 32 - NPC[Player[A].HoldingNPC].Location.Height;
                    NPC[Player[A].HoldingNPC].Location.X = Player[A].Location.X + Player[A].Location.Width / 2.0 - NPC[Player[A].HoldingNPC].Location.Width / 2.0;
                }
                if(Player[A].Mount == 0)
                    Player[A].Frame = 15;
            }
            else if(Warp[Player[A].Warp].Direction2 == 3)
            {
                Player[A].Location.Y = Player[A].Location.Y - 1;
                if(Player[A].Location.Y + Player[A].Location.Height <= Warp[Player[A].Warp].Exit.Y + Warp[Player[A].Warp].Exit.Height)
                    Player[A].Effect2 = 3;
                if(Player[A].HoldingNPC > 0)
                {
                    NPC[Player[A].HoldingNPC].Location.Y = Player[A].Location.Y + Physics.PlayerGrabSpotY[Player[A].Character][Player[A].State] + 32 - NPC[Player[A].HoldingNPC].Location.Height;
                    NPC[Player[A].HoldingNPC].Location.X = Player[A].Location.X + Player[A].Location.Width / 2.0 - NPC[Player[A].HoldingNPC].Location.Width / 2.0;
                }
                if(Player[A].Mount == 0)
                    Player[A].Frame = 15;
            }
            else if(Warp[Player[A].Warp].Direction2 == 4)
            {
                Player[A].Location.X = Player[A].Location.X - 0.5;
                Player[A].Direction = -1;
                if(Player[A].Location.X + Player[A].Location.Width <= Warp[Player[A].Warp].Exit.X + Warp[Player[A].Warp].Exit.Width)
                    Player[A].Effect2 = 3;
                if(Player[A].HoldingNPC > 0)
                {
                    if(Player[A].Character >= 3) // peach/toad leaving a pipe
                    {
                        Player[A].Location.SpeedX = 1;
                        PlayerFrame(A);
                        NPC[Player[A].HoldingNPC].Location.Y = Player[A].Location.Y + Physics.PlayerGrabSpotY[Player[A].Character][Player[A].State] + 32 - NPC[Player[A].HoldingNPC].Location.Height;
                        if(Player[A].Direction < 0)
                            NPC[Player[A].HoldingNPC].Location.X = Player[A].Location.X + Physics.PlayerGrabSpotX[Player[A].Character][Player[A].State];
                        else
                            NPC[Player[A].HoldingNPC].Location.X = Player[A].Location.X + Player[A].Location.Width - Physics.PlayerGrabSpotX[Player[A].Character][Player[A].State] - NPC[Player[A].HoldingNPC].Location.Width;
                    }
                    else
                    {
                        Player[A].Direction = 1;
                        if(Player[A].State == 1)
                            Player[A].Frame = 5;
                        else
                            Player[A].Frame = 8;
                        NPC[Player[A].HoldingNPC].Location.Y = Player[A].Location.Y + Physics.PlayerGrabSpotY[Player[A].Character][Player[A].State] + 32 - NPC[Player[A].HoldingNPC].Location.Height;
                        if(Player[A].Direction > 0)
                            NPC[Player[A].HoldingNPC].Location.X = Player[A].Location.X + Physics.PlayerGrabSpotX[Player[A].Character][Player[A].State];
                        else
                            NPC[Player[A].HoldingNPC].Location.X = Player[A].Location.X + Player[A].Location.Width - Physics.PlayerGrabSpotX[Player[A].Character][Player[A].State] - NPC[Player[A].HoldingNPC].Location.Width;
                    }
                }
                else
                {
                    Player[A].Location.SpeedX = -0.5;
                    PlayerFrame(A);
                    Player[A].Location.SpeedX = 0;
                }
            }
            else if(Warp[Player[A].Warp].Direction2 == 2)
            {
                Player[A].Location.X = Player[A].Location.X + 0.5;
                Player[A].Direction = 1;
                if(Player[A].Location.X >= Warp[Player[A].Warp].Exit.X)
                    Player[A].Effect2 = 3;
                if(Player[A].HoldingNPC > 0)
                {
                    if(Player[A].Character >= 3) // peach/toad leaving a pipe
                    {
                        Player[A].Location.SpeedX = 1;
                        PlayerFrame(A);
                        NPC[Player[A].HoldingNPC].Location.Y = Player[A].Location.Y + Physics.PlayerGrabSpotY[Player[A].Character][Player[A].State] + 32 - NPC[Player[A].HoldingNPC].Location.Height;
                        if(Player[A].Direction < 0)
                            NPC[Player[A].HoldingNPC].Location.X = Player[A].Location.X + Physics.PlayerGrabSpotX[Player[A].Character][Player[A].State];
                        else
                            NPC[Player[A].HoldingNPC].Location.X = Player[A].Location.X + Player[A].Location.Width - Physics.PlayerGrabSpotX[Player[A].Character][Player[A].State] - NPC[Player[A].HoldingNPC].Location.Width;
                    }
                    else
                    {
                        Player[A].Direction = -1;
                        if(Player[A].State == 1)
                            Player[A].Frame = 5;
                        else
                            Player[A].Frame = 8;
                        NPC[Player[A].HoldingNPC].Location.Y = Player[A].Location.Y + Physics.PlayerGrabSpotY[Player[A].Character][Player[A].State] + 32 - NPC[Player[A].HoldingNPC].Location.Height;
                        if(Player[A].Direction > 0)
                            NPC[Player[A].HoldingNPC].Location.X = Player[A].Location.X + Physics.PlayerGrabSpotX[Player[A].Character][Player[A].State];
                        else
                            NPC[Player[A].HoldingNPC].Location.X = Player[A].Location.X + Player[A].Location.Width - Physics.PlayerGrabSpotX[Player[A].Character][Player[A].State] - NPC[Player[A].HoldingNPC].Location.Width;
                    }
                }
                else
                {
                    Player[A].Location.SpeedX = -0.5;
                    PlayerFrame(A);
                    Player[A].Location.SpeedX = 0;
                }
            }
        }
        else if(fEqual(Player[A].Effect2, 3))
        {
            if(Player[A].HoldingNPC > 0)
            {
                if(Warp[Player[A].Warp].Direction2 == 2 || Warp[Player[A].Warp].Direction2 == 4)
                {
                    if(Warp[Player[A].Warp].Direction2 == 2)
                        Player[A].Direction = 1;
                    else if(Warp[Player[A].Warp].Direction2 == 4)
                        Player[A].Direction = -1;
                    if(Player[A].State == 1)
                        Player[A].Frame = 5;
                    else
                        Player[A].Frame = 8;
                    if(Player[A].Controls.Run == false)
                        Player[A].Controls.Run = true;
                    PlayerGrabCode(A);
                }
            }
            Player[A].Effect = 0;
            Player[A].Effect2 = 0;
            Player[A].WarpCD = 20;
            Player[A].Location.SpeedY = 0;
            Player[A].CanJump = false;
            Player[A].CanAltJump = false;
            Player[A].Location.SpeedX = 0;
            Player[A].Bumped2 = 0;
            if(Player[A].HoldingNPC > 0)
                NPC[Player[A].HoldingNPC].Effect = 0;
            if(numPlayers > 2 /*&& nPlay.Online == false*/)
            {
                for(B = 1; B <= numPlayers; B++)
                {
                    if(B != A)
                    {
                        if(Warp[Player[A].Warp].Direction2 != 1)
                            Player[B].Location.Y = Player[A].Location.Y + Player[A].Location.Height - Player[B].Location.Height;
                        else
                            Player[B].Location.Y = Player[A].Location.Y;
                        Player[B].Location.X = Player[A].Location.X + Player[A].Location.Width / 2.0 - Player[B].Location.Width / 2.0;
                        Player[B].Location.SpeedY = dRand() * 24 - 12;
                        Player[B].Effect = 0;
                        Player[B].Effect2 = 0;
                        CheckSection(B);
                    }
                }
            }
        }
    }
    else if(Player[A].Effect == 7) // Door effect
    {
        if(Player[A].HoldingNPC > 0)
        {
            NPC[Player[A].HoldingNPC].Location.Y = Player[A].Location.Y + Physics.PlayerGrabSpotY[Player[A].Character][Player[A].State] + 32 - NPC[Player[A].HoldingNPC].Location.Height;
            NPC[Player[A].HoldingNPC].Location.X = Player[A].Location.X + Player[A].Location.Width / 2.0 - NPC[Player[A].HoldingNPC].Location.Width / 2.0;
        }
        Player[A].Effect2 = Player[A].Effect2 + 1;
        if(Player[A].Mount == 0 && Player[A].Character != 5)
            Player[A].Frame = 13;
        if(Player[A].Character == 5)
            Player[A].Frame = 1;
        if(Player[A].Effect2 >= 30)
        {
            if(Warp[Player[A].Warp].NoYoshi == true)
            {
                if(OwedMount[A] == 0 && Player[A].Mount > 0 && Player[A].Mount != 2)
                {
                    OwedMount[A] = Player[A].Mount;
                    OwedMountType[A] = Player[A].MountType;
                }
                Player[A].Mount = 0;
                Player[A].MountType = 0;
                SizeCheck(A);
                Player[A].MountOffsetY = 0;
                Player[A].Frame = 1;
            }
            Player[A].Location.X = Warp[Player[A].Warp].Exit.X + Warp[Player[A].Warp].Exit.Width / 2.0 - Player[A].Location.Width / 2.0;
            Player[A].Location.Y = Warp[Player[A].Warp].Exit.Y + Warp[Player[A].Warp].Exit.Height - Player[A].Location.Height;
            CheckSection(A);
            if(Player[A].HoldingNPC > 0)
            {
                if(Player[A].Controls.Run == false)
                    Player[A].Controls.Run = true;
                PlayerGrabCode(A);
            }
            Player[A].Effect = 0;
            Player[A].Effect2 = 0;
            Player[A].WarpCD = 40;

            if(!Warp[Player[A].Warp].level.empty())
            {
                GoToLevel = Warp[Player[A].Warp].level;
                GoToLevelNoGameThing = Warp[Player[A].Warp].noEntranceScene;
                Player[A].Effect = 8;
                Player[A].Effect2 = 3000;
                ReturnWarp = Player[A].Warp;
                if(IsEpisodeIntro && NoMap)
                    ReturnWarpSaved = ReturnWarp;
                StartWarp = Warp[Player[A].Warp].LevelWarp;
            }
            else if(Warp[Player[A].Warp].MapWarp == true)
            {
                Player[A].Effect = 8;
                Player[A].Effect2 = 2970;
            }

            if(numPlayers > 2 /*&& nPlay.Online == false*/)
            {
                for(B = 1; B <= numPlayers; B++)
                {
                    if(B != A)
                    {
                        Player[B].Location.Y = Player[A].Location.Y + Player[A].Location.Height - Player[B].Location.Height;
                        Player[B].Location.X = Player[A].Location.X + Player[A].Location.Width / 2.0 - Player[B].Location.Width / 2.0;
                        Player[B].Location.SpeedY = dRand() * 24 - 12;
                        CheckSection(B);
                        if(Player[B].HoldingNPC > 0)
                        {
                            if(Player[B].Direction > 0)
                                NPC[Player[B].HoldingNPC].Location.X = Player[B].Location.X + Physics.PlayerGrabSpotX[Player[B].Character][Player[B].State];
                            else
                                NPC[Player[B].HoldingNPC].Location.X = Player[B].Location.X + Player[B].Location.Width - Physics.PlayerGrabSpotX[Player[B].Character][Player[B].State] - NPC[Player[A].HoldingNPC].Location.Width;
                            NPC[Player[A].HoldingNPC].Location.Y = Player[A].Location.Y + Physics.PlayerGrabSpotY[Player[A].Character][Player[A].State] + 32 - NPC[Player[A].HoldingNPC].Location.Height;
                            NPC[Player[B].HoldingNPC].Section = Player[B].Section;
                        }
                    }
                }
            }
        }
    }
    else if(Player[A].Effect == 8) // Holding Pattern
    {
        if(Player[A].Effect2 < 0)
        {
            Player[A].Location.X = Player[-Player[A].Effect2].Location.X;
            Player[A].Location.Y = Player[-Player[A].Effect2].Location.Y;
            if(Player[-Player[A].Effect2].Dead == true)
                Player[A].Dead = true;
        }
        else if(Player[A].Effect2 <= 30)
        {
            Player[A].Effect2 = Player[A].Effect2 - 1;
            if(Player[A].Effect2 == 0.0)
            {
                Player[A].Effect = 0;
                Player[A].Effect2 = 0;
            }
        }
        else if(fEqual(Player[A].Effect2, 131))
        {
            tempBool = false;
            for(B = 1; B <= numPlayers; B++)
            {
                if(B != A && CheckCollision(Player[A].Location, Player[B].Location))
                    tempBool = true;
            }
            if(tempBool == false)
            {
                Player[A].Effect2 = 130;
                for(C = 1; C <= numBackground; C++)
                {
                    if(CheckCollision(Warp[Player[A].Warp].Exit, Background[C].Location))
                    {
                        if(Background[C].Type == 88)
                            NewEffect(54, Background[C].Location);
                        else if(Background[C].Type == 87)
                            NewEffect(55, Background[C].Location);
                        else if(Background[C].Type == 107)
                            NewEffect(59, Background[C].Location);
                    }
                }
                SoundPause[46] = 0;
                PlaySound(46);
            }
        }
        else if(Player[A].Effect2 <= 130)
        {
            Player[A].Effect2 = Player[A].Effect2 - 1;
            if(fEqual(Player[A].Effect2, 100))
            {
                Player[A].Effect = 0;
                Player[A].Effect2 = 0;
            }
        }
        else if(Player[A].Effect2 <= 300)
        {
            Player[A].Effect2 = Player[A].Effect2 - 1;
            if(fEqual(Player[A].Effect2, 200))
            {
                Player[A].Effect2 = 100;
                Player[A].Effect = 3;
            }
        }
        else if(Player[A].Effect2 <= 1000) // Start Wait
        {
            Player[A].Effect2 = Player[A].Effect2 - 1;
            if(fEqual(Player[A].Effect2, 900))
            {
                Player[A].Effect = 3;
                Player[A].Effect2 = 100;
                if(A == 2)
                {
                    Player[A].Effect = 8;
                    Player[A].Effect2 = 300;
                }
            }
        }
        else if(Player[A].Effect2 <= 2000) // Start Wait
        {
            Player[A].Effect2 = Player[A].Effect2 - 1;
            if(fEqual(Player[A].Effect2, 1900))
            {
                for(C = 1; C <= numBackground; C++)
                {
                    if(CheckCollision(Warp[Player[A].Warp].Exit, Background[C].Location))
                    {
                        if(Background[C].Type == 88)
                            NewEffect(54, Background[C].Location);
                        else if(Background[C].Type == 87)
                            NewEffect(55, Background[C].Location);
                        else if(Background[C].Type == 107)
                            NewEffect(59, Background[C].Location);
                    }
                }
                SoundPause[46] = 0;
                PlaySound(46);
                Player[A].Effect = 8;
                Player[A].Effect2 = 30;
                if(A == 2)
                {
                    Player[A].Effect = 8;
                    Player[A].Effect2 = 131;
                }
            }
        }
        else if(Player[A].Effect2 <= 3000) // warp wait
        {
            Player[A].Effect2 = Player[A].Effect2 - 1;
            if(fEqual(Player[A].Effect2, 2920))
            {
                if(Warp[Player[A].Warp].MapWarp == true)
                {
                    LevelBeatCode = 6;
                    if(!(Warp[Player[A].Warp].MapX == -1 && Warp[Player[A].Warp].MapY == -1))
                    {
                        WorldPlayer[1].Location.X = Warp[Player[A].Warp].MapX;
                        WorldPlayer[1].Location.Y = Warp[Player[A].Warp].MapY;
                        for(B = 1; B <= numWorldLevels; B++)
                        {
                            if(CheckCollision(WorldPlayer[1].Location, WorldLevel[B].Location) == true)
                            {
                                WorldLevel[B].Active = true;
                                curWorldLevel = B;
                            }
                        }
                    }
                }
                EndLevel = true;
                return;
            }
        }
    }
    else if(Player[A].Effect == 4) // Player got fire power
    {
        if(Player[A].Duck && Player[A].Character != 5)
        {
            UnDuck(A);
            Player[A].Frame = 1;
        }
        Player[A].Effect2 = Player[A].Effect2 + 1;
        if(fEqual(Player[A].Effect2 / 5, std::floor(Player[A].Effect2 / 5.0)))
        {
            if(Player[A].State == 1 && Player[A].Character != 5)
            {
                Player[A].State = 2;
                if(Player[A].Mount == 0)
                {
                    Player[A].Location.X = Player[A].Location.X - Physics.PlayerWidth[Player[A].Character][2] * 0.5 + Physics.PlayerWidth[Player[A].Character][1] * 0.5;
                    Player[A].Location.Y = Player[A].Location.Y - Physics.PlayerHeight[Player[A].Character][2] + Physics.PlayerHeight[Player[A].Character][1];
                    Player[A].Location.Width = Physics.PlayerWidth[Player[A].Character][Player[A].State];
                    Player[A].Location.Height = Physics.PlayerHeight[Player[A].Character][Player[A].State];
                }
                else if(Player[A].Mount == 3)
                {
                    YoshiHeight(A);
                }
                else if(Player[A].Character == 2 && Player[A].Mount != 2)
                {
                    Player[A].Location.Y = Player[A].Location.Y - Physics.PlayerHeight[2][2] + Physics.PlayerHeight[1][2];
                    Player[A].Location.Height = Physics.PlayerHeight[Player[A].Character][Player[A].State];
                }
            }
            else if(!(Player[A].State == 3))
                Player[A].State = 3;
            else
                Player[A].State = 2;
        }
        if(Player[A].Effect2 >= 50)
        {
            if(Player[A].State == 2)
                Player[A].State = 3;
            Player[A].Immune = Player[A].Immune + 50;
            Player[A].Immune2 = true;
            Player[A].Effect = 0;
            Player[A].Effect2 = 0;
            Player[A].StandUp = true;
        }
    }
    else if(Player[A].Effect == 41) // Player got ice power
    {
        if(Player[A].Duck == true && Player[A].Character != 5)
        {
            UnDuck(A);
            Player[A].Frame = 1;
        }
        Player[A].Effect2 = Player[A].Effect2 + 1;
        if(fEqual(Player[A].Effect2 / 5, std::floor(Player[A].Effect2 / 5.0)))
        {
            if(Player[A].State == 1 && Player[A].Character != 5)
            {
                Player[A].State = 2;
                if(Player[A].Mount == 0)
                {
                    Player[A].Location.X = Player[A].Location.X - Physics.PlayerWidth[Player[A].Character][2] * 0.5 + Physics.PlayerWidth[Player[A].Character][1] * 0.5;
                    Player[A].Location.Y = Player[A].Location.Y - Physics.PlayerHeight[Player[A].Character][2] + Physics.PlayerHeight[Player[A].Character][1];
                    Player[A].Location.Width = Physics.PlayerWidth[Player[A].Character][Player[A].State];
                    Player[A].Location.Height = Physics.PlayerHeight[Player[A].Character][Player[A].State];
                }
                else if(Player[A].Mount == 3)
                {
                    YoshiHeight(A);
                }
                else if(Player[A].Character == 2 && Player[A].Mount != 2)
                {
                    Player[A].Location.Y = Player[A].Location.Y - Physics.PlayerHeight[2][2] + Physics.PlayerHeight[1][2];
                    Player[A].Location.Height = Physics.PlayerHeight[Player[A].Character][Player[A].State];
                }
            }
            else if(!(Player[A].State == 7))
                Player[A].State = 7;
            else
                Player[A].State = 2;
        }
        if(Player[A].Effect2 >= 50)
        {
            if(Player[A].State == 2)
                Player[A].State = 7;
            Player[A].Immune = Player[A].Immune + 50;
            Player[A].Immune2 = true;
            Player[A].Effect = 0;
            Player[A].Effect2 = 0;
            Player[A].StandUp = true;
        }
    }
    else if(Player[A].Effect == 5) // Player got a leaf
    {
        Player[A].Frame = 1;
        if(Player[A].Effect2 == 0.0)
        {
            if(Player[A].State == 1 && Player[A].Mount == 0)
            {
                Player[A].Location.X = Player[A].Location.X - Physics.PlayerWidth[Player[A].Character][2] * 0.5 + Physics.PlayerWidth[Player[A].Character][1] * 0.5;
                Player[A].Location.Y = Player[A].Location.Y - Physics.PlayerHeight[Player[A].Character][2] + Physics.PlayerHeight[Player[A].Character][1];
                Player[A].State = 4;
                Player[A].Location.Width = Physics.PlayerWidth[Player[A].Character][Player[A].State];
                Player[A].Location.Height = Physics.PlayerHeight[Player[A].Character][Player[A].State];
            }
            else if(Player[A].Mount == 3)
            {
                YoshiHeight(A);
            }
            else if(Player[A].Character == 2 && Player[A].State == 1 && Player[A].Mount == 1)
            {
                Player[A].Location.Y = Player[A].Location.Y - Physics.PlayerHeight[2][2] + Physics.PlayerHeight[1][2];
                Player[A].Location.Height = Physics.PlayerHeight[Player[A].Character][4];
            }
            Player[A].State = 4;
            tempLocation.Width = 32;
            tempLocation.Height = 32;
            tempLocation.X = Player[A].Location.X + Player[A].Location.Width / 2.0 - tempLocation.Width / 2.0;
            tempLocation.Y = Player[A].Location.Y + Player[A].Location.Height / 2.0 - tempLocation.Height / 2.0;
            NewEffect(131, tempLocation, 1, 0, ShadowMode);
        }
        Player[A].Effect2 = Player[A].Effect2 + 1;
        if(fEqual(Player[A].Effect2, 14))
        {
            Player[A].Immune = Player[A].Immune + 50;
            Player[A].Immune2 = true;
            Player[A].Effect = 0;
            Player[A].Effect2 = 0;
            Player[A].StandUp = true;
        }
    }
    else if(Player[A].Effect == 11) // Player got a tanooki suit
    {
        Player[A].Frame = 1;
        Player[A].Immune2 = true;
        if(Player[A].Effect2 == 0.0)
        {
            if(Player[A].State == 1 && Player[A].Mount == 0)
            {
                Player[A].Location.X = Player[A].Location.X - Physics.PlayerWidth[Player[A].Character][2] * 0.5 + Physics.PlayerWidth[Player[A].Character][1] * 0.5;
                Player[A].Location.Y = Player[A].Location.Y - Physics.PlayerHeight[Player[A].Character][2] + Physics.PlayerHeight[Player[A].Character][1];
                Player[A].State = 5;
                Player[A].Location.Width = Physics.PlayerWidth[Player[A].Character][Player[A].State];
                Player[A].Location.Height = Physics.PlayerHeight[Player[A].Character][Player[A].State];
            }
            else if(Player[A].Mount == 3)
            {
                YoshiHeight(A);
            }
            else if(Player[A].Character == 2 && Player[A].State == 1 && Player[A].Mount == 1)
            {
                Player[A].Location.Y = Player[A].Location.Y - Physics.PlayerHeight[2][2] + Physics.PlayerHeight[1][2];
                Player[A].Location.Height = Physics.PlayerHeight[Player[A].Character][4];
            }
            Player[A].State = 5;
            tempLocation.Width = 32;
            tempLocation.Height = 32;
            tempLocation.X = Player[A].Location.X + Player[A].Location.Width / 2.0 - tempLocation.Width / 2.0;
            tempLocation.Y = Player[A].Location.Y + Player[A].Location.Height / 2.0 - tempLocation.Height / 2.0;
            NewEffect(131, tempLocation, 1, 0, ShadowMode);
        }
        Player[A].Effect2 = Player[A].Effect2 + 1;
        if(fEqual(Player[A].Effect2, 14))
        {
            Player[A].Immune = Player[A].Immune + 50;
            Player[A].Immune2 = true;
            Player[A].Effect = 0;
            Player[A].Effect2 = 0;
            Player[A].StandUp = true;
        }
    }
    else if(Player[A].Effect == 12) // Player got a hammer suit
    {
        Player[A].Frame = 1;
        Player[A].Immune2 = true;
        if(Player[A].Effect2 == 0.0)
        {
            if(Player[A].State == 1 && Player[A].Mount == 0)
            {
                Player[A].Location.X = Player[A].Location.X - Physics.PlayerWidth[Player[A].Character][2] * 0.5 + Physics.PlayerWidth[Player[A].Character][1] * 0.5;
                Player[A].Location.Y = Player[A].Location.Y - Physics.PlayerHeight[Player[A].Character][2] + Physics.PlayerHeight[Player[A].Character][1];
                Player[A].State = 5;
                Player[A].Location.Width = Physics.PlayerWidth[Player[A].Character][Player[A].State];
                Player[A].Location.Height = Physics.PlayerHeight[Player[A].Character][Player[A].State];
            }
            else if(Player[A].Mount == 3)
            {
                YoshiHeight(A);
            }
            else if(Player[A].Character == 2 && Player[A].State == 1 && Player[A].Mount == 1)
            {
                Player[A].Location.Y = Player[A].Location.Y - Physics.PlayerHeight[2][2] + Physics.PlayerHeight[1][2];
                Player[A].Location.Height = Physics.PlayerHeight[Player[A].Character][6];
            }
            Player[A].State = 6;
            tempLocation.Width = 32;
            tempLocation.Height = 32;
            tempLocation.X = Player[A].Location.X + Player[A].Location.Width / 2.0 - tempLocation.Width / 2.0;
            tempLocation.Y = Player[A].Location.Y + Player[A].Location.Height / 2.0 - tempLocation.Height / 2.0;
            NewEffect(131, tempLocation, 1, 0, ShadowMode);
        }
        Player[A].Effect2 = Player[A].Effect2 + 1;
        if(Player[A].Effect2 == 14.0)
        {
            Player[A].Immune = Player[A].Immune + 50;
            Player[A].Immune2 = true;
            Player[A].Effect = 0;
            Player[A].Effect2 = 0;
            Player[A].StandUp = true;
        }
    }
    else if(Player[A].Effect == 500) // Change to / from tanooki
    {
        for(B = 1; B <= 2; B++)
        {
            NewEffect(80, newLoc(Player[A].Location.X + dRand() * (Player[A].Location.Width + 8) - 8,
                                 Player[A].Location.Y + dRand() * (Player[A].Location.Height + 8) - 4), 1, 0, ShadowMode);
            Effect[numEffects].Location.SpeedX = dRand() * 2 - 1;
            Effect[numEffects].Location.SpeedY = dRand() * 2 - 1;
        }

        if(Player[A].Effect2 == 0.0)
        {
            UnDuck(A);
            PlaySound(34);
            tempLocation.Width = 32;
            tempLocation.Height = 32;
            tempLocation.X = Player[A].Location.X + Player[A].Location.Width / 2.0 - tempLocation.Width / 2.0;
            tempLocation.Y = Player[A].Location.Y + Player[A].Location.Height / 2.0 - tempLocation.Height / 2.0;
            NewEffect(10, tempLocation, 1, 0, ShadowMode);
            if(Player[A].Stoned == false)
            {
                Player[A].Frame = 0;
                Player[A].Stoned = true;
                Player[A].StonedCD = 15;
            }
            else
            {
                Player[A].StonedCD = 60;
                Player[A].Frame = 1;
                Player[A].Stoned = false;
            }
        }
        Player[A].Effect2 = Player[A].Effect2 + 1;
        Player[A].Immune = 10;
        Player[A].Immune2 = true;
        Player[A].StonedTime = 0;
        if(Player[A].Effect2 >= 5)
        {
            Player[A].Effect2 = 0;
            Player[A].Effect = 0;
            Player[A].Immune = 0;
            Player[A].Immune2 = 0;
        }
    }
    else if(Player[A].Effect == 9) // MultiMario
    {
        if(Player[A].HoldingNPC > numNPCs) // Can't hold an NPC that is dead
            Player[A].HoldingNPC = 0;
        if(Player[A].HoldingNPC > 0)
        {
            NPC[Player[A].HoldingNPC].Effect = 0;
            NPC[Player[A].HoldingNPC].CantHurt = Physics.NPCCanHurtWait;
            NPC[Player[A].HoldingNPC].CantHurtPlayer = A;
            if(Player[A].Direction > 0)
                NPC[Player[A].HoldingNPC].Location.X = Player[A].Location.X + Physics.PlayerGrabSpotX[Player[A].Character][Player[A].State];
            else
                NPC[Player[A].HoldingNPC].Location.X = Player[A].Location.X + Player[A].Location.Width - Physics.PlayerGrabSpotX[Player[A].Character][Player[A].State] - NPC[Player[A].HoldingNPC].Location.Width;
            NPC[Player[A].HoldingNPC].Location.Y = Player[A].Location.Y + Physics.PlayerGrabSpotY[Player[A].Character][Player[A].State] + 32 - NPC[Player[A].HoldingNPC].Location.Height;
        }
        Player[A].MountSpecial = 0;
        Player[A].YoshiTongueLength = 0;
        Player[A].Immune = Player[A].Immune + 1;
        if(Player[A].Immune >= 5)
        {
            Player[A].Immune = 0;
            if(Player[A].Immune2 == true)
                Player[A].Immune2 = false;
            else
                Player[A].Immune2 = true;
        }
        tempBool = true;
        for(B = 1; B <= numPlayers; B++)
        {
            if(B != A && (Player[B].Effect == 0 || fEqual(B, Player[A].Effect2)) && !Player[B].Dead && Player[B].TimeToLive == 0 && CheckCollision(Player[A].Location, Player[B].Location))
                tempBool = false;
        }
        if(tempBool == true)
        {
            Player[A].Effect = 0;
            Player[A].Effect2 = 0;
            Player[A].Immune = 0;
            Player[A].Immune2 = 0;
            Player[A].Location.SpeedY = 0.01;
        }
        else if(Player[A].Effect2 > 0)
        {
            D = float(Player[A].Effect2);
            if(Player[D].Effect == 0)
                Player[A].Effect2 = 0;
            Player[A].Immune2 = true;
            Player[A].Location.X = Player[D].Location.X + Player[D].Location.Width / 2.0 - Player[A].Location.Width / 2.0;
            Player[A].Location.Y = Player[D].Location.Y + Player[D].Location.Height - Player[A].Location.Height;
        }
    }
    else if(Player[A].Effect == 9) // Yoshi eat
    {
        Player[A].HoldingNPC = 0;
        Player[A].StandingOnNPC = 0;
        if(Player[Player[A].Effect2].YoshiPlayer != A)
        {
            Player[A].Effect = 0;
            Player[A].Effect2 = 0;
        }
    }
    else if(Player[A].Effect == 10) // Yoshi swallow
    {
        Player[A].HoldingNPC = 0;
        Player[A].StandingOnNPC = 0;
        Player[A].Section = Player[Player[A].Effect2].Section;
        Player[A].Location.X = Player[Player[A].Effect2].Location.X + Player[Player[A].Effect2].Location.Width / 2.0 - Player[A].Location.Width / 2.0;
        Player[A].Location.Y = Player[Player[A].Effect2].Location.Y + Player[Player[A].Effect2].Location.Height / 2.0 - Player[A].Location.Height / 2.0;
        if(Player[Player[A].Effect2].YoshiPlayer != A)
        {
            Player[A].Effect = 0;
            Player[A].Effect2 = 0;
        }
    }
    else if(Player[A].Effect == 6) // player stole a heldbonus
    {
        Player[A].Immune = Player[A].Immune + 1;
        if(Player[A].Immune >= 5)
        {
            Player[A].Immune = 0;
            if(Player[A].Immune2 == true)
                Player[A].Immune2 = false;
            else
                Player[A].Immune2 = true;
        }
        Player[A].Location.Y = Player[A].Location.Y + 2.2;
        if(Player[A].Location.Y >= Player[A].Effect2)
        {
            Player[A].Location.Y = Player[A].Effect2;
            tempBool = true;
            for(B = 1; B <= numPlayers; B++)
            {
                if(B != A && Player[B].Effect != 6 && CheckCollision(Player[A].Location, Player[B].Location) == true)
                    tempBool = false;
                // tempBool = False
            }
            if(tempBool == true)
            {
                Player[A].Effect = 0;
                Player[A].Effect2 = 0;
                Player[A].Immune = 50;
                Player[A].Immune2 = 0;
                Player[A].Location.SpeedY = 0.01;
            }
        }
        for(B = 1; B <= numPlayers; B++)
        {
            if(B != A && CheckCollision(Player[A].Location, Player[B].Location) == true)
            {
                if(Player[B].Mount == 2)
                {
                    Player[A].Effect = 0;
                    Player[A].Immune = 50;
                    Player[A].Immune2 = false;
                    Player[A].Location.Y = Player[B].Location.Y - Player[A].Location.Height;
                    Player[A].Location.SpeedY = 0.01;
                }
            }
        }
    }
    if(Player[A].Mount == 3 && Player[A].Effect != 9)
    {
        PlayerFrame(A);
    }
//    if(Player[A].Effect == 0)
//    {
//        if(nPlay.Online == true && A == nPlay.MySlot + 1)
//            Netplay::sendData Netplay::PutPlayerControls(nPlay.MySlot) + "1c" + std::to_string(A) + "|" + Player[A].Effect + "|" + Player[A].Effect2 + LB + "1h" + std::to_string(A) + "|" + Player[A].State + LB;
//    }
}
