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

#include <fmt_format_ne.h>

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

#include "main/translate.h"
#include "main/trees.h"
#include "main/game_strings.h"
#include "main/game_info.h"


void s_TriggerDoorEffects(const Location_t& loc, bool do_big_door = true)
{
    for(Background_t& bgo : treeBackgroundQuery(loc, SORTMODE_ID))
    {
        if(CheckCollision(loc, bgo.Location))
        {
            if(bgo.Type == 88)
                NewEffect(EFFID_DOOR_S2_OPEN, bgo.Location);
            else if(bgo.Type == 87)
                NewEffect(EFFID_DOOR_DOUBLE_S3_OPEN, bgo.Location);
            else if(bgo.Type == 107)
                NewEffect(EFFID_DOOR_SIDE_S3_OPEN, bgo.Location);
            else if(do_big_door && bgo.Type == 141)
            {
                Location_t bLoc = bgo.Location;
                bLoc.X += bLoc.Width / 2.0;
                bLoc.Width = 104;
                bLoc.X += -bLoc.Width / 2.0;
                NewEffect(EFFID_BIG_DOOR_OPEN, bLoc);
            }
        }
    }
}

// copied logic from checkWarp function
// parameter release_at_warp determines whether NPC should be dropped at the warp entrance
//   (set to false in new behavior where players are being teleported to warp)
void s_WarpReleaseItems(const Warp_t& warp, int A, bool backward, bool release_at_warp = true)
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
void s_WarpStealMount(int A)
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

static inline bool checkWarp(Warp_t &warp, int B, Player_t &plr, int A, bool backward)
{
    bool canWarp = false;

    bool onGround = !warp.stoodRequired || (plr.Pinched.Bottom1 == 2 || plr.Slope != 0 || plr.StandingOnNPC != 0);

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
        int prevFrame = plr.Frame;

        if(g_config.fix_visual_bugs && warp.Effect == 1 && direction == 3 && plr.Duck && plr.SwordPoke == 0)
        {
            // Show the duck frame only when attempting to go down
            plr.Frame = (plr.Character == 5) ? 5 : 7;
        }

        if(warp.StarsMsg == STRINGINDEX_NONE)
            MessageText = fmt::format_ne(g_gameStrings.warpNeedStarCount, warp.Stars, LanguageFormatNumber(warp.Stars, g_gameInfo.wordStarAccusativeSingular, g_gameInfo.wordStarAccusativeDual_Cnt, g_gameInfo.wordStarAccusativePlural), g_gameInfo.wordStarAccusativeDual_Cnt);
        else
            MessageText = GetS(warp.StarsMsg);

        PauseGame(PauseCode::Message, A);
        MessageText.clear();
        MessageTextMap.clear();

        // Restore previous frame
        if(g_config.fix_visual_bugs && warp.Effect == 1 && direction == 3)
            plr.Frame = prevFrame;

        canWarp = false;
    }

    if(canWarp)
    {
        plr.Slide = false;

        if(warp.Effect != 3)
            plr.Stoned = false;

        if(warp.Locked)
        {
            if(plr.HoldingNPC > 0 && NPC[plr.HoldingNPC].Type == NPCID_KEY)
            {
                NPC[plr.HoldingNPC].Killed = 9;
                NPCQueues::Killed.push_back(plr.HoldingNPC);

                NewEffect(EFFID_SMOKE_S3, NPC[plr.HoldingNPC].Location);
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
            else if(plr.Mount == 3 && plr.YoshiNPC > 0 && NPC[plr.YoshiNPC].Type == NPCID_KEY)
            {
                NPC[plr.YoshiNPC].Killed = 9;
                NPCQueues::Killed.push_back(plr.YoshiNPC);
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

        s_WarpReleaseItems(warp, A, backward);

        if(warp.Effect != 3) // Don't zero speed when passing a portal warp
        {
            plr.Location.SpeedX = 0;
            plr.Location.SpeedY = 0;
        }

        if(warp.eventEnter != EVENT_NONE)
            ProcEvent(warp.eventEnter, A);

        if(warp.Effect == 0 || warp.Effect == 3) // Instant / Portal
        {
            if(warp.Effect == 3)
            {
                if(warp.level != STRINGINDEX_NONE)
                {
                    GoToLevel = GetS(warp.level);
                    GoToLevelNoGameThing = warp.noEntranceScene;
                    plr.Effect = PLREFF_WAITING;
                    plr.Effect2 = 2921;
                    plr.Warp = B;
                    plr.WarpBackward = backward;
                    ReturnWarp = B;
                    if(IsHubLevel)
                        ReturnWarpSaved = ReturnWarp;
                    StartWarp = warp.LevelWarp;
                    return true;
                }
                else if(warp.MapWarp)
                {
                    plr.Effect = PLREFF_WAITING;
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

                    // in the mouth of an onscreen player's Pet?
                    bool in_onscreen_pet = !warp.NoYoshi && InOnscreenPet(o_A, screen);

                    if(!o_p.Dead && o_p.TimeToLive == 0 && !in_onscreen_pet)
                    {
                        RemoveFromPet(o_A);

                        o_p.Location.X = exit.X + exit.Width / 2.0 - o_p.Location.Width / 2.0;
                        o_p.Location.Y = exit.Y + exit.Height - o_p.Location.Height - 0.1;
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

                GetvScreenAuto(vScreen[vscreen_A]);
            }

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
            plr.Location.X = entrance.X + entrance.Width / 2.0 - plr.Location.Width / 2.0;
            plr.Location.Y = entrance.Y + entrance.Height - plr.Location.Height;

            bool same_section = SectionCollision(plr.Section, static_cast<Location_t>(exit));
            bool do_scroll = (warp.transitEffect == LevelDoor::TRANSIT_SCROLL) && same_section;

            s_TriggerDoorEffects(static_cast<Location_t>(entrance));

            if(!do_scroll)
                s_TriggerDoorEffects(static_cast<Location_t>(exit));
        }
    }

    return false; // continue
}

void SuperWarp(const int A)
{
    auto &plr = Player[A];

    if(plr.WarpCD <= 0 && plr.Mount != 2 /* && !plr.GroundPound && !plr.GroundPound2 */)
    {
        for(int B = 1; B <= numWarps; B++)
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
