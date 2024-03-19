/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2024 Vitaly Novichkov <admin@wohlnet.ru>
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

#include <algorithm>

#include <Logger/logger.h>
#include <Utils/maths.h>

#include "../globals.h"
#include "../config.h"
#include "../player.h"
#include "../collision.h"
#include "../sound.h"
#include "../blocks.h"
#include "../npc.h"
#include "../effect.h"
#include "../layers.h"
#include "../editor.h"
#include "../game_main.h"
#include "../compat.h"
#include "../main/trees.h"
#include "../main/game_globals.h"
#include "../frame_timer.h"
#include "../graphics.h"
#include "../controls.h"
#include "../script/msg_preprocessor.h"
#include "script/luna/lunacounter.h"

#include "npc_id.h"
#include "npc_traits.h"
#include "eff_id.h"

#include "npc/npc_queues.h"


void UpdatePlayer()
{
    // int A = 0;
    int B = 0;
    float C = 0;
    float D = 0;
//    Controls_t blankControls;
    float speedVar = 0; // adjusts the players speed by percentages
    // int64_t fBlock = 0; // for collision detection optimizations
    // int64_t lBlock = 0;
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
    int oldStandingOnNpc = 0;
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
//        nPlay.PlayerWaitCount += 1;
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
        oldStandingOnNpc = Player[A].StandingOnNPC;
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
            Player[A].Dismount -= 1;
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
            Player[A].TimeToLive += 1;

            if(Player[A].TimeToLive == 50 && !g_ClonedPlayerMode)
                gDeathCounter.MarkDeath();

            const Screen_t& screen = ScreenByPlayer(A);
            bool dynamic_screen = (screen.Type == ScreenTypes::Dynamic);
            bool shared_screen = (screen.Type == ScreenTypes::SharedScreen);
            bool always_split = (screen.active_end() - screen.active_begin() > 1);
            bool split_screen = dynamic_screen || always_split; // was previously ScreenType == 5 (dynamic_screen)

            if(Player[A].TimeToLive >= 200 || !split_screen)
            {
                B = CheckNearestLiving(A);

                // move dead player towards start point in BattleMode
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
                    if(split_screen || shared_screen)
                    {
                        A1 = (Player[B].Location.X + Player[B].Location.Width * 0.5) - (Player[A].Location.X + Player[A].Location.Width * 0.5);
                        B1 = Player[B].Location.Y - Player[A].Location.Y;
                    }
                    else
                    {
                        const vScreen_t& vscreen = screen.vScreen(screen.active_begin() + 1);
                        A1 = (float)((-vscreen.X + vscreen.Width * 0.5) - (Player[A].Location.X + Player[A].Location.Width * 0.5));
                        B1 = (float)((-vscreen.Y + vscreen.Height * 0.5) - Player[A].Location.Y);
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
                    Player[A].Location.X += X * 10;
                    Player[A].Location.Y += Y * 10;

                    // update Player A section (was previously guarded in ScreenType == 5)
                    // code previously used Player 1 and Player 2 but this doesn't differ from that logic in cheat-free SMBX64
                    if(split_screen && Player[A].Section != Player[B].Section)
                    {
                        C1 = 0;

                        Player[A].Location.X = Player[B].Location.X;
                        Player[A].Location.Y = Player[B].Location.Y;
                        CheckSection(A);
                    }

                    if(C1 < 10 && C1 > -10)
                    {
                        KillPlayer(A);

                        // new logic: mark which player A's ghost is following
                        if(!BattleMode && Player[A].Dead)
                            Player[A].Effect2 = -B;

                        // new logic: fix player's location in split-screen mode
                        if(!dynamic_screen)
                        {
                            Player[A].Location.X = Player[B].Location.X;
                            Player[A].Location.Y = Player[B].Location.Y;
                        }
                    }
                }
                // start fadeout (65 / 3) frames before level end
                else if(Player[A].TimeToLive == 200 - (65 / 3))
                {
                    ProcessLastDead(); // Fade out screen if the last player died
                }
                else if(Player[A].TimeToLive >= 200) // ScreenType = 1
                {
                    KillPlayer(A); // Time to die
                }
            }
        }
        else if(Player[A].Dead)
        {
            // safer than the below code, should always be used except for compatibility concerns
            if(numPlayers > 2)
            {
                // continue following currently-tracked player if possible
                if(Player[A].Effect2 < 0)
                {
                    B = -Player[A].Effect2;

                    // put player back in TimeToLive state if their tracked dead player is gone
                    if(B > numPlayers || Player[B].Dead || Player[B].TimeToLive > 0)
                    {
                        Player[A].Effect2 = 0;
                        Player[A].Dead = false;
                        Player[A].TimeToLive = 200;

                        B = 0;
                    }
                }
                else
                    B = CheckNearestLiving(A);

                if(B)
                {
                    Player[A].Location.X = Player[B].Location.X;
                    Player[A].Location.Y = Player[B].Location.Y;
                    Player[A].Section = Player[B].Section;
                }
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
                Player[A].SlideCounter -= 1;

            // for the purple yoshi ground pound
            if(Player[A].Effect == 0)
            {
                // for the pound pet mount logic
                if(Player[A].Location.SpeedY != 0 && Player[A].StandingOnNPC == 0 && Player[A].Slope == 0)
                {
                    if(Player[A].Mount == 3 && Player[A].MountType == 6) // Purple Yoshi Pound
                    {
                        bool groundPoundByAltRun = !ForcedControls && g_compatibility.pound_by_alt_run;
                        bool poundKeyPressed = groundPoundByAltRun ? Player[A].Controls.AltRun : Player[A].Controls.Down;
                        bool poundKeyRelease = groundPoundByAltRun ? Player[A].AltRunRelease   : Player[A].DuckRelease;

                        if(poundKeyPressed && poundKeyRelease && Player[A].CanPound)
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

                    bool groundPoundByAltRun = !ForcedControls && g_compatibility.pound_by_alt_run;
                    if(groundPoundByAltRun)
                        Player[A].Controls.AltRun = true;
                    else
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
                    Player[A].Location.SpeedY += 1;
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

                SizeCheck(Player[A]); // check that the player is the correct size for it's character/state/mount and set it if not

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
                        UnDuck(Player[A]);
                    Player[A].Slide = true;
                }
                else if(Player[A].Location.SpeedX == 0.0)
                    Player[A].Slide = false;
                if(Player[A].Mount > 0 || Player[A].HoldingNPC > 0)
                    Player[A].Slide = false;

                // unduck a player that should be able to duck
                if(Player[A].Duck && (Player[A].Character == 1 || Player[A].Character == 2) && Player[A].State == 1 && (Player[A].Mount == 0 || Player[A].Mount == 2))
                    UnDuck(Player[A]);

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
                            Player[A].Location.SpeedX += slideSpeed * 2;
                        else if(slideSpeed < 0 && Player[A].Location.SpeedX > 0)
                            Player[A].Location.SpeedX += slideSpeed * 2;
                        else
                            Player[A].Location.SpeedX += slideSpeed;
                    }
                    else if(Player[A].Location.SpeedY == 0.0 || Player[A].StandingOnNPC != 0)
                    {
                        if(Player[A].Location.SpeedX > 0.2)
                            Player[A].Location.SpeedX -= 0.1;
                        else if(Player[A].Location.SpeedX < -0.2)
                            Player[A].Location.SpeedX += 0.1;
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
                        tempLocation.Y += -Physics.PlayerHeight[Player[A].Character][Player[A].State];
                        tempLocation.Width = Physics.PlayerWidth[Player[A].Character][Player[A].State];
                        tempLocation.X += 64 - tempLocation.Width / 2.0;
                        // fBlock = FirstBlock[(tempLocation.X / 32) - 1];
                        // lBlock = LastBlock[((tempLocation.X + tempLocation.Width) / 32.0) + 1];
                        // blockTileGet(tempLocation, fBlock, lBlock);

                        for(int B : treeFLBlockQuery(tempLocation, SORTMODE_NONE))
                        {
                            if(!Block[B].Invis && !BlockIsSizable[Block[B].Type] && !BlockOnlyHitspot1[Block[B].Type] &&
                               !BlockNoClipping[Block[B].Type] && !Block[B].Hidden)
                            {
                                if(CheckCollision(tempLocation, Block[B].Location))
                                {
                                    tempBool = false;
                                    PlaySoundSpatial(SFX_BlockHit, Player[A].Location);
                                    break;
                                }
                            }
                        }

                        if(!tempBool) for(int B : treeNPCQuery(tempLocation, SORTMODE_NONE))
                        {
                            if(NPC[B]->IsABlock && !NPC[B]->StandsOnPlayer && NPC[B].Active && NPC[B].Type != NPCID_VEHICLE)
                            {
                                if(CheckCollision(tempLocation, NPC[B].Location))
                                {
                                    tempBool = false;
                                    PlaySoundSpatial(SFX_BlockHit, Player[A].Location);
                                    break;
                                }
                            }
                        }

                        if(tempBool)
                        {
                            PlayerDismount(A);
                        }
                    }
                }
                else if(Player[A].Driving) // driving
                {
                    if(Player[A].Duck)
                        UnDuck(Player[A]);
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
                            Player[A].Location.SpeedX += 0.15;
                        if(Player[A].Location.SpeedX < 0)
                            Player[A].Location.SpeedX += 0.1;
                    }
                    else if(Player[A].Controls.Left)
                    {
                        if(Player[A].Location.SpeedX > -3)
                            Player[A].Location.SpeedX -= 0.15;
                        if(Player[A].Location.SpeedX > 0)
                            Player[A].Location.SpeedX -= 0.1;
                    }
                    else if(Player[A].Location.SpeedX > 0.1)
                        Player[A].Location.SpeedX -= 0.1;
                    else if(Player[A].Location.SpeedX < -0.1)
                        Player[A].Location.SpeedX += 0.1;
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

                    if(g_compatibility.fix_climb_bgo_speed_adding && Player[A].VineBGO > 0)
                    {
                        Player[A].Location.SpeedX += Background[Player[A].VineBGO].Location.SpeedX;
                        Player[A].Location.SpeedY += Background[Player[A].VineBGO].Location.SpeedY;
                    }
                    else
                    {
                        Player[A].Location.SpeedX += NPC[(int)Player[A].VineNPC].Location.SpeedX;
                        Player[A].Location.SpeedY += NPC[(int)Player[A].VineNPC].Location.SpeedY;
                    }
                }
                else
                {

                    // if none of the above apply then the player controls like normal. remeber this is for the players X movement


                    // ducking for link
                    if(Player[A].Duck && Player[A].WetFrame)
                    {
                        if(Player[A].Location.SpeedY != 0.0 && Player[A].Slope == 0 && Player[A].StandingOnNPC == 0)
                            UnDuck(Player[A]);
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
                                        Player[A].Location.Y += Player[A].Location.Height;
                                        Player[A].Location.Height = 31;
                                        Player[A].Location.Y += -Player[A].Location.Height;
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
                                            Player[A].Location.Y += Player[A].Location.Height;
                                            Player[A].Location.Height = Physics.PlayerDuckHeight[Player[A].Character][Player[A].State];
                                            Player[A].Location.Y += -Player[A].Location.Height;
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
                                            Player[A].Location.Y += -Physics.PlayerDuckHeight[1][2] + Physics.PlayerHeight[1][2];
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
                                UnDuck(Player[A]);
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
                                    Player[A].Location.SpeedX += 0.1 * 0.175;
                                if(Player[A].Character == 3) // PEACH
                                    Player[A].Location.SpeedX += 0.05 * 0.175;
                                if(Player[A].Character == 4) // toad
                                    Player[A].Location.SpeedX += -0.05 * 0.175;
                                Player[A].Location.SpeedX += -0.1 * speedVar;
                            }
                            else // Running
                            {
                                if(Player[A].Character == 2) // LUIGI
                                    Player[A].Location.SpeedX += 0.05 * 0.175;
                                if(Player[A].Character == 3) // PEACH
                                    Player[A].Location.SpeedX += 0.025 * 0.175;
                                if(Player[A].Character == 4) // toad
                                    Player[A].Location.SpeedX += -0.025 * 0.175;
                                if(Player[A].Character == 5) // Link
                                    Player[A].Location.SpeedX += -0.025 * speedVar;
                                else // Mario
                                    Player[A].Location.SpeedX += -0.05 * speedVar;
                            }
                            if(Player[A].Location.SpeedX > 0)
                            {
                                Player[A].Location.SpeedX -= 0.18;
                                if(Player[A].Character == 2) // LUIGI
                                    Player[A].Location.SpeedX += 0.18 * 0.29;
                                if(Player[A].Character == 3) // PEACH
                                    Player[A].Location.SpeedX += 0.09 * 0.29;
                                if(Player[A].Character == 4) // toad
                                    Player[A].Location.SpeedX += -0.09 * 0.29;
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
                                    Player[A].Location.SpeedX += -0.1 * 0.175;
                                if(Player[A].Character == 3) // PEACH
                                    Player[A].Location.SpeedX += -0.05 * 0.175;
                                if(Player[A].Character == 4) // toad
                                    Player[A].Location.SpeedX += 0.05 * 0.175;
                                Player[A].Location.SpeedX += 0.1 * speedVar;
                            }
                            else
                            {
                                if(Player[A].Character == 2) // LUIGI
                                    Player[A].Location.SpeedX += -0.05 * 0.175;
                                if(Player[A].Character == 3) // PEACH
                                    Player[A].Location.SpeedX += -0.025 * 0.175;
                                if(Player[A].Character == 4) // toad
                                    Player[A].Location.SpeedX += 0.025 * 0.175;
                                if(Player[A].Character == 5) // Link
                                    Player[A].Location.SpeedX += 0.025 * speedVar;
                                else // Mario
                                    Player[A].Location.SpeedX += 0.05 * speedVar;
                            }
                            if(Player[A].Location.SpeedX < 0)
                            {
                                Player[A].Location.SpeedX += 0.18;
                                if(Player[A].Character == 2) // LUIGI
                                    Player[A].Location.SpeedX += -0.18 * 0.29;
                                if(Player[A].Character == 3) // PEACH
                                    Player[A].Location.SpeedX += -0.09 * 0.29;
                                if(Player[A].Character == 4) // toad
                                    Player[A].Location.SpeedX += 0.09 * 0.29;
                                if(SuperSpeed)
                                    Player[A].Location.SpeedX = Player[A].Location.SpeedX * 0.95;
                            }
                        }
                        if(SuperSpeed && Player[A].Controls.Run)
                            Player[A].Location.SpeedX += 0.1;
                    }
                    else
                    {
                        if(Player[A].Location.SpeedY == 0.0 || Player[A].StandingOnNPC != 0 || Player[A].Slope > 0 || Player[A].WetFrame) // Only lose speed when not in the air
                        {
                            if(Player[A].Location.SpeedX > 0)
                                Player[A].Location.SpeedX += -0.07 * speedVar;
                            if(Player[A].Location.SpeedX < 0)
                                Player[A].Location.SpeedX += 0.07 * speedVar;
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

                    if(Player[A].WarpShooted &&
                       Player[A].Location.SpeedX < Physics.PlayerRunSpeed * speedVar &&
                       Player[A].Location.SpeedX > -Physics.PlayerRunSpeed * speedVar)
                    {
                        Player[A].WarpShooted = false;
                    }

                    if(!Player[A].WarpShooted && (Player[A].Controls.Run || Player[A].Character == 5))
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
//                        else  // REDURANT GARBAGE
//                        {
//                        }
                    }
                    else
                    {
                        if(Player[A].Location.SpeedX > Physics.PlayerWalkSpeed + 0.1 * speedVar)
                            Player[A].Location.SpeedX -= 0.1;
                        else if(Player[A].Location.SpeedX < -Physics.PlayerWalkSpeed - 0.1 * speedVar)
                            Player[A].Location.SpeedX += 0.1;
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
                    if(iRand(10) == 0)
                    {
                        NewEffect(EFFID_SPARKLE,
                                  newLoc(Player[A].Location.X - 8 + dRand() * (Player[A].Location.Width + 16) - 4,
                                         Player[A].Location.Y - 8 + dRand() * (Player[A].Location.Height + 16)), 1, 0, ShadowMode);
                        Effect[numEffects].Location.SpeedX = dRand() * 0.5 - 0.25;
                        Effect[numEffects].Location.SpeedY = dRand() * 0.5 - 0.25;
                        Effect[numEffects].Frame = 1;
                    }
                    if(Player[A].FairyTime > 0)
                        Player[A].FairyTime -= 1;
                    if(Player[A].FairyTime != -1 && Player[A].FairyTime < 20 && Player[A].Character == 5)
                    {
                        tempLocation = Player[A].Location;
                        tempLocation.Width += 32;
                        tempLocation.Height += 32;
                        tempLocation.X -= 16;
                        tempLocation.Y -= 16;

                        for(int Bi : treeNPCQuery(tempLocation, SORTMODE_NONE))
                        {
                            if(NPC[Bi].Active && !NPC[Bi].Hidden && NPC[Bi]->IsAVine)
                            {
                                if(CheckCollision(tempLocation, NPC[Bi].Location))
                                {
                                    Player[A].FairyTime = 20;
                                    Player[A].FairyCD = 0;
                                    break;
                                }
                            }
                        }

                        for(int B : treeBackgroundQuery(tempLocation, SORTMODE_NONE))
                        {
                            if(B > numBackground)
                                continue;

                            if(BackgroundFence[Background[B].Type] && !Background[B].Hidden)
                            {
                                if(CheckCollision(tempLocation, Background[B].Location))
                                {
                                    Player[A].FairyTime = 20;
                                    Player[A].FairyCD = 0;
                                    break;
                                }
                            }
                        }

                    }
                }
                else if(Player[A].Fairy)
                {
                    PlaySoundSpatial(SFX_HeroFairy, Player[A].Location);
                    Player[A].Immune = 10;
                    Player[A].Effect = 8;
                    Player[A].Effect2 = 4;
                    Player[A].Fairy = false;
                    SizeCheck(Player[A]);
                    NewEffect(EFFID_SMOKE_S5, Player[A].Location);
                    PlayerPush(A, 3);
                }
                else
                    Player[A].FairyTime = 0;
                if(Player[A].FairyCD != 0 && (Player[A].Location.SpeedY == 0.0 || Player[A].Slope != 0 || Player[A].StandingOnNPC != 0 || Player[A].WetFrame))
                    Player[A].FairyCD -= 1;


                // the single pinched variable has been always false since SMBX64
                if(Player[A].StandingOnNPC != 0 && /*!NPC[Player[A].StandingOnNPC].Pinched && */ !FreezeNPCs)
                {
                    if(Player[A].StandingOnNPC < 0)
                        NPC[Player[A].StandingOnNPC].Location = Block[(int)NPC[Player[A].StandingOnNPC].Special].Location;
                    Player[A].Location.SpeedX += NPC[Player[A].StandingOnNPC].Location.SpeedX + NPC[Player[A].StandingOnNPC].BeltSpeed;
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


                Player[A].Location.X += Player[A].Location.SpeedX; // This is where the actual movement happens

                // Players Y movement.
                if(Block[Player[A].Slope].Location.SpeedY != 0.0 && Player[A].Slope != 0)
                    Player[A].Location.Y += Block[Player[A].Slope].Location.SpeedY;

                if(Player[A].Fairy) // the player is a fairy
                {
                    Player[A].WetFrame = false;
                    Player[A].Wet = 0;
                    if(Player[A].FairyCD == 0)
                    {
                        if(Player[A].Controls.Jump || Player[A].Controls.AltJump || Player[A].Controls.Up)
                        {
                            Player[A].Location.SpeedY -= 0.15;
                            if(Player[A].Location.SpeedY > 0)
                                Player[A].Location.SpeedY -= 0.1;
                        }
                        else if(Player[A].Location.SpeedY < -0.1 || Player[A].Controls.Down)
                        {
                            if(Player[A].Location.SpeedY < 3)
                                Player[A].Location.SpeedY += double(Physics.PlayerGravity * 0.05f);
                            if(Player[A].Location.SpeedY < 0)
                                Player[A].Location.SpeedY += double(Physics.PlayerGravity * 0.05f);
                            Player[A].Location.SpeedY += double(Physics.PlayerGravity * 0.1f);
                            if(Player[A].Controls.Down)
                                Player[A].Location.SpeedY += 0.05;
                        }
                        else if(Player[A].Location.SpeedY > 0.1)
                            Player[A].Location.SpeedY -= 0.15;
                        else
                            Player[A].Location.SpeedY = 0;
                    }
                    else
                    {
                        if(Player[A].Controls.Jump || Player[A].Controls.AltJump || Player[A].Controls.Up)
                        {
                            Player[A].Location.SpeedY -= 0.15;
                            if(Player[A].Location.SpeedY > 0)
                                Player[A].Location.SpeedY -= 0.1;
                        }
                        else
                        {
                            if(Player[A].Location.SpeedY < 3)
                                Player[A].Location.SpeedY += Physics.PlayerGravity * 0.05;
                            if(Player[A].Location.SpeedY < 0)
                                Player[A].Location.SpeedY += Physics.PlayerGravity * 0.05;
                            Player[A].Location.SpeedY += Physics.PlayerGravity * 0.1;
                            if(Player[A].Controls.Down)
                                Player[A].Location.SpeedY += 0.05;
                        }
                    }

                    if(Player[A].Location.SpeedY > 4)
                        Player[A].Location.SpeedY = 4;
                    else if(Player[A].Location.SpeedY < -3)
                        Player[A].Location.SpeedY = -3;
                    Player[A].Location.Y += Player[A].Location.SpeedY;
                }
                else if(Player[A].Wet > 0 && Player[A].Quicksand == 0) // the player is swimming
                {
                    if(Player[A].Mount == 1)
                    {
                        if(Player[A].Controls.AltJump && Player[A].CanAltJump)
                        {
                            PlayerDismount(A);
                        }
                    }
                    else if(Player[A].Mount == 3)
                    {
                        if(Player[A].Controls.AltJump && Player[A].CanAltJump)
                        {
                            PlayerDismount(A);
                        }
                    }

                    if(Player[A].Duck)
                    {
                        if(Player[A].StandingOnNPC == 0 && Player[A].Slope == 0 && Player[A].Location.SpeedY != 0 && Player[A].Mount != 1)
                        {
                            if(Player[A].Character <= 2) // unduck wet players that aren't peach o toad
                                UnDuck(Player[A]);
                        }
                    }

                    Player[A].Location.SpeedY += Physics.PlayerGravity * 0.1;
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
                                    PlaySoundSpatial(SFX_Skid, Player[A].Location);
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
                        Player[A].SwimCount -= 1;
                    if(Player[A].SwimCount == 0)
                    {
                        if(Player[A].Mount != 1 || Player[A].Location.SpeedY == Physics.PlayerGravity * 0.1 || Player[A].Slope != 0 || Player[A].StandingOnNPC != 0)
                        {
                            if((Player[A].Controls.Jump && Player[A].CanJump) ||
                               (Player[A].Controls.AltJump && Player[A].CanAltJump))
                            {
                                if(Player[A].Duck && Player[A].Mount != 1 && Player[A].Character <= 2)
                                    UnDuck(Player[A]);
                                if(Player[A].Slope != 0)
                                    Player[A].Location.SpeedY = 0;
                                Player[A].Vine = 0;
                                if(Player[A].StandingOnNPC != 0)
                                {
                                    Player[A].Location.SpeedY = NPC[Player[A].StandingOnNPC].Location.SpeedY;
                                    Player[A].StandingOnNPC = 0;
                                }
                                Player[A].SwimCount = 15;
                                // If .Location.SpeedY = 0 Then .Location.Y += -1
                                if(Player[A].Controls.Down)
                                {
                                    if(Player[A].Location.SpeedY >= Physics.PlayerJumpVelocity * 0.2)
                                    {
                                        Player[A].Location.SpeedY += Physics.PlayerJumpVelocity * 0.2;
                                        if(Player[A].Location.SpeedY < Physics.PlayerJumpVelocity * 0.2)
                                            Player[A].Location.SpeedY = Physics.PlayerJumpVelocity * 0.2;
                                    }
                                }
                                else
                                {
                                    if(Player[A].Controls.Up)
                                        Player[A].Location.SpeedY += Physics.PlayerJumpVelocity * 0.5;
                                    else
                                        Player[A].Location.SpeedY += Physics.PlayerJumpVelocity * 0.4;
                                    if(Player[A].Mount == 1)
                                        Player[A].Location.SpeedY = Physics.PlayerJumpVelocity;
                                }
                                if(Player[A].Location.SpeedY > 0)
                                    Player[A].Location.SpeedY = Physics.PlayerJumpVelocity * 0.2;
                                PlaySoundSpatial(SFX_Swim, Player[A].Location);
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

                    Player[A].Location.Y += Player[A].Location.SpeedY;

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
                                        PlaySoundSpatial(SFX_Skid, Player[A].Location);
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
                                PlayerDismount(A);
                            }
                        }
                        else if(Player[A].Mount == 3)
                        {
                            if(Player[A].Controls.AltJump && Player[A].CanAltJump) // jump off of yoshi
                            {
                                PlayerDismount(A);
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
                                        PlaySoundSpatial(SFX_Whip, Player[A].Location); // Jump sound
                                        Player[A].Jump = Physics.PlayerJumpHeight * 0.6;
                                        NPC[Player[A].StandingOnNPC].Location.SpeedY = Physics.PlayerJumpVelocity * 0.9;
                                    }
                                }
                                else if(Player[A].Jump > 0)
                                    NPC[Player[A].StandingOnNPC].Location.SpeedY = Physics.PlayerJumpVelocity * 0.9;

                            }
                            // if not surfing a shell then proceed like normal
                            else
                            {
                                if((Player[A].Vine > 0 || Player[A].Location.SpeedY == 0 || Player[A].StandingOnNPC != 0 ||
                                    MultiHop || Player[A].Slope > 0 || (Player[A].Location.SpeedY > 0 && Player[A].Quicksand > 0)) && Player[A].CanJump)
                                {
                                    PlaySoundSpatial(SFX_Jump, Player[A].Location); // Jump sound
                                    Player[A].Location.SpeedY = Physics.PlayerJumpVelocity - tempSpeed;
                                    Player[A].Jump = Physics.PlayerJumpHeight;

                                    if(Player[A].Character == 4 && (Player[A].State == 4 || Player[A].State == 5) && !Player[A].SpinJump)
                                        Player[A].DoubleJump = true;

                                    if(Player[A].Character == 2)
                                        Player[A].Jump += 3;

                                    if(Player[A].SpinJump)
                                        Player[A].Jump -= 6;

                                    if(Player[A].StandingOnNPC > 0 && !FreezeNPCs)
                                    {
                                        if(NPC[Player[A].StandingOnNPC].Type != 91)
                                            Player[A].Location.SpeedX += -NPC[Player[A].StandingOnNPC].Location.SpeedX;
                                    }

                                    Player[A].StandingOnNPC = 0; // the player can't stand on an NPC after jumping

                                    if(Player[A].CanFly) // let's the player fly if the conditions are met
                                    {
                                        Player[A].StandingOnNPC = 0;
                                        Player[A].Jump = 30;
                                        if(Player[A].Character == 2)
                                            Player[A].Jump += 3;
                                        if(Player[A].SpinJump)
                                            Player[A].Jump -= 6;
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
#if 0
                                        // FIXME: Duplicated "Character == 3" condition branch [PVS Studio]
                                        else if(Player[A].Character == 3) // special handling for peach
                                            Player[A].FlyCount = 280; // Length of flight time
#endif
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
                                            Player[A].Location.SpeedY += -(40 - 20) * 0.2;
                                        else
                                            Player[A].Location.SpeedY += -(Player[A].Jump - 20) * 0.2;
                                    }
                                }
                                else if(Player[A].CanFly2)
                                {
                                    if(Player[A].Location.SpeedY > Physics.PlayerJumpVelocity * 0.5)
                                    {
                                        Player[A].Location.SpeedY -= 1;
                                        Player[A].CanPound = true;
                                        if(Player[A].YoshiBlue || (Player[A].Mount == 1 && Player[A].MountType == 3))
                                            PlaySoundSpatial(SFX_PetTongue, Player[A].Location);
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
                        if(Player[A].DoubleJump && Player[A].Jump == 0 && Player[A].Location.SpeedY != 0 && Player[A].Slope == 0 &&
                           Player[A].StandingOnNPC == 0 && Player[A].Wet == 0 && Player[A].Vine == 0 &&
                           !Player[A].WetFrame && !Player[A].Fairy && !Player[A].CanFly2)
                        {
                            if(Player[A].Controls.Jump && Player[A].JumpRelease)
                            {
                                PlaySoundSpatial(SFX_Jump, Player[A].Location);
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
                                    NewEffect(EFFID_SPARKLE, tempLocation);
                                    Effect[numEffects].Location.SpeedX = (dRand() * 3) - 1.5;
                                    Effect[numEffects].Location.SpeedY = (dRand() * 0.5) + (1.5 - std::abs(Effect[numEffects].Location.SpeedX)) * 0.5;
                                    Effect[numEffects].Location.SpeedX += -Player[A].Location.SpeedX * 0.2;
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
                                PlaySoundSpatial(SFX_Skid, Player[A].Location);
                                NPC[Player[A].StandingOnNPC].CantHurt = 30;
                                NPC[Player[A].StandingOnNPC].CantHurtPlayer = A;
                                Player[A].Location.SpeedX = NPC[Player[A].StandingOnNPC].Location.SpeedX / 2;
                                Player[A].StandingOnNPC = 0;
                                Player[A].NoShellKick = 30;
                            }
                            else
#endif
                            {
                                if(iRand(10) >= 3)
                                {
                                    tempLocation.Y = Player[A].Location.Y + Player[A].Location.Height - 2 + dRand() * (NPC[Player[A].StandingOnNPC].Location.Height - 8) + 4;
                                    tempLocation.X = Player[A].Location.X - 4 + dRand() * (Player[A].Location.Width - 8) + 4 - 8 * Player[A].Direction;
                                    NewEffect(EFFID_SPARKLE, tempLocation, 1, 0, ShadowMode);
                                    Effect[numEffects].Frame = iRand(3);
                                    Effect[numEffects].Location.SpeedY = (Player[A].Location.Y + Player[A].Location.Height + NPC[Player[A].StandingOnNPC].Location.Height / 32.0 - tempLocation.Y + 12) * 0.05;
                                }
                            }

                            if(NPC[Player[A].StandingOnNPC].Wet == 2)
                            {
                                if(NPC[Player[A].StandingOnNPC].Type == 195)
                                    NPC[Player[A].StandingOnNPC].Special4 = 1;
                                NPC[Player[A].StandingOnNPC].Location.SpeedY += -Physics.NPCGravity * 1.5;
                            }
                        }

                        // START ALT JUMP - this code does the player's spin jump
                        if(Player[A].Controls.AltJump && (Player[A].Character == 1 || Player[A].Character == 2 || Player[A].Character == 4 ||
                                                          (g_compatibility.fix_char3_escape_shell_surf && Player[A].Character == 3 && Player[A].ShellSurf)))
                        {
                            if(Player[A].Location.SpeedX > 0)
                                tempSpeed = Player[A].Location.SpeedX * 0.2;
                            else
                                tempSpeed = -Player[A].Location.SpeedX * 0.2;

                            if((Player[A].Vine > 0 || Player[A].Location.SpeedY == 0 || Player[A].StandingOnNPC != 0 || Player[A].Slope > 0 || MultiHop) && Player[A].CanAltJump) // Player Jumped
                            {
                                if(!Player[A].Duck)
                                {
                                    Player[A].Slope = 0;
                                    Player[A].SpinFireDir = Player[A].Direction;
                                    Player[A].Location.SpeedY = Physics.PlayerJumpVelocity - tempSpeed;
                                    Player[A].Jump = Physics.PlayerJumpHeight;
                                    if(Player[A].Character == 2)
                                        Player[A].Jump += 3;

                                    if(Player[A].StandingOnNPC > 0 && !FreezeNPCs)
                                    {
                                        if(NPC[Player[A].StandingOnNPC].Type != 91)
                                            Player[A].Location.SpeedX += -NPC[Player[A].StandingOnNPC].Location.SpeedX;
                                    }

                                    PlaySoundSpatial(SFX_Whip, Player[A].Location); // Jump sound
                                    Player[A].Jump -= 6;
                                    if(Player[A].Direction == 1)
                                        Player[A].SpinFrame = 0;
                                    else
                                        Player[A].SpinFrame = 6;
                                    Player[A].SpinJump = true;
//                                    if(nPlay.Online == true && nPlay.MySlot + 1 == A)
//                                        Netplay::sendData Netplay::PutPlayerLoc(nPlay.MySlot) + "1l" + std::to_string(A) + LB;
                                    if(Player[A].Duck)
                                        UnDuck(Player[A]);

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
                                            Player[A].Jump += 3;
                                        if(Player[A].SpinJump)
                                            Player[A].Jump -= 6;
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
                                    Player[A].Location.SpeedY += -(Player[A].Jump - 20) * 0.2;
                            }
                            else if(Player[A].CanFly2)
                            {
                                if(Player[A].Location.SpeedY > Physics.PlayerJumpVelocity * 0.5)
                                {
                                    Player[A].Location.SpeedY -= 1;
                                    Player[A].CanPound = true;
                                    if(Player[A].YoshiBlue)
                                        PlaySoundSpatial(SFX_PetTongue, Player[A].Location);
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
                            Player[A].Jump -= 1;

                        if(Player[A].Jump > 0)
                            Player[A].Vine = 0;


                        if(Player[A].Quicksand > 1)
                        {
                            Player[A].Slide = false;
                            if(Player[A].Location.SpeedY < -0.7)
                            {
                                Player[A].Location.SpeedY = -0.7;
                                Player[A].Jump -= 1;
                            }
                            else if(Player[A].Location.SpeedY < 0)
                            {
                                Player[A].Location.SpeedY += 0.1;
                                Player[A].Jump = 0;
                            }
                            if(Player[A].Location.SpeedY >= 0.1)
                                Player[A].Location.SpeedY = 0.1;
                            Player[A].Location.Y += Player[A].Location.SpeedY;
                        }


                        // gravity
                        if(Player[A].Vine == 0)
                        {
                            if(Player[A].NoGravity == 0)
                            {
                                if(Player[A].Character == 2)
                                    Player[A].Location.SpeedY += Physics.PlayerGravity * 0.9;
                                else
                                    Player[A].Location.SpeedY += Physics.PlayerGravity;
                                if(Player[A].HoldingNPC > 0)
                                {
                                    if(NPC[Player[A].HoldingNPC].Type == 278 || NPC[Player[A].HoldingNPC].Type == 279)
                                    {
                                        if(Player[A].Controls.Jump || Player[A].Controls.AltJump)
                                        {
                                            if(Player[A].Character == 2)
                                                Player[A].Location.SpeedY += -Physics.PlayerGravity * 0.9 * 0.8;
                                            else
                                                Player[A].Location.SpeedY += -Physics.PlayerGravity * 0.8;
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
                                Player[A].NoGravity -= 1;
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
                                Player[A].FloatTime -= 1;
                                Player[A].FloatSpeed += Player[A].FloatDir * 0.1;
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
                            if((Player[A].Controls.Jump || Player[A].Controls.AltJump) &&
                              ((Player[A].Location.SpeedY > Physics.PlayerGravity * 5 && Player[A].Character != 3 && Player[A].Character != 4) ||
                                (Player[A].Location.SpeedY > Physics.PlayerGravity * 10 && Player[A].Character == 3) ||
                                (Player[A].Location.SpeedY > Physics.PlayerGravity * 7.5 && Player[A].Character == 4)) &&
                               !Player[A].GroundPound && Player[A].Slope == 0 && Player[A].Character != 5)
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
                                    if(iRand(10) == 0)
                                    {
                                        NewEffect(EFFID_SPARKLE, newLoc(Player[A].Location.X - 8 + dRand() * (Player[A].Location.Width + 16) - 4,
                                                             Player[A].Location.Y - 8 + dRand() * (Player[A].Location.Height + 16)), 1, 0, ShadowMode);
                                        Effect[numEffects].Location.SpeedX = (dRand() * 0.5) - 0.25;
                                        Effect[numEffects].Location.SpeedY = (dRand() * 0.5) - 0.25;
                                        Effect[numEffects].Frame = 1;
                                    }
                                }
                            }
                        }
                    }
                    Player[A].Location.Y += Player[A].Location.SpeedY;
                }

                // princess peach and toad stuff
                if(Player[A].Character == 3 || Player[A].Character == 4 || Player[A].Character == 5)
                {
                    Player[A].HeldBonus = NPCID(0);
                    // power up limiter
                    // If (.Character = 3 Or .Character = 4) And .State > 3 And .State <> 7 Then .State = 2

                    if(Player[A].Mount == 3)
                    {
                        PlayerHurt(A);
                        Player[A].Mount = 0;
                        UpdateYoshiMusic();
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
                        bool hasNoMonts = (g_compatibility.fix_char5_vehicle_climb && Player[A].Mount <= 0) ||
                                           !g_compatibility.fix_char5_vehicle_climb;

                        bool turnFairy = Player[A].FlyCount > 0 ||
                                        ((Player[A].Controls.AltJump || (Player[A].Controls.Jump && Player[A].FloatRelease)) &&
                                          Player[A].Location.SpeedY != Physics.PlayerGravity && Player[A].Slope == 0 &&
                                          Player[A].StandingOnNPC == 0);

                        if(turnFairy && hasNoMonts)
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
                                SizeCheck(Player[A]);
                                PlaySoundSpatial(SFX_HeroFairy, Player[A].Location);
                                Player[A].Immune = 10;
                                Player[A].Effect = 8;
                                Player[A].Effect2 = 4;
                                NewEffect(EFFID_SMOKE_S5, Player[A].Location);
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
                        // Coins += -1
                        // If Coins < 0 Then
                        // Lives += -1
                        // Coins += 99
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
                        for(int B : treeBackgroundQuery(Player[A].Location, SORTMODE_NONE))
                        {
                            if(B > numBackground)
                                continue;

                            if(Background[B].Type == 35)
                            {
                                tempLocation = Background[B].Location;
                                tempLocation.Width = 16;
                                tempLocation.X += 8;
                                tempLocation.Height = 26;
                                tempLocation.Y += 2;
                                if(CheckCollision(Player[A].Location, tempLocation))
                                {
                                    PlaySound(SFX_Key);
                                    StopMusic();
                                    LevelMacro = LEVELMACRO_KEYHOLE_EXIT;
                                    LevelMacroWhich = B;
                                    break;
                                }
                            }
                        }
                    }
                    if(Player[A].SwordPoke < 0)
                    {
                        Player[A].SwordPoke -= 1;
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
                            PlaySoundSpatial(SFX_HeroStab, Player[A].Location);
                            if((Player[A].State == 3 || Player[A].State == 7 || Player[A].State == 6) && Player[A].FireBallCD2 == 0)
                            {
                                Player[A].FireBallCD2 = 40;
                                if(Player[A].State == 6)
                                    Player[A].FireBallCD2 = 25;

                                if(Player[A].State == 6)
                                    PlaySoundSpatial(SFX_HeroSwordBeam, Player[A].Location);
                                else if(Player[A].State == 7)
                                    PlaySoundSpatial(SFX_HeroIce, Player[A].Location);
                                else
                                    PlaySoundSpatial(SFX_HeroFireRod, Player[A].Location);

                                numNPCs++;
                                NPC[numNPCs] = NPC_t();
                                if(ShadowMode)
                                    NPC[numNPCs].Shadow = true;
                                NPC[numNPCs].Type = NPCID_PLR_FIREBALL;
                                if(Player[A].State == 7)
                                    NPC[numNPCs].Type = NPCID_PLR_ICEBALL;
                                if(Player[A].State == 6)
                                    NPC[numNPCs].Type = NPCID_SWORDBEAM;
                                NPC[numNPCs].Projectile = true;
                                NPC[numNPCs].Location.Height = NPC[numNPCs]->THeight;
                                NPC[numNPCs].Location.Width = NPC[numNPCs]->TWidth;
                                NPC[numNPCs].Location.X = Player[A].Location.X + Player[A].Location.Width / 2.0 + (40 * Player[A].Direction) - 8;
                                if(!Player[A].Duck)
                                {
                                    NPC[numNPCs].Location.Y = Player[A].Location.Y + 5;
                                    if(Player[A].State == 6)
                                        NPC[numNPCs].Location.Y += 7;
                                }
                                else
                                {
                                    NPC[numNPCs].Location.Y = Player[A].Location.Y + 18;
                                    if(Player[A].State == 6)
                                        NPC[numNPCs].Location.Y += 4;
                                }


                                NPC[numNPCs].Active = true;
                                NPC[numNPCs].TimeLeft = 100;
                                NPC[numNPCs].Location.SpeedY = 20;
                                NPC[numNPCs].CantHurt = 100;
                                NPC[numNPCs].CantHurtPlayer = A;
                                NPC[numNPCs].Special = Player[A].Character;
                                if(NPC[numNPCs].Type == NPCID_PLR_FIREBALL)
                                    NPC[numNPCs].Frame = 16;
                                NPC[numNPCs].WallDeath = 5;
                                NPC[numNPCs].Location.SpeedY = 0;
                                NPC[numNPCs].Location.SpeedX = 5 * Player[A].Direction + (Player[A].Location.SpeedX / 3);
                                if(Player[A].State == 6)
                                    NPC[numNPCs].Location.SpeedX = 9 * Player[A].Direction + (Player[A].Location.SpeedX / 3);
                                if(Player[A].StandingOnNPC != 0)
                                    NPC[numNPCs].Location.Y += -Player[A].Location.SpeedY;
                                syncLayers_NPC(numNPCs);
                                CheckSectionNPC(numNPCs);
                            }
                        }
                        else
                            TailSwipe(A, false, true);
                        Player[A].SwordPoke += 1;
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
                        if(!Player[A].Duck && Player[A].Location.SpeedY < Physics.PlayerGravity && Player[A].StandingOnNPC == 0 &&
                            Player[A].Slope == 0 && !Player[A].Controls.Up && !Player[A].Stoned) // Link ducks when jumping
                        {
                            Player[A].SwordPoke = 0;
                            Player[A].Duck = true;
                            Player[A].Location.Y += Player[A].Location.Height;
                            Player[A].Location.Height = Physics.PlayerDuckHeight[Player[A].Character][Player[A].State];
                            Player[A].Location.Y += -Player[A].Location.Height;
                        }
                        else if(Player[A].Duck && Player[A].Location.SpeedY > Physics.PlayerGravity && Player[A].StandingOnNPC == 0 && Player[A].Slope == 0) // Link stands when falling
                        {
                            Player[A].SwordPoke = 0;
                            UnDuck(Player[A]);
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
                Player[A].Location.SpeedX += Player[A].Bumped2;
                Player[A].Location.X += Player[A].Bumped2;
                Player[A].Bumped2 = 0;
                if(Player[A].Mount == 0)
                    Player[A].YoshiYellow = false;

                // When it's true - don't check horizonta' section's bounds
                bool hBoundsHandled = false;

                // level wrap
                if(LevelWrap[Player[A].Section] || LevelVWrap[Player[A].Section])
                {
                    const Screen_t& screen = ScreenByPlayer(A);
                    vScreen_t& vscreen = vScreenByPlayer(A);
                    Location_t& pLoc = Player[A].Location;
                    const Location_t& section = level[Player[A].Section];

                    // track whether screen wrapped in each of the four ways
                    bool did_wrap_lr = false;
                    bool did_wrap_rl = false;
                    bool did_wrap_tb = false;
                    bool did_wrap_bt = false;

                    // horizontally
                    if(LevelWrap[Player[A].Section])
                    {
                        if(pLoc.X + pLoc.Width < section.X)
                        {
                            pLoc.X = section.Width - 1;

                            if(vscreen.Width < section.Width - section.X)
                                did_wrap_lr = true;
                        }
                        else if(pLoc.X > section.Width)
                        {
                            pLoc.X = section.X - pLoc.Width + 1;

                            if(vscreen.Width < section.Width - section.X)
                                did_wrap_rl = true;
                        }

                        hBoundsHandled = true;
                    }

                    // vertically
                    if(LevelVWrap[Player[A].Section])
                    {
                        if(pLoc.Y + pLoc.Height < section.Y)
                        {
                            pLoc.Y = section.Height - 1;

                            if(vscreen.Height < section.Height - section.Y)
                                did_wrap_tb = true;
                        }
                        else if(pLoc.Y > section.Height)
                        {
                            pLoc.Y = section.Y - pLoc.Height + 1;

                            if(vscreen.Height < section.Height - section.Y)
                                did_wrap_bt = true;
                        }
                    }

                    // shared screen: teleport other players to other side of section
                    if(screen.Type == ScreenTypes::SharedScreen && (did_wrap_lr || did_wrap_rl || did_wrap_tb || did_wrap_bt))
                    {
                        double target_Y = pLoc.Y + ((Player[A].Mount != 2) ? pLoc.Height : 0);

                        for(int i = 0; i < screen.player_count; i++)
                        {
                            int o_A = screen.players[i];

                            if(o_A == A)
                                continue;

                            Player_t& o_p = Player[o_A];

                            if(o_p.Dead)
                                continue;

                            // center on player that wrapped
                            o_p.Location.X = pLoc.X + pLoc.Width / 2 - o_p.Location.Width / 2;
                            o_p.Location.Y = target_Y - ((o_p.Mount != 2) ? o_p.Location.Height : 0);

                            // make sure fully in section
                            if(did_wrap_lr)
                            {
                                o_p.Location.X = section.Width - 1;
                                if(o_p.Location.SpeedX > 0)
                                    o_p.Location.SpeedX = 0;
                            }

                            if(did_wrap_rl)
                            {
                                o_p.Location.X = section.X - o_p.Location.Width + 1;
                                if(o_p.Location.SpeedX < 0)
                                    o_p.Location.SpeedX = 0;
                            }

                            if(did_wrap_tb)
                                o_p.Location.Y = section.Height - 1;

                            if(did_wrap_bt)
                                o_p.Location.Y = section.Y - o_p.Location.Height + 1;

                            // following effects only needed for living players
                            if(o_p.TimeToLive != 0)
                                continue;

                            // remove from any pet's mouth that doesn't belong to this screen
                            bool onscreen_pet = InOnscreenPet(o_A, screen);

                            // disable collisions and remove from any offscreen pets
                            if(!onscreen_pet)
                            {
                                RemoveFromPet(o_A);
                                o_p.Effect = 6;
                                o_p.Effect2 = o_p.Location.Y;
                            }
                        }

                        GetvScreenAuto(vscreen);
                    }
                }

                // Walk offscreen exit
                if(!hBoundsHandled && OffScreenExit[Player[A].Section])
                {
                    bool offScreenExit = false;
                    if(Player[A].Location.X + Player[A].Location.Width < level[Player[A].Section].X)
                    {
                        offScreenExit = true;
                        for(B = 1; B <= numPlayers; B++)
                            Player[B].TailCount = 0;
                    }
                    else if(Player[A].Location.X > level[Player[A].Section].Width)
                    {
                        offScreenExit = true;
                    }

                    if(offScreenExit)
                    {
                        // Always quit to the world map by off-screen exit
                        if(!NoMap && !FileRecentSubHubLevel.empty())
                        {
                            FileRecentSubHubLevel.clear();
                            ReturnWarp = 0;
                            ReturnWarpSaved = 0;
                        }

                        LevelBeatCode = 3;
                        EndLevel = true;
                        LevelMacro = LEVELMACRO_OFF;
                        LevelMacroCounter = 0;

                        if(g_config.EnableInterLevelFade)
                            g_levelScreenFader.setupFader(4, 0, 65, ScreenFader::S_FADE);
                        else
                            g_levelScreenFader.setupFader(65, 0, 65, ScreenFader::S_FADE);

                        levelWaitForFade();
                    }

                    hBoundsHandled = true;
                }

                if(LevelMacro == LEVELMACRO_CARD_ROULETTE_EXIT || LevelMacro == LEVELMACRO_GOAL_TAPE_EXIT || GameMenu)
                    hBoundsHandled = true;

                if(!hBoundsHandled)
                {
                    // Check edge of levels
                    if(Player[A].Location.X < level[Player[A].Section].X)
                    {
                        Player[A].Location.X = level[Player[A].Section].X;
                        if(Player[A].Location.SpeedX < 0)
                            Player[A].Location.SpeedX = 0;
                        Player[A].Pinched.Left2 = 2;
                        if(AutoX[Player[A].Section] != 0.0f)
                        {
                            Player[A].Pinched.Moving = 2;
                            Player[A].Pinched.MovingLR = true;
                        }
                    }
                    else if(Player[A].Location.X + Player[A].Location.Width > level[Player[A].Section].Width)
                    {
                        Player[A].Location.X = level[Player[A].Section].Width - Player[A].Location.Width;
                        if(Player[A].Location.SpeedX > 0)
                            Player[A].Location.SpeedX = 0;
                        Player[A].Pinched.Right4 = 2;
                        if(AutoX[Player[A].Section] != 0.f)
                        {
                            Player[A].Pinched.Moving = 2;
                            Player[A].Pinched.MovingLR = true;
                        }
                    }
                }

                if(Player[A].Location.Y < level[Player[A].Section].Y - Player[A].Location.Height - 32 && Player[A].StandingOnTempNPC == 0)
                {
                    Player[A].Location.Y = level[Player[A].Section].Y - Player[A].Location.Height - 32;
                    if(AutoY[Player[A].Section] != 0.f)
                    {
                        Player[A].Pinched.Moving = 3;
                        Player[A].Pinched.MovingUD = true;
                    }
                }

                // gives the players the sparkles when he is flying
                if(
                        (
                                (!Player[A].YoshiBlue && (Player[A].CanFly || Player[A].CanFly2)) ||
                                (Player[A].Mount == 3 && Player[A].CanFly2)
                        ) || Player[A].FlySparks
                        )
                {
                    if(iRand(4) == 0)
                    {
                        NewEffect(EFFID_SPARKLE,
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
                if(Player[A].Pinched.Bottom1 > 0)
                    Player[A].Pinched.Bottom1 -= 1;
                if(Player[A].Pinched.Left2 > 0)
                    Player[A].Pinched.Left2 -= 1;
                if(Player[A].Pinched.Top3 > 0)
                    Player[A].Pinched.Top3 -= 1;
                if(Player[A].Pinched.Right4 > 0)
                    Player[A].Pinched.Right4 -= 1;
                if(Player[A].Pinched.Moving > 0)
                {
                    Player[A].Pinched.Moving -= 1;

                    if(Player[A].Pinched.Moving == 0)
                    {
                        Player[A].Pinched.MovingLR = false;
                        Player[A].Pinched.MovingUD = false;
                    }
                }

                if(Player[A].Effect == 0 && Player[A].Pinched.Strict > 0)
                    Player[A].Pinched.Strict -= 1;

                if(Player[A].Character == 5 && Player[A].Duck && (Player[A].Location.SpeedY == Physics.PlayerGravity || Player[A].StandingOnNPC != 0 || Player[A].Slope != 0))
                {
                    Player[A].Location.Y += Player[A].Location.Height;
                    Player[A].Location.Height = 30;
                    Player[A].Location.Y += -Player[A].Location.Height;
                }


                // block collision optimization
                // fBlock = FirstBlock[(Player[A].Location.X / 32) - 1];
                // lBlock = LastBlock[((Player[A].Location.X + Player[A].Location.Width) / 32.0) + 1];
                // blockTileGet(Player[A].Location, fBlock, lBlock);

                UpdatableQuery<BlockRef_t> q(Player[A].Location, SORTMODE_COMPAT, QUERY_FLBLOCK);

                for(auto it = q.begin(); it != q.end(); ++it)
                {
                    int B = *it;

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

                                        // unclear that this does what Redigit thought; maybe it did with a previous version of the coin switch code

                                        // ' fixes a bug with holding an npc that is really a block
                                        if(Player[A].HoldingNPC > 0)
                                        {
                                            if(NPC[Player[A].HoldingNPC].coinSwitchBlockType > 0)
                                            {
                                                if(NPC[Player[A].HoldingNPC].coinSwitchBlockType == B)
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
                                        if(BlockSlope2[Block[B].Type] != 0 && (Player[A].Location.Y > Block[B].Location.Y || (HitSpot != 2 && HitSpot != 4)) && HitSpot != 1 && !ShadowMode)
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
                                                        PlaySoundSpatial(SFX_BlockHit, Player[A].Location);
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

                                                // if we're already on top of another (higher or more leftwards, at level load time) block this frame, consider canceling it
                                                if(tempHit3 > 0)
                                                {
                                                    // the bug this is fixing is vanilla, but this case happens for a single frame every time a slope falls through ground since TheXTech 1.3.6,
                                                    // and only in the rare case where a slope falls through ground *it was originally below* in vanilla
                                                    if(g_compatibility.fix_player_downward_clip && !CompareWalkBlock(tempHit3, B, Player[A].Location))
                                                    {
                                                        // keep the old block, other conditions are VERY likely to cancel it
                                                    }
                                                    else if(!BlockIsSizable[Block[tempHit3].Type])
                                                    {
                                                        if(Block[tempHit3].Location.Y != Block[B].Location.Y)
                                                            tempHit3 = 0;
                                                    }
                                                    else
                                                    {
                                                        // NOTE: looks like a good place for a vb6-style fEqual
                                                        if(Block[tempHit3].Location.Y == Block[B].Location.Y + Block[B].Location.Height)
                                                            tempHit3 = 0;
                                                    }
                                                }

                                                if(tempHit2)
                                                {
                                                    // NOTE: looks like a good place for a vb6-style fEqual
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
                                                            YoshiPound(A, Player[A].Mount, true);
                                                            Player[A].GroundPound = false;
                                                        }
                                                        else if(Player[A].YoshiYellow)
                                                        {
                                                            if(oldSlope == 0)
                                                                YoshiPound(A, Player[A].Mount);
                                                        }

                                                        Player[A].Location.Y = Block[B].Location.Y + (Block[B].Location.Height * Slope) - Player[A].Location.Height - 0.1;

                                                        if(Player[A].Location.SpeedY > Player[A].Location.SpeedX * (Block[B].Location.Height / static_cast<double>(Block[B].Location.Width)) * BlockSlope[Block[B].Type] || !Player[A].Slide)
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
                                                                    UnDuck(Player[A]);
                                                                Player[A].Slide = true;
                                                            }
                                                        }



                                                        if(Player[A].Location.SpeedY < 0 && !Player[A].Slide && !SuperSpeed && !Player[A].Stoned)
                                                            Player[A].Location.SpeedY = 0;
                                                        if(Block[B].Location.SpeedX != 0.0 || Block[B].Location.SpeedY != 0.0)
                                                        {
                                                            NPC[-A] = blankNPC;
                                                            NPC[-A].Location = Block[B].Location;
                                                            NPC[-A].Type = NPCID_METALBARREL;
                                                            NPC[-A].Active = true;
                                                            NPC[-A].TimeLeft = 100;
                                                            NPC[-A].Section = Player[A].Section;
                                                            NPC[-A].Special = B;
                                                            NPC[-A].Special2 = BlockSlope[Block[B].Type];
                                                            Player[A].StandingOnNPC = -A;
                                                            movingBlock = true;
                                                            // NOTE: Here was a bug that makes compare bool with 0 is always false
                                                            if(
                                                                    (g_compatibility.fix_player_slope_speed &&
                                                                     Player[A].Location.SpeedX - NPC[Player[A].StandingOnNPC].Location.SpeedX < 0 &&
                                                                     BlockSlope[Block[B].Type]/*)*/ < 0) ||
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
                                                    NewEffect(EFFID_SKID_DUST, tempLocation);
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
                                            PlaySoundSpatial(SFX_BlockHit, Player[A].Location);
                                        }

                                        if(BlockCheckPlayerFilter(B, A))  // Optmizied
                                            HitSpot = 0;
                                        //if(Block[B].Type == 626 && Player[A].Character == 1)
                                        //    HitSpot = 0;
                                        //if(Block[B].Type == 627 && Player[A].Character == 2)
                                        //    HitSpot = 0;
                                        //if(Block[B].Type == 628 && Player[A].Character == 3)
                                        //    HitSpot = 0;
                                        //if(Block[B].Type == 629 && Player[A].Character == 4)
                                        //    HitSpot = 0;
                                        //if(Block[B].Type == 632 && Player[A].Character == 5)
                                        //    HitSpot = 0;

                                        if(g_compatibility.fix_player_clip_wall_at_npc && (HitSpot == 5 || HitSpot == 3) && oldStandingOnNpc > 0 && Player[A].Jump)
                                        {
                                            // Re-compute the collision with a block to avoid the unnecessary clipping through the wall
                                            auto pLoc = Player[A].Location;
                                            pLoc.SpeedX += NPC[oldStandingOnNpc].Location.SpeedX;
                                            pLoc.SpeedY += NPC[oldStandingOnNpc].Location.SpeedY;
                                            HitSpot = FindRunningCollision(pLoc, Block[B].Location);
                                            D_pLogDebug("Conveyor: Recomputed collision with block %d", B);
                                        }

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

                                            Player[A].Pinched.Bottom1 = 2; // for players getting squashed

                                            if(Block[B].Location.SpeedY != 0)
                                            {
                                                Player[A].Pinched.Moving = 2;
                                                Player[A].Pinched.MovingUD = true;
                                            }

                                            Player[A].Vine = 0; // stop climbing because you are now walking
                                            if(Player[A].Mount == 2) // for the clown car, make a niose and pound the ground if moving down fast enough
                                            {
                                                if(Player[A].Location.SpeedY > 3)
                                                {
                                                    PlaySoundSpatial(SFX_Stone, Player[A].Location);
                                                    YoshiPound(A, Player[A].Mount, true);
                                                }
                                            }

                                            if(tempHit3 == 0) // For walking
                                            {
                                                tempHit3 = B;
                                                tempLocation3 = Block[B].Location;
                                            }
                                            else // Find the best block to walk on if touching multiple blocks
                                            {
                                                if(g_compatibility.fix_player_downward_clip)
                                                {
                                                    if(CompareWalkBlock(tempHit3, B, Player[A].Location))
                                                    {
                                                        tempHit3 = B;
                                                        tempLocation3 = Block[B].Location;
                                                    }
                                                }
                                                else // Using old code
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

                                                        C += -(Player[A].Location.X + Player[A].Location.Width * 0.5);
                                                        D += -(Player[A].Location.X + Player[A].Location.Width * 0.5);
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
                                                Player[A].Pinched.Left2 = 2;
                                                if(Block[B].Location.SpeedX != 0)
                                                {
                                                    Player[A].Pinched.Moving = 2;
                                                    Player[A].Pinched.MovingLR = true;
                                                }
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
                                            Player[A].Pinched.Right4 = 2;
                                            if(Block[B].Location.SpeedX != 0)
                                            {
                                                Player[A].Pinched.Moving = 2;
                                                Player[A].Pinched.MovingLR = true;
                                            }
                                        }
                                        else if(HitSpot == 3) // hit the block from below
                                        {
                                            // add more generous margin to prevent unfair crush death with sloped ceiling
                                            bool ignore = (g_compatibility.fix_player_crush_death
                                                && (Block[B].Location.X + Block[B].Location.Width - 2 < Player[A].Location.X
                                                    || Player[A].Location.X + Player[A].Location.Width - 2 < Block[B].Location.X));

                                            if(!Player[A].ForceHitSpot3 && !Player[A].StandUp && !ignore)
                                                Player[A].Pinched.Top3 = 2;

                                            if(Block[B].Location.SpeedY != 0 && !ignore)
                                            {
                                                Player[A].Pinched.Moving = 2;
                                                Player[A].Pinched.MovingUD = true;
                                            }

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
                                                    Player[A].Pinched.Right4 = 2;
                                                else
                                                    Player[A].Pinched.Left2 = 2;
                                                if(Block[B].Location.SpeedX != 0 || Block[B].Location.SpeedY != 0)
                                                {
                                                    Player[A].Pinched.Moving = 2;

                                                    if(Block[B].Location.SpeedX != 0)
                                                        Player[A].Pinched.MovingLR = true;

                                                    if(Block[B].Location.SpeedY != 0)
                                                        Player[A].Pinched.MovingUD = true;
                                                }
                                                tempLocation.X = Player[A].Location.X;
                                                tempLocation.Width = Player[A].Location.Width;
                                                tempLocation.Y = Player[A].Location.Y + Player[A].Location.Height;
                                                tempLocation.Height = 0.1;
                                                tempBool = false;

                                                // this could have caused an unusual TheXTech bug where lBlock would get overwritten
                                                // (this wouldn't affect VB6 because loop bounds are evaluated only on loop start)

                                                // fBlock = FirstBlock[(tempLocation.X / 32) - 1];
                                                // lBlock = LastBlock[((tempLocation.X + tempLocation.Width) / 32.0) + 1];
                                                // blockTileGet(tempLocation, fBlock, lBlock);

                                                for(int C : treeFLBlockQuery(tempLocation, SORTMODE_COMPAT))
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
                                                    Player[A].Location.X += -4 * Player[A].Direction;
                                                    Player[A].Location.Y += -Player[A].Location.SpeedY;
                                                    Player[A].Location.SpeedX = 0;
                                                    Player[A].Location.SpeedY = 0;

                                                    q.update(Player[A].Location, it);
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
                    Player[A].Location.Y += Player[A].Location.Height;
                    Player[A].Location.Height = Physics.PlayerDuckHeight[Player[A].Character][Player[A].State];
                    Player[A].Location.Y += -Player[A].Location.Height;
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
                            Player[A].Location.SpeedX += -NPC[Player[A].StandingOnNPC].Location.SpeedX;
                            movingBlock = false;
                            Player[A].StandingOnNPC = 0;
                        }
                    }

                    // diggable dirt
                    if(Block[tempHit3].Type == 370 && Player[A].StandingOnNPC <= 0) // dig dirt
                    {
                        DontResetGrabTime = true;
                        B = tempHit3;
                        if(Player[A].TailCount == 0 && Player[A].Controls.Down && Player[A].Controls.Run && Player[A].Mount == 0 && !Player[A].Stoned && Player[A].HoldingNPC == 0 && (Player[A].GrabTime > 0 || Player[A].RunRelease))
                        {
                            if((Player[A].GrabTime >= 12 && Player[A].Character < 3) || (Player[A].GrabTime >= 16 && Player[A].Character == 3) || (Player[A].GrabTime >= 8 && Player[A].Character == 4))
                            {
                                Player[A].Location.SpeedX = Player[A].GrabSpeed;
                                Player[A].GrabSpeed = 0;
                                Block[B].Hidden = true;
                                Block[B].Layer = LAYER_DESTROYED_BLOCKS;
                                syncLayersTrees_Block(B);
                                NewEffect(EFFID_SMOKE_S3, Block[B].Location);
                                Effect[numEffects].Location.SpeedY = -2;
                                Player[A].GrabTime = 0;
                            }
                            else
                            {
                                if(Player[A].GrabTime == 0)
                                {
                                    PlaySoundSpatial(SFX_Grab, Player[A].Location);
                                    Player[A].FrameCount = 0;
                                    Player[A].GrabSpeed = Player[A].Location.SpeedX;
                                }
                                Player[A].Location.SpeedX = 0;
                                Player[A].Slide = false;
                                Player[A].GrabTime += 1;
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
                                YoshiPound(A, Player[A].Mount, true);
                                Player[A].GroundPound = false;
                            }
                            else if(Player[A].YoshiYellow)
                                YoshiPound(A, Player[A].Mount);

                            Player[A].Location.SpeedY = 0;
                            if(tempLocation3.SpeedX != 0.0 || tempLocation3.SpeedY != 0.0)
                            {
                                NPC[-A] = blankNPC;
                                NPC[-A].Location = tempLocation3;
                                NPC[-A].Type = NPCID_METALBARREL;
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
                                PlaySoundSpatial(SFX_BlockHit, Player[A].Location);
                                if(Player[A].Controls.Jump || Player[A].Controls.AltJump)
                                {
                                    PlaySoundSpatial(SFX_Jump, Player[A].Location);
                                    Player[A].Jump = Physics.PlayerBlockJumpHeight;
                                    if(Player[A].Character == 2)
                                        Player[A].Jump += 3;
                                    if(Player[A].SpinJump)
                                        Player[A].Jump -= 6;
                                }
                            }

                            if(Player[A].SpinJump && (Block[tempHit3].Type == 90 || Block[tempHit3].Type == 526) && Player[A].State > 1 && Block[tempHit3].Special == 0)
                            {
                                Player[A].Location.SpeedY = Physics.PlayerJumpVelocity;
                                Block[tempHit3].Kill = true;
                                iBlocks += 1;
                                iBlock[iBlocks] = tempHit3;
                                HitSpot = 0;
                                tempHit3 = 0;
                                Player[A].Jump = 7;

                                if(Player[A].Character == 2)
                                    Player[A].Jump += 3;

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
                            YoshiPound(A, Player[A].Mount, true);
                            Player[A].GroundPound = false;
                        }
                        else if(Player[A].YoshiYellow)
                            YoshiPound(A, Player[A].Mount);

                        if(Player[A].Slope == 0 || Player[A].Slide)
                            Player[A].Location.SpeedY = 0;

                        if(tempLocation3.SpeedX != 0.0 || tempLocation3.SpeedY != 0.0)
                        {
                            NPC[-A] = blankNPC;
                            NPC[-A].Location = tempLocation3;
                            NPC[-A].Type = NPCID_METALBARREL;
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
                            // the single Pinched variable has always been false since SMBX64
                            // if(!NPC[Player[A].StandingOnNPC].Pinched && !FreezeNPCs)
                            if(!FreezeNPCs)
                                Player[A].Location.SpeedX += -NPC[Player[A].StandingOnNPC].Location.SpeedX - NPC[Player[A].StandingOnNPC].BeltSpeed;
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
                            PlaySoundSpatial(SFX_BlockHit, Player[A].Location);
                            if(Player[A].Controls.Jump || Player[A].Controls.AltJump)
                            {
                                PlaySoundSpatial(SFX_Jump, Player[A].Location);
                                Player[A].Jump = Physics.PlayerBlockJumpHeight;
                                if(Player[A].Character == 2)
                                    Player[A].Jump += 3;
                                if(Player[A].SpinJump)
                                    Player[A].Jump -= 6;
                            }
                        }
                        if(Player[A].SpinJump && (Block[tempHit3].Type == 90 || Block[tempHit3].Type == 526) && Player[A].State > 1 && Block[tempHit3].Special == 0)
                        {
                            Player[A].Location.SpeedY = Physics.PlayerJumpVelocity;
                            Block[tempHit3].Kill = true;
                            iBlocks += 1;
                            iBlock[iBlocks] = tempHit3;
                            tempHit3 = 0;
                            Player[A].Jump = 7;
                            if(Player[A].Character == 2)
                                Player[A].Jump += 3;
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
                            Player[A].Location.SpeedX += blockPushX;
                    }
                    else if(Player[A].Location.SpeedX + NPC[Player[A].StandingOnNPC].Location.SpeedX < 0 && Player[A].Controls.Left)
                    {
                        Player[A].Location.SpeedX = 0.2 * Player[A].Direction;
                        if(blockPushX < 0)
                            Player[A].Location.SpeedX += blockPushX;
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
                    C += -(Player[A].Location.X + Player[A].Location.Width * 0.5);
                    D += -(Player[A].Location.X + Player[A].Location.Width * 0.5);
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

                    PlaySoundSpatial(SFX_BlockHit, Player[A].Location);
                    Player[A].Jump = 0;
                    Player[A].Location.Y = Block[B].Location.Y + Block[B].Location.Height + 0.01;
                    Player[A].Location.SpeedY = -0.01 + Block[B].Location.SpeedY;
                    if(Player[A].Fairy)
                        Player[A].Location.SpeedY = 2;
                    if(Player[A].Vine > 0)
                        Player[A].Location.Y += 0.1;
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
                for(int B : treeBackgroundQuery(Player[A].Location, SORTMODE_NONE))
                {
                    if(B > numBackground)
                        continue;

                    if(BackgroundFence[Background[B].Type] && (!g_compatibility.fix_climb_invisible_fences || !Background[B].Hidden))
                    {
                        // FIXME: remove 4 spaces indention as soon as possible from this block below to the next commented brace
                        // if(CheckCollision(Player[A].Location, Background[B].Location))
                        //{
                            tempLocation = Background[B].Location;
                            tempLocation.Height -= 16;
                            tempLocation.Width -= 20;
                            tempLocation.X += 10;

                            if(CheckCollision(Player[A].Location, tempLocation))
                            {
                                if(Player[A].Character == 5)
                                {
                                    bool hasNoMonts = (g_compatibility.fix_char5_vehicle_climb && Player[A].Mount <= 0) ||
                                                       !g_compatibility.fix_char5_vehicle_climb;
                                    if(hasNoMonts && Player[A].Immune == 0 && Player[A].Controls.Up)
                                    {
                                        Player[A].FairyCD = 0;

                                        if(!Player[A].Fairy)
                                        {
                                            Player[A].Fairy = true;
                                            SizeCheck(Player[A]);
                                            PlaySoundSpatial(SFX_HeroFairy, Player[A].Location);
                                            Player[A].Immune = 10;
                                            Player[A].Effect = 8;
                                            Player[A].Effect2 = 4;
                                            NewEffect(EFFID_SMOKE_S5, Player[A].Location);
                                        }

                                        if(Player[A].FairyTime != -1 && Player[A].FairyTime < 20)
                                            Player[A].FairyTime = 20;
                                    }

                                    break;
                                }
                                else if(!Player[A].Fairy && !Player[A].Stoned)
                                {
                                    if(Player[A].Mount == 0 && Player[A].HoldingNPC <= 0)
                                    {
                                        if(Player[A].Vine > 0)
                                        {
                                            if(Player[A].Duck)
                                                UnDuck(Player[A]);
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
                                                UnDuck(Player[A]);
                                            if(Player[A].Location.Y >= Background[B].Location.Y - 20 && Player[A].Vine < 2)
                                                Player[A].Vine = 2;
                                            if(Player[A].Location.Y >= Background[B].Location.Y - 18)
                                                Player[A].Vine = 3;
                                        }

                                        if(Player[A].Vine > 0)
                                        {
                                            Player[A].VineNPC = -1;
                                            if(g_compatibility.fix_climb_bgo_speed_adding)
                                                Player[A].VineBGO = B;
                                        }

                                        if(Player[A].Vine == 3)
                                            break;
                                    }
                                } // !Fairy & !Stoned
                            } // Collide player and temp location
                        // }// Collide player and BGO
                    } // Is BGO climbable and visible?
                } // Next A

                if(Player[A].StandingOnNPC != 0)
                {
                    if(!tempHit2)
                    {
                        // the single Pinched variable has always been false since SMBX64
                        // if(!NPC[Player[A].StandingOnNPC].Pinched && !FreezeNPCs)
                        if(!FreezeNPCs)
                            Player[A].Location.SpeedX += -NPC[Player[A].StandingOnNPC].Location.SpeedX - NPC[Player[A].StandingOnNPC].BeltSpeed;
                    }
                }
                tempHit = false; // Used for JUMP detection
                tempHit2 = false;

                for(int B : treeNPCQuery(Player[A].Location, SORTMODE_ID))
                {
                    if(NPC[B].Active && NPC[B].Killed == 0 && NPC[B].Effect != 5 && NPC[B].Effect != 6)
                    {
                        // If Not (NPC(B).Type = 17 And NPC(B).CantHurt > 0) And Not (.Mount = 2 And NPC(B).Type = 56) And Not NPC(B).vehiclePlr = A And Not NPC(B).Type = 197 And Not NPC(B).Type = 237 Then
                        if(!(Player[A].Mount == 2 && NPC[B].Type == NPCID_VEHICLE) &&
                            NPC[B].vehiclePlr != A &&
                            NPC[B].Type != NPCID_GOALTAPE &&
                            NPC[B].Type != NPCID_ICE_BLOCK
                        )
                        {
                            if(NPC[B].HoldingPlayer == 0 || NPC[B]->IsABonus || (BattleMode && NPC[B].HoldingPlayer != A))
                            {
                                if(CheckCollision(Player[A].Location, NPC[B].Location))
                                {
                                    if((NPC[B].Type == NPCID_METALBARREL || NPC[B].Type == NPCID_CANNONENEMY || NPC[B].Type == NPCID_HPIPE_SHORT || NPC[B].Type == NPCID_HPIPE_LONG || NPC[B].Type == NPCID_VPIPE_SHORT || NPC[B].Type == NPCID_VPIPE_LONG) && NPC[B].Projectile)
                                        PlayerHurt(A);
                                    if((Player[A].Mount == 1 || Player[A].Mount == 3 || Player[A].SpinJump || (Player[A].ShellSurf && NPC[B]->IsAShell) || (Player[A].Stoned && !NPC[B]->CanWalkOn)) && !NPC[B]->MovesPlayer)
                                        HitSpot = BootCollision(Player[A].Location, NPC[B].Location, NPC[B]->CanWalkOn); // find the hitspot for normal mario
                                    else
                                        HitSpot = EasyModeCollision(Player[A].Location, NPC[B].Location, NPC[B]->CanWalkOn); // find the hitspot when in a shoe or on a yoshi

                                    if(!NPC[B].Inert)
                                    {
                                        // battlemode stuff
                                        if(NPC[B].Type == NPCID_PLR_FIREBALL || NPC[B].Type == NPCID_PLR_HEAVY || NPC[B].Type == NPCID_PLR_ICEBALL || NPC[B].Type == NPCID_SWORDBEAM || NPC[B].Type == NPCID_PET_FIRE || NPC[B].Type == NPCID_CHAR3_HEAVY || NPC[B].Type == NPCID_CHAR4_HEAVY)
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
                                                        if(NPC[B].Type == NPCID_SWORDBEAM)
                                                            PlaySoundSpatial(SFX_HeroHit, Player[A].Location);
                                                    }
                                                    PlayerHurt(A);
                                                }
                                            }
                                            HitSpot = 0;
                                        }
                                        if(NPC[B].Type == NPCID_BULLET && NPC[B].CantHurt > 0)
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
                                        if((NPC[B].Type == NPCID_TOOTHY || NPC[B].Type == NPCID_HEAVY_THROWN) && BattleMode && NPC[B].CantHurtPlayer != A)
                                            PlayerHurt(A);
                                        if((NPC[B].Type == NPCID_ICE_CUBE || NPC[B].Type == NPCID_ITEM_POD) && BattleMode &&
                                            NPC[B].CantHurtPlayer != A && NPC[B].Projectile != 0 && NPC[B].BattleOwner != A)
                                        {
                                            if(Player[A].Immune == 0 && NPC[B].Type == NPCID_ITEM_POD)
                                                NPC[B].Special2 = 1;
                                            PlayerHurt(A);
                                            HitSpot = 0;
                                        }
                                        if((NPC[B]->IsAShell || NPCIsVeggie(NPC[B]) ||
                                            NPC[B].Type == NPCID_ICE_CUBE || NPC[B].Type == NPCID_SLIDE_BLOCK) &&
                                            BattleMode && NPC[B].HoldingPlayer > 0 && NPC[B].HoldingPlayer != A)
                                        {
                                            if(Player[A].Immune == 0)
                                            {
                                                PlayerHurt(A);
                                                NPCHit(B, 5, B);
                                            }
                                        }
                                        if(NPCIsAParaTroopa(NPC[B]) && BattleMode && NPC[B].CantHurtPlayer == A)
                                            HitSpot = 0;
                                        if(BattleMode && NPCIsVeggie(NPC[B]) && NPC[B].Projectile != 0)
                                        {
                                            if(NPC[B].CantHurtPlayer != A)
                                            {
                                                if(Player[A].Immune == 0)
                                                {
                                                    PlayerHurt(A);
                                                    NPCHit(B, 4, B);
                                                    PlaySoundSpatial(SFX_SpitBossHit, Player[A].Location);
                                                }
                                            }
                                        }

                                        if(BattleMode && NPC[B].HoldingPlayer > 0 && NPC[B].HoldingPlayer != A)
                                        {
                                            if(NPC[B]->WontHurt)
                                                HitSpot = 0;
                                            else
                                                HitSpot = 5;
                                        }

                                        if(BattleMode && NPC[B].BattleOwner != A && NPC[B].Projectile != 0 && NPC[B].CantHurtPlayer != A)
                                        {
                                            if(NPC[B].Type == NPCID_BOMB || NPC[B].Type == NPCID_LIT_BOMB_S3 || NPC[B].Type == NPCID_CARRY_BLOCK_A || NPC[B].Type == NPCID_CARRY_BLOCK_B || NPC[B].Type == NPCID_CARRY_BLOCK_C || NPC[B].Type == NPCID_CARRY_BLOCK_D || NPC[B].Type == NPCID_HIT_CARRY_FODDER || ((NPC[B]->IsAShell || NPC[B].Type == NPCID_SLIDE_BLOCK) && NPC[B].Location.SpeedX == 0))
                                            {
                                                if(NPC[B]->IsAShell && HitSpot == 1 && Player[A].SpinJump)
                                                {
                                                }
                                                else if(Player[A].Immune == 0)
                                                {
                                                    if(NPC[B].Type != NPCID_SLIDE_BLOCK && !NPC[B]->IsAShell)
                                                        NPCHit(B, 3, B);
                                                    PlayerHurt(A);
                                                    HitSpot = 0;
                                                }
                                            }
                                        }
                                        // end battlemode
                                    }

                                    if(NPC[B].Type == NPCID_ITEM_BUBBLE)
                                    {
                                        NPCHit(B, 1, A);
                                        HitSpot = 0;
                                    }

                                    if(GameOutro)
                                        HitSpot = 0;
                                    if(NPC[B].Type == NPCID_HEAVY_THROWN && NPC[B].CantHurt > 0)
                                        HitSpot = 0;
                                    if(NPC[B].Type == NPCID_ITEM_POD && HitSpot == 1)
                                        HitSpot = 0;
                                    if(NPC[B].Inert) // if the npc is friendly then you can't touch it
                                    {
                                        HitSpot = 0;
                                        if(NPC[B].Text != STRINGINDEX_NONE && Player[A].Controls.Up && !FreezeNPCs)
                                            MessageNPC = B;
                                    }
                                    if(Player[A].Stoned && HitSpot != 1) // if you are a statue then SLAM into the npc
                                    {
                                        if(Player[A].Location.SpeedX > 3 || Player[A].Location.SpeedX < -3)
                                            NPCHit(B, 3, B);
                                    }

                                    // the following code is for spin jumping and landing on things as yoshi/shoe
                                    if(Player[A].Mount == 1 || Player[A].Mount == 3 || Player[A].SpinJump ||
                                       (Player[A].Stoned && !NPC[B]->CanWalkOn))
                                    {
                                        if(HitSpot == 1)
                                        {
                                            if(Player[A].Mount == 1 || Player[A].Mount == 2 || Player[A].Stoned)
                                                NPCHit(B, 8, A);
                                            else if(!(NPC[B].Type == NPCID_FIRE_PLANT || NPC[B].Type == NPCID_QUAD_SPITTER || NPC[B].Type == NPCID_PLANT_S3 || NPC[B].Type == NPCID_LAVABUBBLE ||
                                                      NPC[B].Type == NPCID_SPIKY_S3 || NPC[B].Type == NPCID_SPIKY_S4 || NPC[B].Type == NPCID_SPIKY_BALL_S4 || NPC[B].Type == NPCID_BOTTOM_PLANT ||
                                                      NPC[B].Type == NPCID_SIDE_PLANT || NPC[B].Type == NPCID_CRAB || NPC[B].Type == NPCID_FLY || NPC[B].Type == NPCID_BIG_PLANT ||
                                                      NPC[B].Type == NPCID_PLANT_S1 || NPC[B].Type == NPCID_VILLAIN_S1 || NPC[B].Type == NPCID_WALL_BUG || NPC[B].Type == NPCID_WALL_TURTLE ||
                                                      NPC[B].Type == NPCID_SICK_BOSS || NPC[B].Type == NPCID_WALK_PLANT || NPC[B].Type == NPCID_JUMP_PLANT) && !NPC[B]->CanWalkOn)
                                            {
                                                if(Player[A].Wet > 0 && (NPC[B]->IsFish || NPC[B].Type == NPCID_SQUID_S3 || NPC[B].Type == NPCID_SQUID_S1))
                                                {
                                                }
                                                else
                                                    NPCHit(B, 8, A);
                                            }

                                            if(NPC[B].Killed == 8 || NPC[B]->IsFish || NPC[B].Type == NPCID_SAW ||
                                               NPC[B].Type == NPCID_STONE_S3 || NPC[B].Type == NPCID_STONE_S4 || NPC[B].Type == NPCID_GHOST_S3 ||
                                               NPC[B].Type == NPCID_GHOST_FAST || NPC[B].Type == NPCID_GHOST_S4 || NPC[B].Type == NPCID_BIG_GHOST ||
                                               NPC[B].Type == NPCID_PLANT_S3 || NPC[B].Type == NPCID_LAVABUBBLE || NPC[B].Type == NPCID_SPIKY_S3 ||
                                               NPC[B].Type == NPCID_BOTTOM_PLANT || NPC[B].Type == NPCID_SIDE_PLANT || NPC[B].Type == NPCID_CRAB ||
                                               NPC[B].Type == NPCID_FLY || NPC[B].Type == NPCID_BIG_PLANT || NPC[B].Type == NPCID_PLANT_S1 ||
                                               NPC[B].Type == NPCID_VILLAIN_S1 || NPC[B].Type == NPCID_WALL_BUG || NPC[B].Type == NPCID_WALL_TURTLE ||
                                               NPC[B].Type == NPCID_SICK_BOSS || NPC[B].Type == NPCID_LAVA_MONSTER || NPC[B].Type == NPCID_FIRE_PLANT ||
                                               NPC[B].Type == NPCID_LONG_PLANT_UP || NPC[B].Type == NPCID_WALK_PLANT || NPC[B].Type == NPCID_QUAD_SPITTER ||
                                               NPC[B].Type == NPCID_SPIKY_S4 || NPC[B].Type == NPCID_SPIKY_BALL_S4 || NPC[B].Type == NPCID_JUMP_PLANT) // tap
                                            {
                                                if(NPC[B].Killed == 8 && Player[A].Mount == 1 && Player[A].MountType == 2)
                                                {
                                                    numNPCs++;
                                                    NPC[numNPCs] = NPC_t();
                                                    NPC[numNPCs].Active = true;
                                                    NPC[numNPCs].TimeLeft = 100;
                                                    NPC[numNPCs].Section = Player[A].Section;
                                                    NPC[numNPCs].Type = NPCID_PLR_FIREBALL;
                                                    NPC[numNPCs].Special = Player[A].Character;
                                                    NPC[numNPCs].Location.Height = NPC[numNPCs]->THeight;
                                                    NPC[numNPCs].Location.Width = NPC[numNPCs]->TWidth;
                                                    NPC[numNPCs].Location.Y = Player[A].Location.Height + Player[A].Location.Y - NPC[numNPCs].Location.Height;
                                                    NPC[numNPCs].Location.X = Player[A].Location.X + Player[A].Location.Width / 2.0 - NPC[numNPCs].Location.Width / 2.0;
                                                    NPC[numNPCs].Location.SpeedX = 4;
                                                    NPC[numNPCs].Location.SpeedY = 10;
                                                    syncLayers_NPC(numNPCs);
                                                    numNPCs++;
                                                    NPC[numNPCs] = NPC_t();
                                                    NPC[numNPCs].Active = true;
                                                    NPC[numNPCs].TimeLeft = 100;
                                                    NPC[numNPCs].Section = Player[A].Section;
                                                    NPC[numNPCs].Type = NPCID_PLR_FIREBALL;
                                                    NPC[numNPCs].Special = Player[A].Character;
                                                    NPC[numNPCs].Location.Height = NPC[numNPCs]->THeight;
                                                    NPC[numNPCs].Location.Width = NPC[numNPCs]->TWidth;
                                                    NPC[numNPCs].Location.Y = Player[A].Location.Height + Player[A].Location.Y - NPC[numNPCs].Location.Height;
                                                    NPC[numNPCs].Location.X = Player[A].Location.X + Player[A].Location.Width / 2.0 - NPC[numNPCs].Location.Width / 2.0;
                                                    NPC[numNPCs].Location.SpeedX = -4;
                                                    NPC[numNPCs].Location.SpeedY = 10;
                                                    syncLayers_NPC(numNPCs);
                                                }
                                                if(NPC[B].Killed == 0 && Player[A].SpinJump == 0)
                                                    PlaySoundSpatial(SFX_Stomp, Player[A].Location);
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
                                                            PlaySoundSpatial(SFX_Stomp, Player[A].Location);
                                                    }
                                                }
                                                HitSpot = 0;
                                            }
                                        }
                                    }
                                    else if(Player[A].Mount == 2)
                                    {
                                        if(NPC[B].vehiclePlr == A)
                                            HitSpot = 0;
                                        else if(!(NPC[B].Type == NPCID_BULLET && NPC[B].CantHurt > 0))
                                        {
                                            if((NPC[B].Location.Y + NPC[B].Location.Height > Player[A].Location.Y + 18 && HitSpot != 3) || HitSpot == 1)
                                            {
                                                NPCHit(B, 8, A);
                                                if(NPC[B].Killed == 8)
                                                    HitSpot = 0;

                                                if(NPC[B].Type == NPCID_WALK_BOMB_S2 || NPC[B].Type == NPCID_WALK_BOMB_S3 || NPC[B].Type == NPCID_LIT_BOMB_S3)
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
                                        if(NPC[B].Type == NPCID_STATUE_FIRE || NPC[B].Type == NPCID_VILLAIN_FIRE || NPC[B].Type == NPCID_PLANT_FIRE || NPC[B].Type == NPCID_QUAD_BALL)
                                        {
                                            PlaySoundSpatial(SFX_BlockHit, Player[A].Location);
                                            HitSpot = 0;
                                            NPC[B].Killed = 9;
                                            NPCQueues::Killed.push_back(B);
                                            for(C = 1; C <= 10; ++C)
                                            {
                                                NewEffect(EFFID_PLR_FIREBALL_TRAIL, NPC[B].Location, static_cast<float>(NPC[B].Special));
                                                Effect[numEffects].Location.SpeedX = dRand() * 3 - 1.5 + NPC[B].Location.SpeedX * 0.1;
                                                Effect[numEffects].Location.SpeedY = dRand() * 3 - 1.5 - NPC[B].Location.SpeedY * 0.1;
                                                if(Effect[numEffects].Frame == 0)
                                                    Effect[numEffects].Frame = -iRand(3);
                                                else
                                                    Effect[numEffects].Frame = 5 + iRand(3);
                                            }
                                            NPC[B].Location.X += NPC[B].Location.Width / 2.0 - EffectWidth[10] / 2.0;
                                            NPC[B].Location.Y += NPC[B].Location.Height / 2.0 - EffectHeight[10] / 2.0;
                                            NewEffect(EFFID_SMOKE_S3, NPC[B].Location);

                                            treeNPCUpdate(B);
                                        }
                                    }


                                    if(NPC[B]->IsAVine) // if the player collided with a vine then see if he should climb it
                                    {
                                        if(Player[A].Character == 5)
                                        {
                                            bool hasNoMonts = (g_compatibility.fix_char5_vehicle_climb && Player[A].Mount <= 0) ||
                                                               !g_compatibility.fix_char5_vehicle_climb;
                                            if(hasNoMonts && Player[A].Immune == 0 && Player[A].Controls.Up)
                                            {
                                                Player[A].FairyCD = 0;
                                                if(!Player[A].Fairy)
                                                {
                                                    Player[A].Fairy = true;
                                                    SizeCheck(Player[A]);
                                                    PlaySoundSpatial(SFX_HeroFairy, Player[A].Location);
                                                    Player[A].Immune = 10;
                                                    Player[A].Effect = 8;
                                                    Player[A].Effect2 = 4;
                                                    NewEffect(EFFID_SMOKE_S5, Player[A].Location);
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
                                                        UnDuck(Player[A]);
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
                                                        UnDuck(Player[A]);
                                                    if(Player[A].Location.Y >= NPC[B].Location.Y - 20 && Player[A].Vine < 2)
                                                        Player[A].Vine = 2;
                                                    if(Player[A].Location.Y >= NPC[B].Location.Y - 18)
                                                        Player[A].Vine = 3;
                                                }

                                                if(Player[A].Vine > 0)
                                                {
                                                    Player[A].VineNPC = B;
                                                    if(g_compatibility.fix_climb_bgo_speed_adding)
                                                        Player[A].VineBGO = 0.0;
                                                }
                                            }
                                        }
                                    }

                                    // subcon warps
                                    if(NPC[B].Type == NPCID_MAGIC_DOOR && HitSpot > 0 && Player[A].Controls.Up)
                                    {
                                        if(NPC[B].Special2 >= 0)
                                        {
                                            NPC[B].Killed = 9;
                                            NPCQueues::Killed.push_back(B);
                                            PlaySoundSpatial(SFX_Door, Player[A].Location);
                                            Player[A].Effect = 7;
                                            Player[A].Warp = numWarps + 1;
                                            Player[A].WarpBackward = false;
                                            Warp[numWarps + 1].Entrance = static_cast<SpeedlessLocation_t>(NPC[B].Location);
                                            tempLocation = NPC[B].Location;
                                            tempLocation.X = NPC[B].Location.X - level[Player[A].Section].X + level[NPC[B].Special2].X;
                                            tempLocation.Y = NPC[B].Location.Y - level[Player[A].Section].Y + level[NPC[B].Special2].Y;
                                            Warp[numWarps + 1].Exit = static_cast<SpeedlessLocation_t>(tempLocation);
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
                                            tempLocation = static_cast<Location_t>(Warp[numWarps + 1].Entrance);
                                            tempLocation.Y -= 32;
                                            tempLocation.Height = 64;
                                            NewEffect(EFFID_DOOR_S2_OPEN, tempLocation);
                                            tempLocation = static_cast<Location_t>(Warp[numWarps + 1].Exit);
                                            tempLocation.Y -= 32;
                                            tempLocation.Height = 64;
                                            NewEffect(EFFID_DOOR_S2_OPEN, tempLocation);
                                        }
                                    }


                                    if(HitSpot == 1 && (NPC[B].Type == NPCID_COIN_SWITCH || NPC[B].Type == NPCID_TIME_SWITCH || NPC[B].Type == NPCID_TNT) && NPC[B].Projectile != 0)
                                        HitSpot = 0;

                                    if(NPC[B].Type == NPCID_LOCK_DOOR && Player[A].HasKey)
                                    {
                                        Player[A].HasKey = false;
                                        HitSpot = 0;
                                        NPC[B].Killed = 3;
                                        NPCQueues::Killed.push_back(B);
                                    }

                                    if(NPC[B].Type == NPCID_SLIDE_BLOCK && NPC[B].Projectile != 0 && HitSpot > 1)
                                        HitSpot = 5;

                                    if(HitSpot == 1) // Player landed on a NPC
                                    {
                                        if(NPC[B]->CanWalkOn || (Player[A].ShellSurf && NPC[B]->IsAShell)) // NPCs that can be walked on
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
                                                C += -(Player[A].Location.X + Player[A].Location.Width * 0.5);
                                                D += -(Player[A].Location.X + Player[A].Location.Width * 0.5);
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
                                        else if((NPCIsYoshi(NPC[B]) || NPCIsBoot(NPC[B])) && Player[A].Character != 5 && !Player[A].Fairy)
                                        {
                                            if(Player[A].Mount == 0 && NPC[B].CantHurtPlayer != A && Player[A].Dismount == 0)
                                            {
                                                if(NPCIsBoot(NPC[B]))
                                                {
                                                    UnDuck(Player[A]);
                                                    NPC[B].Killed = 9;
                                                    NPCQueues::Killed.push_back(B);
                                                    if(Player[A].State == 1)
                                                    {
                                                        Player[A].Location.Height = Physics.PlayerHeight[1][2];
                                                        Player[A].Location.Y += -Physics.PlayerHeight[1][2] + Physics.PlayerHeight[Player[A].Character][1];
                                                    }
                                                    Player[A].Mount = 1;
                                                    if(NPC[B].Type == NPCID_GRN_BOOT)
                                                        Player[A].MountType = 1;
                                                    if(NPC[B].Type == NPCID_RED_BOOT)
                                                        Player[A].MountType = 2;
                                                    if(NPC[B].Type == NPCID_BLU_BOOT)
                                                        Player[A].MountType = 3;
                                                    PlaySoundSpatial(SFX_Stomp, Player[A].Location);
                                                }
                                                else if(NPCIsYoshi(NPC[B]) && (Player[A].Character == 1 || Player[A].Character == 2))
                                                {
                                                    UnDuck(Player[A]);
                                                    NPC[B].Killed = 9;
                                                    NPCQueues::Killed.push_back(B);
                                                    Player[A].Mount = 3;
                                                    if(NPC[B].Type == NPCID_PET_GREEN)
                                                        Player[A].MountType = 1;
                                                    else if(NPC[B].Type == NPCID_PET_BLUE)
                                                        Player[A].MountType = 2;
                                                    else if(NPC[B].Type == NPCID_PET_YELLOW)
                                                        Player[A].MountType = 3;
                                                    else if(NPC[B].Type == NPCID_PET_RED)
                                                        Player[A].MountType = 4;
                                                    else if(NPC[B].Type == NPCID_PET_BLACK)
                                                        Player[A].MountType = 5;
                                                    else if(NPC[B].Type == NPCID_PET_PURPLE)
                                                        Player[A].MountType = 6;
                                                    else if(NPC[B].Type == NPCID_PET_PINK)
                                                        Player[A].MountType = 7;
                                                    else if(NPC[B].Type == NPCID_PET_CYAN)
                                                        Player[A].MountType = 8;
                                                    Player[A].YoshiNPC = 0;
                                                    Player[A].YoshiPlayer = 0;
                                                    Player[A].MountSpecial = 0;
                                                    Player[A].YoshiTonugeBool = false;
                                                    Player[A].YoshiTongueLength = 0;
                                                    PlaySoundSpatial(SFX_Pet, Player[A].Location);
                                                    UpdateYoshiMusic();
                                                    YoshiHeight(A);
                                                }
                                            }
                                        }
                                        else if(NPC[B].Type != NPCID_CANNONITEM && NPC[B].Type != NPCID_KEY &&
                                                NPC[B].Type != NPCID_TOOTHYPIPE && NPC[B].Type != NPCID_TOOTHY &&
                                                (!Player[A].SlideKill || NPC[B]->WontHurt)) // NPCs that cannot be walked on
                                        {
                                            if(NPC[B].CantHurtPlayer == A && Player[A].NoShellKick > 0)
                                            {
                                                // Do nothing!
                                            }
                                            else
                                            {
                                                if(NPC[B]->IsABonus) // Bonus
                                                    TouchBonus(A, B);
                                                else if(NPC[B]->IsAShell && NPC[B].Location.SpeedX == 0 && Player[A].HoldingNPC == 0 && Player[A].Controls.Run)
                                                {
                                                    // grab turtle shells
                                                    //if(nPlay.Online == false || nPlay.MySlot + 1 == A)
                                                    {
                                                        if(Player[A].Character >= 3)
                                                            PlaySoundSpatial(SFX_Grab, Player[A].Location);
                                                        else
                                                            UnDuck(Player[A]);

                                                        Player[A].HoldingNPC = B;
                                                        NPC[B].HoldingPlayer = A;
                                                        NPC[B].CantHurt = Physics.NPCCanHurtWait;
                                                        NPC[B].CantHurtPlayer = A;
                                                    }

                                                }
                                                else if(NPC[B]->JumpHurt || (NPC[B]->IsFish && Player[A].WetFrame)) // NPCs that cause damage even when jumped on
                                                {
                                                    if(!(NPC[B].Type == NPCID_PLANT_S3 && NPC[B].Special2 == 4) && !NPC[B]->WontHurt && NPC[B].CantHurtPlayer != A)
                                                    {

                                                        // the n00bcollision function reduces the size of the npc's hit box before it damages the player
                                                        if(n00bCollision(Player[A].Location, NPC[B].Location))
                                                            PlayerHurt(A);
                                                    }
                                                }
                                                else if(NPC[B].Type == NPCID_MINIBOSS) // Special code for BOOM BOOM
                                                {
                                                    if(NPC[B].Special == 0 || Player[A].Mount == 1 || Player[A].Mount == 3)
                                                    {
                                                        if(NPC[B].Special != 0)
                                                            PlaySoundSpatial(SFX_Stomp, Player[A].Location);
                                                        tempHit = true;
                                                        tempLocation.Y = NPC[B].Location.Y - Player[A].Location.Height;
                                                    }
                                                    else if(NPC[B].Special != 4)
                                                    {
                                                        if(n00bCollision(Player[A].Location, NPC[B].Location))
                                                            PlayerHurt(A);
                                                    }
                                                }
                                                else if((NPC[B].Type == NPCID_LIT_BOMB_S3) || NPC[B].Type == NPCID_HIT_CARRY_FODDER)
                                                    NPCHit(B, 1, A); // NPC 'B' was jumped on '1' by player 'A'
                                                else if(NPC[B].Killed != 10 && !NPCIsBoot(NPC[B]) && !NPCIsYoshi(NPC[B]) && !(NPC[B]->IsAShell && NPC[B].CantHurtPlayer == A)) // Bounce off everything except Bonus and Piranha Plants
                                                {
                                                    if(NPC[B].Type == NPCID_SPRING)
                                                        tempSpring = true;
                                                    if(NPC[B]->IsAShell && NPC[B].Location.SpeedX == 0 && NPC[B].Location.SpeedY == 0)
                                                        tempShell = true;
                                                    tempHit = true;
                                                    tempLocation.Y = NPC[B].Location.Y - Player[A].Location.Height;
                                                    if(NPC[B].Type == NPCID_COIN_SWITCH || NPC[B].Type == NPCID_TIME_SWITCH || NPC[B].Type == NPCID_TNT)
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
                                                                (Player[A].WetFrame && (NPC[B].Type == NPCID_GRN_FISH_S3 || NPC[B].Type == NPCID_RED_FISH_S3)) ||
                                                                NPC[B]->IsAVine
                                                        ) && (Player[A].HoldingNPC != B)
                                                        )
                                                {
                                                    if(Player[A].Vine > 0)
                                                    {
                                                        Player[A].Vine = 0;
                                                        Player[A].Jump = 1;
                                                    }

                                                    if(!(NPC[B]->IsAShell && NPC[B].CantHurtPlayer == A))
                                                        NPCHit(B, 1, A); // NPC 'B' was jumped on '1' by player 'A'
                                                }
                                            }
                                        }
                                    }
                                    else if(HitSpot == 0) // if hitspot = 0 then do nothing
                                    {

                                        // player touched an npc anywhere except from the top
                                    }
                                    else if(!(NPC[B].Type == NPCID_MINIBOSS && NPC[B].Special == 4)) // Player touched an NPC
                                    {

/* If (.CanGrabNPCs = True Or NPCIsGrabbable(NPC(B).Type) = True Or (NPC(B).Effect = 2 And NPCIsABonus(NPC(B).Type) = False)) And (NPC(B).Effect = 0 Or NPC(B).Effect = 2) Or (NPCIsAShell(NPC(B).Type) And FreezeNPCs = True) Then      'GRAB EVERYTHING
*/
                                        // grab code
                                        if(
                                            ((Player[A].CanGrabNPCs || NPC[B]->IsGrabbable || (NPC[B].Effect == 2 && !NPC[B]->IsABonus)) && (NPC[B].Effect == 0 || NPC[B].Effect == 2)) ||
                                             (NPC[B]->IsAShell && FreezeNPCs)
                                        ) // GRAB EVERYTHING
                                        {
                                            if(Player[A].Controls.Run)
                                            {
                                                if((HitSpot == 2 && Player[A].Direction == -1) ||
                                                   (HitSpot == 4 && Player[A].Direction == 1) ||
                                                   (NPC[B].Type == NPCID_CANNONITEM || NPC[B].Type == NPCID_TOOTHYPIPE || NPC[B].Effect == 2 || (NPCIsVeggie(NPC[B]) && NPC[B].CantHurtPlayer != A)))
                                                {
                                                    if(Player[A].HoldingNPC == 0)
                                                    {
                                                        if(!NPC[B]->IsAShell || Player[A].Character >= 3)
                                                        {
                                                            if(NPCIsVeggie(NPC[B]))
                                                                PlaySoundSpatial(SFX_Grab2, Player[A].Location);
                                                            else
                                                                PlaySoundSpatial(SFX_Grab, Player[A].Location);
                                                        }
                                                        if(Player[A].Character <= 2)
                                                            UnDuck(Player[A]);
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
                                        if(NPC[B]->IsAShell || (NPC[B].Type == NPCID_SLIDE_BLOCK && NPC[B].Special == 1)) // Turtle shell
                                        {
                                            if(NPC[B].Location.SpeedX == 0 && NPC[B].Location.SpeedY >= 0) // Shell is not moving
                                            {
                                                if(((Player[A].Controls.Run && Player[A].HoldingNPC == 0) || Player[A].HoldingNPC == B) && NPC[B].CantHurtPlayer != A) // Grab the shell
                                                {
                                                    if(Player[A].Character >= 3)
                                                        PlaySoundSpatial(SFX_Grab, Player[A].Location);
                                                    else
                                                        UnDuck(Player[A]);
                                                    Player[A].HoldingNPC = B;
                                                    NPC[B].HoldingPlayer = A;
                                                    NPC[B].CantHurt = Physics.NPCCanHurtWait;
                                                    NPC[B].CantHurtPlayer = A;
                                                }
                                                else if(NPC[B].HoldingPlayer == 0) // Kick the shell
                                                {
                                                    if((Player[A].Mount == 1 || Player[A].Mount == 2 || Player[A].Mount == 3) && NPC[B].Type != NPCID_SLIDE_BLOCK)
                                                    {
                                                        if(NPC[B].Type != NPCID_FLIPPED_RAINBOW_SHELL)
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
                                                        NewEffect(EFFID_STOMP_INIT, tempLocation);
                                                        NPC[B].CantHurt = 0;
                                                        NPC[B].CantHurtPlayer = 0;
                                                        NPCHit(B, 1, A);
                                                    }
                                                }
                                            }
                                            else if(NPC[B].Location.SpeedX != 0) // Got hit by the shell
                                            {
                                                if(NPC[B].CantHurtPlayer != A && !FreezeNPCs && NPC[B].Type != NPCID_FLIPPED_RAINBOW_SHELL)
                                                {
                                                    if(n00bCollision(Player[A].Location, NPC[B].Location))
                                                        PlayerHurt(A);
                                                }
                                            }
                                        }
                                        else if(NPC[B]->IsABonus) // Bonus
                                            TouchBonus(A, B);
                                        else // Everything else
                                        {
                                            if((NPC[B].Type == NPCID_LIT_BOMB_S3 || NPC[B].Type == NPCID_HIT_CARRY_FODDER) && NPC[B].HoldingPlayer != A) // kick the bob-om
                                            {
                                                if(NPC[B].TailCD == 0)
                                                {
                                                    NPC[B].TailCD = 12;
                                                    if(NPC[B].Type != NPCID_HIT_CARRY_FODDER && NPC[B].Type != NPCID_LIT_BOMB_S3)
                                                        NewEffect(EFFID_WHACK, newLoc((Player[A].Location.X + NPC[B].Location.X + (Player[A].Location.Width + NPC[B].Location.Width) / 2.0) / 2, (Player[A].Location.Y + NPC[B].Location.Y + (Player[A].Location.Height + NPC[B].Location.Height) / 2.0) / 2));
                                                    NPCHit(B, 1, A);
                                                }
                                            }
                                            else if(NPC[B].CantHurtPlayer != A && !NPC[B]->WontHurt)
                                            {
                                                if(!(NPC[B].Type == NPCID_BULLET && NPC[B].Projectile != 0))
                                                {
                                                    if(NPC[B].Type >= NPCID_GRN_HIT_TURTLE_S4 && NPC[B].Type <= NPCID_YEL_HIT_TURTLE_S4 && NPC[B].Projectile != 0)
                                                        NPCHit(B, 3, B);
                                                    else
                                                    {
                                                        if(NPC[B].Effect != 2)
                                                        {
                                                            if(Player[A].SlideKill && !NPC[B]->JumpHurt)
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
                                                                MoreScore(NPC[B]->Score, NPC[B].Location, Player[A].Multiplier);
                                                        }
                                                    }
                                                }
                                            }

                                            // this is for NPC that physically push the player
                                            if(NPC[B]->MovesPlayer && NPC[B].Projectile == 0 && Player[A].HoldingNPC != B &&
                                               !(Player[A].Mount == 2 && (NPC[B].Type == NPCID_KEY || NPC[B].Type == NPCID_COIN_SWITCH)) &&
                                               !ShadowMode && NPC[B].Effect != 2)
                                            {
                                                if(Player[A].StandUp && Player[A].StandingOnNPC == 0)
                                                {
                                                    if(HitSpot == 5 && Player[A].Location.Y + Player[A].Location.Height - Physics.PlayerDuckHeight[Player[A].Character][Player[A].State] - Player[A].Location.SpeedY >= NPC[B].Location.Y + NPC[B].Location.Height)
                                                        HitSpot = 3;
                                                }
                                                if(HitSpot == 3)
                                                {
                                                    if(NPC[B].Type == NPCID_ICE_CUBE && Player[A].Character != 5 && Player[A].State > 1)
                                                        NPCHit(B, 3, B);
                                                    tempLocation = Player[A].Location;
                                                    Player[A].Location.SpeedY = 0.1 + NPC[B].Location.SpeedY;
                                                    Player[A].Location.Y = NPC[B].Location.Y + NPC[B].Location.Height + 0.1;

                                                    // fBlock = FirstBlock[(Player[A].Location.X / 32) - 1];
                                                    // lBlock = LastBlock[((Player[A].Location.X + Player[A].Location.Width) / 32.0) + 1];
                                                    // blockTileGet(Player[A].Location, fBlock, lBlock);

                                                    for(int C : treeFLBlockQuery(Player[A].Location, SORTMODE_NONE))
                                                    {
                                                        if(CheckCollision(Player[A].Location, Block[C].Location) &&
                                                           !Block[C].Hidden && !BlockIsSizable[Block[C].Type] &&
                                                           !BlockOnlyHitspot1[Block[C].Type])
                                                        {
                                                            Player[A].Location = tempLocation;
                                                            break;
                                                        }
                                                    }

                                                    PlaySoundSpatial(SFX_BlockHit, Player[A].Location);
                                                    Player[A].Jump = 0;
                                                    if(Player[A].Mount == 2)
                                                        Player[A].Location.SpeedY += 2;
                                                    if(NPC[B].Type == NPCID_METALBARREL || NPC[B].Type == NPCID_CANNONENEMY || NPC[B].Type == NPCID_HPIPE_SHORT || NPC[B].Type == NPCID_HPIPE_LONG || NPC[B].Type == NPCID_VPIPE_SHORT || NPC[B].Type == NPCID_VPIPE_LONG || (NPC[B].Type >= NPCID_TANK_TREADS && NPC[B].Type <= NPCID_SLANT_WOOD_M))
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
                                                        Player[A].Pinched.Right4 = 2;

                                                        if(NPC[B].Type != NPCID_KEY && NPC[B].Type != NPCID_COIN_SWITCH && NPC[B].Type != NPCID_CONVEYOR && (NPC[B].Location.SpeedX != 0 || NPC[B].Location.SpeedY != 0 || NPC[B].BeltSpeed))
                                                        {
                                                            Player[A].Pinched.Moving = 2;

                                                            if(NPC[B].Location.SpeedX != 0 || NPC[B].BeltSpeed)
                                                                Player[A].Pinched.MovingLR = true;
                                                        }

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
                                                        if(!tempBool && NPC[B].Type != NPCID_BULLY)
                                                            Player[A].Location.SpeedX = 0.2 * Player[A].Direction;
                                                        if(NPC[Player[A].StandingOnNPC].Type == 57)
                                                            Player[A].Location.X -= 1;
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
                                                        Player[A].Pinched.Left2 = 2;

                                                        if(NPC[B].Type != NPCID_KEY && NPC[B].Type != NPCID_COIN_SWITCH && NPC[B].Type != NPCID_CONVEYOR && (NPC[B].Location.SpeedX != 0 || NPC[B].Location.SpeedY != 0 || NPC[B].BeltSpeed))
                                                        {
                                                            Player[A].Pinched.Moving = 2;

                                                            if(NPC[B].Location.SpeedX != 0 || NPC[B].BeltSpeed)
                                                                Player[A].Pinched.MovingLR = true;
                                                        }

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
                                                        if(!tempBool && NPC[B].Type != NPCID_BULLY)
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

                                                        for(int C : NPCQueues::Active.no_change)
                                                        {
                                                            if(NPC[C].vehiclePlr == A)
                                                            {
                                                                NPC[C].Location.X += D;
                                                                treeNPCUpdate(C);
                                                            }
                                                        }

                                                        for(int C = 1; C <= numPlayers; C++)
                                                        {
                                                            if(Player[C].StandingOnTempNPC == 56)
                                                                Player[C].Location.X += D;
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
                            Player[A].Jump += 3;
                        if(Player[A].SpinJump)
                            Player[A].Jump -= 6;
                        Player[A].Location.SpeedY = Physics.PlayerJumpVelocity - 4;
                        if(Player[A].Wet > 0)
                            Player[A].Location.SpeedY = Player[A].Location.SpeedY * 0.3;
                    }
                    else
                    {
                        Player[A].Jump = Physics.PlayerNPCJumpHeight;
                        if(Player[A].Character == 2)
                            Player[A].Jump += 3;
                        if(Player[A].SpinJump)
                            Player[A].Jump -= 6;
                        Player[A].Location.SpeedY = Physics.PlayerJumpVelocity;
                        if(Player[A].Wet > 0)
                            Player[A].Location.SpeedY = Player[A].Location.SpeedY * 0.3;
                    }
                    Player[A].Location.Y = tempLocation.Y;
                    if(tempShell)
                        NewEffect(EFFID_STOMP_INIT, newLoc(Player[A].Location.X + Player[A].Location.Width / 2.0 - EffectWidth[132] / 2.0, Player[A].Location.Y + Player[A].Location.Height - EffectHeight[132] / 2.0));
                    else if(!tempSpring)
                        NewEffect(EFFID_WHACK, newLoc(Player[A].Location.X + Player[A].Location.Width / 2.0 - 16, Player[A].Location.Y + Player[A].Location.Height - 16));
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
                        C += -(Player[A].Location.X + Player[A].Location.Width * 0.5);
                        D += -(Player[A].Location.X + Player[A].Location.Width * 0.5);
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
                if(NPC[B].Projectile != 0 && NPCIsVeggie(NPC[B]))
                    B = 0;

                // B is the number of the NPC that the player is standing on
                // .StandingOnNPC is the number of the NPC that the player was standing on last cycle
                // if B = 0 and .standingonnpc > 0 then the player was standing on something and is no longer standing on something


                if(B > 0 && Player[A].SpinJump)
                {
                    if(NPC[B].Type == NPCID_ICE_CUBE)
                    {
                        Player[A].Location.SpeedY = Physics.PlayerJumpVelocity;
                        NPC[B].Multiplier += Player[A].Multiplier;
                        NPCHit(B, 3, B);
                        Player[A].Jump = 7;
                        if(Player[A].Character == 2)
                            Player[A].Jump += 3;
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
                    Player[A].Location.SpeedX += (NPC[Player[A].StandingOnNPC].Location.SpeedX + NPC[Player[A].StandingOnNPC].BeltSpeed);
                else if(B > 0 && Player[A].StandingOnNPC == 0 && NPC[B].playerTemp && Player[A].Location.SpeedY >= 0)
                    Player[A].Location.SpeedX += -(NPC[B].Location.SpeedX + NPC[B].BeltSpeed);

                if(movingBlock) // this is for when the player is standing on a moving block
                {
                    if(B > 0)
                    {
                        if(NPC[B].Type == NPCID_ITEM_BURIED)
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
                            numBlock++;
                            Block[numBlock].Location.Y = NPC[B].Location.Y;
                            // seems weird but I'll sync it since we don't know what could happen inside YoshiPound
                            syncLayersTrees_Block(numBlock);
                            YoshiPound(A, Player[A].Mount, true);
                            Block[numBlock].Location.Y = 0;
                            numBlock--;
                            syncLayersTrees_Block(numBlock + 1);
                            Player[A].GroundPound = false;
                        }
                        else if(Player[A].YoshiYellow)
                        {
                            numBlock++;
                            Block[numBlock].Location.Y = NPC[B].Location.Y;
                            // seems weird but I'll sync it since we don't know what could happen inside YoshiPound
                            syncLayersTrees_Block(numBlock);
                            YoshiPound(A, Player[A].Mount);
                            Block[numBlock].Location.Y = 0;
                            numBlock--;
                            syncLayersTrees_Block(numBlock + 1);
                        }
                    }
                    if(NPC[B].playerTemp == 0)
                        Player[A].StandingOnTempNPC = 0;
                    if(Player[A].Location.SpeedY >= 0)
                        Player[A].StandingOnNPC = B;
                    Player[A].Location.Y = NPC[B].Location.Y - Player[A].Location.Height;
                    if(NPC[B].Type == NPCID_FALL_BLOCK_RED || NPC[B].Type == NPCID_FALL_BLOCK_BROWN)
                        NPC[B].Special2 = 1;
                    if(NPC[B].Type == NPCID_CHECKER_PLATFORM)
                        NPC[B].Special = 1;
                    if(NPC[B].Type == NPCID_PLATFORM_S3 && Player[A].Location.SpeedY > 0)
                        NPC[B].Direction = 1;
                    if(NPC[B].Type == NPCID_RAFT && NPC[B].Special == 0)
                    {
                        NPC[B].Special = 1;
                        SkullRide(B);
                    }
                    if(NPC[B].Type == NPCID_CONVEYOR)
                        Player[A].Location.SpeedY = 0;


                    if(NPC[B].Type == NPCID_VEHICLE && Player[A].Controls.Down && Player[A].Mount == 0 &&
                       !NPC[B].playerTemp && Player[A].DuckRelease &&
                       (Player[A].HoldingNPC == 0 || Player[A].Character == 5))
                    {
                        UnDuck(Player[A]);
                        if(g_compatibility.fix_char5_vehicle_climb && Player[A].Fairy) // Avoid the mortal glitch
                        {
                            Player[A].Fairy = false;
                            PlaySoundSpatial(SFX_HeroFairy, Player[A].Location);
                            NewEffect(EFFID_SMOKE_S5, Player[A].Location);
                        }
                        Player[A].Location = NPC[B].Location;
                        Player[A].Mount = 2;
                        NPC[B].Killed = 9;
                        NPCQueues::Killed.push_back(B);
                        Player[A].HoldingNPC = 0;
                        Player[A].StandingOnNPC = 0;
                        PlaySoundSpatial(SFX_Stomp, Player[A].Location);
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
                            PlaySoundSpatial(SFX_Stone, Player[A].Location);
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
                                Player[A].Location.Y += -Player[A].Location.SpeedY;
                                Player[A].Location.SpeedY = NPC[Player[A].StandingOnNPC].Location.SpeedY;
                                if(Player[A].Location.SpeedY > Physics.PlayerTerminalVelocity)
                                    Player[A].Location.SpeedY = Physics.PlayerTerminalVelocity;
                                Player[A].Location.Y += Player[A].Location.SpeedY;
                            }
                        }
                    }
                }
                else if(Player[A].Mount == 1 && Player[A].Jump > 0)
                {
                    if(B == 0 && Player[A].StandingOnTempNPC > 0)
                        Player[A].Location.SpeedX += (NPC[Player[A].StandingOnNPC].Location.SpeedX + NPC[Player[A].StandingOnNPC].BeltSpeed);
                    else if(B > 0 && Player[A].StandingOnNPC == 0 && NPC[B].playerTemp)
                        Player[A].Location.SpeedX += -(NPC[B].Location.SpeedX + NPC[B].BeltSpeed);
                    Player[A].StandingOnNPC = 0;
                    Player[A].StandingOnTempNPC = 0;
                }
                else
                {
                    if(Player[A].StandingOnNPC != 0)
                    {

                        if(Player[A].StandingOnNPC < 0)
                            Player[A].Location.SpeedX += NPC[Player[A].StandingOnNPC].Location.SpeedX;
                        Player[A].Location.Y += -Player[A].Location.SpeedY;
                        Player[A].Location.SpeedY = NPC[Player[A].StandingOnNPC].Location.SpeedY;
                        if(FreezeNPCs)
                            Player[A].Location.SpeedY = 0;
                        if(Player[A].Location.SpeedY > Physics.PlayerTerminalVelocity)
                            Player[A].Location.SpeedY = Physics.PlayerTerminalVelocity;
                        Player[A].Location.Y += Player[A].Location.SpeedY;
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
                    MessageText = GetS(NPC[MessageNPC].Text);
                    preProcessMessage(MessageText, A);
                    PauseGame(PauseCode::Message, A);
                    MessageText.clear();
                    MessageTextMap.clear();
                    if(NPC[MessageNPC].TriggerTalk != EVENT_NONE)
                        ProcEvent(NPC[MessageNPC].TriggerTalk, A);
                    MessageNPC = 0;
                }
                YoshiEatCode(A);

                // pinch code
                if(!GodMode)
                {
                    const auto& pi = Player[A].Pinched;

                    bool vcrush = pi.Bottom1 && pi.Top3;
                    bool hcrush = pi.Left2 && pi.Right4;

                    // When the player is pushed through the floor or stops ducking while being crushed, they get left+right hits but no bottom hit
                    bool vcrush_plus = vcrush || (hcrush && (pi.Bottom1 || pi.Top3));

                    bool old_condition = pi.Moving && (vcrush || hcrush);

                    bool new_condition = (pi.MovingUD && vcrush_plus) || (pi.MovingLR && hcrush);

                    bool pinch_death = (g_compatibility.fix_player_crush_death && !pi.Strict) ? new_condition : old_condition;

                    if(pinch_death && Player[A].Mount != 2)
                    {
                        if(Player[A].Mount != 2)
                            Player[A].Mount = 0;
                        Player[A].State = 1;
                        Player[A].Immune = 0;
                        Player[A].Immune2 = false;

                        // Pinch death should occur (but might get cancelled for some reason); set a timer of 15 frames to use stricter old condition
                        // Why is that needed here? The details of hitspot detection mean that if a player is pushed through the floor, they get left+right hits, but no bottom hit
                        Player[A].Pinched.Strict = 15;
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
                const Screen_t& screen = ScreenByPlayer(A);

                // if(!LevelWrap[Player[A].Section] && LevelMacro == LEVELMACRO_OFF)
                // This section is fully new logic
                if(screen.Type == ScreenTypes::SharedScreen)
                {
                    Player_t& p = Player[A];
                    const Location_t& section = level[p.Section];

                    const vScreen_t& vscreen = vScreenByPlayer(A);


                    // section for shared screen push
                    bool check_left = true;
                    bool check_right = true;

                    bool vscreen_at_section_bound_left = -vscreen.X <= section.X + 8;
                    bool vscreen_at_section_bound_right = -vscreen.X + vscreen.Width >= section.Width - 8;

                    // normally, don't use the shared screen push at section boundaries
                    if(vscreen_at_section_bound_left)
                        check_left = false;

                    if(vscreen_at_section_bound_right)
                        check_right = false;

                    // do use shared screen push if there's a different player at the other side of the screen
                    for(int o_p_i = 1; o_p_i <= numPlayers; o_p_i++)
                    {
                        const Player_t& o_p = Player[o_p_i];

                        if(o_p.Location.X <= -vscreen.X + 8 && !vscreen_at_section_bound_left)
                            check_right = true;
                        else if(o_p.Location.X + o_p.Location.Width >= -vscreen.X + vscreen.Width - 8 && !vscreen_at_section_bound_right)
                            check_left = true;
                    }

                    if(p.Location.X <= -vscreen.X + 8 && check_left)
                    {
                        if(p.Location.X <= -vscreen.X)
                        {
                            p.Location.X = -vscreen.X;
                            p.Pinched.Left2 = 2;

                            if(p.Location.SpeedX < 0)
                                p.Location.SpeedX = 0;
                        }

                        if(p.Location.SpeedX >= 0 && p.Location.SpeedX < 1)
                            p.Location.SpeedX = 1;
                    }
                    else if(p.Location.X + p.Location.Width >= -vscreen.X + vscreen.Width - 8 && check_right)
                    {
                        if(p.Location.X + p.Location.Width >= -vscreen.X + vscreen.Width)
                        {
                            p.Location.X = -vscreen.X + vscreen.Width - p.Location.Width;
                            p.Pinched.Right4 = 2;

                            if(p.Location.SpeedX > 0)
                                p.Location.SpeedX = 0;
                        }

                        if(p.Location.SpeedX > -1 && p.Location.SpeedX <= 0)
                            p.Location.SpeedX = -1;
                    }


                    // kill a player that falls offscreen
                    if(p.Location.Y > -vscreen.Y + vscreen.Height + 64)
                        PlayerDead(A);
                }

                if(Player[A].Location.Y > level[Player[A].Section].Height + 64)
                    PlayerDead(A);

                if(!NPC[Player[A].StandingOnNPC]->IsAShell)
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

                PlayerFrame(Player[A]); // Update players frames
                Player[A].StandUp = false; // Fixes a block collision bug
                Player[A].StandUp2 = false;
                if(Player[A].ForceHitSpot3)
                    Player[A].StandUp = true;
                Player[A].ForceHitSpot3 = false;
                if(Player[A].ForceHold > 0)
                    Player[A].ForceHold -= 1;
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
        Player[A].AltRunRelease = !Player[A].Controls.AltRun;
    }

    // int C = 0;

    // kill the player temp NPCs, from last to first
    std::sort(NPCQueues::PlayerTemp.begin(), NPCQueues::PlayerTemp.end(),
    [](NPCRef_t a, NPCRef_t b)
    {
        return a > b;
    });


    // for(A = numNPCs; A >= 1; A--)
    int last_NPC = numNPCs + 1;
    for(int A : NPCQueues::PlayerTemp)
    {
        // duplicated entry, no problem
        if(A == last_NPC)
            continue;

        if(NPC[A].playerTemp)
        {
            for(B = 1; B <= numPlayers; B++)
            {
                if(Player[B].StandingOnNPC == A)
                    Player[B].StandingOnTempNPC = NPC[A].Type;
            }
            NPC[0] = NPC[A]; // was NPC[C] = NPC[A] but C was not mutated
            KillNPC(A, 9);
        }
    }

    NPCQueues::PlayerTemp.clear();
}
