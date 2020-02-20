/*
 * A2xTech - A platform game engine ported from old source code for VB6
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

#include <fmt_format_ne.h>


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
    Controls_t blankControls;
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
        Checkpoint = "";
    }
    else
    {
        BattleIntro = 0;
        BattleOutro = 0;
    }


    for(A = 1; A <= numPlayers; A++) // set up players
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
        if(GrabAll == true)
            Player[A].CanGrabNPCs = true;
        else
            Player[A].CanGrabNPCs = false;
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

        if(numPlayers > 2 && GameMenu == false) // online stuff
        {
            /*if(nPlay.Online == true)
            {
                Player[A].Location = Player[1].Location;
                Player[A].Location.X = Player[A].Location.X + A * 32 - 32;
            }
            else*/
            if(GameOutro == true)
            {
                Player[A].Location = Player[1].Location;
                Player[A].Location.X = Player[A].Location.X + A * 52 - 52;
            }
            else
            {
                Player[A].Location = Player[1].Location;
                Player[A].Location.SpeedY = std::rand() % -12 - 6;
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


    if(Checkpoint == FullFileName && Checkpoint != "") // if this level has a checkpoint the put the player in the correct position
    {
        for(A = 1; A <= numNPCs; A++)
        {
            if(NPC[A].Type == 192)
            {
                NPC[A].Killed = 9;
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
                }
                if(numPlayers > 1)
                {
                    Player[1].Location.X = Player[1].Location.X - 16;
                    Player[2].Location.X = Player[2].Location.X + 16;
                }
            }
        }
    }
    else if(StartLevel != FileName) // if not in the level for the checkpoint, blank the checkpoint
        Checkpoint = "";
}

void UpdatePlayer()
{
    int A = 0;
    int B = 0;
    float C = 0;
    float D = 0;
    Controls_t blankControls;
    float speedVar = 0; // adjusts the players speed by percentages
    long long fBlock = 0; // for collision detection optimizations
    long long lBlock = 0;
    double tempSpeed = 0;
    int HitSpot = 0;
    // the hitspot is used for collision detection to find out where to put the player after it collides with a block
    // the numbers tell what side the collision happened so it can move the plaer to the correct position
    // 1 means the player hit the block from the top
    // 2 is from the right
    // 3 is from the bottom
    // 4 is from the left
    RangeArr<int, 1, 2> tempBlockHit;
    RangeArr<double, 1, 2> tempBlockA;
    bool tempHit = false;
    bool tempSpring = false;
    bool tempShell = false;
    bool tempHit2 = false;
    int tempHit3 = 0;
    float tempHitSpeed = 0;
    float oldSpeedY = 0; // holds the players previous Y speed
    Location_t tempLocation;
    Location_t tempLocation3;
    bool spinKill = false;
    int oldSlope = 0;
    float A1 = 0;
    float B1 = 0;
    float C1 = 0;
    float X = 0;
    float Y = 0;
//    bool canWarp = false;
    bool tempBool = false;
    NPC_t blankNPC;
    int MessageNPC = 0;
    // used for slope calculations
    double PlrMid = 0;
    double Slope = 0;
    int tempSlope = 0;
    int tempSlope2 = 0;
    double tempSlope2X = 0; // The old X before player was moved
    int tempSlope3 = 0; // keeps track of hit 5 for slope detection
    bool movingBlock = false; // helps with collisions for moving blocks
    int blockPushX = 0;
    Location_t oldLoc;
//    Location_t curLoc;
//    int oldGrab = 0; // SET BUT NOT USED
    bool DontResetGrabTime = false; // helps with grabbing things from the top
    double SlippySpeedX = 0;
    bool wasSlippy = false;
    double Angle = 0;
    double slideSpeed = 0;
//    void *maxSlideSpeed = nullptr;


    StealBonus(); // allows a dead player to come back to life by using a 1-up
    ClownCar(); // updates players in the clown car

    // online stuff
//    if(nPlay.Online == true)
//    {
//        A = nPlay.MySlot + 1;
//        nPlay.PlayerWaitCount = nPlay.PlayerWaitCount + 1;
//        if(Player[A].Dead == true || Player[A].TimeToLive > 0)
//        {
//            if(nPlay.PlayerWaitCount == 10)
//            {
//                nPlay.PlayerWaitCount = 0;
//                Netplay::sendData "1p" + std::to_string(A) + LB;
//            }
//        }
//        else if(Player[A].Effect != 0)
//        {
//            if(nPlay.PlayerWaitCount >= 10)
//            {
//                Netplay::sendData Netplay::PutPlayerLoc(nPlay.MySlot) + "1c" + std::to_string(A) + "|" + Player[A].Effect + "|" + Player[A].Effect2 + LB + "1h" + std::to_string(A) + "|" + Player[A].State + LB;
//                nPlay.PlayerWaitCount = 0;
//            }
//        }
//        else
//        {
//            if(nPlay.PlayerWaitCount >= 6 && (Player[nPlay.MySlot + 1].Location.SpeedX != 0 || Player[nPlay.MySlot + 1].Location.SpeedY != 0) || nPlay.PlayerWaitCount >= 60)
//            {
//                Netplay::sendData Netplay::PutPlayerLoc(nPlay.MySlot);
//                if(Player[A].YoshiPlayer > 0)
//                    Netplay::sendData Netplay::PutPlayerLoc(Player[A].YoshiPlayer - 1) + "1c" + Player[A].YoshiPlayer + "|" + Player[Player[A].YoshiPlayer].Effect + "|" + Player[Player[A].YoshiPlayer].Effect2 + LB + "1h" + std::to_string(A) + "|" + Player[Player[A].YoshiPlayer].State + LB;
//                if(Player[A].Mount == 3)
//                    Netplay::sendData "1r" + std::to_string(A) + "|" + Player[A].YoshiPlayer + LB;
//                nPlay.PlayerWaitCount = 0;
//            }
//        }
//    }


    // A is the current player, numPlayers is the last player. this loop updates all the players
    for(A = 1; A <= numPlayers; A++)
    {
//        if(nPlay.Online == true && A > 1)
//        {
//            if(nPlay.Player[A - 1].Active == false)
//                Player[A].Dead = true;
//            if(Player[A].Dead == true)
//                nPlay.Player[A - 1].Controls = blankControls;
//        }
        // reset variables from the previous player
        DontResetGrabTime = false;
//        oldGrab = Player[A].HoldingNPC; // SET BUT NOT UNUSED
        movingBlock = false;
        blockPushX = 0;
        Player[A].ShowWarp = 0;
        Player[A].mountBump = 0;
        spinKill = false;
        tempHit = false;
        tempHit2 = false;
        tempHit3 = 0;
        tempBlockHit[1] = 0;
        tempBlockHit[2] = 0;
        tempBlockA[1] = 0;
        tempBlockA[2] = 0;
        if(Player[A].GrabTime > 0) // if grabbing something, take control away from the player
        {
            Player[A].Slide = false;
            Player[A].Controls.Run = true;
            Player[A].Controls.Down = true;
            Player[A].Controls.AltRun = false;
            Player[A].Controls.Jump = false;
            Player[A].Controls.AltJump = false;
        }
        if(Player[A].Dismount > 0) // count down to being able to hop in a shoe or yoshi
            Player[A].Dismount = Player[A].Dismount - 1;
        if(Player[A].Mount != 0 || Player[A].Stoned == true || Player[A].Fairy == true) // if .holdingnpc is -1 then the player can't grab anything. this stops the player from grabbing things while on a yoshi/shoe
            Player[A].HoldingNPC = -1;
        else if(Player[A].HoldingNPC == -1)
            Player[A].HoldingNPC = 0;
        if(Player[A].Controls.Drop == true && Player[A].DropRelease == true) // this is for the single player coop cheat code
        {
            if(SingleCoop > 0 && Player[A].Controls.Down == true)
                SwapCoop();
            else
                DropBonus(A);
        }
        if(Player[A].Controls.Drop == true) // for dropping something from the container. this makes the player have to let go of the drop button before dropping something else
            Player[A].DropRelease = false;
        else
            Player[A].DropRelease = true;
        // Handle the death effecs
        if(Player[A].TimeToLive > 0)
        {
            Player[A].TimeToLive = Player[A].TimeToLive + 1;
            if(Player[A].TimeToLive >= 200 || ScreenType != 5)
            {
                B = CheckLiving();
                if(BattleMode == true && BattleLives[1] > 0 && BattleLives[2] > 0 && BattleWinner == 0)
                {
                    B = 20 + A;
                    Player[20 + A].Location.Width = Player[A].Location.Width;
                    Player[20 + A].Location.Height = Player[A].Location.Height;
                    Player[20 + A].Location.X = PlayerStart[A].X + PlayerStart[A].Width * 0.5 - Player[A].Location.Width;
                    Player[20 + A].Location.Y = PlayerStart[A].Y + PlayerStart[A].Height - Player[A].Location.Height;
                    CheckSection(20 + A);
                    if(Player[A].Section != Player[B].Section)
                    {
                        Player[A].Location = Player[B].Location;
                        Player[A].Section = Player[B].Section;
                    }
                }
                if(B > 0) // Move camera to the other living players
                {
                    if(ScreenType == 5)
                    {
                        A1 = (Player[B].Location.X + Player[B].Location.Width * 0.5) - (Player[A].Location.X + Player[A].Location.Width * 0.5);
                        B1 = Player[B].Location.Y - Player[A].Location.Y;
                    }
                    else
                    {
                        A1 = (float)((-vScreenX[1] + vScreen[1].Width * 0.5) - (Player[A].Location.X + Player[A].Location.Width * 0.5));
                        B1 = (float)((-vScreenY[1] + vScreen[1].Height * 0.5) - Player[A].Location.Y);
                    }
                    C1 = std::sqrt((A1 * A1) + (B1 * B1));
                    if(C1 != 0.0f)
                    {
                        X = A1 / C1;
                        Y = B1 / C1;
                    }
                    else
                    {
                        X = 0;
                        Y = 0;
                    }
                    Player[A].Location.X = Player[A].Location.X + X * 10;
                    Player[A].Location.Y = Player[A].Location.Y + Y * 10;
                    if(ScreenType == 5 && Player[1].Section != Player[2].Section)
                    {
                        C1 = 0;
                        if(A == 1)
                        {
                            Player[A].Location.X = Player[2].Location.X;
                            Player[A].Location.Y = Player[2].Location.Y;
                            CheckSection(A);
                        }
                        else
                        {
                            Player[A].Location.X = Player[1].Location.X;
                            Player[A].Location.Y = Player[1].Location.Y;
                            CheckSection(A);
                        }
                    }
                    if(C1 < 10 && C1 > -10)
                        KillPlayer(A);
                }
                else if(Player[A].TimeToLive >= 200) // ScreenType = 1
                    KillPlayer(A); // Time to die
            }
        }
        else if(Player[A].Dead == true)
        {
            if(numPlayers > 2)
            {
                B = CheckLiving();
                Player[A].Location.X = Player[B].Location.X;
                Player[A].Location.Y = Player[B].Location.Y;
                Player[A].Section = Player[B].Section;
            }
            else
            {
                if(A == 1)
                {
                    Player[A].Location.X = Player[2].Location.X;
                    Player[A].Location.Y = Player[2].Location.Y;
                    CheckSection(A);
                }
                else
                {
                    Player[A].Location.X = Player[1].Location.X;
                    Player[A].Location.Y = Player[1].Location.Y;
                    CheckSection(A);
                }
            }
        }
        else if(Player[A].Dead == false)
        {
            oldLoc = Player[A].Location;
            if(Player[A].SlideCounter > 0) // for making the slide Effect
                Player[A].SlideCounter = Player[A].SlideCounter - 1;

            // for the purple yoshi ground pound
            if(Player[A].Effect == 0)
            {
                if(Player[A].Location.SpeedY != 0 && Player[A].StandingOnNPC == 0 && Player[A].Slope == 0)
                {
                    if(Player[A].Mount == 3 && Player[A].MountType == 6) // Purple Yoshi Pound
                    {
                        if(Player[A].Controls.Down == true && Player[A].DuckRelease == true && Player[A].CanPound == true)
                        {
                            Player[A].GroundPound = true;
                            Player[A].GroundPound2 = true;
                            if(Player[A].Location.SpeedY < 0)
                                Player[A].Location.SpeedY = 0;
                        }
                    }
                }
                else
                    Player[A].CanPound = false;
                if(Player[A].GroundPound == true)
                {
                    if(Player[A].CanPound == false && Player[A].Location.SpeedY < 0)
                        Player[A].GroundPound = false;
                    Player[A].Controls.Down = true;
                    Player[A].CanJump = false;
                    Player[A].Controls.Left = false;
                    Player[A].Controls.Up = false;
                    Player[A].Controls.Right = false;
                    Player[A].Controls.Jump = true;
                    Player[A].Location.SpeedX = Player[A].Location.SpeedX * 0.95;
                    Player[A].RunRelease = false;
                    Player[A].CanFly = false;
                    Player[A].FlyCount = 0;
                    Player[A].CanFly2 = false;
                    Player[A].Location.SpeedY = Player[A].Location.SpeedY + 1;
                    Player[A].CanPound = false;
                    Player[A].Jump = 0;
                }
                else
                {
                    if(Player[A].Location.SpeedY < -5 && ((Player[A].Jump < 15 && Player[A].Jump != 0) || Player[A].CanFly == true))
                        Player[A].CanPound = true;
                    if(Player[A].GroundPound2 == true)
                    {
                        Player[A].Location.SpeedY = -4;
                        Player[A].StandingOnNPC = 0;
                        Player[A].GroundPound2 = false;
                    }
                }

                SizeCheck(A); // check that the player is the correct size for it's character/state/mount and set it if not

                if(Player[A].Stoned == true) // stop the player from climbing/spinning/jumping when in tanooki statue form
                {
                    Player[A].Jump = 0;
                    Player[A].Vine = 0;
                    Player[A].SpinJump = false;
                    Player[A].Controls.Left = false;
                    Player[A].Controls.Right = false;
                    Player[A].Controls.AltJump = false;
                    Player[A].Controls.Jump = false;
                    Player[A].CanAltJump = false;
                    Player[A].CanJump = false;
                }

                // let the player slide if not on a mount and holding something
                if(Player[A].GrabTime > 0)
                    Player[A].Slide = false;
                if(Player[A].Slope > 0 && Player[A].Controls.Down == true && Player[A].Mount == 0 && Player[A].HoldingNPC == 0 && !(Player[A].Character == 3 || Player[A].Character == 4 || Player[A].Character == 5) && Player[A].GrabTime == 0)
                {
                    if(Player[A].Duck == true)
                        UnDuck(A);
                    Player[A].Slide = true;
                }
                else if(Player[A].Location.SpeedX == 0.0)
                    Player[A].Slide = false;
                if(Player[A].Mount > 0 || Player[A].HoldingNPC > 0)
                    Player[A].Slide = false;

                // unduck a player that should be able to duck
                if(Player[A].Duck == true && (Player[A].Character == 1 || Player[A].Character == 2) && Player[A].State == 1 && (Player[A].Mount == 0 || Player[A].Mount == 2))
                    UnDuck(A);

                if(GameMenu == true && Player[A].SpinJump == false) // force the player to look right when on the game menu
                    Player[A].Direction = 1;

                WaterCheck(A); // This sub handles all the water related stuff

                PowerUps(A); // misc power-up code

                if(Player[A].StandingOnNPC > 0)
                {
                    if(NPC[Player[A].StandingOnNPC].Type == 263 && NPC[Player[A].StandingOnNPC].Location.SpeedX == 0.0)
                        Player[A].Slippy = true;
                }
                SlippySpeedX = Player[A].Location.SpeedX;


                // Player's X movement. ---------------------------

                // Modify player's speed if he is running up/down hill
                speedVar = 1; // Speed var is a percentage of the player's speed
                if(Player[A].Slope > 0)
                {
                    if(
                        (Player[A].Location.SpeedX > 0 && BlockSlope[Block[Player[A].Slope].Type] == -1) ||
                        (Player[A].Location.SpeedX < 0 && BlockSlope[Block[Player[A].Slope].Type] == 1)
                    )
                        speedVar = (float)(1 - Block[Player[A].Slope].Location.Height / Block[Player[A].Slope].Location.Width * 0.5);
                    else if(Player[A].Slide == false)
                        speedVar = (float)(1 + (Block[Player[A].Slope].Location.Height / Block[Player[A].Slope].Location.Width * 0.5) * 0.5);
                }
                if(Player[A].Stoned == true) // if statue form reset to normal
                    speedVar = 1;
                if(Player[A].Character == 3)
                    speedVar = (float)(speedVar * 0.93);
                if(Player[A].Character == 4)
                    speedVar = (float)(speedVar * 1.07);

                // modify speedvar to slow the player down under water
                if(Player[A].Wet > 0)
                {
                    if(Player[A].Location.SpeedY == 0 || Player[A].Slope > 0 || Player[A].StandingOnNPC != 0)
                        speedVar = (float)(speedVar * 0.25); // if walking go really slow
                    else
                        speedVar = (float)(speedVar * 0.5); // if swimming go slower faster the walking
                }

                if(Player[A].Slide == true) // Code used to move the player while sliding down a slope
                {
                    if(Player[A].Slope > 0)
                    {
                        Angle = 1 / (Block[Player[A].Slope].Location.Width / static_cast<double>(Block[Player[A].Slope].Location.Height));
                        slideSpeed = 0.1 * Angle * BlockSlope[Block[Player[A].Slope].Type];
                        if(slideSpeed > 0 && Player[A].Location.SpeedX < 0)
                            Player[A].Location.SpeedX = Player[A].Location.SpeedX + slideSpeed * 2;
                        else if(slideSpeed < 0 && Player[A].Location.SpeedX > 0)
                            Player[A].Location.SpeedX = Player[A].Location.SpeedX + slideSpeed * 2;
                        else
                            Player[A].Location.SpeedX = Player[A].Location.SpeedX + slideSpeed;
                    }
                    else if(Player[A].Location.SpeedY == 0 || Player[A].StandingOnNPC != 0)
                    {
                        if(Player[A].Location.SpeedX > 0.2)
                            Player[A].Location.SpeedX = Player[A].Location.SpeedX - 0.1;
                        else if(Player[A].Location.SpeedX < -0.2)
                            Player[A].Location.SpeedX = Player[A].Location.SpeedX + 0.1;
                        else
                        {
                            Player[A].Location.SpeedX = 0;
                            Player[A].Slide = false;
                        }
                    }
                    if(Player[A].Location.SpeedX > 11)
                        Player[A].Location.SpeedX = 11;
                    if(Player[A].Location.SpeedX < -11)
                        Player[A].Location.SpeedX = -11;
                    if(Player[A].Controls.Jump == true || Player[A].Controls.AltJump == true)
                        Player[A].Slide = false;

                    // if not sliding and in the clown car
                }
                else if(Player[A].Mount == 2)
                {
                    if(Player[A].Controls.Jump == false)
                        Player[A].CanJump = true;
                    if(Player[A].Controls.AltJump == true && Player[A].CanAltJump == true) // Jump out of the Clown Car
                    {
                        Player[A].CanJump = false;
                        tempBool = true;
                        tempLocation = Player[A].Location;
                        tempLocation.Height = Physics.PlayerHeight[Player[A].Character][Player[A].State];
                        tempLocation.Y = tempLocation.Y - Physics.PlayerHeight[Player[A].Character][Player[A].State];
                        tempLocation.Width = Physics.PlayerWidth[Player[A].Character][Player[A].State];
                        tempLocation.X = tempLocation.X + 64 - tempLocation.Width / 2.0;
                        fBlock = FirstBlock[(tempLocation.X / 32) - 1];
                        lBlock = LastBlock[((tempLocation.X + tempLocation.Width) / 32.0) + 1];
                        for(B = (int)fBlock; B <= lBlock; B++)
                        {
                            if(Block[B].Invis == false && BlockIsSizable[Block[B].Type] == false && BlockOnlyHitspot1[Block[B].Type] == false && BlockNoClipping[Block[B].Type] == false && Block[B].Hidden == false)
                            {
                                if(CheckCollision(tempLocation, Block[B].Location) == true)
                                {
                                    tempBool = false;
                                    PlaySound(3);
                                }
                            }
                        }
                        for(B = 1; B <= numNPCs; B++)
                        {
                            if(NPCIsABlock[NPC[B].Type] == true && NPCStandsOnPlayer[NPC[B].Type] == false && NPC[B].Active == true && NPC[B].Type != 56)
                            {
                                if(CheckCollision(tempLocation, NPC[B].Location) == true)
                                {
                                    tempBool = false;
                                    PlaySound(3);
                                }
                            }
                        }
                        if(tempBool == true)
                        {
                            Player[A].CanJump = false;
                            PlaySound(1); // Jump sound
                            PlaySound(35);
                            Player[A].Jump = Physics.PlayerJumpHeight;
                            if(Player[A].Character == 2)
                                Player[A].Jump = Player[A].Jump + 3;
                            if(Player[A].SpinJump == true)
                                Player[A].Jump = Player[A].Jump - 6;
                            Player[A].Mount = 0;
                            numNPCs = numNPCs + 1;
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

                            for(B = 1; B <= numPlayers; B++)
                            {
                                if(B != A && Player[B].Mount != 2 && CheckCollision(Player[A].Location, Player[B].Location) == true)
                                    Player[B].Location.Y = Player[A].Location.Y - Player[B].Location.Height;
                                if(Player[B].StandingOnTempNPC == 56)
                                {
                                    Player[B].StandingOnNPC = numNPCs;
                                    Player[B].StandingOnTempNPC = 0;
                                }
                            }

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
                else if(Player[A].Driving == true) // driving
                {
                    if(Player[A].Duck == true)
                        UnDuck(A);
                    Player[A].Driving = false;
                    if(Player[A].StandingOnNPC > 0)
                    {
                        NPC[Player[A].StandingOnNPC].Special4 = 1;
                        if(Player[A].Controls.Left == true)
                            NPC[Player[A].StandingOnNPC].Special5 = -1;
                        else if(Player[A].Controls.Right == true)
                            NPC[Player[A].StandingOnNPC].Special5 = 1;
                        else
                            NPC[Player[A].StandingOnNPC].Special5 = 0;
                        if(Player[A].Controls.Up == true)
                            NPC[Player[A].StandingOnNPC].Special6 = -1;
                        else if(Player[A].Controls.Down == true)
                            NPC[Player[A].StandingOnNPC].Special6 = 1;
                        else
                            NPC[Player[A].StandingOnNPC].Special6 = 0;
                    }
                    Player[A].Location.SpeedX = 0;

                }
                else if(Player[A].Fairy == true) // if a fairy
                {
                    if(Player[A].Controls.Right == true)
                    {
                        if(Player[A].Location.SpeedX < 3)
                            Player[A].Location.SpeedX = Player[A].Location.SpeedX + 0.15;
                        if(Player[A].Location.SpeedX < 0)
                            Player[A].Location.SpeedX = Player[A].Location.SpeedX + 0.1;
                    }
                    else if(Player[A].Controls.Left == true)
                    {
                        if(Player[A].Location.SpeedX > -3)
                            Player[A].Location.SpeedX = Player[A].Location.SpeedX - 0.15;
                        if(Player[A].Location.SpeedX > 0)
                            Player[A].Location.SpeedX = Player[A].Location.SpeedX - 0.1;
                    }
                    else if(Player[A].Location.SpeedX > 0.1)
                        Player[A].Location.SpeedX = Player[A].Location.SpeedX - 0.1;
                    else if(Player[A].Location.SpeedX < -0.1)
                        Player[A].Location.SpeedX = Player[A].Location.SpeedX + 0.1;
                    else
                        Player[A].Location.SpeedX = 0;
                    // if the player is climbing a vine
                }
                else if(Player[A].Vine > 0)
                {
                    if(Player[A].StandingOnNPC > 0 && Player[A].Controls.Up == false)
                        Player[A].Vine = 0;
                    Player[A].CanFly = false;
                    Player[A].CanFly2 = false;
                    Player[A].RunCount = 0;
                    Player[A].SpinJump = false;
                    if(Player[A].Controls.Left == true)
                        Player[A].Location.SpeedX = -1.5;
                    else if(Player[A].Controls.Right == true)
                        Player[A].Location.SpeedX = 1.5;
                    else
                        Player[A].Location.SpeedX = 0;
                    if(Player[A].Controls.Up == true && Player[A].Vine > 2)
                        Player[A].Location.SpeedY = -2;
                    else if(Player[A].Controls.Down == true)
                        Player[A].Location.SpeedY = 3;
                    else
                        Player[A].Location.SpeedY = 0;
                    Player[A].Location.SpeedX = Player[A].Location.SpeedX + NPC[Player[A].VineNPC].Location.SpeedX;
                    Player[A].Location.SpeedY = Player[A].Location.SpeedY + NPC[Player[A].VineNPC].Location.SpeedY;
                }
                else
                {

                    // if none of the above apply then the player controls like normal. remeber this is for the players X movement


                    // ducking for link
                    if(Player[A].Duck == true && Player[A].WetFrame == true)
                    {
                        if(Player[A].Location.SpeedY != 0 && Player[A].Slope == 0 && Player[A].StandingOnNPC == 0)
                            UnDuck(A);
                    }
                    // the following code controls the players ability to duck
                    if(!(Player[A].Character == 5 && ((Player[A].Location.SpeedY != 0 && Player[A].Slope == 0 && Player[A].StandingOnNPC == 0) || Player[A].FireBallCD != 0))) // Link can't duck/unduck in air
                    {
                        if(Player[A].Controls.Down == true && Player[A].SpinJump == false && Player[A].Stoned == false && Player[A].Vine == 0 && Player[A].Slide == false && (Player[A].Slope == 0 || Player[A].Mount > 0 || Player[A].WetFrame == true || Player[A].Character >= 3 || Player[A].GrabTime > 0) && ((Player[A].WetFrame == false || Player[A].Character >= 3) || Player[A].Location.SpeedY == 0 || Player[A].StandingOnNPC != 0 || Player[A].Slope != 0 || Player[A].Mount == 1) && Player[A].Fairy == false && Player[A].ShellSurf == false && Player[A].Driving == false)
                        {
                            Player[A].Bumped = false;
                            if(Player[A].Mount != 2) // cant duck in the clown car
                            {
                                if(Player[A].Mount == 3) // duck on a yoshi
                                {
                                    if(Player[A].Duck == false)
                                    {
                                        Player[A].Location.Y = Player[A].Location.Y + Player[A].Location.Height;
                                        Player[A].Location.Height = 31;
                                        Player[A].Location.Y = Player[A].Location.Y - Player[A].Location.Height;
                                        Player[A].Duck = true;
                                        // If nPlay.Online = True And A = nPlay.MySlot + 1 Then Netplay.sendData Netplay.PutPlayerLoc(nPlay.MySlot) & "1q" & A & LB
//                                        if(nPlay.Online == true && A == nPlay.MySlot + 1)
//                                            Netplay::sendData "1q" + std::to_string(A) + LB;
                                    }
                                }
                                else // normal duck
                                {
                                    if((Player[A].State > 1 && Player[A].HoldingNPC <= 0) || (Player[A].Character == 3 || Player[A].Character == 4 || Player[A].Character == 5))
                                    {
                                        if(Player[A].Duck == false && Player[A].TailCount == 0) // Player ducks
                                        {
                                            if(Player[A].Character == 5)
                                                Player[A].SwordPoke = 0;
                                            Player[A].Duck = true;
                                            Player[A].Location.Y = Player[A].Location.Y + Player[A].Location.Height;
                                            Player[A].Location.Height = Physics.PlayerDuckHeight[Player[A].Character][Player[A].State];
                                            Player[A].Location.Y = Player[A].Location.Y - Player[A].Location.Height;
//                                            if(nPlay.Online == true && A == nPlay.MySlot + 1)
//                                                Netplay::sendData "1q" + std::to_string(A) + LB;
                                        }
                                    }
                                    else if(Player[A].Mount == 1)
                                    {
                                        if(Player[A].Duck == false && Player[A].TailCount == 0) // Player ducks
                                        {
                                            Player[A].Duck = true;
                                            Player[A].Location.Height = Physics.PlayerDuckHeight[1][2];
                                            Player[A].Location.Y = Player[A].Location.Y - Physics.PlayerDuckHeight[1][2] + Physics.PlayerHeight[1][2];
//                                            if(nPlay.Online == true && A == nPlay.MySlot + 1)
//                                                Netplay::sendData "1q" + std::to_string(A) + LB;
                                        }
                                    }
                                }
                            }
                        }
                        else
                        {
                            if(Player[A].Duck == true)
                                UnDuck(A);
                        }
                    }
                    C = 1;
                    // If .Character = 5 Then C = 0.94
                    if(Player[A].Character == 5)
                        C = 0.95F;
                    if(Player[A].Controls.Left == true &&
                        ((Player[A].Duck == false && Player[A].GrabTime == 0) ||
                         (Player[A].Location.SpeedY != 0.0 && Player[A].StandingOnNPC == 0 && Player[A].Slope == 0) ||
                         Player[A].Mount == 1)
                    )
                    {
                        Player[A].Bumped = false;
                        if(Player[A].Controls.Run == true || Player[A].Location.SpeedX > -Physics.PlayerWalkSpeed * speedVar || Player[A].Character == 5)
                        {
                            if(Player[A].Location.SpeedX > -Physics.PlayerWalkSpeed * speedVar * C)
                            {
                                if(Player[A].Character == 2) // LUIGI
                                    Player[A].Location.SpeedX = Player[A].Location.SpeedX + 0.1 * 0.175;
                                if(Player[A].Character == 3) // PEACH
                                    Player[A].Location.SpeedX = Player[A].Location.SpeedX + 0.05 * 0.175;
                                if(Player[A].Character == 4) // toad
                                    Player[A].Location.SpeedX = Player[A].Location.SpeedX - 0.05 * 0.175;
                                Player[A].Location.SpeedX = Player[A].Location.SpeedX - 0.1 * speedVar;
                            }
                            else // Running
                            {
                                if(Player[A].Character == 2) // LUIGI
                                    Player[A].Location.SpeedX = Player[A].Location.SpeedX + 0.05 * 0.175;
                                if(Player[A].Character == 3) // PEACH
                                    Player[A].Location.SpeedX = Player[A].Location.SpeedX + 0.025 * 0.175;
                                if(Player[A].Character == 4) // toad
                                    Player[A].Location.SpeedX = Player[A].Location.SpeedX - 0.025 * 0.175;
                                if(Player[A].Character == 5) // Link
                                    Player[A].Location.SpeedX = Player[A].Location.SpeedX - 0.025 * speedVar;
                                else // Mario
                                    Player[A].Location.SpeedX = Player[A].Location.SpeedX - 0.05 * speedVar;
                            }
                            if(Player[A].Location.SpeedX > 0)
                            {
                                Player[A].Location.SpeedX = Player[A].Location.SpeedX - 0.18;
                                if(Player[A].Character == 2) // LUIGI
                                    Player[A].Location.SpeedX = Player[A].Location.SpeedX + 0.18 * 0.29;
                                if(Player[A].Character == 3) // PEACH
                                    Player[A].Location.SpeedX = Player[A].Location.SpeedX + 0.09 * 0.29;
                                if(Player[A].Character == 4) // toad
                                    Player[A].Location.SpeedX = Player[A].Location.SpeedX - 0.09 * 0.29;
                                if(SuperSpeed == true)
                                    Player[A].Location.SpeedX = Player[A].Location.SpeedX * 0.95;
                            }
                        }
                        if(SuperSpeed == true && Player[A].Controls.Run == true)
                            Player[A].Location.SpeedX = Player[A].Location.SpeedX - 0.1;
                    }
                    else if(Player[A].Controls.Right == true && ((Player[A].Duck == false && Player[A].GrabTime == 0) || (Player[A].Location.SpeedY != 0 && Player[A].StandingOnNPC == 0 && Player[A].Slope == 0) || Player[A].Mount == 1))
                    {
                        Player[A].Bumped = false;
                        if(Player[A].Controls.Run == true || Player[A].Location.SpeedX < Physics.PlayerWalkSpeed * speedVar || Player[A].Character == 5)
                        {
                            if(Player[A].Location.SpeedX < Physics.PlayerWalkSpeed * speedVar * C)
                            {
                                if(Player[A].Character == 2) // LUIGI
                                    Player[A].Location.SpeedX = Player[A].Location.SpeedX - 0.1 * 0.175;
                                if(Player[A].Character == 3) // PEACH
                                    Player[A].Location.SpeedX = Player[A].Location.SpeedX - 0.05 * 0.175;
                                if(Player[A].Character == 4) // toad
                                    Player[A].Location.SpeedX = Player[A].Location.SpeedX + 0.05 * 0.175;
                                Player[A].Location.SpeedX = Player[A].Location.SpeedX + 0.1 * speedVar;
                            }
                            else
                            {
                                if(Player[A].Character == 2) // LUIGI
                                    Player[A].Location.SpeedX = Player[A].Location.SpeedX - 0.05 * 0.175;
                                if(Player[A].Character == 3) // PEACH
                                    Player[A].Location.SpeedX = Player[A].Location.SpeedX - 0.025 * 0.175;
                                if(Player[A].Character == 4) // toad
                                    Player[A].Location.SpeedX = Player[A].Location.SpeedX + 0.025 * 0.175;
                                if(Player[A].Character == 5) // Link
                                    Player[A].Location.SpeedX = Player[A].Location.SpeedX + 0.025 * speedVar;
                                else // Mario
                                    Player[A].Location.SpeedX = Player[A].Location.SpeedX + 0.05 * speedVar;
                            }
                            if(Player[A].Location.SpeedX < 0)
                            {
                                Player[A].Location.SpeedX = Player[A].Location.SpeedX + 0.18;
                                if(Player[A].Character == 2) // LUIGI
                                    Player[A].Location.SpeedX = Player[A].Location.SpeedX - 0.18 * 0.29;
                                if(Player[A].Character == 3) // PEACH
                                    Player[A].Location.SpeedX = Player[A].Location.SpeedX - 0.09 * 0.29;
                                if(Player[A].Character == 4) // toad
                                    Player[A].Location.SpeedX = Player[A].Location.SpeedX + 0.09 * 0.29;
                                if(SuperSpeed == true)
                                    Player[A].Location.SpeedX = Player[A].Location.SpeedX * 0.95;
                            }
                        }
                        if(SuperSpeed == true && Player[A].Controls.Run == true)
                            Player[A].Location.SpeedX = Player[A].Location.SpeedX + 0.1;
                    }
                    else
                    {
                        if(Player[A].Location.SpeedY == 0 || Player[A].StandingOnNPC != 0 || Player[A].Slope > 0 || Player[A].WetFrame == true) // Only lose speed when not in the air
                        {
                            if(Player[A].Location.SpeedX > 0)
                                Player[A].Location.SpeedX = Player[A].Location.SpeedX - 0.07 * speedVar;
                            if(Player[A].Location.SpeedX < 0)
                                Player[A].Location.SpeedX = Player[A].Location.SpeedX + 0.07 * speedVar;
                            if(Player[A].Character == 2) // LUIGI
                                Player[A].Location.SpeedX = Player[A].Location.SpeedX * 1.003;
                            if(Player[A].Character == 3) // PEACH
                                Player[A].Location.SpeedX = Player[A].Location.SpeedX * 1.0015;
                            if(Player[A].Character == 4) // toad
                                Player[A].Location.SpeedX = Player[A].Location.SpeedX * 0.9985;
                            if(SuperSpeed == true)
                                Player[A].Location.SpeedX = Player[A].Location.SpeedX * 0.95;
                        }
                        if(Player[A].Location.SpeedX > -0.18 && Player[A].Location.SpeedX < 0.18)
                        {
                            Player[A].Bumped = false;
                            Player[A].Location.SpeedX = 0;
                        }
                    }
                    if(Player[A].Location.SpeedX < -16)
                        Player[A].Location.SpeedX = -16;
                    else if(Player[A].Location.SpeedX > 16)
                        Player[A].Location.SpeedX = 16;
                    if(Player[A].Controls.Run == true || Player[A].Character == 5)
                    {
                        if(Player[A].Location.SpeedX >= Physics.PlayerRunSpeed * speedVar)
                        {
                            if(SuperSpeed == false)
                                Player[A].Location.SpeedX = Physics.PlayerRunSpeed * speedVar;
                        }
                        else if(Player[A].Location.SpeedX <= -Physics.PlayerRunSpeed * speedVar)
                        {
                            if(SuperSpeed == false)
                                Player[A].Location.SpeedX = -Physics.PlayerRunSpeed * speedVar;
                        }
                        else
                        {
                        }
                    }
                    else
                    {
                        if(Player[A].Location.SpeedX > Physics.PlayerWalkSpeed + 0.1 * speedVar)
                            Player[A].Location.SpeedX = Player[A].Location.SpeedX - 0.1;
                        else if(Player[A].Location.SpeedX < -Physics.PlayerWalkSpeed - 0.1 * speedVar)
                            Player[A].Location.SpeedX = Player[A].Location.SpeedX + 0.1;
                        else if(std::abs(Player[A].Location.SpeedX) > Physics.PlayerWalkSpeed * speedVar)
                        {
                            if(Player[A].Location.SpeedX > 0)
                                Player[A].Location.SpeedX = Physics.PlayerWalkSpeed * speedVar;
                            else
                                Player[A].Location.SpeedX = -Physics.PlayerWalkSpeed * speedVar;
                        }
                    }
                    if(Player[A].Mount == 1 && Player[A].MountType == 3)
                    {
                        Player[A].CanFly2 = true;
                        Player[A].FlyCount = 1000;
                    }
                    if(Player[A].Mount != 3)
                        Player[A].YoshiBlue = false;
                    if(FlyForever == true && Player[A].GroundPound == false)
                    {
                        if(Player[A].Mount == 3)
                            Player[A].YoshiBlue = true;
                        if((Player[A].State == 4 || Player[A].State == 5) || (Player[A].YoshiBlue == true && Player[A].Mount == 3) || (Player[A].Mount == 1 && Player[A].MountType == 3))
                            Player[A].CanFly2 = true;
                        else
                        {
                            Player[A].CanFly2 = false;
                            Player[A].CanFly = false;
                            Player[A].FlyCount = 0;
                            Player[A].YoshiBlue = false;
                        }
                    }
                    // Racoon/Tanooki Mario.  this handles the ability to fly after running
                    if((Player[A].State == 4 || Player[A].State == 5) && Player[A].Wet == 0)
                    {
                        if((Player[A].Location.SpeedY == 0 || Player[A].CanFly2 == true ||
                            Player[A].StandingOnNPC != 0 || Player[A].Slope > 0) &&
                                (std::abs(Player[A].Location.SpeedX) >= Physics.PlayerRunSpeed ||
                                 (Player[A].Character == 3 && std::abs(Player[A].Location.SpeedX) >= 5.58)))
                            Player[A].RunCount = Player[A].RunCount + 1;
                        else
                        {
                            if(!(std::abs(Player[A].Location.SpeedX) >= Physics.PlayerRunSpeed ||
                                 (Player[A].Character == 3 && std::abs(Player[A].Location.SpeedX) >= 5.58)))
                                Player[A].RunCount = Player[A].RunCount - 0.3;
                        }
                        if(Player[A].RunCount >= 35 && Player[A].Character == 1)
                        {
                            Player[A].CanFly = true;
                            Player[A].RunCount = 35;
                        }
                        else if(Player[A].RunCount >= 40 && Player[A].Character == 2)
                        {
                            Player[A].CanFly = true;
                            Player[A].RunCount = 40;
                        }
                        else if(Player[A].RunCount >= 80 && Player[A].Character == 3)
                        {
                            Player[A].CanFly = true;
                            Player[A].RunCount = 80;
                        }
                        else if(Player[A].RunCount >= 60 && Player[A].Character == 4)
                        {
                            Player[A].CanFly = true;
                            Player[A].RunCount = 60;
                        }
                        else if(Player[A].RunCount >= 10 && Player[A].Character == 5) // link flying
                        {
                            Player[A].CanFly = true;
                            Player[A].RunCount = 10;
                        }
                        else
                        {
                            Player[A].CanFly = false;
                            if(Player[A].RunCount < 0)
                                Player[A].RunCount = 0;
                        }
                    }
                    if(Player[A].Location.SpeedY == 0 || Player[A].StandingOnNPC != 0 || Player[A].Slope > 0)
                        Player[A].FlyCount = 1;
                    if(Player[A].FlyCount > 1)
                        Player[A].FlyCount = Player[A].FlyCount - 1;
                    else if(Player[A].FlyCount == 1)
                    {
                        Player[A].CanFly2 = false;
                        Player[A].FlyCount = 0;
                    }
                }


                // stop link when stabbing
                if(Player[A].Character == 5)
                {
                    if(Player[A].FireBallCD > 0 && (Player[A].Location.SpeedY == 0 || Player[A].Slope != 0 || Player[A].StandingOnNPC != 0))
                    {
                        if(Player[A].Slippy == true)
                            Player[A].Location.SpeedX = Player[A].Location.SpeedX * 0.75;
                        else
                            Player[A].Location.SpeedX = 0;
                    }
                }
                // fairy stuff
                if(Player[A].FairyTime != 0 && Player[A].Fairy == true)
                {
                    if(std::rand() % 10 > 9)
                    {
                        NewEffect(80,
                                  newLoc(Player[A].Location.X - 8 + std::fmod(std::rand(), Player[A].Location.Width + 16) - 4,
                                         Player[A].Location.Y - 8 + std::fmod(std::rand(), Player[A].Location.Height + 16)), 1, 0, ShadowMode);
                        Effect[numEffects].Location.SpeedX = std::fmod(std::rand(), 0.5) - 0.25;
                        Effect[numEffects].Location.SpeedY = std::fmod(std::rand(), 0.5) - 0.25;
                        Effect[numEffects].Frame = 1;
                    }
                    if(Player[A].FairyTime > 0)
                        Player[A].FairyTime = Player[A].FairyTime - 1;
                    if(Player[A].FairyTime != -1 && Player[A].FairyTime < 20 && Player[A].Character == 5)
                    {
                        for(B = 1; B <= numNPCs; B++)
                        {
                            if(NPC[B].Active == true)
                            {
                                if(NPC[B].Hidden == false)
                                {
                                    if(NPCIsAVine[NPC[B].Type])
                                    {
                                        tempLocation = NPC[B].Location;
                                        tempLocation.Width = tempLocation.Width + 32;
                                        tempLocation.Height = tempLocation.Height + 32;
                                        tempLocation.X = tempLocation.X - 16;
                                        tempLocation.Y = tempLocation.Y - 16;
                                        if(CheckCollision(tempLocation, Player[A].Location))
                                        {
                                            Player[A].FairyTime = 20;
                                            Player[A].FairyCD = 0;
                                        }
                                    }
                                }
                            }
                        }
                        for(B = 1; B <= numBackground; B++)
                        {
                            if(BackgroundFence[Background[B].Type])
                            {
                                if(Background[B].Hidden == false)
                                {
                                    tempLocation = Background[B].Location;
                                    tempLocation.Width = tempLocation.Width + 32;
                                    tempLocation.Height = tempLocation.Height + 32;
                                    tempLocation.X = tempLocation.X - 16;
                                    tempLocation.Y = tempLocation.Y - 16;
                                    if(CheckCollision(tempLocation, Player[A].Location))
                                    {
                                        Player[A].FairyTime = 20;
                                        Player[A].FairyCD = 0;
                                    }
                                }
                            }
                        }

                    }
                }
                else if(Player[A].Fairy == true)
                {
                    PlaySound(87);
                    Player[A].Immune = 10;
                    Player[A].Effect = 8;
                    Player[A].Effect2 = 4;
                    Player[A].Fairy = false;
                    SizeCheck(A);
                    NewEffect(63, Player[A].Location);
                    PlayerPush(A, 3);
                }
                else
                    Player[A].FairyTime = 0;
                if(Player[A].FairyCD != 0 && (Player[A].Location.SpeedY == 0 || Player[A].Slope != 0 || Player[A].StandingOnNPC != 0 || Player[A].WetFrame == true))
                    Player[A].FairyCD = Player[A].FairyCD - 1;


                if(Player[A].StandingOnNPC != 0 && NPC[Player[A].StandingOnNPC].Pinched == false && FreezeNPCs == false)
                {
                    if(Player[A].StandingOnNPC < 0)
                        NPC[Player[A].StandingOnNPC].Location = Block[NPC[Player[A].StandingOnNPC].Special].Location;
                    Player[A].Location.SpeedX = Player[A].Location.SpeedX + NPC[Player[A].StandingOnNPC].Location.SpeedX + NPC[Player[A].StandingOnNPC].BeltSpeed;
                }

                if(GameOutro == true) // force the player to walk a specific speed during the credits
                {
                    if(Player[A].Location.SpeedX < -2)
                        Player[A].Location.SpeedX = -2;
                    if(Player[A].Location.SpeedX > 2)
                        Player[A].Location.SpeedX = 2;
                }



                // slippy code
                if(Player[A].Slippy == true && (Player[A].Slide == false || Player[A].Slope == 0))
                {
                    if(Player[A].Slope > 0)
                    {
                        Player[A].Location.SpeedX = (Player[A].Location.SpeedX + SlippySpeedX * 4) / 5;
                        if(Player[A].Location.SpeedX > -0.01 && Player[A].Location.SpeedX < 0.01)
                            Player[A].Location.SpeedX = 0;
                    }
                    else
                    {
                        Player[A].Location.SpeedX = (Player[A].Location.SpeedX + SlippySpeedX * 3) / 4;
                        if(Player[A].Location.SpeedX > -0.01 && Player[A].Location.SpeedX < 0.01)
                            Player[A].Location.SpeedX = 0;
                    }
                }
                wasSlippy = Player[A].Slippy;
                Player[A].Slippy = false;

                if(Player[A].Quicksand > 1)
                {
                    Player[A].Slide = false;
                    if(Player[A].Location.SpeedY >= 0)
                        Player[A].Location.SpeedX = Player[A].Location.SpeedX * 0.5;
                }


                Player[A].Location.X = Player[A].Location.X + Player[A].Location.SpeedX; // This is where the actual movement happens

                // Players Y movement.
                if(Block[Player[A].Slope].Location.SpeedY != 0 && Player[A].Slope != 0)
                    Player[A].Location.Y = Player[A].Location.Y + Block[Player[A].Slope].Location.SpeedY;

                if(Player[A].Fairy == true) // the player is a fairy
                {
                    Player[A].WetFrame = false;
                    Player[A].Wet = 0;
                    if(Player[A].FairyCD == 0)
                    {
                        if(Player[A].Controls.Jump == true || Player[A].Controls.AltJump == true || Player[A].Controls.Up == true)
                        {
                            Player[A].Location.SpeedY = Player[A].Location.SpeedY - 0.15;
                            if(Player[A].Location.SpeedY > 0)
                                Player[A].Location.SpeedY = Player[A].Location.SpeedY - 0.1;
                        }
                        else if(Player[A].Location.SpeedY < -0.1 || Player[A].Controls.Down == true)
                        {
                            if(Player[A].Location.SpeedY < 3)
                                Player[A].Location.SpeedY = Player[A].Location.SpeedY + Physics.PlayerGravity * 0.05;
                            if(Player[A].Location.SpeedY < 0)
                                Player[A].Location.SpeedY = Player[A].Location.SpeedY + Physics.PlayerGravity * 0.05;
                            Player[A].Location.SpeedY = Player[A].Location.SpeedY + Physics.PlayerGravity * 0.1;
                            if(Player[A].Controls.Down == true)
                                Player[A].Location.SpeedY = Player[A].Location.SpeedY + 0.05;
                        }
                        else if(Player[A].Location.SpeedY > 0.1)
                            Player[A].Location.SpeedY = Player[A].Location.SpeedY - 0.15;
                        else
                            Player[A].Location.SpeedY = 0;
                    }
                    else
                    {
                        if(Player[A].Controls.Jump == true || Player[A].Controls.AltJump == true || Player[A].Controls.Up == true)
                        {
                            Player[A].Location.SpeedY = Player[A].Location.SpeedY - 0.15;
                            if(Player[A].Location.SpeedY > 0)
                                Player[A].Location.SpeedY = Player[A].Location.SpeedY - 0.1;
                        }
                        else
                        {
                            if(Player[A].Location.SpeedY < 3)
                                Player[A].Location.SpeedY = Player[A].Location.SpeedY + Physics.PlayerGravity * 0.05;
                            if(Player[A].Location.SpeedY < 0)
                                Player[A].Location.SpeedY = Player[A].Location.SpeedY + Physics.PlayerGravity * 0.05;
                            Player[A].Location.SpeedY = Player[A].Location.SpeedY + Physics.PlayerGravity * 0.1;
                            if(Player[A].Controls.Down == true)
                                Player[A].Location.SpeedY = Player[A].Location.SpeedY + 0.05;
                        }
                    }

                    if(Player[A].Location.SpeedY > 4)
                        Player[A].Location.SpeedY = 4;
                    else if(Player[A].Location.SpeedY < -3)
                        Player[A].Location.SpeedY = -3;
                    Player[A].Location.Y = Player[A].Location.Y + Player[A].Location.SpeedY;
                }
                else if(Player[A].Wet > 0 && Player[A].Quicksand == 0) // the player is swimming
                {
                    if(Player[A].Mount == 1)
                    {
                        if(Player[A].Controls.AltJump == true && Player[A].CanAltJump == true)
                        {
                            Player[A].CanJump = false;
                            PlaySound(1); // Jump sound
                            PlaySound(35);
                            Player[A].Location.SpeedY = Physics.PlayerJumpVelocity - tempSpeed;
                            Player[A].Jump = Physics.PlayerJumpHeight;
                            if(Player[A].Character == 2)
                                Player[A].Jump = Player[A].Jump + 3;
                            if(Player[A].SpinJump == true)
                                Player[A].Jump = Player[A].Jump - 6;
                            Player[A].Mount = 0;
                            Player[A].StandingOnNPC = 0;
                            numNPCs = numNPCs + 1;
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
                            Player[A].Location.Y = Player[A].Location.Y + Player[A].Location.Height;
                            Player[A].Location.Height = Physics.PlayerHeight[Player[A].Character][Player[A].State];
                            Player[A].Location.Y = Player[A].Location.Y - Player[A].Location.Height;
                        }
                    }
                    else if(Player[A].Mount == 3)
                    {
                        if(Player[A].Controls.AltJump == true && Player[A].CanAltJump == true)
                        {
                            UnDuck(A);
                            if(Player[A].YoshiNPC > 0 || Player[A].YoshiPlayer > 0)
                                YoshiSpit(A);
                            Player[A].CanJump = false;
                            Player[A].StandingOnNPC = 0;
                            Player[A].Mount = 0;
                            numNPCs = numNPCs + 1;
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
                            // If ShadowMode = True Then .Shadow = True
                            Player[A].Location.Height = Physics.PlayerHeight[Player[A].Character][Player[A].State];
                        }
                    }

                    if(Player[A].Duck == true)
                    {
                        if(Player[A].StandingOnNPC == 0 && Player[A].Slope == 0 && Player[A].Location.SpeedY != 0 && Player[A].Mount != 1)
                        {
                            if(Player[A].Character <= 2) // unduck wet players that aren't peach o toad
                                UnDuck(A);
                        }
                    }

                    Player[A].Location.SpeedY = Player[A].Location.SpeedY + Physics.PlayerGravity * 0.1;
                    if(Player[A].Location.SpeedY >= 3) // Terminal Velocity in water
                        Player[A].Location.SpeedY = 3;
                    if(Player[A].Mount == 1)
                    {
                        if(Player[A].Controls.Left == true || Player[A].Controls.Right == true)
                        {
                            if(Player[A].Location.SpeedY == Physics.PlayerGravity * 0.1 || Player[A].Slope > 0 || (Player[A].StandingOnNPC != 0 && Player[A].Location.Y + Player[A].Location.Height >= NPC[Player[A].StandingOnNPC].Location.Y - NPC[Player[A].StandingOnNPC].Location.SpeedY))
                            {
                                if(Player[A].Controls.Left == true && Player[A].Location.SpeedX - NPC[Player[A].StandingOnNPC].Location.SpeedX - NPC[Player[A].StandingOnNPC].BeltSpeed <= 0)
                                    Player[A].Location.SpeedY = -1.1 + NPC[Player[A].StandingOnNPC].Location.SpeedY;
                                else if(Player[A].Controls.Right == true && Player[A].Location.SpeedX - NPC[Player[A].StandingOnNPC].Location.SpeedX - NPC[Player[A].StandingOnNPC].BeltSpeed >= 0)
                                    Player[A].Location.SpeedY = -1.1 + NPC[Player[A].StandingOnNPC].Location.SpeedY;
                                else
                                    PlaySound(10);
                                Player[A].MountSpecial = 1;
                            }
                        }

                        if(Player[A].Location.SpeedY < -1.1)
                            Player[A].MountSpecial = 0;
                        else if(Player[A].Location.SpeedY > 1.1)
                            Player[A].MountSpecial = 0;
                        else if(Player[A].FloatTime >= 0)
                            Player[A].MountSpecial = 0;


                        if(Player[A].Controls.Jump == true && Player[A].MountSpecial == 1 && Player[A].CanJump == true)
                        {
                            Player[A].Location.SpeedY = Physics.PlayerGravity * 0.1;
                            Player[A].MountSpecial = 0;
                            Player[A].StandUp = true;
                        }
                    }

                    if(Player[A].SwimCount > 0)
                        Player[A].SwimCount = Player[A].SwimCount - 1;
                    if(Player[A].SwimCount == 0)
                    {
                        if(Player[A].Mount != 1 || Player[A].Location.SpeedY == Physics.PlayerGravity * 0.1 || Player[A].Slope != 0 || Player[A].StandingOnNPC != 0)
                        {
                            if((Player[A].Controls.Jump == true && Player[A].CanJump == true) ||
                               (Player[A].Controls.AltJump == true && Player[A].CanAltJump == true))
                            {
                                if(Player[A].Duck == true && Player[A].Mount != 1 && Player[A].Character <= 2)
                                    UnDuck(A);
                                if(Player[A].Slope != 0)
                                    Player[A].Location.SpeedY = 0;
                                Player[A].Vine = 0;
                                if(Player[A].StandingOnNPC != 0)
                                {
                                    Player[A].Location.SpeedY = NPC[Player[A].StandingOnNPC].Location.SpeedY;
                                    Player[A].StandingOnNPC = 0;
                                }
                                Player[A].SwimCount = 15;
                                // If .Location.SpeedY = 0 Then .Location.Y = .Location.Y - 1
                                if(Player[A].Controls.Down == true)
                                {
                                    if(Player[A].Location.SpeedY >= Physics.PlayerJumpVelocity * 0.2)
                                    {
                                        Player[A].Location.SpeedY = Player[A].Location.SpeedY + Physics.PlayerJumpVelocity * 0.2;
                                        if(Player[A].Location.SpeedY < Physics.PlayerJumpVelocity * 0.2)
                                            Player[A].Location.SpeedY = Physics.PlayerJumpVelocity * 0.2;
                                    }
                                }
                                else
                                {
                                    if(Player[A].Controls.Up == true)
                                        Player[A].Location.SpeedY = Player[A].Location.SpeedY + Physics.PlayerJumpVelocity * 0.5;
                                    else
                                        Player[A].Location.SpeedY = Player[A].Location.SpeedY + Physics.PlayerJumpVelocity * 0.4;
                                    if(Player[A].Mount == 1)
                                        Player[A].Location.SpeedY = Physics.PlayerJumpVelocity;
                                }
                                if(Player[A].Location.SpeedY > 0)
                                    Player[A].Location.SpeedY = Physics.PlayerJumpVelocity * 0.2;
                                PlaySound(72);
                            }
                        }
                    }

                    if(Player[A].Controls.Jump == true)
                        Player[A].CanJump = false;
                    else
                        Player[A].CanJump = true;
                    if(Player[A].Controls.AltJump == true)
                        Player[A].CanAltJump = false;
                    else
                        Player[A].CanAltJump = true;
                    if(Player[A].Controls.Up == true)
                    {
                        if(Player[A].Location.SpeedY < -4)
                            Player[A].Location.SpeedY = -4;
                    }
                    else
                    {
                        if(Player[A].Location.SpeedY < -3)
                            Player[A].Location.SpeedY = -3;
                    }

                    Player[A].Location.Y = Player[A].Location.Y + Player[A].Location.SpeedY;

                }
                else // the player is not swimming
                {

                    if(Player[A].Mount != 2) // if not in the clown car
                    {

                        if(Player[A].Mount == 1) // this gives the player the bounce when in the kurbio's shoe
                        {
                            if(Player[A].Controls.Left == true || Player[A].Controls.Right == true)
                            {
                                if(Player[A].Location.SpeedY == 0 || Player[A].Slope > 0 || (Player[A].StandingOnNPC != 0 && Player[A].Location.Y + Player[A].Location.Height >= NPC[Player[A].StandingOnNPC].Location.Y - NPC[Player[A].StandingOnNPC].Location.SpeedY))
                                {
                                    if(Player[A].Controls.Left == true && Player[A].Location.SpeedX - NPC[Player[A].StandingOnNPC].Location.SpeedX - NPC[Player[A].StandingOnNPC].BeltSpeed <= 0)
                                        Player[A].Location.SpeedY = -4.1 + NPC[Player[A].StandingOnNPC].Location.SpeedY;
                                    else if(Player[A].Controls.Right == true && Player[A].Location.SpeedX - NPC[Player[A].StandingOnNPC].Location.SpeedX - NPC[Player[A].StandingOnNPC].BeltSpeed >= 0)
                                        Player[A].Location.SpeedY = -4.1 + NPC[Player[A].StandingOnNPC].Location.SpeedY;
                                    else
                                        PlaySound(10);
                                    Player[A].MountSpecial = 1;
                                }
                            }
                            if(Player[A].Location.SpeedY < -4.1)
                                Player[A].MountSpecial = 0;
                            else if(Player[A].Location.SpeedY > 4.1)
                                Player[A].MountSpecial = 0;
                            if(Player[A].Controls.Jump == true && Player[A].MountSpecial == 1 && Player[A].CanJump == true)
                            {
                                Player[A].Location.SpeedY = 0;
                                Player[A].StandUp = true;
                            }
                        }

                        if(Player[A].Mount == 1)
                        {
                            if(Player[A].Controls.AltJump == true && Player[A].CanAltJump == true) // check to see if the player should jump out of the shoe
                            {
                                UnDuck(A);
                                Player[A].CanJump = false;
                                PlaySound(1); // Jump sound
                                PlaySound(35);
                                Player[A].Location.SpeedY = Physics.PlayerJumpVelocity - tempSpeed;
                                Player[A].Jump = Physics.PlayerJumpHeight;
                                if(Player[A].Character == 2)
                                    Player[A].Jump = Player[A].Jump + 3;
                                if(Player[A].SpinJump == true)
                                    Player[A].Jump = Player[A].Jump - 6;
                                Player[A].Mount = 0;
                                Player[A].StandingOnNPC = 0;
                                numNPCs = numNPCs + 1;
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
                                Player[A].Location.Y = Player[A].Location.Y + Player[A].Location.Height;
                                Player[A].Location.Height = Physics.PlayerHeight[Player[A].Character][Player[A].State];
                                Player[A].Location.Y = Player[A].Location.Y - Player[A].Location.Height;
                            }
                        }
                        else if(Player[A].Mount == 3)
                        {
                            if(Player[A].Controls.AltJump == true && Player[A].CanAltJump == true) // jump off of yoshi
                            {
                                UnDuck(A);
                                if(Player[A].YoshiNPC > 0 || Player[A].YoshiPlayer > 0)
                                    YoshiSpit(A);
                                Player[A].CanJump = false;
                                Player[A].StandingOnNPC = 0;
                                PlaySound(1); // Jump sound
                                Player[A].Location.SpeedY = Physics.PlayerJumpVelocity - tempSpeed;
                                Player[A].Jump = Physics.PlayerJumpHeight;
                                if(Player[A].Character == 2)
                                    Player[A].Jump = Player[A].Jump + 3;
                                if(Player[A].SpinJump == true)
                                    Player[A].Jump = Player[A].Jump - 6;
                                Player[A].Mount = 0;
                                numNPCs = numNPCs + 1;
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
                                Player[A].Location.Height = Physics.PlayerHeight[Player[A].Character][Player[A].State];
                            }
                        }

                        if((Player[A].Location.SpeedY == 0 || Player[A].Jump > 0 || Player[A].Vine > 0) && Player[A].FloatTime == 0) // princess float
                            Player[A].CanFloat = true;
                        if(Player[A].Wet > 0 || Player[A].WetFrame == true)
                            Player[A].CanFloat = false;
                        // handles the regular jump
                        if(Player[A].Controls.Jump == true || (Player[A].Controls.AltJump == true && ((Player[A].Character > 2 && Player[A].Character != 4) || Player[A].Quicksand > 0) && Player[A].CanAltJump == true))
                        {
                            if(Player[A].Location.SpeedX > 0)
                                tempSpeed = Player[A].Location.SpeedX * 0.2; // tempSpeed gives the player a height boost when jumping while running, based off his SpeedX
                            else
                                tempSpeed = -Player[A].Location.SpeedX * 0.2;
                            if(Player[A].ShellSurf == true) // this code modifies the jump based on him riding a shell
                            {
                                if(NPC[Player[A].StandingOnNPC].Location.SpeedY == 0 || NPC[Player[A].StandingOnNPC].Slope > 0)
                                {
                                    if(Player[A].CanJump == true)
                                    {
                                        PlaySound(33); // Jump sound
                                        Player[A].Jump = Physics.PlayerJumpHeight * 0.6;
                                        NPC[Player[A].StandingOnNPC].Location.SpeedY = Physics.PlayerJumpVelocity * 0.9;
                                    }
                                }
                                else if(Player[A].Jump > 0)
                                    NPC[Player[A].StandingOnNPC].Location.SpeedY = Physics.PlayerJumpVelocity * 0.9;

                                // if not surfing a shell then proceed like normal
                            }
                            else
                            {
                                if((Player[A].Vine > 0 || Player[A].Location.SpeedY == 0 || Player[A].StandingOnNPC != 0 || MultiHop == true || Player[A].Slope > 0 || (Player[A].Location.SpeedY > 0 && Player[A].Quicksand > 0)) && Player[A].CanJump == true)
                                {
                                    PlaySound(1); // Jump sound
                                    Player[A].Location.SpeedY = Physics.PlayerJumpVelocity - tempSpeed;
                                    Player[A].Jump = Physics.PlayerJumpHeight;
                                    if(Player[A].Character == 4 && (Player[A].State == 4 || Player[A].State == 5) && Player[A].SpinJump == false)
                                        Player[A].DoubleJump = true;
                                    if(Player[A].Character == 2)
                                        Player[A].Jump = Player[A].Jump + 3;
                                    if(Player[A].SpinJump == true)
                                        Player[A].Jump = Player[A].Jump - 6;
                                    if(Player[A].StandingOnNPC > 0 && FreezeNPCs == false)
                                    {
                                        if(NPC[Player[A].StandingOnNPC].Type != 91)
                                            Player[A].Location.SpeedX = Player[A].Location.SpeedX - NPC[Player[A].StandingOnNPC].Location.SpeedX;
                                    }
                                    Player[A].StandingOnNPC = 0; // the player can't stand on an NPC after jumping
                                    if(Player[A].CanFly == true) // let's the player fly if the conditions are met
                                    {
                                        Player[A].StandingOnNPC = 0;
                                        Player[A].Jump = 30;
                                        if(Player[A].Character == 2)
                                            Player[A].Jump = Player[A].Jump + 3;
                                        if(Player[A].SpinJump == true)
                                            Player[A].Jump = Player[A].Jump - 6;
                                        Player[A].CanFly = false;
                                        Player[A].RunCount = 0;
                                        Player[A].CanFly2 = true;
                                        if(Player[A].Character == 2) // luigi doesn't fly as long as mario
                                            Player[A].FlyCount = 300; // Length of flight time
                                        else if(Player[A].Character == 3) // special handling for peach
                                        {
                                            Player[A].FlyCount = 0;
                                            Player[A].RunCount = 80;
                                            Player[A].CanFly2 = false;
                                            Player[A].Jump = 70;
                                            Player[A].CanFloat = true;
                                            Player[A].FlySparks = true;
                                        }
                                        else if(Player[A].Character == 3) // special handling for peach
                                            Player[A].FlyCount = 280; // Length of flight time
                                        else
                                            Player[A].FlyCount = 320; // Length of flight time
                                    }
                                }
                                else if(Player[A].Jump > 0) // controls the height of the jump
                                {
                                    Player[A].Location.SpeedY = Physics.PlayerJumpVelocity - tempSpeed;
                                    if(Player[A].Jump > 20)
                                    {
                                        if(Player[A].Jump > 40)
                                            Player[A].Location.SpeedY = Player[A].Location.SpeedY - (40 - 20) * 0.2;
                                        else
                                            Player[A].Location.SpeedY = Player[A].Location.SpeedY - (Player[A].Jump - 20) * 0.2;
                                    }
                                }
                                else if(Player[A].CanFly2 == true)
                                {
                                    if(Player[A].Location.SpeedY > Physics.PlayerJumpVelocity * 0.5)
                                    {
                                        Player[A].Location.SpeedY = Player[A].Location.SpeedY - 1;
                                        Player[A].CanPound = true;
                                        if(Player[A].YoshiBlue || (Player[A].Mount == 1 && Player[A].MountType == 3))
                                            PlaySound(50);
                                    }
                                }
                            }
                            Player[A].CanJump = false;
                        }
                        else
                            Player[A].CanJump = true;
                        if(Player[A].Jump > 0)
                            Player[A].Slope = 0;
                        if(Player[A].SpinJump == true || (Player[A].State != 4 && Player[A].State != 5) || Player[A].StandingOnNPC > 0 || Player[A].Slope > 0 || Player[A].Location.SpeedY == 0)
                            Player[A].DoubleJump = false;
                        // double jump code
                        if(Player[A].DoubleJump == true && Player[A].Jump == 0 && Player[A].Location.SpeedY != 0 && Player[A].Slope == 0 && Player[A].StandingOnNPC == 0 && Player[A].Wet == 0 && Player[A].Vine == 0 && Player[A].WetFrame == false && Player[A].Fairy == false && Player[A].CanFly2 == false)
                        {
                            if(Player[A].Controls.Jump == true && Player[A].JumpRelease == true)
                            {
                                PlaySound(1);
                                Player[A].Location.SpeedY = Physics.PlayerJumpVelocity;
                                Player[A].Jump = 10;
                                Player[A].DoubleJump = false;
                                tempLocation = Player[A].Location;
                                tempLocation.Y = Player[A].Location.Y + Player[A].Location.Height - EffectHeight[80] / 2.0 + Player[A].Location.SpeedY;
                                tempLocation.Height = EffectHeight[80];
                                tempLocation.Width = EffectWidth[80];
                                tempLocation.X = Player[A].Location.X;
                                for(B = 1; B <= 10; B++)
                                {
                                    NewEffect(80, tempLocation);
                                    Effect[numEffects].Location.SpeedX = std::rand() % 3 - 1.5;
                                    Effect[numEffects].Location.SpeedY = std::fmod(std::rand(), 0.5) + (1.5 - std::abs(Effect[numEffects].Location.SpeedX)) * 0.5;
                                    Effect[numEffects].Location.SpeedX = Effect[numEffects].Location.SpeedX - Player[A].Location.SpeedX * 0.2;
                                }
                            }
                        }



                        if(Player[A].NoShellKick > 0) // countdown for the next time the player kicks a turtle shell
                            Player[A].NoShellKick = Player[A].NoShellKick - 1;

                        if(Player[A].ShellSurf == true)
                        {
                            if(Player[A].Mount != 0)
                                Player[A].ShellSurf = false;
                            if(Player[A].Direction != NPC[Player[A].StandingOnNPC].Direction && 1 == 2)
                            {
                                Player[A].ShellSurf = false;
                                Player[A].Location.SpeedY = NPC[Player[A].StandingOnNPC].Location.SpeedY;
                                if(Player[A].Location.SpeedY > 0)
                                    Player[A].Location.SpeedY = 0;
                                PlaySound(10);
                                NPC[Player[A].StandingOnNPC].CantHurt = 30;
                                NPC[Player[A].StandingOnNPC].CantHurtPlayer = A;
                                Player[A].Location.SpeedX = NPC[Player[A].StandingOnNPC].Location.SpeedX / 2;
                                Player[A].StandingOnNPC = 0;
                                Player[A].NoShellKick = 30;
                            }
                            else
                            {
                                if(std::rand() % 10 > 3)
                                {
                                    tempLocation.Y = Player[A].Location.Y + Player[A].Location.Height - 2 + std::fmod(std::rand(), NPC[Player[A].StandingOnNPC].Location.Height - 8) + 4;
                                    tempLocation.X = Player[A].Location.X - 4 + std::fmod(std::rand(), Player[A].Location.Width - 8) + 4 - 8 * Player[A].Direction;
                                    NewEffect(80, tempLocation, 1, 0, ShadowMode);
                                    Effect[numEffects].Frame = std::rand() % 3;
                                    Effect[numEffects].Location.SpeedY = (Player[A].Location.Y + Player[A].Location.Height + NPC[Player[A].StandingOnNPC].Location.Height / 32.0 - tempLocation.Y + 12) * 0.05;
                                }
                            }
                            if(NPC[Player[A].StandingOnNPC].Wet == 2)
                            {
                                if(NPC[Player[A].StandingOnNPC].Type == 195)
                                    NPC[Player[A].StandingOnNPC].Special4 = 1;
                                NPC[Player[A].StandingOnNPC].Location.SpeedY = NPC[Player[A].StandingOnNPC].Location.SpeedY - Physics.NPCGravity * 1.5;
                            }
                        }

                        // START ALT JUMP - this code does the player's spin jump
                        if(Player[A].Controls.AltJump == true && (Player[A].Character == 1 || Player[A].Character == 2 || Player[A].Character == 4))
                        {
                            if(Player[A].Location.SpeedX > 0)
                                tempSpeed = Player[A].Location.SpeedX * 0.2;
                            else
                                tempSpeed = -Player[A].Location.SpeedX * 0.2;
                            if((Player[A].Vine > 0 || Player[A].Location.SpeedY == 0 || Player[A].StandingOnNPC != 0 || Player[A].Slope > 0 || MultiHop == true) && Player[A].CanAltJump == true) // Player Jumped
                            {
                                if(Player[A].Duck == false)
                                {
                                    Player[A].Slope = 0;
                                    Player[A].SpinFireDir = Player[A].Direction;
                                    Player[A].Location.SpeedY = Physics.PlayerJumpVelocity - tempSpeed;
                                    Player[A].Jump = Physics.PlayerJumpHeight;
                                    if(Player[A].Character == 2)
                                        Player[A].Jump = Player[A].Jump + 3;

                                    if(Player[A].StandingOnNPC > 0 && FreezeNPCs == false)
                                    {
                                        if(NPC[Player[A].StandingOnNPC].Type != 91)
                                            Player[A].Location.SpeedX = Player[A].Location.SpeedX - NPC[Player[A].StandingOnNPC].Location.SpeedX;
                                    }

                                    PlaySound(33); // Jump sound
                                    Player[A].Jump = Player[A].Jump - 6;
                                    if(Player[A].Direction == 1)
                                        Player[A].SpinFrame = 0;
                                    else
                                        Player[A].SpinFrame = 6;
                                    Player[A].SpinJump = true;
//                                    if(nPlay.Online == true && nPlay.MySlot + 1 == A)
//                                        Netplay::sendData Netplay::PutPlayerLoc(nPlay.MySlot) + "1l" + std::to_string(A) + LB;
                                    if(Player[A].Duck == true)
                                        UnDuck(A);

                                    if(Player[A].ShellSurf == true)
                                    {
                                        Player[A].ShellSurf = false;
                                        Player[A].Location.SpeedX = NPC[Player[A].StandingOnNPC].Location.SpeedX + NPC[Player[A].StandingOnNPC].BeltSpeed * 0.8;
                                        Player[A].Jump = 0;
                                    }

                                    Player[A].StandingOnNPC = 0;

                                    if(Player[A].CanFly == true)
                                    {
                                        Player[A].StandingOnNPC = 0;
                                        Player[A].Jump = 30;
                                        if(Player[A].Character == 2)
                                            Player[A].Jump = Player[A].Jump + 3;
                                        if(Player[A].SpinJump == true)
                                            Player[A].Jump = Player[A].Jump - 6;
                                        Player[A].CanFly = false;
                                        Player[A].RunCount = 0;
                                        Player[A].CanFly2 = true;
                                        Player[A].FlyCount = 150; // Length of flight time
                                    }
                                }
                            }
                            else if(Player[A].Jump > 0)
                            {
                                Player[A].Location.SpeedY = Physics.PlayerJumpVelocity - tempSpeed;
                                if(Player[A].Jump > 20)
                                    Player[A].Location.SpeedY = Player[A].Location.SpeedY - (Player[A].Jump - 20) * 0.2;
                            }
                            else if(Player[A].CanFly2 == true)
                            {
                                if(Player[A].Location.SpeedY > Physics.PlayerJumpVelocity * 0.5)
                                {
                                    Player[A].Location.SpeedY = Player[A].Location.SpeedY - 1;
                                    Player[A].CanPound = true;
                                    if(Player[A].YoshiBlue == true)
                                        PlaySound(50);
                                }
                            }
                            // End If
                            Player[A].CanAltJump = false;

                        }
                        else
                            Player[A].CanAltJump = true;
                        // END ALT JUMP


                        if((Player[A].Location.SpeedY == 0 || Player[A].StandingOnNPC != 0 || Player[A].Slope > 0) && Player[A].SpinJump == true)
                        {
                            Player[A].SpinJump = false;
//                            if(nPlay.Online == true && nPlay.MySlot + 1 == A)
//                                Netplay::sendData Netplay::PutPlayerLoc(nPlay.MySlot) + "1m" + std::to_string(A) + LB;
                            Player[A].TailCount = 0;
                        }
                        if(Player[A].Mount > 0)
                            Player[A].SpinJump = false;
                        if(Player[A].Controls.AltJump == false && Player[A].Controls.Jump == false)
                            Player[A].Jump = 0;
                        if(Player[A].Jump > 0)
                            Player[A].Jump = Player[A].Jump - 1;

                        if(Player[A].Jump > 0)
                            Player[A].Vine = 0;


                        if(Player[A].Quicksand > 1)
                        {
                            Player[A].Slide = false;
                            if(Player[A].Location.SpeedY < -0.7)
                            {
                                Player[A].Location.SpeedY = -0.7;
                                Player[A].Jump = Player[A].Jump - 1;
                            }
                            else if(Player[A].Location.SpeedY < 0)
                            {
                                Player[A].Location.SpeedY = Player[A].Location.SpeedY + 0.1;
                                Player[A].Jump = 0;
                            }
                            if(Player[A].Location.SpeedY >= 0.1)
                                Player[A].Location.SpeedY = 0.1;
                            Player[A].Location.Y = Player[A].Location.Y + Player[A].Location.SpeedY;
                        }


                        // gravity
                        if(Player[A].Vine == 0)
                        {
                            if(Player[A].NoGravity == 0)
                            {
                                if(Player[A].Character == 2)
                                    Player[A].Location.SpeedY = Player[A].Location.SpeedY + Physics.PlayerGravity * 0.9;
                                else
                                    Player[A].Location.SpeedY = Player[A].Location.SpeedY + Physics.PlayerGravity;
                                if(Player[A].HoldingNPC > 0)
                                {
                                    if(NPC[Player[A].HoldingNPC].Type == 278 || NPC[Player[A].HoldingNPC].Type == 279)
                                    {
                                        if(Player[A].Controls.Jump == true || Player[A].Controls.AltJump)
                                        {
                                            if(Player[A].Character == 2)
                                                Player[A].Location.SpeedY = Player[A].Location.SpeedY - Physics.PlayerGravity * 0.9 * 0.8;
                                            else
                                                Player[A].Location.SpeedY = Player[A].Location.SpeedY - Physics.PlayerGravity * 0.8;
                                            if(Player[A].Location.SpeedY > Physics.PlayerGravity * 3)
                                                Player[A].Location.SpeedY = Physics.PlayerGravity * 3;
                                        }
                                        else
                                            NPC[Player[A].HoldingNPC].Special = 0;
                                    }
                                }
                                if(Player[A].Location.SpeedY > Physics.PlayerTerminalVelocity)
                                    Player[A].Location.SpeedY = Physics.PlayerTerminalVelocity;
                            }
                            else
                                Player[A].NoGravity = Player[A].NoGravity - 1;
                        }

                        // princess float


                        if(Player[A].Character == 3 && Player[A].Wet == 0 && Player[A].WetFrame == false)
                        {
                            if(Player[A].Location.SpeedY == 0 || Player[A].StandingOnNPC > 0 || Player[A].Slope > 0 || Player[A].CanFly2 == true)
                                Player[A].CanFloat = true;
                            else if(Player[A].CanFloat == true)
                            {
                                if(Player[A].Jump == 0 && ((Player[A].Controls.Jump == true && Player[A].FloatRelease == true) || (Player[A].Controls.AltJump == true && Player[A].Location.SpeedY > 0)))
                                {
                                    if(Player[A].State == 4 || Player[A].State == 5)
                                    {
                                        Player[A].FloatTime = 100;
                                        Player[A].FlySparks = true;
                                    }
                                    else
                                        Player[A].FloatTime = 65;
                                    Player[A].FloatDir = 1;
                                    if(Player[A].Location.SpeedY < -0.5)
                                        Player[A].FloatSpeed = 0.5;
                                    else if(Player[A].Location.SpeedY > 0.5)
                                        Player[A].FloatSpeed = 0.5;
                                    else
                                        Player[A].FloatSpeed = Player[A].Location.SpeedY;
                                    Player[A].CanFloat = false;
                                }
                            }
                        }
                        if(Player[A].Character == 3 && Player[A].FlySparks == true)
                        {
                            if(Player[A].FloatTime == 0 && Player[A].Location.SpeedY >= 0)
                                Player[A].FlySparks = false;
                        }
                        if(Player[A].CanFloat == true)
                            Player[A].FloatTime = 0;
                        if(Player[A].FloatTime > 0 && Player[A].Character == 3)
                        {
                            if((Player[A].Controls.Jump == true || Player[A].Controls.AltJump == true) && Player[A].Vine == 0)
                            {
                                Player[A].FloatTime = Player[A].FloatTime - 1;
                                Player[A].FloatSpeed = Player[A].FloatSpeed + Player[A].FloatDir * 0.1;
                                if(Player[A].FloatSpeed > 0.8)
                                    Player[A].FloatDir = -1;
                                if(Player[A].FloatSpeed < -0.8)
                                    Player[A].FloatDir = 1;
                                Player[A].Location.SpeedY = Player[A].FloatSpeed;
                                if(Player[A].FloatTime == 0 && Player[A].Location.SpeedY == 0)
                                    Player[A].Location.SpeedY = 0.1;
                            }
                            else
                                Player[A].FloatTime = 0;
                        }


                        // Racoon Mario
                        if((Player[A].State == 4 || Player[A].State == 5) || Player[A].YoshiBlue == true || (Player[A].Mount == 1 && Player[A].MountType == 3))
                        {
                            if((Player[A].Controls.Jump == true || Player[A].Controls.AltJump == true) && ((Player[A].Location.SpeedY > Physics.PlayerGravity * 5 && Player[A].Character != 3 && Player[A].Character != 4) || (Player[A].Location.SpeedY > Physics.PlayerGravity * 10 && Player[A].Character == 3) || (Player[A].Location.SpeedY > Physics.PlayerGravity * 7.5 && Player[A].Character == 4)) && Player[A].GroundPound == false && Player[A].Slope == 0 && Player[A].Character != 5)
                            {
                                if(Player[A].ShellSurf == false)
                                {
                                    if(Player[A].Character == 3)
                                        Player[A].Location.SpeedY = Physics.PlayerGravity * 10;
                                    else if(Player[A].Character == 4)
                                        Player[A].Location.SpeedY = Physics.PlayerGravity * 7.5;
                                    else
                                        Player[A].Location.SpeedY = Physics.PlayerGravity * 5;
                                }
                                else
                                {
                                    if(NPC[Player[A].StandingOnNPC].Location.SpeedY > Physics.PlayerGravity * 5)
                                        NPC[Player[A].StandingOnNPC].Location.SpeedY = Physics.PlayerGravity * 5;
                                }

                                if(
                                    !(
                                        (!Player[A].YoshiBlue && (Player[A].CanFly == true || Player[A].CanFly2 == true)) ||
                                        (Player[A].Mount == 3 && Player[A].CanFly2)
                                    )
                                )
                                {
                                    if(std::rand() % 10 > 9)
                                    {
                                        NewEffect(80, newLoc(Player[A].Location.X - 8 + std::fmod(std::rand(), Player[A].Location.Width + 16) - 4, Player[A].Location.Y - 8 + std::fmod(std::rand(), Player[A].Location.Height + 16)), 1, 0, ShadowMode);
                                        Effect[numEffects].Location.SpeedX = std::fmod(std::rand(), 0.5) - 0.25;
                                        Effect[numEffects].Location.SpeedY = std::fmod(std::rand(), 0.5) - 0.25;
                                        Effect[numEffects].Frame = 1;
                                    }
                                }
                            }
                        }
                    }
                    Player[A].Location.Y = Player[A].Location.Y + Player[A].Location.SpeedY;
                }

                // princess peach and toad stuff
                if(Player[A].Character == 3 || Player[A].Character == 4 || Player[A].Character == 5)
                {
                    Player[A].HeldBonus = 0;
                    // power up limiter
                    // If (.Character = 3 Or .Character = 4) And .State > 3 And .State <> 7 Then .State = 2

                    if(Player[A].Mount == 3)
                    {
                        PlayerHurt(A);
                        Player[A].Mount = 0;
                    }
                    if(Player[A].Slide == true)
                        Player[A].Slide = false;
                    // If .Stoned = True Then .Stoned = False
                    if(Player[A].Hearts == 1 && Player[A].State > 1)
                        Player[A].Hearts = 2;
                    if(Player[A].Hearts > 1 && Player[A].State == 1)
                        Player[A].Hearts = 1;
                    if(Player[A].Hearts == 0)
                    {
                        if(Player[A].State == 1)
                            Player[A].Hearts = 1;
                        if(Player[A].State >= 2)
                            Player[A].Hearts = 2;
                    }
                }

                // link stuff
                if(Player[A].Character == 5)
                {
                    if(Player[A].State == 4 || Player[A].State == 5)
                    {

                        if(Player[A].FlyCount > 0 || ((Player[A].Controls.AltJump == true || (Player[A].Controls.Jump == true && Player[A].FloatRelease == true)) && Player[A].Location.SpeedY != Physics.PlayerGravity && Player[A].Slope == 0 && Player[A].StandingOnNPC == 0))
                        {
                            if(Player[A].FlyCount > 0)
                                Player[A].FairyCD = 0;
                            if(Player[A].Fairy == false && Player[A].FairyCD == 0 && Player[A].Jump == 0 && Player[A].Wet == 0)
                            {
                                Player[A].Jump = 0;
                                if(Player[A].FlyCount == 0)
                                    Player[A].FlyCount = 50;
                                Player[A].FairyTime = Player[A].FlyCount;
                                Player[A].FairyCD = 1;
                                Player[A].FlyCount = 0;
                                Player[A].Fairy = true;
                                SizeCheck(A);
                                PlaySound(87);
                                Player[A].Immune = 10;
                                Player[A].Effect = 8;
                                Player[A].Effect2 = 4;
                                NewEffect(63, Player[A].Location);
                            }
                        }
                        if(Player[A].Controls.Run == true && Player[A].RunRelease == true && (Player[A].FairyTime > 0 || Player[A].Effect == 8))
                        {
                            Player[A].FairyTime = 0;
                            Player[A].Controls.Run = false;
                        }
                        if(Player[A].Fairy == true)
                        {
                            if(Player[A].Slope > 0 || Player[A].StandingOnNPC > 0)
                            {
                                Player[A].FairyTime = 0;
                                Player[A].FairyCD = 0;
                            }
                        }
                        // Coins = Coins - 1
                        // If Coins < 0 Then
                        // Lives = Lives - 1
                        // Coins = Coins + 99
                        // If Lives < 0 Then
                        // Lives = 0
                        // Coins = 0
                        // .FairyTime = 0
                        // End If
                        // End If
                        // End If
                    }
                    if(Player[A].HasKey == true)
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
                    if(Player[A].SwordPoke < 0)
                    {
                        Player[A].SwordPoke = Player[A].SwordPoke - 1;
                        if(Player[A].SwordPoke == -7)
                            Player[A].SwordPoke = 1;
                        if(Player[A].SwordPoke == -40)
                            Player[A].SwordPoke = 0;
                        if(!(wasSlippy == true && Player[A].Controls.Left == false && Player[A].Controls.Right == false))
                        {
                            if(Player[A].FireBallCD == 0 && Player[A].Location.SpeedX != 0)
                                Player[A].SwordPoke = 0;
                        }
                    }
                    else if(Player[A].SwordPoke > 0)
                    {
                        if(Player[A].SwordPoke == 1)
                        {
                            TailSwipe(A, true, true);
                            PlaySound(77);
                            if((Player[A].State == 3 || Player[A].State == 7 || Player[A].State == 6) && Player[A].FireBallCD2 == 0)
                            {
                                Player[A].FireBallCD2 = 40;
                                if(Player[A].State == 6)
                                    Player[A].FireBallCD2 = 25;
                                if(Player[A].State == 6)
                                    PlaySound(90);
                                else
                                    PlaySound(82);

                                numNPCs = numNPCs + 1;
                                if(ShadowMode == true)
                                    NPC[numNPCs].Shadow = true;
                                NPC[numNPCs].Type = 13;
                                if(Player[A].State == 7)
                                    NPC[numNPCs].Type = 265;
                                if(Player[A].State == 6)
                                    NPC[numNPCs].Type = 266;
                                NPC[numNPCs].Projectile = true;
                                NPC[numNPCs].Location.Height = NPCHeight[NPC[numNPCs].Type];
                                NPC[numNPCs].Location.Width = NPCWidth[NPC[numNPCs].Type];
                                NPC[numNPCs].Location.X = Player[A].Location.X + Player[A].Location.Width / 2.0 + (40 * Player[A].Direction) - 8;
                                if(Player[A].Duck == false)
                                {
                                    NPC[numNPCs].Location.Y = Player[A].Location.Y + 5;
                                    if(Player[A].State == 6)
                                        NPC[numNPCs].Location.Y = NPC[numNPCs].Location.Y + 7;
                                }
                                else
                                {
                                    NPC[numNPCs].Location.Y = Player[A].Location.Y + 18;
                                    if(Player[A].State == 6)
                                        NPC[numNPCs].Location.Y = NPC[numNPCs].Location.Y + 4;
                                }


                                NPC[numNPCs].Active = true;
                                NPC[numNPCs].TimeLeft = 100;
                                NPC[numNPCs].Location.SpeedY = 20;
                                NPC[numNPCs].CantHurt = 100;
                                NPC[numNPCs].CantHurtPlayer = A;
                                NPC[numNPCs].Special = Player[A].Character;
                                if(NPC[numNPCs].Type == 13)
                                    NPC[numNPCs].Frame = 16;
                                NPC[numNPCs].WallDeath = 5;
                                NPC[numNPCs].Location.SpeedY = 0;
                                NPC[numNPCs].Location.SpeedX = 5 * Player[A].Direction + (Player[A].Location.SpeedX / 3);
                                if(Player[A].State == 6)
                                    NPC[numNPCs].Location.SpeedX = 9 * Player[A].Direction + (Player[A].Location.SpeedX / 3);
                                if(Player[A].StandingOnNPC != 0)
                                    NPC[numNPCs].Location.Y = NPC[numNPCs].Location.Y - Player[A].Location.SpeedY;
                                CheckSectionNPC(numNPCs);
                            }
                        }
                        else
                            TailSwipe(A, false, true);
                        Player[A].SwordPoke = Player[A].SwordPoke + 1;
                        if(Player[A].Duck == true)
                        {
                            if(Player[A].SwordPoke >= 10)
                            {
                                Player[A].SwordPoke = 0;
                                Player[A].FireBallCD = 7;
                            }
                        }
                        else
                        {
                            if(Player[A].SwordPoke >= 10)
                            {
                                Player[A].SwordPoke = -11;
                                Player[A].FireBallCD = 0;
                            }
                        }
                    }
                    if(Player[A].FireBallCD == 0 && Player[A].Wet == 0 && Player[A].Fairy == false && Player[A].Mount == 0)
                    {
                        if(Player[A].Duck == false && Player[A].Location.SpeedY < Physics.PlayerGravity && Player[A].StandingOnNPC == 0 && Player[A].Slope == 0 && Player[A].Controls.Up == false && Player[A].Stoned == false) // Link ducks when jumping
                        {
                            Player[A].SwordPoke = 0;
                            Player[A].Duck = true;
                            Player[A].Location.Y = Player[A].Location.Y + Player[A].Location.Height;
                            Player[A].Location.Height = Physics.PlayerDuckHeight[Player[A].Character][Player[A].State];
                            Player[A].Location.Y = Player[A].Location.Y - Player[A].Location.Height;
                        }
                        else if(Player[A].Duck == true && Player[A].Location.SpeedY > Physics.PlayerGravity && Player[A].StandingOnNPC == 0 && Player[A].Slope == 0) // Link stands when falling
                        {
                            Player[A].SwordPoke = 0;
                            UnDuck(A);
                        }
                    }
                    if(Player[A].Mount > 0 && Player[A].Mount != 2)
                    {
                        PlayerHurt(A);
                        Player[A].Mount = 0;
                    }
                    Player[A].HoldingNPC = -1;
                }

                if(Player[A].Controls.Jump == true)
                    Player[A].FloatRelease = false;
                else
                    Player[A].FloatRelease = true;

                // Player interactions
                Player[A].Location.SpeedX = Player[A].Location.SpeedX + Player[A].Bumped2;
                Player[A].Location.X = Player[A].Location.X + Player[A].Bumped2;
                Player[A].Bumped2 = 0;
                if(Player[A].Mount == 0)
                    Player[A].YoshiYellow = false;

                // level wrap
                if(LevelWrap[Player[A].Section] == true)
                {
                    if(Player[A].Location.X + Player[A].Location.Width < level[Player[A].Section].X)
                        Player[A].Location.X = level[Player[A].Section].Width - 1;
                    else if(Player[A].Location.X > level[Player[A].Section].Width)
                        Player[A].Location.X = level[Player[A].Section].X - Player[A].Location.Width + 1;
                    // Walk offscreen exit
                }
                else if(OffScreenExit[Player[A].Section] == true)
                {
                    if(Player[A].Location.X + Player[A].Location.Width < level[Player[A].Section].X)
                    {
                        LevelBeatCode = 3;
                        EndLevel = true;
                        for(B = 1; B <= numPlayers; B++)
                            Player[B].TailCount = 0;
                        LevelMacro = 0;
                        LevelMacroCounter = 0;
                        frmMain.clearBuffer();
                        frmMain.repaint();
                    }
                    else if(Player[A].Location.X > level[Player[A].Section].Width)
                    {
                        LevelBeatCode = 3;
                        EndLevel = true;
                        LevelMacro = 0;
                        LevelMacroCounter = 0;
                        frmMain.clearBuffer();
                        frmMain.repaint();
                    }
                }
                else if(LevelMacro != 1 && LevelMacro != 7 && GameMenu == false)
                {
                    // Check edge of levels
                    if(Player[A].Location.X < level[Player[A].Section].X)
                    {
                        Player[A].Location.X = level[Player[A].Section].X;
                        if(Player[A].Location.SpeedX < 0)
                            Player[A].Location.SpeedX = 0;
                        Player[A].Pinched2 = 2;
                        if(AutoX[Player[A].Section] != 0.0f)
                            Player[A].NPCPinched = 2;
                    }
                    else if(Player[A].Location.X + Player[A].Location.Width > level[Player[A].Section].Width)
                    {
                        Player[A].Location.X = level[Player[A].Section].Width - Player[A].Location.Width;
                        if(Player[A].Location.SpeedX > 0)
                            Player[A].Location.SpeedX = 0;
                        Player[A].Pinched4 = 2;
                        if(AutoX[Player[A].Section] != 0.f)
                            Player[A].NPCPinched = 2;
                    }
                }
                if(Player[A].Location.Y < level[Player[A].Section].Y - Player[A].Location.Height - 32 && Player[A].StandingOnTempNPC == 0)
                {
                    Player[A].Location.Y = level[Player[A].Section].Y - Player[A].Location.Height - 32;
                    if(AutoY[Player[A].Section] != 0.f)
                        Player[A].NPCPinched = 3;
                }

                // gives the players the sparkles when he is flying
                if(
                    (
                        (!Player[A].YoshiBlue && (Player[A].CanFly || Player[A].CanFly2)) ||
                        (Player[A].Mount == 3 && Player[A].CanFly2)
                    ) || Player[A].FlySparks
                )
                {
                    if(std::rand() % 4 > 3)
                    {
                        NewEffect(80,
                                  newLoc(Player[A].Location.X - 8 + std::fmod(std::rand(), Player[A].Location.Width + 16) - 4,
                                         Player[A].Location.Y - 8 + std::fmod(std::rand(), Player[A].Location.Height + 16)), 1, 0, ShadowMode);
                        Effect[numEffects].Location.SpeedX = std::fmod(std::rand(), 0.5) - 0.25;
                        Effect[numEffects].Location.SpeedY = std::fmod(std::rand(), 0.5) - 0.25;
                    }
                }


                Tanooki(A); // tanooki suit code



                oldSpeedY = Player[A].Location.SpeedY;


                if(Player[A].StandingOnNPC == -A)
                {
                    if(Player[A].Slope != 0)
                    {
                        B = NPC[Player[A].StandingOnNPC].Special;
                        if(BlockSlope[Block[B].Type] == 1)
                            PlrMid = Player[A].Location.X;
                        else
                            PlrMid = Player[A].Location.X + Player[A].Location.Width;
                        Slope = (PlrMid - Block[B].Location.X) / Block[B].Location.Width;
                        if(BlockSlope[Block[B].Type] < 0)
                            Slope = 1 - Slope;
                        if(Slope < 0)
                            Slope = 0;
                        if(Slope > 1)
                            Slope = 1;
                    }
                }



                // Block collisions.
                oldSlope = Player[A].Slope;
                Player[A].Slope = 0;
                tempSlope = 0;
                tempSlope2 = 0;
                tempSlope3 = 0;
                if(Player[A].Pinched1 > 0)
                    Player[A].Pinched1 = Player[A].Pinched1 - 1;
                if(Player[A].Pinched2 > 0)
                    Player[A].Pinched2 = Player[A].Pinched2 - 1;
                if(Player[A].Pinched3 > 0)
                    Player[A].Pinched3 = Player[A].Pinched3 - 1;
                if(Player[A].Pinched4 > 0)
                    Player[A].Pinched4 = Player[A].Pinched4 - 1;
                if(Player[A].NPCPinched > 0)
                    Player[A].NPCPinched = Player[A].NPCPinched - 1;

                if(Player[A].Character == 5 && Player[A].Duck == true && (Player[A].Location.SpeedY == Physics.PlayerGravity || Player[A].StandingOnNPC != 0 || Player[A].Slope != 0))
                {
                    Player[A].Location.Y = Player[A].Location.Y + Player[A].Location.Height;
                    Player[A].Location.Height = 30;
                    Player[A].Location.Y = Player[A].Location.Y - Player[A].Location.Height;
                }


                // block collision optimization
                fBlock = FirstBlock[(Player[A].Location.X / 32) - 1];
                lBlock = LastBlock[((Player[A].Location.X + Player[A].Location.Width) / 32.0) + 1];

                for(B = (int)fBlock; B <= lBlock; B++)
                {

                    // checks to see if a collision happened
                    if(Player[A].Location.X + Player[A].Location.Width >= Block[B].Location.X)
                    {
                        if(Player[A].Location.X <= Block[B].Location.X + Block[B].Location.Width)
                        {
                            if(Player[A].Location.Y + Player[A].Location.Height >= Block[B].Location.Y)
                            {
                                if(Player[A].Location.Y <= Block[B].Location.Y + Block[B].Location.Height)
                                {

                                    if(Block[B].Hidden == false)
                                    {
                                        HitSpot = FindRunningCollision(Player[A].Location, Block[B].Location); // this finds what part of the block the player collided

                                        if(BlockNoClipping[Block[B].Type] == true) // blocks that the player can't touch are forced to hitspot 0 (which means no collision)
                                            HitSpot = 0;

                                        if(BlockIsSizable[Block[B].Type] || BlockOnlyHitspot1[Block[B].Type] == true) // for sizable blocks, if the player didn't land on them from the top then he can walk through them
                                        {
                                            if(HitSpot != 1)
                                                HitSpot = 0;
                                            if(Player[A].Mount == 2 || Player[A].StandingOnTempNPC == 56)
                                                HitSpot = 0;
                                        }

                                        // for blocks that hurt the player
                                        if(BlockHurts[Block[B].Type])
                                        {
                                            if(Player[A].Mount == 2 ||
                                                (
                                                    (HitSpot == 1 && Player[A].Mount != 0) &&
                                                    !(Block[B].Type == 598)
                                                )
                                            )
                                            {}
                                            else
                                            {
                                                if(HitSpot == 1 && (Block[B].Type == 110 || Block[B].Type == 408 || Block[B].Type == 430 || Block[B].Type == 511))
                                                    PlayerHurt(A);
                                                if(HitSpot == 4 && (Block[B].Type == 269 || Block[B].Type == 429))
                                                    PlayerHurt(A);
                                                if(HitSpot == 3 && (Block[B].Type == 268 || Block[B].Type == 407 || Block[B].Type == 431))
                                                    PlayerHurt(A);
                                                if(HitSpot == 2 && (Block[B].Type == 267 || Block[B].Type == 428))
                                                    PlayerHurt(A);
                                                if(Block[B].Type == 109)
                                                    PlayerHurt(A);
                                                if(Block[B].Type == 598)
                                                {
                                                    if(Player[A].Mount > 0 && HitSpot == 1)
                                                    {
                                                        C = Player[A].Location.Y + Player[A].Location.Height;
                                                        Player[A].Location.Y = Block[B].Location.Y - Player[A].Location.Height;
                                                        PlayerHurt(A);
                                                        Player[A].Location.Y = C - Player[A].Location.Height;
                                                    }
                                                    else
                                                        PlayerHurt(A);
                                                }


                                                if(Player[A].TimeToLive > 0)
                                                    break;
                                            }
                                        }

                                        // hitspot 5 means the game doesn't know where the collision happened
                                        // if the player just stopped ducking and there is a hitspot 5 then force hitspot 3 (hit block from below)
                                        if(HitSpot == 5 && (Player[A].StandUp == true || NPC[Player[A].StandingOnNPC].Location.SpeedY < 0))
                                        {
                                            if(BlockSlope[Block[B].Type] == 0)
                                                HitSpot = 3;
                                        }

                                        // if the block is invisible and the player didn't hit it from below then the player won't collide with it
                                        if(Block[B].Invis == true)
                                        {
                                            if(HitSpot != 3)
                                                HitSpot = 0;
                                        }

                                        // fixes a bug with holding an npc that is really a block
                                        if(Player[A].HoldingNPC > 0)
                                        {
                                            if(NPC[Player[A].HoldingNPC].Block > 0)
                                            {
                                                if(NPC[Player[A].HoldingNPC].Block == B)
                                                    HitSpot = 0;
                                            }
                                        }

                                        // destroy some blocks if the player is touching it as a statue
                                        if(Block[B].Type == 457 && Player[A].Stoned == true)
                                        {
                                            HitSpot = 0;
                                            KillBlock(B);
                                        }

                                        // shadowmode is a cheat that allows the player to walk through walls
                                        if(ShadowMode == true && HitSpot != 1 && !(Block[B].Special > 0 && HitSpot == 3))
                                            HitSpot = 0;

                                        // this handles the collision for blocks that are sloped on the bottom
                                        if(BlockSlope2[Block[B].Type] != 0 && (Player[A].Location.Y > Block[B].Location.Y || (HitSpot != 2 && HitSpot != 4)) && HitSpot != 1 && ShadowMode == false)
                                        {
                                            HitSpot = 0;
                                            tempSlope = B;
                                            if(BlockSlope2[Block[B].Type] == 1)
                                                PlrMid = Player[A].Location.X + Player[A].Location.Width;
                                            else
                                                PlrMid = Player[A].Location.X;
                                            Slope = (PlrMid - Block[B].Location.X) / Block[B].Location.Width;
                                            if(BlockSlope2[Block[B].Type] > 0)
                                                Slope = 1 - Slope;
                                            if(Slope < 0)
                                                Slope = 0;
                                            if(Slope > 1)
                                                Slope = 1;
                                            if(Player[A].Location.Y <= Block[B].Location.Y + Block[B].Location.Height - (Block[B].Location.Height * Slope))
                                            {
                                                if(BlockKills[Block[B].Type] == true)
                                                {
                                                    if(GodMode == false)
                                                        PlayerDead(A);
                                                }
                                                if(Player[A].Location.SpeedY == 0.0 ||
                                                   Player[A].Location.SpeedY == Physics.PlayerGravity || Player[A].Slope > 0 || Player[A].StandingOnNPC != 0)
                                                {
                                                    PlrMid = Player[A].Location.Y;
                                                    Slope = (PlrMid - Block[B].Location.Y) / Block[B].Location.Height;
                                                    if(Slope < 0)
                                                        Slope = 0;
                                                    if(Slope > 1)
                                                        Slope = 1;
                                                    if(BlockSlope2[Block[B].Type] < 0)
                                                        Player[A].Location.X = Block[B].Location.X + Block[B].Location.Width - (Block[B].Location.Width * Slope);
                                                    else
                                                        Player[A].Location.X = Block[B].Location.X + (Block[B].Location.Width * Slope) - Player[A].Location.Width;
                                                    Player[A].Location.SpeedX = 0;

                                                }
                                                else
                                                {
                                                    Player[A].Location.Y = Block[B].Location.Y + Block[B].Location.Height - (Block[B].Location.Height * Slope);
                                                    if(Player[A].Location.SpeedY < 0)
                                                        PlaySound(3);
                                                    if(Player[A].Location.SpeedY < -0.01)
                                                        Player[A].Location.SpeedY = -0.01;
                                                    if(Player[A].Mount == 2)
                                                        Player[A].Location.SpeedY = 2;
                                                    if(Player[A].CanFly2 == true)
                                                        Player[A].Location.SpeedY = 2;
                                                }
                                                Player[A].Jump = 0;
                                            }
                                        }

                                        // collision for blocks that are sloped on the top
                                        if(BlockSlope[Block[B].Type] != 0 && HitSpot != 3 && !(BlockSlope[Block[B].Type] == -1 && HitSpot == 2) && !(BlockSlope[Block[B].Type] == 1 && HitSpot == 4) && (Player[A].Location.Y + Player[A].Location.Height - 4 - C <= Block[B].Location.Y + Block[B].Location.Height || (Player[A].Location.Y + Player[A].Location.Height - 12 <= Block[B].Location.Y + Block[B].Location.Height && Player[A].StandingOnNPC != 0)))
                                        {
                                            HitSpot = 0;
                                            if(
                                                (Player[A].Mount == 1 || Player[A].Location.SpeedY >= 0 ||
                                                 Player[A].Slide || SuperSpeed || Player[A].Stoned) &&
                                                (Player[A].Location.Y + Player[A].Location.Height <= Block[B].Location.Y + Block[B].Location.Height + Player[A].Location.SpeedY + 0.001 ||
                                                (Player[A].Slope == 0 && Block[B].Location.SpeedY < 0))
                                            )
                                            {
                                                if(BlockSlope[Block[B].Type] == 1)
                                                    PlrMid = Player[A].Location.X;
                                                else
                                                    PlrMid = Player[A].Location.X + Player[A].Location.Width;
                                                Slope = (PlrMid - Block[B].Location.X) / Block[B].Location.Width;
                                                if(BlockSlope[Block[B].Type] < 0)
                                                    Slope = 1 - Slope;
                                                if(Slope < 0)
                                                    Slope = 0;
                                                if(Slope > 1)
                                                    Slope = 1;
                                                if(tempHit3 > 0)
                                                {
                                                    if(BlockIsSizable[Block[tempHit3].Type] == false)
                                                    {
                                                        if(Block[tempHit3].Location.Y != Block[B].Location.Y)
                                                            tempHit3 = 0;
                                                    }
                                                    else
                                                    {
                                                        if(Block[tempHit3].Location.Y == Block[B].Location.Y + Block[B].Location.Height)
                                                            tempHit3 = 0;
                                                    }
                                                }
                                                if(tempHit2 == true)
                                                {
                                                    if(Block[tempSlope2].Location.Y + Block[tempSlope2].Location.Height == Block[B].Location.Y && BlockSlope[Block[tempSlope2].Type] == BlockSlope[Block[B].Type])
                                                    {
                                                        tempHit2 = false;
                                                        tempSlope2 = 0;
                                                        Player[A].Location.X = tempSlope2X;
                                                    }
                                                }
                                                if(tempSlope3 > 0)
                                                {
                                                    Player[A].Location.Y = Block[tempSlope3].Location.Y + Block[tempSlope3].Location.Height + 0.01;
                                                    PlrMid = Player[A].Location.Y + Player[A].Location.Height;
                                                    Slope = 1 - (PlrMid - Block[B].Location.Y) / Block[B].Location.Height;
                                                    if(Slope < 0)
                                                        Slope = 0;
                                                    if(Slope > 1)
                                                        Slope = 1;
                                                    if(BlockSlope[Block[B].Type] > 0)
                                                        Player[A].Location.X = Block[B].Location.X + Block[B].Location.Width - (Block[B].Location.Width * Slope);
                                                    else
                                                        Player[A].Location.X = Block[B].Location.X + (Block[B].Location.Width * Slope) - Player[A].Location.Width;
                                                    Player[A].Location.SpeedX = 0;
                                                }
                                                else
                                                {
                                                    if(Player[A].Location.Y >= Block[B].Location.Y + (Block[B].Location.Height * Slope) - Player[A].Location.Height - 0.1)
                                                    {

                                                        if(Player[A].GroundPound == true)
                                                        {
                                                            YoshiPound(A, tempHit3, true);
                                                            Player[A].GroundPound = false;
                                                        }
                                                        else if(Player[A].YoshiYellow == true)
                                                        {
                                                            if(oldSlope == 0)
                                                                YoshiPound(A, tempHit3);
                                                        }

                                                        Player[A].Location.Y = Block[B].Location.Y + (Block[B].Location.Height * Slope) - Player[A].Location.Height - 0.1;

                                                        if(Player[A].Location.SpeedY > Player[A].Location.SpeedX * (Block[B].Location.Height / static_cast<double>(Block[B].Location.Width)) * BlockSlope[Block[B].Type] || Player[A].Slide == false)
                                                        {
                                                            if(Player[A].WetFrame == false)
                                                            {
                                                                C = Player[A].Location.SpeedX * (Block[B].Location.Height / static_cast<double>(Block[B].Location.Width)) * BlockSlope[Block[B].Type];
                                                                Player[A].Location.SpeedY = C;
                                                                if(Player[A].Location.SpeedY > 0 && Player[A].Slide == false && Player[A].Mount != 1 && Player[A].Mount != 2)
                                                                    Player[A].Location.SpeedY = Player[A].Location.SpeedY * 4;
                                                            }
                                                        }

                                                        Player[A].Slope = B;
                                                        if(BlockSlope[Block[B].Type] == 1 && GameMenu == true && Player[A].Location.SpeedX >= 2)
                                                        {
                                                            if(Player[A].Mount == 0 && Player[A].HoldingNPC == 0 && Player[A].Character <= 2)
                                                            {
                                                                if(Player[A].Duck == true)
                                                                    UnDuck(A);
                                                                Player[A].Slide = true;
                                                            }
                                                        }



                                                        if(Player[A].Location.SpeedY < 0 && Player[A].Slide == false && SuperSpeed == false && Player[A].Stoned == false)
                                                            Player[A].Location.SpeedY = 0;
                                                        if(Block[B].Location.SpeedX != 0.0 || Block[B].Location.SpeedY != 0.0)
                                                        {
                                                            NPC[-A] = blankNPC;
                                                            NPC[-A].Location = Block[B].Location;
                                                            NPC[-A].Type = 58;
                                                            NPC[-A].Active = true;
                                                            NPC[-A].TimeLeft = 100;
                                                            NPC[-A].Section = Player[A].Section;
                                                            NPC[-A].Special = B;
                                                            NPC[-A].Special2 = BlockSlope[Block[B].Type];
                                                            Player[A].StandingOnNPC = -A;
                                                            movingBlock = true;
                                                            if(
                                                                    // HERE WAS A BUG: Compare bool with 0 is always false
                                                                    (Player[A].Location.SpeedX - NPC[Player[A].StandingOnNPC].Location.SpeedX < 0 &&
                                                                     BlockSlope[Block[B].Type] < 0) ||
                                                                    (Player[A].Location.SpeedX - NPC[Player[A].StandingOnNPC].Location.SpeedX > 0 &&
                                                                     BlockSlope[Block[B].Type] > 0)
                                                            )
                                                            {
                                                                if((Player[A].Location.SpeedX < 0 && Block[B].Location.SpeedX > 0) || (Player[A].Location.SpeedX > 0 && Block[B].Location.SpeedX < 0))
                                                                    Player[A].Location.SpeedY = 12;
                                                            }
                                                            NPC[-A].Location.Y = Player[A].Location.Y + Player[A].Location.Height;
                                                        }
                                                    }
                                                }
                                            }
                                        }


                                        // this is a fix to help the player deal with lava blocks a bit easier
                                        // it moves the blocks hitbox down a few pixels
                                        if(BlockKills[Block[B].Type] && BlockSlope[Block[B].Type] == 0 && GodMode == false && !(Player[A].Mount == 1 && Player[A].MountType == 2))
                                        {
                                            if(Player[A].Location.Y + Player[A].Location.Height < Block[B].Location.Y + 6)
                                                HitSpot = 0;
                                        }

                                        // kill the player if touching a lava block
                                        if(BlockKills[Block[B].Type] && (HitSpot > 0 || Player[A].Slope == B))
                                        {
                                            if(GodMode == false)
                                            {
                                                if(!(Player[A].Mount == 1 && Player[A].MountType == 2))
                                                {
                                                    PlayerDead(A);
                                                    break;
                                                }
                                                else if(HitSpot != 1 && BlockSlope[Block[B].Type] == 0)
                                                {
                                                    PlayerDead(A);
                                                    break;
                                                }
                                                else
                                                {
                                                    tempLocation.Y = Player[A].Location.Y + Player[A].Location.Height - 2;
                                                    tempLocation.X = Player[A].Location.X - 4 + std::fmod(std::rand(), Player[A].Location.Width + 8) - 4;
                                                    NewEffect(74, tempLocation);
                                                }
                                            }
                                        }

                                        // if hitspot 5 with a sloped block then don't collide with it. the collision should have already been handled by the slope code above
                                        if(HitSpot == 5 && BlockSlope[Block[B].Type] != 0)
                                            HitSpot = 0;

                                        // shelsurfing code
                                        if(HitSpot > 1 && Player[A].ShellSurf == true)
                                        {
                                            Player[A].ShellSurf = false;
                                            Player[A].Location.SpeedY = NPC[Player[A].StandingOnNPC].Location.SpeedY + Physics.PlayerJumpVelocity * 0.75;
                                            Player[A].StandingOnNPC = 0;
                                            PlaySound(3);
                                        }

                                        if(Block[B].Type == 626 && Player[A].Character == 1)
                                            HitSpot = 0;
                                        if(Block[B].Type == 627 && Player[A].Character == 2)
                                            HitSpot = 0;
                                        if(Block[B].Type == 628 && Player[A].Character == 3)
                                            HitSpot = 0;
                                        if(Block[B].Type == 629 && Player[A].Character == 4)
                                            HitSpot = 0;
                                        if(Block[B].Type == 632 && Player[A].Character == 5)
                                            HitSpot = 0;

                                        // the following code is where the collisions are handled




                                        if((HitSpot == 1 || Player[A].Slope == B) && Block[B].Slippy == true)
                                            Player[A].Slippy = true;


                                        if(HitSpot == 5 && Player[A].Quicksand > 0) // fixes quicksand hitspot 3 bug
                                        {
                                            if(Player[A].Location.Y - Player[A].Location.SpeedY < Block[B].Location.Y + Block[B].Location.Height)
                                                HitSpot = 3;
                                        }

                                        if(HitSpot == 1) // landed on the block from the top V
                                        {
                                            if(Player[A].Fairy == true && (Player[A].FairyCD > 0 || Player[A].Location.SpeedY > 0))
                                                Player[A].FairyTime = 0;
                                            Player[A].Pinched1 = 2; // for players getting squashed
                                            if(Block[B].Location.SpeedY != 0)
                                                Player[A].NPCPinched = 2;
                                            Player[A].Vine = 0; // stop climbing because you are now walking
                                            if(Player[A].Mount == 2) // for the clown car, make a niose and pound the ground if moving down fast enough
                                            {
                                                if(Player[A].Location.SpeedY > 3)
                                                {
                                                    PlaySound(37);
                                                    YoshiPound(A, B, true);
                                                }
                                            }
                                            if(tempHit3 == 0) // For walking
                                            {
                                                tempHit3 = B;
                                                tempLocation3 = Block[B].Location;
                                            }
                                            else // Find the best block to walk on if touching multiple blocks
                                            {
                                                if(Block[B].Location.SpeedY != 0 && Block[tempHit3].Location.SpeedY == 0)
                                                {
                                                    tempHit3 = B;
                                                    tempLocation3 = Block[B].Location;
                                                }
                                                else if(Block[B].Location.SpeedY == 0 && Block[tempHit3].Location.SpeedY != 0)
                                                {
                                                }
                                                else
                                                {
                                                    C = Block[B].Location.X + Block[B].Location.Width * 0.5;
                                                    D = Block[tempHit3].Location.X + Block[tempHit3].Location.Width * 0.5;

                                                    C = C - (Player[A].Location.X + Player[A].Location.Width * 0.5);
                                                    D = D - (Player[A].Location.X + Player[A].Location.Width * 0.5);
                                                    if(C < 0)
                                                        C = -C;
                                                    if(D < 0)
                                                        D = -D;
                                                    if(C < D)
                                                        tempHit3 = B;
                                                }

                                                // if this block is moving up give it priority
                                                if(Block[B].Location.SpeedY < 0 && Block[B].Location.Y < Block[tempHit3].Location.Y)
                                                {
                                                    tempHit3 = B;
                                                    tempLocation3 = Block[B].Location;
                                                }

                                            }

                                        }
                                        else if(HitSpot == 2) // hit the block from the right <----
                                        {

                                            if(BlockSlope[Block[oldSlope].Type] == 1 && Block[oldSlope].Location.Y <= Block[B].Location.Y)
                                            {

                                            }
                                            else
                                            {
                                                if(Player[A].Mount == 2)
                                                    Player[A].mountBump = Player[A].Location.X;
                                                tempSlope2X = Player[A].Location.X;
                                                Player[A].Location.X = Block[B].Location.X + Block[B].Location.Width + 0.01;
                                                tempSlope2 = B;
                                                tempHit2 = true;
                                                blockPushX = Block[B].Location.SpeedX;
                                                if(Player[A].Mount == 2)
                                                    Player[A].mountBump = -Player[A].mountBump + Player[A].Location.X;
                                                Player[A].Pinched2 = 2;
                                                if(Block[B].Location.SpeedX != 0)
                                                    Player[A].NPCPinched = 2;
                                            }
                                        }
                                        else if(HitSpot == 4) // hit the block from the left -------.
                                        {
                                            if(Player[A].Mount == 2)
                                                Player[A].mountBump = Player[A].Location.X;
                                            tempSlope2X = Player[A].Location.X;
                                            Player[A].Location.X = Block[B].Location.X - Player[A].Location.Width - 0.01;
                                            tempSlope2 = B;
                                            tempHit2 = true;
                                            blockPushX = Block[B].Location.SpeedX;
                                            if(Player[A].Mount == 2)
                                                Player[A].mountBump = -Player[A].mountBump + Player[A].Location.X;
                                            Player[A].Pinched4 = 2;
                                            if(Block[B].Location.SpeedX != 0)
                                                Player[A].NPCPinched = 2;
                                        }
                                        else if(HitSpot == 3) // hit the block from below
                                        {
                                            if(Player[A].ForceHitSpot3 == false && Player[A].StandUp == false)
                                                Player[A].Pinched3 = 2;
                                            if(Block[B].Location.SpeedY != 0)
                                                Player[A].NPCPinched = 2;
                                            tempHit = true;
                                            if(tempBlockHit[1] == 0)
                                                tempBlockHit[1] = B;
                                            else
                                                tempBlockHit[2] = B;
                                        }
                                        else if(HitSpot == 5) // try to find out where the player hit the block from
                                        {
                                            if(oldSlope > 0)
                                            {
                                                Player[A].Location.Y = Block[B].Location.Y + Block[B].Location.Height + 0.01;
                                                PlrMid = Player[A].Location.Y + Player[A].Location.Height;
                                                Slope = 1 - (PlrMid - Block[oldSlope].Location.Y) / Block[oldSlope].Location.Height;
                                                if(Slope < 0)
                                                    Slope = 0;
                                                if(Slope > 1)
                                                    Slope = 1;
                                                if(BlockSlope[Block[oldSlope].Type] > 0)
                                                    Player[A].Location.X = Block[oldSlope].Location.X + Block[oldSlope].Location.Width - (Block[oldSlope].Location.Width * Slope);
                                                else
                                                    Player[A].Location.X = Block[oldSlope].Location.X + (Block[oldSlope].Location.Width * Slope) - Player[A].Location.Width;
                                                Player[A].Location.SpeedX = 0;
                                            }
                                            else
                                            {
                                                tempSlope3 = B;
                                                if(Player[A].Location.X + Player[A].Location.Width / 2.0 < Block[B].Location.X + Block[B].Location.Width / 2.0)
                                                    Player[A].Pinched4 = 2;
                                                else
                                                    Player[A].Pinched2 = 2;
                                                if(Block[B].Location.SpeedX != 0 || Block[B].Location.SpeedY != 0)
                                                    Player[A].NPCPinched = 2;
                                                tempLocation.X = Player[A].Location.X;
                                                tempLocation.Width = Player[A].Location.Width;
                                                tempLocation.Y = Player[A].Location.Y + Player[A].Location.Height;
                                                tempLocation.Height = 0.1;
                                                tempBool = false;
                                                fBlock = FirstBlock[(tempLocation.X / 32) - 1];
                                                lBlock = LastBlock[((tempLocation.X + tempLocation.Width) / 32.0) + 1];
                                                for(C = fBlock; C <= lBlock; C++)
                                                {
                                                    if(CheckCollision(tempLocation, Block[C].Location) == true && Block[C].Hidden == false)
                                                    {

                                                        if(BlockSlope[Block[C].Type] == 0)
                                                            tempBool = true;
                                                        else
                                                        {
                                                            Player[A].Location.Y = Block[B].Location.Y + Block[B].Location.Height; // + 0.01
                                                            PlrMid = Player[A].Location.Y + Player[A].Location.Height;
                                                            Slope = 1 - (PlrMid - Block[C].Location.Y) / Block[C].Location.Height;
                                                            if(Slope < 0)
                                                                Slope = 0;
                                                            if(Slope > 1)
                                                                Slope = 1;
                                                            if(BlockSlope[Block[C].Type] > 0)
                                                                Player[A].Location.X = Block[C].Location.X + Block[C].Location.Width - (Block[C].Location.Width * Slope);
                                                            else
                                                                Player[A].Location.X = Block[C].Location.X + (Block[C].Location.Width * Slope) - Player[A].Location.Width;
                                                            Player[A].Location.SpeedX = 0;
                                                            break;
                                                        }
                                                    }
                                                }
                                                if(tempBool == true)
                                                {
                                                    Player[A].CanJump = false;
                                                    Player[A].Jump = 0;
                                                    Player[A].Location.X = Player[A].Location.X - 4 * Player[A].Direction;
                                                    Player[A].Location.Y = Player[A].Location.Y - Player[A].Location.SpeedY;
                                                    Player[A].Location.SpeedX = 0;
                                                    Player[A].Location.SpeedY = 0;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                    }
                }

                if(Player[A].Character == 5 && Player[A].Duck == true)
                {
                    Player[A].Location.Y = Player[A].Location.Y + Player[A].Location.Height;
                    Player[A].Location.Height = Physics.PlayerDuckHeight[Player[A].Character][Player[A].State];
                    Player[A].Location.Y = Player[A].Location.Y - Player[A].Location.Height;
                }


                // helps the player run down slopes at different angles
                if(Player[A].Slope == 0 && oldSlope > 0 && Player[A].Mount != 1 && Player[A].Mount != 2 && Player[A].Slide == false)
                {
                    if(Player[A].Location.SpeedY > 0)
                    {
                        C = Player[A].Location.SpeedX * (Block[oldSlope].Location.Height / static_cast<double>(Block[oldSlope].Location.Width)) * BlockSlope[Block[oldSlope].Type];
                        if(C > 0)
                            Player[A].Location.SpeedY = C;
                    }
                }

                if(tempHit3 > 0) // For walking
                {

                    if(Player[A].StandingOnNPC == -A) // fors standing on movable blocks
                    {
                        if(NPC[Player[A].StandingOnNPC].Special2 != 0)
                        {
                            Player[A].Location.SpeedX = Player[A].Location.SpeedX - NPC[Player[A].StandingOnNPC].Location.SpeedX;
                            movingBlock = false;
                            Player[A].StandingOnNPC = 0;
                        }
                    }

                    // diggable dirt
                    if(Block[tempHit3].Type == 370 && Player[A].StandingOnNPC <= 0) // dig dirt
                    {
                        DontResetGrabTime = true;
                        B = tempHit3;
                        if(Player[A].TailCount == 0 && Player[A].Controls.Down == true && Player[A].Controls.Run == true && Player[A].Mount == 0 && Player[A].Stoned == false && Player[A].HoldingNPC == 0 && (Player[A].GrabTime > 0 || Player[A].RunRelease == true))
                        {
                            if((Player[A].GrabTime >= 12 && Player[A].Character < 3) || (Player[A].GrabTime >= 16 && Player[A].Character == 3) || (Player[A].GrabTime >= 8 && Player[A].Character == 4))
                            {
                                Player[A].Location.SpeedX = Player[A].GrabSpeed;
                                Player[A].GrabSpeed = 0;
                                Block[B].Hidden = true;
                                Block[B].Layer = "Destroyed Blocks";
                                NewEffect(10, Block[B].Location);
                                Effect[numEffects].Location.SpeedY = -2;
                                Player[A].GrabTime = 0;
                            }
                            else
                            {
                                if(Player[A].GrabTime == 0)
                                {
                                    PlaySound(23);
                                    Player[A].FrameCount = 0;
                                    Player[A].GrabSpeed = Player[A].Location.SpeedX;
                                }
                                Player[A].Location.SpeedX = 0;
                                Player[A].Slide = false;
                                Player[A].GrabTime = Player[A].GrabTime + 1;
                            }
                        }
                    }
                    if(tempHit2 == true)
                    {
                        if(WalkingCollision(Player[A].Location, Block[tempHit3].Location) == true)
                        {

//                            if(nPlay.Online == true && A == nPlay.MySlot + 1) // online stuffs
//                            {
//                                curLoc = Player[A].Location;
//                                Player[A].Location = oldLoc;
//                                if(Block[tempHit3].Type == 55 && FreezeNPCs == false)
//                                    Netplay::sendData Netplay::PutPlayerLoc(nPlay.MySlot);
//                                else if(Player[A].SpinJump == true && (Block[tempHit3].Type == 90 || Block[tempHit3].Type == 526) && Player[A].State > 1 && Block[tempHit3].Special == 0)
//                                    Netplay::sendData Netplay::PutPlayerLoc(nPlay.MySlot) + "1l" + std::to_string(A) + LB;
//                                Player[A].Location = curLoc;
//                            }

                            Player[A].Location.Y = tempLocation3.Y - Player[A].Location.Height;
                            if(Player[A].GroundPound == true)
                            {
                                YoshiPound(A, tempHit3, true);
                                Player[A].GroundPound = false;
                            }
                            else if(Player[A].YoshiYellow == true)
                                YoshiPound(A, tempHit3);
                            Player[A].Location.SpeedY = 0;
                            if(tempLocation3.SpeedX != 0.0 || tempLocation3.SpeedY != 0.0)
                            {
                                NPC[-A] = blankNPC;
                                NPC[-A].Location = tempLocation3;
                                NPC[-A].Type = 58;
                                NPC[-A].Active = true;
                                NPC[-A].TimeLeft = 100;
                                NPC[-A].Section = Player[A].Section;
                                NPC[-A].Special = tempHit3;
                                Player[A].StandingOnNPC = -A;
                                movingBlock = true;
                                Player[A].Location.SpeedY = 12;
                            }
                            if(Block[tempHit3].Type == 55 && FreezeNPCs == false) // Make the player jump if the block is bouncy
                            {
                                if(Player[A].Slide == false)
                                    Player[A].Multiplier = 0;
                                BlockHit(tempHit3, true);
                                Player[A].Location.SpeedY = Physics.PlayerJumpVelocity;
                                PlaySound(3);
                                if(Player[A].Controls.Jump == true || Player[A].Controls.AltJump == true)
                                {
                                    PlaySound(1);
                                    Player[A].Jump = Physics.PlayerBlockJumpHeight;
                                    if(Player[A].Character == 2)
                                        Player[A].Jump = Player[A].Jump + 3;
                                    if(Player[A].SpinJump == true)
                                        Player[A].Jump = Player[A].Jump - 6;
                                }
                            }
                            if(Player[A].SpinJump == true && (Block[tempHit3].Type == 90 || Block[tempHit3].Type == 526) && Player[A].State > 1 && Block[tempHit3].Special == 0)
                            {
                                Player[A].Location.SpeedY = Physics.PlayerJumpVelocity;
                                Block[tempHit3].Kill = true;
                                iBlocks = iBlocks + 1;
                                iBlock[iBlocks] = tempHit3;
                                HitSpot = 0;
                                tempHit3 = 0;
                                Player[A].Jump = 7;
                                if(Player[A].Character == 2)
                                    Player[A].Jump = Player[A].Jump + 3;
                                if(Player[A].Controls.Down == true)
                                {
                                    Player[A].Jump = 0;
                                    Player[A].Location.SpeedY = Physics.PlayerJumpVelocity * 0.5;
                                }
                            }
                        }
                    }
                    else
                    {


//                        if(nPlay.Online == true && A == nPlay.MySlot + 1) // online stuffs
//                        {
//                            curLoc = Player[A].Location;
//                            Player[A].Location = oldLoc;
//                            if(Block[tempHit3].Type == 55 && FreezeNPCs == false)
//                                Netplay::sendData Netplay::PutPlayerLoc(nPlay.MySlot);
//                            else if(Player[A].SpinJump == true && (Block[tempHit3].Type == 90 || Block[tempHit3].Type == 526) && Player[A].State > 1 && Block[tempHit3].Special == 0)
//                                Netplay::sendData Netplay::PutPlayerLoc(nPlay.MySlot) + "1l" + std::to_string(A) + LB;
//                            Player[A].Location = curLoc;
//                        }


                        Player[A].Location.Y = tempLocation3.Y - Player[A].Location.Height;
                        if(Player[A].StandingOnNPC != 0)
                        {
                            if(NPC[Player[A].StandingOnNPC].Location.Y <= tempLocation3.Y && Player[A].StandingOnNPC != Player[A].HoldingNPC)
                                Player[A].Location.Y = NPC[Player[A].StandingOnNPC].Location.Y - Player[A].Location.Height;
                        }
                        if(Player[A].GroundPound == true)
                        {
                            YoshiPound(A, tempHit3, true);
                            Player[A].GroundPound = false;
                        }
                        else if(Player[A].YoshiYellow == true)
                            YoshiPound(A, tempHit3);
                        if(Player[A].Slope == 0 || Player[A].Slide == true)
                            Player[A].Location.SpeedY = 0;
                        if(tempLocation3.SpeedX != 0.0 || tempLocation3.SpeedY != 0.0)
                        {
                            NPC[-A] = blankNPC;
                            NPC[-A].Location = tempLocation3;
                            NPC[-A].Type = 58;
                            NPC[-A].Active = true;
                            NPC[-A].TimeLeft = 100;
                            NPC[-A].Section = Player[A].Section;
                            NPC[-A].Special = tempHit3;
                            Player[A].StandingOnNPC = -A;
                            movingBlock = true;
                            Player[A].Location.SpeedY = 12;
                        }

                        if(Player[A].StandingOnNPC != 0 && movingBlock == false)
                        {
                            Player[A].Location.SpeedY = 1;
                            if(NPC[Player[A].StandingOnNPC].Pinched == false && FreezeNPCs == false)
                                Player[A].Location.SpeedX = Player[A].Location.SpeedX - NPC[Player[A].StandingOnNPC].Location.SpeedX - NPC[Player[A].StandingOnNPC].BeltSpeed;
                            Player[A].StandingOnNPC = 0;
                        }
                        else if(movingBlock == true)
                        {

                            Player[A].Location.SpeedY = NPC[-A].Location.SpeedY + 1;
                            if(Player[A].Location.SpeedY < 0)
                                Player[A].Location.SpeedY = 0;
                        }
                        else
                        {
                            if(Player[A].Slope == 0 || Player[A].Slide == true)
                                Player[A].Location.SpeedY = 0;
                        }

                        if(Block[tempHit3].Type == 55 && FreezeNPCs == false) // Make the player jump if the block is bouncy
                        {
                            BlockHit(tempHit3, true);
                            if(Player[A].Slide == false)
                                Player[A].Multiplier = 0;
                            Player[A].Location.SpeedY = Physics.PlayerJumpVelocity;
                            PlaySound(3);
                            if(Player[A].Controls.Jump == true || Player[A].Controls.AltJump == true)
                            {
                                PlaySound(1);
                                Player[A].Jump = Physics.PlayerBlockJumpHeight;
                                if(Player[A].Character == 2)
                                    Player[A].Jump = Player[A].Jump + 3;
                                if(Player[A].SpinJump == true)
                                    Player[A].Jump = Player[A].Jump - 6;
                            }
                        }
                        if(Player[A].SpinJump == true && (Block[tempHit3].Type == 90 || Block[tempHit3].Type == 526) && Player[A].State > 1 && Block[tempHit3].Special == 0)
                        {
                            Player[A].Location.SpeedY = Physics.PlayerJumpVelocity;
                            Block[tempHit3].Kill = true;
                            iBlocks = iBlocks + 1;
                            iBlock[iBlocks] = tempHit3;
                            tempHit3 = 0;
                            Player[A].Jump = 7;
                            if(Player[A].Character == 2)
                                Player[A].Jump = Player[A].Jump + 3;
                            if(Player[A].Controls.Down == true)
                            {
                                Player[A].Jump = 0;
                                Player[A].Location.SpeedY = Physics.PlayerJumpVelocity * 0.5;
                            }
                        }



                    }
                }
                if(tempSlope2 > 0 && tempSlope > 0)
                {
                    if(Block[tempSlope].Location.Y + Block[tempSlope].Location.Height == Block[tempSlope2].Location.Y + Block[tempSlope2].Location.Height)
                        tempHit2 = false;
                }
                if(tempHit == false && tempHit2 == true)
                {
                    if(Player[A].Location.SpeedX + NPC[Player[A].StandingOnNPC].Location.SpeedX > 0 && Player[A].Controls.Right == true)
                    {
                        Player[A].Location.SpeedX = 0.2 * Player[A].Direction;
                        if(blockPushX > 0)
                            Player[A].Location.SpeedX = Player[A].Location.SpeedX + blockPushX;
                    }
                    else if(Player[A].Location.SpeedX + NPC[Player[A].StandingOnNPC].Location.SpeedX < 0 && Player[A].Controls.Left == true)
                    {
                        Player[A].Location.SpeedX = 0.2 * Player[A].Direction;
                        if(blockPushX < 0)
                            Player[A].Location.SpeedX = Player[A].Location.SpeedX + blockPushX;
                    }
                    else
                    {
                        if(Player[A].Controls.Right == true || Player[A].Controls.Left == true)
                            Player[A].Location.SpeedX = -NPC[Player[A].StandingOnNPC].Location.SpeedX + 0.2 * Player[A].Direction;
                        else
                            Player[A].Location.SpeedX = 0;
                    }
                    if(Player[A].Mount == 2)
                        Player[A].Location.SpeedX = 0;
                }
                if(tempBlockHit[2] != 0) // Hitting a block from below
                {
                    C = Block[tempBlockHit[1]].Location.X + Block[tempBlockHit[1]].Location.Width * 0.5;
                    D = Block[tempBlockHit[2]].Location.X + Block[tempBlockHit[2]].Location.Width * 0.5;
                    C = C - (Player[A].Location.X + Player[A].Location.Width * 0.5);
                    D = D - (Player[A].Location.X + Player[A].Location.Width * 0.5);
                    if(C < 0)
                        C = -C;
                    if(D < 0)
                        D = -D;
                    if(C < D)
                        B = tempBlockHit[1];
                    else
                        B = tempBlockHit[2];
                }
                else if(tempBlockHit[1] != 0)
                {
                    B = tempBlockHit[1];
                    if(Block[B].Location.X + Block[B].Location.Width - Player[A].Location.X <= 4)
                    {
                        Player[A].Location.X = Block[B].Location.X + Block[B].Location.Width + 0.1;
                        B = 0;
                    }
                    else if(Player[A].Location.X + Player[A].Location.Width - Block[B].Location.X <= 4)
                    {
                        Player[A].Location.X = Block[B].Location.X - Player[A].Location.Width - 0.1;
                        B = 0;
                    }
                }
                else
                    B = 0;
                if(B > 0)
                {

                    // Netplay code
//                    if(nPlay.Online == true && A == nPlay.MySlot + 1) // online stuffs
//                    {
//                        curLoc = Player[A].Location;
//                        Player[A].Location = oldLoc;
//                        Netplay::sendData Netplay::PutPlayerLoc(nPlay.MySlot);
//                        Player[A].Location = curLoc;
//                    }

                    PlaySound(3);
                    Player[A].Jump = 0;
                    Player[A].Location.Y = Block[B].Location.Y + Block[B].Location.Height + 0.01;
                    Player[A].Location.SpeedY = -0.01 + Block[B].Location.SpeedY;
                    if(Player[A].Fairy == true)
                        Player[A].Location.SpeedY = 2;
                    if(Player[A].Vine > 0)
                        Player[A].Location.Y = Player[A].Location.Y + 0.1;
                    if(Player[A].Mount == 2)
                        Player[A].Location.SpeedY = 2;
                    if(Player[A].CanFly2 == true)
                        Player[A].Location.SpeedY = 2;
                    if(Player[A].Mount != 2) // Tell the block it was hit
                        BlockHit(B, false, A);
                    if(Block[B].Type == 55) // If it is a bouncy block the knock the player down
                        Player[A].Location.SpeedY = 3;
                    if(Player[A].State > 1 && Player[A].Character != 5) // If the player was big ask the block nicely to die
                    {
                        if(Player[A].Mount != 2 && Block[B].Type != 293)
                            BlockHitHard(B);
                    }
                }

                if(Player[A].Slide == true && oldSlope > 0 && Player[A].Slope == 0 && Player[A].Location.SpeedY < 0)
                {
                    if(Player[A].NoGravity == 0)
                        Player[A].NoGravity = static_cast<int>(floor(static_cast<double>(Player[A].Location.SpeedY / Physics.PlayerJumpVelocity * 8)));
                }
                else if(Player[A].Slope > 0 || oldSlope > 0 || Player[A].Slide == false)
                    Player[A].NoGravity = 0;
                if(Player[A].Slide == true)
                {
                    if(Player[A].Location.SpeedX > 1 || Player[A].Location.SpeedX < -1)
                        Player[A].SlideKill = true;
                    else
                        Player[A].SlideKill = false;
                }
                else
                    Player[A].SlideKill = false;





                // Check NPC collisions
                if(Player[A].Vine > 0)
                    Player[A].Vine = Player[A].Vine - 1;
                tempBlockHit[1] = 0;
                tempBlockHit[2] = 0;
                tempHitSpeed = 0;
                spinKill = false;


                // check vine backgrounds
                for(B = 1; B <= numBackground; B++)
                {
                    if(BackgroundFence[Background[B].Type] == true)
                    {
                        if(CheckCollision(Player[A].Location, Background[B].Location))
                        {
                            tempLocation = Background[B].Location;
                            tempLocation.Height = tempLocation.Height - 16;
                            tempLocation.Width = tempLocation.Width - 20;
                            tempLocation.X = tempLocation.X + 10;
                            if(CheckCollision(Player[A].Location, tempLocation))
                            {
                                if(Player[A].Character == 5)
                                {
                                    if(Player[A].Immune == 0 && Player[A].Controls.Up == true)
                                    {
                                        Player[A].FairyCD = 0;
                                        if(Player[A].Fairy == false)
                                        {
                                            Player[A].Fairy = true;
                                            SizeCheck(A);
                                            PlaySound(87);
                                            Player[A].Immune = 10;
                                            Player[A].Effect = 8;
                                            Player[A].Effect2 = 4;
                                            NewEffect(63, Player[A].Location);
                                        }
                                        if(Player[A].FairyTime != -1 && Player[A].FairyTime < 20)
                                            Player[A].FairyTime = 20;
                                    }
                                }
                                else if(Player[A].Fairy == false && Player[A].Stoned == false)
                                {
                                    if(Player[A].Mount == 0 && Player[A].HoldingNPC <= 0)
                                    {
                                        if(Player[A].Vine > 0)
                                        {
                                            if(Player[A].Duck == true)
                                                UnDuck(A);
                                            if(Player[A].Location.Y >= Background[B].Location.Y - 20 && Player[A].Vine < 2)
                                                Player[A].Vine = 2;
                                            if(Player[A].Location.Y >= Background[B].Location.Y - 18)
                                                Player[A].Vine = 3;
                                        }
                                        else if((Player[A].Controls.Up == true || (Player[A].Controls.Down == true && !(Player[A].Location.SpeedY == 0) && !(Player[A].StandingOnNPC != 0) && !(Player[A].Slope > 0))) && Player[A].Jump == 0)
                                        {
                                            if(Player[A].Duck == true)
                                                UnDuck(A);
                                            if(Player[A].Location.Y >= Background[B].Location.Y - 20 && Player[A].Vine < 2)
                                                Player[A].Vine = 2;
                                            if(Player[A].Location.Y >= Background[B].Location.Y - 18)
                                                Player[A].Vine = 3;
                                        }
                                        if(Player[A].Vine > 0)
                                            Player[A].VineNPC = -1;
                                    }
                                }
                            }
                        }
                    }
                }

                if(Player[A].StandingOnNPC != 0)
                {
                    if(tempHit2 == false)
                    {
                        if(NPC[Player[A].StandingOnNPC].Pinched == false && FreezeNPCs == false)
                            Player[A].Location.SpeedX = Player[A].Location.SpeedX - NPC[Player[A].StandingOnNPC].Location.SpeedX - NPC[Player[A].StandingOnNPC].BeltSpeed;
                    }
                }
                tempHit = false; // Used for JUMP detection
                tempHit2 = false;

                int tempNumNPCsMax = numNPCs;
                for(B = 1; B <= tempNumNPCsMax; B++)
                {
                    if(NPC[B].Active && NPC[B].Killed == 0 && NPC[B].Effect != 5 && NPC[B].Effect != 6)
                    {
                        // If Not (NPC(B).Type = 17 And NPC(B).CantHurt > 0) And Not (.Mount = 2 And NPC(B).Type = 56) And Not NPC(B).standingOnPlayer = A And Not NPC(B).Type = 197 And Not NPC(B).Type = 237 Then
                        if(!(Player[A].Mount == 2 && NPC[B].Type == 56) && !(NPC[B].standingOnPlayer == A) && !(NPC[B].Type == 197) && !(NPC[B].Type == 237))
                        {
                            if(NPC[B].HoldingPlayer == 0 || NPCIsABonus[NPC[B].Type] || (BattleMode == true && NPC[B].HoldingPlayer != A))
                            {
                                if(CheckCollision(Player[A].Location, NPC[B].Location) == true)
                                {
                                    if((NPC[B].Type == 58 || NPC[B].Type == 21 || NPC[B].Type == 67 || NPC[B].Type == 68 || NPC[B].Type == 69 || NPC[B].Type == 70) && NPC[B].Projectile == true)
                                        PlayerHurt(A);
                                    if((Player[A].Mount == 1 || Player[A].Mount == 3 || Player[A].SpinJump || (Player[A].ShellSurf && NPCIsAShell[NPC[B].Type]) || (Player[A].Stoned && !NPCCanWalkOn[NPC[B].Type])) && !NPCMovesPlayer[NPC[B].Type])
                                        HitSpot = BootCollision(Player[A].Location, NPC[B].Location, NPCCanWalkOn[NPC[B].Type]); // find the hitspot for normal mario
                                    else
                                        HitSpot = EasyModeCollision(Player[A].Location, NPC[B].Location, NPCCanWalkOn[NPC[B].Type]); // find the hitspot when in a shoe or on a yoshi

                                    if(NPC[B].Inert == false)
                                    {
                                        // battlemode stuff
                                        if(NPC[B].Type == 13 || NPC[B].Type == 171 || NPC[B].Type == 265 || NPC[B].Type == 266 || NPC[B].Type == 108 || NPC[B].Type == 291 || NPC[B].Type == 292)
                                        {
                                            if(BattleMode == true && NPC[B].CantHurtPlayer != A)
                                            {
                                                if(Player[A].State == 6 && Player[A].Duck == true && Player[A].Character != 5)
                                                    NPCHit(B, 3, B);
                                                else
                                                {
                                                    if(Player[A].Immune == 0)
                                                    {
                                                        NPCHit(B, 3, B);
                                                        if(NPC[B].Type == 266)
                                                            PlaySound(89);
                                                    }
                                                    PlayerHurt(A);
                                                }
                                            }
                                            HitSpot = 0;
                                        }
                                        if(NPC[B].Type == 17 && NPC[B].CantHurt > 0)
                                        {
                                            if(BattleMode == false)
                                                HitSpot = 0;
                                            else if(NPC[B].CantHurtPlayer != A)
                                            {
                                                if(HitSpot != 1)
                                                    PlayerHurt(A);
                                                else
                                                {
                                                    NPC[B].CantHurt = 0;
                                                    NPC[B].CantHurtPlayer = 0;
                                                    NPC[B].Projectile = false;
                                                }
                                            }
                                        }
                                        if((NPC[B].Type == 50 || NPC[B].Type == 30) && BattleMode == true && NPC[B].CantHurtPlayer != A)
                                            PlayerHurt(A);
                                        if((NPC[B].Type == 263 || NPC[B].Type == 96) && BattleMode == true && NPC[B].CantHurtPlayer != A && NPC[B].Projectile == true && NPC[B].BattleOwner != A)
                                        {
                                            if(Player[A].Immune == 0 && NPC[B].Type == 96)
                                                NPC[B].Special2 = 1;
                                            PlayerHurt(A);
                                            HitSpot = 0;
                                        }
                                        if((NPCIsAShell[NPC[B].Type] == true || NPCIsVeggie[NPC[B].Type] || NPC[B].Type == 263 || NPC[B].Type == 45) && BattleMode == true && NPC[B].HoldingPlayer > 0 && NPC[B].HoldingPlayer != A)
                                        {
                                            if(Player[A].Immune == 0)
                                            {
                                                PlayerHurt(A);
                                                NPCHit(B, 5, B);
                                            }
                                        }
                                        if(NPCIsAParaTroopa[NPC[B].Type] && BattleMode == true && NPC[B].CantHurtPlayer == A)
                                            HitSpot = 0;
                                        if(BattleMode == true && NPCIsVeggie[NPC[B].Type] && NPC[B].Projectile == true)
                                        {
                                            if(NPC[B].CantHurtPlayer != A)
                                            {
                                                if(Player[A].Immune == 0)
                                                {
                                                    PlayerHurt(A);
                                                    NPCHit(B, 4, B);
                                                    PlaySound(39);
                                                }
                                            }
                                        }
                                        if(BattleMode == true && NPC[B].HoldingPlayer > 0 && NPC[B].HoldingPlayer != A)
                                        {
                                            if(NPCWontHurt[NPC[B].Type] == true)
                                                HitSpot = 0;
                                            else
                                                HitSpot = 5;
                                        }
                                        if(BattleMode == true && NPC[B].BattleOwner != A && NPC[B].Projectile == true && NPC[B].CantHurtPlayer != A)
                                        {
                                            if(NPC[B].Type == 134 || NPC[B].Type == 137 || NPC[B].Type == 154 || NPC[B].Type == 155 || NPC[B].Type == 156 || NPC[B].Type == 157 || NPC[B].Type == 166 || ((NPCIsAShell[NPC[B].Type] || NPC[B].Type == 45) && NPC[B].Location.SpeedX == 0))
                                            {
                                                if(NPCIsAShell[NPC[B].Type] && HitSpot == 1 && Player[A].SpinJump == true)
                                                {
                                                }
                                                else if(Player[A].Immune == 0)
                                                {
                                                    if(!(NPC[B].Type == 45) && !NPCIsAShell[NPC[B].Type])
                                                        NPCHit(B, 3, B);
                                                    PlayerHurt(A);
                                                    HitSpot = 0;
                                                }
                                            }
                                        }
                                        // end battlemode
                                    }
                                    if(NPC[B].Type == 283)
                                    {
                                        NPCHit(B, 1, A);
                                        HitSpot = 0;
                                    }
                                    if(GameOutro == true)
                                        HitSpot = 0;
                                    if(NPC[B].Type == 30 && NPC[B].CantHurt > 0)
                                        HitSpot = 0;
                                    if(NPC[B].Type == 96 && HitSpot == 1)
                                        HitSpot = 0;
                                    if(NPC[B].Inert == true) // if the npc is friendly then you can't touch it
                                    {
                                        HitSpot = 0;
                                        if(NPC[B].Text != "" && Player[A].Controls.Up == true && FreezeNPCs == false)
                                            MessageNPC = B;
                                    }
                                    if(Player[A].Stoned == true && HitSpot != 1) // if you are a statue then SLAM into the npc
                                    {
                                        if(Player[A].Location.SpeedX > 3 || Player[A].Location.SpeedX < -3)
                                            NPCHit(B, 3, B);
                                    }

                                    // the following code is for spin jumping and landing on things as yoshi/shoe
                                    if(Player[A].Mount == 1 || Player[A].Mount == 3 || Player[A].SpinJump == true || (Player[A].Stoned == true && NPCCanWalkOn[NPC[B].Type] == false))
                                    {
                                        if(HitSpot == 1)
                                        {
                                            if(Player[A].Mount == 1 || Player[A].Mount == 2 || Player[A].Stoned == true)
                                                NPCHit(B, 8, A);
                                            else if(!(NPC[B].Type == 245 || NPC[B].Type == 275 || NPC[B].Type == 8 || NPC[B].Type == 12 || NPC[B].Type == 36 || NPC[B].Type == 285 || NPC[B].Type == 286 || NPC[B].Type == 51 || NPC[B].Type == 52 || NPC[B].Type == 53 || NPC[B].Type == 54 || NPC[B].Type == 74 || NPC[B].Type == 93 || NPC[B].Type == 200 || NPC[B].Type == 205 || NPC[B].Type == 207 || NPC[B].Type == 201 || NPC[B].Type == 261 || NPC[B].Type == 270) && NPCCanWalkOn[NPC[B].Type] == false)
                                            {
                                                if(Player[A].Wet > 0 && (NPCIsCheep[NPC[B].Type] == true || NPC[B].Type == 231 || NPC[B].Type == 235))
                                                {
                                                }
                                                else
                                                    NPCHit(B, 8, A);
                                            }
                                            if(NPC[B].Killed == 8 || NPCIsCheep[NPC[B].Type] == true || NPC[B].Type == 179 || NPC[B].Type == 37 || NPC[B].Type == 180 || NPC[B].Type == 38 || NPC[B].Type == 42 || NPC[B].Type == 43 || NPC[B].Type == 44 || NPC[B].Type == 8 || NPC[B].Type == 12 || NPC[B].Type == 36 || NPC[B].Type == 51 || NPC[B].Type == 52 || NPC[B].Type == 53 || NPC[B].Type == 54 || NPC[B].Type == 74 || NPC[B].Type == 93 || NPC[B].Type == 200 || NPC[B].Type == 205 || NPC[B].Type == 207 || NPC[B].Type == 201 || NPC[B].Type == 199 || NPC[B].Type == 245 || NPC[B].Type == 256 || NPC[B].Type == 261 || NPC[B].Type == 275 || NPC[B].Type == 285 || NPC[B].Type == 286 || NPC[B].Type == 270) // tap
                                            {
                                                if(NPC[B].Killed == 8 && Player[A].Mount == 1 && Player[A].MountType == 2)
                                                {
                                                    numNPCs = numNPCs + 1;
                                                    NPC[numNPCs].Active = true;
                                                    NPC[numNPCs].TimeLeft = 100;
                                                    NPC[numNPCs].Section = Player[A].Section;
                                                    NPC[numNPCs].Type = 13;
                                                    NPC[numNPCs].Special = Player[A].Character;
                                                    NPC[numNPCs].Location.Height = NPCHeight[NPC[numNPCs].Type];
                                                    NPC[numNPCs].Location.Width = NPCWidth[NPC[numNPCs].Type];
                                                    NPC[numNPCs].Location.Y = Player[A].Location.Height + Player[A].Location.Y - NPC[numNPCs].Location.Height;
                                                    NPC[numNPCs].Location.X = Player[A].Location.X + Player[A].Location.Width / 2.0 - NPC[numNPCs].Location.Width / 2.0;
                                                    NPC[numNPCs].Location.SpeedX = 4;
                                                    NPC[numNPCs].Location.SpeedY = 10;
                                                    numNPCs = numNPCs + 1;
                                                    NPC[numNPCs].Active = true;
                                                    NPC[numNPCs].TimeLeft = 100;
                                                    NPC[numNPCs].Section = Player[A].Section;
                                                    NPC[numNPCs].Type = 13;
                                                    NPC[numNPCs].Special = Player[A].Character;
                                                    NPC[numNPCs].Location.Height = NPCHeight[NPC[numNPCs].Type];
                                                    NPC[numNPCs].Location.Width = NPCWidth[NPC[numNPCs].Type];
                                                    NPC[numNPCs].Location.Y = Player[A].Location.Height + Player[A].Location.Y - NPC[numNPCs].Location.Height;
                                                    NPC[numNPCs].Location.X = Player[A].Location.X + Player[A].Location.Width / 2.0 - NPC[numNPCs].Location.Width / 2.0;
                                                    NPC[numNPCs].Location.SpeedX = -4;
                                                    NPC[numNPCs].Location.SpeedY = 10;
                                                }
                                                if(NPC[B].Killed == 0 && Player[A].SpinJump == 0)
                                                    PlaySound(2);
                                                Player[A].ForceHitSpot3 = true;
                                                if(HitSpot == 1 && !(Player[A].GroundPound == true && NPC[B].Killed == 8))
                                                {
                                                    tempHit = true;
                                                    tempLocation.Y = NPC[B].Location.Y - Player[A].Location.Height;
                                                    if(Player[A].SpinJump == true)
                                                    {
                                                        if(NPC[B].Killed > 0)
                                                        {
                                                            if(Player[A].Controls.Down == true)
                                                                tempHit = false;
                                                            else
                                                                spinKill = true;
                                                        }
                                                        else
                                                            PlaySound(2);
                                                    }
                                                }
                                                HitSpot = 0;
                                            }
                                        }
                                    }
                                    else if(Player[A].Mount == 2)
                                    {
                                        if(NPC[B].standingOnPlayer == A)
                                            HitSpot = 0;
                                        else if(!(NPC[B].Type == 17 && NPC[B].CantHurt > 0))
                                        {
                                            if((NPC[B].Location.Y + NPC[B].Location.Height > Player[A].Location.Y + 18 && HitSpot != 3) || HitSpot == 1)
                                            {
                                                NPCHit(B, 8, A);
                                                if(NPC[B].Killed == 8)
                                                    HitSpot = 0;

                                                if(NPC[B].Type == 135 || NPC[B].Type == 136 || NPC[B].Type == 137)
                                                {
                                                    NPCHit(B, 3, B);
                                                    if(NPC[B].Killed == 3)
                                                        HitSpot = 0;
                                                }

                                            }
                                        }
                                    }

                                    if((Player[A].State == 6 && Player[A].Duck == true && Player[A].Mount == 0 && Player[A].Character != 5) || (Player[A].Mount == 1 && Player[A].MountType == 2)) // Fireball immune for ducking in the hammer suit
                                    {
                                        if(NPC[B].Type == 85 || NPC[B].Type == 87 || NPC[B].Type == 246 || NPC[B].Type == 276)
                                        {
                                            PlaySound(3);
                                            HitSpot = 0;
                                            NPC[B].Killed = 9;
                                            for(C = 1; C <= 10; C++)
                                            {
                                                NewEffect(77, NPC[B].Location, static_cast<float>(NPC[B].Special));
                                                Effect[numEffects].Location.SpeedX = std::rand() % 3 - 1.5 + NPC[B].Location.SpeedX * 0.1;
                                                Effect[numEffects].Location.SpeedY = std::rand() % 3 - 1.5 - NPC[B].Location.SpeedY * 0.1;
                                                if(Effect[numEffects].Frame == 0)
                                                    Effect[numEffects].Frame = -static_cast<int>(floor(static_cast<double>(std::rand() % 3)));
                                                else
                                                    Effect[numEffects].Frame = 5 + static_cast<int>(floor(static_cast<double>(std::rand() % 3)));
                                            }
                                            NPC[B].Location.X = NPC[B].Location.X + NPC[B].Location.Width / 2.0 - EffectWidth[10] / 2.0;
                                            NPC[B].Location.Y = NPC[B].Location.Y + NPC[B].Location.Height / 2.0 - EffectHeight[10] / 2.0;
                                            NewEffect(10, NPC[B].Location);
                                        }
                                    }


                                    if(NPCIsAVine[NPC[B].Type] == true) // if the player collided with a vine then see if he should climb it
                                    {
                                        if(Player[A].Character == 5)
                                        {
                                            if(Player[A].Immune == 0 && Player[A].Controls.Up == true)
                                            {
                                                Player[A].FairyCD = 0;
                                                if(Player[A].Fairy == false)
                                                {
                                                    Player[A].Fairy = true;
                                                    SizeCheck(A);
                                                    PlaySound(87);
                                                    Player[A].Immune = 10;
                                                    Player[A].Effect = 8;
                                                    Player[A].Effect2 = 4;
                                                    NewEffect(63, Player[A].Location);
                                                }
                                                if(Player[A].FairyTime != -1 && Player[A].FairyTime < 20)
                                                    Player[A].FairyTime = 20;
                                            }
                                        }
                                        else if(Player[A].Fairy == false && Player[A].Stoned == false)
                                        {
                                            if(Player[A].Mount == 0 && Player[A].HoldingNPC <= 0)
                                            {
                                                if(Player[A].Vine > 0)
                                                {
                                                    if(Player[A].Duck == true)
                                                        UnDuck(A);
                                                    if(Player[A].Location.Y >= NPC[B].Location.Y - 20 && Player[A].Vine < 2)
                                                        Player[A].Vine = 2;
                                                    if(Player[A].Location.Y >= NPC[B].Location.Y - 18)
                                                        Player[A].Vine = 3;
                                                }
                                                else if((Player[A].Controls.Up == true || (Player[A].Controls.Down == true && !(Player[A].Location.SpeedY == 0) && !(Player[A].StandingOnNPC != 0) && !(Player[A].Slope > 0))) && Player[A].Jump == 0)
                                                {
                                                    if(Player[A].Duck == true)
                                                        UnDuck(A);
                                                    if(Player[A].Location.Y >= NPC[B].Location.Y - 20 && Player[A].Vine < 2)
                                                        Player[A].Vine = 2;
                                                    if(Player[A].Location.Y >= NPC[B].Location.Y - 18)
                                                        Player[A].Vine = 3;
                                                }
                                                if(Player[A].Vine > 0)
                                                    Player[A].VineNPC = B;
                                            }
                                        }
                                    }

                                    // subcon warps
                                    if(NPC[B].Type == 289 && HitSpot > 0 && Player[A].Controls.Up == true)
                                    {
                                        if(NPC[B].Special2 >= 0)
                                        {
                                            NPC[B].Killed = 9;
                                            PlaySound(46);
                                            Player[A].Effect = 7;
                                            Player[A].Warp = numWarps + 1;
                                            Warp[numWarps + 1].Entrance = NPC[B].Location;
                                            tempLocation = NPC[B].Location;
                                            tempLocation.X = NPC[B].Location.X - level[Player[A].Section].X + level[NPC[B].Special2].X;
                                            tempLocation.Y = NPC[B].Location.Y - level[Player[A].Section].Y + level[NPC[B].Special2].Y;
                                            Warp[numWarps + 1].Exit = tempLocation;
                                            Warp[numWarps + 1].Hidden = false;
                                            Warp[numWarps + 1].NoYoshi = false;
                                            Warp[numWarps + 1].WarpNPC = true;
                                            Warp[numWarps + 1].Locked = false;
                                            Warp[numWarps + 1].Stars = 0;
                                            Player[A].Location.SpeedX = 0;
                                            Player[A].Location.SpeedY = 0;
                                            // Stop
                                            Player[A].Location.X = Warp[Player[A].Warp].Entrance.X + Warp[Player[A].Warp].Entrance.Width / 2.0 - Player[A].Location.Width / 2.0;
                                            Player[A].Location.Y = Warp[Player[A].Warp].Entrance.Y + Warp[Player[A].Warp].Entrance.Height - Player[A].Location.Height;
                                            tempLocation = Warp[numWarps + 1].Entrance;
                                            tempLocation.Y = tempLocation.Y - 32;
                                            tempLocation.Height = 64;
                                            NewEffect(54, tempLocation);
                                            tempLocation = Warp[numWarps + 1].Exit;
                                            tempLocation.Y = tempLocation.Y - 32;
                                            tempLocation.Height = 64;
                                            NewEffect(54, tempLocation);
                                        }
                                    }


                                    if(HitSpot == 1 && (NPC[B].Type == 32 || NPC[B].Type == 238 || NPC[B].Type == 239) && NPC[B].Projectile == true)
                                        HitSpot = 0;

                                    if(NPC[B].Type == 255 && Player[A].HasKey == true)
                                    {
                                        Player[A].HasKey = false;
                                        HitSpot = 0;
                                        NPC[B].Killed = 3;
                                    }

                                    if(NPC[B].Type == 45 && NPC[B].Projectile == true && HitSpot > 1)
                                        HitSpot = 5;

                                    if(HitSpot == 1) // Player landed on a NPC
                                    {
                                        if(NPCCanWalkOn[NPC[B].Type] == true || (Player[A].ShellSurf == true && NPCIsAShell[NPC[B].Type] == true)) // NPCs that can be walked on
                                        {
                                            // the player landed on an NPC he can stand on
                                            if(tempBlockHit[1] == 0)
                                                tempBlockHit[1] = B;
                                            else if(tempBlockHit[2] == 0)
                                                tempBlockHit[2] = B;
                                            else if(Player[A].StandingOnNPC == B)
                                            {
                                                // if standing on 2 or more NPCs find out the best one to stand on
                                                C = NPC[tempBlockHit[1]].Location.X + NPC[tempBlockHit[1]].Location.Width * 0.5;
                                                D = NPC[tempBlockHit[2]].Location.X + NPC[tempBlockHit[2]].Location.Width * 0.5;
                                                C = C - (Player[A].Location.X + Player[A].Location.Width * 0.5);
                                                D = D - (Player[A].Location.X + Player[A].Location.Width * 0.5);
                                                if(C < 0)
                                                    C = -C;
                                                if(D < 0)
                                                    D = -D;
                                                if(C < D)
                                                    tempBlockHit[2] = B;
                                                else
                                                    tempBlockHit[1] = B;
                                            }
                                            else
                                                tempBlockHit[2] = B;

                                            // if landing on a yoshi or boot, mount up!
                                        }
                                        else if((NPCIsYoshi[NPC[B].Type] || NPCIsBoot[NPC[B].Type]) && Player[A].Character != 5 && Player[A].Fairy == false)
                                        {
                                            if(Player[A].Mount == 0 && NPC[B].CantHurtPlayer != A && Player[A].Dismount == 0)
                                            {
                                                if(NPCIsBoot[NPC[B].Type])
                                                {
                                                    UnDuck(A);
                                                    NPC[B].Killed = 9;
                                                    if(Player[A].State == 1)
                                                    {
                                                        Player[A].Location.Height = Physics.PlayerHeight[1][2];
                                                        Player[A].Location.Y = Player[A].Location.Y - Physics.PlayerHeight[1][2] + Physics.PlayerHeight[Player[A].Character][1];
                                                    }
                                                    Player[A].Mount = 1;
                                                    if(NPC[B].Type == 35)
                                                        Player[A].MountType = 1;
                                                    if(NPC[B].Type == 191)
                                                        Player[A].MountType = 2;
                                                    if(NPC[B].Type == 193)
                                                        Player[A].MountType = 3;
                                                    PlaySound(2);
                                                }
                                                else if(NPCIsYoshi[NPC[B].Type] && (Player[A].Character == 1 || Player[A].Character == 2))
                                                {
                                                    UnDuck(A);
                                                    NPC[B].Killed = 9;
                                                    Player[A].Mount = 3;
                                                    if(NPC[B].Type == 95)
                                                        Player[A].MountType = 1;
                                                    else if(NPC[B].Type == 98)
                                                        Player[A].MountType = 2;
                                                    else if(NPC[B].Type == 99)
                                                        Player[A].MountType = 3;
                                                    else if(NPC[B].Type == 100)
                                                        Player[A].MountType = 4;
                                                    else if(NPC[B].Type == 148)
                                                        Player[A].MountType = 5;
                                                    else if(NPC[B].Type == 149)
                                                        Player[A].MountType = 6;
                                                    else if(NPC[B].Type == 150)
                                                        Player[A].MountType = 7;
                                                    else if(NPC[B].Type == 228)
                                                        Player[A].MountType = 8;
                                                    Player[A].YoshiNPC = 0;
                                                    Player[A].YoshiPlayer = 0;
                                                    Player[A].MountSpecial = 0;
                                                    Player[A].YoshiTonugeBool = false;
                                                    Player[A].YoshiTongueLength = 0;
                                                    PlaySound(48);
                                                    YoshiHeight(A);
                                                }
                                            }
                                        }
                                        else if(!(NPC[B].Type == 22) && !(NPC[B].Type == 31) && !(NPC[B].Type == 49) && !(NPC[B].Type == 50) && (Player[A].SlideKill == false || NPCWontHurt[NPC[B].Type] == true)) // NPCs that cannot be walked on
                                        {
                                            if(NPC[B].CantHurtPlayer == A && Player[A].NoShellKick > 0)
                                            {
                                            }
                                            else
                                            {
                                                if(NPCIsABonus[NPC[B].Type]) // Bonus
                                                    TouchBonus(A, B);
                                                else if(NPCIsAShell[NPC[B].Type] == true && NPC[B].Location.SpeedX == 0 && Player[A].HoldingNPC == 0 && Player[A].Controls.Run == true)
                                                {
                                                    // grab turtle shells
                                                    //if(nPlay.Online == false || nPlay.MySlot + 1 == A)
                                                    {
                                                        if(Player[A].Character >= 3)
                                                            PlaySound(23);
                                                        else
                                                            UnDuck(A);
                                                        Player[A].HoldingNPC = B;
                                                        NPC[B].HoldingPlayer = A;
                                                        NPC[B].CantHurt = Physics.NPCCanHurtWait;
                                                        NPC[B].CantHurtPlayer = A;
                                                    }

                                                }
                                                else if(NPCJumpHurt[NPC[B].Type] || (NPCIsCheep[NPC[B].Type] == true && Player[A].WetFrame == true)) // NPCs that cause damage even when jumped on
                                                {
                                                    if(!(NPC[B].Type == 8 && NPC[B].Special2 == 4) && NPCWontHurt[NPC[B].Type] == false && NPC[B].CantHurtPlayer != A)
                                                    {

                                                        // the n00bcollision function reduces the size of the npc's hit box before it damages the player
                                                        if(n00bCollision(Player[A].Location, NPC[B].Location))
                                                            PlayerHurt(A);
                                                    }
                                                }
                                                else if(NPC[B].Type == 15) // Special code for BOOM BOOM
                                                {
                                                    if(NPC[B].Special == 0 || Player[A].Mount == 1 || Player[A].Mount == 3)
                                                    {
                                                        if(NPC[B].Special != 0)
                                                            PlaySound(2);
                                                        tempHit = true;
                                                        tempLocation.Y = NPC[B].Location.Y - Player[A].Location.Height;
                                                    }
                                                    else if(NPC[B].Special != 4)
                                                    {
                                                        if(n00bCollision(Player[A].Location, NPC[B].Location))
                                                            PlayerHurt(A);
                                                    }
                                                }
                                                else if((NPC[B].Type == 137) || NPC[B].Type == 166)
                                                    NPCHit(B, 1, A); // NPC 'B' was jumped on '1' by player 'A'
                                                else if(NPC[B].Killed != 10 && NPCIsBoot[NPC[B].Type] == false && NPCIsYoshi[NPC[B].Type] == false && !(NPCIsAShell[NPC[B].Type] && NPC[B].CantHurtPlayer == A)) // Bounce off everything except Bonus and Piranha Plants
                                                {
                                                    if(NPC[B].Type == 26)
                                                        tempSpring = true;
                                                    if(NPCIsAShell[NPC[B].Type] && NPC[B].Location.SpeedX == 0 && NPC[B].Location.SpeedY == 0)
                                                        tempShell = true;
                                                    tempHit = true;
                                                    tempLocation.Y = NPC[B].Location.Y - Player[A].Location.Height;
                                                    if(NPC[B].Type == 32 || NPC[B].Type == 238 || NPC[B].Type == 239)
                                                    {
                                                        tempHit = false;
                                                        Player[A].Jump = false;
                                                        Player[A].Location.SpeedY = Physics.PlayerJumpVelocity;
                                                        Player[A].Location.SpeedY = -Physics.PlayerGravity;
                                                    }
                                                }

                                                if(
                                                    !(Player[A].WetFrame &&
                                                        (NPC[B].Type == 229 || NPC[B].Type == 230 ||
                                                         NPCIsAVine[NPC[B].Type])) &&
                                                    Player[A].HoldingNPC != B
                                                )
                                                {
                                                    if(Player[A].Vine > 0)
                                                    {
                                                        Player[A].Vine = 0;
                                                        Player[A].Jump = 1;
                                                    }
                                                    if(!(NPCIsAShell[NPC[B].Type] && NPC[B].CantHurtPlayer == A))
                                                        NPCHit(B, 1, A); // NPC 'B' was jumped on '1' by player 'A'
                                                }
                                            }
                                        }
                                    }
                                    else if(HitSpot == 0) // if hitspot = 0 then do nothing
                                    {

                                        // player touched an npc anywhere except from the top
                                    }
                                    else if(!(NPC[B].Type == 15 && NPC[B].Special == 4)) // Player touched an NPC
                                    {
                                        // grab code
                                        if(
                                                ((
                                                    Player[A].CanGrabNPCs ||
                                                    NPCIsGrabbable[NPC[B].Type] ||
                                                    (NPC[B].Effect == 2 && !NPCIsABonus[NPC[B].Type])
                                                 ) && (NPC[B].Effect == 0 || NPC[B].Effect == 2)) ||
                                                (NPCIsAShell[NPC[B].Type] && FreezeNPCs == true)
                                        ) // GRAB EVERYTHING
                                        {
                                            if(Player[A].Controls.Run == true)
                                            {
                                                if((HitSpot == 2 && Player[A].Direction == -1) || (HitSpot == 4 && Player[A].Direction == 1) || (NPC[B].Type == 22 || NPC[B].Type == 49 || NPC[B].Effect == 2 || (NPCIsVeggie[NPC[B].Type && NPC[B].CantHurtPlayer != A])))
                                                {
                                                    if(Player[A].HoldingNPC == 0)
                                                    {
                                                        if(!NPCIsAShell[NPC[B].Type] || Player[A].Character >= 3)
                                                        {
                                                            if(NPCIsVeggie[NPC[B].Type] == true)
                                                                PlaySound(73);
                                                            else
                                                                PlaySound(23);
                                                        }
                                                        if(Player[A].Character <= 2)
                                                            UnDuck(A);
                                                        Player[A].HoldingNPC = B;
                                                        NPC[B].Direction = Player[A].Direction;
                                                        NPC[B].Frame = EditorNPCFrame(NPC[B].Type, NPC[B].Direction);
                                                        NPC[B].HoldingPlayer = A;
                                                        NPC[B].CantHurt = Physics.NPCCanHurtWait;
                                                        NPC[B].CantHurtPlayer = A;
                                                    }
                                                }
                                            }
                                        }
                                        if(NPCIsAShell[NPC[B].Type] || (NPC[B].Type == 45 && NPC[B].Special == 1)) // Turtle shell
                                        {
                                            if(NPC[B].Location.SpeedX == 0 && NPC[B].Location.SpeedY >= 0) // Shell is not moving
                                            {
                                                if(((Player[A].Controls.Run == true && Player[A].HoldingNPC == 0) || Player[A].HoldingNPC == B) && !(NPC[B].CantHurtPlayer == A)) // Grab the shell
                                                {
                                                    if(Player[A].Character >= 3)
                                                        PlaySound(23);
                                                    else
                                                        UnDuck(A);
                                                    Player[A].HoldingNPC = B;
                                                    NPC[B].HoldingPlayer = A;
                                                    NPC[B].CantHurt = Physics.NPCCanHurtWait;
                                                    NPC[B].CantHurtPlayer = A;
                                                }
                                                else if(NPC[B].HoldingPlayer == 0) // Kick the shell
                                                {
                                                    if((Player[A].Mount == 1 || Player[A].Mount == 2 || Player[A].Mount == 3) && NPC[B].Type != 45)
                                                    {
                                                        if(NPC[B].Type != 195)
                                                        {
                                                            tempLocation.Y = Player[A].Location.Y;
                                                            tempHit = true;
                                                            NPCHit(B, 8, A);
                                                        }
                                                    }
                                                    else
                                                    {
                                                        tempLocation.Height = 0;
                                                        tempLocation.Width = 0;
                                                        tempLocation.Y = (Player[A].Location.Y + NPC[B].Location.Y * 4) / 5;
                                                        tempLocation.X = (Player[A].Location.X + NPC[B].Location.X * 4) / 5;
                                                        NewEffect(132, tempLocation);
                                                        NPC[B].CantHurt = 0;
                                                        NPC[B].CantHurtPlayer = 0;
                                                        NPCHit(B, 1, A);
                                                    }
                                                }
                                            }
                                            else if(NPC[B].Location.SpeedX != 0) // Got hit by the shell
                                            {
                                                if(NPC[B].CantHurtPlayer != A && FreezeNPCs == false && NPC[B].Type != 195)
                                                {
                                                    if(n00bCollision(Player[A].Location, NPC[B].Location))
                                                        PlayerHurt(A);
                                                }
                                            }
                                        }
                                        else if(NPCIsABonus[NPC[B].Type]) // Bonus
                                            TouchBonus(A, B);
                                        else // Everything else
                                        {
                                            if((NPC[B].Type == 137 || NPC[B].Type == 166) && NPC[B].HoldingPlayer != A) // kick the bob-om
                                            {
                                                if(NPC[B].TailCD == 0)
                                                {
                                                    NPC[B].TailCD = 12;
                                                    if(NPC[B].Type != 166 && NPC[B].Type != 137)
                                                        NewEffect(75, newLoc((Player[A].Location.X + NPC[B].Location.X + (Player[A].Location.Width + NPC[B].Location.Width) / 2.0) / 2, (Player[A].Location.Y + NPC[B].Location.Y + (Player[A].Location.Height + NPC[B].Location.Height) / 2.0) / 2));
                                                    NPCHit(B, 1, A);
                                                }
                                            }
                                            else if(NPC[B].CantHurtPlayer != A && NPCWontHurt[NPC[B].Type] == false)
                                            {
                                                if(!(NPC[B].Type == 17 && NPC[B].Projectile == true))
                                                {
                                                    if(NPC[B].Type >= 117 && NPC[B].Type <= 120 && NPC[B].Projectile == true)
                                                        NPCHit(B, 3, B);
                                                    else
                                                    {
                                                        if(NPC[B].Effect != 2)
                                                        {
                                                            if(Player[A].SlideKill == true && NPCJumpHurt[NPC[B].Type] == false)
                                                                NPCHit(B, 3, B);
                                                            if(NPC[B].Killed == 0)
                                                            {
                                                                if(n00bCollision(Player[A].Location, NPC[B].Location))
                                                                {
                                                                    if(BattleMode == true && NPC[B].HoldingPlayer != A && NPC[B].HoldingPlayer > 0 && Player[A].Immune == 0)
                                                                        NPCHit(B, 5, B);
                                                                    PlayerHurt(A);
                                                                }
                                                            }
                                                            else
                                                                MoreScore(NPCScore[NPC[B].Type], NPC[B].Location, Player[A].Multiplier);
                                                        }
                                                    }
                                                }
                                            }

                                            // this is for NPC that physically push the player
                                            if(NPCMovesPlayer[NPC[B].Type] == true && NPC[B].Projectile == false && !(Player[A].HoldingNPC == B) && !(Player[A].Mount == 2 && (NPC[B].Type == 31 || NPC[B].Type == 32)) && ShadowMode == false && NPC[B].Effect != 2)
                                            {
                                                if(Player[A].StandUp == true && Player[A].StandingOnNPC == 0)
                                                {
                                                    if(HitSpot == 5 && Player[A].Location.Y + Player[A].Location.Height - Physics.PlayerDuckHeight[Player[A].Character][Player[A].State] - Player[A].Location.SpeedY >= NPC[B].Location.Y + NPC[B].Location.Height)
                                                        HitSpot = 3;
                                                }
                                                if(HitSpot == 3)
                                                {
                                                    if(NPC[B].Type == 263 && Player[A].Character != 5 && Player[A].State > 1)
                                                        NPCHit(B, 3, B);
                                                    tempLocation = Player[A].Location;
                                                    Player[A].Location.SpeedY = 0.1 + NPC[B].Location.SpeedY;
                                                    Player[A].Location.Y = NPC[B].Location.Y + NPC[B].Location.Height + 0.1;
                                                    fBlock = FirstBlock[(Player[A].Location.X / 32) - 1];
                                                    lBlock = LastBlock[((Player[A].Location.X + Player[A].Location.Width) / 32.0) + 1];
                                                    for(C = fBlock; C <= lBlock; C++)
                                                    {
                                                        if(CheckCollision(Player[A].Location, Block[C].Location) && Block[C].Hidden == false && BlockIsSizable[Block[C].Type] == false && BlockOnlyHitspot1[Block[C].Type] == false)
                                                            Player[A].Location = tempLocation;
                                                    }
                                                    PlaySound(3);
                                                    Player[A].Jump = 0;
                                                    if(Player[A].Mount == 2)
                                                        Player[A].Location.SpeedY = Player[A].Location.SpeedY + 2;
                                                    if(NPC[B].Type == 58 || NPC[B].Type == 21 || NPC[B].Type == 67 || NPC[B].Type == 68 || NPC[B].Type == 69 || NPC[B].Type == 70 || (NPC[B].Type >= 78 && NPC[B].Type <= 83))
                                                    {
                                                        if(NPC[B].Location.SpeedY >= Physics.NPCGravity * 20)
                                                            PlayerHurt(A);
                                                    }
                                                }
                                                else
                                                {
                                                    tempBool = false;
                                                    if(Player[A].StandingOnNPC != 0)
                                                    {
                                                        if(NPC[Player[A].StandingOnNPC].Type == 57)
                                                            tempBool = true;
                                                    }

                                                    D = Player[A].Location.X;
                                                    if(Player[A].Location.X + Player[A].Location.Width / 2.0 < NPC[B].Location.X + NPC[B].Location.Width / 2.0)
                                                    {
                                                        Player[A].Pinched4 = 2;

                                                        if(NPC[B].Type != 31 && NPC[B].Type != 32 && NPC[B].Type != 57 && (NPC[B].Location.SpeedX != 0 || NPC[B].Location.SpeedY != 0 || NPC[B].BeltSpeed))
                                                            Player[A].NPCPinched = 2;
                                                        Player[A].Location.X = NPC[B].Location.X - Player[A].Location.Width - 0.1;
                                                        tempHit2 = true;
                                                        Player[A].RunCount = 0;
                                                        tempHitSpeed = NPC[B].Location.SpeedX + NPC[B].BeltSpeed;
                                                        if(tempHit3 != 0)
                                                        {
                                                            if(std::abs(Block[tempHit3].Location.X - NPC[B].Location.X) < 1)
                                                            {
                                                                Player[A].Location.X = NPC[B].Location.X - Player[A].Location.Width - 1;
                                                                Player[A].Location.SpeedY = oldSpeedY;
                                                            }
                                                        }
                                                        if(tempBool == false && NPC[B].Type != 168)
                                                            Player[A].Location.SpeedX = 0.2 * Player[A].Direction;
                                                        if(NPC[Player[A].StandingOnNPC].Type == 57)
                                                            Player[A].Location.X = Player[A].Location.X - 1;
                                                        if(tempBlockHit[1] > 0)
                                                        {
                                                            if(NPC[B].Location.X >= NPC[tempBlockHit[1]].Location.X - 2 && NPC[B].Location.X <= NPC[tempBlockHit[1]].Location.X + 2)
                                                                tempBlockHit[1] = tempBlockHit[2];
                                                        }
                                                        if(tempBlockHit[2] > 0)
                                                        {
                                                            if(NPC[B].Location.X >= NPC[tempBlockHit[2]].Location.X - 2 && NPC[B].Location.X <= NPC[tempBlockHit[2]].Location.X + 2)
                                                                tempBlockHit[2] = 0;
                                                        }
                                                    }
                                                    else
                                                    {
                                                        Player[A].Pinched2 = 2;
                                                        if(NPC[B].Type != 31 && NPC[B].Type != 32 && NPC[B].Type != 57 && (NPC[B].Location.SpeedX != 0 || NPC[B].Location.SpeedY != 0 || NPC[B].BeltSpeed))
                                                            Player[A].NPCPinched = 2;
                                                        Player[A].Location.X = NPC[B].Location.X + NPC[B].Location.Width + 0.01;
                                                        tempHit2 = true;
                                                        Player[A].RunCount = 0;
                                                        tempHitSpeed = NPC[B].Location.SpeedX + NPC[B].BeltSpeed;
                                                        if(tempHit3 != 0)
                                                        {
                                                            if(std::abs(Block[tempHit3].Location.X + Block[tempHit3].Location.Width - NPC[B].Location.X - NPC[B].Location.Width) < 1)
                                                            {
                                                                Player[A].Location.X = NPC[B].Location.X + NPC[B].Location.Width + 1;
                                                                Player[A].Location.SpeedY = oldSpeedY;
                                                            }
                                                        }
                                                        if(tempBool == false && NPC[B].Type != 168)
                                                            Player[A].Location.SpeedX = 0.2 * Player[A].Direction;
                                                        if(tempBlockHit[1] > 0)
                                                        {
                                                            if(NPC[B].Location.X + NPC[B].Location.Width >= NPC[tempBlockHit[1]].Location.X + NPC[tempBlockHit[1]].Location.Width - 2 && NPC[B].Location.X + NPC[B].Location.Width <= NPC[tempBlockHit[1]].Location.X + NPC[tempBlockHit[1]].Location.Width + 2)
                                                                tempBlockHit[1] = tempBlockHit[2];
                                                        }
                                                        if(tempBlockHit[2] > 0)
                                                        {
                                                            if(NPC[B].Location.X + NPC[B].Location.Width >= NPC[tempBlockHit[2]].Location.X + NPC[tempBlockHit[2]].Location.Width - 2 && NPC[B].Location.X + NPC[B].Location.Width <= NPC[tempBlockHit[2]].Location.X + NPC[tempBlockHit[2]].Location.Width + 2)
                                                                tempBlockHit[2] = 0;
                                                        }
                                                    }
                                                    if(Player[A].Mount == 2)
                                                    {
                                                        D = Player[A].Location.X - D;
                                                        for(C = 1; C <= numNPCs; C++)
                                                        {
                                                            if(NPC[C].standingOnPlayer == A)
                                                                NPC[C].Location.X = NPC[C].Location.X + D;
                                                        }
                                                        for(C = 1; C <= numPlayers; C++)
                                                        {
                                                            if(Player[C].StandingOnTempNPC == 56)
                                                                Player[C].Location.X = Player[C].Location.X + D;
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                // if the player collided on the left or right of some npcs then stop his movement
                if(tempHit2 == true)
                {
                    if(Player[A].Location.SpeedX + NPC[Player[A].StandingOnNPC].Location.SpeedX + tempHitSpeed > 0 && Player[A].Controls.Right == true)
                        Player[A].Location.SpeedX = 0.2 * Player[A].Direction + tempHitSpeed;
                    else if(Player[A].Location.SpeedX + NPC[Player[A].StandingOnNPC].Location.SpeedX + tempHitSpeed < 0 && Player[A].Controls.Left == true)
                        Player[A].Location.SpeedX = 0.2 * Player[A].Direction + tempHitSpeed;
                    else
                    {
                        if(Player[A].Controls.Right == true || Player[A].Controls.Left == true)
                            Player[A].Location.SpeedX = -NPC[Player[A].StandingOnNPC].Location.SpeedX + 0.2 * Player[A].Direction;
                        else
                            Player[A].Location.SpeedX = 0;
                    }
                }
                if(tempHit == true) // For multiple NPC hits
                {
                    if(Player[A].Character == 4 && (Player[A].State == 4 || Player[A].State == 5) && Player[A].SpinJump == false)
                        Player[A].DoubleJump = true;
                    Player[A].CanJump = true;
                    if(tempSpring == true)
                    {
                        Player[A].Jump = Physics.PlayerSpringJumpHeight;
                        if(Player[A].Character == 2)
                            Player[A].Jump = Player[A].Jump + 3;
                        if(Player[A].SpinJump == true)
                            Player[A].Jump = Player[A].Jump - 6;
                        Player[A].Location.SpeedY = Physics.PlayerJumpVelocity - 4;
                        if(Player[A].Wet > 0)
                            Player[A].Location.SpeedY = Player[A].Location.SpeedY * 0.3;
                    }
                    else
                    {
                        Player[A].Jump = Physics.PlayerNPCJumpHeight;
                        if(Player[A].Character == 2)
                            Player[A].Jump = Player[A].Jump + 3;
                        if(Player[A].SpinJump == true)
                            Player[A].Jump = Player[A].Jump - 6;
                        Player[A].Location.SpeedY = Physics.PlayerJumpVelocity;
                        if(Player[A].Wet > 0)
                            Player[A].Location.SpeedY = Player[A].Location.SpeedY * 0.3;
                    }
                    Player[A].Location.Y = tempLocation.Y;
                    if(tempShell == true)
                        NewEffect(132, newLoc(Player[A].Location.X + Player[A].Location.Width / 2.0 - EffectWidth[132] / 2.0, Player[A].Location.Y + Player[A].Location.Height - EffectHeight[132] / 2.0));
                    else if(tempSpring == false)
                        NewEffect(75, newLoc(Player[A].Location.X + Player[A].Location.Width / 2.0 - 16, Player[A].Location.Y + Player[A].Location.Height - 16));
                    else
                        tempSpring = false;
                    PlayerPush(A, 3);
                    if(Player[A].YoshiBlue == true)
                    {
                        Player[A].CanFly2 = true;
                        Player[A].FlyCount = 300;
                    }
                    if(spinKill == true)
                    {
                        Player[A].Jump = 0;
                        Player[A].Location.SpeedY = Physics.PlayerJumpVelocity; // * 0.5
                    }
                }



                // Find out which NPC to stand on

                // this code is for standing on moving NPCs.
                if(tempBlockHit[2] != 0)
                {
                    if(NPC[tempBlockHit[1]].Location.Y == NPC[tempBlockHit[2]].Location.Y)
                    {
                        C = NPC[tempBlockHit[1]].Location.X + NPC[tempBlockHit[1]].Location.Width * 0.5;
                        D = NPC[tempBlockHit[2]].Location.X + NPC[tempBlockHit[2]].Location.Width * 0.5;
                        C = C - (Player[A].Location.X + Player[A].Location.Width * 0.5);
                        D = D - (Player[A].Location.X + Player[A].Location.Width * 0.5);
                        if(C < 0)
                            C = -C;
                        if(D < 0)
                            D = -D;
                        if(C < D)
                            B = tempBlockHit[1];
                        else
                            B = tempBlockHit[2];
                    }
                    else
                    {
                        if(NPC[tempBlockHit[1]].Location.Y < NPC[tempBlockHit[2]].Location.Y)
                            B = tempBlockHit[1];
                        else
                            B = tempBlockHit[2];

                    }
                }
                else if(tempBlockHit[1] != 0)
                    B = tempBlockHit[1];
                else
                    B = 0;
                if(NPC[tempBlockHit[1]].Type >= 60 && NPC[tempBlockHit[1]].Type <= 66)
                    B = tempBlockHit[1];
                else if(NPC[tempBlockHit[2]].Type >= 60 && NPC[tempBlockHit[2]].Type <= 66)
                    B = tempBlockHit[2];
                if(NPC[B].Effect == 2)
                    B = 0;
                if(NPC[B].Projectile == true && NPCIsVeggie[NPC[B].Type] == true)
                    B = 0;

                // B is the number of the NPC that the player is standing on
                // .StandingOnNPC is the number of the NPC that the player was standing on last cycle
                // if B = 0 and .standingonnpc > 0 then the player was standing on something and is no longer standing on something


                if(B > 0 && Player[A].SpinJump == true)
                {
                    if(NPC[B].Type == 263)
                    {
                        Player[A].Location.SpeedY = Physics.PlayerJumpVelocity;
                        NPC[B].Multiplier = NPC[B].Multiplier + Player[A].Multiplier;
                        NPCHit(B, 3, B);
                        Player[A].Jump = 7;
                        if(Player[A].Character == 2)
                            Player[A].Jump = Player[A].Jump + 3;
                        if(Player[A].Controls.Down == true)
                        {
                            Player[A].Jump = 0;
                            Player[A].Location.SpeedY = Physics.PlayerJumpVelocity * 0.5;
                        }
                        B = 0;
                    }
                }

                if(Player[A].HoldingNPC == B) // cant hold an npc that you are standing on
                    B = 0;
                if(B == 0 && Player[A].StandingOnTempNPC > 0 && Player[A].Mount == 0)
                    Player[A].Location.SpeedX = Player[A].Location.SpeedX + (NPC[Player[A].StandingOnNPC].Location.SpeedX + NPC[Player[A].StandingOnNPC].BeltSpeed);
                else if(B > 0 && Player[A].StandingOnNPC == 0 && NPC[B].playerTemp == true && Player[A].Location.SpeedY >= 0)
                    Player[A].Location.SpeedX = Player[A].Location.SpeedX - (NPC[B].Location.SpeedX + NPC[B].BeltSpeed);

                if(movingBlock == true) // this is for when the player is standing on a moving block
                {
                    if(B > 0)
                    {
                        if(NPC[B].Type == 91)
                            movingBlock = false;
                        else
                            B = -A;
                    }
                    else
                        B = -A;
                }

                if(B != 0)
                {
                    if(Player[A].StandingOnNPC == 0)
                    {
                        if(Player[A].GroundPound == true)
                        {
                            numBlock = numBlock + 1;
                            Block[numBlock].Location.Y = NPC[B].Location.Y;
                            YoshiPound(A, numBlock, true);
                            Block[numBlock].Location.Y = 0;
                            numBlock = numBlock - 1;
                            Player[A].GroundPound = false;
                        }
                        else if(Player[A].YoshiYellow == true)
                        {
                            numBlock = numBlock + 1;
                            Block[numBlock].Location.Y = NPC[B].Location.Y;
                            YoshiPound(A, numBlock);
                            Block[numBlock].Location.Y = 0;
                            numBlock = numBlock - 1;
                        }
                    }
                    if(NPC[B].playerTemp == 0)
                        Player[A].StandingOnTempNPC = 0;
                    if(Player[A].Location.SpeedY >= 0)
                        Player[A].StandingOnNPC = B;
                    Player[A].Location.Y = NPC[B].Location.Y - Player[A].Location.Height;
                    if(NPC[B].Type == 46 || NPC[B].Type == 212)
                        NPC[B].Special2 = 1;
                    if(NPC[B].Type == 105)
                        NPC[B].Special = 1;
                    if(NPC[B].Type == 104 && Player[A].Location.SpeedY > 0)
                        NPC[B].Direction = 1;
                    if(NPC[B].Type == 190 && NPC[B].Special == 0)
                    {
                        NPC[B].Special = 1;
                        SkullRide(B);
                    }
                    if(NPC[B].Type == 57)
                        Player[A].Location.SpeedY = 0;


                    if(NPC[B].Type == 56 && Player[A].Controls.Down == true && Player[A].Mount == 0 && NPC[B].playerTemp == false && Player[A].DuckRelease == true && (Player[A].HoldingNPC == 0 || Player[A].Character == 5))
                    {
                        UnDuck(A);
                        Player[A].Location = NPC[B].Location;
                        Player[A].Mount = 2;
                        NPC[B].Killed = 9;
                        Player[A].HoldingNPC = 0;
                        Player[A].StandingOnNPC = 0;
                        PlaySound(2);
                        for(C = 1; C <= numPlayers; C++)
                        {
                            if(Player[C].StandingOnNPC == B)
                                Player[C].StandingOnTempNPC = 56;
                        }
                    }
                    else if(Player[A].Mount == 2)
                    {
                        Player[A].StandingOnNPC = 0;
                        if(Player[A].Location.SpeedY > 4 + NPC[B].Location.SpeedY)
                            PlaySound(37);
                        Player[A].Location.SpeedY = NPC[B].Location.SpeedY;
                    }

                }
                else if(Player[A].Mount == 1 && Player[A].Jump == 0)
                {
                    if(Player[A].StandingOnNPC != 0)
                    {
                        if(Player[A].Location.X > NPC[Player[A].StandingOnNPC].Location.X + NPC[Player[A].StandingOnNPC].Location.Width || Player[A].Location.X + Player[A].Location.Width < NPC[Player[A].StandingOnNPC].Location.X)
                        {
                            Player[A].StandingOnNPC = 0;
                            Player[A].StandingOnTempNPC = 0;
                            if(Player[A].Location.SpeedY > 4.1)
                            {
                                Player[A].Location.Y = Player[A].Location.Y - Player[A].Location.SpeedY;
                                Player[A].Location.SpeedY = NPC[Player[A].StandingOnNPC].Location.SpeedY;
                                if(Player[A].Location.SpeedY > Physics.PlayerTerminalVelocity)
                                    Player[A].Location.SpeedY = Physics.PlayerTerminalVelocity;
                                Player[A].Location.Y = Player[A].Location.Y + Player[A].Location.SpeedY;
                            }
                        }
                    }
                }
                else if(Player[A].Mount == 1 && Player[A].Jump > 0)
                {
                    if(B == 0 && Player[A].StandingOnTempNPC > 0)
                        Player[A].Location.SpeedX = Player[A].Location.SpeedX + (NPC[Player[A].StandingOnNPC].Location.SpeedX + NPC[Player[A].StandingOnNPC].BeltSpeed);
                    else if(B > 0 && Player[A].StandingOnNPC == 0 && NPC[B].playerTemp == true)
                        Player[A].Location.SpeedX = Player[A].Location.SpeedX - (NPC[B].Location.SpeedX + NPC[B].BeltSpeed);
                    Player[A].StandingOnNPC = 0;
                    Player[A].StandingOnTempNPC = 0;
                }
                else
                {
                    if(Player[A].StandingOnNPC != 0)
                    {

                        if(Player[A].StandingOnNPC < 0)
                            Player[A].Location.SpeedX = Player[A].Location.SpeedX + NPC[Player[A].StandingOnNPC].Location.SpeedX;
                        Player[A].Location.Y = Player[A].Location.Y - Player[A].Location.SpeedY;
                        Player[A].Location.SpeedY = NPC[Player[A].StandingOnNPC].Location.SpeedY;
                        if(FreezeNPCs == true)
                            Player[A].Location.SpeedY = 0;
                        if(Player[A].Location.SpeedY > Physics.PlayerTerminalVelocity)
                            Player[A].Location.SpeedY = Physics.PlayerTerminalVelocity;
                        Player[A].Location.Y = Player[A].Location.Y + Player[A].Location.SpeedY;
                    }
                    Player[A].StandingOnNPC = 0;
                    Player[A].StandingOnTempNPC = 0;
                }

                if(Player[A].StandingOnNPC > 0 && Player[A].Mount == 0) // driving stuff
                {
                    if(NPC[Player[A].StandingOnNPC].Type == 290)
                    {
                        Player[A].Driving = true;
                        Player[A].Location.X = NPC[Player[A].StandingOnNPC].Location.X + NPC[Player[A].StandingOnNPC].Location.Width / 2.0 - Player[A].Location.Width / 2.0;
                        Player[A].Direction = NPC[Player[A].StandingOnNPC].DefaultDirection;
                    }
                }

                if((Player[A].Location.SpeedY == 0 || Player[A].StandingOnNPC != 0 || Player[A].Slope > 0) && Player[A].Slide == false && FreezeNPCs == false)
                    Player[A].Multiplier = 0;
                if(Player[A].Mount == 2)
                    Player[A].Multiplier = 0;
                PlayerCollide(A);

                // Talk to NPC
                if(MessageNPC > 0)
                {
                    MessageText = NPC[MessageNPC].Text;
                    PauseGame(A);
                    MessageText = "";
                    if(NPC[MessageNPC].TriggerTalk != "")
                        ProcEvent(NPC[MessageNPC].TriggerTalk);
                    MessageNPC = 0;
                }
                YoshiEatCode(A);

                // pinch code
                if(GodMode == false)
                {
                    if(((Player[A].Pinched1 > 0 && Player[A].Pinched3 > 0) || (Player[A].Pinched2 > 0 && Player[A].Pinched4 > 0)) && Player[A].NPCPinched > 0 && Player[A].Mount != 2)
                    {
                        if(Player[A].Mount != 2)
                            Player[A].Mount = 0;
                        Player[A].State = 1;
                        Player[A].Immune = 0;
                        Player[A].Immune2 = false;
                        PlayerHurt(A);
                    }
                }

                SuperWarp(A); // this sub checks warps

                // shell surf
                if(Player[A].ShellSurf == true && Player[A].StandingOnNPC != 0)
                {
                    Player[A].Location.X = NPC[Player[A].StandingOnNPC].Location.X + NPC[Player[A].StandingOnNPC].Location.Width / 2.0 - Player[A].Location.Width / 2.0;
                    Player[A].Location.SpeedX = 0; // 1 * .Direction
                    if(NPC[Player[A].StandingOnNPC].Location.SpeedX == 0)
                        Player[A].ShellSurf = false;
                }

                // Check edge of screen
                if(LevelWrap[Player[A].Section] == false && LevelMacro == 0)
                {
                    if(ScreenType == 3)
                    {
                        if(Player[A].Location.X < -vScreenX[1])
                        {
                            Player[A].Location.X = -vScreenX[1] + 1;
                            Player[A].Location.SpeedX = 4;
                        }
                        else if(Player[A].Location.X > -vScreenX[1] + frmMain.ScaleWidth - Player[A].Location.Width)
                        {
                            Player[A].Location.X = -vScreenX[1] + frmMain.ScaleWidth - Player[A].Location.Width - 1;
                            Player[A].Location.SpeedX = -4;
                        }
                    }
                }
                if(Player[A].Location.Y > level[Player[A].Section].Height + 64)
                    PlayerDead(A);
                if(NPCIsAShell[NPC[Player[A].StandingOnNPC].Type] == false)
                    Player[A].ShellSurf = false;

                PlayerGrabCode(A, DontResetGrabTime); // Player holding NPC code **GRAB CODE**

                if(Player[A].Controls.Run == false && Player[A].Controls.AltRun == false)
                    Player[A].RunRelease = true;
                else
                    Player[A].RunRelease = false;

                if(Player[A].Controls.Jump == false && Player[A].Controls.AltJump == false)
                    Player[A].JumpRelease = true;
                else
                    Player[A].JumpRelease = false;

                PlayerFrame(A); // Update players frames
                Player[A].StandUp = false; // Fixes a block collision bug
                Player[A].StandUp2 = false;
                if(Player[A].ForceHitSpot3 == true)
                    Player[A].StandUp = true;
                Player[A].ForceHitSpot3 = false;
                if(Player[A].ForceHold > 0)
                    Player[A].ForceHold = Player[A].ForceHold - 1;
            }
            else // Player special effects
                PlayerEffects(A);
//            if(nPlay.Online == true)
//            {
//                if(A == nPlay.MySlot + 1)
//                {
//                    if(Player[A].HoldingNPC != oldGrab)
//                    {
//                        if(Player[A].HoldingNPC > 0)
//                        {
//                            NPC[Player[A].HoldingNPC].NetTimeout = 10;
//                            C = Player[A].HoldingNPC;
//                            Netplay::sendData "K" + std::to_string(C) + "|" + NPC[C].Type + "|" + NPC[C].Location.X + "|" + NPC[C].Location.Y + "|" + std::to_string(NPC[C].Location.Width) + "|" + std::to_string(NPC[C].Location.Height) + "|" + NPC[C].Location.SpeedX + "|" + NPC[C].Location.SpeedY + "|" + NPC[C].Section + "|" + NPC[C].TimeLeft + "|" + NPC[C].Direction + "|" + std::to_string(static_cast<int>(floor(static_cast<double>(NPC[C].Projectile)))) + "|" + NPC[C].Special + "|" + NPC[C].Special2 + "|" + NPC[C].Special3 + "|" + NPC[C].Special4 + "|" + NPC[C].Special5 + "|" + NPC[C].Effect + LB + "1n" + std::to_string(A) + "|" + Player[A].HoldingNPC + "|" + NPC[Player[A].HoldingNPC].Type + LB;
//                        }
//                        else
//                        {
//                            C = oldGrab;
//                            Netplay::sendData Netplay::PutPlayerControls(nPlay.MySlot) + "K" + std::to_string(C) + "|" + NPC[C].Type + "|" + NPC[C].Location.X + "|" + NPC[C].Location.Y + "|" + std::to_string(NPC[C].Location.Width) + "|" + std::to_string(NPC[C].Location.Height) + "|" + NPC[C].Location.SpeedX + "|" + NPC[C].Location.SpeedY + "|" + NPC[C].Section + "|" + NPC[C].TimeLeft + "|" + NPC[C].Direction + "|" + std::to_string(static_cast<int>(floor(static_cast<double>(NPC[C].Projectile)))) + "|" + NPC[C].Special + "|" + NPC[C].Special2 + "|" + NPC[C].Special3 + "|" + NPC[C].Special4 + "|" + NPC[C].Special5 + "|" + NPC[C].Effect + LB + "1o" + std::to_string(A) + "|" + std::to_string(oldGrab) + LB;
//                        }
//                    }
//                    else if(Player[A].HoldingNPC > 0)
//                    {
//                        C = Player[A].HoldingNPC;
//                        NPC[Player[A].HoldingNPC].NetTimeout = 10;
//                        if(nPlay.PlayerWaitCount % 3 == 0)
//                            Netplay::sendData "K" + std::to_string(C) + "|" + NPC[C].Type + "|" + NPC[C].Location.X + "|" + NPC[C].Location.Y + "|" + std::to_string(NPC[C].Location.Width) + "|" + std::to_string(NPC[C].Location.Height) + "|" + NPC[C].Location.SpeedX + "|" + NPC[C].Location.SpeedY + "|" + NPC[C].Section + "|" + NPC[C].TimeLeft + "|" + NPC[C].Direction + "|" + std::to_string(static_cast<int>(floor(static_cast<double>(NPC[C].Projectile)))) + "|" + NPC[C].Special + "|" + NPC[C].Special2 + "|" + NPC[C].Special3 + "|" + NPC[C].Special4 + "|" + NPC[C].Special5 + "|" + NPC[C].Effect + LB + "1n" + std::to_string(A) + "|" + Player[A].HoldingNPC + "|" + NPC[Player[A].HoldingNPC].Type + LB;
//                    }
//                }
//            }
        }
        if(Player[A].Controls.Down == true)
            Player[A].DuckRelease = false;
        else
            Player[A].DuckRelease = true;
    }
    C = 0;
    for(A = numNPCs; A >= 1; A--)
    {
        if(NPC[A].playerTemp == true)
        {
            for(B = 1; B <= numPlayers; B++)
            {
                if(Player[B].StandingOnNPC == A)
                    Player[B].StandingOnTempNPC = NPC[A].Type;
            }
            NPC[C] = NPC[A];
            KillNPC(A, 9);
        }
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
                numNPCs = numNPCs + 1;
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
                        numNPCs = numNPCs + 1;
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
        numNPCs = numNPCs + 1;
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

    SDL_Delay(500);
    Lives = Lives - 1;
    if(Lives >= 0.f)
    {
        LevelMacro = 0;
        LevelMacroCounter = 0;
        ClearLevel();
        if(RestartLevel == true)
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
    if(LevelSelect == true)
        return;
    oldSection = Player[A].Section;
    for(B = 0; B <= maxSections; B++)
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
    if(foundSection == false)
    {
        for(B = 0; B <= maxSections; B++)
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
    if(!(Player[A].Effect == 9))
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
    if(Player[A].Stoned == true)
    {
        Player[A].Frame = 0;
        Player[A].FrameCount = 0;
        if(Player[A].Location.SpeedX != 0)
        {
            if(Player[A].Location.SpeedY == 0 || Player[A].Slope > 0 || Player[A].StandingOnNPC != 0)
            {
                if(Player[A].SlideCounter <= 0)
                {
                    Player[A].SlideCounter = 2 + std::rand() % 2;
                    tempLocation.Y = Player[A].Location.Y + Player[A].Location.Height - 5;
                    tempLocation.X = Player[A].Location.X + Player[A].Location.Width / 2.0 - 4;
                    NewEffect(74, tempLocation, 1, 0, ShadowMode);
                }
            }
        }
        return;
    }
// sliding frames
    if(Player[A].Slide == true && (Player[A].Character == 1 || Player[A].Character == 2))
    {
        if(Player[A].Location.SpeedX != 0.0)
        {
            if(Player[A].Location.SpeedY == 0.0 || Player[A].Slope > 0 || Player[A].StandingOnNPC != 0)
            {
                if(Player[A].SlideCounter <= 0 && Player[A].SlideKill == true)
                {
                    Player[A].SlideCounter = 2 + std::rand() % 2;
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
        if(Player[A].Location.SpeedX != NPC[Player[A].VineNPC].Location.SpeedX ||
           Player[A].Location.SpeedY < NPC[Player[A].VineNPC].Location.SpeedY - 0.1) // Or .Location.SpeedY > 0.1 Then
        {
            Player[A].FrameCount = Player[A].FrameCount + 1;
            if(Player[A].FrameCount >= 8)
            {
                Player[A].Frame = Player[A].Frame + 1;
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
    if(LevelSelect == false && Player[A].Effect != 3)
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
    if((Player[A].SpinJump == true || Player[A].ShellSurf == true) && Player[A].Mount == 0)
    {
        if(Player[A].SpinFrame < 4 || Player[A].SpinFrame >= 9)
            Player[A].Direction = -1;
        else
            Player[A].Direction = 1;
        if(Player[A].ShellSurf == true)
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
                                        Player[A].SlideCounter = 2 + std::rand() % 2;
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
                                        Player[A].SlideCounter = 2 + std::rand() % 2;
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
                            Player[A].SlideCounter = 2 + std::rand() % 2;
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
                                        Player[A].SlideCounter = 2 + std::rand() % 2;
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
                                        Player[A].SlideCounter = 2 + std::rand() % 2;
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
                            Player[A].SlideCounter = 2 + std::rand() % 2;
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
                        UpdateGraphics();
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
    for(A = 1; A <= numNPCs; A++)
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
            NewEffect (80, newLoc(tailLoc.X + std::fmod(std::rand(), tailLoc.Width) - 4, tailLoc.Y + std::fmod(std::rand(), tailLoc.Height)), 1, 0, ShadowMode);
            Effect[numEffects].Location.SpeedX = (0.5 + std::rand() % 1) * Player[plr].Direction;
            Effect[numEffects].Location.SpeedY = std::rand() % 1 - 0.5;
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
    for(B = 1; B <= numNPCs; B++)
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
                    NPC[B].Type = 139 + static_cast<int>(floor(static_cast<double>(std::rand() % 9)));
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
                numNPCs = numNPCs + 1;
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

        for(B = 1; B <= numNPCs; B++)
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
                B = static_cast<int>(floor(static_cast<double>(std::rand() % 9)));
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
    int C = 0;
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
            numNPCs = numNPCs + 1;
            NPC[numNPCs] = blankNPC;
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

            for(B = 1; B <= numNPCs; B++)
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
                                numNPCs = numNPCs + 1;
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
                                if(NPC[numNPCs].Direction == 1)
                                    NPC[numNPCs].Frame = 2;
                            }
                            for(C = 1; C <= numNPCs; C++)
                            {
                                if(NPC[C].Type == 50 && NPC[C].Special == A && NPC[C].Special2 == B)
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
                    for(C = 1; C <= numNPCs; C++)
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
    if(UnderWater[Player[A].Section] == true)
        Player[A].Wet = 2;
    if(Player[A].Wet > 0)
    {
        Player[A].SpinJump = false;
        Player[A].WetFrame = true;
        Player[A].Slide = false;
    }
    else if(Player[A].WetFrame == true)
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
    for(B = 1; B <= numWater; B++)
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
        if(Player[A].Location.SpeedY < 0 && (Player[A].Controls.AltJump == true || Player[A].Controls.Jump == true) && Player[A].Controls.Down == false)
        {
            Player[A].Jump = 12;
            Player[A].Location.SpeedY = Physics.PlayerJumpVelocity;
        }
    }
    else if(Player[A].Wet == 2 && Player[A].Quicksand == 0)
    {
        if(std::rand() % 100 > 97)
        {
            if(Player[A].Direction == 1)
                tempLocation = newLoc(Player[A].Location.X + Player[A].Location.Width - std::rand() % 8, Player[A].Location.Y + 4 + std::rand() % 8, 8, 8);
            else
                tempLocation = newLoc(Player[A].Location.X - 8 + std::rand() % 8, Player[A].Location.Y + 4 + std::rand() % 8, 8, 8);
            if(UnderWater[Player[A].Section] == false)
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
    if(Player[A].Fairy == true)
           return;
// tanooki
    if(Player[A].Stoned == true && Player[A].Controls.Down == true && Player[A].StandingOnNPC == 0)
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
        if(Player[A].Mount == 0 && Player[A].State == 5 && Player[A].Controls.AltRun == true && Player[A].Bombs == 0)
        {
            if(Player[A].Stoned == false)
                Player[A].Effect = 500;
        }
        else if(Player[A].Stoned == true)
            Player[A].Effect = 500;
    }
    else
        Player[A].StonedCD = Player[A].StonedCD - 1;
    if(Player[A].Stoned == true)
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
                if(Player[A].Immune2 == true)
                    Player[A].Immune2 = false;
                else
                    Player[A].Immune2 = true;
            }
        }
    }
}

void PowerUps(int A)
{
    bool BoomOut = false;
    int B = 0;

    if(Player[A].Fairy == true)
    {
        Player[A].SwordPoke = 0;
        Player[A].FireBallCD = 0;
        Player[A].FireBallCD2 = 0;
        Player[A].TailCount = 0;
        return;
    }


    if(Player[A].State == 6 && Player[A].Character == 4 && Player[A].Controls.Run == true && Player[A].RunRelease == true)
    {
        for(B = 1; B <= numNPCs; B++)
        {
            if(NPC[B].Active == true)
            {
                if(NPC[B].Type == 292)
                {
                    if(NPC[B].Special5 == A)
                        BoomOut = true;
                }
            }
        }
    }

// Hammer Throw Code
        if(Player[A].Slide == false && Player[A].Vine == 0 && Player[A].State == 6 && Player[A].Duck == false && Player[A].Mount != 2 && Player[A].Mount != 3 && Player[A].HoldingNPC <= 0 && Player[A].Character != 5)
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
                        numNPCs = numNPCs + 1;
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
                        numNPCs = numNPCs + 1;
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
                        if(NPC[numNPCs].Special == 2)
                            NPC[numNPCs].Frame = 4;
                        if(NPC[numNPCs].Special == 3)
                            NPC[numNPCs].Frame = 8;
                        if(NPC[numNPCs].Special == 4)
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
                            if(NPC[numNPCs].Special == 2)
                                NPC[numNPCs].Location.SpeedX = NPC[numNPCs].Location.SpeedX * 0.85;
                            if(Player[A].Controls.Up == true)
                            {
                                if(Player[A].StandingOnNPC != 0)
                                    NPC[numNPCs].Location.SpeedY = -6 + NPC[Player[A].StandingOnNPC].Location.SpeedY * 0.1;
                                else
                                    NPC[numNPCs].Location.SpeedY = -6 + Player[A].Location.SpeedY * 0.1;
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
            numNPCs = numNPCs + 1;
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
    int B = 0;
    int C = 0;
    Location_t tempLocation;
    bool canWarp = false;

    if(Player[A].WarpCD <= 0 && Player[A].Mount != 2 && Player[A].GroundPound == false && Player[A].GroundPound2 == false)
    {
        for(B = 1; B <= numWarps; B++)
        {
            if(CheckCollision(Player[A].Location, Warp[B].Entrance) && Warp[B].Hidden == false)
            {
                Player[A].ShowWarp = B;
                canWarp = false;
                if(Warp[B].Direction == 1 && Player[A].Controls.Up == true)
                {
                    if(WarpCollision(Player[A].Location, B) == true)
                        canWarp = true;
                }
                else if(Warp[B].Direction == 2 && Player[A].Controls.Left == true)
                {
                    if(WarpCollision(Player[A].Location, B) == true)
                        canWarp = true;
                }
                else if(Warp[B].Direction == 3 && Player[A].Controls.Down == true)
                {
                    if(WarpCollision(Player[A].Location, B) == true)
                        canWarp = true;
                }
                else if(Warp[B].Direction == 4 && Player[A].Controls.Right == true)
                {
                    if(WarpCollision(Player[A].Location, B) == true)
                        canWarp = true;
                }
                else if(Warp[B].Effect == 0)
                    canWarp = true;
                if(Warp[B].LevelEnt == true)
                    canWarp = false;
                if(Warp[B].Stars > numStars && canWarp == true)
                {
                    if(Warp[B].Stars == 1)
                        MessageText = "You need 1 star to enter.";
                    else
                        MessageText = fmt::format_ne("You need {0} stars to enter.", Warp[B].Stars);
                    PauseGame(A);
                    MessageText = "";
                    canWarp = false;
                }

                if(canWarp == true)
                {
                    Player[A].Slide = false;
                    Player[A].Stoned = false;
                    if(Warp[B].Locked == true)
                    {
                        if(Player[A].HoldingNPC > 0 && NPC[Player[A].HoldingNPC].Type == 31)
                        {
                            NPC[Player[A].HoldingNPC].Killed = 9;
                            NewEffect(10, NPC[Player[A].HoldingNPC].Location);
                            Warp[B].Locked = false;
                            int tempVar = numBackground + numLocked;
                            for(C = numBackground; C <= tempVar; C++)
                            {
                                if(Background[C].Type == 98)
                                {
                                    if(CheckCollision(Warp[B].Entrance, Background[C].Location) == true)
                                    {
                                        Background[C].Layer = "";
                                        Background[C].Hidden = true;
                                    }
                                }
                            }
                        }
                        else if(Player[A].Mount == 3 && Player[A].YoshiNPC > 0 && NPC[Player[A].YoshiNPC].Type == 31)
                        {
                            NPC[Player[A].YoshiNPC].Killed = 9;
                            Player[A].YoshiNPC = 0;
                            Warp[B].Locked = false;
                            int tempVar2 = numBackground + numLocked;
                            for(C = numBackground; C <= tempVar2; C++)
                            {
                                if(Background[C].Type == 98)
                                {
                                    if(CheckCollision(Warp[B].Entrance, Background[C].Location) == true)
                                    {
                                        Background[C].Layer = "";
                                        Background[C].Hidden = true;
                                    }
                                }
                            }
                        }
                        else if(Player[A].HasKey == true)
                        {
                            Player[A].HasKey = false;
                            Warp[B].Locked = false;
                            int tempVar3 = numBackground + numLocked;
                            for(C = numBackground; C <= tempVar3; C++)
                            {
                                if(Background[C].Type == 98)
                                {
                                    if(CheckCollision(Warp[B].Entrance, Background[C].Location) == true)
                                    {
                                        Background[C].Layer = "";
                                        Background[C].Hidden = true;
                                    }
                                }
                            }
                        }
                        else
                            canWarp = false;
                    }
                }

                if(canWarp == true)
                {
                    UnDuck(A);
                    Player[A].YoshiTongueLength = 0;
                    Player[A].MountSpecial = 0;
                    Player[A].FrameCount = 0;
                    Player[A].TailCount = 0;
                    Player[A].CanFly = false;
                    Player[A].CanFly2 = false;
                    Player[A].RunCount = 0;
                    if(Warp[B].NoYoshi == true && Player[A].YoshiPlayer > 0)
                    {
                        YoshiSpit(A);
                    }
                    if(Warp[B].WarpNPC == false || (Player[A].Mount == 3 && (Player[A].YoshiNPC != 0 || Player[A].YoshiPlayer != 0) && Warp[B].NoYoshi == true))
                    {
                        if(Player[A].HoldingNPC > 0)
                        {
                            if(NPC[Player[A].HoldingNPC].Type == 29)
                            {
                                NPCHit(Player[A].HoldingNPC, 3, Player[A].HoldingNPC);
                            }
                        }
                        if(Player[A].Character == 3 ||
                          (Player[A].Character == 4 && Warp[B].Effect == 1 && Warp[B].Direction == 1))
                            NPC[Player[A].HoldingNPC].Location.Y = Warp[B].Entrance.Y;
                        Player[A].HoldingNPC = 0;
                        if(Player[A].YoshiNPC > 0)
                        {
                            YoshiSpit(A);
                        }
                    }
                    if(Player[A].HoldingNPC > 0)
                    {
                        if(NPC[Player[A].HoldingNPC].Type == 263) // can't bring ice through warps
                        {
                            NPC[Player[A].HoldingNPC].HoldingPlayer = 0;
                            Player[A].HoldingNPC = 0;
                        }
                    }
                    Player[A].StandingOnNPC = 0;
                    Player[A].Location.SpeedX = 0;
                    Player[A].Location.SpeedY = 0;
                    if(Warp[B].Effect == 0)
                    {
                        Player[A].Location.X = Warp[B].Exit.X + Warp[B].Exit.Width / 2.0 - Player[A].Location.Width / 2.0;
                        Player[A].Location.Y = Warp[B].Exit.Y + Warp[B].Exit.Height - Player[A].Location.Height - 0.1;
                        CheckSection(A);
                        Player[A].WarpCD = 50;
                        break;
                    }
                    else if(Warp[B].Effect == 1)
                    {
                        PlaySound(17);
                        Player[A].Effect = 3;
                        Player[A].Warp = B;
//                        if(nPlay.Online == true && A == nPlay.MySlot + 1)
//                            Netplay::sendData Netplay::PutPlayerLoc(nPlay.MySlot) + "1j" + std::to_string(A) + "|" + Player[A].Warp + LB;
                    }
                    else if(Warp[B].Effect == 2)
                    {
                        PlaySound(46);
                        Player[A].Effect = 7;
                        Player[A].Warp = B;
//                        if(nPlay.Online == true && A == nPlay.MySlot + 1)
//                            Netplay::sendData Netplay::PutPlayerLoc(nPlay.MySlot) + "1j" + std::to_string(A) + "|" + Player[A].Warp + LB;
                        Player[A].Location.X = Warp[Player[A].Warp].Entrance.X + Warp[Player[A].Warp].Entrance.Width / 2.0 - Player[A].Location.Width / 2.0;
                        Player[A].Location.Y = Warp[Player[A].Warp].Entrance.Y + Warp[Player[A].Warp].Entrance.Height - Player[A].Location.Height;
                        for(C = 1; C <= numBackground; C++)
                        {
                            if((CheckCollision(Warp[B].Entrance, Background[C].Location) | CheckCollision(Warp[B].Exit, Background[C].Location)) != 0)
                            {
                                if(Background[C].Type == 88)
                                    NewEffect(54, Background[C].Location);
                                else if(Background[C].Type == 87)
                                    NewEffect(55, Background[C].Location);
                                else if(Background[C].Type == 107)
                                    NewEffect(59, Background[C].Location);
                                else if(Background[C].Type == 141)
                                {
                                    tempLocation = Background[C].Location;
                                    tempLocation.X = tempLocation.X + tempLocation.Width / 2.0;
                                    tempLocation.Width = 104;
                                    tempLocation.X = tempLocation.X - tempLocation.Width / 2.0;
                                    NewEffect(103, tempLocation);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if(Player[A].Mount != 2)
        Player[A].WarpCD = Player[A].WarpCD - 1;
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
                    else if(static_cast<int>(floor(static_cast<double>(std::rand() % 2))) == 1)
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
                            B = static_cast<int>(floor(static_cast<double>(std::rand() % 9)));
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
                        numNPCs = numNPCs + 1;
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
                            NPC[Player[A].HoldingNPC].Location.X = Player[A].Location.X + Player[A].Location.Width / 2.0 - NPC[Player[A].HoldingNPC].Location.Width / 2.0 + std::rand() % 4 - 2;
                            NPC[Player[A].HoldingNPC].Location.Y = Player[A].Location.Y - NPC[Player[A].HoldingNPC].Location.Height - 4 + std::rand() % 4 - 2;
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
                    numNPCs = numNPCs + 1;
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
        for(B = 1; B <= 100; B++)
        {
            if(Layer[B].Name != "")
            {
                if(Layer[B].Name == NPC[LayerNPC].AttLayer)
                {
                    if(NPC[LayerNPC].Location.X - lyrX == 0 && NPC[LayerNPC].Location.Y - lyrY == 0)
                    {
                        if(Layer[B].SpeedX != 0 || Layer[B].SpeedY != 0)
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
                    Player[A].SlideCounter = 2 + std::rand() % 2;
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
                Player[A].SlideCounter = 2 + std::rand() % 2;
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
        if(Player[A].Effect2 == 0)
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
        else if(Player[A].Effect2 == 1)
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
            if(Player[A].Duck == true)
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
                        Player[B].Location.SpeedY = std::rand() % 24 - 12;
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
            if(Warp[Player[A].Warp].level != "")
            {
                GoToLevel = Warp[Player[A].Warp].level;
                Player[A].Effect = 8;
                Player[A].Effect2 = 2970;
                ReturnWarp = Player[A].Warp;
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
        else if(Player[A].Effect2 == 2)
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
        else if(Player[A].Effect2 == 3)
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
                        Player[B].Location.SpeedY = std::rand() % 24 - 12;
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
            if(Warp[Player[A].Warp].level != "")
            {
                GoToLevel = Warp[Player[A].Warp].level;
                Player[A].Effect = 8;
                Player[A].Effect2 = 3000;
                ReturnWarp = Player[A].Warp;
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
                        Player[B].Location.SpeedY = std::rand() % 24 - 12;
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
            if(Player[A].Effect2 == 0)
            {
                Player[A].Effect = 0;
                Player[A].Effect2 = 0;
            }
        }
        else if(Player[A].Effect2 == 131)
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
            if(Player[A].Effect2 == 100)
            {
                Player[A].Effect = 0;
                Player[A].Effect2 = 0;
            }
        }
        else if(Player[A].Effect2 <= 300)
        {
            Player[A].Effect2 = Player[A].Effect2 - 1;
            if(Player[A].Effect2 == 200)
            {
                Player[A].Effect2 = 100;
                Player[A].Effect = 3;
            }
        }
        else if(Player[A].Effect2 <= 1000) // Start Wait
        {
            Player[A].Effect2 = Player[A].Effect2 - 1;
            if(Player[A].Effect2 == 900)
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
            if(Player[A].Effect2 == 1900)
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
            if(Player[A].Effect2 == 2920)
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
        if(Player[A].Duck == true && Player[A].Character != 5)
        {
            UnDuck(A);
            Player[A].Frame = 1;
        }
        Player[A].Effect2 = Player[A].Effect2 + 1;
        if(Player[A].Effect2 / 5 == static_cast<int>(floor(static_cast<double>(Player[A].Effect2 / 5))))
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
        if(Player[A].Effect2 / 5 == static_cast<int>(floor(static_cast<double>(Player[A].Effect2 / 5))))
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
        if(Player[A].Effect2 == 0)
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
        if(Player[A].Effect2 == 14)
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
        if(Player[A].Effect2 == 0)
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
        if(Player[A].Effect2 == 14)
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
        if(Player[A].Effect2 == 0)
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
            NewEffect(80, newLoc(Player[A].Location.X + std::fmod(std::rand(), Player[A].Location.Width + 8) - 8,
                                 Player[A].Location.Y + std::fmod(std::rand(), Player[A].Location.Height + 8) - 4), 1, 0, ShadowMode);
            Effect[numEffects].Location.SpeedX = std::rand() % 2 - 1;
            Effect[numEffects].Location.SpeedY = std::rand() % 2 - 1;
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
            if(B != A && (Player[B].Effect == 0 || B == Player[A].Effect2) && Player[B].Dead == false && Player[B].TimeToLive == 0 && CheckCollision(Player[A].Location, Player[B].Location) == true)
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
            D = Player[A].Effect2;
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
