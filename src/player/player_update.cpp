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
                    PlayerFairyMovementX(A);
                // if the player is climbing a vine
                else if(Player[A].Vine > 0)
                    PlayerVineMovement(A);
                // if none of the above apply then the player controls like normal. remeber this is for the players X movement
                else
                    PlayerMovementX(A, cursed_value_C);


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
                    PlayerFairyMovementY(A);
                else if(Player[A].Wet > 0 && Player[A].Quicksand == 0) // the player is swimming
                    PlayerSwimMovementY(A);
                else if(Player[A].Mount == 2)
                {
                    // vehicle has own Y movement code elsewhere
                }
                else // the player is not swimming
                    PlayerMovementY(A);

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
