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
#include "player/player_effect.h"
#include "player/player_update_priv.h"
#include "../collision.h"
#include "../sound.h"
#include "../blocks.h"
#include "../npc.h"
#include "../effect.h"
#include "../layers.h"
#include "../editor.h"
#include "../game_main.h"
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
    // int B = 0;
//    Controls_t blankControls;
    // float speedVar = 0; // adjusts the players speed by percentages
    // int64_t fBlock = 0; // for collision detection optimizations
    // int64_t lBlock = 0;
    // double tempSpeed = 0;
    // int HitSpot = 0;
    // the hitspot is used for collision detection to find out where to put the player after it collides with a block
    // the numbers tell what side the collision happened so it can move the plaer to the correct position
    // 1 means the player hit the block from the top
    // 2 is from the right
    // 3 is from the bottom
    // 4 is from the left
    // int tempBlockHit[3] = {0};
//    double tempBlockA[3] = {0};
    // bool tempHit = false;
    bool tempSpring = false;
    bool tempShell = false;
    // bool tempHit2 = false;
    // int tempHit3 = 0;
    // float tempHitSpeed = 0;
    // float oldSpeedY = 0; // holds the players previous Y speed
    // int oldStandingOnNpc = 0;
    // Location_t tempLocation;
    // Location_t tempLocation3;
    // bool spinKill = false;
    // int oldSlope = 0;
    // float A1 = 0;
    // float B1 = 0;
    // float C1 = 0;
    // float X = 0;
    // float Y = 0;
//    bool canWarp = false;
    // bool tempBool = false;
    // NPC_t blankNPC;
    // int MessageNPC = 0;
    // used for slope calculations
    // double PlrMid = 0;
    // double Slope = 0;
    // int tempSlope = 0;
    // int tempSlope2 = 0;
    // double tempSlope2X = 0; // The old X before player was moved
    // int tempSlope3 = 0; // keeps track of hit 5 for slope detection
    // bool movingBlock = false; // helps with collisions for moving blocks
    // int blockPushX = 0;
    // Location_t oldLoc;
//    Location_t curLoc;
//    int oldGrab = 0; // SET BUT NOT USED
    // bool DontResetGrabTime = false; // helps with grabbing things from the top
    // double SlippySpeedX = 0;
    // bool wasSlippy = false;
    // double Angle = 0;
    // double slideSpeed = 0;
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
//        else if(Player[A].Effect != PLREFF_NORMAL)
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
//        oldGrab = Player[A].HoldingNPC; // SET BUT NOT UNUSED
        Player[A].ShowWarp = 0;
        Player[A].mountBump = 0;
        // bool spinKill = false;

        // this was shared over players in SMBX 1.3 -- if the line marked "MOST CURSED LINE" in player_block_logic.cpp becomes a source of incompatibility, we will need to restore that logic
        float cursed_value_C = 0;

        // Location_t tempLocation3;
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
            bool split_screen = (screen.active_end() - screen.active_begin() > 1);
            bool normal_multiplayer = (dynamic_screen || shared_screen || split_screen);

            bool player_timer_done = (Player[A].TimeToLive >= 200);

            // checks if the dead player can move towards a target location
            bool player_can_move = (player_timer_done || shared_screen || !normal_multiplayer);

            // there was a guard here that has now been moved into the subclauses
            // if(Player[A].TimeToLive >= 200 || ScreenType != 5)
            int B = CheckNearestLiving(A);

            bool someone_else_alive = false;
            for(int o_A = 1; o_A <= numPlayers; o_A++)
            {
                if(o_A == A)
                    continue;

                if(!Player[o_A].Dead || (BattleMode && BattleLives[o_A] > 0))
                {
                    someone_else_alive = true;

                    if(shared_screen && B == 0 && Player[o_A].TimeToLive < Player[A].TimeToLive)
                        B = o_A;

                    break;
                }
            }

            // allow smooth panning in cloned player mode
            if(g_config.multiplayer_pause_controls && !normal_multiplayer && g_ClonedPlayerMode && B == 0 && someone_else_alive)
                B = 1;

            // move dead player towards start point in BattleMode
            bool battle_respawn = (BattleMode && BattleLives[A] > 0 && someone_else_alive && BattleWinner == 0);
            if(battle_respawn)
                B = 20 + A;

            if(battle_respawn && player_timer_done)
            {
                Player[B].Location.Width = Player[A].Location.Width;
                Player[B].Location.Height = Player[A].Location.Height;

                // eventually, check for valid starts
                constexpr int valid_start_count = 2;
                int use_start = (A - 1) % valid_start_count + 1;

                // NOTE, there is a bugfix here without a compat flag, previously the * 0.5 did not exist
                constexpr bool do_bugfix = true;
                Player[B].Location.X = PlayerStart[use_start].X + PlayerStart[use_start].Width * 0.5 - Player[A].Location.Width * (do_bugfix ? 0.5 : 1.0);
                Player[B].Location.Y = PlayerStart[use_start].Y + PlayerStart[use_start].Height - Player[A].Location.Height;
                CheckSection(B);
                if(Player[A].Section != Player[B].Section)
                {
                    Player[A].Location = Player[B].Location;
                    Player[A].Section = Player[B].Section;
                }
            }

            if(B > 0 && player_can_move) // Move camera to the other living players
            {
                float A1, B1;
                if(shared_screen)
                {
                    const vScreen_t& vscreen = screen.vScreen(screen.active_begin() + 1);
                    A1 = (Player[B].Location.X + Player[B].Location.Width * 0.5) - (Player[A].Location.X + Player[A].Location.Width * 0.5);
                    if(!g_config.multiplayer_pause_controls)
                        B1 = (float)((-vscreen.Y + vscreen.Height * 0.5) - Player[A].Location.Y);
                    else
                        B1 = (float)((-vscreen.Y + vscreen.Height * 0.5) - Player[A].Location.Y - Player[A].Location.Height);
                }
                else if(normal_multiplayer)
                {
                    A1 = (Player[B].Location.X + Player[B].Location.Width * 0.5) - (Player[A].Location.X + Player[A].Location.Width * 0.5);
                    if(!g_config.multiplayer_pause_controls)
                        B1 = Player[B].Location.Y - Player[A].Location.Y;
                    else
                        B1 = Player[B].Location.Y + Player[B].Location.Height - Player[A].Location.Y - Player[A].Location.Height;
                }
                else
                {
                    const vScreen_t& vscreen = screen.vScreen(screen.active_begin() + 1);
                    A1 = (float)((-vscreen.X + vscreen.Width * 0.5) - (Player[A].Location.X + Player[A].Location.Width * 0.5));
                    if(!g_config.multiplayer_pause_controls)
                        B1 = (float)((-vscreen.Y + vscreen.Height * 0.5) - Player[A].Location.Y);
                    else
                        B1 = (float)((-vscreen.Y + vscreen.Height * 0.5) - Player[A].Location.Y - Player[A].Location.Height);
                }

                float C1 = std::sqrt((A1 * A1) + (B1 * B1));
                float X, Y;
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
                if(normal_multiplayer && Player[A].Section != Player[B].Section)
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
                    if(normal_multiplayer && !shared_screen && Player[A].Dead)
                    {
                        Player[A].Effect2 = -B;

                        // new logic: fix player's location
                        if(g_config.multiplayer_pause_controls)
                        {
                            Player[A].Location.X = Player[B].Location.X + Player[B].Location.Width / 2 - Player[A].Location.Width / 2;
                            Player[A].Location.Y = Player[B].Location.Y + Player[B].Location.Height - Player[A].Location.Height;
                        }
                    }
                }
            }
            // start fadeout (65 / 3) frames before level end
            else if(!BattleMode && B == 0 && Player[A].TimeToLive == 200 - (65 / 3))
            {
                ProcessLastDead(); // Fade out screen if the last player died
            }
            else if(!BattleMode && player_timer_done) // ScreenType = 1
            {
                double old_LocX = Player[A].Location.X;
                double old_LocY = Player[A].Location.Y;
                KillPlayer(A); // Time to die

                // new logic: fix player's location
                if(g_config.multiplayer_pause_controls)
                {
                    Player[A].Location.X = old_LocX;
                    Player[A].Location.Y = old_LocY;
                }
            }
        }
        else if(Player[A].Dead)
        {
            // safer than the below code, should always be used except for compatibility concerns
            if(numPlayers > 2 || g_config.multiplayer_pause_controls)
            {
                int B;

                // continue following currently-tracked player if possible
                if(Player[A].Effect2 < 0)
                {
                    B = -Player[A].Effect2;

                    // check if tracked dead player is gone
                    if(B > numPlayers || Player[B].Dead || Player[B].TimeToLive > 0)
                    {
                        Player[A].Effect2 = 0;
                        B = 0;

                        // put player back in TimeToLive state if there are still other players
                        if(CheckNearestLiving(A))
                        {
                            Player[A].Dead = false;
                            Player[A].TimeToLive = 200;
                        }
                    }
                }
                else
                    B = CheckNearestLiving(A);

                if(B)
                {
                    Player[A].Location.X = Player[B].Location.X + Player[B].Location.Width / 2 - Player[A].Location.Width / 2;
                    Player[A].Location.Y = Player[B].Location.Y + Player[B].Location.Height - Player[A].Location.Height;
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
            // Location_t oldLoc = Player[A].Location; // only used in online code

            if(Player[A].SlideCounter > 0) // for making the slide Effect
                Player[A].SlideCounter -= 1;

            if(Player[A].Effect == PLREFF_NORMAL)
            {
                // for the pound pet mount logic
                PlayerPoundLogic(A);

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

                // unduck a player that shouldn't be able to duck
                if(Player[A].Duck && (Player[A].Character == 1 || Player[A].Character == 2) && Player[A].State == 1 && (Player[A].Mount == 0 || Player[A].Mount == 2))
                    UnDuck(Player[A]);

                if(GameMenu && !Player[A].SpinJump) // force the player to look right when on the game menu
                    Player[A].Direction = 1;

                WaterCheck(A); // This sub handles all the water related stuff

                PowerUps(A); // misc power-up code

                if(Player[A].StandingOnNPC > 0)
                {
                    if(NPC[Player[A].StandingOnNPC].Type == NPCID_ICE_CUBE && NPC[Player[A].StandingOnNPC].Location.SpeedX == 0.0)
                        Player[A].Slippy = true;
                }

                double SlippySpeedX = Player[A].Location.SpeedX;


                // Player's X movement. ---------------------------

                // Modify player's speed if he is running up/down hill
                float speedVar = 1; // Speed var is a percentage of the player's speed
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
                        double Angle = 1 / (Block[Player[A].Slope].Location.Width / static_cast<double>(Block[Player[A].Slope].Location.Height));
                        double slideSpeed = 0.1 * Angle * BlockSlope[Block[Player[A].Slope].Type];

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

                }
                // if not sliding and in the clown car
                else if(Player[A].Mount == 2)
                {
                    if(!Player[A].Controls.Jump)
                        Player[A].CanJump = true;

                    if(!Player[A].Controls.AltJump && g_config.fix_vehicle_altjump_lock)
                        Player[A].CanAltJump = true;

                    if(Player[A].Controls.AltJump && Player[A].CanAltJump) // Jump out of the Clown Car
                    {
                        if(g_config.fix_vehicle_altjump_lock)
                            Player[A].CanAltJump = false;

                        Player[A].CanJump = false;

                        bool dismount_safe = true;

                        Location_t tempLocation = Player[A].Location;
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
                                    dismount_safe = false;
                                    PlaySoundSpatial(SFX_BlockHit, Player[A].Location);
                                    break;
                                }
                            }
                        }

                        if(dismount_safe) for(int B : treeNPCQuery(tempLocation, SORTMODE_NONE))
                        {
                            if(NPC[B]->IsABlock && !NPC[B]->StandsOnPlayer && NPC[B].Active && NPC[B].Type != NPCID_VEHICLE)
                            {
                                if(CheckCollision(tempLocation, NPC[B].Location))
                                {
                                    dismount_safe = false;
                                    PlaySoundSpatial(SFX_BlockHit, Player[A].Location);
                                    break;
                                }
                            }
                        }

                        if(dismount_safe)
                            PlayerDismount(A);
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

                    if(g_config.fix_climb_bgo_speed_adding && Player[A].VineBGO > 0)
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

                    cursed_value_C = 1;
                    // If .Character = 5 Then C = 0.94
                    if(Player[A].Character == 5)
                        cursed_value_C = 0.95F;
                    if(Player[A].Controls.Left &&
                       ((!Player[A].Duck && Player[A].GrabTime == 0) ||
                        (Player[A].Location.SpeedY != 0.0 && Player[A].StandingOnNPC == 0 && Player[A].Slope == 0) ||
                        Player[A].Mount == 1)
                    )
                    {
                        Player[A].Bumped = false;
                        if(Player[A].Controls.Run || Player[A].Location.SpeedX > -Physics.PlayerWalkSpeed * speedVar || Player[A].Character == 5)
                        {
                            if(Player[A].Location.SpeedX > -Physics.PlayerWalkSpeed * speedVar * cursed_value_C)
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
                            if(Player[A].Location.SpeedX < Physics.PlayerWalkSpeed * speedVar * cursed_value_C)
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
                PlayerFairyTimerUpdate(A);


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
                bool wasSlippy = Player[A].Slippy;
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
                }
                else if(Player[A].Wet > 0 && Player[A].Quicksand == 0) // the player is swimming
                {
                    if(Player[A].Mount == 1)
                    {
                        if(Player[A].Controls.AltJump && Player[A].CanAltJump)
                            PlayerDismount(A);
                    }
                    else if(Player[A].Mount == 3)
                    {
                        if(Player[A].Controls.AltJump && Player[A].CanAltJump)
                            PlayerDismount(A);
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

                    Player[A].CanJump = !Player[A].Controls.Jump;
                    Player[A].CanAltJump = !Player[A].Controls.AltJump;

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
                                PlayerDismount(A);
                        }
                        else if(Player[A].Mount == 3)
                        {
                            if(Player[A].Controls.AltJump && Player[A].CanAltJump) // jump off of yoshi
                                PlayerDismount(A);
                        }

                        if((Player[A].Location.SpeedY == 0.0 || Player[A].Jump > 0 || Player[A].Vine > 0) && Player[A].FloatTime == 0) // princess float
                            Player[A].CanFloat = true;

                        if(Player[A].Wet > 0 || Player[A].WetFrame)
                            Player[A].CanFloat = false;

                        // handles the regular jump
                        if(Player[A].Controls.Jump || (Player[A].Controls.AltJump &&
                           ((Player[A].Character > 2 && Player[A].Character != 4) || Player[A].Quicksand > 0 || g_config.disable_spin_jump) &&
                           Player[A].CanAltJump))
                        {
                            double tempSpeed;
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
                                Location_t tempLocation = Player[A].Location;
                                tempLocation.Y = Player[A].Location.Y + Player[A].Location.Height - EffectHeight[80] / 2.0 + Player[A].Location.SpeedY;
                                tempLocation.Height = EffectHeight[80];
                                tempLocation.Width = EffectWidth[80];
                                tempLocation.X = Player[A].Location.X;

                                for(int B = 1; B <= 10; B++)
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
                                    Location_t tempLocation;
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
                                                          (g_config.fix_char3_escape_shell_surf && Player[A].Character == 3 && Player[A].ShellSurf))
                                                      && (!g_config.disable_spin_jump || Player[A].ShellSurf))
                        {
                            double tempSpeed;
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

                                        if(g_config.disable_spin_jump)
                                            Player[A].SpinJump = false;
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
                }

                Player[A].Location.Y += Player[A].Location.SpeedY;

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
                        bool hasNoMonts = (g_config.fix_char5_vehicle_climb && Player[A].Mount <= 0) ||
                                           !g_config.fix_char5_vehicle_climb;

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
                                Player[A].Effect = PLREFF_WAITING;
                                Player[A].Effect2 = 4;
                                NewEffect(EFFID_SMOKE_S5, Player[A].Location);
                            }
                        }

                        if(Player[A].Controls.Run && Player[A].RunRelease && (Player[A].FairyTime > 0 || Player[A].Effect == PLREFF_WAITING))
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
                                Location_t tempLocation = Background[B].Location;
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
                    PlayerLevelWrapLogic(A);

                if(LevelWrap[Player[A].Section])
                    hBoundsHandled = true;

                // Walk offscreen exit
                if(!hBoundsHandled && OffScreenExit[Player[A].Section])
                {
                    bool offScreenExit = false;
                    if(Player[A].Location.X + Player[A].Location.Width < level[Player[A].Section].X)
                    {
                        offScreenExit = true;
                        for(int B = 1; B <= numPlayers; B++)
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

                if(Player[A].Location.Y < level[Player[A].Section].Y - Player[A].Location.Height - 32 && Player[A].StandingOnVehiclePlr == 0)
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

                float oldSpeedY = Player[A].Location.SpeedY; // holds the players previous Y speed

                // this whole section is dead code, since there are no uses of these definitions of PlrMid and Slope
#if 0
                if(Player[A].StandingOnNPC == -A)
                {
                    if(Player[A].Slope != 0)
                    {
                        B = NPC[Player[A].StandingOnNPC].Special;

                        double PlrMid;
                        if(BlockSlope[Block[B].Type] == 1)
                            PlrMid = Player[A].Location.X;
                        else
                            PlrMid = Player[A].Location.X + Player[A].Location.Width;

                        double Slope = (PlrMid - Block[B].Location.X) / Block[B].Location.Width;

                        if(BlockSlope[Block[B].Type] < 0)
                            Slope = 1 - Slope;
                        if(Slope < 0)
                            Slope = 0;
                        if(Slope > 1)
                            Slope = 1;
                    }
                }
#endif

                // decrement pinched timers
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

                if(Player[A].Effect == PLREFF_NORMAL && Player[A].Pinched.Strict > 0)
                    Player[A].Pinched.Strict -= 1;

                // Block collisions.
                bool DontResetGrabTime = false; // helps with grabbing things from the top
                bool movingBlock = false; // helps with collisions for moving blocks
                int tempHit3 = 0;
                PlayerBlockLogic(A, tempHit3, movingBlock, DontResetGrabTime, cursed_value_C);

                // Vine collisions.
                PlayerVineLogic(A);


                // Check NPC collisions
                int MessageNPC = 0;
                PlayerNPCLogic(A, tempSpring, tempShell, MessageNPC, movingBlock, tempHit3, oldSpeedY);

                // reduce player's multiplier
                if((Player[A].Location.SpeedY == 0 || Player[A].StandingOnNPC != 0 || Player[A].Slope > 0) && !Player[A].Slide && !FreezeNPCs)
                    Player[A].Multiplier = 0;

                if(Player[A].Mount == 2)
                    Player[A].Multiplier = 0;

                // Player-player collisions
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

                    bool pinch_death = (g_config.fix_player_crush_death && !pi.Strict) ? new_condition : old_condition;

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
                PlayerSharedScreenLogic(A);

                if(Player[A].Location.Y > level[Player[A].Section].Height + 64)
                    PlayerDead(A);

                if(!NPC[Player[A].StandingOnNPC]->IsAShell)
                    Player[A].ShellSurf = false;

                PlayerGrabCode(A, DontResetGrabTime); // Player holding NPC code **GRAB CODE**

                Player[A].RunRelease = !Player[A].Controls.Run && !Player[A].Controls.AltRun;
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

        Player[A].DuckRelease = !Player[A].Controls.Down;
        Player[A].AltRunRelease = !Player[A].Controls.AltRun;
    }

    CleanupVehicleNPCs();
}

void CleanupVehicleNPCs()
{
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
            for(int B = 1; B <= numPlayers; B++)
            {
                if(Player[B].StandingOnNPC == A)
                    Player[B].StandingOnVehiclePlr = NPC[A].Variant; // newly stores the player who owns the vehicle
            }
            NPC[0] = NPC[A]; // was NPC[C] = NPC[A] but C was not mutated
            KillNPC(A, 9);
        }
    }

    NPCQueues::PlayerTemp.clear();
}
