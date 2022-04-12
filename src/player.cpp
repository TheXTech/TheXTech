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

#include <SDL2/SDL_timer.h>

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
#include "config.h"
#include "main/level_file.h"
#include "main/game_globals.h"
#include "main/trees.h"
#include "main/menu_main.h"
#include "core/render.h"
#include "core/events.h"
#include "compat.h"

#include "controls.h"


//void WaterCheck(const int A);
//// Private Sub Tanooki(A As Integer)
//void Tanooki(const int A);
//// Private Sub PowerUps(A As Integer)
//void PowerUps(const int A);
//// Private Sub SuperWarp(A As Integer)
//void SuperWarp(const int A);
//// Private Sub PlayerCollide(A As Integer)
//void PlayerCollide(const int A);
//// Private Sub PlayerEffects(A As Integer)
//void PlayerEffects(const int A);


static void setupPlayerAtCheckpoints(NPC_t &npc, Checkpoint_t &cp)
{
    Location_t tempLocation;
    int B;
    int C;
    tempLocation = npc.Location;
    tempLocation.Height = 600;

    C = 0;
    for(B = 1; B <= numBlock; B++)
    {
        if(CheckCollision(tempLocation, Block[B].Location))
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
        Player[B].Location.X = npc.Location.X + npc.Location.Width / 2.0 - Player[B].Location.Width / 2.0;
        CheckSection(B);
        pLogDebug("Restore player %d at checkpoint ID=%d by X=%g, Y=%g",
                  B, cp.id, Player[B].Location.X, Player[B].Location.Y);
    }

    if(numPlayers > 1)
    {
        Player[1].Location.X -= 16;
        Player[2].Location.X += 16;
    }
}

static void setupCheckpoints()
{
    if(Checkpoint != FullFileName || Checkpoint.empty())
    {
        if(StartLevel != FileNameFull && !LevelSelect)
        {
            pLogDebug("Clear check-points at SetupPlayers()");
            Checkpoint.clear();
            CheckpointsList.clear();
        }
        return;
    }

    pLogDebug("Trying to restore %zu checkpoints...", CheckpointsList.size());
    if(!g_compatibility.enable_multipoints && CheckpointsList.empty())
    {
        pLogDebug("Using legacy algorithm");
        CheckpointsList.push_back(Checkpoint_t());
    }
    for(int cpId = 0; cpId < int(CheckpointsList.size()); cpId++)
    {
        auto &cp = CheckpointsList[size_t(cpId)];

        for(int numNPCsMax = numNPCs, A = 1; A <= numNPCsMax; A++)
        {
            if(NPC[A].Type != 192)
                continue;

            if(g_compatibility.enable_multipoints && cp.id != Maths::iRound(NPC[A].Special))
                continue;

            NPC[A].Killed = 9;

            // found a last id, leave player here
            if(!g_compatibility.enable_multipoints || cpId == int(CheckpointsList.size() - 1))
            {
                setupPlayerAtCheckpoints(NPC[A], cp);
                if(g_compatibility.enable_multipoints)
                    break;// Stop to find NPCs
            }
        }// for NPCs

        if(!g_compatibility.enable_multipoints)
            break;
    } // for Check points
}

void SetupPlayers()
{
//    Location_t tempLocation;
//    Controls_t blankControls;
    int A = 0;
    int B = 0;
//    int C = 0;
    FreezeNPCs = false;
    qScreen = false;
    ForcedControls = false;
    // online stuff
    //    if(nPlay.Online)
    //    {
    //        for(A = 0; A <= 15; A++)
    //        {
    //            nPlay.Player[A].Controls = blankControls;
    //            nPlay.MyControls = blankControls;
    //        }
    //    }
    //    if(nPlay.Online)
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
    if(BattleMode)
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
        //        if(nPlay.Online) // online stuff
        //        {
        //            Player[A].State = 2; // Super mario
        //            Player[A].Mount = 0;
        //            if(A == nPlay.MySlot + 1)
        //            {
        //                if(frmNetplay::optPlayer(2).Value)
        //                    Player[A].Character = 2;
        //                else if(frmNetplay::optPlayer(3).Value)
        //                    Player[A].Character = 3;
        //                else if(frmNetplay::optPlayer(4).Value)
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
                Player[A].Hearts += 1;
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
        Player[A].WarpBackward = false;
        Player[A].WarpShooted = false;
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
        //        if(nPlay.Online && nPlay.Mode == 0)
        //        {
        //            if(nPlay.Player[A - 1].Active == false)
        //                Player[A].Dead = true;
        //        }
        Player[A].TimeToLive = 0;
        Player[A].Bumped = false;
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
            /*if(nPlay.Online)
            {
                Player[A].Location = Player[1].Location;
                Player[A].Location.X += A * 32 - 32;
            }
            else*/
            if(GameOutro)
            {
                Player[A].Location = Player[1].Location;
                Player[A].Location.X += A * 52 - 52;
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

        //        if(nPlay.Online && A <= 15)
        //        {
        //            if(nPlay.Player[A - 1].Active == false && A != 1)
        //                Player[A].Dead = true;
        //        }
        SizeCheck(Player[A]);
    }
    //    if(nPlay.Online)
    //    {
    //        Netplay::sendData "1d" + (nPlay.MySlot + 1) + "|" + Player[nPlay.MySlot + 1].Character + "|" + Player[nPlay.MySlot + 1].State + LB + Netplay::PutPlayerLoc(nPlay.MySlot + 1);
    //        StartMusic Player[nPlay.MySlot + 1].Section;
    //    }
    UpdateYoshiMusic();
    SetupScreens(); // setup the screen depending on how many players there are
    setupCheckpoints(); // setup the checkpoint and restpore the player at it if needed
}

void PlayerHurt(const int A)
{
    if(GodMode || GameOutro || BattleOutro > 0)
            return;

    auto &p = Player[A];
    Location_t tempLocation;
    int B = 0;

    if(p.Dead || p.TimeToLive > 0 || p.Stoned || p.Immune > 0 || p.Effect > 0)
        return;

    Controls::Rumble(A, 250, 0.5);
//    if(nPlay.Online) // netplay stuffs
//    {
//        if(nPlay.Allow == false && A != nPlay.MySlot + 1)
//            return;
//        if(A == nPlay.MySlot + 1)
//            Netplay::sendData Netplay::PutPlayerLoc(nPlay.MySlot) + "1a" + std::to_string(A) + "|" + p.State + LB;
//    }
    p.DoubleJump = false;
    p.GrabSpeed = 0;
    p.GrabTime = 0;
    p.Slide = false;
    p.SlideKill = false;
    p.CanFly = false;
    p.CanFly2 = false;
    p.FlyCount = 0;
    p.RunCount = 0;

    if(p.Fairy)
    {
        PlaySound(SFX_ZeldaFairy);
        p.Immune = 30;
        p.Effect = 8;
        p.Effect2 = 4;
        p.Fairy = false;
        p.FairyTime = 0;

        // FIXME: Here is a possible vanilla bug: B is always 0 even at original code
        SizeCheck(Player[B]);

        NewEffect(63, p.Location);
        if(p.Character == 5)
        {
            p.FrameCount = -10;
            p.Location.SpeedX = 3 * -p.Direction;
            p.Location.SpeedY = -7.01;
            p.StandingOnNPC = 0;
            p.FireBallCD = 20;
            PlaySound(SFX_ZeldaHurt);
        }
        return;
    }

    if(GameMenu)
    {
        if(p.State > 1)
            p.Hearts = 2;
        else
            p.Hearts = 1;
    }

    if(NPC[p.HoldingNPC].Type == 13)
        p.HoldingNPC = 0;

    if(LevelMacro == LEVELMACRO_OFF)
    {
        if(p.Immune == 0)
        {
            if(p.Mount == 1)
            {
                p.Mount = 0;
                PlaySound(SFX_Boot);
                UnDuck(Player[A]);
                tempLocation = p.Location;
                tempLocation.SpeedX = 5 * -p.Direction;
                if(p.MountType == 1)
                    NewEffect(26, tempLocation);
                else if(p.MountType == 2)
                    NewEffect(101, tempLocation);
                else
                    NewEffect(102, tempLocation);
                p.Location.Y += p.Location.Height;
                p.Location.Height = Physics.PlayerHeight[p.Character][p.State];
                p.Location.Y += -p.Location.Height;
                p.Immune = 150;
                p.Immune2 = true;
            }
            else if(p.Mount == 3)
            {
                UnDuck(Player[A]);
                PlaySound(SFX_YoshiHurt);
                p.Immune = 100;
                p.Immune2 = true;
                p.CanJump = false;
                p.Location.SpeedX = 0;
                if(p.Location.SpeedY > Physics.PlayerJumpVelocity)
                    p.Location.SpeedY = Physics.PlayerJumpVelocity;
                p.Jump = 0;
                p.Mount = 0;
                p.YoshiBlue = false;
                p.YoshiRed = false;
                p.GroundPound = false;
                p.GroundPound2 = false;
                p.YoshiYellow = false;
                p.Dismount = p.Immune;
                UpdateYoshiMusic();
                numNPCs++;
                NPC[numNPCs] = NPC_t();
                if(p.YoshiNPC > 0 || p.YoshiPlayer > 0)
                {
                    YoshiSpit(A);
                }
                // If ShadowMode = True Then .Shadow = True
                NPC[numNPCs].Direction = p.Direction;
                NPC[numNPCs].Active = true;
                NPC[numNPCs].TimeLeft = 100;
                if(p.MountType == 1)
                    NPC[numNPCs].Type = 95;
                else if(p.MountType == 2)
                    NPC[numNPCs].Type = 98;
                else if(p.MountType == 3)
                    NPC[numNPCs].Type = 99;
                else if(p.MountType == 4)
                    NPC[numNPCs].Type = 100;
                else if(p.MountType == 5)
                    NPC[numNPCs].Type = 148;
                else if(p.MountType == 6)
                    NPC[numNPCs].Type = 149;
                else if(p.MountType == 7)
                    NPC[numNPCs].Type = 150;
                else if(p.MountType == 8)
                    NPC[numNPCs].Type = 228;
                NPC[numNPCs].Special = 1;
                NPC[numNPCs].Location.Height = 32;
                NPC[numNPCs].Location.Width = 32;
                NPC[numNPCs].Location.Y = p.Location.Y + p.Location.Height - 33;
                NPC[numNPCs].Location.X = static_cast<int>(floor(static_cast<double>(p.Location.X + p.Location.Width / 2.0 - 16)));
                NPC[numNPCs].Location.SpeedY = 0.5;
                NPC[numNPCs].Location.SpeedX = 0;
                NPC[numNPCs].CantHurt = 10;
                NPC[numNPCs].CantHurtPlayer = A;
                syncLayers_NPC(numNPCs);

                p.Location.Height = Physics.PlayerHeight[p.Character][p.State];
            }
            else
            {
                if(p.Character == 3 || p.Character == 4)
                {
                    if(p.Hearts == 3 && (p.State == 2 || p.State == 4 || p.State == 5 || p.State == 6))
                    {
                        p.State = 2;
                        p.Immune = 150;
                        p.Immune2 = true;
                        p.Hearts -= 1;
                        PlaySound(SFX_PlayerHit);
                        return;
                    }
                    else
                    {
                        p.Hearts -= 1;
                        if(p.Hearts == 0)
                            p.State = 1;
                        else if(p.State == 3 && p.Hearts == 2)
                        {
                            p.Effect = 227;
                            p.Effect2 = 0;
                            PlaySound(SFX_PlayerShrink);
                            return;
                        }
                        else if(p.State == 7 && p.Hearts == 2)
                        {
                            p.Effect = 228;
                            p.Effect2 = 0;
                            PlaySound(SFX_PlayerShrink);
                            return;
                        }
                        else
                            p.State = 2;
                    }
                }
                else if(p.Character == 5)
                {
                    p.Hearts -= 1;
                    if(p.Hearts > 0)
                    {
                        p.State = (p.Hearts == 1) ? 1 : 2;
//                        if(p.Hearts == 1)
//                            p.State = 1;
//                        else
//                            p.State = 2;
                        // Always false because of previous setup
//                        if(p.State < 1)
//                            p.State = 1;
                        if(p.Mount == 0)
                        {
                            p.FrameCount = -10;
                            p.Location.SpeedX = 3 * -p.Direction;
                            p.Location.SpeedY = -7.01;
                            p.FireBallCD = 30;
                            p.SwordPoke = 0;
                        }
                        p.Immune = 150;
                        p.Immune2 = true;
                        PlaySound(SFX_ZeldaHurt);
                        return;
                    }
                }
                if(p.State > 1)
                {
                    PlaySound(SFX_PlayerShrink);
                    p.StateNPC = 0;
                    p.Effect = 2;
                }
                else
                {
                    PlayerDead(A);
                    p.HoldingNPC = 0;
                    if(p.Mount == 2)
                    {
                        p.Mount = 0;
                        numNPCs++;
                        NPC[numNPCs] = NPC_t();
                        NPC[numNPCs].Direction = p.Direction;
                        if(NPC[numNPCs].Direction == 1)
                            NPC[numNPCs].Frame = 4;
                        NPC[numNPCs].Frame += SpecialFrame[2];
                        NPC[numNPCs].Active = true;
                        NPC[numNPCs].TimeLeft = 100;
                        NPC[numNPCs].Type = 56;
                        NPC[numNPCs].Location.Height = 128;
                        NPC[numNPCs].Location.Width = 128;
                        NPC[numNPCs].Location.Y = static_cast<int>(floor(static_cast<double>(p.Location.Y)));
                        NPC[numNPCs].Location.X = static_cast<int>(floor(static_cast<double>(p.Location.X)));
                        NPC[numNPCs].Location.SpeedY = 0;
                        NPC[numNPCs].Location.SpeedX = 0;
                        NPC[numNPCs].CantHurt = 10;
                        NPC[numNPCs].CantHurtPlayer = A;
                        syncLayers_NPC(numNPCs);

                        p.Location.Height = Physics.PlayerHeight[p.Character][p.State];
                        p.Location.Width = Physics.PlayerWidth[p.Character][p.State];
                        p.Location.X += 64 - Physics.PlayerWidth[p.Character][p.State] / 2;
                        p.ForceHitSpot3 = true;
                        p.Location.Y = NPC[numNPCs].Location.Y - p.Location.Height;

                        for(int B = 1; B <= numNPCs; B++)
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

void PlayerDeathEffect(int A)
{
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
}

void PlayerDead(int A)
{
    Controls::Rumble(A, 400, 0.8);

    bool tempBool = false;
    int B = 0;
    auto &p = Player[A];

//    if(nPlay.Online) // netplay stuffs
//    {
//        if(nPlay.Allow == false && A != nPlay.MySlot + 1)
//            return;
//        if(A == nPlay.MySlot + 1)
//            Netplay::sendData Netplay::PutPlayerLoc(nPlay.MySlot) + "1b" + std::to_string(A) + LB;
//    }

    if(p.Character == 5)
        PlaySound(SFX_ZeldaDied);
    else
    {
        if(BattleMode)
            PlaySound(SFX_PlayerDied2);
        else if(numPlayers > 2)
        {
            for(B = 1; B <= numPlayers; B++)
            {
                if(!Player[B].Dead && Player[B].TimeToLive == 0 && A != B)
                    tempBool = true;
            }
            if(tempBool)
                PlaySound(SFX_PlayerDied2);
            else
                PlaySound(SFX_PlayerDied);
        }
        else
            PlaySound(SFX_PlayerDied);
    }

    if(p.YoshiNPC > 0 || p.YoshiPlayer > 0)
    {
        YoshiSpit(A);
    }

    p.Location.SpeedX = 0;
    p.Location.SpeedY = 0;
    p.Hearts = 0;
    p.Wet = 0;
    p.WetFrame = false;
    p.Quicksand = 0;
    p.Effect = 0;
    p.Effect2 = 0;
    p.Fairy = false;

    if(p.Mount == 2)
    {
        numNPCs++;
        NPC[numNPCs] = NPC_t();
        NPC[numNPCs].Direction = p.Direction;
        if(Maths::iRound(NPC[numNPCs].Direction) == 1)
            NPC[numNPCs].Frame = 4;
        NPC[numNPCs].Frame += SpecialFrame[2];
        NPC[numNPCs].Active = true;
        NPC[numNPCs].TimeLeft = 100;
        NPC[numNPCs].Type = 56;
        NPC[numNPCs].Location.Height = 128;
        NPC[numNPCs].Location.Width = 128;
        NPC[numNPCs].Location.Y = static_cast<int>(floor(static_cast<double>(p.Location.Y)));
        NPC[numNPCs].Location.X = static_cast<int>(floor(static_cast<double>(p.Location.X)));
        NPC[numNPCs].Location.SpeedY = 0;
        NPC[numNPCs].Location.SpeedX = 0;
        NPC[numNPCs].CantHurt = 10;
        NPC[numNPCs].CantHurtPlayer = A;
        syncLayers_NPC(numNPCs);

        p.Mount = 0;
        p.Location.Y -= 32;
        p.Location.Height = 32;
        SizeCheck(Player[A]);
    }

    p.Mount = 0;
    p.State = 1;
    p.HoldingNPC = 0;
    p.GroundPound = false;
    p.GroundPound2 = false;

    PlayerDeathEffect(A);

    p.TimeToLive = 1;

    if(CheckLiving() == 0 && !GameMenu && !BattleMode)
    {
        StopMusic();
        FreezeNPCs = false;
    }

    if(A == SingleCoop)
        SwapCoop();
}

void KillPlayer(const int A)
{
    Location_t tempLocation;
    auto &p = Player[A];

    p.Location.SpeedX = 0;
    p.Location.SpeedY = 0;
    p.State = 1;
    p.Stoned = false;
    p.Pinched1 = 0;
    p.Pinched2 = 0;
    p.Pinched3 = 0;
    p.Pinched4 = 0;
    p.NPCPinched = 0;
    p.TimeToLive = 0;
    p.Direction = 1;
    p.Frame = 1;
    p.Mount = 0;
    p.Dead = true;
    p.Location.X = 0;
    p.Location.Y = 0;
    p.Location.Width = Physics.PlayerWidth[p.Character][p.State];
    p.Location.Height = Physics.PlayerHeight[p.Character][p.State];

    if(p.HoldingNPC > 0)
    {
        if(NPC[p.HoldingNPC].Type == 272)
            NPC[p.HoldingNPC].Projectile = true;
    }

    p.HoldingNPC = 0;
    if(BattleMode)
    {
        if(BattleLives[A] <= 0)
        {
            if(BattleOutro == 0)
            {
                BattleOutro = 1;
                PlaySound(SFX_GotStar);
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
                BattleLives[A] -= 1;
            PlaySound(SFX_Raccoon);
            p.Frame = 1;
            p.Location.SpeedX = 0;
            p.Location.SpeedY = 0;
            p.Mount = 0;
            p.State = 2;
            p.Hearts = 2;
            p.Effect = 0;
            p.Location.Width = Physics.PlayerWidth[p.Character][p.State];
            p.Location.Height = Physics.PlayerHeight[p.Character][p.State];
            p.Location.X = PlayerStart[A].X + PlayerStart[A].Width * 0.5 - p.Location.Width * 0.5;
            p.Location.Y = PlayerStart[A].Y + PlayerStart[A].Height - p.Location.Height;
            p.Direction = 1;
            p.Dead = false;
            CheckSection(A);
            if(p.Location.X + p.Location.Width / 2.0 > level[p.Section].X + (level[p.Section].Width - level[p.Section].X) / 2)
                p.Direction = -1;
            p.Immune = 300;
            tempLocation.Width = 32;
            tempLocation.Height = 32;
            tempLocation.X = p.Location.X + p.Location.Width / 2.0 - tempLocation.Width / 2.0;
            tempLocation.Y = p.Location.Y + p.Location.Height / 2.0 - tempLocation.Height / 2.0;
            NewEffect(131, tempLocation);
            UpdateYoshiMusic();
        }
    }
}

int CheckDead()
{
    int A = 0;
    for(A = 1; A <= numPlayers; A++)
    {
        if(Player[A].Dead && Player[A].State > 0 && Player[A].Character > 0)
        {
//            if(nPlay.Online == false)
            return A;
//            else
//            {
//                if(nPlay.Player[A - 1].Active || A == 1)
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
        if(!Player[A].Dead && Player[A].TimeToLive == 0)
            return A;
    }
    return 0;
}

int LivingPlayersLeft()
{
    int ret = 0;

    for(int A = 1; A <= numPlayers; A++)
    {
        if(!Player[A].Dead)
            ret++;
    }

    return ret;
}

bool LivingPlayers() // Checks if anybody alive
{
    bool ret = false;

    for(int A = 1; A <= numPlayers; A++)
    {
        if(!Player[A].Dead)
        {
            ret = true;
            break;
        }
    }

    return ret;
}

void ProcessLastDead()
{
    if(!g_config.EnableInterLevelFade || BattleMode)
        return;

    if(LivingPlayersLeft() <= 1)
    {
        FadeOutMusic(500);
        g_levelScreenFader.setupFader(3, 0, 65, ScreenFader::S_FADE);
        levelWaitForFade();
    }
}

void EveryonesDead()
{
//    int A = 0; // UNUSED
    if(BattleMode)
        return;

    StopMusic();
    LevelMacro = LEVELMACRO_OFF;
    FreezeNPCs = false;

// Play fade effect instead of wait (see ProcessLastDead() above)
    if(!g_config.EnableInterLevelFade)
    {
        XRender::setTargetTexture();
        XRender::clearBuffer();
        XRender::repaint();
//    if(MagicHand)
//        BitBlt frmLevelWindow::vScreen[1].hdc, 0, 0, frmLevelWindow::vScreen[1].ScaleWidth, frmLevelWindow::vScreen[1].ScaleHeight, 0, 0, 0, vbWhiteness;
        PGE_Delay(500);
    }

    Lives--;
    if(Lives >= 0.f)
    {
        LevelMacro = LEVELMACRO_OFF;
        LevelMacroCounter = 0;

        ResetSoundFX();
        ClearLevel();
        if(RestartLevel)
        {
            OpenLevel(FullFileName);
            LevelSelect = false;
            LevelRestartRequested = true;
//            SetupPlayers();
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
        LevelMacro = LEVELMACRO_OFF;
        LevelMacroCounter = 0;
        ResetSoundFX();
        ClearLevel();
        LevelSelect = true;
        GameMenu = true;
        MenuMode = MENU_MAIN;
        MenuCursor = 0;
    }
    XEvents::doEvents();
}

void UnDuck(Player_t &p)
{
    if(p.Duck && p.GrabTime == 0) // Player stands up
    {
        if(p.Location.SpeedY != 0.0) // Fixes a block collision bug
            p.StandUp = true;
        p.StandUp2 = true;
        p.Frame = 1;
        p.FrameCount = 0;
        p.Duck = false;

        if(p.Mount == 3)
        {
            p.Location.Y += p.Location.Height;
            if(p.State == 1)
                p.Location.Height = 54;
            else
                p.Location.Height = 60;
            p.Location.Y += -p.Location.Height;
        }
        else
        {
            if(p.State == 1 && p.Mount == 1)
            {
                p.Location.Height = Physics.PlayerHeight[1][2];
                p.Location.Y += -Physics.PlayerHeight[1][2] + Physics.PlayerDuckHeight[1][2];
            }
            else
            {
                p.Location.Y += p.Location.Height;
                p.Location.Height = Physics.PlayerHeight[p.Character][p.State];
                p.Location.Y += -p.Location.Height;
            }
        }

        SizeCheck(p);
    }
}

void CheckSection(const int A)
{
    // finds out what section the player is in and handles the music for section changes
    int B = 0;
    int C = 0;
    int oldSection = 0;
    bool foundSection = false;
    auto &p = Player[A];

    if(LevelSelect)
        return;

    oldSection = p.Section;

    for(B = 0; B <= numSections; B++)
    {
        if(p.Location.X + p.Location.Width >= level[B].X)
        {
            if(p.Location.X <= level[B].Width)
            {
                if(p.Location.Y + p.Location.Height >= level[B].Y)
                {
                    if(p.Location.Y <= level[B].Height)
                    {
                        foundSection = true;
                        if(oldSection != B /*&& (nPlay.Online == false || nPlay.MySlot == A - 1)*/)
                        {
                            ClearBuffer = true;
                            p.Section = B;

                            //if(nPlay.Online)
                            //{
                            //    if(nPlay.MySlot == A - 1)
                            //        Netplay::sendData "1e" + std::to_string(A) + "|" + p.Section;
                            //    else
                            //        return;
                            //}

                            UpdateSoundFX(B);

                            if(curMusic >= 0 && !GameMenu) // Dont interupt boss / switch music
                            {
                                if(curMusic != bgMusic[B])
                                {
                                    StartMusic(B);
                                }
                                else if(bgMusic[B] == 24)
                                {
                                    if(oldSection >= 0)
                                    {
                                        if(CustomMusic[oldSection] != CustomMusic[p.Section])
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
            if(p.Location.X + p.Location.Width >= LevelREAL[B].X)
            {
                if(p.Location.X <= LevelREAL[B].Width)
                {
                    if(p.Location.Y + p.Location.Height >= LevelREAL[B].Y)
                    {
                        if(p.Location.Y <= LevelREAL[B].Height)
                        {
                            p.Section = B;

                            //if(nPlay.Online)
                            //{
                            //    if(nPlay.MySlot == A - 1)
                            //        Netplay::sendData "1e" + std::to_string(A) + "|" + p.Section;
                            //    else
                            //        return;
                            //}

                            if(oldSection != B)
                            {
                                ClearBuffer = true;
                                UpdateSoundFX(B);

                                if(curMusic != 6 && curMusic >= 0 && curMusic != 15) // Dont interupt boss / switch music
                                {
                                    if(curMusic != bgMusic[B])
                                    {
                                        StartMusic(B);
                                    }
                                    else if(bgMusic[B] == 24)
                                    {
                                        if(CustomMusic[B] != CustomMusic[p.Section])
                                        {
                                            StartMusic(B);
                                        }
                                    }
                                }
                            }

                            for(C = 1; C <= numPlayers; C++)
                            {
                                if(Player[C].Section == p.Section && C != A)
                                {
                                    p.Location.X = Player[C].Location.X + Player[C].Location.Width / 2.0 - p.Location.Width / 2.0;
                                    p.Location.Y = Player[C].Location.Y + Player[C].Location.Height - p.Location.Height - 0.01;
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

void PlayerFrame(const int A)
{
    PlayerFrame(Player[A]);
}

void PlayerFrame(Player_t &p)
{
// updates the players GFX
    Location_t tempLocation;
//    auto &p = Player[A];

// cause the flicker when he is immune
    if(p.Effect != 9)
    {
        if(p.Immune > 0)
        {
            p.Immune -= 1;
            if(p.Immune % 3 == 0)
            {
                p.Immune2 = !p.Immune2;
//                if(!p.Immune2)
//                    p.Immune2 = true;
//                else
//                    p.Immune2 = false;
            }
        }
        else
            p.Immune2 = false;
    }

// find frames for link
    if(p.Character == 5)
    {
        LinkFrame(p);
        return;
    }

// for the grab animation when picking something up from the top
    if(p.GrabTime > 0)
    {
        p.FrameCount += 1;
        if(p.FrameCount <= 6)
            p.Frame = 23;
        else if(p.FrameCount <= 12)
            p.Frame = 22;
        else
        {
            p.FrameCount = 0;
            p.Frame = 23;
        }
        return;
    }

// statue frames
    if(p.Stoned)
    {
        p.Frame = 0;
        p.FrameCount = 0;
        if(p.Location.SpeedX != 0.0)
        {
            if(p.Location.SpeedY == 0.0 || p.Slope > 0 || p.StandingOnNPC != 0)
            {
                if(p.SlideCounter <= 0)
                {
                    p.SlideCounter = 2 + iRand_round(2); // p(2) = 25%, p(3) = 50%, p(4) = 25%
                    tempLocation.Y = p.Location.Y + p.Location.Height - 5;
                    tempLocation.X = p.Location.X + p.Location.Width / 2.0 - 4;
                    NewEffect(74, tempLocation, 1, 0, ShadowMode);
                }
            }
        }
        return;
    }

// sliding frames
    if(p.Slide && (p.Character == 1 || p.Character == 2))
    {
        if(p.Location.SpeedX != 0.0)
        {
            if(p.Location.SpeedY == 0.0 || p.Slope > 0 || p.StandingOnNPC != 0)
            {
                if(p.SlideCounter <= 0 && p.SlideKill)
                {
                    p.SlideCounter = 2 + iRand_round(2);
                    tempLocation.Y = p.Location.Y + p.Location.Height - 4;
                    if(p.Location.SpeedX < 0)
                        tempLocation.X = p.Location.X + p.Location.Width / 2.0 - 4 + 6;
                    else
                        tempLocation.X = p.Location.X + p.Location.Width / 2.0 - 4 - 6;
                    NewEffect(74, tempLocation, 1, 0, ShadowMode);
                }
            }
        }
        p.Frame = 24;
        return;
    }

// climbing a vine/ladder
    if(p.Vine > 0)
    {
        bool doesPlayerMoves = false;

        if(g_compatibility.fix_climb_bgo_speed_adding && p.VineBGO > 0)
        {
            doesPlayerMoves = !fEqual(p.Location.SpeedX,  Background[p.VineBGO].Location.SpeedX) ||
                               p.Location.SpeedY < Background[p.VineBGO].Location.SpeedY - 0.1;
        }
        else
        {
            doesPlayerMoves = !fEqual(p.Location.SpeedX,  NPC[(int)p.VineNPC].Location.SpeedX) ||
                               p.Location.SpeedY < NPC[(int)p.VineNPC].Location.SpeedY - 0.1;
        }

        if(doesPlayerMoves) // Or .Location.SpeedY > 0.1 Then
        {
            p.FrameCount += 1;
            if(p.FrameCount >= 8)
            {
                p.Frame += 1;
                p.FrameCount = 0;
            }
            PlaySound(SFX_Climbing);
        }

        if(p.Frame < 25)
            p.Frame = 26;
        else if(p.Frame > 26)
            p.Frame = 25;

        return;
    }

// this finds the players direction
    if(!LevelSelect && p.Effect != 3)
    {
        if(!(p.Mount == 3 && p.MountSpecial > 0))
        {
            if(p.Controls.Left)
                p.Direction = -1;
            if(p.Controls.Right)
                p.Direction = 1;
        }
    }

    if(p.Driving && p.StandingOnNPC > 0)
        p.Direction = NPC[p.StandingOnNPC].DefaultDirection;

    if(p.Fairy)
        return;

// ducking and holding
    if(p.HoldingNPC > 0 && p.Duck)
    {
        p.Frame = 27;
        return;
    }

    p.MountOffsetY = 0;

// for the spinjump/shellsurf
    if((p.SpinJump || p.ShellSurf) && p.Mount == 0)
    {
        if(p.SpinFrame < 4 || p.SpinFrame >= 9)
            p.Direction = -1;
        else
            p.Direction = 1;
        if(p.ShellSurf)
        {
            if(NPC[p.StandingOnNPC].Location.SpeedX > 0)
                p.Direction = -p.Direction;
        }
        p.SpinFrame += 1;
        if(p.SpinFrame < 0)
            p.SpinFrame = 14;
        if(p.SpinFrame < 3)
        {
            p.Frame = 1;
            if(p.HoldingNPC > 0)
            {
                if(p.State == 1)
                    p.Frame = 5;
                else
                    p.Frame = 8;
            }
            if(p.State == 4 || p.State == 5)
                p.Frame = 12;
        }
        else if(p.SpinFrame < 6)
            p.Frame = 13;
        else if(p.SpinFrame < 9)
        {
            p.Frame = 1;
            if(p.HoldingNPC > 0)
            {
                if(p.State == 1)
                    p.Frame = 5;
                else
                    p.Frame = 8;
            }
            if(p.State == 4 || p.State == 5)
                p.Frame = 12;
        }
        else if(p.SpinFrame < 12 - 1)
            p.Frame = 15;
        else
        {
            p.Frame = 15;
            p.SpinFrame = -1;
        }
    }
    else
    {
        if(p.State == 1 && (p.Character == 1 || p.Character == 2)) // Small Mario & Luigi
        {
            if(p.HoldingNPC == 0) // not holding anything
            {
                if(p.WetFrame && p.Location.SpeedY != 0 && p.Slope == 0 && p.StandingOnNPC == 0 && !p.Duck && p.Quicksand == 0) // swimming
                {
                    if(p.Location.SpeedY < 0 || p.Frame == 42 || p.Frame == 43)
                    {
                        if(p.Frame != 40 && p.Frame != 42 && p.Frame != 43)
                            p.FrameCount = 6;

                        p.FrameCount += 1;

                        if(p.FrameCount < 6)
                            p.Frame = 40;
                        else if(p.FrameCount < 12)
                            p.Frame = 42;
                        else if(p.FrameCount < 18)
                            p.Frame = 43;
                        else
                        {
                            p.Frame = 43;
                            p.FrameCount = 0;
                        }
                    }
                    else
                    {
                        p.FrameCount += 1;

                        if(p.FrameCount < 10)
                            p.Frame = 40;
                        else if(p.FrameCount < 20)
                            p.Frame = 41;
                        else
                        {
                            p.Frame = 41;
                            p.FrameCount = 0;
                        }
                    }
                }
                else // not swimming
                {
                    if(p.Location.SpeedY == 0 || p.StandingOnNPC != 0 || p.Slope > 0 || (p.Location.SpeedY > 0 && p.Quicksand > 0))
                    {
                        if(p.Location.SpeedX > 0 && (p.Controls.Left || (p.Direction == -1 && p.Bumped)) && p.Effect == 0 && p.Quicksand == 0)
                        {
                            if(!LevelSelect)
                            {
                                if(p.Mount != 2 && !p.WetFrame && !p.Duck)
                                {
                                    PlaySound(SFX_Skid);
                                    if(p.SlideCounter <= 0)
                                    {
                                        p.SlideCounter = 2 + iRand_round(2);
                                        tempLocation.Y = p.Location.Y + p.Location.Height - 5;
                                        tempLocation.X = p.Location.X + p.Location.Width / 2.0 - 4 + 8 * -p.Direction;
                                        NewEffect(74, tempLocation, 1, 0, ShadowMode);
                                    }
                                }

                                p.Frame = 4;
                            }
                        }
                        else if(p.Location.SpeedX < 0 && (p.Controls.Right || (p.Direction == 1 && p.Bumped)) && p.Effect == 0 && p.Quicksand == 0)
                        {
                            if(!LevelSelect)
                            {
                                if(p.Mount != 2 && !p.WetFrame && !p.Duck)
                                {
                                    PlaySound(SFX_Skid);
                                    if(p.SlideCounter <= 0)
                                    {
                                        p.SlideCounter = 2 + iRand_round(2);
                                        tempLocation.Y = p.Location.Y + p.Location.Height - 5;
                                        tempLocation.X = p.Location.X + p.Location.Width / 2.0 - 4 + 8 * -p.Direction;
                                        NewEffect(74, tempLocation, 1, 0, ShadowMode);
                                    }
                                }

                                p.Frame = 4;
                            }
                        }
                        else
                        {
                            if(p.Location.SpeedX != 0 && !(p.Slippy && !p.Controls.Left && !p.Controls.Right))
                            {
                                p.FrameCount += 1;

                                if(p.Location.SpeedX > Physics.PlayerWalkSpeed - 1.5 || p.Location.SpeedX < -Physics.PlayerWalkSpeed + 1.5)
                                    p.FrameCount += 1;

                                if(p.Location.SpeedX > Physics.PlayerWalkSpeed || p.Location.SpeedX < -Physics.PlayerWalkSpeed)
                                    p.FrameCount += 1;

                                if(p.Location.SpeedX > Physics.PlayerWalkSpeed + 1 || p.Location.SpeedX < -Physics.PlayerWalkSpeed - 1)
                                    p.FrameCount += 1;

                                if(p.Location.SpeedX > Physics.PlayerWalkSpeed + 2 || p.Location.SpeedX < -Physics.PlayerWalkSpeed - 2)
                                    p.FrameCount += 1;

                                if(p.FrameCount >= 10)
                                {
                                    p.FrameCount = 0;
                                    if(p.Frame == 1)
                                        p.Frame = 2;
                                    else
                                        p.Frame = 1;
                                }
                            }
                            else
                            {
                                p.Frame = 1;
                                p.FrameCount = 0;
                            }
                        }
                    }
                    else
                        p.Frame = 3;
                }
            }
            else
            {
                if(p.Location.SpeedY == 0 || p.StandingOnNPC != 0 || p.Slope > 0)
                {
                    if(p.Mount != 2 &&
                       ((p.Controls.Left && p.Location.SpeedX > 0) || (p.Controls.Right && p.Location.SpeedX < 0)) &&
                        p.Effect == 0 && !p.Duck)
                    {
                        PlaySound(SFX_Skid);
                        if(p.SlideCounter <= 0)
                        {
                            p.SlideCounter = 2 + iRand_round(2);
                            tempLocation.Y = p.Location.Y + p.Location.Height - 5;
                            tempLocation.X = p.Location.X + p.Location.Width / 2.0 - 4 + 10 * -p.Direction;
                            NewEffect(74, tempLocation, 1, 0, ShadowMode);
                        }
                    }

                    if(p.Location.SpeedX != 0)
                    {
                        p.FrameCount += 2;

                        if(p.Location.SpeedX > Physics.PlayerWalkSpeed || p.Location.SpeedX < -Physics.PlayerWalkSpeed)
                            p.FrameCount += 3;

                        if(p.FrameCount >= 10)
                        {
                            p.FrameCount = 0;
                            if(p.Frame == 5)
                                p.Frame = 6;
                            else
                                p.Frame = 5;
                        }
                    }
                    else
                    {
                        p.Frame = 5;
                        p.FrameCount = 0;
                    }
                }
                else
                    p.Frame = 6;
            }
        }
        else if(p.FrameCount >= 100 && p.FrameCount <= 118 && (p.State == 3 || p.State == 6 || p.State == 7)) // Fire Mario and Luigi
        {
            if(p.Duck)
            {
                p.FrameCount = 0;
                p.Frame = 7;
            }
            else
            {
                if(p.FrameCount <= 106)
                {
                    p.Frame = 11;
                    if(p.WetFrame && p.Quicksand == 0 && p.Location.SpeedY != 0 && p.Slope == 0 && p.StandingOnNPC == 0 && p.Character <= 2)
                        p.Frame = 43;
                }
                else if(p.FrameCount <= 112)
                {
                    p.Frame = 12;
                    if(p.WetFrame && p.Quicksand == 0 && p.Location.SpeedY != 0 && p.Slope == 0 && p.StandingOnNPC == 0 && p.Character <= 2)
                        p.Frame = 44;
                }
                else
                {
                    p.Frame = 11;
                    if(p.WetFrame && p.Quicksand == 0 && p.Location.SpeedY != 0 && p.Slope == 0 && p.StandingOnNPC == 0 && p.Character <= 2)
                        p.Frame = 43;
                }

                p.FrameCount += 1;

                if(FlameThrower)
                    p.FrameCount += 2;

                if(p.FrameCount > 118)
                    p.FrameCount = 0;
            }
        }
        else if(p.TailCount > 0) // Racoon Mario
        {
            if(p.TailCount < 5 || p.TailCount >= 20)
                p.Frame = 12;
            else if(p.TailCount < 10)
                p.Frame = 15;
            else if(p.TailCount < 15)
                p.Frame = 14;
            else
                p.Frame = 13;
        }
        else // Large Mario, Luigi, and Peach
        {
            if(p.HoldingNPC == 0 || (p.Effect == 3 && p.Character >= 3))
            {
                if(p.WetFrame && p.Location.SpeedY != 0 && p.Slope == 0 &&
                   p.StandingOnNPC == 0 && !p.Duck && p.Quicksand == 0)
                {
                    if(p.Location.SpeedY < 0 || p.Frame == 43 || p.Frame == 44)
                    {
                        if(p.Character <= 2)
                        {
                            if(p.Frame != 40 && p.Frame != 43 && p.Frame != 44)
                                p.FrameCount = 6;
                        }

                        p.FrameCount += 1;
                        if(p.FrameCount < 6)
                            p.Frame = 40;
                        else if(p.FrameCount < 12)
                            p.Frame = 43;
                        else if(p.FrameCount < 18)
                            p.Frame = 44;
                        else
                        {
                            p.Frame = 44;
                            p.FrameCount = 0;
                        }
                    }
                    else
                    {
                        p.FrameCount += 1;
                        if(p.FrameCount < 10)
                            p.Frame = 40;
                        else if(p.FrameCount < 20)
                            p.Frame = 41;
                        else if(p.FrameCount < 30)
                            p.Frame = 42;
                        else if(p.FrameCount < 40)
                            p.Frame = 41;
                        else
                        {
                            p.Frame = 41;
                            p.FrameCount = 0;
                        }
                    }

                    if(p.Character >= 3)
                    {
                        if(p.Frame == 43)
                            p.Frame = 1;
                        else if(p.Frame == 44)
                            p.Frame = 2;
                        else
                            p.Frame = 5;

                    }
                }
                else
                {
                    if(p.Location.SpeedY == 0 || p.StandingOnNPC != 0 || p.Slope > 0 || (p.Quicksand > 0 && p.Location.SpeedY > 0))
                    {
                        if(p.Location.SpeedX > 0 && (p.Controls.Left || (p.Direction == -1 && p.Bumped)) &&
                           p.Effect == 0 && !p.Duck && p.Quicksand == 0)
                        {
                            if(!LevelSelect)
                            {
                                if(p.Mount != 2 && p.Wet == 0)
                                {
                                    PlaySound(SFX_Skid);
                                    if(p.SlideCounter <= 0)
                                    {
                                        p.SlideCounter = 2 + iRand_round(2);
                                        tempLocation.Y = p.Location.Y + p.Location.Height - 5;
                                        tempLocation.X = p.Location.X + p.Location.Width / 2.0 - 4 + 6 * -p.Direction;
                                        NewEffect(74, tempLocation, 1, 0, ShadowMode);
                                    }
                                }
                                p.Frame = 6;
                            }
                        }
                        else if(p.Location.SpeedX < 0 && (p.Controls.Right || (p.Direction == 1 && p.Bumped)) &&
                                p.Effect == 0 && !p.Duck && p.Quicksand == 0)
                        {
                            if(!LevelSelect)
                            {
                                if(p.Mount != 2 && p.Wet == 0)
                                {
                                    PlaySound(SFX_Skid);
                                    if(p.SlideCounter <= 0)
                                    {
                                        p.SlideCounter = 2 + iRand_round(2);
                                        tempLocation.Y = p.Location.Y + p.Location.Height - 5;
                                        tempLocation.X = p.Location.X + p.Location.Width / 2.0 - 4 + 10 * -p.Direction;
                                        NewEffect(74, tempLocation, 1, 0, ShadowMode);
                                    }
                                }
                                p.Frame = 6;
                            }
                        }
                        else
                        {
                            if(p.Location.SpeedX != 0 && !(p.Slippy && !p.Controls.Left && !p.Controls.Right))
                            {
                                p.FrameCount += 1;

                                if(p.Location.SpeedX >= Physics.PlayerWalkSpeed || p.Location.SpeedX <= -Physics.PlayerWalkSpeed)
                                    p.FrameCount += 1;

                                if(p.Location.SpeedX > Physics.PlayerWalkSpeed + 1.5 || p.Location.SpeedX < -Physics.PlayerWalkSpeed - 1.5)
                                    p.FrameCount += 1;

                                if(p.FrameCount >= 5 && p.FrameCount < 10)
                                {
                                    if(p.CanFly && p.Character != 3)
                                        p.Frame = 16;
                                    else
                                        p.Frame = 1;
                                }
                                else if(p.FrameCount >= 10 && p.FrameCount < 15)
                                {
                                    if(p.CanFly && p.Character != 3)
                                        p.Frame = 17;
                                    else
                                        p.Frame = 2;
                                }
                                else if(p.FrameCount >= 15 && p.FrameCount < 20)
                                {
                                    if(p.CanFly && p.Character != 3)
                                        p.Frame = 18;
                                    else
                                        p.Frame = 3;
                                }
                                else if(p.FrameCount >= 20)
                                {
                                    p.FrameCount -= 20;
                                    if(p.CanFly && p.Character != 3)
                                        p.Frame = 17;
                                    else
                                        p.Frame = 2;
                                }
                            }
                            else
                            {
                                p.Frame = 1;
                                p.FrameCount = 0;
                            }
                        }
                    }
                    else
                    {
                        if(p.CanFly2)
                        {
                            if(!p.Controls.Jump && !p.Controls.AltJump)
                            {
                                if(p.Location.SpeedY < 0)
                                    p.Frame = 19;
                                else
                                    p.Frame = 21;
                            }
                            else
                            {
                                p.FrameCount += 1;
                                if(!(p.Frame == 19 || p.Frame == 20 || p.Frame == 21))
                                    p.Frame = 19;
                                if(p.FrameCount >= 5)
                                {
                                    p.FrameCount = 0;
                                    if(p.Frame == 19)
                                        p.Frame = 20;
                                    else if(p.Frame == 20)
                                        p.Frame = 21;
                                    else
                                        p.Frame = 19;
                                }
                            }
                        }
                        else
                        {
                            if(p.Location.SpeedY < 0)
                                p.Frame = 4;
                            else
                            {
                                if((p.State == 4 || p.State == 5) && p.Controls.Jump && !(p.Character == 3 || p.Character == 4))
                                {
                                    p.FrameCount += 1;
                                    if(!(p.Frame == 3 || p.Frame == 5 || p.Frame == 11))
                                        p.Frame = 11;
                                    if(p.FrameCount >= 5)
                                    {
                                        p.FrameCount = 0;
                                        if(p.Frame == 11)
                                            p.Frame = 3;
                                        else if(p.Frame == 3)
                                            p.Frame = 5;
                                        else
                                            p.Frame = 11;
                                    }
                                }
                                else
                                    p.Frame = 5;
                            }
                        }
                    }
                    if(p.Duck)
                        p.Frame = 7;
                }
            }
            else
            {
                if(p.Location.SpeedY == 0 || p.StandingOnNPC != 0 || p.Slope > 0)
                {
                    if(p.Mount != 2 &&
                       ((p.Controls.Left && p.Location.SpeedX > 0) || (p.Controls.Right && p.Location.SpeedX < 0)) &&
                       p.Effect == 0 && !p.Duck)
                    {
                        PlaySound(SFX_Skid);
                        if(p.SlideCounter <= 0)
                        {
                            p.SlideCounter = 2 + iRand_round(2);
                            tempLocation.Y = p.Location.Y + p.Location.Height - 5;
                            tempLocation.X = p.Location.X + p.Location.Width / 2.0 - 4 + 10 * -p.Direction;
                            NewEffect(74, tempLocation, 1, 0, ShadowMode);
                        }
                    }
                    if(p.Location.SpeedX != 0)
                    {
                        p.FrameCount += 1;
                        if(p.Location.SpeedX > Physics.PlayerWalkSpeed || p.Location.SpeedX < -Physics.PlayerWalkSpeed)
                            p.FrameCount += 1;
                        if(p.FrameCount >= 5 && p.FrameCount < 10)
                            p.Frame = 8;
                        else if(p.FrameCount >= 10 && p.FrameCount < 15)
                            p.Frame = 9;
                        else if(p.FrameCount >= 15 && p.FrameCount < 20)
                            p.Frame = 10;
                        else if(p.FrameCount >= 20)
                        {
                            p.FrameCount = 0;
                            p.Frame = 9;
                        }
                    }
                    else
                    {
                        p.Frame = 8;
                        p.FrameCount = 0;
                    }
                }
                else
                {
                    p.Frame = 10;
                    if(p.Character == 3)
                        p.Frame = 9;
                }
            }
        }

        if(p.Mount == 1) // Goomba's Shoe
        {
            p.MountOffsetY = 0;//-p.Location.SpeedY / 2; FIXME: Verify this didn't broke anything

            if(p.Duck || p.StandingOnNPC != 0)
                p.MountOffsetY = 0;
            if(p.Direction == 1)
                p.MountFrame = 2 + SpecialFrame[1];
            else
                p.MountFrame = 0 + SpecialFrame[1];

            p.Frame = 1;
        }
        else if(p.Mount == 2) // Koopa Clown Car
        {
            p.Frame = 1;
            p.MountFrame = SpecialFrame[2];

            if(p.Direction == 1)
                p.MountFrame += 4;
        }
        else if(p.Mount == 3) // Green Yoshi
        {
            p.YoshiBY = 42;
            p.YoshiBX = 0;
            p.YoshiTY = 10;
            p.YoshiTX = 20;
            p.Frame = 30;
            p.YoshiBFrame = 0;
            p.YoshiTFrame = 0;
            p.MountOffsetY = 0;

            if(p.Location.SpeedY < 0 && p.StandingOnNPC == 0 && p.Slope == 0)
            {
                p.YoshiBFrame = 3;
                p.YoshiTFrame = 2;
            }
            else if(p.Location.SpeedY > 0 && p.StandingOnNPC == 0 && p.Slope == 0)
            {
                p.YoshiBFrame = 2;
                p.YoshiTFrame = 0;
            }
            else
            {
                if(p.Location.SpeedX != 0.0)
                {
                    if(p.Effect == 0)
                        p.YoshiBFrameCount += 1 + (std::abs(p.Location.SpeedX * 0.7));

                    if((p.Direction == -1 && p.Location.SpeedX > 0) || (p.Direction == 1 && p.Location.SpeedX < 0))
                        p.YoshiBFrameCount = 24;

                    if(p.YoshiBFrameCount < 0)
                        p.YoshiBFrameCount = 0;

                    if(p.YoshiBFrameCount > 32)
                    {
                        p.YoshiBFrame = 0;
                        p.YoshiBFrameCount = 0;
                    }
                    else if(p.YoshiBFrameCount > 24)
                    {
                        p.YoshiBFrame = 1;
                        p.YoshiTX -= 1;
                        p.YoshiTY += 2;
                        p.YoshiBY += 1;
                        p.MountOffsetY += 1;
                    }
                    else if(p.YoshiBFrameCount > 16)
                    {
                        p.YoshiBFrame = 2;
                        p.YoshiTX -= 2;
                        p.YoshiTY += 4;
                        p.YoshiBY += 2;
                        p.MountOffsetY += 2;
                    }
                    else if(p.YoshiBFrameCount > 8)
                    {
                        p.YoshiBFrame = 1;
                        p.YoshiTX -= 1;
                        p.YoshiTY += 2;
                        p.YoshiBY += 1;
                        p.MountOffsetY += 1;
                    }
                    else
                        p.YoshiBFrame = 0;
                }
                else
                    p.YoshiBFrameCount = 0;
            }

            if(p.MountSpecial == 1)
            {
                if(p.Controls.Up ||
                   (p.StandingOnNPC == 0 && p.Location.SpeedY != 0.0 &&
                    p.Slope == 0 && !p.Controls.Down))
                {
                    // .YoshiBFrame = 0
                    p.YoshiTFrame = 3;
                    // useless self-assignment code [PVS-Studio]
                    // p.MountOffsetY = p.MountOffsetY;
                    p.YoshiTongue.Y += p.MountOffsetY;
                    //p.YoshiTongue.Y = p.YoshiTongue.Y + p.MountOffsetY;
                }
                else
                {
                    // defaults
                    p.YoshiBY = 42;
                    p.YoshiBX = 0;
                    p.YoshiTY = 10;
                    p.YoshiTX = 20;
                    p.YoshiBFrame = 5;
                    p.YoshiTFrame = 4;
                    p.YoshiBY += 8;
                    p.YoshiTY += 24;
                    p.YoshiTX += 12;
                    /*
                    p.MountOffsetY = 0;
                    p.MountOffsetY += 8;
                    */
                    p.MountOffsetY = 8;
                }
            }

            if(p.Duck)
            {
                p.Frame = 31;
                if(p.MountSpecial == 0)
                    p.YoshiBFrame = 6;
                p.YoshiBFrameCount = 0;
            }

            if(p.YoshiTFrameCount > 0)
            {
                if(p.YoshiNPC == 0 && p.YoshiPlayer == 0)
                    p.YoshiTFrameCount += 1;

                if(p.YoshiTFrameCount < 10)
                    p.YoshiTFrame = 1;
                else if(p.YoshiTFrameCount < 20)
                    p.YoshiTFrame = 2;
                else
                    p.YoshiTFrameCount = 0;
            }
            else if(p.YoshiTFrameCount < 0)
            {
                p.YoshiTFrameCount -= 1;
                if(p.YoshiTFrameCount > -10)
                    p.YoshiTFrame = 3;
                else
                    p.YoshiTFrameCount = 0;
            }

            if(p.Direction == 1)
            {
                p.YoshiTFrame += 5;
                p.YoshiBFrame += 7;
            }
            else
            {
                p.YoshiBX = -p.YoshiBX;
                p.YoshiTX = -p.YoshiTX;
            }

            if(!p.Duck || p.MountSpecial > 0)
            {
                p.MountOffsetY -= (72 - p.Location.Height);
                p.YoshiBY -= (72 - p.Location.Height);
                p.YoshiTY -= (72 - p.Location.Height);
            }
            else
            {
                p.MountOffsetY -= (64 - p.Location.Height);
                p.YoshiBY -= (64 - p.Location.Height);
                p.YoshiTY -= (64 - p.Location.Height);
            }

            p.YoshiBX -= 4;
            p.YoshiTX -= 4;

            if(p.YoshiBlue)
            {
                if(p.Location.SpeedY == 0 || p.StandingOnNPC != 0)
                    p.YoshiWingsFrame = 1;
                else if(p.Location.SpeedY < 0)
                {
                    p.YoshiWingsFrameCount += 1;
                    if(p.YoshiWingsFrameCount < 6)
                        p.YoshiWingsFrame = 1;
                    else if(p.YoshiWingsFrameCount < 12)
                        p.YoshiWingsFrame = 0;
                    else
                    {
                        p.YoshiWingsFrameCount = 0;
                        p.YoshiWingsFrame = 0;
                    }
                }
                else
                {
                    p.YoshiWingsFrameCount += 1;
                    if(p.YoshiWingsFrameCount < 12)
                        p.YoshiWingsFrame = 1;
                    else if(p.YoshiWingsFrameCount < 24)
                        p.YoshiWingsFrame = 0;
                    else
                    {
                        p.YoshiWingsFrameCount = 0;
                        p.YoshiWingsFrame = 0;
                    }
                }
                if(p.GroundPound)
                    p.YoshiWingsFrame = 0;
                if(p.Direction == 1)
                    p.YoshiWingsFrame += 2;
            }
        }
    }

    if(p.Mount == 1 && p.MountType == 3)
    {
        if(p.Location.SpeedY == 0.0 || p.StandingOnNPC != 0)
            p.YoshiWingsFrame = 1;
        else if(p.Location.SpeedY < 0)
        {
            p.YoshiWingsFrameCount += 1;
            if(p.YoshiWingsFrameCount < 6)
                p.YoshiWingsFrame = 1;
            else if(p.YoshiWingsFrameCount < 12)
                p.YoshiWingsFrame = 0;
            else
            {
                p.YoshiWingsFrameCount = 0;
                p.YoshiWingsFrame = 0;
            }
        }
        else
        {
            p.YoshiWingsFrameCount += 1;
            if(p.YoshiWingsFrameCount < 12)
                p.YoshiWingsFrame = 1;
            else if(p.YoshiWingsFrameCount < 24)
                p.YoshiWingsFrame = 0;
            else
            {
                p.YoshiWingsFrameCount = 0;
                p.YoshiWingsFrame = 0;
            }
        }

        if(p.GroundPound)
            p.YoshiWingsFrame = 0;

        if(p.Direction == 1)
            p.YoshiWingsFrame += 2;
    }
}

void UpdatePlayerBonus(const int A, const int B)
{
    auto &p = Player[A];

    // 1 player growing
    // 4 fire flower
    // 5 leaf
    if(p.State != 1 || (p.Effect == 1 || p.Effect == 4 || p.Effect == 5))
    {
        if(B == 9 || B == 185 || B == 184 || B == 250)
        {
            if(p.HeldBonus == 0)
                p.HeldBonus = B;
        }
        else if((p.State == 2 || p.Effect == 1) && !(p.Effect == 4 || p.Effect == 5))
        {
            if(p.HeldBonus == 0)
            {
                if(p.StateNPC == 184)
                    p.HeldBonus = 184;
                else if(p.StateNPC == 185)
                    p.HeldBonus = 185;
                else
                    p.HeldBonus = 9;
            }
        }
        else
        {
            if(p.State == 3 || p.Effect == 4)
            {
                if(p.StateNPC == 183)
                    p.HeldBonus = 183;
                else if(p.StateNPC == 182)
                    p.HeldBonus = 182;
                else
                    p.HeldBonus = 14;
            }
            if(p.State == 4 || p.Effect == 5)
                p.HeldBonus = 34;
            if(p.State == 5 || p.Effect == 11)
                p.HeldBonus = 169;
            if(p.State == 6 || p.Effect == 12)
                p.HeldBonus = 170;
            if(p.State == 7 || p.Effect == 41)
            {
                if(p.StateNPC == 277)
                    p.HeldBonus = 277;
                else
                    p.HeldBonus = 264;
            }
        }
    }

    if(p.Character == 3 || p.Character == 4 || p.Character == 5)
        p.HeldBonus = 0;
}

void TailSwipe(const int plr, bool boo, bool Stab, int StabDir)
{
    auto &p = Player[plr];
    Location_t tailLoc;
    Location_t tempLoc;
    Location_t stabLoc;
    NPC_t oldNPC;
    int A = 0;
    long long B = 0;
    int C = 0;
    // int64_t fBlock = 0;
    // int64_t lBlock = 0;

    if(Stab)
    {
        if(!p.Duck)
        {
            if(StabDir == 1)
            {
                tailLoc.Width = 6;
                tailLoc.Height = 14;
                tailLoc.Y = p.Location.Y - tailLoc.Height;
                if(p.Direction == 1)
                    tailLoc.X = p.Location.X + p.Location.Width - 4;
                else
                    tailLoc.X = p.Location.X - tailLoc.Width + 4;
            }
            else if(StabDir == 2)
            {
                // tailLoc.Width = 8
                tailLoc.Height = 8;
                if(p.Location.SpeedY >= 10)
                    tailLoc.Height = 12;
                else if(p.Location.SpeedY >= 8)
                    tailLoc.Height = 10;
                tailLoc.Y = p.Location.Y + p.Location.Height;
                // tailLoc.X = .Location.X + .Location.Width / 2 - tailLoc.Width / 2 + (2 * .Direction)
                tailLoc.Width = p.Location.Width - 2;
                tailLoc.X = p.Location.X + 1;
            }
            else
            {
                tailLoc.Width = 38;
                tailLoc.Height = 6;
                tailLoc.Y = p.Location.Y + p.Location.Height - 42;
                if(p.Direction == 1)
                    tailLoc.X = p.Location.X + p.Location.Width;
                else
                    tailLoc.X = p.Location.X - tailLoc.Width;
            }
        }
        else
        {
            tailLoc.Width = 38;
            tailLoc.Height = 8;
            tailLoc.Y = p.Location.Y + p.Location.Height - 22;
            if(p.Direction == 1)
                tailLoc.X = p.Location.X + p.Location.Width;
            else
                tailLoc.X = p.Location.X - tailLoc.Width;
        }
    }
    else
    {
        tailLoc.Width = 18;
        tailLoc.Height = 12;
        tailLoc.Y = p.Location.Y + p.Location.Height - 26;
        if(p.Direction == 1)
            tailLoc.X = p.Location.X + p.Location.Width;
        else
            tailLoc.X = p.Location.X - tailLoc.Width;
    }

    if(p.Character == 4) // move tail down for toad
        tailLoc.Y += 4;

    if(boo) // the bool flag means hit a block
    {
        // fBlock = FirstBlock[(tailLoc.X / 32) - 1];
        // lBlock = LastBlock[((tailLoc.X + tailLoc.Width) / 32.0) + 1];
        // blockTileGet(tailLoc, fBlock, lBlock);

        for(Block_t* block : treeBlockQuery(tailLoc, SORTMODE_LOC))
        {
            A = block - &Block[1] + 1;
            if(!BlockIsSizable[Block[A].Type] && !Block[A].Hidden && (Block[A].Type != 293 || Stab) && !Block[A].Invis && !BlockNoClipping[Block[A].Type])
            {
                if(CheckCollision(tailLoc, Block[A].Location))
                {
                    if(Block[A].ShakeY == 0 && Block[A].ShakeY2 == 0 && Block[A].ShakeY3 == 0)
                    {
                        if(Block[A].Special > 0 || Block[A].Type == 55 || Block[A].Type == 159 || Block[A].Type == 90)
                            PlaySound(SFX_BlockHit);
//                        if(nPlay.Online && plr - 1 == nPlay.MySlot)
//                            Netplay::sendData Netplay::PutPlayerLoc(nPlay.MySlot) + "1g" + std::to_string(plr) + "|" + p.TailCount - 1;
#if XTECH_ENABLE_WEIRD_GFX_UPDATES
                        UpdateGraphics(true); // FIXME: Why this extra graphics update is here? It causes the lag while whipping blocks by the tail
#endif

                        BlockHit(A, (StabDir == 2), plr);
                        //if(StabDir == 2)
                        //{
                        //    BlockHit(A, true, plr);
                        //}
                        //else
                        //{
                        //    BlockHit(A, false, plr);
                        //}

                        BlockHitHard(A);
                        if(!Stab)
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
                                if(Block[A].Type == 293 || Block[A].Type == 370 || Block[A].ShakeY != 0 || Block[A].ShakeY2 != 0 || Block[A].ShakeY3 != 0 || Block[A].Hidden || BlockHurts[Block[A].Type])
                                {
                                    if(BlockHurts[Block[A].Type])
                                        PlaySound(SFX_Spring);
                                    p.Location.Y -= 0.1;
                                    p.Location.SpeedY = Physics.PlayerJumpVelocity;
                                    p.StandingOnNPC = 0;
                                    if(p.Controls.Jump || p.Controls.AltJump)
                                        p.Jump = 10;
                                }
                            }
                            if(Block[A].Type == 370)
                            {
                                PlaySound(SFX_ZeldaGrass);
                                Block[A].Hidden = true;
                                Block[A].Layer = LAYER_DESTROYED_BLOCKS;
                                syncLayersTrees_Block(A);
                                NewEffect(10, Block[A].Location);
                                Effect[numEffects].Location.SpeedY = -2;
                            }

                            if(Block[A].Type == 457 && p.State == 6)
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
        if(NPC[A].Active && NPC[A].Effect == 0 && !(NPCIsAnExit[NPC[A].Type] || (NPCIsACoin[NPC[A].Type] && !Stab)) &&
            NPC[A].CantHurtPlayer != plr && !(p.StandingOnNPC == A && p.ShellSurf))
        {
            if(NPC[A].Type != 13 && NPC[A].Type != 265 && !(NPC[A].Type == 17 && NPC[A].Projectile) &&
                NPC[A].Type != 108 && NPC[A].Type != 197 && NPC[A].Type != 192)
            {
                stabLoc = NPC[A].Location;
                if(NPCHeightGFX[NPC[A].Type] > NPC[A].Location.Height && NPC[A].Type != 8 && NPC[A].Type != 15 &&
                    NPC[A].Type != 205 && NPC[A].Type != 9 && NPC[A].Type != 51 && NPC[A].Type != 52 &&
                    NPC[A].Type != 74 && NPC[A].Type != 93 && NPC[A].Type != 245)
                {
                    stabLoc.Y += stabLoc.Height;
                    stabLoc.Height = NPCHeightGFX[NPC[A].Type];
                    stabLoc.Y += -stabLoc.Height;
                }

                if(NPC[A].Type == 91 && Stab)
                    stabLoc.Y += -stabLoc.Height;

                if(CheckCollision(tailLoc, stabLoc) && NPC[A].Killed == 0 && NPC[A].TailCD == 0 && !(StabDir != 0 && NPC[A].Type == 91))
                {
                    oldNPC = NPC[A];
                    if(Stab)
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
                                p.Location.SpeedY = Physics.PlayerJumpVelocity;
                                p.StandingOnNPC = 0;
                                if(p.Controls.Jump || p.Controls.AltJump)
                                    p.Jump = 10;
                            }
                            else
                            {
                                NPCHit(A, 10, plr);
                            }

                            if(StabDir == 2 && (NPC[A].Killed == 10 || NPC[A].Damage != B || NPC[A].Type != C))
                            {
                                p.Location.SpeedY = Physics.PlayerJumpVelocity;
                                p.StandingOnNPC = 0;
                                if(p.Controls.Jump || p.Controls.AltJump)
                                    p.Jump = 10;
                            }
                        }
                    }
                    else
                    {
                        NPCHit( A, 7, plr);
                        if(NPC[A].Killed > 0 || NPC[A].Type != oldNPC.Type || NPC[A].Projectile != oldNPC.Projectile || (NPC[A].Location.SpeedY != oldNPC.Location.SpeedY))
                        {
//                            if(nPlay.Online && plr - 1 == nPlay.MySlot)
//                                Netplay::sendData Netplay::PutPlayerLoc(nPlay.MySlot) + "1g" + std::to_string(plr) + "|" + p.TailCount - 1;
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

    if(BattleMode)
    {
        for(A = 1; A <= numPlayers; A++)
        {
            if(A != plr)
            {
                stabLoc = Player[A].Location;

                if(CheckCollision(tailLoc, stabLoc) && Player[A].Effect == 0 &&
                    Player[A].Immune == 0 && !Player[A].Dead && Player[A].TimeToLive == 0)
                {
                    if(Stab)
                    {
                        if(StabDir == 2)
                        {
                            p.Location.SpeedY = Physics.PlayerJumpVelocity;
                            p.StandingOnNPC = 0;
                            if(p.Controls.Jump || p.Controls.AltJump)
                                p.Jump = 10;
                        }

                        PlayerHurt(A);
                        PlaySound(SFX_ZeldaHit);
                    }
                    else
                    {
                        Player[A].Location.SpeedX = 6 * p.Direction;
                        Player[A].Location.SpeedY = -5;
                        PlaySound(SFX_Stomp);
                    }
                }
            }
        }
    }

    if(!Stab)
    {
        if(((p.TailCount) % 10 == 0 && !p.SpinJump) || ((p.TailCount) % 5 == 0 && p.SpinJump))
        {
            NewEffect (80, newLoc(tailLoc.X + (dRand() * tailLoc.Width) - 4, tailLoc.Y + (dRand() * tailLoc.Height)), 1, 0, ShadowMode);
            Effect[numEffects].Location.SpeedX = (0.5 + dRand() * 1) * p.Direction;
            Effect[numEffects].Location.SpeedY = dRand() * 1 - 0.5;
        }
    }
}

void YoshiHeight(const int A)
{
    auto &p = Player[A];

    if(p.Mount == 3)
    {
        p.Location.Y += p.Location.Height;
        p.Location.Height = (p.State == 1) ? 54 : 60;
        //if(p.State == 1)
        //    p.Location.Height = 54;
        //else
        //    p.Location.Height = 60;
        p.Location.Y += -p.Location.Height;
    }
}

void YoshiEat(const int A)
{
    Location_t tempLocation;
    auto &p = Player[A];

    for(int B = 1; B <= numPlayers; B++)
    {
        auto &p2 = Player[B];

        if(B != A && p2.Effect == 0 && !p2.Dead && p2.TimeToLive == 0 && p2.Mount == 0)
        {
            if(CheckCollision(p.YoshiTongue, p2.Location))
            {
                p.YoshiPlayer = B;
                p2.HoldingNPC = 0;
                return;
            }
        }
    }

    for(int numNPCsMax6 = numNPCs, B = 1; B <= numNPCsMax6; B++)
    {
        auto &n = NPC[B];
        if(((NPCIsACoin[n.Type] && n.Special == 1) || !NPCNoYoshi[n.Type]) &&
           n.Active && ((!NPCIsACoin[n.Type] || n.Special == 1) || n.Type == 103) &&
           !NPCIsAnExit[n.Type] && !n.Generator && !n.Inert && !NPCIsYoshi[n.Type] &&
            n.Effect != 5 && n.Immune == 0 && n.Type != 91 && !(n.Projectile && n.Type == 17) && n.HoldingPlayer == 0)
        {
            tempLocation = n.Location;
            if(n.Type == 91)
                tempLocation.Y = n.Location.Y - 16;

            if(CheckCollision(p.YoshiTongue, tempLocation))
            {
                if(n.Type == 91)
                {
                    if(!NPCNoYoshi[(int)n.Special])
                    {
                        PlaySound(SFX_Grab);
                        n.Generator = false;
                        n.Frame = 0;
                        n.Type = n.Special;
                        n.Special = 0;

                        if(NPCIsYoshi[n.Type])
                        {
                            n.Special = n.Type;
                            n.Type = 96;
                        }

                        n.Location.Height = NPCHeight[n.Type];
                        n.Location.Width = NPCWidth[n.Type];

                        if(!(n.Type == 21 || n.Type == 22 || n.Type == 26 || n.Type == 31 || n.Type == 32 || n.Type == 35 || n.Type == 49 || NPCIsAnExit[n.Type]))
                            n.DefaultType = 0;

                        n.Effect = 5;
                        n.Effect2 = A;
                        p.YoshiNPC = B;
                    }
                }
                else if(n.Type == 283)
                {
                    NPCHit(B, 3, B);
                }
                else
                {
                    n.Effect = 5;
                    n.Effect2 = A;
                    n.Location.Height = NPCHeight[n.Type];
                    p.YoshiNPC = B;
                }

                if(n.Type == 147)
                {
                    n.Type = 139 + iRand(9);
                    if(n.Type == 147)
                        n.Type = 92;
                    n.Location.X += n.Location.Width / 2.0;
                    n.Location.Y += n.Location.Height / 2.0;
                    n.Location.Width = NPCWidth[n.Type];
                    n.Location.Height = NPCHeight[n.Type];
                    n.Location.X += -n.Location.Width / 2.0;
                    n.Location.Y += -n.Location.Height / 2.0;
                }
                break;
            }
        }
    }
}

void YoshiSpit(const int A)
{
    int B = 0;
    auto &p = Player[A];

//    if(nPlay.Online && A - 1 == nPlay.MySlot)
//        Netplay::sendData Netplay::PutPlayerControls(nPlay.MySlot);
    p.YoshiTFrameCount = -1;

    if(p.YoshiPlayer > 0)
    {
        Player[p.YoshiPlayer].Section = p.Section;
        Player[p.YoshiPlayer].Effect = 0;
        Player[p.YoshiPlayer].Effect2 = 0;
        Player[p.YoshiPlayer].Slide = true;

        if(Player[p.YoshiPlayer].State > 1)
            Player[p.YoshiPlayer].Location.Height = Physics.PlayerDuckHeight[Player[p.YoshiPlayer].Character][Player[p.YoshiPlayer].State];
        // Player(.YoshiPlayer).Location.Y = Player(.YoshiPlayer).Location.Y - Physics.PlayerDuckHeight(Player(.YoshiPlayer).Character, Player(.YoshiPlayer).State) + Physics.PlayerHeight(Player(.YoshiPlayer).Character, Player(.YoshiPlayer).State)
        // Player(.YoshiPlayer).Duck = True

        if(p.Controls.Down)
        {
            Player[p.YoshiPlayer].Location.X = p.Location.X + p.YoshiTX + Player[p.YoshiPlayer].Location.Width * p.Direction;
            Player[p.YoshiPlayer].Location.X += 5;
            Player[p.YoshiPlayer].Location.Y = p.Location.Y + p.Location.Height - Player[p.YoshiPlayer].Location.Height;
            Player[p.YoshiPlayer].Location.SpeedX = 0 + p.Location.SpeedX * 0.3;
            Player[p.YoshiPlayer].Location.SpeedY = 1 + p.Location.SpeedY * 0.3;
        }
        else
        {
            Player[p.YoshiPlayer].Location.X = p.Location.X + p.YoshiTX + Player[p.YoshiPlayer].Location.Width * p.Direction;
            Player[p.YoshiPlayer].Location.X += 5;
            Player[p.YoshiPlayer].Location.Y = p.Location.Y + 1;
            Player[p.YoshiPlayer].Location.SpeedX = 7 * p.Direction + p.Location.SpeedX * 0.3;
            Player[p.YoshiPlayer].Location.SpeedY = -3 + p.Location.SpeedY * 0.3;
        }

        Player[p.YoshiPlayer].Direction = -p.Direction;
        Player[p.YoshiPlayer].Bumped = true;

        // Simplified code
        PlayerPush(p.YoshiPlayer, (p.Direction == 1) ? 2 : 4);
        //if(p.Direction == 1)
        //    PlayerPush(p.YoshiPlayer, 2);
        //else
        //    PlayerPush(p.YoshiPlayer, 4);

        p.YoshiPlayer = 0;
        PlaySound(SFX_BirdoSpit);
    }
    else
    {
        NPC[p.YoshiNPC].RealSpeedX = 0;
        if(NPCIsAShell[NPC[p.YoshiNPC].Type])
            NPC[p.YoshiNPC].Special = 0;

        if((NPCIsAShell[NPC[p.YoshiNPC].Type] || NPCIsABot[NPC[p.YoshiNPC].Type] || NPC[p.YoshiNPC].Type == 194) &&
           NPC[p.YoshiNPC].Type != 24 && p.YoshiRed)
        {
            NPC[p.YoshiNPC].Killed = 9;
            PlaySound(SFX_BigFireball);
            for(B = 1; B <= 3; B++)
            {
                numNPCs++;
                NPC[numNPCs] = NPC_t();
                NPC[numNPCs].Direction = p.Direction;
                NPC[numNPCs].Type = 108;
                NPC[numNPCs].Frame = EditorNPCFrame(NPC[numNPCs].Type, NPC[numNPCs].Direction);
                NPC[numNPCs].Active = true;
                NPC[numNPCs].Section = p.Section;
                NPC[numNPCs].TimeLeft = 100;
                NPC[numNPCs].Effect = 0;
                NPC[numNPCs].Location.X = p.Location.X + p.YoshiTX + 32 * p.Direction;
                NPC[numNPCs].Location.Y = p.Location.Y + p.YoshiTY;
                NPC[numNPCs].Location.Width = 32;
                NPC[numNPCs].Location.Height = 32;

                if(B == 1)
                {
                    NPC[numNPCs].Location.SpeedY = -0.8;
                    NPC[numNPCs].Location.SpeedX = 5 * p.Direction;
                }
                else if(B == 2)
                {
                    NPC[numNPCs].Location.SpeedY = 0;
                    NPC[numNPCs].Location.SpeedX = 5.5 * p.Direction;
                }
                else
                {
                    NPC[numNPCs].Location.SpeedY = 0.8;
                    NPC[numNPCs].Location.SpeedX = 5 * p.Direction;
                }

                syncLayers_NPC(numNPCs);
            }
        }
        else
        {
            NPC[p.YoshiNPC].Direction = p.Direction;
            NPC[p.YoshiNPC].Frame = 0;
            NPC[p.YoshiNPC].WallDeath = 5;
            NPC[p.YoshiNPC].FrameCount = 0;
            NPC[p.YoshiNPC].Frame = EditorNPCFrame(NPC[p.YoshiNPC].Type, NPC[p.YoshiNPC].Direction);
            NPC[p.YoshiNPC].Active = true;
            NPC[p.YoshiNPC].Section = p.Section;
            NPC[p.YoshiNPC].TimeLeft = 100;
            NPC[p.YoshiNPC].Effect = 0;
            NPC[p.YoshiNPC].Effect2 = 0;
            NPC[p.YoshiNPC].Location.X = p.Location.X + p.YoshiTX + 32 * p.Direction;
            NPC[p.YoshiNPC].Location.Y = p.Location.Y + p.YoshiTY;
            if(p.Duck)
                NPC[p.YoshiNPC].Location.Y -= 8;
            NPC[p.YoshiNPC].Location.Y -= 2;
            NPC[p.YoshiNPC].Location.SpeedX = 0;
            NPC[p.YoshiNPC].Location.SpeedY = 0;



            if(NPC[p.YoshiNPC].Type == 45)
                NPC[p.YoshiNPC].Special = 1;

            PlaySound(SFX_BirdoSpit);

            if(!p.Controls.Down || (p.Location.SpeedY != 0 && p.StandingOnNPC == 0 && p.Slope == 0))
            {
                if(NPCIsAShell[NPC[p.YoshiNPC].Type])
                {
                    SoundPause[9] = 2;
                    // NPCHit .YoshiNPC, 1, A
                    NPC[p.YoshiNPC].Location.SpeedX = Physics.NPCShellSpeed * p.Direction;
                }
                else if(NPC[p.YoshiNPC].Type == 45)
                    NPC[p.YoshiNPC].Location.SpeedX = Physics.NPCShellSpeed * p.Direction;
                else
                {
                    NPC[p.YoshiNPC].Projectile = true;
                    NPC[p.YoshiNPC].Location.SpeedX = 7 * p.Direction;
                    NPC[p.YoshiNPC].Location.SpeedY = -1.3;
                }
            }

            if(NPC[p.YoshiNPC].Type == 237)
            {
                NPC[p.YoshiNPC].Direction = p.Direction;
                NPC[p.YoshiNPC].Projectile = true;
                NPC[p.YoshiNPC].Location.SpeedX = Physics.NPCShellSpeed * p.Direction * 0.6 + p.Location.SpeedX * 0.4;
                NPC[p.YoshiNPC].TurnAround = false;
            }
        }
    }

    p.FireBallCD = 20;
    p.YoshiNPC = 0;
    p.YoshiRed = false;

    if(p.YoshiBlue)
    {
        p.CanFly = false;
        p.CanFly2 = false;
    }

    p.YoshiBlue = false;
    p.YoshiYellow = false;
}

void YoshiPound(const int A, int mount, bool BreakBlocks)
{
    int B = 0;
    Location_t tempLocation;
    Location_t tempLocation2;
    auto &p = Player[A];

    if(p.Location.SpeedY > 3)
    {

        tempLocation.Width = 128;
        tempLocation.X = p.Location.X + p.Location.Width / 2.0 - tempLocation.Width / 2.0;
        tempLocation.Height = 32;
        tempLocation.Y = p.Location.Y + p.Location.Height - 16;

        for(int numNPCsMax7 = numNPCs, B = 1; B <= numNPCsMax7; B++)
        {
            if(!NPC[B].Hidden && NPC[B].Active && NPC[B].Effect == 0)
            {
                tempLocation2 = NPC[B].Location;
                tempLocation2.Y += tempLocation2.Height - 4;
                tempLocation2.Height = 8;
                if(CheckCollision(tempLocation, tempLocation2))
                {
                    Block[0].Location.Y = NPC[B].Location.Y + NPC[B].Location.Height;
                    NPCHit(B, 2, 0);
                }
            }
        }

        if(BreakBlocks)
        {
            for(B = 1; B <= numBlock; B++)
            {
                auto &b = Block[B];
                if(b.Hidden || b.Invis || BlockNoClipping[b.Type] || BlockIsSizable[b.Type])
                    continue;

                if(g_compatibility.fix_dont_switch_player_by_clowncar && mount == 2 &&
                    ((b.Type >= 622 && b.Type <= 625) || b.Type == 631))
                    continue; // Forbid playable character switch when riding a clown car

                if(!CheckCollision(p.Location, b.Location))
                    continue;

                BlockHit(B, true, A);
                BlockHitHard(B);
            }
        }

        tempLocation.Width = 32;
        tempLocation.Height = 32;
        tempLocation.Y = p.Location.Y + p.Location.Height - 16;
        tempLocation.X = p.Location.X + p.Location.Width / 2.0 - 16 - 16;
        NewEffect(10, tempLocation);
        Effect[numEffects].Location.SpeedX = -2;
        tempLocation.X = p.Location.X + p.Location.Width / 2.0 - 16 + 16;
        NewEffect(10, tempLocation);
        Effect[numEffects].Location.SpeedX = 2;
        PlaySound(SFX_Twomp);
        if(BreakBlocks && g_config.GameplayShakeScreenPound)
            doShakeScreen(0, 4, SHAKE_SEQUENTIAL, 4, 0.2);
    }
}

void PlayerDismount(const int A)
{
    double tempSpeed;
    if(Player[A].Location.SpeedX > 0)
        tempSpeed = Player[A].Location.SpeedX * 0.2; // tempSpeed gives the player a height boost when jumping while running, based off their SpeedX
    else
        tempSpeed = -Player[A].Location.SpeedX * 0.2;

    // jump out of boot
    if(Player[A].Mount == 1)
    {
        // if not swimming
        if(Player[A].Wet <= 0 || Player[A].Quicksand != 0)
            UnDuck(Player[A]);
        Player[A].CanJump = false;
        PlaySound(SFX_Jump); // Jump sound
        PlaySound(SFX_Boot);
        Player[A].Location.SpeedY = Physics.PlayerJumpVelocity - tempSpeed;
        Player[A].Jump = Physics.PlayerJumpHeight;
        if(Player[A].Character == 2)
            Player[A].Jump += 3;
        if(Player[A].SpinJump)
            Player[A].Jump -= 6;
        Player[A].Mount = 0;
        Player[A].StandingOnNPC = 0;
        numNPCs++;
        NPC[numNPCs] = NPC_t();
        Player[A].FlyCount = 0;
        Player[A].RunCount = 0;
        Player[A].CanFly = false;
        Player[A].CanFly2 = false;
        NPC[numNPCs].Direction = Player[A].Direction;
        NPC[numNPCs].Active = true;
        NPC[numNPCs].TimeLeft = 100;

        if(Player[A].MountType == 1)
            NPC[numNPCs].Type = 35;
        else if(Player[A].MountType == 2)
            NPC[numNPCs].Type = 191;
        else if(Player[A].MountType == 3)
            NPC[numNPCs].Type = 193;

        NPC[numNPCs].Location.Height = 32;
        NPC[numNPCs].Location.Width = 32;
        NPC[numNPCs].Location.Y = Player[A].Location.Y + Player[A].Location.Height - 32;
        NPC[numNPCs].Location.X = static_cast<int>(floor(static_cast<double>(Player[A].Location.X + Player[A].Location.Width / 2.0 - 16)));
        NPC[numNPCs].Location.SpeedY = 1;
        NPC[numNPCs].Location.SpeedX = (Player[A].Location.SpeedX - NPC[Player[A].StandingOnNPC].Location.SpeedX) * 0.8;
        NPC[numNPCs].CantHurt = 10;
        NPC[numNPCs].CantHurtPlayer = A;

        syncLayers_NPC(numNPCs);

        Player[A].Location.Y = Player[A].Location.Y + Player[A].Location.Height;
        Player[A].Location.Height = Physics.PlayerHeight[Player[A].Character][Player[A].State];
        Player[A].Location.Y = Player[A].Location.Y - Player[A].Location.Height;
    }
    // jump out of clown car
    else if(Player[A].Mount == 2)
    {
        Player[A].CanJump = false;
        PlaySound(SFX_Jump); // Jump sound
        PlaySound(SFX_Boot);
        Player[A].Jump = Physics.PlayerJumpHeight;
        if(Player[A].Character == 2)
            Player[A].Jump = Player[A].Jump + 3;
        if(Player[A].SpinJump)
            Player[A].Jump = Player[A].Jump - 6;
        Player[A].Mount = 0;
        numNPCs++;
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
        syncLayers_NPC(numNPCs);

        Player[A].Location.SpeedY = double(Physics.PlayerJumpVelocity) - tempSpeed;
        Player[A].Location.Height = Physics.PlayerHeight[Player[A].Character][Player[A].State];
        Player[A].Location.Width = Physics.PlayerWidth[Player[A].Character][Player[A].State];
        Player[A].Location.X = Player[A].Location.X + 64 - Physics.PlayerWidth[Player[A].Character][Player[A].State] / 2;
        Player[A].StandUp = true;
        Player[A].StandUp2 = true;
        Player[A].ForceHitSpot3 = true;
        Player[A].Dismount = 30;
        Player[A].Slope = 0;
        Player[A].Location.Y = NPC[numNPCs].Location.Y - Player[A].Location.Height;

        for(int B = 1; B <= numPlayers; B++)
        {
            if(B != A && Player[B].Mount != 2 && CheckCollision(Player[A].Location, Player[B].Location))
                Player[B].Location.Y = Player[A].Location.Y - Player[B].Location.Height;
            if(Player[B].StandingOnTempNPC == 56)
            {
                Player[B].StandingOnNPC = numNPCs;
                Player[B].StandingOnTempNPC = 0;
            }
        }

        for(int numNPCsMax3 = numNPCs, B = 1; B <= numNPCsMax3; B++)
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
    // jump off yoshi
    else if(Player[A].Mount == 3)
    {
        UnDuck(Player[A]);
        if(Player[A].YoshiNPC > 0 || Player[A].YoshiPlayer > 0)
            YoshiSpit(A);
        Player[A].CanJump = false;
        Player[A].StandingOnNPC = 0;
        Player[A].Mount = 0;
        UpdateYoshiMusic();

        numNPCs++;
        NPC[numNPCs] = NPC_t();
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

        NPC[numNPCs].Location.Height = 32;
        NPC[numNPCs].Location.Width = 32;
        NPC[numNPCs].Location.Y = Player[A].Location.Y + Player[A].Location.Height - 32;
        NPC[numNPCs].Location.X = static_cast<int>(floor(static_cast<double>(Player[A].Location.X + Player[A].Location.Width / 2.0 - 16)));
        NPC[numNPCs].Location.SpeedY = 0.5;
        NPC[numNPCs].Location.SpeedX = 0;
        NPC[numNPCs].CantHurt = 10;
        NPC[numNPCs].CantHurtPlayer = A;
        syncLayers_NPC(numNPCs);

        Player[A].Location.Height = Physics.PlayerHeight[Player[A].Character][Player[A].State];
        // if not swimming
        if(Player[A].Wet <= 0 || Player[A].Quicksand != 0)
        {
            PlaySound(SFX_Jump); // Jump sound
            Player[A].Location.SpeedY = Physics.PlayerJumpVelocity - tempSpeed;
            Player[A].Jump = Physics.PlayerJumpHeight;
            if(Player[A].Character == 2)
                Player[A].Jump = Player[A].Jump + 3;
            if(Player[A].SpinJump)
                Player[A].Jump = Player[A].Jump - 6;
        }
    }
}

void SwapCoop()
{
    if(SingleCoop == 1)
    {
        if(Player[2].Dead || Player[2].TimeToLive > 0)
            return;

        SingleCoop = 2;
    }
    else
    {
        if(Player[1].Dead || Player[1].TimeToLive > 0)
            return;
        SingleCoop = 1;
    }

    Player[1].DropRelease = false;
    Player[1].Controls.Drop = true;
    Player[2].DropRelease = false;
    Player[2].Controls.Drop = true;
    PlaySound(SFX_Camera);

    Player[SingleCoop].Immune = 50;

    if(curMusic >= 0 && curMusic != bgMusic[Player[SingleCoop].Section])
    {
        StopMusic();
        StartMusic(Player[SingleCoop].Section);
    }
}

void PlayerPush(const int A, int HitSpot)
{
    Location_t tempLocation;
    // int64_t fBlock = 0;
    // int64_t lBlock = 0;

    if(ShadowMode)
        return;

    auto &p = Player[A];

    // fBlock = FirstBlock[(p.Location.X / 32) - 1];
    // lBlock = LastBlock[((p.Location.X + p.Location.Width) / 32.0) + 1];
    // blockTileGet(p.Location, fBlock, lBlock);

    for(Block_t* block : treeBlockQuery(p.Location, SORTMODE_LOC))
    {
        int B = block - &Block[1] + 1;
        Block_t& b = *block;

        if(b.Hidden || BlockIsSizable[b.Type])
            continue;

        if(g_compatibility.fix_player_filter_bounce && BlockCheckPlayerFilter(B, A))
            continue;

        if(BlockSlope[b.Type] == 0 && BlockSlope2[b.Type] == 0)
        {
            tempLocation = p.Location;
            tempLocation.Height -= 1;
            if(CheckCollision(tempLocation, b.Location))
            {
                if(!BlockOnlyHitspot1[b.Type] && !BlockNoClipping[b.Type])
                {
                    if(HitSpot == 2)
                        p.Location.X = b.Location.X - p.Location.Height - 0.01;
                    else if(HitSpot == 3)
                        p.Location.Y = b.Location.Y + b.Location.Height + 0.01;
                    else if(HitSpot == 4)
                        p.Location.X = b.Location.X + b.Location.Width + 0.01;
                }
            }
        }
    }
}

void SizeCheck(Player_t &p)
{
//On Error Resume Next

// player size fix
// height
    if(p.State == 0)
        p.State = 1;
    if(p.Character == 0)
        p.Character = 1;

    if(p.Fairy)
    {
        if(p.Duck)
        {
            UnDuck(p);
        }
        if(p.Location.Width != 22.0)
        {
            p.Location.X += p.Location.Width / 2.0 - 11;
            p.Location.Width = 22;
        }
        if(p.Location.Height != 26.0)
        {
            p.Location.Y += p.Location.Height - 26;
            p.Location.Height = 26;
        }
    }
    else if(p.Mount == 0)
    {
        if(!p.Duck)
        {
            if(p.Location.Height != Physics.PlayerHeight[p.Character][p.State])
            {
                p.Location.Y += p.Location.Height;
                p.Location.Height = Physics.PlayerHeight[p.Character][p.State];
                p.Location.Y += -p.Location.Height;
            }
        }
        else
        {
            if(p.Location.Height != Physics.PlayerDuckHeight[p.Character][p.State])
            {
                p.Location.Y += p.Location.Height;
                p.Location.Height = Physics.PlayerDuckHeight[p.Character][p.State];
                p.Location.Y += -p.Location.Height;
            }
        }
    }
    else if(p.Mount == 1)
    {
        if(p.Duck)
        {
            if(p.Location.Height != Physics.PlayerDuckHeight[p.Character][2])
            {
                p.Location.Y += p.Location.Height;
                p.Location.Height = Physics.PlayerDuckHeight[p.Character][2];
                p.Location.Y += -p.Location.Height;
            }
        }
        else if(p.Character == 2 && p.State > 1)
        {
            if(p.Location.Height != Physics.PlayerHeight[1][2])
            {
                p.Location.Y += p.Location.Height;
                p.Location.Height = Physics.PlayerHeight[p.Character][p.State];
                p.Location.Y += -p.Location.Height;
            }
        }
        else
        {
            if(p.Location.Height != Physics.PlayerHeight[1][2])
            {
                p.Location.Y += p.Location.Height;
                p.Location.Height = Physics.PlayerHeight[1][2];
                p.Location.Y += -p.Location.Height;
            }
        }
    }
    else if(p.Mount == 2)
    {
        if(p.Location.Height != 128)
        {
            p.Location.Y += p.Location.Height;
            p.Location.Height = 128;
            p.Location.Y += -p.Location.Height;
        }
    }
    else if(p.Mount == 3)
    {
        if(!p.Duck)
        {
            if(p.State == 1)
            {
                if(p.Location.Height != Physics.PlayerHeight[1][2])
                {
                    p.Location.Y += p.Location.Height;
                    p.Location.Height = Physics.PlayerHeight[1][2];
                    p.Location.Y += -p.Location.Height;
                }
            }
            else
            {
                if(p.Location.Height != Physics.PlayerHeight[2][2])
                {
                    p.Location.Y += p.Location.Height;
                    p.Location.Height = Physics.PlayerHeight[2][2];
                    p.Location.Y += -p.Location.Height;
                }
            }
        }
        else
        {
            if(p.Location.Height != 31)
            {
                p.Location.Y += p.Location.Height;
                p.Location.Height = 31;
                p.Location.Y += -p.Location.Height;
            }
        }
    }
// width
    if(p.Mount == 2)
    {
        if(p.Location.Width != 127.9)
        {
            p.Location.X += p.Location.Width / 2.0;
            p.Location.Width = 127.9;
            p.Location.X += -p.Location.Width / 2.0;
        }
    }
    else
    {
        if(p.Location.Width != Physics.PlayerWidth[p.Character][p.State])
        {
            p.Location.X += p.Location.Width / 2.0;
            p.Location.Width = Physics.PlayerWidth[p.Character][p.State];
            p.Location.X += -p.Location.Width / 2.0;
        }
    }
}

void YoshiEatCode(const int A)
{
    int B = 0;
    Location_t tempLocation;
    auto &p = Player[A];

    if(p.Mount == 3 && !p.Fairy)
    {
    // Shell Colors
        if(p.MountSpecial == 0)
        {
            if(p.YoshiNPC > 0)
            {
                if(NPC[p.YoshiNPC].Type == 7 || NPC[p.YoshiNPC].Type == 6)
                    p.YoshiRed = true;
                if(NPC[p.YoshiNPC].Type == 110 || NPC[p.YoshiNPC].Type == 114 || NPC[p.YoshiNPC].Type == 128)
                    p.YoshiRed = true;
                if(NPC[p.YoshiNPC].Type == 110 || NPC[p.YoshiNPC].Type == 114 || NPC[p.YoshiNPC].Type == 128)
                    p.YoshiRed = true;
                if(NPC[p.YoshiNPC].Type == 174 || NPC[p.YoshiNPC].Type == 175 || NPC[p.YoshiNPC].Type == 177 || NPC[p.YoshiNPC].Type == 194)
                    p.YoshiRed = true;

                if(NPC[p.YoshiNPC].Type == 111 || NPC[p.YoshiNPC].Type == 115 || NPC[p.YoshiNPC].Type == 194 || NPC[p.YoshiNPC].Type == 195)
                {
                    if(!p.YoshiBlue)
                    {
                        p.CanFly2 = true;
                        p.CanFly2 = true;
                        p.FlyCount = 300;
                    }
                    p.YoshiBlue = true;
                }

                if(NPC[p.YoshiNPC].Type == 112 || NPC[p.YoshiNPC].Type == 116 ||
                   NPC[p.YoshiNPC].Type == 127 || NPC[p.YoshiNPC].Type == 194 ||
                   NPC[p.YoshiNPC].Type == 195)
                    p.YoshiYellow = true;
            }

            if(p.YoshiNPC > 0 || p.YoshiPlayer > 0)
            {
                if(p.MountType == 2 || p.MountType == 5)
                {
                    if(!p.YoshiBlue)
                    {
                        p.CanFly2 = true;
                        p.CanFly2 = true;
                        p.FlyCount = 300;
                    }
                    p.YoshiBlue = true;
                }
                if(p.MountType == 3 || p.MountType == 5)
                    p.YoshiYellow = true;
                if(p.MountType == 4 || p.MountType == 5)
                    p.YoshiRed = true;//4;
            }
        }

        if(p.YoshiBlue)
        {
            p.CanFly = true;
            p.RunCount = 1000;
        }

        if(p.MountSpecial == 0)
        {
            if(NPC[p.YoshiNPC].Type == 31) // key check
            {
                for(B = 1; B <= numBackground; B++)
                {
                    if(Background[B].Type == 35)
                    {
                        tempLocation = Background[B].Location;
                        tempLocation.Width = 16;
                        tempLocation.X += 8;
                        tempLocation.Height = 26;
                        tempLocation.Y += 2;

                        if(CheckCollision(p.Location, tempLocation))
                        {
                            PlaySound(SFX_Key);
                            StopMusic();
                            LevelMacro = LEVELMACRO_KEYHOLE_EXIT;
                            break;
                        }
                    }
                }
            }
            else if(NPC[p.YoshiNPC].Type == 45)
                NPC[p.YoshiNPC].Special = 1;

            if(p.FireBallCD > 0)
                p.FireBallCD -= 1;

            if(p.Controls.Run)
            {
                if(p.RunRelease)
                {
                    if(p.YoshiNPC == 0 && p.YoshiPlayer == 0)
                    {
                        if(p.FireBallCD == 0)
                        {
                            p.MountSpecial = 1;
                            p.YoshiTongueLength = 0;
                            p.YoshiTonugeBool = false;
                            PlaySound(SFX_YoshiTongue);
                        }
                    }
                    else
                    {
                        YoshiSpit(A);
                    }
                }
            }
        }

        if(p.MountSpecial != 0)
        {
            p.YoshiTFrameCount = 0;

            if(p.YoshiNPC > 0 || p.YoshiPlayer > 0)
                p.YoshiTonugeBool = true;

            if(!p.YoshiTonugeBool)
            {
                if(p.MountType <= 4)
                {
                    if(p.YoshiTongueLength < 64 * 0.7)
                        p.YoshiTongueLength += 6;
                    else
                        p.YoshiTongueLength += 3;
                }
                else
                {
                    if(p.YoshiTongueLength < 80 * 0.7)
                        p.YoshiTongueLength += 8; // 7.5 will be rounded into 8;
                    else
                        p.YoshiTongueLength += 4; // 3.75 will be rounded into 4
                }

//                if(p.YoshiTongueLength >= 64 && p.MountType <= 4)
//                    p.YoshiTonugeBool = true;
//                else if(p.YoshiTongueLength >= 80)
//                    p.YoshiTonugeBool = true;

                // Simplified expression than commented above
                if((p.YoshiTongueLength >= 64 && p.MountType <= 4) || (p.YoshiTongueLength >= 80))
                    p.YoshiTonugeBool = true;
            }
            else
            {
                if(p.MountType <= 4)
                {
                    if(p.YoshiTongueLength < 64 * 0.7)
                        p.YoshiTongueLength -= 6;
                    else
                        p.YoshiTongueLength -= 3;
                }
                else
                {
                    if(p.YoshiTongueLength < 80 * 0.7)
                        p.YoshiTongueLength -= 8; // 7.5;
                    else
                        p.YoshiTongueLength -= 4; // 3.75;
                }
                if(p.YoshiTongueLength <= -8)
                {
                    p.YoshiTongueLength = 0;
                    p.YoshiTonugeBool = false;
                    p.MountSpecial = 0;
                }
            }

            p.YoshiTongue.Height = 12;
            p.YoshiTongue.Width = 16;
            p.YoshiTongueX = p.Location.X + p.Location.Width / 2.0;

            if(p.Controls.Up || (p.StandingOnNPC == 0 && p.Slope == 0 && p.Location.SpeedY != 0 && !p.Controls.Down))
            {
                p.YoshiTongueX += p.Direction * (22);
                p.YoshiTongue.Y = p.Location.Y + 8 + (p.Location.Height - 54);
                p.YoshiTongue.X = p.YoshiTongueX + p.YoshiTongueLength * p.Direction;
            }
            else
            {
                p.YoshiTongueX += p.Direction * (34);
                p.YoshiTongue.Y = p.Location.Y + 30 + (p.Location.Height - 54);
                p.YoshiTongue.X = p.YoshiTongueX + p.YoshiTongueLength * p.Direction;
            }

            if(p.Direction == -1)
                p.YoshiTongue.X -= 16;

            if(p.YoshiNPC == 0 && p.YoshiPlayer == 0)
            {
                YoshiEat(A);
            }

            if(p.YoshiNPC > 0)
            {
                NPC[p.YoshiNPC].Effect2 = A;
                NPC[p.YoshiNPC].Effect3 = 5;
                if(!p.YoshiTonugeBool)
                    p.YoshiTonugeBool = true;
                NPC[p.YoshiNPC].Location.X = p.YoshiTongue.X - NPC[p.YoshiNPC].Location.Width / 2.0 + 8 + 4 * p.Direction;
                NPC[p.YoshiNPC].Location.Y = p.YoshiTongue.Y - NPC[p.YoshiNPC].Location.Height / 2.0 + 6;
            }

            if(p.YoshiPlayer > 0)
            {
                Player[p.YoshiPlayer].Effect = 9;
                Player[p.YoshiPlayer].Effect2 = A;
                Player[p.YoshiPlayer].Location.X = p.YoshiTongue.X + p.YoshiTongue.Width / 2.0 - Player[p.YoshiPlayer].Location.Width / 2.0;
                Player[p.YoshiPlayer].Location.Y = p.YoshiTongue.Y + p.YoshiTongue.Height / 2.0 - Player[p.YoshiPlayer].Location.Height / 2.0;
                if(Player[p.YoshiPlayer].Location.Y + Player[p.YoshiPlayer].Location.Height > p.Location.Y + p.Location.Height)
                    Player[p.YoshiPlayer].Location.Y = p.Location.Y + p.Location.Height - Player[p.YoshiPlayer].Location.Height;
            }
        }
        if(p.MountSpecial == 0 && p.YoshiNPC > 0)
        {
            p.YoshiTFrameCount = 1;

            if(NPC[p.YoshiNPC].Type == 4 || NPC[p.YoshiNPC].Type == 76)
                NPC[p.YoshiNPC].Type = 5;
            else if(NPC[p.YoshiNPC].Type == 6 || NPC[p.YoshiNPC].Type == 161)
                NPC[p.YoshiNPC].Type = 7;
            else if(NPC[p.YoshiNPC].Type == 23)
                NPC[p.YoshiNPC].Type = 24;
            else if(NPC[p.YoshiNPC].Type == 72)
                NPC[p.YoshiNPC].Type = 73;
            else if(NPC[p.YoshiNPC].Type >= 109 && NPC[p.YoshiNPC].Type <= 112)
                NPC[p.YoshiNPC].Type += 4;
            else if(NPC[p.YoshiNPC].Type >= 121 && NPC[p.YoshiNPC].Type <= 124)
            {
                NPC[p.YoshiNPC].Type -= 8;
                NPC[p.YoshiNPC].Special = 0;
            }
            else if(NPC[p.YoshiNPC].Type == 173 || NPC[p.YoshiNPC].Type == 176)
            {
                NPC[p.YoshiNPC].Type = 172;
                NPC[p.YoshiNPC].Location.Height = 28;
            }
            else if(NPC[p.YoshiNPC].Type == 175 || NPC[p.YoshiNPC].Type == 177)
            {
                NPC[p.YoshiNPC].Type = 174;
                NPC[p.YoshiNPC].Location.Height = 28;
            }

            NPC[p.YoshiNPC].Location.Height = NPCHeight[NPC[p.YoshiNPC].Type];
            if((NPCIsGrabbable[NPC[p.YoshiNPC].Type] || NPCIsAShell[NPC[p.YoshiNPC].Type] || NPC[p.YoshiNPC].Type == 40 || NPCIsABot[NPC[p.YoshiNPC].Type] || NPC[p.YoshiNPC].Type == 194 || NPC[p.YoshiNPC].Type == 135 || NPC[p.YoshiNPC].Type == 136 || NPC[p.YoshiNPC].Type == 137) && (NPC[p.YoshiNPC].Type != 166))
            {
                if(NPC[p.YoshiNPC].Type == 135)
                    NPC[p.YoshiNPC].Special = 450;
                if(NPC[p.YoshiNPC].Type == 134)
                    NPC[p.YoshiNPC].Special = 250;
                if(NPC[p.YoshiNPC].Type == 136 || NPC[p.YoshiNPC].Type == 137)
                {
                    NPC[p.YoshiNPC].Special = 250;
                    NPC[p.YoshiNPC].Type = 137;
                    NPC[p.YoshiNPC].Location.Height = NPCHeight[NPC[p.YoshiNPC].Type];
                }

                NPC[p.YoshiNPC].Effect = 6;
                NPC[p.YoshiNPC].Effect2 = A;
                NPC[p.YoshiNPC].Active = false;

                if(NPC[p.YoshiNPC].Type == 49)
                {
                    NPC[p.YoshiNPC].Special = 0;
                    NPC[p.YoshiNPC].Special2 = 0;
                }
            }
            else if(p.MountType == 7 && !NPCIsABonus[NPC[p.YoshiNPC].Type])
            {
                B = iRand(9);
                NPC[p.YoshiNPC].Type = 139 + B;
                if(NPC[p.YoshiNPC].Type == 147)
                    NPC[p.YoshiNPC].Type = 92;
                NPC[p.YoshiNPC].Location.X += NPC[p.YoshiNPC].Location.Width / 2.0;
                NPC[p.YoshiNPC].Location.Y += NPC[p.YoshiNPC].Location.Height / 2.0;
                NPC[p.YoshiNPC].Location.Width = NPCWidth[NPC[p.YoshiNPC].Type];
                NPC[p.YoshiNPC].Location.Height = NPCHeight[NPC[p.YoshiNPC].Type];
                NPC[p.YoshiNPC].Location.X += -NPC[p.YoshiNPC].Location.Width / 2.0;
                NPC[p.YoshiNPC].Location.Y += -NPC[p.YoshiNPC].Location.Height / 2.0;
                NPC[p.YoshiNPC].Effect = 6;
                NPC[p.YoshiNPC].Effect2 = A;
                NPC[p.YoshiNPC].Active = false;
            }
            else if(p.MountType == 8 && !NPCIsABonus[NPC[p.YoshiNPC].Type])
            {
                NPC[p.YoshiNPC].Type = 237;
                NPC[p.YoshiNPC].Location.X += NPC[p.YoshiNPC].Location.Width / 2.0;
                NPC[p.YoshiNPC].Location.Y += NPC[p.YoshiNPC].Location.Height / 2.0;
                NPC[p.YoshiNPC].Location.Width = NPCWidth[NPC[p.YoshiNPC].Type];
                NPC[p.YoshiNPC].Location.Height = NPCHeight[NPC[p.YoshiNPC].Type];
                NPC[p.YoshiNPC].Location.X += -NPC[p.YoshiNPC].Location.Width / 2.0;
                NPC[p.YoshiNPC].Location.Y += -NPC[p.YoshiNPC].Location.Height / 2.0;
                NPC[p.YoshiNPC].Effect = 6;
                NPC[p.YoshiNPC].Effect2 = A;
                NPC[p.YoshiNPC].Active = false;
            }
            else
            {
                if(NPCIsABonus[NPC[p.YoshiNPC].Type])
                {
                    TouchBonus(A, p.YoshiNPC);
                    p.YoshiNPC = 0;
                }
                else
                {
                    MoreScore(NPCScore[NPC[p.YoshiNPC].Type], NPC[p.YoshiNPC].Location, p.Multiplier);
                    NPC[p.YoshiNPC].Killed = 9;
                    p.YoshiNPC = 0;
                    p.FireBallCD = 30;
                    Coins += 1;

                    if(Coins >= 100)
                    {
                        if(Lives < 99)
                        {
                            Lives += 1;
                            PlaySound(SFX_1up);
                            Coins -= 100;
                        }
                        else
                            Coins = 99;
                    }

                    PlaySound(SFX_YoshiSwallow);
                }
            }
        }
        else if(p.MountSpecial == 0 && p.YoshiPlayer > 0)
        {
            Player[p.YoshiPlayer].Effect = 10;
            Player[p.YoshiPlayer].Effect2 = A;
            Player[p.YoshiPlayer].Location.X = p.Location.X + p.Location.Width / 2.0 - Player[p.YoshiPlayer].Location.Width / 2.0;
            Player[p.YoshiPlayer].Location.Y = p.Location.Y + p.Location.Height / 2.0 - Player[p.YoshiPlayer].Location.Height / 2.0;
            p.YoshiTFrameCount = 1;
        }
    }
}

void RespawnPlayer(int A, double Direction, double CenterX, double StopY)
{
    Player[A].Location.Width = Physics.PlayerWidth[Player[A].Character][Player[A].State];
    Player[A].Location.Height = Physics.PlayerHeight[Player[A].Character][Player[A].State];
    Player[A].Frame = 1;
    Player[A].Direction = Direction;
    Player[A].Dead = false;
    Player[A].Location.SpeedX = 0;
    Player[A].Location.SpeedY = 0;
    Player[A].Effect = 6;
    // location where player stops flashing
    Player[A].Effect2 = StopY - Player[A].Location.Height;
    Player[A].Location.Y = -vScreenY[1] - Player[A].Location.Height;
    Player[A].Location.X = CenterX - Player[A].Location.Width / 2.0;
}

void RespawnPlayerTo(int A, int TargetPlayer)
{
    double CenterX = Player[TargetPlayer].Location.X + Player[TargetPlayer].Location.Width / 2.0;

    // don't lose a player when it targets a player who is already respawning
    double StopY;
    if(Player[TargetPlayer].Effect == 6)
        StopY = Player[TargetPlayer].Effect2 + Player[TargetPlayer].Location.Height;
    else if(Player[TargetPlayer].Mount == 2)
        StopY = Player[TargetPlayer].Location.Y;
    else
        StopY = Player[TargetPlayer].Location.Y + Player[TargetPlayer].Location.Height;

    // technically this would fix a vanilla bug (possible weird effects after Player 2 dies, Player 1 goes through Warp, Player 2 respawns)
    //   so I will do it where it only affects the new code.
    // Player[A].Section = Player[TargetPlayer].Section;
    RespawnPlayer(A, Player[TargetPlayer].Direction, CenterX, StopY);
}

void StealBonus()
{
    int A = 0;
    int B = 0;
    int C = 0;
    UNUSED(C);
//    Location_t tempLocation;

    // dead players steal life
    if(BattleMode)
        return;

    if(numPlayers == 2 /*&& nPlay.Online == false*/)
    {
        if((Player[1].Dead || Player[1].TimeToLive > 0) && (Player[2].Dead || Player[2].TimeToLive > 0))
            return;
        for(A = 1; A <= numPlayers; A++)
        {
            if(Player[A].Dead)
            {
                // find other player
                if(A == 1)
                    B = 2;
                else
                    B = 1;

                if(Lives > 0 && LevelMacro == LEVELMACRO_OFF)
                {
                    if(Player[A].Controls.Jump || Player[A].Controls.Run)
                    {
                        Lives -= 1;
                        Player[A].State = 1;
                        Player[A].Hearts = 1;
                        // old, dead HeldBonus code
                        // if(B == 1)
                        //     C = -40;
                        // if(B == 2)
                        //     C = 40;
                        RespawnPlayerTo(A, B);
                        PlaySound(SFX_DropItem);
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
        // commenting out because:
        //   (1) misplaced; (2) doesn't work with abstract controls
        // if(numPlayers > 2 && GameMenu == false && LevelMacro == LEVELMACRO_OFF && nPlay.Online == false)
        //     Player[A].Controls = Player[1].Controls;
        if(Player[A].Mount == 2 && Player[A].Dead == false && Player[A].TimeToLive == 0)
        {
            if(Player[A].Effect == 0)
            {
                if(Player[A].Controls.Left)
                {
                    Player[A].Location.SpeedX -= 0.1;
                    if(Player[A].Location.SpeedX > 0)
                        Player[A].Location.SpeedX -= 0.15;
                }
                else if(Player[A].Controls.Right)
                {
                    Player[A].Location.SpeedX += 0.1;
                    if(Player[A].Location.SpeedX < 0)
                        Player[A].Location.SpeedX += 0.15;
                }
                else
                {
                    if(Player[A].Location.SpeedX > 0.2)
                        Player[A].Location.SpeedX -= 0.05;
                    else if(Player[A].Location.SpeedX < -0.2)
                        Player[A].Location.SpeedX += 0.05;
                    else
                        Player[A].Location.SpeedX = 0;
                }

                if(Player[A].Controls.Up)
                {
                    Player[A].Location.SpeedY -= 0.1;
                    if(Player[A].Location.SpeedY > 0)
                        Player[A].Location.SpeedY -= 0.2;
                }
                else if(Player[A].Controls.Down)
                {
                    Player[A].Location.SpeedY += 0.2;
                    if(Player[A].Location.SpeedY < 0)
                        Player[A].Location.SpeedY += 0.2;
                }
                else
                {
                    if(Player[A].Location.SpeedY > 0.1)
                        Player[A].Location.SpeedY -= 0.1;
                    else if(Player[A].Location.SpeedY < -0.1)
                        Player[A].Location.SpeedY += 0.1;
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

            NPC[numNPCs].Location.Y += NPC[numNPCs].Location.SpeedY;
            NPC[numNPCs].Location.X += NPC[numNPCs].Location.SpeedX;
            NPC[numNPCs].Section = Player[A].Section;
            syncLayers_NPC(numNPCs);

            for(B = 1; B <= numPlayers; B++)
            {
                if(Player[B].StandingOnTempNPC == 56)
                {
                    Player[B].StandingOnNPC = numNPCs;
                    Player[B].Location.X += double(Player[A].mountBump);
                    if(Player[B].Effect != 0)
                    {
                        Player[B].Location.Y = Player[A].Location.Y - Player[B].Location.Height;
                        Player[B].Location.X += Player[A].Location.SpeedX;
                    }
                }
            }

            for(int numNPCsMax8 = numNPCs, B = 1; B <= numNPCsMax8; B++)
            {
                if(NPC[B].standingOnPlayer == A && NPC[B].Type != 50)
                {
                    if(Player[A].Effect == 0)
                        NPC[B].Location.X += Player[A].Location.SpeedX + double(Player[A].mountBump);
                    NPC[B].TimeLeft = 100;
                    NPC[B].Location.SpeedY = Player[A].Location.SpeedY;
                    NPC[B].Location.SpeedX = 0;
                    if(Player[A].Effect != 0)
                        NPC[B].Location.SpeedY = 0;
                    NPC[B].Location.Y = Player[A].Location.Y + NPC[B].Location.SpeedY + 0.1 - NPC[B].standingOnPlayerY;
                    if(Player[A].Controls.Run)
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
                                syncLayers_NPC(numNPCs);
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
                    tempLocation.Y += tempLocation.Height + 0.1;
                    tempLocation.X += 0.5;
                    tempLocation.Width -= 1;
                    tempLocation.Height = 1;

                    for(int numNPCsMax10 = numNPCs, C = 1; C <= numNPCsMax10; C++)
                    {
                        if(B != C && (NPC[C].standingOnPlayer == A || NPC[C].playerTemp))
                        {
                            if(CheckCollision(tempLocation, NPC[C].Location))
                                tempBool = true;
                        }
                    }

                    if(!tempBool)
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

void WaterCheck(const int A)
{
    Location_t tempLocation;
    int B = 0;
    auto &p = Player[A];

    if(p.Wet > 0)
    {
        p.Wet -= 1;
        p.Multiplier = 0;
    }

    if(p.Quicksand > 0)
    {
        p.Quicksand -= 1;
        if(p.Quicksand == 0)
            p.WetFrame = false;
    }

    if(UnderWater[p.Section])
        p.Wet = 2;

    if(p.Wet > 0)
    {
        p.SpinJump = false;
        p.WetFrame = true;
        p.Slide = false;
    }
    else if(p.WetFrame)
    {
        if(p.Location.SpeedY >= 3.1 || p.Location.SpeedY <= -3.1)
        {
            p.WetFrame = false;
            tempLocation.Width = 32;
            tempLocation.Height = 32;
            tempLocation.X = p.Location.X + p.Location.Width / 2.0 - tempLocation.Width / 2.0;
            tempLocation.Y = p.Location.Y + p.Location.Height - tempLocation.Height;
            NewEffect(114, tempLocation);
        }
    }

    for(int numWaterMax = numWater, B = 1; B <= numWaterMax; B++)
    {
        if(!Water[B].Hidden)
        {
            if(CheckCollision(p.Location, Water[B].Location))
            {
                if(p.Wet == 0 && p.Mount != 2)
                {
                    p.FlyCount = 0;
                    p.CanFly = false;
                    p.CanFly2 = false;

                    if(!p.Controls.Jump && !p.Controls.AltJump)
                        p.CanJump = true;

                    p.SwimCount = 0;

                    if(p.Location.SpeedY > 0.5)
                        p.Location.SpeedY = 0.5;
                    if(p.Location.SpeedY < -1.5)
                        p.Location.SpeedY = -1.5;

                    if(!p.WetFrame)
                    {
                        if(p.Location.SpeedX > 0.5)
                            p.Location.SpeedX = 0.5;
                        if(p.Location.SpeedX < -0.5)
                            p.Location.SpeedX = -0.5;
                    }

                    if(p.Location.SpeedY > 0 && !p.WetFrame)
                    {
                        tempLocation.Width = 32;
                        tempLocation.Height = 32;
                        tempLocation.X = p.Location.X + p.Location.Width / 2.0 - tempLocation.Width / 2.0;
                        tempLocation.Y = p.Location.Y + p.Location.Height - tempLocation.Height;
                        NewEffect(114, tempLocation);
                    }
                }

                p.Wet = 2;
                p.SpinJump = false;

                if(Water[B].Quicksand)
                    p.Quicksand = 3;
            }
        }
    }

    if(p.Mount == 2)
    {
        p.Wet = 0;
        p.WetFrame = false;
    }

    if(p.Wet == 1)
    {
        if(p.Location.SpeedY < 0 && (p.Controls.AltJump || p.Controls.Jump) && !p.Controls.Down)
        {
            p.Jump = 12;
            p.Location.SpeedY = double(Physics.PlayerJumpVelocity);
        }
    }
    else if(p.Wet == 2 && p.Quicksand == 0)
    {
        if(iRand(100) >= 97)
        {
            if(p.Direction == 1)
                tempLocation = newLoc(p.Location.X + p.Location.Width - dRand() * 8, p.Location.Y + 4 + dRand() * 8, 8, 8);
            else
                tempLocation = newLoc(p.Location.X - 8 + dRand() * 8, p.Location.Y + 4 + dRand() * 8, 8, 8);
            if(!UnderWater[p.Section])
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

void Tanooki(const int A)
{
    auto &p = Player[A];

    if(p.Fairy)
           return;
// tanooki
    if(p.Stoned && p.Controls.Down && p.StandingOnNPC == 0)
    {
        p.Location.SpeedX = p.Location.SpeedX * 0.8;
        if(p.Location.SpeedX >= -0.5 && p.Location.SpeedX <= 0.5)
            p.Location.SpeedX = 0;
        if(p.Location.SpeedY < 8)
            p.Location.SpeedY += 0.25;
    }

    if(p.StonedCD == 0)
    {
        // If .Mount = 0 And .State = 5 And .Controls.Run = True And .Controls.Down = True Then
        if(p.Mount == 0 && p.State == 5 && p.Controls.AltRun && p.Bombs == 0)
        {
            if(!p.Stoned)
                p.Effect = 500;
        }
        else if(p.Stoned)
            p.Effect = 500;
    }
    else
        p.StonedCD -= 1;

    if(p.Stoned)
    {
        p.StonedTime += 1;
        if(p.StonedTime >= 240)
        {
            p.Effect = 500;
            p.StonedCD = 60;
        }
        else if(p.StonedTime >= 180)
        {
            p.Immune += 1;
            if(p.Immune % 3 == 0)
            {
                p.Immune2 = !p.Immune2;
//                if(p.Immune2)
//                    p.Immune2 = false;
//                else
//                    p.Immune2 = true;
            }
        }
    }
}

void PowerUps(const int A)
{
    bool BoomOut = false;
    auto &p = Player[A];
    //int B = 0;

    if(p.Fairy)
    {
        p.SwordPoke = 0;
        p.FireBallCD = 0;
        p.FireBallCD2 = 0;
        p.TailCount = 0;
        return;
    }


    if(p.State == 6 && p.Character == 4 && p.Controls.Run && p.RunRelease)
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
        if(!p.Slide && p.Vine == 0 && p.State == 6 && !p.Duck && p.Mount != 2 && p.Mount != 3 && p.HoldingNPC <= 0 && p.Character != 5)
        {
            if(p.Controls.Run && !p.SpinJump && p.FireBallCD <= 0 && !BoomOut)
            {
                if(p.RunRelease || FlameThrower)
                {
                    if(numNPCs < maxNPCs - 100)
                    {
//                        if(nPlay.Online && A - 1 == nPlay.MySlot)
//                            Netplay::sendData Netplay::PutPlayerControls(nPlay.MySlot) + "1f" + std::to_string(A) + "|" + p.FireBallCD - 1;
                        p.FrameCount = 110;
                        p.FireBallCD = 25;
                        numNPCs++;
                        NPC[numNPCs] = NPC_t();
                        if(ShadowMode)
                            NPC[numNPCs].Shadow = true;
                        NPC[numNPCs].Type = 171;
                        if(p.Character == 3)
                        {
                            p.FireBallCD = 45;
                            NPC[numNPCs].Type = 291;
                            if(p.Controls.AltRun && p.Mount == 0)
                            {
                                NPC[numNPCs].HoldingPlayer = A;
                                p.HoldingNPC = numNPCs;
                                PlaySound(SFX_Grab2);
                            }
                            else
                                PlaySound(SFX_Throw);
                        }
                        else if(p.Character == 4)
                        {

                            p.FireBallCD = 0;
                            if(FlameThrower)
                                p.FireBallCD = 40;
                            NPC[numNPCs].Type = 292;
                            NPC[numNPCs].Special5 = A;
                            NPC[numNPCs].Special6 = p.Direction;
                            PlaySound(SFX_Throw);
                        }
                        else
                            PlaySound(playerHammerSFX);

                        NPC[numNPCs].Projectile = true;
                        NPC[numNPCs].Location.Height = NPCHeight[NPC[numNPCs].Type];
                        NPC[numNPCs].Location.Width = NPCWidth[NPC[numNPCs].Type];
                        NPC[numNPCs].Location.X = p.Location.X + Physics.PlayerGrabSpotX[p.Character][p.State] * p.Direction;
                        NPC[numNPCs].Location.Y = p.Location.Y + Physics.PlayerGrabSpotY[p.Character][p.State];
                        NPC[numNPCs].Active = true;
                        NPC[numNPCs].TimeLeft = 100;
                        NPC[numNPCs].Location.SpeedY = 20;
                        NPC[numNPCs].CantHurt = 100;
                        NPC[numNPCs].CantHurtPlayer = A;

                        if(p.Controls.Up)
                        {
                            NPC[numNPCs].Location.SpeedX = 2 * p.Direction + p.Location.SpeedX * 0.9;
                            if(p.StandingOnNPC == 0)
                                NPC[numNPCs].Location.SpeedY = -8 + p.Location.SpeedY * 0.3;
                            else
                                NPC[numNPCs].Location.SpeedY = -8 + NPC[p.StandingOnNPC].Location.SpeedY * 0.3;
                            NPC[numNPCs].Location.Y -= 24;
                            NPC[numNPCs].Location.X += -6 * p.Direction;
                            if(p.Character == 3)
                            {
                                NPC[numNPCs].Location.SpeedY += 1;
                                NPC[numNPCs].Location.SpeedX = NPC[numNPCs].Location.SpeedX * 1.5;
                            }
                            else if(p.Character == 4)
                            {
                                NPC[numNPCs].Location.SpeedY = -8;
                                NPC[numNPCs].Location.SpeedX = 12 * p.Direction + p.Location.SpeedX;
                            }
                        }
                        else
                        {
                            NPC[numNPCs].Location.SpeedX = 4 * p.Direction + p.Location.SpeedX * 0.9;
                            if(p.StandingOnNPC == 0)
                                NPC[numNPCs].Location.SpeedY = -5 + p.Location.SpeedY * 0.3;
                            else
                                NPC[numNPCs].Location.SpeedY = -5 + NPC[p.StandingOnNPC].Location.SpeedY * 0.3;
                            if(p.Character == 3)
                                NPC[numNPCs].Location.SpeedY += 1;
                            else if(p.Character == 4)
                            {
                                NPC[numNPCs].Location.SpeedY = -5;
                                NPC[numNPCs].Location.SpeedX = 10 * p.Direction + p.Location.SpeedX;
                                NPC[numNPCs].Location.Y -= 12;
                            }
                        }
                        if(p.Character == 4)
                            NPC[numNPCs].Location.X = p.Location.X + p.Location.Width / 2.0 - NPC[numNPCs].Location.Width / 2.0;
                        syncLayers_NPC(numNPCs);
                        CheckSectionNPC(numNPCs);
                    }
                }
            }
        }




// Fire Mario / Luigi code ---- FIRE FLOWER ACTION BALLS OF DOOM
        if(!p.Slide && p.Vine == 0 && (p.State == 3 || p.State == 7) && !p.Duck && p.Mount != 2 && p.Mount != 3 && p.HoldingNPC <= 0 && p.Character != 5)
        {
            if(((p.Controls.Run && !p.SpinJump) || (p.SpinJump && p.Direction != p.SpinFireDir)) && p.FireBallCD <= 0)
            {
                if((p.RunRelease || p.SpinJump) || (FlameThrower && p.HoldingNPC <= 0))
                {
                    if(p.SpinJump)
                        p.SpinFireDir = p.Direction;

                    if(numNPCs < maxNPCs - 100)
                    {
//                        if(nPlay.Online && A - 1 == nPlay.MySlot)
//                            Netplay::sendData Netplay::PutPlayerControls(nPlay.MySlot) + "1f" + std::to_string(A) + "|" + p.FireBallCD - 1;
                        if(!p.SpinJump)
                            p.FrameCount = 110;

                        numNPCs++;
                        NPC[numNPCs] = NPC_t();
                        if(ShadowMode)
                            NPC[numNPCs].Shadow = true;
                        NPC[numNPCs].Type = 13;
                        if(p.State == 7)
                            NPC[numNPCs].Type = 265;
                        NPC[numNPCs].Projectile = true;
                        NPC[numNPCs].Location.Height = NPCHeight[NPC[numNPCs].Type];
                        NPC[numNPCs].Location.Width = NPCWidth[NPC[numNPCs].Type];
                        NPC[numNPCs].Location.X = p.Location.X + Physics.PlayerGrabSpotX[p.Character][p.State] * p.Direction + 4;
                        NPC[numNPCs].Location.Y = p.Location.Y + Physics.PlayerGrabSpotY[p.Character][p.State];
                        NPC[numNPCs].Active = true;
                        NPC[numNPCs].TimeLeft = 100;
                        NPC[numNPCs].Location.SpeedY = 20;
                        NPC[numNPCs].CantHurt = 100;
                        NPC[numNPCs].CantHurtPlayer = A;
                        NPC[numNPCs].Special = p.Character;

                        if(p.State == 7)
                            NPC[numNPCs].Special = 1;

                        if((p.Character == 3 || p.Character == 4) && p.Mount == 0 && p.Controls.AltRun) // peach holds fireballs
                        {
                            p.HoldingNPC = numNPCs;
                            NPC[numNPCs].HoldingPlayer = A;
                        }

                        if(Maths::iRound(NPC[numNPCs].Special) == 2)
                            NPC[numNPCs].Frame = 4;
                        if(Maths::iRound(NPC[numNPCs].Special) == 3)
                            NPC[numNPCs].Frame = 8;
                        if(Maths::iRound(NPC[numNPCs].Special) == 4)
                            NPC[numNPCs].Frame = 12;

                        syncLayers_NPC(numNPCs);
                        CheckSectionNPC(numNPCs);
                        p.FireBallCD = 30;
                        if(p.Character == 2)
                            p.FireBallCD = 35;
                        if(p.Character == 3)
                            p.FireBallCD = 40;
                        if(p.Character == 4)
                            p.FireBallCD = 25;

                        NPC[numNPCs].Location.SpeedX = 5 * p.Direction + (p.Location.SpeedX / 3.5);

                        if(p.State == 7)
                        {
                            NPC[numNPCs].Location.SpeedY = 5;

                            if(p.Controls.Up)
                            {
                                if(p.StandingOnNPC != 0)
                                    NPC[numNPCs].Location.SpeedY = -8 + NPC[p.StandingOnNPC].Location.SpeedY * 0.1;
                                else
                                    NPC[numNPCs].Location.SpeedY = -8 + p.Location.SpeedY * 0.1;
                                NPC[numNPCs].Location.SpeedX = NPC[numNPCs].Location.SpeedX * 0.9;
                            }
                            if(FlameThrower)
                            {
                                NPC[numNPCs].Location.SpeedX = NPC[numNPCs].Location.SpeedX * 1.5;
                                NPC[numNPCs].Location.SpeedY = NPC[numNPCs].Location.SpeedY * 1.5;
                            }
                            if(p.StandingOnNPC != 0)
                                NPC[numNPCs].Location.SpeedX = 5 * p.Direction + (p.Location.SpeedX / 3.5) + NPC[p.StandingOnNPC].Location.SpeedX / 3.5;
                            PlaySound(SFX_Iceball);
                            NPC[numNPCs].Location.SpeedX = NPC[numNPCs].Location.SpeedX * 0.8;
                        }
                        else
                        {
                            if(Maths::iRound(NPC[numNPCs].Special) == 2)
                                NPC[numNPCs].Location.SpeedX = NPC[numNPCs].Location.SpeedX * 0.85;

                            if(p.Controls.Up)
                            {
                                if(p.StandingOnNPC != 0)
                                    NPC[numNPCs].Location.SpeedY = -6 + NPC[p.StandingOnNPC].Location.SpeedY * 0.1;
                                else
                                    NPC[numNPCs].Location.SpeedY = -6 + p.Location.SpeedY * 0.1;
                                NPC[numNPCs].Location.SpeedX = NPC[numNPCs].Location.SpeedX * 0.9;
                            }

                            if(FlameThrower)
                            {
                                NPC[numNPCs].Location.SpeedX = NPC[numNPCs].Location.SpeedX * 1.5;
                                NPC[numNPCs].Location.SpeedY = NPC[numNPCs].Location.SpeedY * 1.5;
                            }

                            if(p.StandingOnNPC != 0)
                                NPC[numNPCs].Location.SpeedX = 5 * p.Direction + (p.Location.SpeedX / 3.5) + NPC[p.StandingOnNPC].Location.SpeedX / 3.5;
                            PlaySound(SFX_Fireball);
                        }
                    }
                }
            }
        }
// RacoonMario
        if(!p.Slide && p.Vine == 0 && (p.State == 4 || p.State == 5) && !p.Duck && p.HoldingNPC == 0 && p.Mount != 2 && !p.Stoned && p.Effect == 0 && p.Character != 5)
        {
             if(p.Controls.Run || p.SpinJump)
             {
                if(p.TailCount == 0 || p.TailCount >= 12)
                {
                    if(p.RunRelease || p.SpinJump)
                    {
                        p.TailCount = 1;
                        if(!p.SpinJump)
                            PlaySound(SFX_Tail);
                    }
                }
             }
        }

        if(p.TailCount > 0)
        {
            p.TailCount += 1;
            if(p.TailCount == 25)
                p.TailCount = 0;
            if(p.TailCount % 7 == 0 || (p.SpinJump && p.TailCount) % 2 == 0)
            {
                TailSwipe(A, true);
            }
            else
            {
                TailSwipe(A);
            }
            if(p.HoldingNPC > 0)
                p.TailCount = 0;
        }


// link stab
    if(p.Character == 5 && p.Vine == 0 && p.Mount == 0 && !p.Stoned && p.FireBallCD == 0)
    {
        if(p.Bombs > 0 && p.Controls.AltRun && p.RunRelease)
        {
            p.FireBallCD = 10;
            p.Bombs -= 1;
            numNPCs++;
            NPC[numNPCs] = NPC_t();
            NPC[numNPCs].Active = true;
            NPC[numNPCs].TimeLeft = Physics.NPCTimeOffScreen;
            NPC[numNPCs].Section = p.Section;
            NPC[numNPCs].Type = 134;
            NPC[numNPCs].Location.Width = NPCWidth[NPC[numNPCs].Type];
            NPC[numNPCs].Location.Height = NPCHeight[NPC[numNPCs].Type];
            NPC[numNPCs].CantHurtPlayer = A;
            NPC[numNPCs].CantHurt = 1000;

            if(p.Duck && (p.Location.SpeedY == 0 || p.Slope > 0 || p.StandingOnNPC != 0))
            {
                NPC[numNPCs].Location.X = p.Location.X + p.Location.Width / 2.0 - NPC[numNPCs].Location.Width / 2.0;
                NPC[numNPCs].Location.Y = p.Location.Y + p.Location.Height - NPC[numNPCs].Location.Height;
                NPC[numNPCs].Location.SpeedX = 0;
                NPC[numNPCs].Location.SpeedY = 0;
                PlaySound(SFX_Grab);
            }
            else
            {
                NPC[numNPCs].Location.X = p.Location.X + p.Location.Width / 2.0 - NPC[numNPCs].Location.Width / 2.0;
                NPC[numNPCs].Location.Y = p.Location.Y;
                NPC[numNPCs].Location.SpeedX = 5 * p.Direction;
                NPC[numNPCs].Location.SpeedY = -6;
                NPC[numNPCs].Projectile = true;
                if(p.Location.SpeedY == 0 || p.Slope > 0 || p.StandingOnNPC != 0)
                    p.SwordPoke = -10;
                PlaySound(SFX_Throw);
            }

            syncLayers_NPC(numNPCs);
        }
        else if(p.FireBallCD == 0 && p.Controls.Run && p.RunRelease)
        {
            p.FireBallCD = 20;
            if(p.Location.SpeedY != Physics.PlayerGravity && p.StandingOnNPC == 0 && p.Slope == 0) // Link ducks when jumping
            {
                if(p.Wet == 0 && !p.WetFrame)
                {
                    if(p.Controls.Down && !p.Duck && p.Mount == 0)
                    {
                        p.Duck = true;
                        p.Location.Y += p.Location.Height;
                        p.Location.Height = Physics.PlayerDuckHeight[p.Character][p.State];
                        p.Location.Y += -p.Location.Height;
                    }
                    else if(!p.Controls.Down && p.Duck)
                    {
                        UnDuck(Player[A]);
                    }
                }
            }
            if(p.Duck)
                p.SwordPoke = 1;
            else
                p.SwordPoke = -1;
        }
        else if(p.Controls.Up && p.Location.SpeedY < 0 && !p.Duck && p.SwordPoke == 0) // Link stabs up
        {
            if(!p.WetFrame && p.Frame == 10)
            {
                TailSwipe(A, true, true, 1);
            }
        }
        else if(p.Controls.Down && (p.Location.SpeedY > 0 && p.StandingOnNPC == 0 && p.Slope == 0) && !p.Duck && p.SwordPoke == 0) // Link stabs down
        {
            if(!p.WetFrame && p.Frame == 9)
            {
                TailSwipe(A, true, true, 2);
            }
        }
    }


// cooldown timer
    p.FireBallCD2 -= 1;
    if(p.FireBallCD2 < 0)
        p.FireBallCD2 = 0;

    if(!(p.Character == 3 && NPC[p.HoldingNPC].Type == 13))
    {
        p.FireBallCD -= 1;
        if(FlameThrower)
            p.FireBallCD -= 3;
        if(p.FireBallCD < 0)
            p.FireBallCD = 0;
    }
}


static SDL_INLINE bool checkWarp(Warp_t &warp, int B, Player_t &plr, int A, bool backward)
{
    bool canWarp = false;

    bool onGround = (!g_compatibility.require_ground_to_enter_warps && !warp.stoodRequired) ||
                    (plr.Pinched1 == 2 || plr.Slope != 0 || plr.StandingOnNPC != 0);

    auto &entrance      = backward ? warp.Exit        : warp.Entrance;
    auto &exit          = backward ? warp.Entrance    : warp.Exit;
    auto &direction     = backward ? warp.Direction2  : warp.Direction;

    if(!CheckCollision(plr.Location, entrance))
        return false; // continue

    plr.ShowWarp = B;

    if(warp.Effect == 3) // Portal
        canWarp = true;
    else if(direction == 1 && plr.Controls.Up) // Pipe
    {
        if(WarpCollision(plr.Location, entrance, direction) && (warp.Effect != 2 || onGround))
            canWarp = true;
    }
    else if(direction == 2 && plr.Controls.Left)
    {
        if(WarpCollision(plr.Location, entrance, direction) && onGround)
            canWarp = true;
    }
    else if(direction == 3 && plr.Controls.Down)
    {
        if(WarpCollision(plr.Location, entrance, direction) && onGround)
            canWarp = true;
    }
    else if(direction == 4 && plr.Controls.Right)
    {
        if(WarpCollision(plr.Location, entrance, direction) && onGround)
            canWarp = true;
    }
    // NOTE: Would be correct to move this up, but leave this here for a compatibility to keep the same behavior
    else if(warp.Effect == 0) // Instant
        canWarp = true;

    if(warp.LevelEnt)
        canWarp = false;

    if(warp.Stars > numStars && canWarp)
    {
        if(warp.StarsMsg == STRINGINDEX_NONE)
            MessageText = (warp.Stars == 1) ?
                              "You need 1 star to enter." :
                              fmt::format_ne("You need {0} stars to enter.", warp.Stars);
        else
            MessageText = GetS(warp.StarsMsg);

        PauseGame(PauseCode::Message, A);
        MessageText.clear();
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
                for(int C = numBackground + 1; C <= allBGOs; C++)
                {
                    if(Background[C].Type == 98)
                    {
                        if(CheckCollision(entrance, Background[C].Location) ||
                           (warp.twoWay && CheckCollision(exit, Background[C].Location)))
                        {
                            // this makes Background[C] disappear and never reappear
                            Background[C].Layer = LAYER_NONE;
                            Background[C].Hidden = true;
                            syncLayers_BGO(C);
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

                for(int C = numBackground + 1; C <= allBGOs; C++)
                {
                    if(Background[C].Type == 98)
                    {
                        if(CheckCollision(entrance, Background[C].Location) ||
                           (warp.twoWay && CheckCollision(exit, Background[C].Location)))
                        {
                            // this makes Background[C] disappear and never reappear
                            Background[C].Layer = LAYER_NONE;
                            Background[C].Hidden = true;
                            syncLayers_BGO(C);
                        }
                    }
                }
            }
            else if(plr.HasKey)
            {
                plr.HasKey = false;
                warp.Locked = false;
                int allBGOs = numBackground + numLocked;
                for(int C = numBackground + 1; C <= allBGOs; C++)
                {
                    if(Background[C].Type == 98)
                    {
                        if(CheckCollision(entrance, Background[C].Location) ||
                           (warp.twoWay && CheckCollision(exit, Background[C].Location)))
                        {
                            // this makes Background[C] disappear and never reappear
                            Background[C].Layer = LAYER_NONE;
                            Background[C].Hidden = true;
                            syncLayers_BGO(C);
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
        UnDuck(Player[A]);
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
              (plr.Character == 4 && warp.Effect == 1 && direction == 1))
                NPC[plr.HoldingNPC].Location.Y = entrance.Y;
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

        if(warp.eventEnter != EVENT_NONE)
            ProcEvent(warp.eventEnter);

        if(warp.Effect == 0 || warp.Effect == 3) // Instant / Portal
        {
            if(warp.Effect == 3)
            {
                if(warp.level != STRINGINDEX_NONE)
                {
                    GoToLevel = GetS(warp.level);
                    GoToLevelNoGameThing = warp.noEntranceScene;
                    plr.Effect = 8;
                    plr.Effect2 = 2921;
                    plr.Warp = B;
                    plr.WarpBackward = backward;
                    ReturnWarp = B;
                    if(IsEpisodeIntro && NoMap)
                        ReturnWarpSaved = ReturnWarp;
                    StartWarp = warp.LevelWarp;
                    return true;
                }
                else if(warp.MapWarp)
                {
                    plr.Effect = 8;
                    plr.Effect2 = 2921;
                    plr.Warp = B;
                    plr.WarpBackward = backward;
                    return true;
                }
            }

            plr.Location.X = exit.X + exit.Width / 2.0 - plr.Location.Width / 2.0;
            plr.Location.Y = exit.Y + exit.Height - plr.Location.Height - 0.1;
            CheckSection(A);
            plr.WarpCD = (warp.Effect == 3) ? 10 : 50;
            return true; // break
        }
        else if(warp.Effect == 1) // Pipe
        {
            PlaySound(SFX_Warp);
            plr.Effect = 3;
            if(g_compatibility.fix_fairy_stuck_in_pipe)
                plr.Effect2 = 0;
            plr.Warp = B;
            plr.WarpBackward = backward;
//                        if(nPlay.Online && A == nPlay.MySlot + 1)
//                            Netplay::sendData Netplay::PutPlayerLoc(nPlay.MySlot) + "1j" + std::to_string(A) + "|" + plr.Warp + LB;
        }
        else if(warp.Effect == 2) // Door
        {
            PlaySound(SFX_Door);
            plr.Effect = 7;
            if(g_compatibility.fix_fairy_stuck_in_pipe)
                plr.Effect2 = 0;
            plr.Warp = B;
            plr.WarpBackward = backward;
//                        if(nPlay.Online && A == nPlay.MySlot + 1)
//                            Netplay::sendData Netplay::PutPlayerLoc(nPlay.MySlot) + "1j" + std::to_string(A) + "|" + plr.Warp + LB;
            plr.Location.X = entrance.X + entrance.Width / 2.0 - plr.Location.Width / 2.0;
            plr.Location.Y = entrance.Y + entrance.Height - plr.Location.Height;

            for(int C = 1; C <= numBackground; C++)
            {
                if(CheckCollision(entrance, Background[C].Location) || CheckCollision(exit, Background[C].Location))
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
                        bLoc.X += bLoc.Width / 2.0;
                        bLoc.Width = 104;
                        bLoc.X += -bLoc.Width / 2.0;
                        NewEffect(103, bLoc);
                    }
                }
            }
        }
    }

    return false; // continue
}

void SuperWarp(const int A)
{
    auto &plr = Player[A];

    if(plr.WarpCD <= 0 && plr.Mount != 2 && !plr.GroundPound && !plr.GroundPound2)
    {
        for(int B = 1; B <= numWarps; B++)
        {
            auto &warp = Warp[B];

            if(warp.Hidden)
                continue;

            if(checkWarp(warp, B, plr, A, false))
                break;

            if(warp.twoWay) // Check the same warp again if two-way
            {
                if(checkWarp(warp, B, plr, A, true))
                    break;
            }
        }
    }
    else if(plr.Mount != 2)
        plr.WarpCD--;
}

void PlayerCollide(const int A)
{
    Location_t tempLocation;
    Location_t tempLocation3;
    int HitSpot = 0;
    auto &p1 = Player[A];

// Check player collisions
    for(int B = 1; B <= numPlayers; B++)
    {
        auto &p2 = Player[B];

        if(B != A && !p2.Dead && p2.TimeToLive == 0 &&
           (p2.Effect == 0 || p2.Effect == 3) &&
           !(p2.Mount == 2 || p1.Mount == 2) &&
           (!BattleMode || (p1.Immune == 0 && p2.Immune == 0)))
        {
            tempLocation = p1.Location;

            if(p1.StandingOnNPC != 0 && !FreezeNPCs)
                tempLocation.SpeedY = NPC[p1.StandingOnNPC].Location.SpeedY;

            tempLocation3 = p2.Location;

            if(p2.StandingOnNPC != 0 && !FreezeNPCs)
                tempLocation3.SpeedY = NPC[p2.StandingOnNPC].Location.SpeedY;

            if(CheckCollision(tempLocation, tempLocation3))
            {
                HitSpot = FindCollision(tempLocation, tempLocation3);
                if(HitSpot == 5)
                {
                    if(p1.StandUp2 && p1.Location.Y > p2.Location.Y)
                        HitSpot = 3;
                    else if(p2.StandUp2 && p1.Location.Y < p2.Location.Y)
                        HitSpot = 1;
                }

                if(BattleMode)
                {
                    if(p2.SlideKill)
                    {
                        HitSpot = 0;
                        PlayerHurt(A);
                    }
                    if(p1.SlideKill)
                    {
                        HitSpot = 0;
                        PlayerHurt(B);
                    }
                    if(p1.Stoned && (p1.Location.SpeedX > 3 || p1.Location.SpeedX < -3 || HitSpot == 1))
                    {
                        PlayerHurt(B);
                    }
                    else if(p2.Stoned && (p2.Location.SpeedX > 3 || p2.Location.SpeedX < -3 || HitSpot == 3))
                    {
                        PlayerHurt(A);
                    }
                }

                if(p1.Stoned || p2.Stoned)
                    HitSpot = 0;
                if(HitSpot == 2 || HitSpot == 4)
                {
                    if(numPlayers < 3 /*|| nPlay.Online*/)
                        PlaySound(SFX_Skid);
                    tempLocation = p1.Location;
                    p1.Location.SpeedX = p2.Location.SpeedX;
                    p2.Location.SpeedX = tempLocation.SpeedX;
                    p1.Bumped = true;
                    p2.Bumped = true;
                }
                else if(HitSpot == 1)
                {
                    if(numPlayers < 3 /*|| nPlay.Online*/)
                        PlaySound(SFX_Stomp);
                    p1.Location.Y = p2.Location.Y - p1.Location.Height - 0.1;
                    PlayerPush(A, 3);
                    p1.Location.SpeedY = Physics.PlayerJumpVelocity;
                    p1.Jump = Physics.PlayerHeadJumpHeight;
                    if(p1.Character == 2)
                        p1.Jump += 3;
                    if(p1.SpinJump)
                        p1.Jump -= 6;
                    p2.Jump = 0;
                    if(p2.Location.SpeedY <= 0)
                        p2.Location.SpeedY = 0.1;
                    p2.CanJump = false;
                    NewEffect(75, newLoc(p1.Location.X + p1.Location.Width / 2.0 - 16, p1.Location.Y + p1.Location.Height - 16));
                }
                else if(HitSpot == 3)
                {
                    if(numPlayers < 3/* || nPlay.Online*/)
                        PlaySound(SFX_Stomp);
                    p2.Location.Y = p1.Location.Y - p2.Location.Height - 0.1;
                    PlayerPush(B, 3);
                    p2.Location.SpeedY = Physics.PlayerJumpVelocity;
                    p2.Jump = Physics.PlayerHeadJumpHeight;
                    if(p2.Character == 2)
                        p1.Jump += 3;
                    if(p1.SpinJump)
                        p1.Jump -= 6;
                    p1.Jump = 0;
                    if(p1.Location.SpeedY <= 0)
                        p1.Location.SpeedY = 0.1;
                    p1.CanJump = false;
                    NewEffect(75, newLoc(p2.Location.X + p2.Location.Width / 2.0 - 16, p2.Location.Y + p2.Location.Height - 16));
                }
                else if(HitSpot == 5)
                {
                    if(p1.Location.X + p1.Location.Width / 2.0 > p2.Location.X + p2.Location.Width / 2.0)
                    {
                        p1.Bumped2 = 1;
                        p2.Bumped2 = -1;
                    }
                    else if(p1.Location.X + p1.Location.Width / 2.0 < p2.Location.X + p2.Location.Width / 2.0)
                    {
                        p1.Bumped2 = -1;
                        p2.Bumped2 = 1;
                    }
                    else if(iRand(2) == 0)
                    {
                        p1.Bumped2 = -1;
                        p2.Bumped2 = 1;
                    }
                    else
                    {
                        p1.Bumped2 = 1;
                        p2.Bumped2 = -1;
                    }
                    // If Player(A).Bumped2 < -1 Then Player(A).Bumped2 = -1 - Rnd
                    // If Player(A).Bumped2 > 1 Then Player(A).Bumped2 = 1 + Rnd
                    // If Player(B).Bumped2 < -1 Then Player(B).Bumped2 = -1 - Rnd
                    // If Player(B).Bumped2 > 1 Then Player(B).Bumped2 = 1 + Rnd
                }
                if(BattleMode)
                {
                    if(HitSpot == 1 && p1.Mount == 1)
                    {
                        PlayerHurt(B);
                    }
                    else if(HitSpot == 3 && p2.Mount == 1)
                    {
                        PlayerHurt(A);
                    }
                }
            }
        }
    }
}

void PlayerGrabCode(const int A, bool DontResetGrabTime)
{
    // this code handles all the grab related stuff
    // for grabbing something while walking into it, refer to the NPC collision code in sub UpdatePlayer()
    Location_t tempLocation;
    int LayerNPC = 0;
    int B = 0;
    bool tempBool = false;
    double lyrX = 0;
    double lyrY = 0;
    auto &p = Player[A];

    if(p.StandingOnNPC != 0 && p.HoldingNPC == 0)
    {
        if(NPCGrabFromTop[NPC[p.StandingOnNPC].Type])
        {
            if(((p.Controls.Run && p.Controls.Down) || ((p.Controls.Down || p.Controls.Run) && p.GrabTime > 0)) && (p.RunRelease || p.GrabTime > 0) && p.TailCount == 0)
            {
                if((p.GrabTime >= 12 && p.Character < 3) || (p.GrabTime >= 16 && p.Character == 3) || (p.GrabTime >= 8 && p.Character == 4))
                {
                    p.Location.SpeedX = p.GrabSpeed;
                    p.GrabSpeed = 0;
                    p.GrabTime = 0;
                    p.TailCount = 0;

                    if(p.Character == 1 || p.Character == 2)
                    {
                        UnDuck(Player[A]);
                    }

                    p.HoldingNPC = p.StandingOnNPC;
                    p.Location.SpeedY = NPC[p.StandingOnNPC].Location.SpeedY;
                    if(p.Location.SpeedY == 0)
                        p.Location.SpeedY = 0.01;
                    p.CanJump = false;
                    if(NPCIsAShell[NPC[p.StandingOnNPC].Type])
                        p.Location.SpeedX = NPC[p.StandingOnNPC].Location.SpeedX;
                    NPC[p.StandingOnNPC].HoldingPlayer = A;
                    NPC[p.StandingOnNPC].CantHurt = Physics.NPCCanHurtWait;
                    NPC[p.StandingOnNPC].CantHurtPlayer = A;
                    NPC[p.StandingOnNPC].Direction = p.Direction;
                    NPCFrames(p.StandingOnNPC);

                    if(NPC[p.StandingOnNPC].Type == 91)
                    {
                        p.Location.SpeedX += NPC[p.StandingOnNPC].Location.SpeedX;
                        NPC[p.StandingOnNPC].Direction = p.Direction;
                        NPC[p.StandingOnNPC].Generator = false;
                        NPC[p.StandingOnNPC].Frame = 0;
                        NPC[p.StandingOnNPC].Frame = EditorNPCFrame(NPC[p.StandingOnNPC].Type, NPC[p.StandingOnNPC].Direction);
                        NPC[p.StandingOnNPC].Type = NPC[p.StandingOnNPC].Special;
                        if(NPC[p.StandingOnNPC].Type == 287)
                        {
                            NPC[p.StandingOnNPC].Type = RandomBonus();
                            NPC[p.StandingOnNPC].DefaultSpecial = NPC[p.StandingOnNPC].Type;
                        }
                        CharStuff(p.StandingOnNPC);
                        NPC[p.StandingOnNPC].Special = 0;
                        if(NPCIsYoshi[NPC[p.StandingOnNPC].Type])
                        {
                            NPC[p.StandingOnNPC].Special = NPC[p.StandingOnNPC].Type;
                            NPC[p.StandingOnNPC].Type = 96;
                        }
                        if(!(NPC[p.StandingOnNPC].Type == 21 || NPC[p.StandingOnNPC].Type == 22 || NPC[p.StandingOnNPC].Type == 26 || NPC[p.StandingOnNPC].Type == 31 || NPC[p.StandingOnNPC].Type == 32 || NPC[p.StandingOnNPC].Type == 35 || NPC[p.StandingOnNPC].Type == 191 || NPC[p.StandingOnNPC].Type == 193 || NPC[p.StandingOnNPC].Type == 49 || NPCIsAnExit[NPC[p.StandingOnNPC].Type]))
                        {
                            if(!BattleMode)
                                NPC[p.StandingOnNPC].DefaultType = 0;
                        }
                        NPC[p.StandingOnNPC].Location.Height = NPCHeight[NPC[p.StandingOnNPC].Type];
                        NPC[p.StandingOnNPC].Location.Width = NPCWidth[NPC[p.StandingOnNPC].Type];
                        if(NPC[p.StandingOnNPC].Type == 147)
                        {
                            B = iRand(9);
                            NPC[p.StandingOnNPC].Type = 139 + B;
                            if(NPC[p.StandingOnNPC].Type == 147)
                                NPC[p.StandingOnNPC].Type = 92;
                            NPC[p.StandingOnNPC].Location.X += NPC[p.StandingOnNPC].Location.Width / 2.0;
                            NPC[p.StandingOnNPC].Location.Y += NPC[p.StandingOnNPC].Location.Height / 2.0;
                            NPC[p.StandingOnNPC].Location.Width = NPCWidth[NPC[p.StandingOnNPC].Type];
                            NPC[p.StandingOnNPC].Location.Height = NPCHeight[NPC[p.StandingOnNPC].Type];
                            NPC[p.StandingOnNPC].Location.X += -NPC[p.StandingOnNPC].Location.Width / 2.0;
                            NPC[p.StandingOnNPC].Location.Y += -NPC[p.StandingOnNPC].Location.Height / 2.0;
                        }
                        NPCFrames(p.StandingOnNPC);
                        p.StandingOnNPC = 0;
                    }
                }
                else
                {
                    if(p.GrabTime == 0)
                    {
                        if(NPC[p.StandingOnNPC].Type == 91 || NPCIsVeggie[NPC[p.StandingOnNPC].Type])
                            PlaySound(SFX_Grab2);
                        else
                            PlaySound(SFX_Grab);
                        p.FrameCount = 0;
                        p.GrabSpeed = p.Location.SpeedX;
                    }
                    p.Location.SpeedX = 0;
                    p.GrabTime += 1;
                    p.Slide = false;
                }
            }
            else
                p.GrabTime = 0;
        }
        else
            p.GrabTime = 0;
    }
    else if(!DontResetGrabTime)
        p.GrabTime = 0;
    if(p.HoldingNPC > numNPCs) // Can't hold an NPC that is dead
        p.HoldingNPC = 0;
    if(p.HoldingNPC > 0)
    {
        lyrX = NPC[p.HoldingNPC].Location.X;
        lyrY = NPC[p.HoldingNPC].Location.Y;
        LayerNPC = p.HoldingNPC;
        if(NPC[p.HoldingNPC].Type == 278 || NPC[p.HoldingNPC].Type == 279)
        {
            if(p.Jump == 0)
                NPC[p.HoldingNPC].Special = 1;
            else if(p.Jump > 0 && NPC[p.HoldingNPC].Special == 1)
            {
                p.Jump = p.Jump * 1.5;
                NPC[p.HoldingNPC].Special = 0;
            }
            if(NPC[p.HoldingNPC].Type == 279)
            {
                NPC[p.HoldingNPC].Special2 += 1;
                if(p.SpinJump)
                {
                    if(NPC[p.HoldingNPC].Special3 == 0)
                        NPC[p.HoldingNPC].Special3 = p.Direction;
                    else if(NPC[p.HoldingNPC].Special3 == -p.Direction && NPC[p.HoldingNPC].Special2 >= 25)
                    {
                        NPC[p.HoldingNPC].Special3 = p.Direction;
                        NPC[p.HoldingNPC].Special2 = 100;
                    }
                    else if(NPC[p.HoldingNPC].Special2 >= 25)
                        NPC[p.HoldingNPC].Special2 = 25;

                }


                if(NPC[p.HoldingNPC].Special2 == 20 || NPC[p.HoldingNPC].Special2 == 40 || NPC[p.HoldingNPC].Special2 == 60 || NPC[p.HoldingNPC].Special2 == 80)
                {
                    if(NPC[p.HoldingNPC].Special2 == 20 || NPC[p.HoldingNPC].Special2 == 60)
                        B = 1;
                    else if(NPC[p.HoldingNPC].Special2 == 40)
                        B = 2;
                    else if(NPC[p.HoldingNPC].Special2 == 80)
                    {
                        B = 3;
                        NPC[p.HoldingNPC].Special2 = 0;
                    }

                        PlaySound(SFX_ZeldaFire);

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
                        NPC[numNPCs].TimeLeft = NPC[p.HoldingNPC].TimeLeft;
                        NPC[numNPCs].Layer = LAYER_SPAWNED_NPCS;
                        NPC[numNPCs].Location.Y = NPC[p.HoldingNPC].Location.Y + NPC[p.HoldingNPC].Location.Height - NPC[numNPCs].Location.Height;
                        NPC[numNPCs].Direction = p.Direction;
                        if(NPC[numNPCs].Direction == 1)
                            NPC[numNPCs].Location.X = NPC[p.HoldingNPC].Location.X + NPC[p.HoldingNPC].Location.Width * 2 - 8;
                        else
                            NPC[numNPCs].Location.X = NPC[p.HoldingNPC].Location.X - NPC[numNPCs].Location.Width - NPC[p.HoldingNPC].Location.Width + 8;

                        if(B == 1)
                            NPC[numNPCs].Location.SpeedX = 7 * NPC[numNPCs].Direction + (p.Location.SpeedX / 3.5);
                        else if(B == 2)
                        {
                            NPC[numNPCs].Location.SpeedX = 6.5 * NPC[numNPCs].Direction + (p.Location.SpeedX / 3.5);
                            NPC[numNPCs].Location.SpeedY = -1.5;
                        }
                        else
                        {
                            NPC[numNPCs].Location.SpeedX = 6.5 * NPC[numNPCs].Direction + (p.Location.SpeedX / 3.5);
                            NPC[numNPCs].Location.SpeedY = 1.5;
                        }

                        NPC[numNPCs].Projectile = true;
                        NPC[numNPCs].Frame = EditorNPCFrame(NPC[numNPCs].Type, NPC[numNPCs].Direction);

                        syncLayers_NPC(numNPCs);
                    // Next B
                }
            }
        }

        if(NPC[p.HoldingNPC].Type == 263)
            NPC[p.HoldingNPC].Special3 = 0;
        NPC[p.HoldingNPC].TimeLeft = Physics.NPCTimeOffScreen;
        NPC[p.HoldingNPC].Effect = 0;
        NPC[p.HoldingNPC].CantHurt = Physics.NPCCanHurtWait;
        NPC[p.HoldingNPC].CantHurtPlayer = A;
        if(NPCIsVeggie[NPC[p.HoldingNPC].Type])
            NPC[p.HoldingNPC].CantHurt = 1000;
        if(p.Controls.Run || p.ForceHold > 0)
        {

        // hold above head
            if(p.Character == 3 || p.Character == 4 || (p.Duck))
            {
                NPC[p.HoldingNPC].Bouce = true;
                NPC[p.HoldingNPC].Location.X = p.Location.X + p.Location.Width / 2.0 - NPC[p.HoldingNPC].Location.Width / 2.0;
                if(p.Character == 3) // princess peach
                {
                    if(p.State == 1)
                        NPC[p.HoldingNPC].Location.Y = p.Location.Y - NPC[p.HoldingNPC].Location.Height;
                    else
                        NPC[p.HoldingNPC].Location.Y = p.Location.Y - NPC[p.HoldingNPC].Location.Height + 6;
                }
                else // toad
                {
                    if(p.State == 1)
                        NPC[p.HoldingNPC].Location.Y = p.Location.Y - NPC[p.HoldingNPC].Location.Height + 6;
                    else
                    {
                        if(NPC[p.HoldingNPC].Type == 13 || NPC[p.HoldingNPC].Type == 265)
                        {
                            NPC[p.HoldingNPC].Location.X = p.Location.X + p.Location.Width / 2.0 - NPC[p.HoldingNPC].Location.Width / 2.0 + dRand() * 4 - 2;
                            NPC[p.HoldingNPC].Location.Y = p.Location.Y - NPC[p.HoldingNPC].Location.Height - 4 + dRand() * 4 - 2;
                        }
                        else
                            NPC[p.HoldingNPC].Location.Y = p.Location.Y - NPC[p.HoldingNPC].Location.Height + 10;
                    }
                }
            }
            else
            {
                if(p.Direction > 0)
                    NPC[p.HoldingNPC].Location.X = p.Location.X + Physics.PlayerGrabSpotX[p.Character][p.State];
                else
                    NPC[p.HoldingNPC].Location.X = p.Location.X + p.Location.Width - Physics.PlayerGrabSpotX[p.Character][p.State] - NPC[p.HoldingNPC].Location.Width;
                NPC[p.HoldingNPC].Location.Y = p.Location.Y + Physics.PlayerGrabSpotY[p.Character][p.State] + 32 - NPC[p.HoldingNPC].Location.Height;
            }
            if(NPC[p.HoldingNPC].Type == 49 && !FreezeNPCs)
            {
                if(NPC[p.HoldingNPC].Special == 0.0)
                {
                    NPC[p.HoldingNPC].Special = 1;
                    NPC[p.HoldingNPC].Special2 = numNPCs + 1;
                    numNPCs++;
                    NPC[numNPCs] = NPC_t();
                    NPC[numNPCs].Active = true;
                    NPC[numNPCs].Section = p.Section;
                    NPC[numNPCs].TimeLeft = 100;
                    NPC[numNPCs].Type = 50;
                    NPC[numNPCs].Location.Height = 32;
                    NPC[numNPCs].Location.Width = 48;
                    NPC[numNPCs].Special = A;
                    if(p.Direction > 0)
                        NPC[numNPCs].Frame = 2;
                    syncLayers_NPC(numNPCs);
                }
                for(B = 1; B <= numNPCs; B++)
                {
                    if(NPC[B].Type == 50 && NPC[B].Special == A)
                    {
                        NPC[B].CantHurt = 10;
                        NPC[B].CantHurtPlayer = A;
                        NPC[B].Projectile = true;
                        NPC[B].Direction = p.Direction;
                        NPC[B].TimeLeft = 100;
                        if(p.Direction > 0)
                            NPC[B].Location.X = NPC[p.HoldingNPC].Location.X + 32;
                        else
                            NPC[B].Location.X = NPC[p.HoldingNPC].Location.X - NPC[B].Location.Width;
                        NPC[B].Location.Y = NPC[p.HoldingNPC].Location.Y;
                    }
                }
            }
        }
        else
        {
            NPC[p.HoldingNPC].Location.SpeedX = 0;
            NPC[p.HoldingNPC].Location.SpeedY = 0;
            NPC[p.HoldingNPC].WallDeath = 5;
            if(NPC[p.HoldingNPC].Type == 29)
            {
                NPCHit(p.HoldingNPC, 3, p.HoldingNPC);
            }
            if(NPCIsACoin[NPC[p.HoldingNPC].Type] && !p.Controls.Down) // Smoke effect for coins
                NewEffect(10, NPC[p.HoldingNPC].Location);
            if(p.Controls.Up && !NPCIsACoin[NPC[p.HoldingNPC].Type] && NPC[p.HoldingNPC].Type != 17) // Throw the npc up
            {
                if(NPCIsAShell[NPC[p.HoldingNPC].Type] || NPC[p.HoldingNPC].Type == 45 || NPC[p.HoldingNPC].Type == 263)
                {
                    if(p.Controls.Left || p.Controls.Right) // Up and forward
                    {
                        NPC[p.HoldingNPC].Location.SpeedX = Physics.NPCShellSpeed * p.Direction;
                        NPC[p.HoldingNPC].Location.SpeedY = -7;
                        tempLocation.Height = 0;
                        tempLocation.Width = 0;
                        tempLocation.Y = (p.Location.Y + NPC[p.HoldingNPC].Location.Y * 4) / 5;
                        tempLocation.X = (p.Location.X + NPC[p.HoldingNPC].Location.X * 4) / 5;
                        if(NPC[p.HoldingNPC].Type != 263)
                            NewEffect(132, tempLocation);
                    }
                    else
                    {
                        NPC[p.HoldingNPC].Location.SpeedY = -Physics.NPCShellSpeedY;
                        tempLocation.Height = 0;
                        tempLocation.Width = 0;
                        tempLocation.Y = (p.Location.Y + NPC[p.HoldingNPC].Location.Y * 4) / 5;
                        tempLocation.X = (p.Location.X + NPC[p.HoldingNPC].Location.X * 4) / 5;
                        if(NPC[p.HoldingNPC].Type != 263)
                            NewEffect(132, tempLocation);
                    }
                }
                else
                {
                    if(p.Controls.Left || p.Controls.Right) // Up and forward
                    {
                        if(p.Character == 3 || p.Character == 4)
                        {
                            NPC[p.HoldingNPC].Location.SpeedX = 5 * p.Direction;
                            NPC[p.HoldingNPC].Location.SpeedY = -6;
                        }
                        else
                        {
                            NPC[p.HoldingNPC].Location.SpeedY = -8;
                            NPC[p.HoldingNPC].Location.SpeedX = 3 * p.Direction;
                        }
                    }
                    else
                    {
                        NPC[p.HoldingNPC].Location.SpeedY = -10;
                        if(p.Character == 3) // peach
                            NPC[p.HoldingNPC].Location.SpeedY = -9;
                    }
                }
                if(NPCIsVeggie[NPC[p.HoldingNPC].Type] || NPC[p.HoldingNPC].Type == 19 || NPC[p.HoldingNPC].Type == 20 || NPC[p.HoldingNPC].Type == 25 || NPC[p.HoldingNPC].Type == 129 || NPC[p.HoldingNPC].Type == 130 || NPC[p.HoldingNPC].Type == 131 || NPC[p.HoldingNPC].Type == 132 || NPC[p.HoldingNPC].Type == 134 || NPC[p.HoldingNPC].Type == 135 || NPC[p.HoldingNPC].Type == 154 || NPC[p.HoldingNPC].Type == 155 || NPC[p.HoldingNPC].Type == 156 || NPC[p.HoldingNPC].Type == 157 || NPC[p.HoldingNPC].Type == 40 || NPC[p.HoldingNPC].Type == 240 || NPC[p.HoldingNPC].Type == 13 || NPC[p.HoldingNPC].Type == 265 || NPC[p.HoldingNPC].Type == 288 || NPC[p.HoldingNPC].Type == 291)
                    PlaySound(SFX_Throw);
                else
                    PlaySound(SFX_ShellHit);
                NPC[p.HoldingNPC].Projectile = true;
            }
            else if(p.Controls.Down && NPC[p.HoldingNPC].Type != 17) // Drop
            {
                tempBool = false;
                if((p.Direction == 1 && p.Location.SpeedX > 3) || (p.Direction == -1 && p.Location.SpeedX < -3))
                    tempBool = true;
                if(tempBool && NPC[p.HoldingNPC].Type == 195)
                {
                    p.Location.SpeedX = 0;
                    NPC[p.HoldingNPC].Location.SpeedX = Physics.NPCShellSpeed * p.Direction;
                    NPC[p.HoldingNPC].Projectile = true;
                    NPC[p.HoldingNPC].CantHurt = 0;
                    NPC[p.HoldingNPC].CantHurtPlayer = 0;
                    NPC[p.HoldingNPC].HoldingPlayer = 0;
                    PlaySound(SFX_ShellHit);
                    NewEffect(73, newLoc(NPC[p.HoldingNPC].Location.X, NPC[p.HoldingNPC].Location.Y + NPC[p.HoldingNPC].Location.Height - 16));
                    NPC[p.HoldingNPC].Location.X = p.Location.X + p.Location.Width / 2.0 - NPC[p.HoldingNPC].Location.Width / 2.0;
                    NPC[p.HoldingNPC].Location.Y = p.Location.Y + p.Location.Height - NPC[p.HoldingNPC].Location.Height;
                    p.Location.Y = NPC[p.HoldingNPC].Location.Y - p.Location.Height;
                    NPC[p.HoldingNPC].Location.SpeedY = p.Location.SpeedY;
                    p.StandingOnNPC = p.HoldingNPC;
                    p.HoldingNPC = 0;
                    p.ShellSurf = true;
                    p.Jump = 0;
                    p.Location.SpeedY = 10;
                }
                else
                {
                    if(p.Direction == 1)
                        NPC[p.HoldingNPC].Location.X = p.Location.X + p.Location.Width + 0.1;
                    else
                        NPC[p.HoldingNPC].Location.X = p.Location.X - NPC[p.HoldingNPC].Location.Width - 0.1;
                    NPC[p.HoldingNPC].Projectile = false;
                    if(NPC[p.HoldingNPC].Type == 272)
                        NPC[p.HoldingNPC].Projectile = true;
                    if(p.StandingOnNPC != 0)
                        NPC[p.HoldingNPC].Location.Y += NPC[p.StandingOnNPC].Location.SpeedY;
                }
                if(NPC[p.HoldingNPC].Type == 13 || NPC[p.HoldingNPC].Type == 265 || NPC[p.HoldingNPC].Type == 291)
                {
                    NPC[p.HoldingNPC].Location.X = p.Location.X + p.Location.Width / 2.0 - NPC[p.HoldingNPC].Location.Width / 2.0;
                    if(p.State == 1)
                        NPC[p.HoldingNPC].Location.Y = p.Location.Y - NPC[p.HoldingNPC].Location.Height;
                    else
                        NPC[p.HoldingNPC].Location.Y = p.Location.Y - NPC[p.HoldingNPC].Location.Height + 6;
                    PlaySound(SFX_Throw);
                    NPC[p.HoldingNPC].Location.SpeedX = 0;
                    NPC[p.HoldingNPC].Location.SpeedY = 20;
                }
            }
            else if(!NPCIsAShell[NPC[p.HoldingNPC].Type] &&
                    NPC[p.HoldingNPC].Type != 45 &&
                    NPC[p.HoldingNPC].Type != 263 &&
                    !NPCIsACoin[NPC[p.HoldingNPC].Type]) // if not a shell or a coin the kick it up and forward
            {
            // peach
                if(p.Character == 3)
                {
                    if(NPC[p.HoldingNPC].Type == 13 || NPC[p.HoldingNPC].Type == 265 || (p.Location.SpeedY != 0 && p.StandingOnNPC == 0 && p.Slope == 0))
                    {
                        NPC[p.HoldingNPC].Location.SpeedX = 5 * p.Direction + p.Location.SpeedX * 0.3;
                        NPC[p.HoldingNPC].Location.SpeedY = 3;
                    }
                    else
                    {
                        NPC[p.HoldingNPC].Location.SpeedX = 5 * p.Direction + p.Location.SpeedX * 0.3;
                        NPC[p.HoldingNPC].Location.SpeedY = 0;
                    }
            // toad
                }
                else if(p.Character == 4)
                {
                    if(NPC[p.HoldingNPC].Type == 13 || NPC[p.HoldingNPC].Type == 265 || (p.Location.SpeedY != 0 && p.StandingOnNPC == 0 && p.Slope == 0))
                    {
                        NPC[p.HoldingNPC].Location.SpeedX = 6 * p.Direction + p.Location.SpeedX * 0.4;
                        NPC[p.HoldingNPC].Location.SpeedY = 3.5;
                    }
                    else
                    {
                        NPC[p.HoldingNPC].Location.SpeedX = 6 * p.Direction + p.Location.SpeedX * 0.4;
                        NPC[p.HoldingNPC].Location.SpeedY = 0;
                        NPC[p.HoldingNPC].CantHurt = NPC[p.HoldingNPC].CantHurt * 2;
                    }
                }
                else
                {
                    NPC[p.HoldingNPC].Location.SpeedX = 5 * p.Direction;
                    NPC[p.HoldingNPC].Location.SpeedY = -6;
                }
                NPC[p.HoldingNPC].Projectile = true;
                if(NPCIsVeggie[NPC[p.HoldingNPC].Type] || NPC[p.HoldingNPC].Type == 19 || NPC[p.HoldingNPC].Type == 20 || NPC[p.HoldingNPC].Type == 25 || NPC[p.HoldingNPC].Type == 129 || NPC[p.HoldingNPC].Type == 130 || NPC[p.HoldingNPC].Type == 131 || NPC[p.HoldingNPC].Type == 132 || NPC[p.HoldingNPC].Type == 134 || NPC[p.HoldingNPC].Type == 135 || NPC[p.HoldingNPC].Type == 154 || NPC[p.HoldingNPC].Type == 155 || NPC[p.HoldingNPC].Type == 156 || NPC[p.HoldingNPC].Type == 157 || NPC[p.HoldingNPC].Type == 40 || NPC[p.HoldingNPC].Type == 240 || NPC[p.HoldingNPC].Type == 13 || NPC[p.HoldingNPC].Type == 265 || NPC[p.HoldingNPC].Type == 288 || NPC[p.HoldingNPC].Type == 291)
                    PlaySound(SFX_Throw);
                else if(NPC[p.HoldingNPC].Type == 17)
                    PlaySound(SFX_Bullet);
                else
                    PlaySound(SFX_ShellHit);
            }
            else if(NPCIsAShell[NPC[p.HoldingNPC].Type])
            {
                NPC[p.HoldingNPC].Location.SpeedY = 0;
                NPC[p.HoldingNPC].Location.SpeedX = 0;
                NPC[p.HoldingNPC].HoldingPlayer = 0;
                NPC[p.HoldingNPC].CantHurt = 0;
                NPC[p.HoldingNPC].CantHurtPlayer = 0;
                NPC[p.HoldingNPC].Projectile = false;
                NPCHit(p.HoldingNPC, 1, A);
                tempLocation.Height = 0;
                tempLocation.Width = 0;
                tempLocation.Y = (p.Location.Y + NPC[p.HoldingNPC].Location.Y * 4) / 5;
                tempLocation.X = (p.Location.X + NPC[p.HoldingNPC].Location.X * 4) / 5;
                NewEffect(132, tempLocation);
            }
            else if(NPC[p.HoldingNPC].Type == 263)
            {
                 PlaySound(SFX_ShellHit);
                NPC[p.HoldingNPC].Location.SpeedX = Physics.NPCShellSpeed * p.Direction;
                NPC[p.HoldingNPC].CantHurt = Physics.NPCCanHurtWait;
                NPC[p.HoldingNPC].CantHurtPlayer = A;
                NPC[p.HoldingNPC].Projectile = true;
            }
            if(NPC[p.HoldingNPC].Type == 134 && NPC[p.HoldingNPC].Location.SpeedX != 0)
            {
                NPC[p.HoldingNPC].Location.SpeedX += p.Location.SpeedX * 0.5;
                if(p.StandingOnNPC != 0)
                    NPC[p.HoldingNPC].Location.SpeedX += NPC[p.StandingOnNPC].Location.SpeedX;
            }
            if(NPC[p.HoldingNPC].Type == 13 && NPC[p.HoldingNPC].Special == 4) // give toad fireballs a little spunk
            {
                if(NPC[p.HoldingNPC].Location.SpeedY < 0)
                    NPC[p.HoldingNPC].Location.SpeedY = NPC[p.HoldingNPC].Location.SpeedY * 1.1;
            }
            if(NPC[p.HoldingNPC].Type == 291)
            {
                if(p.Location.SpeedX != 0 && NPC[p.HoldingNPC].Location.SpeedX != 0)
                    NPC[p.HoldingNPC].Location.SpeedX += p.Location.SpeedX * 0.5;
            }

        if(NPC[p.HoldingNPC].Type == 292)
        {
            NPC[p.HoldingNPC].Special5 = A;
            NPC[p.HoldingNPC].Special6 = p.Direction;
            NPC[p.HoldingNPC].Location.SpeedY = -8;
            NPC[p.HoldingNPC].Location.SpeedX = 12 * p.Direction + p.Location.SpeedX;
            NPC[p.HoldingNPC].Projectile = true;
        }


            NPC[p.HoldingNPC].HoldingPlayer = 0;
            p.HoldingNPC = 0;
        }
    }

    if(LayerNPC > 0)
    {
        int B = NPC[LayerNPC].AttLayer;
        // for(B = 1; B <= maxLayers; B++)
        {
            if(B != LAYER_NONE)
            {
                // if(Layer[B].Name == NPC[LayerNPC].AttLayer)
                {
                    if(NPC[LayerNPC].Location.X - lyrX == 0.0 && NPC[LayerNPC].Location.Y - lyrY == 0.0)
                    {
                        if(Layer[B].SpeedX != 0.0f || Layer[B].SpeedY != 0.0f)
                        {
                            Layer[B].EffectStop = true;
                            Layer[B].SpeedX = 0;
                            Layer[B].SpeedY = 0;
                            for(int C : Layer[B].blocks)
                            {
                                // if(Block[C].Layer == Layer[B].Name)
                                {
                                    Block[C].Location.SpeedX = 0;
                                    Block[C].Location.SpeedY = 0;
                                }
                            }
                            if(g_compatibility.enable_climb_bgo_layer_move)
                            {
                                for(int C : Layer[B].BGOs)
                                {
                                    if(BackgroundFence[Background[C].Type])
                                    {
                                        Background[C].Location.SpeedX = 0;
                                        Background[C].Location.SpeedY = 0;
                                    }
                                }
                            }
                            for(int C : Layer[B].NPCs)
                            {
                                // if(NPC[C].Layer == Layer[B].Name)
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
            // else
            //     break;
        }
    }
}

void LinkFrame(const int A)
{
    LinkFrame(Player[A]);
}

void LinkFrame(Player_t &p)
{
    Location_t tempLocation;
    //auto &p = Player[A];

    p.MountOffsetY = 0;

    // Hurt frame
    if(p.FrameCount == -10)
    {
        if(p.SwordPoke == 0)
        {
            if(p.Location.SpeedY == 0.0 ||
               p.StandingOnNPC != 0 ||
               p.Slope != 0 || p.Wet > 0 ||
               p.Immune == 0) // Hurt Frame
               p.FrameCount = 0;
            else
            {
                p.Frame = 11;
                return;
            }
        }
        else
            p.FrameCount = 0;
    }

    if(p.Stoned)
    {
        p.Frame = 12;
        if(p.Location.SpeedX != 0.0)
        {
            if(p.Location.SpeedY == 0.0 || p.Slope > 0 || p.StandingOnNPC != 0)
            {
                if(p.SlideCounter <= 0)
                {
                    p.SlideCounter = 2 + iRand_round(2);
                    tempLocation.Y = p.Location.Y + p.Location.Height - 5;
                    tempLocation.X = p.Location.X + p.Location.Width / 2.0 - 4;
                    NewEffect(74, tempLocation, 1, 0, ShadowMode);
                }
            }
        }
        return;
    }

    if(!LevelSelect && p.Effect == 0 && p.FireBallCD == 0)
    {
        if(p.Controls.Left)
            p.Direction = -1;
        if(p.Controls.Right)
            p.Direction = 1;
    }

    if(p.Fairy)
        return;

    if(p.SwordPoke < 0) // Drawing back
    {
        if(!p.Duck)
            p.Frame = 6;
        else
            p.Frame = 8;
    }
    else if(p.SwordPoke > 0) // Stabbing
    {
        if(!p.Duck)
            p.Frame = 7;
        else
            p.Frame = 8;
    }
    else if(p.Mount == 2) // Clown Car
    {
        p.Frame = 1;
        p.MountFrame = SpecialFrame[2];
        if(p.Direction == 1)
            p.MountFrame += 4;
    }
    else if(p.Duck) // Ducking
        p.Frame = 5;
    else if(p.WetFrame && p.Location.SpeedY != 0.0 && p.Slope == 0 && p.StandingOnNPC == 0 && !p.Duck && p.Quicksand == 0) // Link is swimming
    {
        if(p.Location.SpeedY < 0.5 || p.Frame != 3)
        {
            if(p.Frame != 1 && p.Frame != 2 && p.Frame != 3 && p.Frame != 4)
                p.FrameCount = 6;

            p.FrameCount += 1;

            if(p.FrameCount < 6)
                p.Frame = 3;
            else if(p.FrameCount < 12)
                p.Frame = 2;
            else if(p.FrameCount < 18)
                p.Frame = 3;
            else if(p.FrameCount < 24)
                p.Frame = 1;
            else
            {
                p.Frame = 3;
                p.FrameCount = 0;
            }
        }
        else
            p.Frame = 3;
    }
    else if(p.Location.SpeedY != 0.0 && p.StandingOnNPC == 0 && p.Slope == 0 && !(p.Quicksand > 0 && p.Location.SpeedY > 0)) // Jumping/falling
    {
        if(p.Location.SpeedY < 0)
        {
            if(p.Controls.Up)
                p.Frame = 10;
            else
                p.Frame = 5;
        }
        else
        {
            if(p.Controls.Down)
                p.Frame = 9;
            else
                p.Frame = 3;
        }
    }
    else if(p.Location.SpeedX == 0.0 || (p.Slippy && !p.Controls.Left && !p.Controls.Right)) // Standing
        p.Frame = 1;
    else // Running
    {
        p.FrameCount += 1;

        if(p.Location.SpeedX > Physics.PlayerWalkSpeed - 1.5 || p.Location.SpeedX < -Physics.PlayerWalkSpeed + 1.5)
            p.FrameCount += 1;

        if(p.Location.SpeedX > Physics.PlayerWalkSpeed || p.Location.SpeedX < -Physics.PlayerWalkSpeed)
            p.FrameCount += 1;

        if(p.Location.SpeedX > Physics.PlayerWalkSpeed + 1 || p.Location.SpeedX < -Physics.PlayerWalkSpeed - 1)
            p.FrameCount += 1;

        if(p.Location.SpeedX > Physics.PlayerWalkSpeed + 2 || p.Location.SpeedX < -Physics.PlayerWalkSpeed - 2)
            p.FrameCount += 1;

        if(p.FrameCount >= 8)
        {
            p.FrameCount = 0;
            p.Frame -= 1;
        }

        if(p.Frame <= 0)
            p.Frame = 4;
        else if(p.Frame >= 5)
            p.Frame = 1;

        if(p.Location.SpeedX >= Physics.PlayerRunSpeed * 0.9 || p.Location.SpeedX <= -Physics.PlayerRunSpeed * 0.9)
        {
            if(p.SlideCounter <= 0)
            {
                PlaySound(SFX_ZeldaDash);
                p.SlideCounter = 2 + iRand_round(2);
                tempLocation.Y = p.Location.Y + p.Location.Height - 4;

                if(p.Location.SpeedX < 0)
                    tempLocation.X = p.Location.X + p.Location.Width / 2.0 - 6 - 4;
                else
                    tempLocation.X = p.Location.X + p.Location.Width / 2.0 + 6 - 4;

                NewEffect(74, tempLocation, 1, 0, ShadowMode);
            }
        }
    }
}

void PlayerEffects(const int A)
{
    int B = 0;
    // float C = 0;
    float D = 0;
    bool tempBool = false;
    Location_t tempLocation;
    auto &p = Player[A];

    if(p.Effect != 8 && p.Fairy)
    {
        p.Fairy = false;
        SizeCheck(Player[A]);
    }

    p.TailCount = 0;
    p.Pinched1 = 0;
    p.Pinched2 = 0;
    p.Pinched3 = 0;
    p.Pinched4 = 0;
    p.NPCPinched = 0;
    p.SwordPoke = 0;

    if(!p.YoshiBlue && p.Effect != 500)
    {
        p.CanFly = false;
        p.CanFly2 = false;
        p.RunCount = 0;
    }

    p.Immune2 = false;

    if(p.Effect == 1) // Player growing effect
    {

        p.Frame = 1;
        p.Effect2 += 1;
        if(p.Effect2 / 5 == static_cast<int>(floor(static_cast<double>(p.Effect2 / 5))))
        {
            if(p.State == 1)
            {
                p.State = 2;
                if(p.Mount == 0)
                {
                    p.Location.X += -Physics.PlayerWidth[p.Character][2] * 0.5 + Physics.PlayerWidth[p.Character][1] * 0.5;
                    p.Location.Y += -Physics.PlayerHeight[p.Character][2] + Physics.PlayerHeight[p.Character][1];
                    p.Location.Width = Physics.PlayerWidth[p.Character][p.State];
                    p.Location.Height = Physics.PlayerHeight[p.Character][p.State];
                }
                else if(p.Mount == 3)
                {
                    YoshiHeight(A);
                }
                else if(p.Character == 2 && p.Mount != 2)
                {
                    p.Location.Y += -Physics.PlayerHeight[2][2] + Physics.PlayerHeight[1][2];
                    p.Location.Height = Physics.PlayerHeight[p.Character][p.State];
                }
            }
            else
            {
                p.State = 1;
                if(p.Mount == 0)
                {
                    p.Location.X += -Physics.PlayerWidth[p.Character][1] * 0.5 + Physics.PlayerWidth[p.Character][2] * 0.5;
                    p.Location.Y += -Physics.PlayerHeight[p.Character][1] + Physics.PlayerHeight[p.Character][2];
                    p.Location.Width = Physics.PlayerWidth[p.Character][p.State];
                    p.Location.Height = Physics.PlayerHeight[p.Character][1];
                }
                else if(p.Mount == 3)
                {
                    YoshiHeight(A);
                }
                else if(p.Character == 2 && p.Mount != 2)
                {
                    p.Location.Y += -Physics.PlayerHeight[1][2] + Physics.PlayerHeight[2][2];
                    p.Location.Height = Physics.PlayerHeight[1][2];
                }
            }
        }
        if(p.Effect2 >= 50 && p.State == 2)
        {
            p.Immune += 50;
            p.Immune2 = true;
            p.Effect = 0;
            p.Effect2 = 0;
            p.StandUp = true;
        }
    }
    else if(p.Effect == 2) // Player shrinking effect
    {
        if(p.Duck)
        {
            p.StandUp = true; // Fixes a block collision bug
            p.Duck = false;
            p.Location.Height = Physics.PlayerHeight[p.Character][p.State];
            p.Location.Y += -Physics.PlayerHeight[p.Character][p.State] + Physics.PlayerDuckHeight[p.Character][p.State];
        }
        p.Frame = 1;
        p.Effect2 += 1;
        if(p.Effect2 / 5 == static_cast<int>(floor(static_cast<double>(p.Effect2 / 5))))
        {
            if(p.State == 1)
            {
                p.State = 2;
                if(p.Mount == 3)
                {
                    YoshiHeight(A);
                }
                else if(p.Mount != 2)
                {
                    p.Location.X += -Physics.PlayerWidth[p.Character][2] * 0.5 + Physics.PlayerWidth[p.Character][1] * 0.5;
                    p.Location.Y += -Physics.PlayerHeight[p.Character][2] + Physics.PlayerHeight[p.Character][1];
                    p.Location.Width = Physics.PlayerWidth[p.Character][p.State];
                    p.Location.Height = Physics.PlayerHeight[p.Character][p.State];
                }
            }
            else
            {
                p.State = 1;
                if(p.Mount == 3)
                {
                    YoshiHeight(A);
                }
                else if(p.Mount != 2)
                {
                    p.Location.X += -Physics.PlayerWidth[p.Character][1] * 0.5 + Physics.PlayerWidth[p.Character][2] * 0.5;
                    p.Location.Y += -Physics.PlayerHeight[p.Character][1] + Physics.PlayerHeight[p.Character][2];
                    p.Location.Width = Physics.PlayerWidth[p.Character][p.State];
                    p.Location.Height = Physics.PlayerHeight[p.Character][1];
                }
            }
        }
        if(p.Effect2 >= 50)
        {
            if(p.State == 2)
            {
                p.State = 1;
                if(p.Mount != 2)
                {
                    p.Location.X += -Physics.PlayerWidth[p.Character][1] * 0.5 + Physics.PlayerWidth[p.Character][2] * 0.5;
                    p.Location.Y += -Physics.PlayerHeight[p.Character][1] + Physics.PlayerHeight[p.Character][2];
                    p.Location.Width = Physics.PlayerWidth[p.Character][p.State];
                    p.Location.Height = Physics.PlayerHeight[p.Character][p.State];
                }
            }
            p.Immune = 150;
            p.Immune2 = true;
            p.Effect = 0;
            p.Effect2 = 0;
            // If numPlayers <= 2 Then DropBonus A
        }
    }
    else if(p.Effect == 227) // Player losing firepower
    {
        if(p.Duck)
        {
            p.StandUp = true; // Fixes a block collision bug
            p.Duck = false;
            p.Location.Height = Physics.PlayerHeight[p.Character][p.State];
            p.Location.Y += -Physics.PlayerHeight[p.Character][p.State] + Physics.PlayerDuckHeight[p.Character][p.State];
        }
        p.Frame = 1;
        p.Effect2 += 1;
        if(p.Effect2 / 5 == static_cast<int>(floor(static_cast<double>(p.Effect2 / 5))))
        {
            if(p.State == 2)
                p.State = 3;
            else
                p.State = 2;
        }
        if(p.Effect2 >= 50)
        {
            if(p.State == 3)
                p.State = 2;
            p.Immune = 150;
            p.Immune2 = true;
            p.Effect = 0;
            p.Effect2 = 0;
            // If numPlayers <= 2 Then DropBonus A
        }
    }
    else if(p.Effect == 228) // Player losing icepower
    {
        if(p.Duck)
        {
            p.StandUp = true; // Fixes a block collision bug
            p.Duck = false;
            p.Location.Height = Physics.PlayerHeight[p.Character][p.State];
            p.Location.Y += -Physics.PlayerHeight[p.Character][p.State] + Physics.PlayerDuckHeight[p.Character][p.State];
        }

        p.Frame = 1;
        p.Effect2 += 1;

        if(fEqual(p.Effect2 / 5, std::floor(p.Effect2 / 5)))
        {
            if(p.State == 2)
                p.State = 7;
            else
                p.State = 2;
        }

        if(p.Effect2 >= 50)
        {
            if(p.State == 7)
                p.State = 2;
            p.Immune = 150;
            p.Immune2 = true;
            p.Effect = 0;
            p.Effect2 = 0;
            // If numPlayers <= 2 Then DropBonus A
        }
    }
    else if(p.Effect == 3) // Warp effect
    {
        p.SpinJump = false;
        p.TailCount = 0;
        p.Location.SpeedY = 0;

        bool backward = p.WarpBackward;
        auto &warp = Warp[p.Warp];
        auto &warp_enter = backward ? warp.Exit : warp.Entrance;
        auto &warp_exit = backward ? warp.Entrance : warp.Exit;
        auto &warp_dir_enter = backward ? warp.Direction2 : warp.Direction;
        auto &warp_dir_exit = backward ? warp.Direction : warp.Direction2;

        if(p.Effect2 == 0.0) // Entering pipe
        {
            double leftToGoal = 0.0;
            double sign = +1.0;

            if(warp_dir_enter == 3)
            {
                p.Location.Y += 1;
                p.Location.X = warp_enter.X + warp_enter.Width / 2.0 - p.Location.Width / 2.0;

                sign = (warp_enter.Y + warp_enter.Height) > p.Location.Y ? +1.0 : -1.0;
                leftToGoal = SDL_fabs((warp_enter.Y + warp_enter.Height) - p.Location.Y) * sign;

                if(p.Location.Y > warp_enter.Y + warp_enter.Height + 8)
                    p.Effect2 = 1;

                if(p.Mount == 0)
                    p.Frame = 15;

                if(p.HoldingNPC > 0)
                {
                    NPC[p.HoldingNPC].Location.Y = p.Location.Y + Physics.PlayerGrabSpotY[p.Character][p.State] + 32 - NPC[p.HoldingNPC].Location.Height;
                    NPC[p.HoldingNPC].Location.X = p.Location.X + p.Location.Width / 2.0 - NPC[p.HoldingNPC].Location.Width / 2.0;
                }
            }
            else if(warp_dir_enter == 1)
            {
                p.Location.Y -= 1;
                p.Location.X = warp_enter.X + warp_enter.Width / 2.0 - p.Location.Width / 2.0;

                sign = (p.Location.Y + p.Location.Height) > warp_enter.Y ? +1.0 : -1.0;
                leftToGoal = SDL_fabs(warp_enter.Y - (p.Location.Y + p.Location.Height)) * sign;

                if(p.Location.Y + p.Location.Height + 8 < warp_enter.Y)
                    p.Effect2 = 1;

                if(p.HoldingNPC > 0)
                {
                    NPC[p.HoldingNPC].Location.Y = p.Location.Y + Physics.PlayerGrabSpotY[p.Character][p.State] + 32 - NPC[p.HoldingNPC].Location.Height;
                    NPC[p.HoldingNPC].Location.X = p.Location.X + p.Location.Width / 2.0 - NPC[p.HoldingNPC].Location.Width / 2.0;
                }
                if(p.Mount == 0)
                    p.Frame = 15;
            }
            else if(warp_dir_enter == 2)
            {
                if(p.Mount == 3)
                {
                    p.Duck = true;
                    p.Location.Height = 30;
                }

                p.Direction = -1; // makes (p.Direction > 0) always false
                p.Location.Y = warp_enter.Y + warp_enter.Height - p.Location.Height - 2;
                p.Location.X -= 0.5;

                sign = (p.Location.X + p.Location.Width) > warp_enter.X ? +1.0 : -1.0;
                leftToGoal = SDL_fabs((warp_enter.X - (p.Location.X + p.Location.Width)) * 2) * sign;

                if(p.Location.X + p.Location.Width + 8 < warp_enter.X)
                    p.Effect2 = 1;

                if(p.HoldingNPC > 0)
                {
                    NPC[p.HoldingNPC].Location.Y = p.Location.Y + Physics.PlayerGrabSpotY[p.Character][p.State] + 32 - NPC[p.HoldingNPC].Location.Height;
//                    if(p.Direction > 0) // Always false
//                        NPC[p.HoldingNPC].Location.X = p.Location.X + Physics.PlayerGrabSpotX[p.Character][p.State];
//                    else
                    NPC[p.HoldingNPC].Location.X = p.Location.X + p.Location.Width - Physics.PlayerGrabSpotX[p.Character][p.State] - NPC[p.HoldingNPC].Location.Width;
                }
                p.Location.SpeedX = -0.5;
                PlayerFrame(p);
                p.Location.SpeedX = 0;
            }
            else if(warp_dir_enter == 4)
            {
                if(p.Mount == 3)
                {
                    p.Duck = true;
                    p.Location.Height = 30;
                }
                p.Direction = 1; // Makes (p.Direction > 0) always true
                p.Location.Y = warp_enter.Y + warp_enter.Height - p.Location.Height - 2;
                p.Location.X += 0.5;

                sign = p.Location.X < (warp_enter.X + warp_enter.Width) ? +1.0 : -1.0;
                leftToGoal = SDL_fabs(((warp_enter.X + warp_enter.Width) - p.Location.X) * 2) * sign;

                if(p.Location.X > warp_enter.X + warp_enter.Width + 8)
                    p.Effect2 = 1;

                if(p.HoldingNPC > 0)
                {
                    NPC[p.HoldingNPC].Location.Y = p.Location.Y + Physics.PlayerGrabSpotY[p.Character][p.State] + 32 - NPC[p.HoldingNPC].Location.Height;
//                    if(p.Direction > 0) // always true
                    NPC[p.HoldingNPC].Location.X = p.Location.X + Physics.PlayerGrabSpotX[p.Character][p.State];
//                    else
//                        NPC[p.HoldingNPC].Location.X = p.Location.X + p.Location.Width - Physics.PlayerGrabSpotX[p.Character][p.State] - NPC[p.HoldingNPC].Location.Width;
                }
                p.Location.SpeedX = 0.5;
                PlayerFrame(p);
                p.Location.SpeedX = 0;
            }

            D_pLogDebug("Warping: %g (same section? %s!)", leftToGoal, SectionCollision(p.Section, warp_exit) ? "yes" : "no");
            switch(warp.transitEffect)
            {
            default:
            case LevelDoor::TRANSIT_NONE:
                if(Maths::iRound(leftToGoal) == 0 && warp.level == STRINGINDEX_NONE && !warp.MapWarp && !SectionCollision(p.Section, warp_exit))
                    g_levelVScreenFader[A].setupFader(g_config.EnableInterLevelFade ? 8 : 64, 0, 65, ScreenFader::S_FADE);
                break;

            case LevelDoor::TRANSIT_SCROLL:
                // uses fade effect if not same section
                if(Maths::iRound(leftToGoal) == 24 && !SectionCollision(p.Section, warp_exit))
                    g_levelVScreenFader[A].setupFader(3, 0, 65, ScreenFader::S_FADE);
                break;

            case LevelDoor::TRANSIT_FADE:
                if(Maths::iRound(leftToGoal) == 24)
                    g_levelVScreenFader[A].setupFader(3, 0, 65, ScreenFader::S_FADE);
                break;

            case LevelDoor::TRANSIT_CIRCLE_FADE:
                if(Maths::iRound(leftToGoal) == 24)
                    g_levelVScreenFader[A].setupFader(3, 0, 65, ScreenFader::S_CIRCLE,
                                                      true,
                                                      Maths::iRound(warp_enter.X + warp_enter.Width / 2),
                                                      Maths::iRound(warp_enter.Y + warp_enter.Height / 2), A);
                break;

            case LevelDoor::TRANSIT_FLIP_H:
                if(Maths::iRound(leftToGoal) == 24)
                    g_levelVScreenFader[A].setupFader(3, 0, 65, ScreenFader::S_FLIP_H);
                break;

            case LevelDoor::TRANSIT_FLIP_V:
                if(Maths::iRound(leftToGoal) == 24)
                    g_levelVScreenFader[A].setupFader(3, 0, 65, ScreenFader::S_FLIP_V);
                break;
            }
        }
        else if(fEqual(p.Effect2, 1))  // Exiting pipe (initialization)
        {
            if(warp.NoYoshi)
            {
                if(OwedMount[A] == 0 && p.Mount > 0 && p.Mount != 2)
                {
                    OwedMount[A] = p.Mount;
                    OwedMountType[A] = p.MountType;
                }
                UnDuck(Player[A]);
                p.Mount = 0;
                p.MountType = 0;
                p.MountOffsetY = 0;
                SizeCheck(Player[A]);
                UpdateYoshiMusic();
            }

            if(warp_dir_exit == 1)
            {
                p.Location.X = warp_exit.X + warp_exit.Width / 2.0 - p.Location.Width / 2.0;
                p.Location.Y = warp_exit.Y - p.Location.Height - 8;
                if(p.Mount == 0)
                    p.Frame = 15;
                if(p.HoldingNPC > 0)
                {
                    NPC[p.HoldingNPC].Location.Y = p.Location.Y + Physics.PlayerGrabSpotY[p.Character][p.State] + 32 - NPC[p.HoldingNPC].Location.Height;
                    NPC[p.HoldingNPC].Location.X = p.Location.X + p.Location.Width / 2.0 - NPC[p.HoldingNPC].Location.Width / 2.0;
                }
            }
            else if(warp_dir_exit == 3)
            {
                p.Location.X = warp_exit.X + warp_exit.Width / 2.0 - p.Location.Width / 2.0;
                p.Location.Y = warp_exit.Y + warp_exit.Height + 8;
                if(p.Mount == 0)
                    p.Frame = 15;
                if(p.HoldingNPC > 0)
                {
                    NPC[p.HoldingNPC].Location.Y = p.Location.Y + Physics.PlayerGrabSpotY[p.Character][p.State] + 32 - NPC[p.HoldingNPC].Location.Height;
                    NPC[p.HoldingNPC].Location.X = p.Location.X + p.Location.Width / 2.0 - NPC[p.HoldingNPC].Location.Width / 2.0;
                }
            }
            else if(warp_dir_exit == 2)
            {
                if(p.Mount == 3)
                {
                    p.Duck = true;
                    p.Location.Height = 30;
                }
                p.Location.X = warp_exit.X - p.Location.Width - 8;
                p.Location.Y = warp_exit.Y + warp_exit.Height - p.Location.Height - 2;
                if(p.Mount == 0)
                    p.Frame = 1;
                p.Direction = 1;
                if(p.HoldingNPC > 0)
                {
                    if(p.State == 1)
                        p.Frame = 5;
                    else
                        p.Frame = 8;
                    NPC[p.HoldingNPC].Location.Y = p.Location.Y + Physics.PlayerGrabSpotY[p.Character][p.State] + 32 - NPC[p.HoldingNPC].Location.Height;
                    p.Direction = -1; // Makes (p.Direction > 0) always false
//                    if(p.Direction > 0) // always false
//                        NPC[p.HoldingNPC].Location.X = p.Location.X + Physics.PlayerGrabSpotX[p.Character][p.State];
//                    else
                    NPC[p.HoldingNPC].Location.X = p.Location.X + p.Location.Width - Physics.PlayerGrabSpotX[p.Character][p.State] - NPC[p.HoldingNPC].Location.Width;
                }
            }
            else if(warp_dir_exit == 4)
            {
                if(p.Mount == 3)
                {
                    p.Duck = true;
                    p.Location.Height = 30;
                }
                p.Location.X = warp_exit.X + warp_exit.Width + 8;
                p.Location.Y = warp_exit.Y + warp_exit.Height - p.Location.Height - 2;
                if(p.Mount == 0)
                    p.Frame = 1;
                p.Direction = -1;
                if(p.HoldingNPC > 0)
                {
                    if(p.State == 1)
                        p.Frame = 5;
                    else
                        p.Frame = 8;
                    p.Direction = 1; // Makes always true
                    NPC[p.HoldingNPC].Location.Y = p.Location.Y + Physics.PlayerGrabSpotY[p.Character][p.State] + 32 - NPC[p.HoldingNPC].Location.Height;
//                    if(p.Direction > 0) // always true
                    NPC[p.HoldingNPC].Location.X = p.Location.X + Physics.PlayerGrabSpotX[p.Character][p.State];
//                    else
//                        NPC[p.HoldingNPC].Location.X = p.Location.X + p.Location.Width - Physics.PlayerGrabSpotX[p.Character][p.State] - NPC[p.HoldingNPC].Location.Width;
                }
            }

            p.Effect2 = 100;
            if(p.Duck)
            {
                if(warp_dir_exit == 1 || warp_dir_exit == 3)
                {
                    UnDuck(Player[A]);
                }
            }

            int last_section = p.Section;

            CheckSection(A);

            bool same_section = (last_section == p.Section);

            if(p.HoldingNPC > 0)
            {
                CheckSectionNPC(p.HoldingNPC);
            }

            if(numPlayers > 2/* && nPlay.Online == false*/)
            {
                for(B = 1; B <= numPlayers; B++)
                {
                    if(B != A)
                    {
                        if(warp_dir_exit != 3)
                            Player[B].Location.Y = p.Location.Y + p.Location.Height - Player[B].Location.Height;
                        else
                            Player[B].Location.Y = p.Location.Y;
                        Player[B].Location.X = p.Location.X + p.Location.Width / 2.0 - Player[B].Location.Width / 2.0;
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

            if(numPlayers <= 2 && (g_levelVScreenFader[A].isVisible() || warp.transitEffect == LevelDoor::TRANSIT_SCROLL))
            {
                switch(warp.transitEffect)
                {
                default:
                case LevelDoor::TRANSIT_NONE:
                    g_levelVScreenFader[A].setupFader(g_config.EnableInterLevelFade ? 8 : 64, 65, 0, ScreenFader::S_FADE);
                    break;

                case LevelDoor::TRANSIT_SCROLL:
                    if(same_section)
                    {
                        qScreenX[A] = vScreenX[A];
                        qScreenY[A] = vScreenY[A];
                        qScreenLoc[A] = vScreen[A];
                        qScreen = true;
                    }
                    // follows fade logic if cross section
                    else if(g_levelVScreenFader[A].isVisible())
                    {
                        g_levelVScreenFader[A].setupFader(3, 65, 0, ScreenFader::S_FADE);
                    }
                    break;

                case LevelDoor::TRANSIT_FADE:
                    g_levelVScreenFader[A].setupFader(3, 65, 0, ScreenFader::S_FADE);
                    break;

                case LevelDoor::TRANSIT_CIRCLE_FADE:
                    g_levelVScreenFader[A].setupFader(2, 65, 0, ScreenFader::S_CIRCLE,
                                                      true,
                                                      Maths::iRound(warp_exit.X + warp_exit.Width / 2),
                                                      Maths::iRound(warp_exit.Y + warp_exit.Height /2),
                                                      A);
                    break;

                case LevelDoor::TRANSIT_FLIP_H:
                    g_levelVScreenFader[A].setupFader(3, 65, 0, ScreenFader::S_FLIP_H);
                    break;

                case LevelDoor::TRANSIT_FLIP_V:
                    g_levelVScreenFader[A].setupFader(3, 65, 0, ScreenFader::S_FLIP_V);
                    break;
                }
            }

            if(warp.level != STRINGINDEX_NONE)
            {
                GoToLevel = GetS(warp.level);
                GoToLevelNoGameThing = warp.noEntranceScene;
                p.Effect = 8;
                p.Effect2 = 2970;
                ReturnWarp = p.Warp;
                if(IsEpisodeIntro && NoMap)
                    ReturnWarpSaved = ReturnWarp;
                StartWarp = warp.LevelWarp;
            }
            else if(warp.MapWarp)
            {
                p.Effect = 8;
                p.Effect2 = 2970;
            }
        }
        else if(p.Effect2 >= 100) // Waiting until exit pipe
        {
            p.Effect2 += 1;

            if(p.Effect2 >= 110)
            {
                p.Effect2 = 2;
                if(backward || !warp.cannonExit)
                    PlaySound(SFX_Warp);
            }
        }
        else if(fEqual(p.Effect2, 2)) // Proceeding the pipe exiting
        {
            if(!backward && warp.cannonExit)
            {
                switch(warp_dir_exit)
                {
                case LevelDoor::EXIT_DOWN:
                    p.Location.Y = warp_exit.Y;
                    break;
                case LevelDoor::EXIT_UP:
                    p.Location.Y = (warp_exit.Y + warp_exit.Height) - p.Location.Height;
                    break;
                case LevelDoor::EXIT_LEFT:
                    p.Location.X = (warp_exit.X + warp_exit.Width) - p.Location.Width;
                    p.Direction = -1;
                    break;
                case LevelDoor::EXIT_RIGHT:
                    p.Location.X = warp_exit.X;
                    p.Direction = +1;
                    break;
                }
                p.Effect2 = 3;
                if(p.HoldingNPC > 0)
                {
                    if(p.ForceHold < 5) // Prevent NPC being stuck in the wall/ceiling
                        p.ForceHold = 5;
                    PlayerGrabCode(A);
                }
            }
            else if(warp_dir_exit == LevelDoor::EXIT_DOWN)
            {
                p.Location.Y += 1;

                if(p.Location.Y >= warp_exit.Y)
                    p.Effect2 = 3;

                if(p.HoldingNPC > 0)
                {
                    NPC[p.HoldingNPC].Location.Y = p.Location.Y + Physics.PlayerGrabSpotY[p.Character][p.State] + 32 - NPC[p.HoldingNPC].Location.Height;
                    NPC[p.HoldingNPC].Location.X = p.Location.X + p.Location.Width / 2.0 - NPC[p.HoldingNPC].Location.Width / 2.0;
                }

                if(p.Mount == 0)
                    p.Frame = 15;
            }
            else if(warp_dir_exit == LevelDoor::EXIT_UP)
            {
                p.Location.Y -= 1;

                if(p.Location.Y + p.Location.Height <= warp_exit.Y + warp_exit.Height)
                    p.Effect2 = 3;

                if(p.HoldingNPC > 0)
                {
                    NPC[p.HoldingNPC].Location.Y = p.Location.Y + Physics.PlayerGrabSpotY[p.Character][p.State] + 32 - NPC[p.HoldingNPC].Location.Height;
                    NPC[p.HoldingNPC].Location.X = p.Location.X + p.Location.Width / 2.0 - NPC[p.HoldingNPC].Location.Width / 2.0;
                }

                if(p.Mount == 0)
                    p.Frame = 15;
            }
            else if(warp_dir_exit == LevelDoor::EXIT_LEFT)
            {
                p.Location.X -= 0.5;
                p.Direction = -1; // makes (p.Direction < 0) always true

                if(p.Location.X + p.Location.Width <= warp_exit.X + warp_exit.Width)
                    p.Effect2 = 3;

                if(p.HoldingNPC > 0)
                {
                    if(p.Character >= 3) // peach/toad leaving a pipe
                    {
                        p.Location.SpeedX = 1;
                        PlayerFrame(p);
                        NPC[p.HoldingNPC].Location.Y = p.Location.Y + Physics.PlayerGrabSpotY[p.Character][p.State] + 32 - NPC[p.HoldingNPC].Location.Height;

//                        if(p.Direction < 0) // always true
                        NPC[p.HoldingNPC].Location.X = p.Location.X + Physics.PlayerGrabSpotX[p.Character][p.State];
//                        else
//                            NPC[p.HoldingNPC].Location.X = p.Location.X + p.Location.Width - Physics.PlayerGrabSpotX[p.Character][p.State] - NPC[p.HoldingNPC].Location.Width;
                    }
                    else
                    {
                        p.Direction = 1; // makes (p.Direction > 0) always true

                        if(p.State == 1)
                            p.Frame = 5;
                        else
                            p.Frame = 8;

                        NPC[p.HoldingNPC].Location.Y = p.Location.Y + Physics.PlayerGrabSpotY[p.Character][p.State] + 32 - NPC[p.HoldingNPC].Location.Height;

//                        if(p.Direction > 0) // always true
                        NPC[p.HoldingNPC].Location.X = p.Location.X + Physics.PlayerGrabSpotX[p.Character][p.State];
//                        else
//                            NPC[p.HoldingNPC].Location.X = p.Location.X + p.Location.Width - Physics.PlayerGrabSpotX[p.Character][p.State] - NPC[p.HoldingNPC].Location.Width;
                    }
                }
                else
                {
                    p.Location.SpeedX = -0.5;
                    PlayerFrame(p);
                    p.Location.SpeedX = 0;
                }
            }
            else if(warp_dir_exit == LevelDoor::EXIT_RIGHT)
            {
                p.Location.X += 0.5;
                p.Direction = 1; // makes (p.Direction < 0) always false

                if(p.Location.X >= warp_exit.X)
                    p.Effect2 = 3;

                if(p.HoldingNPC > 0)
                {
                    if(p.Character >= 3) // peach/toad leaving a pipe
                    {
                        p.Location.SpeedX = 1;
                        PlayerFrame(p);
                        NPC[p.HoldingNPC].Location.Y = p.Location.Y + Physics.PlayerGrabSpotY[p.Character][p.State] + 32 - NPC[p.HoldingNPC].Location.Height;

//                        if(p.Direction < 0) // always false
//                            NPC[p.HoldingNPC].Location.X = p.Location.X + Physics.PlayerGrabSpotX[p.Character][p.State];
//                        else
                        NPC[p.HoldingNPC].Location.X = p.Location.X + p.Location.Width - Physics.PlayerGrabSpotX[p.Character][p.State] - NPC[p.HoldingNPC].Location.Width;
                    }
                    else
                    {
                        p.Direction = -1; // makes (p.Direction > 0) always false

                        if(p.State == 1)
                            p.Frame = 5;
                        else
                            p.Frame = 8;

                        NPC[p.HoldingNPC].Location.Y = p.Location.Y + Physics.PlayerGrabSpotY[p.Character][p.State] + 32 - NPC[p.HoldingNPC].Location.Height;

//                        if(p.Direction > 0) // always false
//                            NPC[p.HoldingNPC].Location.X = p.Location.X + Physics.PlayerGrabSpotX[p.Character][p.State];
//                        else
                        NPC[p.HoldingNPC].Location.X = p.Location.X + p.Location.Width - Physics.PlayerGrabSpotX[p.Character][p.State] - NPC[p.HoldingNPC].Location.Width;
                    }
                }
                else
                {
                    p.Location.SpeedX = -0.5;
                    PlayerFrame(p);
                    p.Location.SpeedX = 0;
                }
            }
        }
        else if(fEqual(p.Effect2, 3)) // Finishing the pipe exiting / shooting
        {
            if(!backward && warp.cannonExit)
            {
                PlaySound(SFX_Bullet);
                auto loc = warp_exit;
                if(warp_dir_exit == LevelDoor::EXIT_LEFT || warp_dir_exit == LevelDoor::EXIT_RIGHT)
                    loc.Y += loc.Height - (p.Location.Height / 2) - (loc.Height / 2);
                NewEffect(132, loc, p.Direction); // Cannon pipe shoot effect
            }

            if(p.HoldingNPC > 0)
            {
                if(warp_dir_exit == LevelDoor::EXIT_LEFT || warp_dir_exit == LevelDoor::EXIT_RIGHT)
                {
                    if(warp_dir_exit == 2)
                        p.Direction = 1;
                    else if(warp_dir_exit == 4)
                        p.Direction = -1;

                    if(p.State == 1)
                        p.Frame = 5;
                    else
                        p.Frame = 8;

                    if(!p.Controls.Run)
                        p.Controls.Run = true;

                    PlayerGrabCode(A);
                }
            }

            p.Effect = 0;
            p.Effect2 = 0;
            p.WarpCD = 20;
            p.CanJump = false;
            p.CanAltJump = false;
            p.Bumped2 = 0;
            if(!backward && warp.cannonExit)
            {
                switch(warp_dir_exit)
                {
                case LevelDoor::EXIT_DOWN:
                    p.Location.SpeedY = warp.cannonExitSpeed;
                    break;
                case LevelDoor::EXIT_UP:
                    p.Location.SpeedY = -warp.cannonExitSpeed;
                    break;
                case LevelDoor::EXIT_LEFT:
                    p.Location.SpeedX = -warp.cannonExitSpeed;
                    p.Direction = -1;
                    break;
                case LevelDoor::EXIT_RIGHT:
                    p.Location.SpeedX = warp.cannonExitSpeed;
                    p.Direction = +1;
                    break;
                }

                if(warp_dir_exit == LevelDoor::EXIT_LEFT || warp_dir_exit == LevelDoor::EXIT_RIGHT)
                    p.WarpShooted = true;
            }
            else
            {
                p.Location.SpeedY = 0;
                p.Location.SpeedX = 0;
            }

            if(p.HoldingNPC > 0)
                NPC[p.HoldingNPC].Effect = 0;

            if(numPlayers > 2 /*&& nPlay.Online == false*/)
            {
                for(B = 1; B <= numPlayers; B++)
                {
                    if(B != A)
                    {
                        if(warp_dir_exit != 1)
                            Player[B].Location.Y = p.Location.Y + p.Location.Height - Player[B].Location.Height;
                        else
                            Player[B].Location.Y = p.Location.Y;

                        Player[B].Location.X = p.Location.X + p.Location.Width / 2.0 - Player[B].Location.Width / 2.0;
                        Player[B].Location.SpeedY = dRand() * 24 - 12;
                        Player[B].Effect = 0;
                        Player[B].Effect2 = 0;
                        CheckSection(B);
                    }
                }
            }
        }
    }
    else if(p.Effect == 7) // Door effect
    {
        bool backward = p.WarpBackward;
        auto &warp = Warp[p.Warp];
        auto &warp_enter = backward ? warp.Exit : warp.Entrance;
        auto &warp_exit = backward ? warp.Entrance : warp.Exit;

        if(p.HoldingNPC > 0)
        {
            NPC[p.HoldingNPC].Location.Y = p.Location.Y + Physics.PlayerGrabSpotY[p.Character][p.State] + 32 - NPC[p.HoldingNPC].Location.Height;
            NPC[p.HoldingNPC].Location.X = p.Location.X + p.Location.Width / 2.0 - NPC[p.HoldingNPC].Location.Width / 2.0;
        }

        p.Effect2 += 1;

        if(p.Mount == 0 && p.Character != 5)
            p.Frame = 13;

        if(p.Character == 5)
            p.Frame = 1;

        switch(warp.transitEffect)
        {
        default:
        case LevelDoor::TRANSIT_NONE:
            if(fEqual(p.Effect2, 20) && warp.level == STRINGINDEX_NONE && !warp.MapWarp && !SectionCollision(p.Section, warp_exit))
                g_levelVScreenFader[A].setupFader(g_config.EnableInterLevelFade ? 9 : 64, 0, 65, ScreenFader::S_FADE);
            break;

        case LevelDoor::TRANSIT_SCROLL:
            // uses fade effect if not same section
            if(fEqual(p.Effect2, 5) && !SectionCollision(p.Section, warp_exit))
                g_levelVScreenFader[A].setupFader(3, 0, 65, ScreenFader::S_FADE);
            break;

        case LevelDoor::TRANSIT_FADE:
            if(fEqual(p.Effect2, 5))
                g_levelVScreenFader[A].setupFader(3, 0, 65, ScreenFader::S_FADE);
            break;

        case LevelDoor::TRANSIT_CIRCLE_FADE:
            if(fEqual(p.Effect2, 5))
                g_levelVScreenFader[A].setupFader(3, 0, 65, ScreenFader::S_CIRCLE,
                                                  true,
                                                  Maths::iRound(warp_enter.X + warp_enter.Width / 2),
                                                  Maths::iRound(warp_enter.Y + warp_enter.Height / 2), A);
            break;

        case LevelDoor::TRANSIT_FLIP_H:
            if(fEqual(p.Effect2, 5))
                g_levelVScreenFader[A].setupFader(3, 0, 65, ScreenFader::S_FLIP_H);
            break;

        case LevelDoor::TRANSIT_FLIP_V:
            if(fEqual(p.Effect2, 5))
                g_levelVScreenFader[A].setupFader(3, 0, 65, ScreenFader::S_FLIP_V);
            break;
        }

        if(p.Effect2 >= 30)
        {
            if(warp.NoYoshi)
            {
                if(OwedMount[A] == 0 && p.Mount > 0 && p.Mount != 2)
                {
                    OwedMount[A] = p.Mount;
                    OwedMountType[A] = p.MountType;
                }

                p.Mount = 0;
                p.MountType = 0;
                SizeCheck(Player[A]);
                p.MountOffsetY = 0;
                p.Frame = 1;
                UpdateYoshiMusic();
            }

            p.Location.X = warp_exit.X + warp_exit.Width / 2.0 - p.Location.Width / 2.0;
            p.Location.Y = warp_exit.Y + warp_exit.Height - p.Location.Height;

            int last_section = p.Section;

            CheckSection(A);

            bool same_section = (last_section == p.Section);

            if(p.HoldingNPC > 0)
            {
                if(!p.Controls.Run)
                    p.Controls.Run = true;

                PlayerGrabCode(A);
            }

            p.Effect = 0;
            p.Effect2 = 0;
            p.WarpCD = 40;

            if(g_levelVScreenFader[A].isVisible() || warp.transitEffect == LevelDoor::TRANSIT_SCROLL)
            {
                switch(warp.transitEffect)
                {
                default:
                case LevelDoor::TRANSIT_NONE:
                    g_levelVScreenFader[A].setupFader(g_config.EnableInterLevelFade ? 8 : 64, 65, 0, ScreenFader::S_FADE);
                    break;

                case LevelDoor::TRANSIT_SCROLL:
                    if(same_section)
                    {
                        qScreenX[A] = vScreenX[A];
                        qScreenY[A] = vScreenY[A];
                        qScreenLoc[A] = vScreen[A];
                        qScreen = true;
                    }
                    // follows fade logic if cross section
                    else if(g_levelVScreenFader[A].isVisible())
                    {
                        g_levelVScreenFader[A].setupFader(3, 65, 0, ScreenFader::S_FADE);
                    }
                    break;

                case LevelDoor::TRANSIT_FADE:
                    g_levelVScreenFader[A].setupFader(3, 65, 0, ScreenFader::S_FADE);
                    break;

                case LevelDoor::TRANSIT_CIRCLE_FADE:
                    g_levelVScreenFader[A].setupFader(2, 65, 0, ScreenFader::S_CIRCLE,
                                                      true,
                                                      Maths::iRound(warp_exit.X + warp_exit.Width / 2),
                                                      Maths::iRound(warp_exit.Y + warp_exit.Height /2),
                                                      A);
                    break;

                case LevelDoor::TRANSIT_FLIP_H:
                    g_levelVScreenFader[A].setupFader(3, 65, 0, ScreenFader::S_FLIP_H);
                    break;

                case LevelDoor::TRANSIT_FLIP_V:
                    g_levelVScreenFader[A].setupFader(3, 65, 0, ScreenFader::S_FLIP_V);
                    break;
                }
            }

            if(warp.level != STRINGINDEX_NONE)
            {
                GoToLevel = GetS(warp.level);
                GoToLevelNoGameThing = warp.noEntranceScene;
                p.Effect = 8;
                p.Effect2 = 3000;
                ReturnWarp = p.Warp;

                if(IsEpisodeIntro && NoMap)
                    ReturnWarpSaved = ReturnWarp;

                StartWarp = warp.LevelWarp;
            }
            else if(warp.MapWarp)
            {
                p.Effect = 8;
                p.Effect2 = 2970;
            }

            if(numPlayers > 2 /*&& nPlay.Online == false*/)
            {
                for(B = 1; B <= numPlayers; B++)
                {
                    if(B != A)
                    {
                        Player[B].Location.Y = p.Location.Y + p.Location.Height - Player[B].Location.Height;
                        Player[B].Location.X = p.Location.X + p.Location.Width / 2.0 - Player[B].Location.Width / 2.0;
                        Player[B].Location.SpeedY = dRand() * 24 - 12;
                        CheckSection(B);

                        if(Player[B].HoldingNPC > 0)
                        {
                            if(Player[B].Direction > 0)
                                NPC[Player[B].HoldingNPC].Location.X = Player[B].Location.X + Physics.PlayerGrabSpotX[Player[B].Character][Player[B].State];
                            else
                                NPC[Player[B].HoldingNPC].Location.X = Player[B].Location.X + Player[B].Location.Width - Physics.PlayerGrabSpotX[Player[B].Character][Player[B].State] - NPC[p.HoldingNPC].Location.Width;

                            NPC[p.HoldingNPC].Location.Y = p.Location.Y + Physics.PlayerGrabSpotY[p.Character][p.State] + 32 - NPC[p.HoldingNPC].Location.Height;
                            NPC[Player[B].HoldingNPC].Section = Player[B].Section;
                        }
                    }
                }
            }
        }
    }
    else if(p.Effect == 8) // Holding Pattern
    {
        if(p.Effect2 < 0)
        {
            p.Location.X = Player[-p.Effect2].Location.X;
            p.Location.Y = Player[-p.Effect2].Location.Y;
            if(Player[-p.Effect2].Dead)
                p.Dead = true;
        }
        else if(p.Effect2 <= 30)
        {
            p.Effect2 -= 1;
            if(p.Effect2 == 0.0)
            {
                p.Effect = 0;
                p.Effect2 = 0;
            }
        }
        else if(fEqual(p.Effect2, 131))
        {
            tempBool = false;
            for(B = 1; B <= numPlayers; B++)
            {
                if(B != A && CheckCollision(p.Location, Player[B].Location))
                    tempBool = true;
            }

            if(!tempBool)
            {
                p.Effect2 = 130;

                for(int c = 1; c <= numBackground; c++)
                {
                    if(CheckCollision(Warp[p.Warp].Exit, Background[c].Location))
                    {
                        if(Background[c].Type == 88)
                            NewEffect(54, Background[c].Location);
                        else if(Background[c].Type == 87)
                            NewEffect(55, Background[c].Location);
                        else if(Background[c].Type == 107)
                            NewEffect(59, Background[c].Location);
                    }
                }

                SoundPause[46] = 0;
                PlaySound(SFX_Door);
            }
        }
        else if(p.Effect2 <= 130)
        {
            p.Effect2 -= 1;
            if(fEqual(p.Effect2, 100))
            {
                p.Effect = 0;
                p.Effect2 = 0;
            }
        }
        else if(p.Effect2 <= 300)
        {
            p.Effect2 -= 1;
            if(fEqual(p.Effect2, 200))
            {
                p.Effect2 = 100;
                p.Effect = 3;
            }
        }
        else if(p.Effect2 <= 1000) // Start Wait
        {
            p.Effect2 -= 1;
            if(fEqual(p.Effect2, 900))
            {
                p.Effect = 3;
                p.Effect2 = 100;
                if(A == 2)
                {
                    p.Effect = 8;
                    p.Effect2 = 300;
                }
            }
        }
        else if(p.Effect2 <= 2000) // Start Wait
        {
            p.Effect2 -= 1;

            if(fEqual(p.Effect2, 1900))
            {
                for(int c = 1; c <= numBackground; c++)
                {
                    if(CheckCollision(Warp[p.Warp].Exit, Background[c].Location))
                    {
                        if(Background[c].Type == 88)
                            NewEffect(54, Background[c].Location);
                        else if(Background[c].Type == 87)
                            NewEffect(55, Background[c].Location);
                        else if(Background[c].Type == 107)
                            NewEffect(59, Background[c].Location);
                    }
                }

                SoundPause[46] = 0;
                PlaySound(SFX_Door);
                p.Effect = 8;
                p.Effect2 = 30;

                if(A == 2)
                {
                    p.Effect = 8;
                    p.Effect2 = 131;
                }
            }
        }
        else if(p.Effect2 <= 3000) // warp wait
        {
            p.Effect2 -= 1;

            auto &w = Warp[p.Warp];

            if(g_config.EnableInterLevelFade && (w.MapWarp || w.level != STRINGINDEX_NONE) && Maths::iRound(p.Effect2) == 2955 && !g_levelScreenFader.isFadingIn())
                g_levelScreenFader.setupFader(2, 0, 65, ScreenFader::S_FADE);

            if(fEqual(p.Effect2, 2920))
            {
                if(w.MapWarp)
                {
                    LevelBeatCode = 6;

                    if(!(w.MapX == -1 && w.MapY == -1))
                    {
                        WorldPlayer[1].Location.X = w.MapX;
                        WorldPlayer[1].Location.Y = w.MapY;

                        for(int l = 1; l <= numWorldLevels; ++l)
                        {
                            if(CheckCollision(WorldPlayer[1].Location, WorldLevel[l].Location))
                            {
                                WorldLevel[l].Active = true;
                                curWorldLevel = l;
                            }
                        }
                    }
                }
                EndLevel = true;
                return;
            }
        }
    }
    else if(p.Effect == 4) // Player got fire power
    {
        if(p.Duck && p.Character != 5)
        {
            UnDuck(Player[A]);
            p.Frame = 1;
        }

        p.Effect2 += 1;

        if(fEqual(p.Effect2 / 5, std::floor(p.Effect2 / 5.0)))
        {
            if(p.State == 1 && p.Character != 5)
            {
                p.State = 2;

                if(p.Mount == 0)
                {
                    p.Location.X += -Physics.PlayerWidth[p.Character][2] * 0.5 + Physics.PlayerWidth[p.Character][1] * 0.5;
                    p.Location.Y += -Physics.PlayerHeight[p.Character][2] + Physics.PlayerHeight[p.Character][1];
                    p.Location.Width = Physics.PlayerWidth[p.Character][p.State];
                    p.Location.Height = Physics.PlayerHeight[p.Character][p.State];
                }
                else if(p.Mount == 3)
                {
                    YoshiHeight(A);
                }
                else if(p.Character == 2 && p.Mount != 2)
                {
                    p.Location.Y += -Physics.PlayerHeight[2][2] + Physics.PlayerHeight[1][2];
                    p.Location.Height = Physics.PlayerHeight[p.Character][p.State];
                }
            }
            else if(p.State != 3)
                p.State = 3;
            else
                p.State = 2;
        }

        if(p.Effect2 >= 50)
        {
            if(p.State == 2)
                p.State = 3;
            p.Immune += 50;
            p.Immune2 = true;
            p.Effect = 0;
            p.Effect2 = 0;
            p.StandUp = true;
        }
    }
    else if(p.Effect == 41) // Player got ice power
    {
        if(p.Duck && p.Character != 5)
        {
            UnDuck(Player[A]);
            p.Frame = 1;
        }

        p.Effect2 += 1;

        if(fEqual(p.Effect2 / 5, std::floor(p.Effect2 / 5.0)))
        {
            if(p.State == 1 && p.Character != 5)
            {
                p.State = 2;
                if(p.Mount == 0)
                {
                    p.Location.X += -Physics.PlayerWidth[p.Character][2] * 0.5 + Physics.PlayerWidth[p.Character][1] * 0.5;
                    p.Location.Y += -Physics.PlayerHeight[p.Character][2] + Physics.PlayerHeight[p.Character][1];
                    p.Location.Width = Physics.PlayerWidth[p.Character][p.State];
                    p.Location.Height = Physics.PlayerHeight[p.Character][p.State];
                }
                else if(p.Mount == 3)
                {
                    YoshiHeight(A);
                }
                else if(p.Character == 2 && p.Mount != 2)
                {
                    p.Location.Y += -Physics.PlayerHeight[2][2] + Physics.PlayerHeight[1][2];
                    p.Location.Height = Physics.PlayerHeight[p.Character][p.State];
                }
            }
            else if(p.State != 7)
                p.State = 7;
            else
                p.State = 2;
        }

        if(p.Effect2 >= 50)
        {
            if(p.State == 2)
                p.State = 7;
            p.Immune += 50;
            p.Immune2 = true;
            p.Effect = 0;
            p.Effect2 = 0;
            p.StandUp = true;
        }
    }
    else if(p.Effect == 5) // Player got a leaf
    {
        p.Frame = 1;

        if(p.Effect2 == 0.0)
        {
            if(p.State == 1 && p.Mount == 0)
            {
                p.Location.X += -Physics.PlayerWidth[p.Character][2] * 0.5 + Physics.PlayerWidth[p.Character][1] * 0.5;
                p.Location.Y += -Physics.PlayerHeight[p.Character][2] + Physics.PlayerHeight[p.Character][1];
                p.State = 4;
                p.Location.Width = Physics.PlayerWidth[p.Character][p.State];
                p.Location.Height = Physics.PlayerHeight[p.Character][p.State];
            }
            else if(p.Mount == 3)
            {
                YoshiHeight(A);
            }
            else if(p.Character == 2 && p.State == 1 && p.Mount == 1)
            {
                p.Location.Y += -Physics.PlayerHeight[2][2] + Physics.PlayerHeight[1][2];
                p.Location.Height = Physics.PlayerHeight[p.Character][4];
            }
            p.State = 4;
            tempLocation.Width = 32;
            tempLocation.Height = 32;
            tempLocation.X = p.Location.X + p.Location.Width / 2.0 - tempLocation.Width / 2.0;
            tempLocation.Y = p.Location.Y + p.Location.Height / 2.0 - tempLocation.Height / 2.0;
            NewEffect(131, tempLocation, 1, 0, ShadowMode);
        }

        p.Effect2 += 1;

        if(fEqual(p.Effect2, 14))
        {
            p.Immune += 50;
            p.Immune2 = true;
            p.Effect = 0;
            p.Effect2 = 0;
            p.StandUp = true;
        }
    }
    else if(p.Effect == 11) // Player got a tanooki suit
    {
        p.Frame = 1;
        p.Immune2 = true;
        if(p.Effect2 == 0.0)
        {
            if(p.State == 1 && p.Mount == 0)
            {
                p.Location.X += -Physics.PlayerWidth[p.Character][2] * 0.5 + Physics.PlayerWidth[p.Character][1] * 0.5;
                p.Location.Y += -Physics.PlayerHeight[p.Character][2] + Physics.PlayerHeight[p.Character][1];
                p.State = 5;
                p.Location.Width = Physics.PlayerWidth[p.Character][p.State];
                p.Location.Height = Physics.PlayerHeight[p.Character][p.State];
            }
            else if(p.Mount == 3)
            {
                YoshiHeight(A);
            }
            else if(p.Character == 2 && p.State == 1 && p.Mount == 1)
            {
                p.Location.Y += -Physics.PlayerHeight[2][2] + Physics.PlayerHeight[1][2];
                p.Location.Height = Physics.PlayerHeight[p.Character][4];
            }
            p.State = 5;
            tempLocation.Width = 32;
            tempLocation.Height = 32;
            tempLocation.X = p.Location.X + p.Location.Width / 2.0 - tempLocation.Width / 2.0;
            tempLocation.Y = p.Location.Y + p.Location.Height / 2.0 - tempLocation.Height / 2.0;
            NewEffect(131, tempLocation, 1, 0, ShadowMode);
        }
        p.Effect2 += 1;
        if(fEqual(p.Effect2, 14))
        {
            p.Immune += 50;
            p.Immune2 = true;
            p.Effect = 0;
            p.Effect2 = 0;
            p.StandUp = true;
        }
    }
    else if(p.Effect == 12) // Player got a hammer suit
    {
        p.Frame = 1;
        p.Immune2 = true;

        if(p.Effect2 == 0.0)
        {
            if(p.State == 1 && p.Mount == 0)
            {
                p.Location.X += -Physics.PlayerWidth[p.Character][2] * 0.5 + Physics.PlayerWidth[p.Character][1] * 0.5;
                p.Location.Y += -Physics.PlayerHeight[p.Character][2] + Physics.PlayerHeight[p.Character][1];
                p.State = 5;
                p.Location.Width = Physics.PlayerWidth[p.Character][p.State];
                p.Location.Height = Physics.PlayerHeight[p.Character][p.State];
            }
            else if(p.Mount == 3)
            {
                YoshiHeight(A);
            }
            else if(p.Character == 2 && p.State == 1 && p.Mount == 1)
            {
                p.Location.Y += -Physics.PlayerHeight[2][2] + Physics.PlayerHeight[1][2];
                p.Location.Height = Physics.PlayerHeight[p.Character][6];
            }

            p.State = 6;
            tempLocation.Width = 32;
            tempLocation.Height = 32;
            tempLocation.X = p.Location.X + p.Location.Width / 2.0 - tempLocation.Width / 2.0;
            tempLocation.Y = p.Location.Y + p.Location.Height / 2.0 - tempLocation.Height / 2.0;
            NewEffect(131, tempLocation, 1, 0, ShadowMode);
        }

        p.Effect2 += 1;

        if(p.Effect2 == 14.0)
        {
            p.Immune += 50;
            p.Immune2 = true;
            p.Effect = 0;
            p.Effect2 = 0;
            p.StandUp = true;
        }
    }
    else if(p.Effect == 500) // Change to / from tanooki
    {
        for(B = 1; B <= 2; B++)
        {
            NewEffect(80, newLoc(p.Location.X + dRand() * (p.Location.Width + 8) - 8,
                                 p.Location.Y + dRand() * (p.Location.Height + 8) - 4), 1, 0, ShadowMode);
            Effect[numEffects].Location.SpeedX = dRand() * 2 - 1;
            Effect[numEffects].Location.SpeedY = dRand() * 2 - 1;
        }

        if(p.Effect2 == 0.0)
        {
            UnDuck(Player[A]);
            PlaySound(SFX_Raccoon);
            tempLocation.Width = 32;
            tempLocation.Height = 32;
            tempLocation.X = p.Location.X + p.Location.Width / 2.0 - tempLocation.Width / 2.0;
            tempLocation.Y = p.Location.Y + p.Location.Height / 2.0 - tempLocation.Height / 2.0;
            NewEffect(10, tempLocation, 1, 0, ShadowMode);

            if(!p.Stoned)
            {
                p.Frame = 0;
                p.Stoned = true;
                p.StonedCD = 15;
            }
            else
            {
                p.StonedCD = 60;
                p.Frame = 1;
                p.Stoned = false;
            }
        }

        p.Effect2 += 1;
        p.Immune = 10;
        p.Immune2 = true;
        p.StonedTime = 0;

        if(p.Effect2 >= 5)
        {
            p.Effect2 = 0;
            p.Effect = 0;
            p.Immune = 0;
            p.Immune2 = false;
        }
    }
    else if(p.Effect == 9) // MultiMario
    {
        if(p.HoldingNPC > numNPCs) // Can't hold an NPC that is dead
            p.HoldingNPC = 0;

        if(p.HoldingNPC > 0)
        {
            NPC[p.HoldingNPC].Effect = 0;
            NPC[p.HoldingNPC].CantHurt = Physics.NPCCanHurtWait;
            NPC[p.HoldingNPC].CantHurtPlayer = A;

            if(p.Direction > 0)
                NPC[p.HoldingNPC].Location.X = p.Location.X + Physics.PlayerGrabSpotX[p.Character][p.State];
            else
                NPC[p.HoldingNPC].Location.X = p.Location.X + p.Location.Width - Physics.PlayerGrabSpotX[p.Character][p.State] - NPC[p.HoldingNPC].Location.Width;

            NPC[p.HoldingNPC].Location.Y = p.Location.Y + Physics.PlayerGrabSpotY[p.Character][p.State] + 32 - NPC[p.HoldingNPC].Location.Height;
        }

        p.MountSpecial = 0;
        p.YoshiTongueLength = 0;
        p.Immune += 1;

        if(p.Immune >= 5)
        {
            p.Immune = 0;
            if(p.Immune2)
                p.Immune2 = false;
            else
                p.Immune2 = true;
        }

        tempBool = true;

        for(B = 1; B <= numPlayers; B++)
        {
            if(B != A && (Player[B].Effect == 0 || fEqual(B, p.Effect2)) && !Player[B].Dead && Player[B].TimeToLive == 0 && CheckCollision(p.Location, Player[B].Location))
                tempBool = false;
        }

        if(tempBool)
        {
            p.Effect = 0;
            p.Effect2 = 0;
            p.Immune = 0;
            p.Immune2 = false;
            p.Location.SpeedY = 0.01;
        }
        else if(p.Effect2 > 0)
        {
            D = float(p.Effect2);

            if(Player[D].Effect == 0)
                p.Effect2 = 0;

            p.Immune2 = true;
            p.Location.X = Player[D].Location.X + Player[D].Location.Width / 2.0 - p.Location.Width / 2.0;
            p.Location.Y = Player[D].Location.Y + Player[D].Location.Height - p.Location.Height;
        }
    }
#if 0 /* FIXME: Dead code, because of Redigit's mistake */
    else if(p.Effect == 9) // Yoshi eat
    {
        p.HoldingNPC = 0;
        p.StandingOnNPC = 0;

        if(Player[p.Effect2].YoshiPlayer != A)
        {
            p.Effect = 0;
            p.Effect2 = 0;
        }
    }
#endif
    else if(p.Effect == 10) // Yoshi swallow
    {
        p.HoldingNPC = 0;
        p.StandingOnNPC = 0;
        p.Section = Player[p.Effect2].Section;
        p.Location.X = Player[p.Effect2].Location.X + Player[p.Effect2].Location.Width / 2.0 - p.Location.Width / 2.0;
        p.Location.Y = Player[p.Effect2].Location.Y + Player[p.Effect2].Location.Height / 2.0 - p.Location.Height / 2.0;

        if(Player[p.Effect2].YoshiPlayer != A)
        {
            p.Effect = 0;
            p.Effect2 = 0;
        }
    }
    else if(p.Effect == 6) // player stole a heldbonus
    {
        p.Immune += 1;

        if(p.Immune >= 5)
        {
            p.Immune = 0;
            if(p.Immune2)
                p.Immune2 = false;
            else
                p.Immune2 = true;
        }

        p.Location.Y += 2.2;

        if(p.Location.Y >= p.Effect2)
        {
            p.Location.Y = p.Effect2;
            tempBool = true;
            for(B = 1; B <= numPlayers; B++)
            {
                if(B != A && Player[B].Effect != 6 && CheckCollision(p.Location, Player[B].Location))
                    tempBool = false;
                // tempBool = False
            }
            if(tempBool)
            {
                p.Effect = 0;
                p.Effect2 = 0;
                p.Immune = 50;
                p.Immune2 = false;
                p.Location.SpeedY = 0.01;
            }
        }
        for(B = 1; B <= numPlayers; B++)
        {
            if(B != A && CheckCollision(p.Location, Player[B].Location))
            {
                if(Player[B].Mount == 2)
                {
                    p.Effect = 0;
                    p.Immune = 50;
                    p.Immune2 = false;
                    p.Location.Y = Player[B].Location.Y - p.Location.Height;
                    p.Location.SpeedY = 0.01;
                }
            }
        }
    }

    if(p.Mount == 3 && p.Effect != 9)
    {
        PlayerFrame(p);
    }
//    if(Player[A].Effect == 0)
//    {
//        if(nPlay.Online && A == nPlay.MySlot + 1)
//            Netplay::sendData Netplay::PutPlayerControls(nPlay.MySlot) + "1c" + std::to_string(A) + "|" + Player[A].Effect + "|" + Player[A].Effect2 + LB + "1h" + std::to_string(A) + "|" + Player[A].State + LB;
//    }
}

// make a death effect for player and release all items linked to them.
// used for the Die SwapCharacter and for the DropPlayer.
// do this BEFORE changing/erasing any player fields
void PlayerGone(const int A)
{
    PlaySound(SFX_PlayerDied2);
    if(!Player[A].Dead && Player[A].TimeToLive == 0)
    {
        PlayerDismount(A);
        if(Player[A].HoldingNPC > 0)
        {
            NPC[Player[A].HoldingNPC].HoldingPlayer = 0;
            Player[A].HoldingNPC = 0;
        }
        PlayerDeathEffect(A);
    }
}

void AddPlayer(int Character)
{
    numPlayers++;
    Player_t& p = Player[numPlayers];
    p = Player_t();
    p.Character = Character;
    p.State = SavedChar[p.Character].State;
    p.HeldBonus = SavedChar[p.Character].HeldBonus;
    p.Mount = SavedChar[p.Character].Mount;
    p.MountType = SavedChar[p.Character].MountType;
    p.Hearts = SavedChar[p.Character].Hearts;
    if(p.State == 0)
    {
        p.State = 1;
    }

    p.Frame = 1;
    if(p.Character == 3)
        p.CanFloat = true;
    p.Direction = 1.;

    SizeCheck(Player[numPlayers]);

    // the rest only matters during level play
    if(LevelSelect && (StartLevel.empty() || !NoMap))
        return;

    int alivePlayer = CheckLiving();
    if(alivePlayer == 0 || alivePlayer == numPlayers)
        alivePlayer = 1;
    p.Section = Player[alivePlayer].Section;
    RespawnPlayerTo(numPlayers, alivePlayer);

    SetupScreens();
}

void DropPlayer(const int A)
{
    PlayerGone(A);
    if(A < 1 || A > numPlayers)
        return;

    // IMPORTANT - removes all references to player A,
    //   decrements all references to higher players

    // NPC player references
    for(int C = 1; C <= numNPCs; C++)
    {
        NPC_t& n = NPC[C];
        // most of these should not be equal because PlayerGone has already been called.
        if(n.standingOnPlayer > A)
            n.standingOnPlayer --;
        else if(n.standingOnPlayer == A)
            n.standingOnPlayer = 0;
        if(n.HoldingPlayer > A)
            n.HoldingPlayer --;
        else if(n.HoldingPlayer == A)
            n.HoldingPlayer = 0;
        if(n.CantHurt > A)
            n.CantHurt --;
        else if(n.CantHurt == A)
            n.CantHurt = 0;
        if(n.CantHurtPlayer > A)
            n.CantHurtPlayer --;
        else if(n.CantHurtPlayer == A)
            n.CantHurtPlayer = 0;
        if(n.BattleOwner > A)
            n.BattleOwner --;
        else if(n.BattleOwner == A)
            n.BattleOwner = 0;
        if(n.JustActivated > A)
            n.JustActivated --;
        else if(n.JustActivated == A)
            n.JustActivated = 1;
    }

    // Block player references
    // Block[B].IsPlayer is only set for tempBlocks, so no correction here

    // Player player references
    for(int B = 1; B <= numPlayers; B++)
    {
        if(Player[B].YoshiPlayer == A)
            Player[B].YoshiPlayer = 0;
        else if(Player[B].YoshiPlayer > A)
            Player[B].YoshiPlayer --;
    }

    // saves player without their mount, but mount is still onscreen and available
    SavedChar[Player[A].Character] = Player[A];
    for(int B = A; B < numPlayers; B++)
    {
        Player[B] = std::move(Player[B+1]);
        OwedMount[B] = OwedMount[B+1];
        OwedMountType[B] = OwedMountType[B+1];
        BattleLives[B] = BattleLives[B+1];
    }

    numPlayers --;

    // the rest only matters during level play
    if(LevelSelect && (StartLevel.empty() || !NoMap))
        return;

    SetupScreens();
}

void SwapCharacter(int A, int Character, bool Die, bool FromBlock)
{
    // if already dead or respawning, don't die again
    if(Player[A].Dead || Player[A].Effect == 6)
        Die = false;
    if(Die && Lives <= 0)
    {
        return;
    }

    if(Die)
        PlayerGone(A);

    SavedChar[Player[A].Character] = Player[A];

    // the following is identical to the code moved from blocks.cpp
    Player[A].Character = Character;
    auto &p = Player[A];
    p.State = SavedChar[p.Character].State;
    p.HeldBonus = SavedChar[p.Character].HeldBonus;
    p.Mount = SavedChar[p.Character].Mount;
    p.MountType = SavedChar[p.Character].MountType;
    p.Hearts = SavedChar[p.Character].Hearts;
    if(p.State == 0)
    {
        p.State = 1;
    }
    p.FlySparks = false;
    p.Immune = 50;
    if(FromBlock)
    {
        p.Effect = 8;
        p.Effect2 = 14;
    }

    if(!Die)
    {
        if(FromBlock)
        {
            // make player top match old player top, for bricks (from blocks.cpp)
            if(p.Mount <= 1)
            {
                p.Location.Height = Physics.PlayerHeight[p.Character][p.State];
                if(p.Mount == 1 && p.State == 1)
                {
                    p.Location.Height = Physics.PlayerHeight[1][2];
                }
                p.StandUp = true;
            }
        }
        else
        {
            // make player bottom match old player bottom, to avoid floor glitches
            UnDuck(Player[A]);
            SizeCheck(Player[A]);
        }

        if(!LevelSelect)
        {
            Location_t tempLocation = p.Location;
            tempLocation.Y = p.Location.Y + p.Location.Height / 2.0 - 16;
            tempLocation.X = p.Location.X + p.Location.Width / 2.0 - 16;
            NewEffect(10, tempLocation);
        }
    }

    if(Die)
    {
        // make player bottom match old player bottom, for respawn
        Lives --;
        UnDuck(Player[A]);
        SizeCheck(Player[A]);
        RespawnPlayerTo(A, A);
        PlaySound(SFX_DropItem);
    }
}

// returns whether a player is allowed to swap characters
bool SwapCharAllowed()
{
    if(LevelSelect || GameMenu)
        return true;
    else
        return false;
}
