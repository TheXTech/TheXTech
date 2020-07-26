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

#include "../globals.h"
#include "../player.h"
#include "../collision.h"
#include "../sound.h"
#include "../blocks.h"
#include "../npc.h"
#include "../effect.h"
#include "../layers.h"
#include "../editor.h"
#include "../game_main.h"

#include <Utils/maths.h>


void UpdatePlayer()
{
    int A = 0;
    int B = 0;
    float C = 0;
    float D = 0;
//    Controls_t blankControls;
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
    int tempBlockHit[3] = {0};
//    double tempBlockA[3] = {0};
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
    for(int tmpNumPlayers = numPlayers, A = 1; A <= tmpNumPlayers; A++)
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
//        tempBlockA[1] = 0; // Unused
//        tempBlockA[2] = 0;
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
        if(Player[A].Mount != 0 || Player[A].Stoned || Player[A].Fairy) // if .holdingnpc is -1 then the player can't grab anything. this stops the player from grabbing things while on a yoshi/shoe
            Player[A].HoldingNPC = -1;
        else if(Player[A].HoldingNPC == -1)
            Player[A].HoldingNPC = 0;
        if(Player[A].Controls.Drop && Player[A].DropRelease) // this is for the single player coop cheat code
        {
            if(SingleCoop > 0 && Player[A].Controls.Down)
                SwapCoop();
            else
                DropBonus(A);
        }
        // for dropping something from the container. this makes the player have to let go of the drop button before dropping something else
        Player[A].DropRelease = !Player[A].Controls.Drop;
//        if(Player[A].Controls.Drop == true)
//            Player[A].DropRelease = false;
//        else
//            Player[A].DropRelease = true;
        // Handle the death effecs
        if(Player[A].TimeToLive > 0)
        {
            Player[A].TimeToLive = Player[A].TimeToLive + 1;
            if(Player[A].TimeToLive >= 200 || ScreenType != 5)
            {
                B = CheckLiving();
                if(BattleMode && BattleLives[1] > 0 && BattleLives[2] > 0 && BattleWinner == 0)
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
        else if(Player[A].Dead)
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
        else if(!Player[A].Dead)
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
                        if(Player[A].Controls.Down && Player[A].DuckRelease && Player[A].CanPound)
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
                if(Player[A].GroundPound)
                {
                    if(!Player[A].CanPound && Player[A].Location.SpeedY < 0)
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
                    if(Player[A].Location.SpeedY < -5 && ((Player[A].Jump < 15 && Player[A].Jump != 0) || Player[A].CanFly))
                        Player[A].CanPound = true;
                    if(Player[A].GroundPound2)
                    {
                        Player[A].Location.SpeedY = -4;
                        Player[A].StandingOnNPC = 0;
                        Player[A].GroundPound2 = false;
                    }
                }

                SizeCheck(A); // check that the player is the correct size for it's character/state/mount and set it if not

                if(Player[A].Stoned) // stop the player from climbing/spinning/jumping when in tanooki statue form
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
                if(Player[A].Slope > 0 && Player[A].Controls.Down &&
                   Player[A].Mount == 0 && Player[A].HoldingNPC == 0 &&
                   !(Player[A].Character == 3 || Player[A].Character == 4 || Player[A].Character == 5) &&
                   Player[A].GrabTime == 0)
                {
                    if(Player[A].Duck)
                        UnDuck(A);
                    Player[A].Slide = true;
                }
                else if(Player[A].Location.SpeedX == 0.0)
                    Player[A].Slide = false;
                if(Player[A].Mount > 0 || Player[A].HoldingNPC > 0)
                    Player[A].Slide = false;

                // unduck a player that should be able to duck
                if(Player[A].Duck && (Player[A].Character == 1 || Player[A].Character == 2) && Player[A].State == 1 && (Player[A].Mount == 0 || Player[A].Mount == 2))
                    UnDuck(A);

                if(GameMenu && !Player[A].SpinJump) // force the player to look right when on the game menu
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
                    else if(!Player[A].Slide)
                        speedVar = (float)(1 + (Block[Player[A].Slope].Location.Height / Block[Player[A].Slope].Location.Width * 0.5) * 0.5);
                }
                if(Player[A].Stoned) // if statue form reset to normal
                    speedVar = 1;
                if(Player[A].Character == 3)
                    speedVar = (speedVar * 0.93f);
                if(Player[A].Character == 4)
                    speedVar = (speedVar * 1.07f);

                // modify speedvar to slow the player down under water
                if(Player[A].Wet > 0)
                {
                    if(Player[A].Location.SpeedY == 0.0 || Player[A].Slope > 0 || Player[A].StandingOnNPC != 0)
                        speedVar = (float)(speedVar * 0.25f); // if walking go really slow
                    else
                        speedVar = (float)(speedVar * 0.5f); // if swimming go slower faster the walking
                }

                if(Player[A].Slide) // Code used to move the player while sliding down a slope
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
                    else if(Player[A].Location.SpeedY == 0.0 || Player[A].StandingOnNPC != 0)
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
                    if(Player[A].Controls.Jump || Player[A].Controls.AltJump)
                        Player[A].Slide = false;

                    // if not sliding and in the clown car
                }
                else if(Player[A].Mount == 2)
                {
                    if(!Player[A].Controls.Jump)
                        Player[A].CanJump = true;
                    if(Player[A].Controls.AltJump && Player[A].CanAltJump) // Jump out of the Clown Car
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
                            if(!Block[B].Invis && !BlockIsSizable[Block[B].Type] && !BlockOnlyHitspot1[Block[B].Type] &&
                               !BlockNoClipping[Block[B].Type] && !Block[B].Hidden)
                            {
                                if(CheckCollision(tempLocation, Block[B].Location))
                                {
                                    tempBool = false;
                                    PlaySound(3);
                                }
                            }
                        }

                        for(int numNPCsMax2 = numNPCs, B = 1; B <= numNPCsMax2; B++)
                        {
                            if(NPCIsABlock[NPC[B].Type] && !NPCStandsOnPlayer[NPC[B].Type] && NPC[B].Active && NPC[B].Type != 56)
                            {
                                if(CheckCollision(tempLocation, NPC[B].Location))
                                {
                                    tempBool = false;
                                    PlaySound(3);
                                }
                            }
                        }

                        if(tempBool)
                        {
                            Player[A].CanJump = false;
                            PlaySound(1); // Jump sound
                            PlaySound(35);
                            Player[A].Jump = Physics.PlayerJumpHeight;
                            if(Player[A].Character == 2)
                                Player[A].Jump = Player[A].Jump + 3;
                            if(Player[A].SpinJump)
                                Player[A].Jump = Player[A].Jump - 6;
                            Player[A].Mount = 0;
                            numNPCs += 1;
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
                    }
                }
                else if(Player[A].Driving) // driving
                {
                    if(Player[A].Duck)
                        UnDuck(A);
                    Player[A].Driving = false;
                    if(Player[A].StandingOnNPC > 0)
                    {
                        NPC[Player[A].StandingOnNPC].Special4 = 1;
                        if(Player[A].Controls.Left)
                            NPC[Player[A].StandingOnNPC].Special5 = -1;
                        else if(Player[A].Controls.Right)
                            NPC[Player[A].StandingOnNPC].Special5 = 1;
                        else
                            NPC[Player[A].StandingOnNPC].Special5 = 0;
                        if(Player[A].Controls.Up)
                            NPC[Player[A].StandingOnNPC].Special6 = -1;
                        else if(Player[A].Controls.Down)
                            NPC[Player[A].StandingOnNPC].Special6 = 1;
                        else
                            NPC[Player[A].StandingOnNPC].Special6 = 0;
                    }
                    Player[A].Location.SpeedX = 0;

                }
                else if(Player[A].Fairy) // if a fairy
                {
                    if(Player[A].Controls.Right)
                    {
                        if(Player[A].Location.SpeedX < 3)
                            Player[A].Location.SpeedX = Player[A].Location.SpeedX + 0.15;
                        if(Player[A].Location.SpeedX < 0)
                            Player[A].Location.SpeedX = Player[A].Location.SpeedX + 0.1;
                    }
                    else if(Player[A].Controls.Left)
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
                }
                    // if the player is climbing a vine
                else if(Player[A].Vine > 0)
                {
                    if(Player[A].StandingOnNPC > 0 && !Player[A].Controls.Up)
                        Player[A].Vine = 0;
                    Player[A].CanFly = false;
                    Player[A].CanFly2 = false;
                    Player[A].RunCount = 0;
                    Player[A].SpinJump = false;
                    if(Player[A].Controls.Left)
                        Player[A].Location.SpeedX = -1.5;
                    else if(Player[A].Controls.Right)
                        Player[A].Location.SpeedX = 1.5;
                    else
                        Player[A].Location.SpeedX = 0;
                    if(Player[A].Controls.Up && Player[A].Vine > 2)
                        Player[A].Location.SpeedY = -2;
                    else if(Player[A].Controls.Down)
                        Player[A].Location.SpeedY = 3;
                    else
                        Player[A].Location.SpeedY = 0;
                    if(Player[A].VineBGO > 0)
                    {
                        Player[A].Location.SpeedX += Background[Player[A].VineBGO].Location.SpeedX;
                        Player[A].Location.SpeedY += Background[Player[A].VineBGO].Location.SpeedY;
                    }
                    else
                    {
                        Player[A].Location.SpeedX += NPC[Player[A].VineNPC].Location.SpeedX;
                        Player[A].Location.SpeedY += NPC[Player[A].VineNPC].Location.SpeedY;
                    }
                }
                else
                {

                    // if none of the above apply then the player controls like normal. remeber this is for the players X movement


                    // ducking for link
                    if(Player[A].Duck && Player[A].WetFrame)
                    {
                        if(Player[A].Location.SpeedY != 0.0 && Player[A].Slope == 0 && Player[A].StandingOnNPC == 0)
                            UnDuck(A);
                    }
                    // the following code controls the players ability to duck
                    if(!(Player[A].Character == 5 && ((Player[A].Location.SpeedY != 0.0 && Player[A].Slope == 0 && Player[A].StandingOnNPC == 0) || Player[A].FireBallCD != 0))) // Link can't duck/unduck in air
                    {
                        if(Player[A].Controls.Down && !Player[A].SpinJump &&
                           !Player[A].Stoned && Player[A].Vine == 0 && !Player[A].Slide &&
                           (Player[A].Slope == 0 || Player[A].Mount > 0 || Player[A].WetFrame ||
                            Player[A].Character >= 3 || Player[A].GrabTime > 0) &&
                           ((!Player[A].WetFrame || Player[A].Character >= 3) ||
                            Player[A].Location.SpeedY == 0.0 || Player[A].StandingOnNPC != 0 ||
                            Player[A].Slope != 0 || Player[A].Mount == 1) &&
                           !Player[A].Fairy && !Player[A].ShellSurf && !Player[A].Driving)
                        {
                            Player[A].Bumped = false;
                            if(Player[A].Mount != 2) // cant duck in the clown car
                            {
                                if(Player[A].Mount == 3) // duck on a yoshi
                                {
                                    if(!Player[A].Duck)
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
                                        if(!Player[A].Duck && Player[A].TailCount == 0) // Player ducks
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
                                        if(!Player[A].Duck && Player[A].TailCount == 0) // Player ducks
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
                            if(Player[A].Duck)
                                UnDuck(A);
                        }
                    }
                    C = 1;
                    // If .Character = 5 Then C = 0.94
                    if(Player[A].Character == 5)
                        C = 0.95F;
                    if(Player[A].Controls.Left &&
                       ((!Player[A].Duck && Player[A].GrabTime == 0) ||
                        (Player[A].Location.SpeedY != 0.0 && Player[A].StandingOnNPC == 0 && Player[A].Slope == 0) ||
                        Player[A].Mount == 1)
                    )
                    {
                        Player[A].Bumped = false;
                        if(Player[A].Controls.Run || Player[A].Location.SpeedX > -Physics.PlayerWalkSpeed * speedVar || Player[A].Character == 5)
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
                                if(SuperSpeed)
                                    Player[A].Location.SpeedX = Player[A].Location.SpeedX * 0.95;
                            }
                        }
                        if(SuperSpeed && Player[A].Controls.Run)
                            Player[A].Location.SpeedX -= 0.1;
                    }
                    else if(Player[A].Controls.Right && ((!Player[A].Duck && Player[A].GrabTime == 0) || (Player[A].Location.SpeedY != 0 && Player[A].StandingOnNPC == 0 && Player[A].Slope == 0) || Player[A].Mount == 1))
                    {
                        Player[A].Bumped = false;
                        if(Player[A].Controls.Run || Player[A].Location.SpeedX < Physics.PlayerWalkSpeed * speedVar || Player[A].Character == 5)
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
                                if(SuperSpeed)
                                    Player[A].Location.SpeedX = Player[A].Location.SpeedX * 0.95;
                            }
                        }
                        if(SuperSpeed && Player[A].Controls.Run)
                            Player[A].Location.SpeedX = Player[A].Location.SpeedX + 0.1;
                    }
                    else
                    {
                        if(Player[A].Location.SpeedY == 0.0 || Player[A].StandingOnNPC != 0 || Player[A].Slope > 0 || Player[A].WetFrame) // Only lose speed when not in the air
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
                            if(SuperSpeed)
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
                    if(Player[A].Controls.Run || Player[A].Character == 5)
                    {
                        if(Player[A].Location.SpeedX >= Physics.PlayerRunSpeed * speedVar)
                        {
                            if(!SuperSpeed)
                                Player[A].Location.SpeedX = Physics.PlayerRunSpeed * speedVar;
                        }
                        else if(Player[A].Location.SpeedX <= -Physics.PlayerRunSpeed * speedVar)
                        {
                            if(!SuperSpeed)
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

                    if(FlyForever && !Player[A].GroundPound)
                    {
                        if(Player[A].Mount == 3)
                            Player[A].YoshiBlue = true;
                        if((Player[A].State == 4 || Player[A].State == 5) || (Player[A].YoshiBlue && Player[A].Mount == 3) || (Player[A].Mount == 1 && Player[A].MountType == 3))
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
                        if( (Player[A].Location.SpeedY == 0.0 ||
                             Player[A].CanFly2 ||
                             Player[A].StandingOnNPC != 0 ||
                             Player[A].Slope > 0) &&
                            (std::abs(Player[A].Location.SpeedX) >= double(Physics.PlayerRunSpeed) ||
                            (Player[A].Character == 3 && std::abs(Player[A].Location.SpeedX) >= 5.58 - 0.001))) // Rounding error of SpeedX makes an evil here
                        {
                            Player[A].RunCount += 1;
                        }
                        else
                        {
                            if(!(std::abs(Player[A].Location.SpeedX) >= double(Physics.PlayerRunSpeed) ||
                                 (Player[A].Character == 3 && std::abs(Player[A].Location.SpeedX) >= 5.58 - 0.001)) )
                            {
                                Player[A].RunCount -= 0.3f;
                            }
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

                    if(Player[A].Location.SpeedY == 0.0 || Player[A].StandingOnNPC != 0 || Player[A].Slope > 0)
                        Player[A].FlyCount = 1;

                    if(Player[A].FlyCount > 1)
                        Player[A].FlyCount -= 1;
                    else if(Player[A].FlyCount == 1)
                    {
                        Player[A].CanFly2 = false;
                        Player[A].FlyCount = 0;
                    }
                }


                // stop link when stabbing
                if(Player[A].Character == 5)
                {
                    if(Player[A].FireBallCD > 0 && (Player[A].Location.SpeedY == 0.0 || Player[A].Slope != 0 || Player[A].StandingOnNPC != 0))
                    {
                        if(Player[A].Slippy)
                            Player[A].Location.SpeedX = Player[A].Location.SpeedX * 0.75;
                        else
                            Player[A].Location.SpeedX = 0;
                    }
                }
                // fairy stuff
                if(Player[A].FairyTime != 0 && Player[A].Fairy)
                {
                    if(dRand() * 10.0 > 9.0)
                    {
                        NewEffect(80,
                                  newLoc(Player[A].Location.X - 8 + dRand() * (Player[A].Location.Width + 16) - 4,
                                         Player[A].Location.Y - 8 + dRand() * (Player[A].Location.Height + 16)), 1, 0, ShadowMode);
                        Effect[numEffects].Location.SpeedX = std::fmod(dRand(), 0.5) - 0.25;
                        Effect[numEffects].Location.SpeedY = std::fmod(dRand(), 0.5) - 0.25;
                        Effect[numEffects].Frame = 1;
                    }
                    if(Player[A].FairyTime > 0)
                        Player[A].FairyTime = Player[A].FairyTime - 1;
                    if(Player[A].FairyTime != -1 && Player[A].FairyTime < 20 && Player[A].Character == 5)
                    {
                        for(int numNPCsMax4 = numNPCs, Bi = 1; Bi <= numNPCsMax4; Bi++)
                        {
                            if(NPC[Bi].Active && !NPC[Bi].Hidden && NPCIsAVine[NPC[Bi].Type])
                            {
                                tempLocation = NPC[Bi].Location;
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

                        for(B = 1; B <= numBackground; B++)
                        {
                            if(BackgroundFence[Background[B].Type] && !Background[B].Hidden)
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
                else if(Player[A].Fairy)
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
                if(Player[A].FairyCD != 0 && (Player[A].Location.SpeedY == 0.0 || Player[A].Slope != 0 || Player[A].StandingOnNPC != 0 || Player[A].WetFrame))
                    Player[A].FairyCD = Player[A].FairyCD - 1;


                if(Player[A].StandingOnNPC != 0 && !NPC[Player[A].StandingOnNPC].Pinched && !FreezeNPCs)
                {
                    if(Player[A].StandingOnNPC < 0)
                        NPC[Player[A].StandingOnNPC].Location = Block[NPC[Player[A].StandingOnNPC].Special].Location;
                    Player[A].Location.SpeedX = Player[A].Location.SpeedX + NPC[Player[A].StandingOnNPC].Location.SpeedX + NPC[Player[A].StandingOnNPC].BeltSpeed;
                }

                if(GameOutro) // force the player to walk a specific speed during the credits
                {
                    if(Player[A].Location.SpeedX < -2)
                        Player[A].Location.SpeedX = -2;
                    if(Player[A].Location.SpeedX > 2)
                        Player[A].Location.SpeedX = 2;
                }



                // slippy code
                if(Player[A].Slippy && (!Player[A].Slide || Player[A].Slope == 0))
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
                if(Block[Player[A].Slope].Location.SpeedY != 0.0 && Player[A].Slope != 0)
                    Player[A].Location.Y = Player[A].Location.Y + Block[Player[A].Slope].Location.SpeedY;

                if(Player[A].Fairy) // the player is a fairy
                {
                    Player[A].WetFrame = false;
                    Player[A].Wet = 0;
                    if(Player[A].FairyCD == 0)
                    {
                        if(Player[A].Controls.Jump || Player[A].Controls.AltJump || Player[A].Controls.Up)
                        {
                            Player[A].Location.SpeedY = Player[A].Location.SpeedY - 0.15;
                            if(Player[A].Location.SpeedY > 0)
                                Player[A].Location.SpeedY = Player[A].Location.SpeedY - 0.1;
                        }
                        else if(Player[A].Location.SpeedY < -0.1 || Player[A].Controls.Down)
                        {
                            if(Player[A].Location.SpeedY < 3)
                                Player[A].Location.SpeedY = Player[A].Location.SpeedY + double(Physics.PlayerGravity * 0.05f);
                            if(Player[A].Location.SpeedY < 0)
                                Player[A].Location.SpeedY = Player[A].Location.SpeedY + double(Physics.PlayerGravity * 0.05f);
                            Player[A].Location.SpeedY = Player[A].Location.SpeedY + double(Physics.PlayerGravity * 0.1f);
                            if(Player[A].Controls.Down)
                                Player[A].Location.SpeedY = Player[A].Location.SpeedY + 0.05;
                        }
                        else if(Player[A].Location.SpeedY > 0.1)
                            Player[A].Location.SpeedY = Player[A].Location.SpeedY - 0.15;
                        else
                            Player[A].Location.SpeedY = 0;
                    }
                    else
                    {
                        if(Player[A].Controls.Jump || Player[A].Controls.AltJump || Player[A].Controls.Up)
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
                            if(Player[A].Controls.Down)
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
                        if(Player[A].Controls.AltJump && Player[A].CanAltJump)
                        {
                            Player[A].CanJump = false;
                            PlaySound(1); // Jump sound
                            PlaySound(35);
                            Player[A].Location.SpeedY = Physics.PlayerJumpVelocity - tempSpeed;
                            Player[A].Jump = Physics.PlayerJumpHeight;
                            if(Player[A].Character == 2)
                                Player[A].Jump = Player[A].Jump + 3;
                            if(Player[A].SpinJump)
                                Player[A].Jump = Player[A].Jump - 6;
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
                            Player[A].Location.Y = Player[A].Location.Y + Player[A].Location.Height;
                            Player[A].Location.Height = Physics.PlayerHeight[Player[A].Character][Player[A].State];
                            Player[A].Location.Y = Player[A].Location.Y - Player[A].Location.Height;
                        }
                    }
                    else if(Player[A].Mount == 3)
                    {
                        if(Player[A].Controls.AltJump && Player[A].CanAltJump)
                        {
                            UnDuck(A);
                            if(Player[A].YoshiNPC > 0 || Player[A].YoshiPlayer > 0)
                                YoshiSpit(A);
                            Player[A].CanJump = false;
                            Player[A].StandingOnNPC = 0;
                            Player[A].Mount = 0;
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
                            // If ShadowMode = True Then .Shadow = True
                            Player[A].Location.Height = Physics.PlayerHeight[Player[A].Character][Player[A].State];
                        }
                    }

                    if(Player[A].Duck)
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
                        if(Player[A].Controls.Left || Player[A].Controls.Right)
                        {
                            if(Player[A].Location.SpeedY == Physics.PlayerGravity * 0.1 || Player[A].Slope > 0 || (Player[A].StandingOnNPC != 0 && Player[A].Location.Y + Player[A].Location.Height >= NPC[Player[A].StandingOnNPC].Location.Y - NPC[Player[A].StandingOnNPC].Location.SpeedY))
                            {
                                if(Player[A].Controls.Left && Player[A].Location.SpeedX - NPC[Player[A].StandingOnNPC].Location.SpeedX - NPC[Player[A].StandingOnNPC].BeltSpeed <= 0)
                                    Player[A].Location.SpeedY = -1.1 + NPC[Player[A].StandingOnNPC].Location.SpeedY;
                                else if(Player[A].Controls.Right && Player[A].Location.SpeedX - NPC[Player[A].StandingOnNPC].Location.SpeedX - NPC[Player[A].StandingOnNPC].BeltSpeed >= 0)
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


                        if(Player[A].Controls.Jump && Player[A].MountSpecial == 1 && Player[A].CanJump)
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
                            if((Player[A].Controls.Jump && Player[A].CanJump) ||
                               (Player[A].Controls.AltJump && Player[A].CanAltJump))
                            {
                                if(Player[A].Duck && Player[A].Mount != 1 && Player[A].Character <= 2)
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
                                if(Player[A].Controls.Down)
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
                                    if(Player[A].Controls.Up)
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

                    if(Player[A].Controls.Jump)
                        Player[A].CanJump = false;
                    else
                        Player[A].CanJump = true;
                    if(Player[A].Controls.AltJump)
                        Player[A].CanAltJump = false;
                    else
                        Player[A].CanAltJump = true;
                    if(Player[A].Controls.Up)
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
                            if(Player[A].Controls.Left || Player[A].Controls.Right)
                            {
                                if(Player[A].Location.SpeedY == 0 || Player[A].Slope > 0 || (Player[A].StandingOnNPC != 0 && Player[A].Location.Y + Player[A].Location.Height >= NPC[Player[A].StandingOnNPC].Location.Y - NPC[Player[A].StandingOnNPC].Location.SpeedY))
                                {
                                    if(Player[A].Controls.Left && Player[A].Location.SpeedX - NPC[Player[A].StandingOnNPC].Location.SpeedX - NPC[Player[A].StandingOnNPC].BeltSpeed <= 0)
                                        Player[A].Location.SpeedY = -4.1 + NPC[Player[A].StandingOnNPC].Location.SpeedY;
                                    else if(Player[A].Controls.Right && Player[A].Location.SpeedX - NPC[Player[A].StandingOnNPC].Location.SpeedX - NPC[Player[A].StandingOnNPC].BeltSpeed >= 0)
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
                            if(Player[A].Controls.Jump && Player[A].MountSpecial == 1 && Player[A].CanJump)
                            {
                                Player[A].Location.SpeedY = 0;
                                Player[A].StandUp = true;
                            }
                        }

                        if(Player[A].Mount == 1)
                        {
                            if(Player[A].Controls.AltJump && Player[A].CanAltJump) // check to see if the player should jump out of the shoe
                            {
                                UnDuck(A);
                                Player[A].CanJump = false;
                                PlaySound(1); // Jump sound
                                PlaySound(35);
                                Player[A].Location.SpeedY = Physics.PlayerJumpVelocity - tempSpeed;
                                Player[A].Jump = Physics.PlayerJumpHeight;
                                if(Player[A].Character == 2)
                                    Player[A].Jump = Player[A].Jump + 3;
                                if(Player[A].SpinJump)
                                    Player[A].Jump = Player[A].Jump - 6;
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
                                Player[A].Location.Y = Player[A].Location.Y + Player[A].Location.Height;
                                Player[A].Location.Height = Physics.PlayerHeight[Player[A].Character][Player[A].State];
                                Player[A].Location.Y = Player[A].Location.Y - Player[A].Location.Height;
                            }
                        }
                        else if(Player[A].Mount == 3)
                        {
                            if(Player[A].Controls.AltJump && Player[A].CanAltJump) // jump off of yoshi
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
                                if(Player[A].SpinJump)
                                    Player[A].Jump = Player[A].Jump - 6;
                                Player[A].Mount = 0;
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
                                Player[A].Location.Height = Physics.PlayerHeight[Player[A].Character][Player[A].State];
                            }
                        }

                        if((Player[A].Location.SpeedY == 0.0 || Player[A].Jump > 0 || Player[A].Vine > 0) && Player[A].FloatTime == 0) // princess float
                            Player[A].CanFloat = true;
                        if(Player[A].Wet > 0 || Player[A].WetFrame)
                            Player[A].CanFloat = false;
                        // handles the regular jump
                        if(Player[A].Controls.Jump || (Player[A].Controls.AltJump &&
                           ((Player[A].Character > 2 && Player[A].Character != 4) || Player[A].Quicksand > 0) &&
                           Player[A].CanAltJump))
                        {
                            if(Player[A].Location.SpeedX > 0)
                                tempSpeed = Player[A].Location.SpeedX * 0.2; // tempSpeed gives the player a height boost when jumping while running, based off his SpeedX
                            else
                                tempSpeed = -Player[A].Location.SpeedX * 0.2;
                            if(Player[A].ShellSurf) // this code modifies the jump based on him riding a shell
                            {
                                if(NPC[Player[A].StandingOnNPC].Location.SpeedY == 0 || NPC[Player[A].StandingOnNPC].Slope > 0)
                                {
                                    if(Player[A].CanJump)
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
                                    if(Player[A].SpinJump)
                                        Player[A].Jump = Player[A].Jump - 6;
                                    if(Player[A].StandingOnNPC > 0 && !FreezeNPCs)
                                    {
                                        if(NPC[Player[A].StandingOnNPC].Type != 91)
                                            Player[A].Location.SpeedX = Player[A].Location.SpeedX - NPC[Player[A].StandingOnNPC].Location.SpeedX;
                                    }
                                    Player[A].StandingOnNPC = 0; // the player can't stand on an NPC after jumping
                                    if(Player[A].CanFly) // let's the player fly if the conditions are met
                                    {
                                        Player[A].StandingOnNPC = 0;
                                        Player[A].Jump = 30;
                                        if(Player[A].Character == 2)
                                            Player[A].Jump = Player[A].Jump + 3;
                                        if(Player[A].SpinJump)
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
                                else if(Player[A].CanFly2)
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
                        if(Player[A].SpinJump || (Player[A].State != 4 && Player[A].State != 5) || Player[A].StandingOnNPC > 0 || Player[A].Slope > 0 || Player[A].Location.SpeedY == 0)
                            Player[A].DoubleJump = false;
                        // double jump code
                        if(Player[A].DoubleJump && Player[A].Jump == 0 && Player[A].Location.SpeedY != 0 && Player[A].Slope == 0 && Player[A].StandingOnNPC == 0 && Player[A].Wet == 0 && Player[A].Vine == 0 && Player[A].WetFrame == false && Player[A].Fairy == false && Player[A].CanFly2 == false)
                        {
                            if(Player[A].Controls.Jump && Player[A].JumpRelease)
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
                                    Effect[numEffects].Location.SpeedX = (dRand() * 3) - 1.5;
                                    Effect[numEffects].Location.SpeedY = (dRand() *0.5) + (1.5 - std::abs(Effect[numEffects].Location.SpeedX)) * 0.5;
                                    Effect[numEffects].Location.SpeedX = Effect[numEffects].Location.SpeedX - Player[A].Location.SpeedX * 0.2;
                                }
                            }
                        }



                        if(Player[A].NoShellKick > 0) // countdown for the next time the player kicks a turtle shell
                            Player[A].NoShellKick--;

                        if(Player[A].ShellSurf)
                        {
                            if(Player[A].Mount != 0)
                                Player[A].ShellSurf = false;

                            // FIXME: SOME DEAD CODE BECAUSE OF "1 == 2"
#if 0
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
#endif
                            {
                                if(dRand() * 10.0 > 3.0)
                                {
                                    tempLocation.Y = Player[A].Location.Y + Player[A].Location.Height - 2 + dRand() * (NPC[Player[A].StandingOnNPC].Location.Height - 8) + 4;
                                    tempLocation.X = Player[A].Location.X - 4 + dRand() * (Player[A].Location.Width - 8) + 4 - 8 * Player[A].Direction;
                                    NewEffect(80, tempLocation, 1, 0, ShadowMode);
                                    Effect[numEffects].Frame = iRand() % 3;
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
                        if(Player[A].Controls.AltJump && (Player[A].Character == 1 || Player[A].Character == 2 || Player[A].Character == 4))
                        {
                            if(Player[A].Location.SpeedX > 0)
                                tempSpeed = Player[A].Location.SpeedX * 0.2;
                            else
                                tempSpeed = -Player[A].Location.SpeedX * 0.2;
                            if((Player[A].Vine > 0 || Player[A].Location.SpeedY == 0 || Player[A].StandingOnNPC != 0 || Player[A].Slope > 0 || MultiHop == true) && Player[A].CanAltJump == true) // Player Jumped
                            {
                                if(!Player[A].Duck)
                                {
                                    Player[A].Slope = 0;
                                    Player[A].SpinFireDir = Player[A].Direction;
                                    Player[A].Location.SpeedY = Physics.PlayerJumpVelocity - tempSpeed;
                                    Player[A].Jump = Physics.PlayerJumpHeight;
                                    if(Player[A].Character == 2)
                                        Player[A].Jump = Player[A].Jump + 3;

                                    if(Player[A].StandingOnNPC > 0 && !FreezeNPCs)
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
                                    if(Player[A].Duck)
                                        UnDuck(A);

                                    if(Player[A].ShellSurf)
                                    {
                                        Player[A].ShellSurf = false;
                                        Player[A].Location.SpeedX = NPC[Player[A].StandingOnNPC].Location.SpeedX + NPC[Player[A].StandingOnNPC].BeltSpeed * 0.8;
                                        Player[A].Jump = 0;
                                    }

                                    Player[A].StandingOnNPC = 0;

                                    if(Player[A].CanFly)
                                    {
                                        Player[A].StandingOnNPC = 0;
                                        Player[A].Jump = 30;
                                        if(Player[A].Character == 2)
                                            Player[A].Jump = Player[A].Jump + 3;
                                        if(Player[A].SpinJump)
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
                            else if(Player[A].CanFly2)
                            {
                                if(Player[A].Location.SpeedY > Physics.PlayerJumpVelocity * 0.5)
                                {
                                    Player[A].Location.SpeedY = Player[A].Location.SpeedY - 1;
                                    Player[A].CanPound = true;
                                    if(Player[A].YoshiBlue)
                                        PlaySound(50);
                                }
                            }
                            // End If
                            Player[A].CanAltJump = false;

                        }
                        else
                            Player[A].CanAltJump = true;
                        // END ALT JUMP


                        if((Player[A].Location.SpeedY == 0 || Player[A].StandingOnNPC != 0 || Player[A].Slope > 0) && Player[A].SpinJump)
                        {
                            Player[A].SpinJump = false;
//                            if(nPlay.Online == true && nPlay.MySlot + 1 == A)
//                                Netplay::sendData Netplay::PutPlayerLoc(nPlay.MySlot) + "1m" + std::to_string(A) + LB;
                            Player[A].TailCount = 0;
                        }
                        if(Player[A].Mount > 0)
                            Player[A].SpinJump = false;
                        if(!Player[A].Controls.AltJump && !Player[A].Controls.Jump)
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
                                        if(Player[A].Controls.Jump || Player[A].Controls.AltJump)
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


                        if(Player[A].Character == 3 && Player[A].Wet == 0 && !Player[A].WetFrame)
                        {
                            if(Player[A].Location.SpeedY == 0 || Player[A].StandingOnNPC > 0 || Player[A].Slope > 0 || Player[A].CanFly2)
                                Player[A].CanFloat = true;
                            else if(Player[A].CanFloat)
                            {
                                if(Player[A].Jump == 0 && ((Player[A].Controls.Jump && Player[A].FloatRelease) ||
                                  (Player[A].Controls.AltJump && Player[A].Location.SpeedY > 0)))
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
                        if(Player[A].Character == 3 && Player[A].FlySparks)
                        {
                            if(Player[A].FloatTime == 0 && Player[A].Location.SpeedY >= 0)
                                Player[A].FlySparks = false;
                        }
                        if(Player[A].CanFloat)
                            Player[A].FloatTime = 0;
                        if(Player[A].FloatTime > 0 && Player[A].Character == 3)
                        {
                            if((Player[A].Controls.Jump || Player[A].Controls.AltJump) && Player[A].Vine == 0)
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
                        if((Player[A].State == 4 || Player[A].State == 5) || Player[A].YoshiBlue || (Player[A].Mount == 1 && Player[A].MountType == 3))
                        {
                            if((Player[A].Controls.Jump || Player[A].Controls.AltJump) && ((Player[A].Location.SpeedY > Physics.PlayerGravity * 5 && Player[A].Character != 3 && Player[A].Character != 4) || (Player[A].Location.SpeedY > Physics.PlayerGravity * 10 && Player[A].Character == 3) || (Player[A].Location.SpeedY > Physics.PlayerGravity * 7.5 && Player[A].Character == 4)) && Player[A].GroundPound == false && Player[A].Slope == 0 && Player[A].Character != 5)
                            {
                                if(!Player[A].ShellSurf)
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
                                                (!Player[A].YoshiBlue && (Player[A].CanFly || Player[A].CanFly2)) ||
                                                (Player[A].Mount == 3 && Player[A].CanFly2)
                                        )
                                 )
                                {
                                    if(dRand() * 10.0 > 9.0)
                                    {
                                        NewEffect(80, newLoc(Player[A].Location.X - 8 + dRand() * (Player[A].Location.Width + 16) - 4,
                                                             Player[A].Location.Y - 8 + dRand() * (Player[A].Location.Height + 16)), 1, 0, ShadowMode);
                                        Effect[numEffects].Location.SpeedX = (dRand() * 0.5) - 0.25;
                                        Effect[numEffects].Location.SpeedY = (dRand() * 0.5) - 0.25;
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
                    if(Player[A].Slide)
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

                        if(Player[A].FlyCount > 0 ||
                           ((Player[A].Controls.AltJump || (Player[A].Controls.Jump && Player[A].FloatRelease)) &&
                             Player[A].Location.SpeedY != Physics.PlayerGravity && Player[A].Slope == 0 &&
                             Player[A].StandingOnNPC == 0))
                        {
                            if(Player[A].FlyCount > 0)
                                Player[A].FairyCD = 0;
                            if(!Player[A].Fairy && Player[A].FairyCD == 0 && Player[A].Jump == 0 && Player[A].Wet == 0)
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

                        if(Player[A].Controls.Run && Player[A].RunRelease && (Player[A].FairyTime > 0 || Player[A].Effect == 8))
                        {
                            Player[A].FairyTime = 0;
                            Player[A].Controls.Run = false;
                        }

                        if(Player[A].Fairy)
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

                    if(Player[A].HasKey)
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
                                if(CheckCollision(Player[A].Location, tempLocation))
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
                        if(!(wasSlippy && !Player[A].Controls.Left && !Player[A].Controls.Right))
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

                                numNPCs++;
                                NPC[numNPCs] = NPC_t();
                                if(ShadowMode)
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
                                if(!Player[A].Duck)
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
                        if(Player[A].Duck)
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
                    if(Player[A].FireBallCD == 0 && Player[A].Wet == 0 && !Player[A].Fairy && Player[A].Mount == 0)
                    {
                        if(!Player[A].Duck && Player[A].Location.SpeedY < Physics.PlayerGravity && Player[A].StandingOnNPC == 0 && Player[A].Slope == 0 && Player[A].Controls.Up == false && Player[A].Stoned == false) // Link ducks when jumping
                        {
                            Player[A].SwordPoke = 0;
                            Player[A].Duck = true;
                            Player[A].Location.Y = Player[A].Location.Y + Player[A].Location.Height;
                            Player[A].Location.Height = Physics.PlayerDuckHeight[Player[A].Character][Player[A].State];
                            Player[A].Location.Y = Player[A].Location.Y - Player[A].Location.Height;
                        }
                        else if(Player[A].Duck && Player[A].Location.SpeedY > Physics.PlayerGravity && Player[A].StandingOnNPC == 0 && Player[A].Slope == 0) // Link stands when falling
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

                Player[A].FloatRelease = !Player[A].Controls.Jump;
//                if(Player[A].Controls.Jump == true)
//                    Player[A].FloatRelease = false;
//                else
//                    Player[A].FloatRelease = true;

                // Player interactions
                Player[A].Location.SpeedX = Player[A].Location.SpeedX + Player[A].Bumped2;
                Player[A].Location.X = Player[A].Location.X + Player[A].Bumped2;
                Player[A].Bumped2 = 0;
                if(Player[A].Mount == 0)
                    Player[A].YoshiYellow = false;

                // When it's true - don't check horizonta' section's bounds
                bool hBoundsHandled = false;

                // level wrap
                if(LevelWrap[Player[A].Section] || LevelVWrap[Player[A].Section])
                {
                    // horizontally
                    if(LevelWrap[Player[A].Section])
                    {
                        if(Player[A].Location.X + Player[A].Location.Width < level[Player[A].Section].X)
                            Player[A].Location.X = level[Player[A].Section].Width - 1;
                        else if(Player[A].Location.X > level[Player[A].Section].Width)
                            Player[A].Location.X = level[Player[A].Section].X - Player[A].Location.Width + 1;
                        hBoundsHandled = true;
                    }

                    // vertically
                    if(LevelVWrap[Player[A].Section])
                    {
                        if(Player[A].Location.Y + Player[A].Location.Height < level[Player[A].Section].Y)
                            Player[A].Location.Y = level[Player[A].Section].Height - 1;
                        else if(Player[A].Location.Y > level[Player[A].Section].Height)
                            Player[A].Location.Y = level[Player[A].Section].Y - Player[A].Location.Height + 1;
                    }
                }

                // Walk offscreen exit
                if(!hBoundsHandled && OffScreenExit[Player[A].Section])
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
                    hBoundsHandled = true;
                }

                if(!hBoundsHandled && LevelMacro != 1 && LevelMacro != 7 && !GameMenu)
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
                    if(dRand() * 4.0 > 3.0)
                    {
                        NewEffect(80,
                                  newLoc(Player[A].Location.X - 8 + dRand() * (Player[A].Location.Width + 16) - 4,
                                                  Player[A].Location.Y - 8 + dRand() * (Player[A].Location.Height + 16)),
                                                  1, 0, ShadowMode);
                        Effect[numEffects].Location.SpeedX = (dRand() * 0.5) - 0.25;
                        Effect[numEffects].Location.SpeedY = (dRand() * 0.5) - 0.25;
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

                if(Player[A].Character == 5 && Player[A].Duck && (Player[A].Location.SpeedY == Physics.PlayerGravity || Player[A].StandingOnNPC != 0 || Player[A].Slope != 0))
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

                                    if(!Block[B].Hidden)
                                    {
                                        HitSpot = FindRunningCollision(Player[A].Location, Block[B].Location); // this finds what part of the block the player collided

                                        if(BlockNoClipping[Block[B].Type]) // blocks that the player can't touch are forced to hitspot 0 (which means no collision)
                                            HitSpot = 0;

                                        if(BlockIsSizable[Block[B].Type] || BlockOnlyHitspot1[Block[B].Type]) // for sizable blocks, if the player didn't land on them from the top then he can walk through them
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
                                                   Block[B].Type != 598
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
                                        if(HitSpot == 5 && (Player[A].StandUp || NPC[Player[A].StandingOnNPC].Location.SpeedY < 0))
                                        {
                                            if(BlockSlope[Block[B].Type] == 0)
                                                HitSpot = 3;
                                        }

                                        // if the block is invisible and the player didn't hit it from below then the player won't collide with it
                                        if(Block[B].Invis)
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
                                        if(Block[B].Type == 457 && Player[A].Stoned)
                                        {
                                            HitSpot = 0;
                                            KillBlock(B);
                                        }

                                        // shadowmode is a cheat that allows the player to walk through walls
                                        if(ShadowMode && HitSpot != 1 && !(Block[B].Special > 0 && HitSpot == 3))
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
                                                if(BlockKills[Block[B].Type])
                                                {
                                                    if(!GodMode)
                                                        PlayerDead(A);
                                                }
                                                if(Player[A].Location.SpeedY == 0.0 ||
                                                   fEqual(float(Player[A].Location.SpeedY), Physics.PlayerGravity) || Player[A].Slope > 0 || Player[A].StandingOnNPC != 0)
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
                                                    if(Player[A].CanFly2)
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
                                                    if(!BlockIsSizable[Block[tempHit3].Type])
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

                                                if(tempHit2)
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

                                                        if(Player[A].GroundPound)
                                                        {
                                                            YoshiPound(A, tempHit3, true);
                                                            Player[A].GroundPound = false;
                                                        }
                                                        else if(Player[A].YoshiYellow)
                                                        {
                                                            if(oldSlope == 0)
                                                                YoshiPound(A, tempHit3);
                                                        }

                                                        Player[A].Location.Y = Block[B].Location.Y + (Block[B].Location.Height * Slope) - Player[A].Location.Height - 0.1;

                                                        if(Player[A].Location.SpeedY > Player[A].Location.SpeedX * (Block[B].Location.Height / static_cast<double>(Block[B].Location.Width)) * BlockSlope[Block[B].Type] || Player[A].Slide == false)
                                                        {
                                                            if(!Player[A].WetFrame)
                                                            {
                                                                C = Player[A].Location.SpeedX * (Block[B].Location.Height / static_cast<double>(Block[B].Location.Width)) * BlockSlope[Block[B].Type];
                                                                Player[A].Location.SpeedY = C;
                                                                if(Player[A].Location.SpeedY > 0 && !Player[A].Slide && Player[A].Mount != 1 && Player[A].Mount != 2)
                                                                    Player[A].Location.SpeedY = Player[A].Location.SpeedY * 4;
                                                            }
                                                        }

                                                        Player[A].Slope = B;
                                                        if(BlockSlope[Block[B].Type] == 1 && GameMenu && Player[A].Location.SpeedX >= 2)
                                                        {
                                                            if(Player[A].Mount == 0 && Player[A].HoldingNPC == 0 && Player[A].Character <= 2)
                                                            {
                                                                if(Player[A].Duck)
                                                                    UnDuck(A);
                                                                Player[A].Slide = true;
                                                            }
                                                        }



                                                        if(Player[A].Location.SpeedY < 0 && !Player[A].Slide && !SuperSpeed && !Player[A].Stoned)
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
                                        if(BlockKills[Block[B].Type] && BlockSlope[Block[B].Type] == 0 && !GodMode && !(Player[A].Mount == 1 && Player[A].MountType == 2))
                                        {
                                            if(Player[A].Location.Y + Player[A].Location.Height < Block[B].Location.Y + 6)
                                                HitSpot = 0;
                                        }

                                        // kill the player if touching a lava block
                                        if(BlockKills[Block[B].Type] && (HitSpot > 0 || Player[A].Slope == B))
                                        {
                                            if(!GodMode)
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
                                                    tempLocation.X = Player[A].Location.X - 4 + dRand() * (Player[A].Location.Width + 8) - 4;
                                                    NewEffect(74, tempLocation);
                                                }
                                            }
                                        }

                                        // if hitspot 5 with a sloped block then don't collide with it. the collision should have already been handled by the slope code above
                                        if(HitSpot == 5 && BlockSlope[Block[B].Type] != 0)
                                            HitSpot = 0;

                                        // shelsurfing code
                                        if(HitSpot > 1 && Player[A].ShellSurf)
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




                                        if((HitSpot == 1 || Player[A].Slope == B) && Block[B].Slippy)
                                            Player[A].Slippy = true;


                                        if(HitSpot == 5 && Player[A].Quicksand > 0) // fixes quicksand hitspot 3 bug
                                        {
                                            if(Player[A].Location.Y - Player[A].Location.SpeedY < Block[B].Location.Y + Block[B].Location.Height)
                                                HitSpot = 3;
                                        }

                                        if(HitSpot == 1) // landed on the block from the top V
                                        {
                                            if(Player[A].Fairy && (Player[A].FairyCD > 0 || Player[A].Location.SpeedY > 0))
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
                                                // Just a blank block :-P
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
                                            if(!Player[A].ForceHitSpot3 && !Player[A].StandUp)
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
                                                for(auto C = fBlock; C <= lBlock; C++)
                                                {
                                                    if(CheckCollision(tempLocation, Block[C].Location) && !Block[C].Hidden)
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

                                                if(tempBool)
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

                if(Player[A].Character == 5 && Player[A].Duck)
                {
                    Player[A].Location.Y = Player[A].Location.Y + Player[A].Location.Height;
                    Player[A].Location.Height = Physics.PlayerDuckHeight[Player[A].Character][Player[A].State];
                    Player[A].Location.Y = Player[A].Location.Y - Player[A].Location.Height;
                }


                // helps the player run down slopes at different angles
                if(Player[A].Slope == 0 && oldSlope > 0 && Player[A].Mount != 1 && Player[A].Mount != 2 && !Player[A].Slide)
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
                        if(Player[A].TailCount == 0 && Player[A].Controls.Down && Player[A].Controls.Run && Player[A].Mount == 0 && Player[A].Stoned == false && Player[A].HoldingNPC == 0 && (Player[A].GrabTime > 0 || Player[A].RunRelease))
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
                    if(tempHit2)
                    {
                        if(WalkingCollision(Player[A].Location, Block[tempHit3].Location))
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
                            if(Player[A].GroundPound)
                            {
                                YoshiPound(A, tempHit3, true);
                                Player[A].GroundPound = false;
                            }
                            else if(Player[A].YoshiYellow)
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

                            if(Block[tempHit3].Type == 55 && !FreezeNPCs) // Make the player jump if the block is bouncy
                            {
                                if(!Player[A].Slide)
                                    Player[A].Multiplier = 0;
                                BlockHit(tempHit3, true);
                                Player[A].Location.SpeedY = Physics.PlayerJumpVelocity;
                                PlaySound(3);
                                if(Player[A].Controls.Jump || Player[A].Controls.AltJump)
                                {
                                    PlaySound(1);
                                    Player[A].Jump = Physics.PlayerBlockJumpHeight;
                                    if(Player[A].Character == 2)
                                        Player[A].Jump = Player[A].Jump + 3;
                                    if(Player[A].SpinJump)
                                        Player[A].Jump = Player[A].Jump - 6;
                                }
                            }

                            if(Player[A].SpinJump && (Block[tempHit3].Type == 90 || Block[tempHit3].Type == 526) && Player[A].State > 1 && Block[tempHit3].Special == 0)
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

                                if(Player[A].Controls.Down)
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

                        if(Player[A].GroundPound)
                        {
                            YoshiPound(A, tempHit3, true);
                            Player[A].GroundPound = false;
                        }
                        else if(Player[A].YoshiYellow)
                            YoshiPound(A, tempHit3);

                        if(Player[A].Slope == 0 || Player[A].Slide)
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

                        if(Player[A].StandingOnNPC != 0 && !movingBlock)
                        {
                            Player[A].Location.SpeedY = 1;
                            if(!NPC[Player[A].StandingOnNPC].Pinched && !FreezeNPCs)
                                Player[A].Location.SpeedX = Player[A].Location.SpeedX - NPC[Player[A].StandingOnNPC].Location.SpeedX - NPC[Player[A].StandingOnNPC].BeltSpeed;
                            Player[A].StandingOnNPC = 0;
                        }
                        else if(movingBlock)
                        {
                            Player[A].Location.SpeedY = NPC[-A].Location.SpeedY + 1;
                            if(Player[A].Location.SpeedY < 0)
                                Player[A].Location.SpeedY = 0;
                        }
                        else
                        {
                            if(Player[A].Slope == 0 || Player[A].Slide)
                                Player[A].Location.SpeedY = 0;
                        }

                        if(Block[tempHit3].Type == 55 && !FreezeNPCs) // Make the player jump if the block is bouncy
                        {
                            BlockHit(tempHit3, true);
                            if(!Player[A].Slide)
                                Player[A].Multiplier = 0;
                            Player[A].Location.SpeedY = Physics.PlayerJumpVelocity;
                            PlaySound(3);
                            if(Player[A].Controls.Jump || Player[A].Controls.AltJump)
                            {
                                PlaySound(1);
                                Player[A].Jump = Physics.PlayerBlockJumpHeight;
                                if(Player[A].Character == 2)
                                    Player[A].Jump = Player[A].Jump + 3;
                                if(Player[A].SpinJump)
                                    Player[A].Jump = Player[A].Jump - 6;
                            }
                        }
                        if(Player[A].SpinJump && (Block[tempHit3].Type == 90 || Block[tempHit3].Type == 526) && Player[A].State > 1 && Block[tempHit3].Special == 0)
                        {
                            Player[A].Location.SpeedY = Physics.PlayerJumpVelocity;
                            Block[tempHit3].Kill = true;
                            iBlocks = iBlocks + 1;
                            iBlock[iBlocks] = tempHit3;
                            tempHit3 = 0;
                            Player[A].Jump = 7;
                            if(Player[A].Character == 2)
                                Player[A].Jump = Player[A].Jump + 3;
                            if(Player[A].Controls.Down)
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
                if(!tempHit && tempHit2)
                {
                    if(Player[A].Location.SpeedX + NPC[Player[A].StandingOnNPC].Location.SpeedX > 0 && Player[A].Controls.Right)
                    {
                        Player[A].Location.SpeedX = 0.2 * Player[A].Direction;
                        if(blockPushX > 0)
                            Player[A].Location.SpeedX = Player[A].Location.SpeedX + blockPushX;
                    }
                    else if(Player[A].Location.SpeedX + NPC[Player[A].StandingOnNPC].Location.SpeedX < 0 && Player[A].Controls.Left)
                    {
                        Player[A].Location.SpeedX = 0.2 * Player[A].Direction;
                        if(blockPushX < 0)
                            Player[A].Location.SpeedX = Player[A].Location.SpeedX + blockPushX;
                    }
                    else
                    {
                        if(Player[A].Controls.Right || Player[A].Controls.Left)
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
                    if(Player[A].Fairy)
                        Player[A].Location.SpeedY = 2;
                    if(Player[A].Vine > 0)
                        Player[A].Location.Y = Player[A].Location.Y + 0.1;
                    if(Player[A].Mount == 2)
                        Player[A].Location.SpeedY = 2;
                    if(Player[A].CanFly2)
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

                if(Player[A].Slide && oldSlope > 0 && Player[A].Slope == 0 && Player[A].Location.SpeedY < 0)
                {
                    if(Player[A].NoGravity == 0)
                        Player[A].NoGravity = static_cast<int>(floor(static_cast<double>(Player[A].Location.SpeedY / Physics.PlayerJumpVelocity * 8)));
                }
                else if(Player[A].Slope > 0 || oldSlope > 0 || !Player[A].Slide)
                    Player[A].NoGravity = 0;

//                if(Player[A].Slide)  // Simplified below
//                {
//                    if(Player[A].Location.SpeedX > 1 || Player[A].Location.SpeedX < -1)
//                        Player[A].SlideKill = true;
//                    else
//                        Player[A].SlideKill = false;
//                }
//                else
//                    Player[A].SlideKill = false;
                Player[A].SlideKill = Player[A].Slide && (std::abs(Player[A].Location.SpeedX) > 1);





                // Check NPC collisions
                if(Player[A].Vine > 0)
                    Player[A].Vine -= 1;
                tempBlockHit[1] = 0;
                tempBlockHit[2] = 0;
                tempHitSpeed = 0;
                spinKill = false;


                // check vine backgrounds
                for(B = 1; B <= numBackground; B++)
                {
                    if(BackgroundFence[Background[B].Type] && !Background[B].Hidden)
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
                                    if(Player[A].Immune == 0 && Player[A].Controls.Up)
                                    {
                                        Player[A].FairyCD = 0;
                                        if(!Player[A].Fairy)
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
                                else if(!Player[A].Fairy && !Player[A].Stoned)
                                {
                                    if(Player[A].Mount == 0 && Player[A].HoldingNPC <= 0)
                                    {
                                        if(Player[A].Vine > 0)
                                        {
                                            if(Player[A].Duck)
                                                UnDuck(A);
                                            if(Player[A].Location.Y >= Background[B].Location.Y - 20 && Player[A].Vine < 2)
                                                Player[A].Vine = 2;
                                            if(Player[A].Location.Y >= Background[B].Location.Y - 18)
                                                Player[A].Vine = 3;
                                        }
                                        else if((Player[A].Controls.Up || (Player[A].Controls.Down &&
                                                                           Player[A].Location.SpeedY != 0 &&
                                                                           Player[A].StandingOnNPC == 0 &&
                                                                           Player[A].Slope <= 0)) && Player[A].Jump == 0)
                                        {
                                            if(Player[A].Duck)
                                                UnDuck(A);
                                            if(Player[A].Location.Y >= Background[B].Location.Y - 20 && Player[A].Vine < 2)
                                                Player[A].Vine = 2;
                                            if(Player[A].Location.Y >= Background[B].Location.Y - 18)
                                                Player[A].Vine = 3;
                                        }

                                        if(Player[A].Vine > 0)
                                        {
                                            Player[A].VineNPC = -1;
                                            Player[A].VineBGO = B;
                                        }
                                    }
                                } // !Fairy & !Stoned
                            } // Collide player and temp location
                        }// Collide player and BGO
                    } // Is BGO climbable and visible?
                } // Next A

                if(Player[A].StandingOnNPC != 0)
                {
                    if(!tempHit2)
                    {
                        if(!NPC[Player[A].StandingOnNPC].Pinched && !FreezeNPCs)
                            Player[A].Location.SpeedX = Player[A].Location.SpeedX - NPC[Player[A].StandingOnNPC].Location.SpeedX - NPC[Player[A].StandingOnNPC].BeltSpeed;
                    }
                }
                tempHit = false; // Used for JUMP detection
                tempHit2 = false;

                for(int tempNumNPCsMax = numNPCs, B = 1; B <= tempNumNPCsMax; B++)
                {
                    if(NPC[B].Active && NPC[B].Killed == 0 && NPC[B].Effect != 5 && NPC[B].Effect != 6)
                    {
                        // If Not (NPC(B).Type = 17 And NPC(B).CantHurt > 0) And Not (.Mount = 2 And NPC(B).Type = 56) And Not NPC(B).standingOnPlayer = A And Not NPC(B).Type = 197 And Not NPC(B).Type = 237 Then
                        if(!(Player[A].Mount == 2 && NPC[B].Type == 56) &&
                            NPC[B].standingOnPlayer != A &&
                            NPC[B].Type != 197 &&
                            NPC[B].Type != 237
                        )
                        {
                            if(NPC[B].HoldingPlayer == 0 || NPCIsABonus[NPC[B].Type] || (BattleMode && NPC[B].HoldingPlayer != A))
                            {
                                if(CheckCollision(Player[A].Location, NPC[B].Location))
                                {
                                    if((NPC[B].Type == 58 || NPC[B].Type == 21 || NPC[B].Type == 67 || NPC[B].Type == 68 || NPC[B].Type == 69 || NPC[B].Type == 70) && NPC[B].Projectile == true)
                                        PlayerHurt(A);
                                    if((Player[A].Mount == 1 || Player[A].Mount == 3 || Player[A].SpinJump || (Player[A].ShellSurf && NPCIsAShell[NPC[B].Type]) || (Player[A].Stoned && !NPCCanWalkOn[NPC[B].Type])) && !NPCMovesPlayer[NPC[B].Type])
                                        HitSpot = BootCollision(Player[A].Location, NPC[B].Location, NPCCanWalkOn[NPC[B].Type]); // find the hitspot for normal mario
                                    else
                                        HitSpot = EasyModeCollision(Player[A].Location, NPC[B].Location, NPCCanWalkOn[NPC[B].Type]); // find the hitspot when in a shoe or on a yoshi

                                    if(!NPC[B].Inert)
                                    {
                                        // battlemode stuff
                                        if(NPC[B].Type == 13 || NPC[B].Type == 171 || NPC[B].Type == 265 || NPC[B].Type == 266 || NPC[B].Type == 108 || NPC[B].Type == 291 || NPC[B].Type == 292)
                                        {
                                            if(BattleMode && NPC[B].CantHurtPlayer != A)
                                            {
                                                if(Player[A].State == 6 && Player[A].Duck && Player[A].Character != 5)
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
                                            if(!BattleMode)
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
                                        if((NPC[B].Type == 50 || NPC[B].Type == 30) && BattleMode && NPC[B].CantHurtPlayer != A)
                                            PlayerHurt(A);
                                        if((NPC[B].Type == 263 || NPC[B].Type == 96) && BattleMode &&
                                            NPC[B].CantHurtPlayer != A && NPC[B].Projectile != 0 && NPC[B].BattleOwner != A)
                                        {
                                            if(Player[A].Immune == 0 && NPC[B].Type == 96)
                                                NPC[B].Special2 = 1;
                                            PlayerHurt(A);
                                            HitSpot = 0;
                                        }
                                        if((NPCIsAShell[NPC[B].Type] || NPCIsVeggie[NPC[B].Type] ||
                                            NPC[B].Type == 263 || NPC[B].Type == 45) &&
                                            BattleMode && NPC[B].HoldingPlayer > 0 && NPC[B].HoldingPlayer != A)
                                        {
                                            if(Player[A].Immune == 0)
                                            {
                                                PlayerHurt(A);
                                                NPCHit(B, 5, B);
                                            }
                                        }
                                        if(NPCIsAParaTroopa[NPC[B].Type] && BattleMode && NPC[B].CantHurtPlayer == A)
                                            HitSpot = 0;
                                        if(BattleMode && NPCIsVeggie[NPC[B].Type] && NPC[B].Projectile != 0)
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

                                        if(BattleMode && NPC[B].HoldingPlayer > 0 && NPC[B].HoldingPlayer != A)
                                        {
                                            if(NPCWontHurt[NPC[B].Type])
                                                HitSpot = 0;
                                            else
                                                HitSpot = 5;
                                        }

                                        if(BattleMode && NPC[B].BattleOwner != A && NPC[B].Projectile != 0 && NPC[B].CantHurtPlayer != A)
                                        {
                                            if(NPC[B].Type == 134 || NPC[B].Type == 137 || NPC[B].Type == 154 || NPC[B].Type == 155 || NPC[B].Type == 156 || NPC[B].Type == 157 || NPC[B].Type == 166 || ((NPCIsAShell[NPC[B].Type] || NPC[B].Type == 45) && NPC[B].Location.SpeedX == 0))
                                            {
                                                if(NPCIsAShell[NPC[B].Type] && HitSpot == 1 && Player[A].SpinJump)
                                                {
                                                }
                                                else if(Player[A].Immune == 0)
                                                {
                                                    if(NPC[B].Type != 45 && !NPCIsAShell[NPC[B].Type])
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

                                    if(GameOutro)
                                        HitSpot = 0;
                                    if(NPC[B].Type == 30 && NPC[B].CantHurt > 0)
                                        HitSpot = 0;
                                    if(NPC[B].Type == 96 && HitSpot == 1)
                                        HitSpot = 0;
                                    if(NPC[B].Inert) // if the npc is friendly then you can't touch it
                                    {
                                        HitSpot = 0;
                                        if(NPC[B].Text != "" && Player[A].Controls.Up && !FreezeNPCs)
                                            MessageNPC = B;
                                    }
                                    if(Player[A].Stoned && HitSpot != 1) // if you are a statue then SLAM into the npc
                                    {
                                        if(Player[A].Location.SpeedX > 3 || Player[A].Location.SpeedX < -3)
                                            NPCHit(B, 3, B);
                                    }

                                    // the following code is for spin jumping and landing on things as yoshi/shoe
                                    if(Player[A].Mount == 1 || Player[A].Mount == 3 || Player[A].SpinJump ||
                                       (Player[A].Stoned && !NPCCanWalkOn[NPC[B].Type]))
                                    {
                                        if(HitSpot == 1)
                                        {
                                            if(Player[A].Mount == 1 || Player[A].Mount == 2 || Player[A].Stoned)
                                                NPCHit(B, 8, A);
                                            else if(!(NPC[B].Type == 245 || NPC[B].Type == 275 || NPC[B].Type == 8 || NPC[B].Type == 12 || NPC[B].Type == 36 || NPC[B].Type == 285 || NPC[B].Type == 286 || NPC[B].Type == 51 || NPC[B].Type == 52 || NPC[B].Type == 53 || NPC[B].Type == 54 || NPC[B].Type == 74 || NPC[B].Type == 93 || NPC[B].Type == 200 || NPC[B].Type == 205 || NPC[B].Type == 207 || NPC[B].Type == 201 || NPC[B].Type == 261 || NPC[B].Type == 270) && NPCCanWalkOn[NPC[B].Type] == false)
                                            {
                                                if(Player[A].Wet > 0 && (NPCIsCheep[NPC[B].Type] || NPC[B].Type == 231 || NPC[B].Type == 235))
                                                {
                                                }
                                                else
                                                    NPCHit(B, 8, A);
                                            }
                                            if(NPC[B].Killed == 8 || NPCIsCheep[NPC[B].Type] || NPC[B].Type == 179 ||
                                               NPC[B].Type == 37 || NPC[B].Type == 180 || NPC[B].Type == 38 ||
                                               NPC[B].Type == 42 || NPC[B].Type == 43 || NPC[B].Type == 44 ||
                                               NPC[B].Type == 8 || NPC[B].Type == 12 || NPC[B].Type == 36 ||
                                               NPC[B].Type == 51 || NPC[B].Type == 52 || NPC[B].Type == 53 ||
                                               NPC[B].Type == 54 || NPC[B].Type == 74 || NPC[B].Type == 93 ||
                                               NPC[B].Type == 200 || NPC[B].Type == 205 || NPC[B].Type == 207 ||
                                               NPC[B].Type == 201 || NPC[B].Type == 199 || NPC[B].Type == 245 ||
                                               NPC[B].Type == 256 || NPC[B].Type == 261 || NPC[B].Type == 275 ||
                                               NPC[B].Type == 285 || NPC[B].Type == 286 || NPC[B].Type == 270) // tap
                                            {
                                                if(NPC[B].Killed == 8 && Player[A].Mount == 1 && Player[A].MountType == 2)
                                                {
                                                    numNPCs++;
                                                    NPC[numNPCs] = NPC_t();
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
                                                    numNPCs++;
                                                    NPC[numNPCs] = NPC_t();
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
                                                if(HitSpot == 1 && !(Player[A].GroundPound && NPC[B].Killed == 8))
                                                {
                                                    tempHit = true;
                                                    tempLocation.Y = NPC[B].Location.Y - Player[A].Location.Height;
                                                    if(Player[A].SpinJump)
                                                    {
                                                        if(NPC[B].Killed > 0)
                                                        {
                                                            if(Player[A].Controls.Down)
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

                                    if((Player[A].State == 6 && Player[A].Duck && Player[A].Mount == 0 && Player[A].Character != 5) || (Player[A].Mount == 1 && Player[A].MountType == 2)) // Fireball immune for ducking in the hammer suit
                                    {
                                        if(NPC[B].Type == 85 || NPC[B].Type == 87 || NPC[B].Type == 246 || NPC[B].Type == 276)
                                        {
                                            PlaySound(3);
                                            HitSpot = 0;
                                            NPC[B].Killed = 9;
                                            for(C = 1; C <= 10; ++C)
                                            {
                                                NewEffect(77, NPC[B].Location, static_cast<float>(NPC[B].Special));
                                                Effect[numEffects].Location.SpeedX = dRand() * 3 - 1.5 + NPC[B].Location.SpeedX * 0.1;
                                                Effect[numEffects].Location.SpeedY = dRand() * 3 - 1.5 - NPC[B].Location.SpeedY * 0.1;
                                                if(Effect[numEffects].Frame == 0)
                                                    Effect[numEffects].Frame = -(iRand() % 3);
                                                else
                                                    Effect[numEffects].Frame = 5 + (iRand() % 3);
                                            }
                                            NPC[B].Location.X = NPC[B].Location.X + NPC[B].Location.Width / 2.0 - EffectWidth[10] / 2.0;
                                            NPC[B].Location.Y = NPC[B].Location.Y + NPC[B].Location.Height / 2.0 - EffectHeight[10] / 2.0;
                                            NewEffect(10, NPC[B].Location);
                                        }
                                    }


                                    if(NPCIsAVine[NPC[B].Type]) // if the player collided with a vine then see if he should climb it
                                    {
                                        if(Player[A].Character == 5)
                                        {
                                            if(Player[A].Immune == 0 && Player[A].Controls.Up)
                                            {
                                                Player[A].FairyCD = 0;
                                                if(!Player[A].Fairy)
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
                                        else if(!Player[A].Fairy && !Player[A].Stoned)
                                        {
                                            if(Player[A].Mount == 0 && Player[A].HoldingNPC <= 0)
                                            {
                                                if(Player[A].Vine > 0)
                                                {
                                                    if(Player[A].Duck)
                                                        UnDuck(A);
                                                    if(Player[A].Location.Y >= NPC[B].Location.Y - 20 && Player[A].Vine < 2)
                                                        Player[A].Vine = 2;
                                                    if(Player[A].Location.Y >= NPC[B].Location.Y - 18)
                                                        Player[A].Vine = 3;
                                                }
                                                else if((Player[A].Controls.Up ||
                                                         (Player[A].Controls.Down &&
                                                          !fEqual(Player[A].Location.SpeedY, 0.0) && // Not .Location.SpeedY = 0
                                                          Player[A].StandingOnNPC == 0 && // Not .StandingOnNPC <> 0
                                                          Player[A].Slope <= 0) // Not .Slope > 0
                                                        ) && Player[A].Jump == 0)
                                                {
                                                    if(Player[A].Duck)
                                                        UnDuck(A);
                                                    if(Player[A].Location.Y >= NPC[B].Location.Y - 20 && Player[A].Vine < 2)
                                                        Player[A].Vine = 2;
                                                    if(Player[A].Location.Y >= NPC[B].Location.Y - 18)
                                                        Player[A].Vine = 3;
                                                }

                                                if(Player[A].Vine > 0)
                                                {
                                                    Player[A].VineNPC = B;
                                                    Player[A].VineBGO = 0.0;
                                                }
                                            }
                                        }
                                    }

                                    // subcon warps
                                    if(NPC[B].Type == 289 && HitSpot > 0 && Player[A].Controls.Up)
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


                                    if(HitSpot == 1 && (NPC[B].Type == 32 || NPC[B].Type == 238 || NPC[B].Type == 239) && NPC[B].Projectile != 0)
                                        HitSpot = 0;

                                    if(NPC[B].Type == 255 && Player[A].HasKey)
                                    {
                                        Player[A].HasKey = false;
                                        HitSpot = 0;
                                        NPC[B].Killed = 3;
                                    }

                                    if(NPC[B].Type == 45 && NPC[B].Projectile != 0 && HitSpot > 1)
                                        HitSpot = 5;

                                    if(HitSpot == 1) // Player landed on a NPC
                                    {
                                        if(NPCCanWalkOn[NPC[B].Type] || (Player[A].ShellSurf && NPCIsAShell[NPC[B].Type])) // NPCs that can be walked on
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
                                        else if((NPCIsYoshi[NPC[B].Type] || NPCIsBoot[NPC[B].Type]) && Player[A].Character != 5 && !Player[A].Fairy)
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
                                        else if(NPC[B].Type != 22 && NPC[B].Type != 31 &&
                                                NPC[B].Type != 49 && NPC[B].Type != 50 &&
                                                (!Player[A].SlideKill || NPCWontHurt[NPC[B].Type])) // NPCs that cannot be walked on
                                        {
                                            if(NPC[B].CantHurtPlayer == A && Player[A].NoShellKick > 0)
                                            {
                                                // Do nothing!
                                            }
                                            else
                                            {
                                                if(NPCIsABonus[NPC[B].Type]) // Bonus
                                                    TouchBonus(A, B);
                                                else if(NPCIsAShell[NPC[B].Type] && NPC[B].Location.SpeedX == 0 && Player[A].HoldingNPC == 0 && Player[A].Controls.Run == true)
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
                                                else if(NPCJumpHurt[NPC[B].Type] || (NPCIsCheep[NPC[B].Type] && Player[A].WetFrame)) // NPCs that cause damage even when jumped on
                                                {
                                                    if(!(NPC[B].Type == 8 && NPC[B].Special2 == 4) && !NPCWontHurt[NPC[B].Type] && NPC[B].CantHurtPlayer != A)
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
                                                else if(NPC[B].Killed != 10 && !NPCIsBoot[NPC[B].Type] && !NPCIsYoshi[NPC[B].Type] && !(NPCIsAShell[NPC[B].Type] && NPC[B].CantHurtPlayer == A)) // Bounce off everything except Bonus and Piranha Plants
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

                                                // If Not (.WetFrame = True And (NPC(B).Type = 229 Or NPC(B).Type = 230) Or NPCIsAVine(NPC(B).Type)) And .HoldingNPC <> B Then
                                                if(
                                                        !(
                                                                (Player[A].WetFrame && (NPC[B].Type == 229 || NPC[B].Type == 230)) ||
                                                                NPCIsAVine[NPC[B].Type]
                                                        ) && (Player[A].HoldingNPC != B)
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

/* If (.CanGrabNPCs = True Or NPCIsGrabbable(NPC(B).Type) = True Or (NPC(B).Effect = 2 And NPCIsABonus(NPC(B).Type) = False)) And (NPC(B).Effect = 0 Or NPC(B).Effect = 2) Or (NPCIsAShell(NPC(B).Type) And FreezeNPCs = True) Then      'GRAB EVERYTHING
*/
                                        // grab code
                                        if(
                                            ((Player[A].CanGrabNPCs || NPCIsGrabbable[NPC[B].Type] || (NPC[B].Effect == 2 && !NPCIsABonus[NPC[B].Type])) && (NPC[B].Effect == 0 || NPC[B].Effect == 2)) ||
                                             (NPCIsAShell[NPC[B].Type] && FreezeNPCs)
                                        ) // GRAB EVERYTHING
                                        {
                                            if(Player[A].Controls.Run)
                                            {
                                                if((HitSpot == 2 && Player[A].Direction == -1) ||
                                                   (HitSpot == 4 && Player[A].Direction == 1) ||
                                                   (NPC[B].Type == 22 || NPC[B].Type == 49 || NPC[B].Effect == 2 || (NPCIsVeggie[NPC[B].Type] && NPC[B].CantHurtPlayer != A)))
                                                {
                                                    if(Player[A].HoldingNPC == 0)
                                                    {
                                                        if(!NPCIsAShell[NPC[B].Type] || Player[A].Character >= 3)
                                                        {
                                                            if(NPCIsVeggie[NPC[B].Type])
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
                                                if(((Player[A].Controls.Run && Player[A].HoldingNPC == 0) || Player[A].HoldingNPC == B) && NPC[B].CantHurtPlayer != A) // Grab the shell
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
                                                if(NPC[B].CantHurtPlayer != A && !FreezeNPCs && NPC[B].Type != 195)
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
                                            else if(NPC[B].CantHurtPlayer != A && !NPCWontHurt[NPC[B].Type])
                                            {
                                                if(!(NPC[B].Type == 17 && NPC[B].Projectile != 0))
                                                {
                                                    if(NPC[B].Type >= 117 && NPC[B].Type <= 120 && NPC[B].Projectile != 0)
                                                        NPCHit(B, 3, B);
                                                    else
                                                    {
                                                        if(NPC[B].Effect != 2)
                                                        {
                                                            if(Player[A].SlideKill && !NPCJumpHurt[NPC[B].Type])
                                                                NPCHit(B, 3, B);
                                                            if(NPC[B].Killed == 0)
                                                            {
                                                                if(n00bCollision(Player[A].Location, NPC[B].Location))
                                                                {
                                                                    if(BattleMode && NPC[B].HoldingPlayer != A && NPC[B].HoldingPlayer > 0 && Player[A].Immune == 0)
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
                                            if(NPCMovesPlayer[NPC[B].Type] && NPC[B].Projectile == 0 && !(Player[A].HoldingNPC == B) &&
                                              !(Player[A].Mount == 2 && (NPC[B].Type == 31 || NPC[B].Type == 32)) &&
                                               !ShadowMode && NPC[B].Effect != 2)
                                            {
                                                if(Player[A].StandUp && Player[A].StandingOnNPC == 0)
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
                                                        if(CheckCollision(Player[A].Location, Block[C].Location) &&
                                                           !Block[C].Hidden && !BlockIsSizable[Block[C].Type] &&
                                                           !BlockOnlyHitspot1[Block[C].Type])
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
                                                        if(!tempBool && NPC[B].Type != 168)
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
                                                        if(!tempBool && NPC[B].Type != 168)
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
                if(tempHit2)
                {
                    if(Player[A].Location.SpeedX + NPC[Player[A].StandingOnNPC].Location.SpeedX + tempHitSpeed > 0 && Player[A].Controls.Right)
                        Player[A].Location.SpeedX = 0.2 * Player[A].Direction + tempHitSpeed;
                    else if(Player[A].Location.SpeedX + NPC[Player[A].StandingOnNPC].Location.SpeedX + tempHitSpeed < 0 && Player[A].Controls.Left)
                        Player[A].Location.SpeedX = 0.2 * Player[A].Direction + tempHitSpeed;
                    else
                    {
                        if(Player[A].Controls.Right || Player[A].Controls.Left)
                            Player[A].Location.SpeedX = -NPC[Player[A].StandingOnNPC].Location.SpeedX + 0.2 * Player[A].Direction;
                        else
                            Player[A].Location.SpeedX = 0;
                    }
                }
                if(tempHit) // For multiple NPC hits
                {
                    if(Player[A].Character == 4 && (Player[A].State == 4 || Player[A].State == 5) && !Player[A].SpinJump)
                        Player[A].DoubleJump = true;
                    Player[A].CanJump = true;
                    if(tempSpring)
                    {
                        Player[A].Jump = Physics.PlayerSpringJumpHeight;
                        if(Player[A].Character == 2)
                            Player[A].Jump = Player[A].Jump + 3;
                        if(Player[A].SpinJump)
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
                        if(Player[A].SpinJump)
                            Player[A].Jump = Player[A].Jump - 6;
                        Player[A].Location.SpeedY = Physics.PlayerJumpVelocity;
                        if(Player[A].Wet > 0)
                            Player[A].Location.SpeedY = Player[A].Location.SpeedY * 0.3;
                    }
                    Player[A].Location.Y = tempLocation.Y;
                    if(tempShell)
                        NewEffect(132, newLoc(Player[A].Location.X + Player[A].Location.Width / 2.0 - EffectWidth[132] / 2.0, Player[A].Location.Y + Player[A].Location.Height - EffectHeight[132] / 2.0));
                    else if(!tempSpring)
                        NewEffect(75, newLoc(Player[A].Location.X + Player[A].Location.Width / 2.0 - 16, Player[A].Location.Y + Player[A].Location.Height - 16));
                    else
                        tempSpring = false;
                    PlayerPush(A, 3);
                    if(Player[A].YoshiBlue)
                    {
                        Player[A].CanFly2 = true;
                        Player[A].FlyCount = 300;
                    }
                    if(spinKill)
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
                if(NPC[B].Projectile != 0 && NPCIsVeggie[NPC[B].Type])
                    B = 0;

                // B is the number of the NPC that the player is standing on
                // .StandingOnNPC is the number of the NPC that the player was standing on last cycle
                // if B = 0 and .standingonnpc > 0 then the player was standing on something and is no longer standing on something


                if(B > 0 && Player[A].SpinJump)
                {
                    if(NPC[B].Type == 263)
                    {
                        Player[A].Location.SpeedY = Physics.PlayerJumpVelocity;
                        NPC[B].Multiplier = NPC[B].Multiplier + Player[A].Multiplier;
                        NPCHit(B, 3, B);
                        Player[A].Jump = 7;
                        if(Player[A].Character == 2)
                            Player[A].Jump = Player[A].Jump + 3;
                        if(Player[A].Controls.Down)
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
                else if(B > 0 && Player[A].StandingOnNPC == 0 && NPC[B].playerTemp && Player[A].Location.SpeedY >= 0)
                    Player[A].Location.SpeedX = Player[A].Location.SpeedX - (NPC[B].Location.SpeedX + NPC[B].BeltSpeed);

                if(movingBlock) // this is for when the player is standing on a moving block
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
                        if(Player[A].GroundPound)
                        {
                            numBlock = numBlock + 1;
                            Block[numBlock].Location.Y = NPC[B].Location.Y;
                            YoshiPound(A, numBlock, true);
                            Block[numBlock].Location.Y = 0;
                            numBlock = numBlock - 1;
                            Player[A].GroundPound = false;
                        }
                        else if(Player[A].YoshiYellow)
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


                    if(NPC[B].Type == 56 && Player[A].Controls.Down && Player[A].Mount == 0 &&
                       !NPC[B].playerTemp && Player[A].DuckRelease &&
                       (Player[A].HoldingNPC == 0 || Player[A].Character == 5))
                    {
                        UnDuck(A);
                        Player[A].Location = NPC[B].Location;
                        Player[A].Mount = 2;
                        NPC[B].Killed = 9;
                        Player[A].HoldingNPC = 0;
                        Player[A].StandingOnNPC = 0;
                        PlaySound(2);
                        for(C = 1; C <= numPlayers; ++C)
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
                    else if(B > 0 && Player[A].StandingOnNPC == 0 && NPC[B].playerTemp)
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
                        if(FreezeNPCs)
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

                if((Player[A].Location.SpeedY == 0 || Player[A].StandingOnNPC != 0 || Player[A].Slope > 0) &&
                   !Player[A].Slide && !FreezeNPCs)
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
                if(!GodMode)
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
                if(Player[A].ShellSurf && Player[A].StandingOnNPC != 0)
                {
                    Player[A].Location.X = NPC[Player[A].StandingOnNPC].Location.X + NPC[Player[A].StandingOnNPC].Location.Width / 2.0 - Player[A].Location.Width / 2.0;
                    Player[A].Location.SpeedX = 0; // 1 * .Direction
                    if(NPC[Player[A].StandingOnNPC].Location.SpeedX == 0)
                        Player[A].ShellSurf = false;
                }

                // Check edge of screen
                if(!LevelWrap[Player[A].Section] && LevelMacro == 0)
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
                if(!NPCIsAShell[NPC[Player[A].StandingOnNPC].Type])
                    Player[A].ShellSurf = false;

                PlayerGrabCode(A, DontResetGrabTime); // Player holding NPC code **GRAB CODE**

//                if(Player[A].Controls.Run == false && Player[A].Controls.AltRun == false) // Simplified below
//                    Player[A].RunRelease = true;
//                else
//                    Player[A].RunRelease = false;
                Player[A].RunRelease = !Player[A].Controls.Run && !Player[A].Controls.AltRun;

//                if(Player[A].Controls.Jump == false && Player[A].Controls.AltJump == false) // Simplified below
//                    Player[A].JumpRelease = true;
//                else
//                    Player[A].JumpRelease = false;
                Player[A].JumpRelease = !Player[A].Controls.Jump && !Player[A].Controls.AltJump;

                PlayerFrame(A); // Update players frames
                Player[A].StandUp = false; // Fixes a block collision bug
                Player[A].StandUp2 = false;
                if(Player[A].ForceHitSpot3)
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

//        if(Player[A].Controls.Down == true) // Simplified below
//            Player[A].DuckRelease = false;
//        else
//            Player[A].DuckRelease = true;
        Player[A].DuckRelease = !Player[A].Controls.Down;
    }

    C = 0;
    for(A = numNPCs; A >= 1; A--)
    {
        if(NPC[A].playerTemp)
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
