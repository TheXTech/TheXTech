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

#include <fmt_format_ne.h>

#include "sdl_proxy/sdl_stdinc.h"

#include "globals.h"

#include "player.h"
#include "graphics.h"
#include "npc.h"
#include "npc_id.h"
#include "npc_traits.h"
#include "npc/npc_queues.h"
#include "sound.h"
#include "editor.h"
#include "effect.h"
#include "game_main.h"
#include "eff_id.h"
#include "collision.h"
#include "layers.h"
#include "config.h"
#include "message.h"

#include "main/game_globals.h"
#include "main/translate.h"
#include "main/trees.h"
#include "main/game_strings.h"
#include "main/game_info.h"
#include "main/game_loop_interrupt.h"


static constexpr int plr_warp_scroll_speed = 8; // 8px / frame
static constexpr int plr_warp_scroll_max_frames = 260; // 4 seconds

static void s_TriggerDoorEffects(const Location_t& loc, bool do_big_door = true)
{
    for(Background_t& bgo : treeBackgroundQuery(loc, SORTMODE_ID))
    {
        if(CheckCollision(loc, bgo.Location))
        {
            if(bgo.Type == 88)
                NewEffect(EFFID_DOOR_S2_OPEN, static_cast<Location_t>(bgo.Location));
            else if(bgo.Type == 87)
                NewEffect(EFFID_DOOR_DOUBLE_S3_OPEN, static_cast<Location_t>(bgo.Location));
            else if(bgo.Type == 107)
                NewEffect(EFFID_DOOR_SIDE_S3_OPEN, static_cast<Location_t>(bgo.Location));
            else if(do_big_door && bgo.Type == 141)
            {
                Location_t bLoc = static_cast<Location_t>(bgo.Location);
                bLoc.set_width_center(104);
                NewEffect(EFFID_BIG_DOOR_OPEN, bLoc);
            }
        }
    }
}

// copied logic from checkWarp function
// parameter release_at_warp determines whether NPC should be dropped at the warp entrance
//   (set to false in new behavior where players are being teleported to warp)
static void s_WarpReleaseItems(const Warp_t& warp, int A, bool backward, bool release_at_warp = true)
{
    Player_t& plr = Player[A];

    int direction  = backward ? warp.Direction2 : warp.Direction;
    auto &entrance = backward ? warp.Exit       : warp.Entrance;

    if(warp.NoYoshi && plr.YoshiPlayer > 0)
        YoshiSpit(A);

    if(!warp.WarpNPC || (plr.Mount == 3 && (plr.YoshiNPC != 0 || plr.YoshiPlayer != 0) && warp.NoYoshi))
    {
        if(plr.HoldingNPC > 0)
        {
            if(NPC[plr.HoldingNPC].Type == NPCID_HEAVY_THROWER)
                NPCHit(plr.HoldingNPC, 3, plr.HoldingNPC);
        }

        if((plr.Character == 3 && release_at_warp) ||
          (plr.Character == 4 && warp.Effect == 1 && direction == 1 && release_at_warp))
        {
            NPC[plr.HoldingNPC].Location.Y = entrance.Y;

            if(plr.HoldingNPC > 0 && plr.HoldingNPC <= numNPCs)
                treeNPCUpdate(plr.HoldingNPC);
        }

        plr.HoldingNPC = 0;

        if(plr.YoshiNPC > 0)
            YoshiSpit(A);
    }

    if(plr.HoldingNPC > 0)
    {
        if(NPC[plr.HoldingNPC].Type == NPCID_ICE_CUBE) // can't bring ice through warps
        {
            NPC[plr.HoldingNPC].HoldingPlayer = 0;
            plr.HoldingNPC = 0;
        }
    }

    plr.StandingOnNPC = 0;
}

// steal the mount from a player (because they just entered a no-mount warp)
static void s_WarpStealMount(int A)
{
    Player_t& p = Player[A];
    if(OwedMount[A] == 0 && p.Mount > 0 && p.Mount != 2)
    {
        OwedMount[A] = p.Mount;
        OwedMountType[A] = p.MountType;
    }
    p.Mount = 0;
    p.MountType = 0;
    p.MountOffsetY = 0;
    SizeCheck(Player[A]);
    UpdateYoshiMusic();
}

// fix the location of any players in pets, for after a player gets teleported
static void s_FixPlayersInPets(const Screen_t& screen)
{
    for(int plr_i = 0; plr_i < screen.player_count; plr_i++)
    {
        int o_A = screen.players[plr_i];

        const Player_t& o_p = Player[o_A];
        if(o_p.Effect == PLREFF_PET_INSIDE)
            PlayerEffects(o_A);
    }
}

static void s_CheckWarpLevelExit(Player_t& plr, const Warp_t& warp, int lvl_counter, int map_counter)
{
    if(warp.level != STRINGINDEX_NONE)
    {
        GoToLevel = GetS(warp.level);
        GoToLevelNoGameThing = warp.noEntranceScene;
        plr.Effect = PLREFF_WAITING;
        plr.Effect2 = lvl_counter;
        ReturnWarp = plr.Warp;

        if(IsHubLevel)
            ReturnWarpSaved = ReturnWarp;

        StartWarp = warp.LevelWarp;
    }
    else if(warp.MapWarp)
    {
        plr.Effect = PLREFF_WAITING;
        plr.Effect2 = map_counter;
    }
}

static void s_InitWarpScroll(Player_t& p, const Location_t& warp_enter, const Location_t& warp_exit, int min_frames = 0)
{
    unsigned int warp_dist = (int)num_t::dist(warp_enter.X - warp_exit.X, warp_enter.Y - warp_exit.Y);

    int scroll_frames = warp_dist / plr_warp_scroll_speed;
    if(scroll_frames < min_frames)
        scroll_frames = min_frames;
    if(scroll_frames > plr_warp_scroll_max_frames)
        scroll_frames = plr_warp_scroll_max_frames;

    p.Effect2 = 128 + scroll_frames;
}

static void s_WarpFaderLogic(bool is_reverse, int A, int transitEffect, const Location_t& focus, bool normal_ready, bool none_ready)
{
    int fader_index = vScreenIdxByPlayer(A);
    SDL_assert_release(0 <= fader_index && fader_index <= c_vScreenCount);

    auto& fader = g_levelVScreenFader[fader_index];

    if(is_reverse && !fader.isVisible())
        return;

    int fade_from = (is_reverse) ? 65 :  0;
    int fade_to   = (is_reverse) ?  0 : 65;
    int rate      = 3;
    bool ready    = normal_ready;

    switch(transitEffect)
    {
    default:
        if(transitEffect >= ScreenFader::S_CUSTOM)
            goto generic_fade;

    // fallthrough
    case LevelDoor::TRANSIT_SCROLL:
    case LevelDoor::TRANSIT_NONE:
        ready = none_ready;
        rate = (g_config.EnableInterLevelFade ? 8 : 64);

    // fallthrough
    case LevelDoor::TRANSIT_FADE:
        transitEffect = ScreenFader::S_FADE;
        goto generic_fade;

    case LevelDoor::TRANSIT_CIRCLE_FADE:
        transitEffect = ScreenFader::S_CIRCLE;
        goto generic_fade;

    case LevelDoor::TRANSIT_FLIP_H:
        transitEffect = ScreenFader::S_FLIP_H;
        goto generic_fade;

    case LevelDoor::TRANSIT_FLIP_V:
        transitEffect = ScreenFader::S_FLIP_V;
        goto generic_fade;

    generic_fade:
        if(ready)
            fader.setupFader(rate, fade_from, fade_to, transitEffect,
                             true,
                             num_t::round(focus.X + focus.Width / 2),
                             num_t::round(focus.Y + focus.Height / 2),
                             fader_index);
        break;
    }
}

void s_delay_pipe_exit(int A)
{
    Player_t& p = Player[A];

    const auto& warp = Warp[p.Warp];
    const auto warp_dir_exit = (p.WarpBackward) ? warp.Direction : warp.Direction2;
    bool warp_vertical = (warp_dir_exit == LevelDoor::EXIT_UP) || (warp_dir_exit == LevelDoor::EXIT_DOWN);

    // number of players ahead of this one
    int frames_before = 0;

    for(int o_A = 1; o_A <= numPlayers; o_A++)
    {
        if(o_A == A)
            continue;

        Player_t& o_p = Player[o_A];
        if(!o_p.Dead && o_p.TimeToLive == 0 && o_p.Effect == PLREFF_WARP_PIPE && o_p.Warp == p.Warp && o_p.WarpBackward == p.WarpBackward && o_p.Effect2 > 1 && (o_p.Effect2 < 128 || o_p.Effect2 >= 2000))
        {
            // wait for player to warp
            if(warp_vertical)
                frames_before += (int)o_p.Location.Height;
            else
                frames_before += (int)o_p.Location.Width;

            // pause between players
            frames_before += 70;
        }
    }

    // put in new pipe holding state
    if(frames_before)
    {
        p.Effect = PLREFF_WARP_PIPE;
        p.Effect2 = 2010 + frames_before;
    }
}

bool PlayerWaitingInWarp(const Player_t& p)
{
    return (p.Effect == PLREFF_WARP_PIPE && p.Effect2 >= 2000)
        || (p.Effect == PLREFF_WAITING && p.Effect2 > 30 && p.Effect2 <= 2000);
}

bool PlayerScrollingInWarp(const Player_t& p)
{
    return (p.Effect == PLREFF_WARP_PIPE || p.Effect == PLREFF_WARP_DOOR)
        && (p.Effect2 >= 128 && p.Effect2 <= 128 + plr_warp_scroll_max_frames);
}

void PlayerEffectWarpPipe(int A)
{
    Player_t& p = Player[A];

    p.SpinJump = false;
    p.TailCount = 0;
    p.Location.SpeedY = 0;

    bool backward = p.WarpBackward;
    const auto &warp = Warp[p.Warp];
    Location_t warp_enter = static_cast<Location_t>(backward ? warp.Exit : warp.Entrance);
    Location_t warp_exit = static_cast<Location_t>(backward ? warp.Entrance : warp.Exit);
    const auto &warp_dir_enter = backward ? warp.Direction2 : warp.Direction;
    const auto &warp_dir_exit = backward ? warp.Direction : warp.Direction2;

    bool same_section = SectionCollision(p.Section, warp_exit);
    bool do_scroll = (warp.transitEffect == LevelDoor::TRANSIT_SCROLL) && same_section;
    bool is_level_quit = warp.level != STRINGINDEX_NONE || warp.MapWarp;

    // teleport other players into warp in shared screen mode
    Screen_t& screen = ScreenByPlayer(A);
    bool is_shared_screen = (screen.Type == 3);

    if(p.Effect2 == 0) // Entering pipe
    {
        int leftToGoal = 0;

        if(warp_dir_enter == 3)
        {
            p.Location.Y += 1;
            p.Location.X = warp_enter.X + (warp_enter.Width - p.Location.Width) / 2;

            leftToGoal = (int)(warp_enter.Y + warp_enter.Height - p.Location.Y + 0.5_n);

            if(p.Location.Y > warp_enter.Y + warp_enter.Height + 8)
            {
                if(do_scroll)
                    s_InitWarpScroll(p, warp_enter, warp_exit);
                else
                    p.Effect2 = 1;
            }

            if(p.Mount == 0)
                p.Frame = 15;

            if(p.HoldingNPC > 0)
            {
                NPC[p.HoldingNPC].Location.Y = p.Location.Y + Physics.PlayerGrabSpotY[p.Character][p.State] + 32 - NPC[p.HoldingNPC].Location.Height;
                NPC[p.HoldingNPC].Location.X = p.Location.X + (p.Location.Width - NPC[p.HoldingNPC].Location.Width) / 2;
            }
        }
        else if(warp_dir_enter == 1)
        {
            p.Location.Y -= 1;
            p.Location.X = warp_enter.X + (warp_enter.Width - p.Location.Width) / 2;

            leftToGoal = (int)(p.Location.Y + p.Location.Height - warp_enter.Y + 0.5_n);

            if(p.Location.Y + p.Location.Height + 8 < warp_enter.Y)
            {
                if(do_scroll)
                    s_InitWarpScroll(p, warp_enter, warp_exit);
                else
                    p.Effect2 = 1;
            }

            if(p.HoldingNPC > 0)
            {
                NPC[p.HoldingNPC].Location.Y = p.Location.Y + Physics.PlayerGrabSpotY[p.Character][p.State] + 32 - NPC[p.HoldingNPC].Location.Height;
                NPC[p.HoldingNPC].Location.X = p.Location.X + (p.Location.Width - NPC[p.HoldingNPC].Location.Width) / 2;
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
            p.Location.X -= 0.5_n;

            leftToGoal = (int)((p.Location.X + p.Location.Width - warp_enter.X) * 2 + 0.5_n);

            if(p.Location.X + p.Location.Width + 8 < warp_enter.X)
            {
                if(do_scroll)
                    s_InitWarpScroll(p, warp_enter, warp_exit);
                else
                    p.Effect2 = 1;
            }

            if(p.HoldingNPC > 0)
            {
                NPC[p.HoldingNPC].Location.Y = p.Location.Y + Physics.PlayerGrabSpotY[p.Character][p.State] + 32 - NPC[p.HoldingNPC].Location.Height;
//                    if(p.Direction > 0) // Always false
//                        NPC[p.HoldingNPC].Location.X = p.Location.X + Physics.PlayerGrabSpotX[p.Character][p.State];
//                    else
                NPC[p.HoldingNPC].Location.X = p.Location.X + p.Location.Width - Physics.PlayerGrabSpotX[p.Character][p.State] - NPC[p.HoldingNPC].Location.Width;
            }
            p.Location.SpeedX = -0.5_n;
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
            p.Location.X += 0.5_n;

            leftToGoal = (int)((warp_enter.X + warp_enter.Width - p.Location.X) * 2 + 0.5_n);

            if(p.Location.X > warp_enter.X + warp_enter.Width + 8)
            {
                if(do_scroll)
                    s_InitWarpScroll(p, warp_enter, warp_exit);
                else
                    p.Effect2 = 1;
            }

            if(p.HoldingNPC > 0)
            {
                NPC[p.HoldingNPC].Location.Y = p.Location.Y + Physics.PlayerGrabSpotY[p.Character][p.State] + 32 - NPC[p.HoldingNPC].Location.Height;
//                    if(p.Direction > 0) // always true
                NPC[p.HoldingNPC].Location.X = p.Location.X + Physics.PlayerGrabSpotX[p.Character][p.State];
//                    else
//                        NPC[p.HoldingNPC].Location.X = p.Location.X + p.Location.Width - Physics.PlayerGrabSpotX[p.Character][p.State] - NPC[p.HoldingNPC].Location.Width;
            }
            p.Location.SpeedX = 0.5_n;
            PlayerFrame(p);
            p.Location.SpeedX = 0;
        }

        if(p.HoldingNPC > 0 && p.HoldingNPC <= numNPCs)
            treeNPCUpdate(p.HoldingNPC);

        // teleport other players into the pipe warp
        if(is_shared_screen && leftToGoal == 8)
        {
            int vscreen_A = vScreenIdxByPlayer(A);
            bool do_tele = !vScreenCollision(vscreen_A, warp_exit);

            if(do_tele)
            {
                SharedScreenAvoidJump_Pre(screen);

                for(int plr_i = 0; plr_i < screen.player_count; plr_i++)
                {
                    int o_A = screen.players[plr_i];
                    if(o_A == A)
                        continue;

                    Player_t& o_p = Player[o_A];

                    // revive someone in wings and bring to warp
                    if(o_p.Effect == PLREFF_COOP_WINGS)
                        o_p.Dead = false;

                    // in the mouth of an onscreen player's Pet?
                    bool in_onscreen_pet = !warp.NoYoshi && InOnscreenPet(o_A, screen);

                    bool status_match = (o_p.Effect == p.Effect && o_p.Warp == p.Warp && o_p.WarpBackward == p.WarpBackward);

                    if(!o_p.Dead && o_p.TimeToLive == 0 && !in_onscreen_pet && !status_match)
                    {
                        RemoveFromPet(o_A);

                        s_WarpReleaseItems(warp, o_A, p.WarpBackward, false);

                        o_p.Warp = p.Warp;
                        o_p.WarpBackward = p.WarpBackward;
                        o_p.Effect = p.Effect;
                        // make other player behind so that this player will exit first
                        o_p.Effect2 = 0;
                        o_p.Location.X = warp_enter.X + (warp_enter.Width - o_p.Location.Width) / 2;
                        o_p.Location.Y = warp_enter.Y + (warp_enter.Height - o_p.Location.Height) / 2;
                        o_p.Location.SpeedX = 0;
                        o_p.Location.SpeedY = 0;
                    }
                }

                SharedScreenAvoidJump_Post(screen, (do_scroll) ? 0 : 200);
            }
        }

        // D_pLogDebug("Warping: %g (same section? %s!)", leftToGoal, SectionCollision(p.Section, warp_exit) ? "yes" : "no");

        // trigger fader when there are 16 pixels left (for normal effects) or 0 pixels left (for none or scroll)
        s_WarpFaderLogic(false, A, warp.transitEffect, warp_enter, leftToGoal == 16, !is_level_quit && !same_section && leftToGoal == 0);
    }
    else if(p.Effect2 == 1)  // Exiting pipe (initialization)
    {
        if(warp.NoYoshi)
        {
            UnDuck(p);
            s_WarpStealMount(A);
        }

        if(warp_dir_exit == 1)
        {
            p.Location.X = warp_exit.X + (warp_exit.Width - p.Location.Width) / 2;
            p.Location.Y = warp_exit.Y - p.Location.Height - 8;

            if(p.Mount == 0)
                p.Frame = 15;

            if(p.HoldingNPC > 0)
            {
                NPC[p.HoldingNPC].Location.Y = p.Location.Y + Physics.PlayerGrabSpotY[p.Character][p.State] + 32 - NPC[p.HoldingNPC].Location.Height;
                NPC[p.HoldingNPC].Location.X = p.Location.X + (p.Location.Width - NPC[p.HoldingNPC].Location.Width) / 2;
            }
        }
        else if(warp_dir_exit == 3)
        {
            p.Location.X = warp_exit.X + (warp_exit.Width - p.Location.Width) / 2;
            p.Location.Y = warp_exit.Y + warp_exit.Height + 8;

            if(p.Mount == 0)
                p.Frame = 15;

            if(p.HoldingNPC > 0)
            {
                NPC[p.HoldingNPC].Location.Y = p.Location.Y + Physics.PlayerGrabSpotY[p.Character][p.State] + 32 - NPC[p.HoldingNPC].Location.Height;
                NPC[p.HoldingNPC].Location.X = p.Location.X + (p.Location.Width - NPC[p.HoldingNPC].Location.Width) / 2;
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

        if(p.HoldingNPC > 0 && p.HoldingNPC <= numNPCs)
            treeNPCUpdate(p.HoldingNPC);

        p.Effect2 = 100;
        if(p.Duck)
        {
            if(warp_dir_exit == 1 || warp_dir_exit == 3)
                UnDuck(Player[A]);
        }

        CheckSection(A);

        if(p.HoldingNPC > 0)
            CheckSectionNPC(p.HoldingNPC);

        // set any other players warping to the same pipe to this state (needed to avoid splitting a shared screen)
        if(is_shared_screen && !do_scroll)
        {
            for(int plr_i = 0; plr_i < screen.player_count; plr_i++)
            {
                int o_A = screen.players[plr_i];
                if(A == o_A)
                    continue;

                Player_t& o_p = Player[o_A];
                if(!o_p.Dead && o_p.TimeToLive == 0 && o_p.Effect == PLREFF_WARP_PIPE && o_p.Effect2 == 0 && o_p.Warp == Player[A].Warp && o_p.WarpBackward == Player[A].WarpBackward)
                {
                    o_p.Location.X = p.Location.X + (p.Location.Width - o_p.Location.Width) / 2;
                    o_p.Location.Y = p.Location.Y + p.Location.Height - o_p.Location.Height;

                    CheckSection(o_A);

                    o_p.Effect = PLREFF_WARP_PIPE;
                    o_p.Effect2 = 1;
                }
            }

            // update position of any players in pets
            s_FixPlayersInPets(screen);

            // disable any tempX/TempY (no longer needed)
            SharedScreenResetTemp(screen);
        }

        // delay based on number of players ahead of this one
        if(is_shared_screen)
            s_delay_pipe_exit(A);

        // many-player code
        if(g_ClonedPlayerMode)
        {
            for(int B = 1; B <= numPlayers; B++)
            {
                if(B != A)
                {
                    if(warp_dir_exit != 3)
                        Player[B].Location.Y = p.Location.Y + p.Location.Height - Player[B].Location.Height;
                    else
                        Player[B].Location.Y = p.Location.Y;

                    Player[B].Location.X = p.Location.X + (p.Location.Width - Player[B].Location.Width) / 2;
                    Player[B].Location.SpeedY = dRand() * 24 - 12;
                    Player[B].Effect = PLREFF_WAITING;
                    Player[B].Effect2 = 0;
                    CheckSection(B);

                    if(Player[B].HoldingNPC > 0)
                        CheckSectionNPC(Player[B].HoldingNPC);
                }
            }
        }

        // reverse screen fade
        if(!is_level_quit)
            s_WarpFaderLogic(true, A, warp.transitEffect, warp_exit, true, true);

        s_CheckWarpLevelExit(p, warp, 2970, 2970);
    }
    else if(p.Effect2 >= 2000) // NEW >2P holding state for pipe exit
    {
        p.Effect2 -= 1;

        if(p.Effect2 <= 2000)
        {
            SharedScreenAvoidJump_Pre(screen);

            p.Effect2 = 2;
            if(backward || !warp.cannonExit)
                PlaySoundSpatial(SFX_Warp, p.Location);

            SharedScreenAvoidJump_Post(screen, 0);
        }
    }
    else if(p.Effect2 > 128) // Scrolling between pipes
    {
        num_t targetX = p.Location.X;
        num_t targetY = p.Location.Y;

        if(warp_dir_exit == 1)
        {
            targetX = warp_exit.X + (warp_exit.Width - p.Location.Width) / 2;
            targetY = warp_exit.Y - p.Location.Height - 8;
        }
        else if(warp_dir_exit == 3)
        {
            targetX = warp_exit.X + (warp_exit.Width - p.Location.Width) / 2;
            targetY = warp_exit.Y + warp_exit.Height + 8;
        }
        else if(warp_dir_exit == 2)
        {
            if(p.Mount == 3)
                p.Location.Height = 30;

            targetX = warp_exit.X - p.Location.Width - 8;
            targetY = warp_exit.Y + warp_exit.Height - p.Location.Height - 2;
        }
        else if(warp_dir_exit == 4)
        {
            if(p.Mount == 3)
                p.Location.Height = 30;

            targetX = warp_exit.X + warp_exit.Width + 8;
            targetY = warp_exit.Y + warp_exit.Height - p.Location.Height - 2;
        }

        int frames_left = p.Effect2 - 128;

        p.Location.X += (targetX - p.Location.X) / frames_left;
        p.Location.Y += (targetY - p.Location.Y) / frames_left;

        p.Effect2 -= 1;

        if(p.Effect2 <= 128)
            p.Effect2 = 1;
    }
    else if(p.Effect2 >= 100) // Waiting until exit pipe
    {
        p.Effect2 += 1;

        if(p.Effect2 >= 110)
        {
            p.Effect2 = 2;
            if(backward || !warp.cannonExit)
                PlaySoundSpatial(SFX_Warp, p.Location);
        }
    }
    else if(p.Effect2 == 2) // Proceeding the pipe exiting
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
                NPC[p.HoldingNPC].Location.X = p.Location.X + (p.Location.Width - NPC[p.HoldingNPC].Location.Width) / 2;
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
                NPC[p.HoldingNPC].Location.X = p.Location.X + (p.Location.Width - NPC[p.HoldingNPC].Location.Width) / 2;
            }

            if(p.Mount == 0)
                p.Frame = 15;

            // make players less likely to collide chaotically out of UP exits
            if(is_shared_screen || (numPlayers > 2 && !g_ClonedPlayerMode))
                p.StandUp2 = true;
        }
        else if(warp_dir_exit == LevelDoor::EXIT_LEFT)
        {
            p.Location.X -= 0.5_n;
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
                p.Location.SpeedX = -0.5_n;
                PlayerFrame(p);
                p.Location.SpeedX = 0;
            }
        }
        else if(warp_dir_exit == LevelDoor::EXIT_RIGHT)
        {
            p.Location.X += 0.5_n;
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
                p.Location.SpeedX = -0.5_n;
                PlayerFrame(p);
                p.Location.SpeedX = 0;
            }
        }

        if(p.HoldingNPC > 0 && p.HoldingNPC <= numNPCs)
            treeNPCUpdate(p.HoldingNPC);
    }
    else if(p.Effect2 == 3) // Finishing the pipe exiting / shooting
    {
        if(!backward && warp.cannonExit)
        {
            PlaySoundSpatial(SFX_Bullet, p.Location);
            auto loc = warp_exit;
            if(warp_dir_exit == LevelDoor::EXIT_LEFT || warp_dir_exit == LevelDoor::EXIT_RIGHT)
                loc.Y += loc.Height - (p.Location.Height / 2) - (loc.Height / 2);
            NewEffect(EFFID_STOMP_INIT, loc, p.Direction); // Cannon pipe shoot effect
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

        p.Effect = PLREFF_NORMAL;
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
            NPC[p.HoldingNPC].Effect = NPCEFF_NORMAL;

        if(g_ClonedPlayerMode)
        {
            for(int B = 1; B <= numPlayers; B++)
            {
                if(B != A)
                {
                    if(warp_dir_exit != 1)
                        Player[B].Location.Y = p.Location.Y + p.Location.Height - Player[B].Location.Height;
                    else
                        Player[B].Location.Y = p.Location.Y;

                    Player[B].Location.X = p.Location.X + (p.Location.Width - Player[B].Location.Width) / 2;
                    Player[B].Location.SpeedY = dRand() * 24 - 12;
                    Player[B].Effect = PLREFF_NORMAL;
                    Player[B].Effect2 = 0;
                    CheckSection(B);
                }
            }
        }

        if(warp.eventExit != EVENT_NONE)
            TriggerEvent(warp.eventExit, A);
    }
}

void PlayerEffectWarpDoor(int A)
{
    Player_t& p = Player[A];

    bool backward = p.WarpBackward;
    const Warp_t &warp = Warp[p.Warp];
    const Location_t warp_enter = static_cast<Location_t>(backward ? warp.Exit : warp.Entrance);
    const Location_t warp_exit = static_cast<Location_t>(backward ? warp.Entrance : warp.Exit);

    bool same_section = SectionCollision(p.Section, warp_exit);
    bool do_scroll = (warp.transitEffect == LevelDoor::TRANSIT_SCROLL) && same_section;
    bool is_level_quit = warp.level != STRINGINDEX_NONE || warp.MapWarp;

    if(p.HoldingNPC > 0)
    {
        NPC[p.HoldingNPC].Location.Y = p.Location.Y + Physics.PlayerGrabSpotY[p.Character][p.State] + 32 - NPC[p.HoldingNPC].Location.Height;
        NPC[p.HoldingNPC].Location.X = p.Location.X + (p.Location.Width - NPC[p.HoldingNPC].Location.Width) / 2;
        treeNPCUpdate(p.HoldingNPC);
    }

    p.Effect2 += 1;

    if(p.Mount == 0 && p.Character != 5)
        p.Frame = 13;

    if(p.Character == 5)
        p.Frame = 1;

    // trigger warp fader when p.Effect2 is 5 (for normal animations) or 20 (for none/scroll animations)
    s_WarpFaderLogic(false, A, warp.transitEffect, warp_enter, p.Effect2 == 5, !is_level_quit && !same_section && (p.Effect2 == 20));

    // teleport other players into door in shared screen mode
    Screen_t& screen = ScreenByPlayer(A);
    bool is_shared_screen = (screen.Type == 3);
    if(is_shared_screen && (p.Effect2 == 15))
    {
        int vscreen_A = vScreenIdxByPlayer(A);
        bool do_tele = is_shared_screen && !vScreenCollision(vscreen_A, warp_exit);

        if(do_tele)
        {
            SharedScreenAvoidJump_Pre(screen);

            for(int plr_i = 0; plr_i < screen.player_count; plr_i++)
            {
                int o_A = screen.players[plr_i];
                if(o_A == A)
                    continue;

                Player_t& o_p = Player[o_A];

                // revive someone in wings and bring to warp
                if(o_p.Effect == PLREFF_COOP_WINGS)
                    o_p.Dead = false;

                // in the mouth of an onscreen player's Pet?
                bool in_onscreen_pet = !warp.NoYoshi && InOnscreenPet(o_A, screen);

                bool status_match = (o_p.Effect == p.Effect && o_p.Warp == p.Warp && o_p.WarpBackward == p.WarpBackward);

                if(!o_p.Dead && o_p.TimeToLive == 0 && !in_onscreen_pet && !status_match)
                {
                    RemoveFromPet(o_A);

                    s_WarpReleaseItems(warp, o_A, p.WarpBackward, false);

                    o_p.Warp = p.Warp;
                    o_p.WarpBackward = p.WarpBackward;
                    o_p.Effect = p.Effect;
                    // 1 frame behind so that this player will exit first
                    o_p.Effect2 = 14;
                    o_p.Location.X = warp_enter.X + (warp_enter.Width - o_p.Location.Width) / 2;
                    o_p.Location.Y = warp_enter.Y + warp_enter.Height - o_p.Location.Height;
                    o_p.Location.SpeedX = 0;
                    o_p.Location.SpeedY = 0;
                }
            }

            SharedScreenAvoidJump_Post(screen, (do_scroll) ? 0 : 200);
        }
    }

    // start the scroll effect
    if(do_scroll && p.Effect2 == 29)
    {
        s_InitWarpScroll(p, warp_enter, warp_exit, 30);
        SoundPause[SFX_Door] = 60;
    }
    // process the scroll effect
    else if(p.Effect2 >= 128)
    {
        num_t targetX = warp_exit.X + (warp_exit.Width - p.Location.Width) / 2;
        num_t targetY = warp_exit.Y + warp_exit.Height - p.Location.Height;

        // += 1 above
        p.Effect2 -= 1;

        int frames_left = p.Effect2 - 128;

        p.Location.X += (targetX - p.Location.X) / frames_left;
        p.Location.Y += (targetY - p.Location.Y) / frames_left;

        if(frames_left == 30)
        {
            s_TriggerDoorEffects(warp_exit);
            PlaySoundSpatial(SFX_Door, warp_exit);
        }

        p.Effect2 -= 1;

        if(p.Effect2 <= 128)
            p.Effect2 = 30;
    }

    // finalize the warp
    if(p.Effect2 >= 30 && p.Effect2 < 128)
    {
        if(warp.NoYoshi)
        {
            s_WarpStealMount(A);
            p.Frame = 1;
        }

        p.Location.X = warp_exit.X + (warp_exit.Width - p.Location.Width) / 2;
        p.Location.Y = warp_exit.Y + warp_exit.Height - p.Location.Height;

        // set any other players warping to the same door into the door holding pattern (needed to avoid splitting a shared screen)
        if(is_shared_screen)
        {
            for(int plr_i = 0; plr_i < screen.player_count; plr_i++)
            {
                int o_A = screen.players[plr_i];
                if(A == o_A)
                    continue;

                Player_t& o_p = Player[o_A];
                if(!o_p.Dead && o_p.TimeToLive == 0 && o_p.Effect == PLREFF_WARP_DOOR && o_p.Warp == Player[A].Warp && o_p.WarpBackward == Player[A].WarpBackward)
                {
                    o_p.Location.X = warp_exit.X + (warp_exit.Width - o_p.Location.Width) / 2;
                    o_p.Location.Y = warp_exit.Y + warp_exit.Height - o_p.Location.Height;

                    CheckSection(o_A);

                    o_p.Effect = PLREFF_WAITING;
                    o_p.Effect2 = 131;
                    o_p.WarpCD = 40;

                    if(warp.NoYoshi)
                    {
                        s_WarpStealMount(A);
                        p.Frame = 1;
                    }
                }
            }

            // update position of any players in pets
            s_FixPlayersInPets(screen);

            SharedScreenResetTemp(screen);
        }

        CheckSection(A);

        if(p.HoldingNPC > 0)
        {
            if(!p.Controls.Run)
                p.Controls.Run = true;

            PlayerGrabCode(A);
        }

        p.Effect = PLREFF_NORMAL;
        p.Effect2 = 0;
        p.WarpCD = 40;

        if(!is_level_quit)
        {
            // reverse warp fader
            s_WarpFaderLogic(true, A, warp.transitEffect, warp_exit, true, true);

            if(warp.eventExit != EVENT_NONE)
                TriggerEvent(warp.eventExit, A);
        }

        s_CheckWarpLevelExit(p, warp, 3000, 2970);

        if(g_ClonedPlayerMode)
        {
            for(int B = 1; B <= numPlayers; B++)
            {
                if(B != A)
                {
                    Player[B].Location.Y = p.Location.Y + p.Location.Height - Player[B].Location.Height;
                    Player[B].Location.X = p.Location.X + (p.Location.Width - Player[B].Location.Width) / 2;
                    Player[B].Location.SpeedY = dRand() * 24 - 12;
                    CheckSection(B);

                    if(Player[B].HoldingNPC > 0)
                    {
                        if(Player[B].Direction > 0)
                            NPC[Player[B].HoldingNPC].Location.X = Player[B].Location.X + Physics.PlayerGrabSpotX[Player[B].Character][Player[B].State];
                        else
                            NPC[Player[B].HoldingNPC].Location.X = Player[B].Location.X + Player[B].Location.Width - Physics.PlayerGrabSpotX[Player[B].Character][Player[B].State] - NPC[p.HoldingNPC].Location.Width;

                        // TODO: investigate this possible crashing bug (should be Player[B].HoldingNPC)
                        NPC[p.HoldingNPC].Location.Y = p.Location.Y + Physics.PlayerGrabSpotY[p.Character][p.State] + 32 - NPC[p.HoldingNPC].Location.Height;
                        NPC[Player[B].HoldingNPC].Section = Player[B].Section;

                        if(p.HoldingNPC > 0 && p.HoldingNPC <= numNPCs)
                            treeNPCUpdate(p.HoldingNPC);

                        // already checked this one > 0 above
                        if(Player[B].HoldingNPC <= numNPCs)
                            treeNPCUpdate(Player[B].HoldingNPC);
                    }
                }
            }
        }
    }
}

void PlayerEffectWarpWait(int A)
{
    Player_t& p = Player[A];

    // door exit holding pattern (exit blocked)
    if(p.Effect2 == 131)
    {
        bool tempBool = false;
        for(int B = 1; B <= numPlayers; B++)
        {
            // Was previously only B != A. New conditions only apply in >2P
            bool check_coll = B != A && !Player[B].Dead && (Player[B].Effect != PLREFF_WAITING || B < A) && (Player[B].Effect != PLREFF_PET_INSIDE);
            if(check_coll && CheckCollision(p.Location, Player[B].Location))
                tempBool = true;
        }

        if(!tempBool)
        {
            p.Effect2 = 130;

            const auto& warp_exit = p.WarpBackward ? Warp[p.Warp].Entrance : Warp[p.Warp].Exit;

            s_TriggerDoorEffects(static_cast<Location_t>(warp_exit), false);

            SoundPause[SFX_Door] = 0;
            PlaySoundSpatial(SFX_Door, p.Location);
        }
    }
    // door exit wait
    else if(p.Effect2 <= 130)
    {
        p.Effect2 -= 1;
        if(p.Effect2 == 100)
        {
            Screen_t& screen = ScreenByPlayer(A);

            SharedScreenAvoidJump_Pre(screen);

            p.Effect = PLREFF_NORMAL;
            p.Effect2 = 0;

            SharedScreenAvoidJump_Post(screen, 0);
        }
    }
    // 2P holding condition for start warp (pipe exit)
    else if(p.Effect2 <= 300)
    {
        p.Effect2 -= 1;
        if(p.Effect2 == 200)
        {
            Screen_t& screen = ScreenByPlayer(A);

            SharedScreenAvoidJump_Pre(screen);

            p.Effect2 = 100;
            p.Effect = PLREFF_WARP_PIPE;

            SharedScreenAvoidJump_Post(screen, 0);
        }
    }
    else if(p.Effect2 <= 1000) // Start Wait for pipe
    {
        p.Effect2 -= 1;
        if(p.Effect2 == 900)
        {
            p.Effect = PLREFF_WARP_PIPE;
            p.Effect2 = 100;

            const Screen_t& screen = ScreenByPlayer(A);

            bool do_modern = !g_ClonedPlayerMode && (numPlayers > 2 || screen.Type == ScreenTypes::SharedScreen || XMessage::GetStatus() != XMessage::Status::local);
            if(!do_modern)
            {
                // 2P holding condition for start warp
                if(A == screen.players[1])
                {
                    p.Effect = PLREFF_WAITING;
                    p.Effect2 = 300;
                }
            }
            else
                s_delay_pipe_exit(A);
        }
    }
    else if(p.Effect2 <= 2000) // Start Wait for door
    {
        p.Effect2 -= 1;

        if(p.Effect2 == 1900)
        {
            s_TriggerDoorEffects(static_cast<Location_t>(Warp[p.Warp].Exit), false);

            SoundPause[SFX_Door] = 0;

            // SMBX 1.3 logic for P1, see below
            // p.Effect = PLREFF_WAITING;
            // p.Effect2 = 30;

            if(A >= 2 && !g_ClonedPlayerMode)
            {
                p.Effect = PLREFF_WAITING;
                p.Effect2 = 131;
            }
            else
                PlaySoundSpatial(SFX_Door, p.Location);
        }
        // new code to replicate SMBX 1.3 logic setting Effect2 to 30
        else if(p.Effect2 == 1870)
        {
            // Trigger an exit event at door warp that was used to enter the level
            if(Warp[p.Warp].eventExit != EVENT_NONE)
                TriggerEvent(Warp[p.Warp].eventExit, A);

            p.Effect = PLREFF_NORMAL;
            p.Effect2 = 0;
        }
    }
    else if(p.Effect2 <= 3000) // exit warp wait
    {
        p.Effect2 -= 1;

        auto &w = Warp[p.Warp];

        if(g_config.EnableInterLevelFade && (w.MapWarp || w.level != STRINGINDEX_NONE) && (p.Effect2 == 2955) && !g_levelScreenFader.isFadingIn())
            g_levelScreenFader.setupFader(2, 0, 65, ScreenFader::S_FADE);

        if(p.Effect2 == 2920)
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

static inline bool checkWarp(Warp_t &warp, int B, Player_t &plr, int A, bool backward)
{
    bool onGround = !warp.stoodRequired || (plr.Pinched.Bottom1 == 2 || plr.Slope != 0 || plr.StandingOnNPC != 0);

    const auto &entrance      = backward ? warp.Exit        : warp.Entrance;
    const auto &exit          = backward ? warp.Entrance    : warp.Exit;
    const auto &direction     = backward ? warp.Direction2  : warp.Direction;

    if(!CheckCollision(plr.Location, entrance))
        return false; // continue

    plr.ShowWarp = B;

    if(warp.LevelEnt)
        return false;

    bool canWarp = false;

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

    if(!canWarp)
        return false;

    if(warp.Stars > numStars)
    {
        bool do_swap_frame = (g_config.fix_visual_bugs && warp.Effect == 1 && direction == 3 && plr.Duck && plr.SwordPoke == 0);
        int old_plr_frame = plr.Frame;

        if(do_swap_frame)
        {
            // Show the duck frame only when attempting to go down
            plr.Frame = (plr.Character == 5) ? 5 : 7;
        }

        if(warp.StarsMsg == STRINGINDEX_NONE)
            MessageText = fmt::format_ne(g_gameStrings.warpNeedStarCount, warp.Stars, LanguageFormatNumber(warp.Stars, g_gameInfo.wordStarAccusativeSingular, g_gameInfo.wordStarAccusativeDual_Cnt, g_gameInfo.wordStarAccusativePlural), g_gameInfo.wordStarAccusativeDual_Cnt);
        else
            MessageText = GetS(warp.StarsMsg);

        // store all warp-local vars
        PauseInit(PauseCode::Message, A);
        g_gameLoopInterrupt.site = GameLoopInterrupt::UpdatePlayer_SuperWarp;
        g_gameLoopInterrupt.A = A;
        g_gameLoopInterrupt.B = B;
        g_gameLoopInterrupt.C = old_plr_frame;
        g_gameLoopInterrupt.bool4 = do_swap_frame;

        // return true to break out of the loop
        return true;
    }

    plr.Slide = false;

    if(warp.Effect != 3)
        plr.Stoned = false;

    if(warp.Locked)
    {
        // if player has a key, consume it and allow unlocking the warp
        if(plr.HoldingNPC > 0 && NPC[plr.HoldingNPC].Type == NPCID_KEY)
        {
            NPC[plr.HoldingNPC].Killed = 9;
            NPCQueues::Killed.push_back(plr.HoldingNPC);

            NewEffect(EFFID_SMOKE_S3, NPC[plr.HoldingNPC].Location);
        }
        else if(plr.Mount == 3 && plr.YoshiNPC > 0 && NPC[plr.YoshiNPC].Type == NPCID_KEY)
        {
            NPC[plr.YoshiNPC].Killed = 9;
            NPCQueues::Killed.push_back(plr.YoshiNPC);
            plr.YoshiNPC = 0;
        }
        else if(plr.HasKey)
        {
            plr.HasKey = false;
        }
        // otherwise, don't allow unlocking
        else
            return false;

        // if player can still warp, unlock the warp
        warp.Locked = false;

        // remove warp lock icon
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

    // execute the warp!
    UnDuck(Player[A]);
    plr.YoshiTongueLength = 0;
    plr.MountSpecial = 0;
    plr.FrameCount = 0;
    plr.TailCount = 0;
    plr.CanFly = false;
    plr.CanFly2 = false;
    plr.RunCount = 0;

    s_WarpReleaseItems(warp, A, backward);

    if(warp.Effect != 3) // Don't zero speed when passing a portal warp
    {
        plr.Location.SpeedX = 0;
        plr.Location.SpeedY = 0;
    }

    if(warp.eventEnter != EVENT_NONE)
        TriggerEvent(warp.eventEnter, A);

    if(warp.Effect == 0 || warp.Effect == 3) // Instant / Portal
    {
        if(warp.Effect == 3 && (warp.level != STRINGINDEX_NONE || warp.MapWarp))
        {
            plr.Warp = B;
            plr.WarpBackward = backward;
            s_CheckWarpLevelExit(plr, warp, 2921, 2921);

            return true;
        }

        plr.Location.X = exit.X + (exit.Width - plr.Location.Width) / 2;
        plr.Location.Y = exit.Y + exit.Height - plr.Location.Height - 0.1_n;
        CheckSection(A);
        plr.WarpCD = (warp.Effect == 3) ? 10 : 50;

        const Screen_t& screen = ScreenByPlayer(A);
        int vscreen_A = vScreenIdxByPlayer(A);
        bool is_shared_screen = (screen.Type == 3);
        bool do_tele = is_shared_screen && !vScreenCollision(vscreen_A, exit);

        // teleport other players using the instant/portal warp in shared screen mode
        if(do_tele)
        {
            for(int plr_i = 0; plr_i < screen.player_count; plr_i++)
            {
                int o_A = screen.players[plr_i];
                if(o_A == A)
                    continue;

                Player_t& o_p = Player[o_A];

                // revive someone in wings and bring to warp
                if(o_p.Effect == PLREFF_COOP_WINGS)
                    o_p.Dead = false;

                // in the mouth of an onscreen player's Pet?
                bool in_onscreen_pet = !warp.NoYoshi && InOnscreenPet(o_A, screen);

                if(!o_p.Dead && o_p.TimeToLive == 0 && !in_onscreen_pet)
                {
                    RemoveFromPet(o_A);

                    o_p.Location.X = exit.X + (exit.Width - o_p.Location.Width) / 2;
                    o_p.Location.Y = exit.Y + exit.Height - o_p.Location.Height - 0.1_n;
                    CheckSection(o_A);

                    if(warp.Effect != 3) // Don't zero speed when passing a portal warp
                    {
                        o_p.Location.SpeedX = 0;
                        o_p.Location.SpeedY = 0;
                    }

                    o_p.Vine = plr.Vine;
                    o_p.WarpCD = (warp.Effect == 3) ? 10 : 50;

                    // put other player in no-collide mode
                    o_p.Effect = PLREFF_NO_COLLIDE;
                    o_p.Effect2 = A;
                }
            }

            // update position of any players in pets
            s_FixPlayersInPets(screen);

            GetvScreenAuto(vScreen[vscreen_A]);
        }

        if(warp.eventExit != EVENT_NONE)
            TriggerEvent(warp.eventExit, A);

        return true; // break
    }
    else if(warp.Effect == 1) // Pipe
    {
        PlaySoundSpatial(SFX_Warp, plr.Location);
        plr.Effect = PLREFF_WARP_PIPE;
        if(g_config.fix_fairy_stuck_in_pipe)
            plr.Effect2 = 0;
        plr.Warp = B;
        plr.WarpBackward = backward;
//                        if(nPlay.Online && A == nPlay.MySlot + 1)
//                            Netplay::sendData Netplay::PutPlayerLoc(nPlay.MySlot) + "1j" + std::to_string(A) + "|" + plr.Warp + LB;
    }
    else if(warp.Effect == 2) // Door
    {
        PlaySoundSpatial(SFX_Door, plr.Location);
        plr.Effect = PLREFF_WARP_DOOR;

        if(g_config.fix_fairy_stuck_in_pipe)
            plr.Effect2 = 0;

        plr.Warp = B;
        plr.WarpBackward = backward;
//                        if(nPlay.Online && A == nPlay.MySlot + 1)
//                            Netplay::sendData Netplay::PutPlayerLoc(nPlay.MySlot) + "1j" + std::to_string(A) + "|" + plr.Warp + LB;
        plr.Location.X = entrance.X + (entrance.Width - plr.Location.Width) / 2;
        plr.Location.Y = entrance.Y + entrance.Height - plr.Location.Height;

        bool same_section = SectionCollision(plr.Section, static_cast<Location_t>(exit));
        bool do_scroll = (warp.transitEffect == LevelDoor::TRANSIT_SCROLL) && same_section;

        s_TriggerDoorEffects(static_cast<Location_t>(entrance));

        if(!do_scroll)
            s_TriggerDoorEffects(static_cast<Location_t>(exit));
    }

    return false; // continue
}

void SuperWarp(const int A)
{
    auto &plr = Player[A];
    int B = 1;

    if(g_gameLoopInterrupt.site != GameLoopInterrupt::None)
    {
        // clear message text
        MessageText.clear();

        // Restore previous frame if needed
        if(g_gameLoopInterrupt.bool4)
            plr.Frame = g_gameLoopInterrupt.C;

        // check the warp after the one that triggered the message
        B = g_gameLoopInterrupt.B + 1;

        // reset the g_gameLoopInterrupt state
        g_gameLoopInterrupt.site = GameLoopInterrupt::None;
    }

    if(plr.WarpCD <= 0 && plr.Mount != 2 /* && !plr.GroundPound && !plr.GroundPound2 */)
    {
        for(; B <= numWarps; B++)
        {
            auto &warp = Warp[B];

            if(warp.Hidden)
                continue;

            // In normal mode, ignore pounds only for pipe / door warps. In compat mode, ignore pounds for all warps.
            bool ground_pound = plr.GroundPound || plr.GroundPound2;
            bool skip_pounds = !g_config.fix_pound_skip_warp || warp.Effect == 1 || warp.Effect == 2;
            if(ground_pound && skip_pounds)
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
