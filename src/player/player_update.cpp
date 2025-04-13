/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2025 Vitaly Novichkov <admin@wohlnet.ru>
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
#include "../frame_timer.h"
#include "../graphics.h"
#include "../controls.h"
#include "../script/msg_preprocessor.h"

#include "npc_id.h"
#include "npc_traits.h"
#include "eff_id.h"

#include "npc/npc_queues.h"
#include "main/game_loop_interrupt.h"


void p_PlayerMakeFlySparkle(const Location_t& loc, int Frame)
{
    NewEffect(EFFID_SPARKLE,
        newLoc(loc.X - 8 + dRand() * ((int)loc.Width + 16) - 4,
            loc.Y - 8 + dRand() * ((int)loc.Height + 16)),
        1, 0, ShadowMode);
    Effect[numEffects].Location.SpeedX = (dRand() / 2) - 0.25_n;
    Effect[numEffects].Location.SpeedY = (dRand() / 2) - 0.25_n;
    Effect[numEffects].Frame = Frame;
}

static void UpdateInvincibility()
{
    if(InvincibilityTime <= 0 || FreezeNPCs || LevelMacro != LEVELMACRO_OFF || !CheckLiving())
        return;

    if(InvincibilityTime == Physics.NPCPSwitch && !GameMenu)
    {
        StopMusic();
        StartMusic(-1);
    }

    InvincibilityTime--;

    if(InvincibilityTime == 195)
        PlaySound(SFX_CoinSwitchTimeout);

    if(InvincibilityTime == 0)
    {
        for(int A = 1; A <= numPlayers; A++)
        {
            if(!Player[A].Dead && Player[A].TimeToLive == 0)
                Player[A].Immune += 120;
        }

        if(!GameMenu)
            SwitchEndResumeMusic();
    }
}

bool UpdatePlayer()
{
    // these variables do not persist over the interrupt/resume routine
    num_t SlippySpeedX;
    tempf_t cursed_value_C;

    switch(g_gameLoopInterrupt.site)
    {
    case GameLoopInterrupt::UpdatePlayer_MessageNPC:
        goto resume_MessageNPC;
    case GameLoopInterrupt::UpdatePlayer_TriggerTalk:
        goto resume_TriggerTalk;
    case GameLoopInterrupt::UpdatePlayer_SuperWarp:
        goto resume_SuperWarp;
    default:
        break;
    }

    // these variables persist over the interrupt/resume routine
    int A;
    bool tempSpring;
    bool tempShell;
    tempSpring = false; // this one is probably safe to not share between players. it is reset between players unless tempShell is also hit
    tempShell = false; // this one marks whether a player has collided with a shell in the current frame and modifies the effects created when any player hits the top of NPC

    UpdateInvincibility(); // updates player invincibility status

    StealBonus(); // allows a dead player to come back to life by using a 1-up
    ClownCar(); // updates players in the clown car


    // A is the current player, numPlayers is the last player. this loop updates all the players
    for(A = 1; A <= numPlayers; A++)
    {
        // reset variables
        Player[A].ShowWarp = 0;
        Player[A].mountBump = 0;

        // this was shared over players in SMBX 1.3 -- if the line marked "MOST CURSED LINE" in player_block_logic.cpp becomes a source of incompatibility, we will need to restore that logic
        // this will include modifying the GameLoopInterrupt struct to include it
        // for now, this variable does not persist over the interrupt/resume routine
        cursed_value_C = 0;

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

        if(Player[A].Controls.Drop && Player[A].DropRelease && Player[A].Effect != PLREFF_COOP_WINGS)
        {
            // this is for the single player coop cheat code
            if(SingleCoop > 0 && Player[A].Controls.Down)
                SwapCoop();
            else
                DropBonus(A);
        }

        // for dropping something from the container. this makes the player have to let go of the drop button before dropping something else
        Player[A].DropRelease = !Player[A].Controls.Drop;

        // Handle the death effecs
        if(Player[A].TimeToLive > 0)
            UpdatePlayerTimeToLive(A);
        else if(Player[A].Effect == PLREFF_COOP_WINGS)
            PlayerEffectWings(A);
        else if(Player[A].Dead)
            UpdatePlayerDead(A);
        else
        {
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

                if(Player[A].Rolling)
                    Player[A].Slide = true;
                else if(Player[A].Slope > 0 && Player[A].Controls.Down &&
                   Player[A].Mount == 0 && Player[A].HoldingNPC == 0 &&
                   !(Player[A].Character == 3 || Player[A].Character == 4 || Player[A].Character == 5) &&
                   Player[A].GrabTime == 0)
                {
                    if(Player[A].Duck)
                        UnDuck(Player[A]);
                    Player[A].Slide = true;
                }
                else if(Player[A].Location.SpeedX == 0)
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
                    if(NPC[Player[A].StandingOnNPC].Type == NPCID_ICE_CUBE && NPC[Player[A].StandingOnNPC].Location.SpeedX == 0)
                        Player[A].Slippy = true;
                }

                if(Player[A].CurMazeZone)
                    PlayerMazeZoneMovement(A);
                // normal player movement
                else
                {
                    // this variable does not persist over the interrupt/resume routine
                    SlippySpeedX = Player[A].Location.SpeedX;


                    // Player's X movement. ---------------------------

                    // Code used to move the player while sliding down a slope
                    if(Player[A].Slide && !Player[A].Rolling)
                        PlayerSlideMovementX(A);
                    // TODO: mount-dependent logic
                    // if not sliding and in the clown car
                    else if(Player[A].Mount == 2)
                        PlayerVehicleDismountCheck(A);
                    // driving (standing on NPCID_COCKPIT)
                    else if(Player[A].Driving)
                        PlayerCockpitMovementX(A);
                    // if a fairy
                    else if(Player[A].Fairy)
                        PlayerFairyMovementX(A);
                    // TODO: state-dependent logic
                    // if the player is climbing a vine
                    else if(Player[A].Vine > 0)
                        PlayerVineMovement(A);
                    // if none of the above apply then the player controls like normal. remeber this is for the players X movement
                    else
                        PlayerMovementX(A, cursed_value_C);


                    // stop link when stabbing
                    if(Player[A].Character == 5)
                    {
                        if(Player[A].FireBallCD > 0 && (Player[A].Location.SpeedY == 0 || Player[A].Slope != 0 || Player[A].StandingOnNPC != 0))
                        {
                            if(Player[A].Slippy)
                                Player[A].Location.SpeedX = Player[A].Location.SpeedX * 0.75_rb;
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
                            NPC[Player[A].StandingOnNPC].Location = Block[NPC[Player[A].StandingOnNPC].Special].Location;

                        Player[A].Location.SpeedX += NPC[Player[A].StandingOnNPC].Location.SpeedX + (num_t)NPC[Player[A].StandingOnNPC].BeltSpeed;
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
                            if(Player[A].Location.SpeedX > -0.01_n && Player[A].Location.SpeedX < 0.01_n)
                                Player[A].Location.SpeedX = 0;
                        }
                        else
                        {
                            Player[A].Location.SpeedX = (Player[A].Location.SpeedX + SlippySpeedX * 3) / 4;
                            if(Player[A].Location.SpeedX > -0.01_n && Player[A].Location.SpeedX < 0.01_n)
                                Player[A].Location.SpeedX = 0;
                        }
                    }

                    // moved Slippy reset to immediately before the player Block logic
                    // bool wasSlippy = Player[A].Slippy;
                    // Player[A].Slippy = false;

                    if(Player[A].Quicksand > 1)
                    {
                        Player[A].Slide = false;
                        if(Player[A].Location.SpeedY >= 0)
                            Player[A].Location.SpeedX = Player[A].Location.SpeedX / 2;
                    }

                    // Apply movement -- this is where the actual movement happens
                    Player[A].Location.X += Player[A].Location.SpeedX;


                    // Players Y movement.
                    if(Block[Player[A].Slope].Location.SpeedY != 0 && Player[A].Slope != 0)
                        Player[A].Location.Y += Block[Player[A].Slope].Location.SpeedY;

                    if(Player[A].Fairy) // the player is a fairy
                        PlayerFairyMovementY(A);
                    // TODO: state-dependent logic
                    else if(Player[A].Wet > 0 && Player[A].Quicksand == 0) // the player is swimming
                        PlayerSwimMovementY(A);
                    else if(Player[A].Mount == 2)
                    {
                        // vehicle has own Y movement code in ClownCar()
                    }
                    else // the player is not swimming
                        PlayerMovementY(A);

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

                    if(Player[A].Slide && !Player[A].Rolling)
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
                    PlayerChar5Logic(A);

                Player[A].FloatRelease = !Player[A].Controls.Jump;

                // Player interactions
                Player[A].Location.SpeedX += Player[A].Bumped2;
                Player[A].Location.X += Player[A].Bumped2;
                Player[A].Bumped2 = 0;
                if(Player[A].Mount == 0)
                    Player[A].YoshiYellow = false;

                // perform various level bounds checks (level wrap, offscreen exit, left/right/top boundaries)
                PlayerLevelBoundsLogic(A);

                // gives the players the sparkles when he is flying
                if(
                        (
                                (!Player[A].YoshiBlue && (Player[A].CanFly || Player[A].CanFly2)) ||
                                (Player[A].Mount == 3 && Player[A].CanFly2)
                        ) || Player[A].FlySparks || InvincibilityTime
                        )
                {
                    if(iRand(4) == 0)
                        p_PlayerMakeFlySparkle(Player[A].Location, 0);
                }

                Tanooki(A); // tanooki suit code

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
                PlayerPinchedTimerUpdate(A);

                // these variables persist over the interrupt/resume routine
                int MessageNPC;
                bool DontResetGrabTime;
                MessageNPC = 0;
                DontResetGrabTime = false; // helps with grabbing things from the top

                // scoping variables shared between block logic and NPC logic
                {
                    tempf_t oldSpeedY = (tempf_t)Player[A].Location.SpeedY; // holds the players previous Y speed
                    bool movingBlock = false; // helps with collisions for moving blocks
                    int floorBlock = 0; // was previously called tempHit3

                    Player[A].Slippy = false;
                    Player[A].SlippyWall = false;

                    if(!Player[A].CurMazeZone)
                    {
                        // Block collisions.
                        PlayerBlockLogic(A, floorBlock, movingBlock, DontResetGrabTime, cursed_value_C);

                        // Vine collisions.
                        PlayerVineLogic(A);
                    }

                    // Check NPC collisions
                    PlayerNPCLogic(A, tempSpring, tempShell, MessageNPC, movingBlock, floorBlock, oldSpeedY);
                }

                // reduce player's multiplier
                if((Player[A].Location.SpeedY == 0 || Player[A].StandingOnNPC != 0 || Player[A].Slope > 0) && !Player[A].Slide && !FreezeNPCs && !InvincibilityTime)
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
                    PauseInit(PauseCode::Message, A);

                    // store entire routine state into g_gameLoopInterrupt
                    g_gameLoopInterrupt.site = GameLoopInterrupt::UpdatePlayer_MessageNPC;
                    g_gameLoopInterrupt.A = A;
                    g_gameLoopInterrupt.B = MessageNPC;
                    g_gameLoopInterrupt.bool1 = DontResetGrabTime;
                    g_gameLoopInterrupt.bool2 = tempSpring;
                    g_gameLoopInterrupt.bool3 = tempShell;
                    return true;

resume_MessageNPC:
                    // restore only the essentials for now, we may need to pause/resume again
                    A = g_gameLoopInterrupt.A;
                    MessageNPC = g_gameLoopInterrupt.B;

                    MessageText.clear();

                    if(NPC[MessageNPC].TriggerTalk != EVENT_NONE)
                    {
                        eventindex_t resume_index;
                        resume_index = ProcEvent_Safe(false, NPC[MessageNPC].TriggerTalk, A);
                        while(resume_index != EVENT_NONE)
                        {
                            g_gameLoopInterrupt.B = resume_index;
                            g_gameLoopInterrupt.site = GameLoopInterrupt::UpdatePlayer_TriggerTalk;
                            return true;

resume_TriggerTalk:
                            A = g_gameLoopInterrupt.A;
                            resume_index = g_gameLoopInterrupt.B;
                            resume_index = ProcEvent_Safe(true, resume_index, A);
                        }
                    }

                    // finish restoring state from g_gameLoopInterrupt
                    DontResetGrabTime = g_gameLoopInterrupt.bool1;
                    tempSpring = g_gameLoopInterrupt.bool2;
                    tempShell = g_gameLoopInterrupt.bool3;
                    g_gameLoopInterrupt.site = GameLoopInterrupt::None;

                    MessageNPC = 0;
                }

                YoshiEatCode(A);

                // pinch code
                if(!GodMode)
                    PlayerPinchedDeathCheck(A);

                if(false)
                {
resume_SuperWarp:
                    A = g_gameLoopInterrupt.A;

                    // restore some locals from this procedure
                    DontResetGrabTime = g_gameLoopInterrupt.bool1;
                    tempSpring = g_gameLoopInterrupt.bool2;
                    tempShell = g_gameLoopInterrupt.bool3;

                    // SuperWarp will resume at the correct index based on g_gameLoopInterrupt.B, and then clear g_gameLoopInterrupt.site
                }

                SuperWarp(A); // this sub checks warps

                if(g_gameLoopInterrupt.site != GameLoopInterrupt::None)
                {
                    // save some locals from this procedure
                    g_gameLoopInterrupt.bool1 = DontResetGrabTime;
                    g_gameLoopInterrupt.bool2 = tempSpring;
                    g_gameLoopInterrupt.bool3 = tempShell;
                    return true;
                }

                // shell surf
                if(Player[A].ShellSurf && Player[A].StandingOnNPC != 0)
                {
                    Player[A].Location.X = NPC[Player[A].StandingOnNPC].Location.X + (NPC[Player[A].StandingOnNPC].Location.Width - Player[A].Location.Width) / 2;
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
        }

        Player[A].DuckRelease = !Player[A].Controls.Down;
        Player[A].AltRunRelease = !Player[A].Controls.AltRun;
    }

    CleanupVehicleNPCs();

    return false;
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
