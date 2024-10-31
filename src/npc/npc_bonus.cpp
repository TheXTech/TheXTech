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

#include "../globals.h"
#include "../npc.h"
#include "../npc_id.h"
#include "../eff_id.h"
#include "../sound.h"
#include "../collision.h"
#include "../effect.h"
#include "../graphics.h"
#include "../player.h"
#include "../game_main.h"
#include "../core/events.h"
#include "../config.h"
#include "../layers.h"

#include "npc_traits.h"

#include "main/level_medals.h"

#include "npc/npc_queues.h"

#include "../controls.h"

#include <Utils/maths.h>
#include <Logger/logger.h>
#ifdef THEXTECH_INTERPROC_SUPPORTED
#   include <InterProcess/intproc.h>
#endif


static void s_PowerupScore(NPCRef_t n)
{
    if(g_config.custom_powerup_collect_score)
        MoreScore((*n)->Score, n->Location);
    else
        MoreScore(6, n->Location);
}


inline void RumbleForPowerup(int A)
{
    Controls::Rumble(A, 200, 0.25);
}

void DropBonus(int A)
{
    const Player_t& plr = Player[A];

    // does player not have a bonus?
    if(plr.HeldBonus <= 0)
        return;

    // is player a clone?
    if(g_ClonedPlayerMode && A != 1)
    {
        Player[A].HeldBonus = NPCID(0);
        return;
    }

    // is player not meant to have a bonus?
    if(Player[A].Character == 3 || Player[A].Character == 4)
    {
        Player[A].HeldBonus = NPCID(0);
        return;
    }

    // drop the bonus!
    PlaySoundSpatial(SFX_DropItem, plr.Location);
    numNPCs++;
    NPC[numNPCs] = NPC_t();
    NPC[numNPCs].Type = Player[A].HeldBonus;
    NPC[numNPCs].Location.Width = NPC[numNPCs]->TWidth;
    NPC[numNPCs].Location.Height = 32;

    // need to find a position to place the bonus -- look for ths HUD
    const Screen_t& screen = ScreenByPlayer(A);
    bool is_shared = (screen.player_count > 1) && (screen.Type != 6) && (screen.active_end() == screen.active_begin() + 1);

    // HUD offset for player
    int hud_offset = 0;

    if(is_shared && screen.player_count > 2)
    {
        int hud_width = 84 * screen.player_count;
        for(int i = 0; i < screen.player_count; i++)
        {
            if(A == screen.players[i])
            {
                // find center for player item box; this is the center of the i'th portion of the HUD (out of plr_count portions)
                hud_offset = (hud_width * (i * 2 + 1)) / (screen.player_count * 2) - hud_width / 2;
                break;
            }
        }
    }
    else if(is_shared)
    {
        if(A == screen.players[0])
            hud_offset = -40;
        if(A == screen.players[1])
            hud_offset = 40;
    }

    // update the vScreen for the player
    vScreen_t& vscreen = vScreenByPlayer(A);
    GetvScreenAuto(vscreen);

    // also update canonical vScreen if needed
    if(!screen.is_canonical())
        GetvScreenAuto(vScreenByPlayer_canonical(A));

    // find the HUD
    double ScreenTop = -vscreen.Y;

    if(vscreen.Height > 600)
        ScreenTop += vscreen.Height / 2 - 300;

    double CenterX = -vscreen.X + vscreen.Width / 2;

    // place NPC at HUD
    NPC[numNPCs].Location.X = CenterX + hud_offset - NPC[numNPCs].Location.Width / 2.0;
    NPC[numNPCs].Location.Y = ScreenTop + 16 + 12;

    // finish initializing the NPC
    NPC[numNPCs].Location.SpeedX = 0;
    NPC[numNPCs].Location.SpeedY = 0;
    NPC[numNPCs].Effect = NPCEFF_DROP_ITEM;
    NPC[numNPCs].Effect2 = 1;
    NPC[numNPCs].Active = true;
    NPC[numNPCs].TimeLeft = 200;
    syncLayers_NPC(numNPCs);
    CheckSectionNPC(numNPCs);

    // enable modern NPC spawn code
    bool small_screen_cam = (g_config.small_screen_cam && screen.W < screen.canonical_screen().W);
    bool always_shared = (is_shared && screen.Type == ScreenTypes::SharedScreen);
    if(!ForcedControls && (always_shared || small_screen_cam))
    {
        NPC[numNPCs].Special5 = 120;
        NPC[numNPCs].Effect3 = A;
        NPC[numNPCs].Effect2 = 1;
    }

    // erase bonus
    Player[A].HeldBonus = NPCID(0);
}

void CheckAfterStarTake(bool many)
{
    int allBGOs = numBackground + numLocked;
    for(int c = 1; c <= numWarps; c++)
    {
        auto &w = Warp[c];
        if((!many && (w.Stars == numStars)) || (many && (w.Stars <= numStars)))
        {
            for(int d = numBackground + 1; d <= allBGOs; d++)
            {
                auto &b = Background[d];
                if(b.Type == 160 &&
                    (CheckCollision(w.Entrance, b.Location) ||
                     (w.twoWay && CheckCollision(w.Exit, b.Location)))
                )
                {
                    // this makes the background permanently disappear
                    b.Layer = LAYER_NONE;
                    b.Hidden = true;
                    syncLayers_BGO(d);
                }
            }
        }
    }
}

static void s_MovePlayersToExit(int got_exit_A)
{
    const auto& p_A = Player[got_exit_A];

    for(int C = 1; C <= numPlayers; C++)
    {
        if(got_exit_A == C) // And DScreenType <> 5 Then
            continue;

        auto& p_C = Player[C];

        p_C.Section = p_A.Section;
        p_C.Location.Y = p_A.Location.Y + p_A.Location.Height      - p_C.Location.Height;
        p_C.Location.X = p_A.Location.X + p_A.Location.Width / 2.0 - p_C.Location.Width / 2.0;
        p_C.Location.SpeedX = 0;
        p_C.Location.SpeedY = 0;
        p_C.Effect = PLREFF_WAITING;
        p_C.Effect2 = -got_exit_A;
    }
}

void TouchBonus(int A, int B)
{
    // INCORRECT NOTE: the only way to reach this code in SMBX 1.3 when Player[A].Effect is not PLREFF_NORMAL is if NPC[B]->IsACoin is true and NPC[B] is on Char4's boomerang
    // NOTE: this is not true. It can also be reached if two bonuses are touched in the same frame.

    // ban collecting dropped item during first stage of modern item drop process
    if(NPC[B].Effect == NPCEFF_DROP_ITEM && NPC[B].Effect3 != 0)
        return;

    // don't get just-thrown items, except coins
    if(NPC[B].CantHurtPlayer == A && !(NPC[B]->IsACoin && Player[A].HoldingNPC != B && NPC[B].Killed == 0))
        return;

    int sfx_extra_item     = (Player[A].Character == 5) ? SFX_HeroHeart : SFX_GotItem;
    int sfx_grow_item      = (Player[A].Character == 5) ? SFX_HeroItem  : SFX_PlayerGrow;
    int sfx_transform_item = (Player[A].Character == 5) ? SFX_HeroItem  : SFX_Transform;

    // moved up here so that the hearts logic works
    NPCID civilian_type = NPCID(0);

    if(NPCIsToad(NPC[B]))
    {
        civilian_type = NPC[B].Type;
        NPC[B].Type = NPCID_POWER_S3;
    }

    // give hearts to heart chars
    if(Player[A].Character == 3 || Player[A].Character == 4 || Player[A].Character == 5)
    {
        // only the first three types were checked in SMBX 1.3, the others were moved here for simplicity
        if(NPC[B].Type == NPCID_LEAF_POWER || NPC[B].Type == NPCID_STATUE_POWER || NPC[B].Type == NPCID_HEAVY_POWER
            || NPC[B].Type == NPCID_POWER_S3 || NPC[B].Type == NPCID_POWER_S1 || NPC[B].Type == NPCID_POWER_S4 || NPC[B].Type == NPCID_POWER_S2 || NPC[B].Type == NPCID_POWER_S5
            || NPC[B].Type == NPCID_FIRE_POWER_S3 || NPC[B].Type == NPCID_FIRE_POWER_S1 || NPC[B].Type == NPCID_FIRE_POWER_S4
            || NPC[B].Type == NPCID_ICE_POWER_S3 || NPC[B].Type == NPCID_ICE_POWER_S4)
        {
            Player[A].Hearts += 1;

            if(Player[A].Hearts > 3)
                Player[A].Hearts = 3;
        }
    }

    // If .Character = 3 Or .Character = 4 Then  'for chars 3/4 turn leaf and suits into a mushroom
    // If NPC(B).Type = 34 Or NPC(B).Type = 169 Or NPC(B).Type = 170 Then NPC(B).Type = 9
    // End If

    if(NPC[B].Type == NPCID_SWAP_POWER) // ? mushroom
    {
        int touched_power_i = A;
        int target_i = CheckNearestLiving(A);

        // NOTE: this previously used hardcoded 1 and 2; now it targets the living player nearest A, prioritizing local players
        if(g_ClonedPlayerMode)
        {
            touched_power_i = 1;
            target_i = 2;
        }

        Player_t& p_touched = Player[touched_power_i];
        Player_t& p_target = Player[target_i];

        if(target_i != 0
            && !p_touched.Dead && p_touched.TimeToLive == 0 && p_touched.Immune == 0
            && !p_target.Dead && p_target.TimeToLive == 0 && p_target.Immune == 0)
        {
            // tempLocation = p_touched.Location;

            // swap location
            double touched_X = p_touched.Location.X;
            double touched_Y = p_touched.Location.Y;
            p_touched.Location.X = p_target.Location.X + p_target.Location.Width / 2.0  - p_touched.Location.Width / 2.0;
            p_touched.Location.Y = p_target.Location.Y + p_target.Location.Height       - p_touched.Location.Height;
            p_target.Location.X  = touched_X           + p_touched.Location.Width / 2.0 - p_target.Location.Width / 2.0;
            p_target.Location.Y  = touched_Y           + p_touched.Location.Height      - p_target.Location.Height;

            // swap some variables
            std::swap(p_touched.Direction, p_target.Direction);
            std::swap(p_touched.Slope, p_target.Slope);
            std::swap(p_touched.StandingOnNPC, p_target.StandingOnNPC);

            // make players immune
            if(p_touched.Immune < 10)
                p_touched.Immune = 10;
            if(p_target.Immune < 10)
                p_target.Immune = 10;

            // play sound in both locations (so both players will hear if remote)
            int old_pause = SoundPause[SFX_BossBeat];
            PlaySoundSpatial(SFX_BossBeat, p_touched.Location);
            SoundPause[SFX_BossBeat] = old_pause;
            PlaySoundSpatial(SFX_BossBeat, p_target.Location);
        }
    }
    else if(NPC[B].Type == NPCID_FLY_POWER) // Player is a fairy
    {
        // don't kill the powerup if it can't be used!
        if(Player[A].Mount == 2)
            return;

        if(!Player[A].Fairy)
        {
            Player[A].Immune = 30;
            Player[A].Effect = PLREFF_WAITING;
            Player[A].Effect2 = 4;
            Player[A].Fairy = true;
            SizeCheck(Player[A]);
            NewEffect(EFFID_SMOKE_S5, Player[A].Location);
        }

        PlaySoundSpatial(SFX_HeroFairy, NPC[B].Location);
        Player[A].FairyTime = -1;
    }
    else if(NPC[B].Type == NPCID_LIFE_S3 || NPC[B].Type == NPCID_LIFE_S4 || NPC[B].Type == NPCID_LIFE_S1) // player touched a 1up mushroom
    {
        MoreScore(10, NPC[B].Location);
    }
    else if(NPC[B].Type == NPCID_TIMER_S3 && NPC[B].Effect != NPCEFF_DROP_ITEM && (Player[A].Character == 1 || Player[A].Character == 2)) // send the clock to the item container
    {
        Player[A].HeldBonus = NPCID_TIMER_S3;
        PlaySoundSpatial(SFX_GotItem, NPC[B].Location);
    }
    else if(NPC[B].Type == NPCID_TIMER_S2 || NPC[B].Type == NPCID_TIMER_S3) // player touched the clock
    {
        PSwitchStop = Physics.NPCPSwitch;
        FreezeNPCs = true;
        PSwitchPlayer = A;
    }
    else if(NPC[B].Type == NPCID_CHECKPOINT) // player touched the chekpoint
    {
        RumbleForPowerup(A);

        if(Player[A].State == 1)
            Player[A].State = 2;
        if(Player[A].Hearts == 1)
            Player[A].Hearts = 2;
        SizeCheck(Player[A]);

        PlaySoundSpatial(SFX_Checkpoint, NPC[B].Location);

        Checkpoint = FullFileName;
        Checkpoint_t cp;
        cp.id = NPC[B].Special;
        CheckpointsList.push_back(cp);
        g_curLevelMedals.on_checkpoint();
        pLogDebug("Added checkpoint ID %d", cp.id);
    }
    else if(NPC[B].Type == NPCID_3_LIFE) // player touched the 3up moon
    {
        MoreScore(12, NPC[B].Location);
    }
    else if(NPC[B].Type == NPCID_AXE)
    {
        // go straight to code to kill NPC
    }
    else if(NPC[B].Type == NPCID_POISON) // Bonus is a POISON mushroom
    {
        PlayerHurt(A);
    }
    else if(NPC[B].Type == NPCID_POWER_S3 || NPC[B].Type == NPCID_POWER_S1 || NPC[B].Type == NPCID_POWER_S4 || NPC[B].Type == NPCID_POWER_S2 || NPC[B].Type == NPCID_POWER_S5) // Bonus is a mushroom
    {
        if(Player[A].Character == 5 && Player[A].State == 1)
            Player[A].State = 2;

        UpdatePlayerBonus(A, NPC[B].Type);

        if(Player[A].State == 1 && Player[A].Character != 5)
        {
            RumbleForPowerup(A);

            if(Player[A].Duck)
                UnDuck(Player[A]);

            Player[A].StateNPC = NPC[B].Type;
            Player[A].Frame = 1;
            Player[A].Effect = PLREFF_TURN_BIG;

            if(Player[A].Mount > 0)
                UnDuck(Player[A]);

            PlaySoundSpatial(SFX_PlayerGrow, NPC[B].Location);
        }
        else if(NPC[B].Type == NPCID_POWER_S5)
            PlaySoundSpatial(SFX_HeroHeart, NPC[B].Location);
        else
            PlaySoundSpatial(SFX_GotItem, NPC[B].Location);

        if(NPC[B].Effect != NPCEFF_DROP_ITEM)
            s_PowerupScore(B);
    }
    // logic combined across all powerup NPCs
    else if(NPC[B].Type == NPCID_FIRE_POWER_S3 || NPC[B].Type == NPCID_FIRE_POWER_S1 || NPC[B].Type == NPCID_FIRE_POWER_S4
        || NPC[B].Type == NPCID_ICE_POWER_S3 || NPC[B].Type == NPCID_ICE_POWER_S4
        || NPC[B].Type == NPCID_LEAF_POWER || NPC[B].Type == NPCID_STATUE_POWER || NPC[B].Type == NPCID_HEAVY_POWER)
    {
        int target_state = 2;
        PlayerEffect target_effect = PLREFF_TURN_BIG;
        bool reset_effect2 = true;
        int use_sfx = sfx_transform_item;

        if(NPC[B].Type == NPCID_FIRE_POWER_S3 || NPC[B].Type == NPCID_FIRE_POWER_S1 || NPC[B].Type == NPCID_FIRE_POWER_S4)
        {
            target_state = 3;
            target_effect = PLREFF_TURN_FIRE;
            reset_effect2 = false;
            use_sfx = sfx_grow_item;
        }
        else if(NPC[B].Type == NPCID_ICE_POWER_S3 || NPC[B].Type == NPCID_ICE_POWER_S4)
        {
            target_state = 7;
            target_effect = PLREFF_TURN_ICE;
            reset_effect2 = false;
            use_sfx = sfx_grow_item;
        }
        else if(NPC[B].Type == NPCID_LEAF_POWER)
        {
            target_state = 4;
            target_effect = PLREFF_TURN_LEAF;
        }
        else if(NPC[B].Type == NPCID_STATUE_POWER)
        {
            target_state = 5;
            target_effect = PLREFF_TURN_STATUE;
        }
        else if(NPC[B].Type == NPCID_HEAVY_POWER)
        {
            target_state = 6;
            target_effect = PLREFF_TURN_HEAVY;
        }

        UpdatePlayerBonus(A, NPC[B].Type);
        Player[A].StateNPC = NPC[B].Type;

        if(Player[A].State != target_state)
        {
            RumbleForPowerup(A);
            Player[A].Frame = 1;
            Player[A].Effect = target_effect;

            if(reset_effect2)
                Player[A].Effect2 = 0;

            if(Player[A].Mount > 0)
                UnDuck(Player[A]);

            PlaySoundSpatial(use_sfx, NPC[B].Location);
        }
        else
            PlaySoundSpatial(sfx_extra_item, NPC[B].Location);

        if(NPC[B].Effect != NPCEFF_DROP_ITEM)
            s_PowerupScore(B);
    }
#if 0
    else if(NPC[B].Type == NPCID_FIRE_POWER_S3 || NPC[B].Type == NPCID_FIRE_POWER_S1 || NPC[B].Type == NPCID_FIRE_POWER_S4) // Bonus is a fire flower
    {
        UpdatePlayerBonus(A, NPC[B].Type);
        Player[A].StateNPC = NPC[B].Type;

        if(Player[A].State != 3)
        {
            RumbleForPowerup(A);
            Player[A].Frame = 1;
            Player[A].Effect = PLREFF_TURN_FIRE;
            if(Player[A].Mount > 0)
                UnDuck(Player[A]);

            PlaySoundSpatial(sfx_grow_item, NPC[B].Location);
        }
        else
            PlaySoundSpatial(sfx_extra_item, NPC[B].Location);

        if(NPC[B].Effect != NPCEFF_DROP_ITEM)
            s_PowerupScore(B);
    }
    else if(NPC[B].Type == NPCID_ICE_POWER_S3 || NPC[B].Type == NPCID_ICE_POWER_S4) // Bonus is an ice flower
    {
        UpdatePlayerBonus(A, NPC[B].Type);
        Player[A].StateNPC = NPC[B].Type;

        if(Player[A].State != 7)
        {
            RumbleForPowerup(A);
            Player[A].Frame = 1;
            Player[A].Effect = PLREFF_TURN_ICE;
            if(Player[A].Mount > 0)
                UnDuck(Player[A]);

            PlaySoundSpatial(sfx_grow_item, NPC[B].Location);
        }
        else
            PlaySoundSpatial(sfx_extra_item, NPC[B].Location);

        if(NPC[B].Effect != NPCEFF_DROP_ITEM)
            s_PowerupScore(B);
    }
    else if(NPC[B].Type == NPCID_LEAF_POWER) // Bonus is a leaf
    {
        UpdatePlayerBonus(A, NPC[B].Type);
        Player[A].StateNPC = NPC[B].Type;

        if(Player[A].State != 4)
        {
            RumbleForPowerup(A);
            Player[A].Frame = 1;
            Player[A].Effect = PLREFF_TURN_LEAF;
            Player[A].Effect2 = 0;
            if(Player[A].Mount > 0)
                UnDuck(Player[A]);

            PlaySoundSpatial(sfx_transform_item, NPC[B].Location);
        }
        else
            PlaySoundSpatial(sfx_extra_item, NPC[B].Location);

        if(NPC[B].Effect != NPCEFF_DROP_ITEM)
            s_PowerupScore(B);
    }
    else if(NPC[B].Type == NPCID_STATUE_POWER) // Bonus is a Tanooki Suit
    {
        UpdatePlayerBonus(A, NPC[B].Type);
        Player[A].StateNPC = NPC[B].Type;

        if(Player[A].State != 5)
        {
            RumbleForPowerup(A);
            Player[A].Frame = 1;
            Player[A].Effect = PLREFF_TURN_STATUE;
            Player[A].Effect2 = 0;
            if(Player[A].Mount > 0)
                UnDuck(Player[A]);

            PlaySoundSpatial(sfx_transform_item, NPC[B].Location);
        }
        else
            PlaySoundSpatial(sfx_extra_item, NPC[B].Location);

        if(NPC[B].Effect != NPCEFF_DROP_ITEM)
            s_PowerupScore(B);
    }
    else if(NPC[B].Type == NPCID_HEAVY_POWER) // Bonus is a Hammer Suit
    {
        UpdatePlayerBonus(A, NPC[B].Type);
        Player[A].StateNPC = NPC[B].Type;

        if(Player[A].State != 6)
        {
            RumbleForPowerup(A);
            Player[A].Frame = 1;
            Player[A].Effect = PLREFF_TURN_HEAVY;
            Player[A].Effect2 = 0;
            if(Player[A].Mount > 0)
                UnDuck(Player[A]);

            PlaySoundSpatial(sfx_transform_item, NPC[B].Location);
        }
        else
            PlaySoundSpatial(sfx_extra_item, NPC[B].Location);

        if(NPC[B].Effect != NPCEFF_DROP_ITEM)
            s_PowerupScore(B);
    }
#endif
    else if(NPC[B]->IsACoin) // Bonus is a coin
    {
        if(NPC[B].Type == NPCID_RING)
            PlaySoundSpatial(SFX_RingGet, NPC[B].Location);
        else if(NPC[B].Type == NPCID_GEM_1 || NPC[B].Type == NPCID_GEM_5 || NPC[B].Type == NPCID_GEM_20)
            PlaySoundSpatial(SFX_HeroRupee, NPC[B].Location);
        else if(NPC[B].Type != NPCID_MEDAL)
            PlaySoundSpatial(SFX_Coin, NPC[B].Location);

        if(NPC[B].Type == NPCID_GEM_5 || NPC[B].Type == NPCID_COIN_5)
            Coins += 5;
        else if(NPC[B].Type == NPCID_GEM_20)
            Coins += 20;
        else
            Coins += 1;

        if(Coins >= 100)
            Got100Coins();

        if(NPC[B].Type == NPCID_MEDAL)
        {
            PlaySoundSpatial(SFX_MedalGet, NPC[B].Location);
            auto& medal_score = NPCTraits[NPC[B].Type].Score;
            MoreScore(medal_score, NPC[B].Location);
            medal_score += 1;
            if(medal_score > 14)
                medal_score = 14;

            g_curLevelMedals.get(NPC[B].Variant - 1);
        }
        else
            MoreScore(1, NPC[B].Location);

        NewEffect(EFFID_COIN_COLLECT, NPC[B].Location);
    }
    else if(NPCIsAnExit(NPC[B]) && LevelMacro == LEVELMACRO_OFF) // Level exit
    {
        if(NPC[B].Type != NPCID_STAR_COLLECT)
        {
            TurnNPCsIntoCoins();
            FreezeNPCs = false;
            if(g_ClonedPlayerMode)
                Player[1] = Player[A];
        }

        if(NPC[B].Type == NPCID_ITEMGOAL)
        {
            if(NPC[B].Frame == 0)
                MoreScore(10, Player[A].Location);
            if(NPC[B].Frame == 1)
                MoreScore(6, Player[A].Location);
            if(NPC[B].Frame == 2)
                MoreScore(8, Player[A].Location);

            LevelMacro = LEVELMACRO_CARD_ROULETTE_EXIT;

            s_MovePlayersToExit(A);

            StopMusic();
            XEvents::doEvents();
            PlaySound(SFX_CardRouletteClear);
        }
        else if(NPC[B].Type == NPCID_GOALORB_S3)
        {
            LevelMacro = LEVELMACRO_QUESTION_SPHERE_EXIT;

            s_MovePlayersToExit(A);

            StopMusic();
            PlaySound(SFX_DungeonClear);
        }
        else if(NPC[B].Type == NPCID_GOALORB_S2)
        {
            LevelMacro = LEVELMACRO_CRYSTAL_BALL_EXIT;

            s_MovePlayersToExit(A);

            StopMusic();
            PlaySound(SFX_CrystalBallExit);
        }
        else if(NPC[B].Type == NPCID_STAR_EXIT || NPC[B].Type == NPCID_STAR_COLLECT)
        {
            bool star_gotten = false;

            for(const auto& star : Star)
            {
                bool bySection = NPC[B].Variant == 0 && (star.Section == NPC[B].Section || star.Section == -1);
                bool byId = NPC[B].Variant > 0 && -(star.Section + 100) == int(NPC[B].Variant);

                if(star.level == FileNameFull && (bySection || byId))
                    star_gotten = true;
            }

            if(!star_gotten)
            {
                Star_t star;
                star.level = FileNameFull;
                // Positive - section number, Negative - UID of each star per level
                int special = (int)NPC[B].Variant;
                star.Section = special <= 0 ? NPC[B].Section : -special - 100;
                if(special > 0)
                    pLogDebug("Got a star with UID=%d", special);
                Star.push_back(std::move(star));
                numStars = (int)Star.size();
#ifdef THEXTECH_INTERPROC_SUPPORTED
                IntProc::sendStarsNumber(numStars);
#endif
                CheckAfterStarTake(false);
            }

            if(NPC[B].Type == NPCID_STAR_EXIT)
            {
                LevelMacro = LEVELMACRO_STAR_EXIT;

                s_MovePlayersToExit(A);

                StopMusic();
                PlaySound(SFX_GotStar);
            }
            else
                PlaySoundSpatial(SFX_MedalGet, NPC[B].Location);
        }
    }

    if(civilian_type > 0)
        NPC[B].Type = civilian_type;

    NPC[B].Killed = 9;
    NPCQueues::Killed.push_back(B);
}
