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

#include <cmath>
#include <ctime>
#include <Utils/maths.h>
#include <pge_delay.h>
#include <Logger/logger.h>

#include "sdl_proxy/sdl_stdinc.h"

#include "globals.h"
#include "player.h"
#include "player/player_effect.h"
#include "player/player_update_priv.h"
#include "graphics.h"
#include "collision.h"
#include "npc.h"
#include "npc_id.h"
#include "eff_id.h"
#include "npc_traits.h"
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
#include "main/level_medals.h"
#include "core/render.h"
#include "core/events.h"
#include "script/luna/lunacounter.h"

#include "npc/npc_queues.h"

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
    for(int bi : treeBlockQuery(tempLocation, SORTMODE_COMPAT))
    {
        if(CheckCollision(tempLocation, Block[bi].Location))
        {
            if(C == 0)
                C = bi;
            else
            {
                if(Block[bi].Location.Y < Block[C].Location.Y)
                    C = bi;
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

    if(numPlayers > 1 && g_config.multiplayer_pause_controls && !g_ClonedPlayerMode)
    {
        for(B = 1; B <= numPlayers; B++)
            DodgePlayers(B);
    }
    else if(numPlayers > 1)
    {
        Player[1].Location.X -= 16;
        Player[2].Location.X += 16;
    }
}

static void setupCheckpoints()
{
    if(Checkpoint != FullFileName || Checkpoint.empty())
    {
        if(!IsHubLevel && !LevelSelect)
        {
            pLogDebug("Clear check-points at SetupPlayers()");
            Checkpoint.clear();
            CheckpointsList.clear();
            g_curLevelMedals.reset_checkpoint();
        }
        return;
    }

    // restore medals from checkpoint
    g_curLevelMedals.resume_from_checkpoint();

    pLogDebug("Trying to restore %zu checkpoints...", CheckpointsList.size());
    if(!g_config.fix_vanilla_checkpoints && CheckpointsList.empty())
    {
        pLogDebug("Using legacy algorithm");
        CheckpointsList.push_back(Checkpoint_t());
    }
    for(int cpId = 0; cpId < int(CheckpointsList.size()); cpId++)
    {
        auto &cp = CheckpointsList[size_t(cpId)];

        for(int A = 1; A <= numNPCs; A++)
        {
            if(NPC[A].Type != NPCID_CHECKPOINT)
                continue;

            if(g_config.fix_vanilla_checkpoints && cp.id != NPC[A].Special)
                continue;

            NPC[A].Killed = 9;
            NPCQueues::Killed.push_back(A);

            // found a last id, leave player here
            if(!g_config.fix_vanilla_checkpoints || cpId == int(CheckpointsList.size() - 1))
            {
                setupPlayerAtCheckpoints(NPC[A], cp);
                if(g_config.fix_vanilla_checkpoints)
                    break;// Stop to find NPCs
            }
        }// for NPCs

        if(!g_config.fix_vanilla_checkpoints)
            break;
    } // for Check points
}

struct PlayerStartInfo_t
{
    static constexpr int start_pos_count = 2;
    std::array<uint8_t, start_pos_count> players_at_start{};
    std::array<uint8_t, c_screenCount> start_for_screen{}; // used by shared screens only
};

static void s_PlacePlayerAtStart(int A, PlayerStartInfo_t& player_start_info)
{
    /**************************************
    ** (1) pick which start point to use **
    **************************************/
    int use_start = -1;

    // check for shared screen
    const int plr_screen_i = ScreenIdxByPlayer(A);
    const Screen_t& plr_screen = ScreenByPlayer(A);
    if(plr_screen.Type == ScreenTypes::SharedScreen)
        use_start = (int)player_start_info.start_for_screen[plr_screen_i] - 1;

    // if not forced by shared screen, choose start with the least players
    if(use_start == -1)
    {
        // default to initial start
        use_start = 0;

        // check whether a later start has fewer players
        for(int start = 1; start < PlayerStartInfo_t::start_pos_count; start++)
        {
            if(PlayerStart[start + 1].isNull())
                continue;

            if(player_start_info.players_at_start[start] < player_start_info.players_at_start[use_start])
                use_start = start;
        }

        // add either the player alone, or all players on the player's screen, to the count
        if(plr_screen.Type == ScreenTypes::SharedScreen)
        {
            player_start_info.players_at_start[use_start] += plr_screen.player_count;
            player_start_info.start_for_screen[plr_screen_i] = use_start + 1;
        }
        else
            player_start_info.players_at_start[use_start] += 1;
    }


    /*******************************
    ** (2) basic player placement **
    *******************************/

    // place at start
    const auto& ps = PlayerStart[use_start + 1];
    auto& pLoc = Player[A].Location;

    double ps_X = ps.X + ps.Width * 0.5 - pLoc.Width * 0.5;
    double ps_Y = ps.Y + ps.Height - pLoc.Height;

    pLoc.X = ps_X;
    pLoc.Y = ps_Y;
    Player[A].Direction = ps.Direction;


    /**********************************
    ** (3) logic to avoid collisions **
    **********************************/

    // ignore collisions in clone mode
    if(g_ClonedPlayerMode)
        return;

    DodgePlayers(A);
}

void DodgePlayers(int plr_A)
{
    auto& pLoc = Player[plr_A].Location;
    const Screen_t& plr_screen = ScreenByPlayer(plr_A);

    // save current position
    double orig_X = pLoc.X;
    double orig_Y = pLoc.Y;


    // check section of current position for later use
    int cur_section = -1;
    for(int B = 0; B < numSections; B++)
    {
        if(pLoc.X + pLoc.Width >= level[B].X
            && pLoc.X <= level[B].Width
            && pLoc.Y + pLoc.Height >= level[B].Y
            && pLoc.Y <= level[B].Height)
        {
            cur_section = B;
        }
    }


    // check for floor of current position for later use
    bool orig_has_floor = false;
    const Location_t orig_floor_check = newLoc(orig_X + pLoc.Width / 2, pLoc.Y + pLoc.Height, 1, 48);

    for(BlockRef_t b_ref : treeBlockQuery(orig_floor_check, SORTMODE_NONE))
    {
        const Block_t& b = b_ref;
        int B = (int)b_ref;

        if(b.Hidden || b.Invis || BlockNoClipping[b.Type])
            continue;

        if(BlockCheckPlayerFilter(B, plr_A))
            continue;

        if(CheckCollision(orig_floor_check, b.Location))
        {
            orig_has_floor = true;
            break;
        }
    }


    // first try to place players backwards from current position, then do forwards if that doesn't work
    bool forwards_direction = false;

    // this loop repeats each time the player is placed to avoid collisions
    while(true)
    {
        // (a) check for player collision
        bool hit = false;

        for(int B = 1; B < plr_A; B++)
        {
            if(CheckCollision(pLoc, Player[B].Location))
            {
                hit = true;
                break;
            }
        }

        if(!hit)
            break;


        // (b) prepare to restore old position on failure
        bool failed = false;

        double old_X = pLoc.X;
        double old_Y = pLoc.Y;


        // (c) X logic: move player backwards, and check it hasn't moved off section / off screen
        constexpr int plr_spacing = 40;
        const int X_move = (plr_spacing - plr_spacing * 2 * forwards_direction) * Player[plr_A].Direction;
        pLoc.X -= X_move;

        // check for failures of being outside of section X bounds (partially offscreen)
        if(!failed && cur_section != -1 && (pLoc.X < level[cur_section].X || pLoc.X + pLoc.Width > level[cur_section].Width))
            failed = true;

        // also check being too far from start point (Shared Screen mode)
        if(!failed && plr_screen.Type == ScreenTypes::SharedScreen && std::abs(pLoc.X - orig_X) > plr_screen.W * 0.75)
            failed = true;


        // (d) Y logic: do floor checks, check player hasn't moved off screen, and confirm the player didn't cross a ceiling
        constexpr int max_height_add = 160;
        double top_bound = pLoc.Y - max_height_add;
        bool check_floor = true;

        // perform floor check (move player upwards until they are above blocks)
        while(!failed && check_floor)
        {
            check_floor = false;

            // check the whole range from player's old position to player's new position (prevents crossing walls)
            double left_X = pLoc.X;
            double right_X = pLoc.X + pLoc.Width;
            if(X_move < 0)
                right_X -= X_move;
            else
                left_X -= X_move;

            const Location_t floor_check_range = newLoc(left_X, pLoc.Y + pLoc.Height, right_X - left_X, pLoc.Height);

            for(BlockRef_t b_ref : treeBlockQuery(floor_check_range, SORTMODE_NONE))
            {
                const Block_t& b = b_ref;
                int B = (int)b_ref;

                if(b.Hidden || b.Invis || BlockIsSizable[b.Type] || BlockOnlyHitspot1[b.Type] || BlockNoClipping[b.Type])
                    continue;

                if(BlockCheckPlayerFilter(B, plr_A))
                    continue;

                if(CheckCollision(floor_check_range, b.Location))
                {
                    double new_Y = blockGetTopYTouching(b, pLoc);

                    if(new_Y - pLoc.Height < pLoc.Y)
                    {
                        check_floor = true;
                        pLoc.Y = new_Y - pLoc.Height;

                        break;
                    }
                }
            }

            // check we didn't go too high
            if(pLoc.Y < top_bound)
                failed = true;
        }

        // perform cliff check (move player downwards until they are on blocks) if the original position had a floor
        if(!failed && orig_has_floor && pLoc.Y >= old_Y)
        {
            bool found_floor = false;
            double top_Y = 0.0;

            const Location_t new_floor_check = newLoc(pLoc.X, pLoc.Y + pLoc.Height, pLoc.Width, max_height_add);

            for(BlockRef_t b_ref : treeBlockQuery(new_floor_check, SORTMODE_NONE))
            {
                const Block_t& b = b_ref;
                int B = (int)b_ref;

                if(b.Hidden || b.Invis || BlockNoClipping[b.Type])
                    continue;

                if(BlockCheckPlayerFilter(B, plr_A))
                    continue;

                if((BlockIsSizable[b.Type] || BlockOnlyHitspot1[b.Type]) && b.Location.Y < new_floor_check.Y)
                    continue;

                if(CheckCollision(new_floor_check, b.Location))
                {
                    double new_Y = blockGetTopYTouching(b, new_floor_check);

                    if(!found_floor || new_Y < top_Y)
                    {
                        found_floor = true;
                        top_Y = new_Y;
                    }
                }
            }

            if(found_floor)
                pLoc.Y = top_Y - pLoc.Height;
            else
                failed = true;
        }

        // check for failures of being outside of section Y bounds (totally offscreen)
        if(!failed && cur_section != -1 && (pLoc.Y + pLoc.Height < level[cur_section].Y || pLoc.Y > level[cur_section].Height))
            failed = true;

        // check being too far from original position (Shared Screen mode)
        if(!failed && plr_screen.Type == ScreenTypes::SharedScreen && std::abs(pLoc.Y - orig_Y) > plr_screen.H * 0.75)
            failed = true;

        // check we didn't cross a ceiling block (in the previous column)
        if(!failed && pLoc.Y < old_Y)
        {
            const Location_t ceiling_check = newLoc(old_X + pLoc.Width / 2, pLoc.Y, 1, old_Y - pLoc.Y);

            for(BlockRef_t b_ref : treeBlockQuery(pLoc, SORTMODE_NONE))
            {
                const Block_t& b = b_ref;
                int B = (int)b_ref;

                if(b.Hidden || b.Invis || BlockIsSizable[b.Type] || BlockOnlyHitspot1[b.Type] || BlockNoClipping[b.Type])
                    continue;

                if(BlockCheckPlayerFilter(B, plr_A))
                    continue;

                if(CheckCollision(ceiling_check, b.Location))
                {
                    failed = true;
                    break;
                }
            }
        }

        // perform lava check
        if(!failed)
        {
            const Location_t lava_check = newLoc(pLoc.X, pLoc.Y + pLoc.Height, pLoc.Width, 31);

            for(BlockRef_t b_ref : treeBlockQuery(lava_check, SORTMODE_NONE))
            {
                const Block_t& b = b_ref;
                int B = (int)b_ref;

                if(b.Hidden || b.Invis || BlockNoClipping[b.Type])
                    continue;

                if(BlockCheckPlayerFilter(B, plr_A))
                    continue;

                if(CheckCollision(lava_check, b.Location))
                {
                    if(BlockHurts[b.Type] || BlockKills[b.Type])
                    {
                        failed = true;
                        break;
                    }
                }
            }
        }


        // (d) on failure, first restart and try forwards direction
        if(failed && !forwards_direction)
        {
            pLoc.X = orig_X;
            pLoc.Y = orig_Y;
            forwards_direction = true;
        }
        // otherwise, restore old position and disable player collisions
        else if(failed)
        {
            pLoc.X = old_X;
            pLoc.Y = old_Y;

            Player[plr_A].Effect = PLREFF_RESPAWN;
            Player[plr_A].Effect2 = pLoc.Y;
            break;
        }
    }
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
    qScreen_canonical = false;
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
        g_curLevelMedals.reset_checkpoint();
    }
    else
    {
        BattleIntro = 0;
        BattleOutro = 0;
    }


    // new-added struct to handle start points in >2P
    PlayerStartInfo_t player_start_info; ////// Some sort of corruption here?? Seems unable to select the start correctly. Fix before pushing.

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
                Player[A].HeldBonus = NPCID(0);
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
                Player[A].HeldBonus = NPCID_POWER_S3;
            Player[A].Hearts = 0;
        }
        if(Player[A].Character == 5)
            Player[A].Mount = 0;

        Player[A].Direction = 1; // Moved from below to here
        Player[A].Location.Width = Physics.PlayerWidth[Player[A].Character][Player[A].State]; // set height
        Player[A].Location.Height = Physics.PlayerHeight[Player[A].Character][Player[A].State]; // set width
        if(Player[A].State == 1 && Player[A].Mount == 1) // if small and in a shoe then set the height to super mario
            Player[A].Location.Height = Physics.PlayerHeight[1][2];

        // moved from below to here
        Player[A].Effect = PLREFF_NORMAL;
        Player[A].Effect2 = 0;

        // modern multiplayer placement code
        if(g_config.multiplayer_pause_controls)
        {
            s_PlacePlayerAtStart(A, player_start_info);
        }
        // legacy multiplayer placement code
        else
        {
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
        Player[A].AltRunRelease = false;
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

        // Player[A].NPCPinched = 0;
        // Player[A].Pinched1 = 0;
        // Player[A].Pinched2 = 0;
        // Player[A].Pinched3 = 0;
        // Player[A].Pinched4 = 0;
        Player[A].Pinched = PinchedInfo_t();

        Player[A].StandingOnNPC = 0;
        Player[A].StandingOnVehiclePlr = 0;
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
        // Player[A].Effect = PLREFF_NORMAL; // moved above, possibly set in start-pos code
        // Player[A].Effect2 = 0;
        Player[A].Immune = 0;
        Player[A].Immune2 = false;

        // new code to prevent char5 from attacking at spawn (thanks to Sapphire Bullet Bill for the suggestion)
        if(!LevelEditor && BattleMode && numPlayers > 2)
            Player[A].Immune = 90;

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

        // legacy code for >2P, unused in modern gameplay
        if(numPlayers > 2 && !GameMenu) // find correct positions without start locations
        {
            if(GameOutro)
            {
                Player[A].Location = Player[1].Location;
                Player[A].Location.X += A * 52 - 52;
            }
            // many-player code
            else if(g_ClonedPlayerMode)
            {
                Player[A].Location = Player[1].Location;
                Player[A].Location.SpeedY = dRand() * -12 - 6;
            }
        }

        // section check
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
    if(!LevelSelect)
        SetupScreens(); // setup the screen depending on how many players there are
    setupCheckpoints(); // setup the checkpoint and restpore the player at it if needed

    // prepare vScreens for SharedScreen since UpdatePlayer happens before UpdateGraphics
    for(int screen_i = 0; screen_i < c_screenCount; screen_i++)
    {
        Screen_t& screen = Screens[screen_i];
        if(screen.Type == ScreenTypes::SharedScreen)
        {
            // CenterScreens(screen);
            GetvScreenAuto(screen.vScreen(1));
        }
    }
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
        PlaySoundSpatial(SFX_HeroFairy, p.Location);
        p.Immune = 30;
        p.Effect = PLREFF_WAITING;
        p.Effect2 = 4;
        p.Fairy = false;
        p.FairyTime = 0;

        // FIXME: Here is a possible vanilla bug: B is always 0 even at original code
        SizeCheck(Player[B]);

        NewEffect(EFFID_SMOKE_S5, p.Location);
        if(p.Character == 5)
        {
            p.FrameCount = -10;
            p.Location.SpeedX = 3 * -p.Direction;
            p.Location.SpeedY = -7.01;
            p.StandingOnNPC = 0;
            p.FireBallCD = 20;
            PlaySoundSpatial(SFX_HeroHurt, p.Location);
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

    if(NPC[p.HoldingNPC].Type == NPCID_PLR_FIREBALL)
        p.HoldingNPC = 0;

    if(LevelMacro == LEVELMACRO_OFF)
    {
        if(p.Immune == 0)
        {
            Controls::Rumble(A, 250, 0.5);

            if(p.Mount == 1)
            {
                p.Mount = 0;
                PlaySoundSpatial(SFX_Boot, p.Location);
                UnDuck(Player[A]);
                tempLocation = p.Location;
                tempLocation.SpeedX = 5 * -p.Direction;
                if(p.MountType == 1)
                    NewEffect(EFFID_GRN_BOOT_DIE, tempLocation);
                else if(p.MountType == 2)
                    NewEffect(EFFID_RED_BOOT_DIE, tempLocation);
                else
                    NewEffect(EFFID_BLU_BOOT_DIE, tempLocation);
                p.Location.Y += p.Location.Height;
                p.Location.Height = Physics.PlayerHeight[p.Character][p.State];
                p.Location.Y += -p.Location.Height;
                p.Immune = 150;
                p.Immune2 = true;
            }
            else if(p.Mount == 3)
            {
                UnDuck(Player[A]);
                PlaySoundSpatial(SFX_PetHurt, p.Location);
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
                    NPC[numNPCs].Type = NPCID_PET_GREEN;
                else if(p.MountType == 2)
                    NPC[numNPCs].Type = NPCID_PET_BLUE;
                else if(p.MountType == 3)
                    NPC[numNPCs].Type = NPCID_PET_YELLOW;
                else if(p.MountType == 4)
                    NPC[numNPCs].Type = NPCID_PET_RED;
                else if(p.MountType == 5)
                    NPC[numNPCs].Type = NPCID_PET_BLACK;
                else if(p.MountType == 6)
                    NPC[numNPCs].Type = NPCID_PET_PURPLE;
                else if(p.MountType == 7)
                    NPC[numNPCs].Type = NPCID_PET_PINK;
                else if(p.MountType == 8)
                    NPC[numNPCs].Type = NPCID_PET_CYAN;
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
                // TODO: State-dependent moment
                if(p.Character == 3 || p.Character == 4)
                {
                    if(p.Hearts == 3 && (p.State == 2 || p.State == 4 || p.State == 5 || p.State == 6))
                    {
                        p.State = 2;
                        p.Immune = 150;
                        p.Immune2 = true;
                        p.Hearts -= 1;
                        PlaySoundSpatial(SFX_PlayerHit, p.Location);
                        return;
                    }
                    else
                    {
                        p.Hearts -= 1;
                        if(p.Hearts == 0)
                            p.State = 1;
                        else if(p.State == 3 && p.Hearts == 2)
                        {
                            p.Effect = PLREFF_FIRE_TO_BIG;
                            p.Effect2 = 0;
                            PlaySoundSpatial(SFX_PlayerShrink, p.Location);
                            return;
                        }
                        else if(p.State == 7 && p.Hearts == 2)
                        {
                            p.Effect = PLREFF_ICE_TO_BIG;
                            p.Effect2 = 0;
                            PlaySoundSpatial(SFX_PlayerShrink, p.Location);
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
                        PlaySoundSpatial(SFX_HeroHurt, p.Location);
                        return;
                    }
                }

                if(p.State > 1)
                {
                    PlaySoundSpatial(SFX_PlayerShrink, p.Location);
                    p.StateNPC = NPCID_NULL;
                    p.Effect = PLREFF_TURN_SMALL;
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
                        NPC[numNPCs].Type = NPCID_VEHICLE;
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

                        for(int B : NPCQueues::Active.no_change)
                        {
                            if(NPC[B].vehiclePlr == A)
                            {
                                NPC[B].vehiclePlr = 0;
                                NPC[B].Location.SpeedY = 0;
                                NPC[B].Location.Y = NPC[numNPCs].Location.Y - 0.1 - NPC[B].vehicleYOffset;
                                treeNPCUpdate(B);
                                if(NPC[B].tempBlock > 0)
                                    treeNPCSplitTempBlock(B);

                                NPC[B].vehicleYOffset = 0;
                                if(NPC[B].Type == NPCID_CANNONITEM)
                                    NPC[B].Special = 0;
                                if(NPC[B].Type == NPCID_TOOTHY)
                                {
                                    NPC[B].Killed = 9;
                                    NPCQueues::Killed.push_back(B);
                                    NPC[B].Special = 0;
                                }
                                else if(NPC[B].Type == NPCID_TOOTHYPIPE)
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
        NewEffect(EFFID_CHAR1_DIE, Player[A].Location, 1, 0, ShadowMode);
    else if(Player[A].Character == 2)
        NewEffect(EFFID_CHAR2_DIE, Player[A].Location, 1, 0, ShadowMode);
    else if(Player[A].Character == 3)
        NewEffect(EFFID_CHAR3_DIE, Player[A].Location, 1, 0, ShadowMode);
    else if(Player[A].Character == 4)
        NewEffect(EFFID_CHAR4_DIE, Player[A].Location, 1, 0, ShadowMode);
    else if(Player[A].Character == 5)
    {
        NewEffect(EFFID_CHAR5_DIE, Player[A].Location, static_cast<float>(Player[A].Direction), 0, ShadowMode);
        Effect[numEffects].Location.SpeedX = 2 * -Player[A].Direction;
    }
}

void PlayerDead(int A)
{
    Controls::Rumble(A, 400, 0.8f);

    g_curLevelMedals.on_any_death();

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
        PlaySoundSpatial(SFX_HeroDied, p.Location);
    else
    {
        if(BattleMode)
            PlaySoundSpatial(SFX_PlayerDied2, p.Location);
        else if(g_ClonedPlayerMode)
        {
            for(B = 1; B <= numPlayers; B++)
            {
                if(!Player[B].Dead && Player[B].TimeToLive == 0 && A != B)
                    tempBool = true;
            }
            if(tempBool)
                PlaySoundSpatial(SFX_PlayerDied2, p.Location);
            else
                PlaySoundSpatial(SFX_PlayerDied, p.Location);
        }
        else
            PlaySoundSpatial(SFX_PlayerDied, p.Location);
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
    p.Effect = PLREFF_NORMAL;
    p.Effect2 = 0;
    p.Fairy = false;

    if(p.Mount == 2)
    {
        numNPCs++;
        NPC[numNPCs] = NPC_t();
        NPC[numNPCs].Direction = p.Direction;
        if(NPC[numNPCs].Direction == 1)
            NPC[numNPCs].Frame = 4;
        NPC[numNPCs].Frame += SpecialFrame[2];
        NPC[numNPCs].Active = true;
        NPC[numNPCs].TimeLeft = 100;
        NPC[numNPCs].Type = NPCID_VEHICLE;
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
        g_curLevelMedals.on_all_dead();
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

    // p.Pinched1 = 0;
    // p.Pinched2 = 0;
    // p.Pinched3 = 0;
    // p.Pinched4 = 0;
    // p.NPCPinched = 0;
    p.Pinched = PinchedInfo_t();

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
        if(NPC[p.HoldingNPC].Type == NPCID_VINE_BUG)
            NPC[p.HoldingNPC].Projectile = true;
    }

    p.HoldingNPC = 0;
    if(BattleMode)
    {
        if(BattleLives[A] <= 0)
        {
            int other_alive = 0;
            bool two_others_alive = false;
            for(int o_A = 1; o_A <= numPlayers; o_A++)
            {
                if(o_A == A)
                    continue;

                if(!Player[o_A].Dead || BattleLives[o_A] > 0)
                {
                    if(other_alive != 0)
                    {
                        two_others_alive = true;
                        break;
                    }
                    else
                        other_alive = o_A;
                }
            }

            if(!two_others_alive && BattleOutro == 0)
            {
                BattleOutro = 1;
                PlaySound(SFX_GotStar);
                StopMusic();
            }

            if(!two_others_alive && BattleWinner == 0)
                BattleWinner = other_alive;
        }

        if(A == BattleWinner || (BattleWinner == 0 && BattleLives[A] > 0))
        {
            if(BattleLives[A] > 0)
                BattleLives[A] -= 1;

            PlaySoundSpatial(SFX_Transform, p.Location);
            p.Frame = 1;
            p.Location.SpeedX = 0;
            p.Location.SpeedY = 0;
            p.Mount = 0;
            p.State = 2;
            p.Hearts = 2;
            p.Effect = PLREFF_NORMAL;

            p.Location.Width = Physics.PlayerWidth[p.Character][p.State];
            p.Location.Height = Physics.PlayerHeight[p.Character][p.State];

            // eventually, check for valid starts
            constexpr int valid_start_count = 2;
            int use_start = (A - 1) % valid_start_count + 1;

            p.Location.X = PlayerStart[use_start].X + PlayerStart[use_start].Width * 0.5 - p.Location.Width * 0.5;
            p.Location.Y = PlayerStart[use_start].Y + PlayerStart[use_start].Height - p.Location.Height;
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
            NewEffect(EFFID_SMOKE_S4, tempLocation);
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

int CheckNearestLiving(const int A)
{
    const Player_t& p = Player[A];
    const Screen_t& screen = ScreenByPlayer(A);

    int    closest      = 0;
    double closest_dist = 0;

    // first, look on same screen
    for(int plr_i = 0; plr_i < screen.player_count; plr_i++)
    {
        int o_A = screen.players[plr_i];
        const Player_t& o_p = Player[o_A];

        if(o_A != A && !o_p.Dead && o_p.TimeToLive == 0 && o_A != p.YoshiPlayer)
        {
            double dist = (o_p.Location.X - p.Location.X) * (o_p.Location.X - p.Location.X) + (o_p.Location.Y - p.Location.Y) * (o_p.Location.Y - p.Location.Y);

            if(closest == 0 || closest_dist > dist)
            {
                closest = o_A;
                closest_dist = dist;
            }
        }
    }

    if(closest)
        return closest;

    // next, look at all players
    for(int o_A = 1; o_A <= numPlayers; o_A++)
    {
        const Player_t& o_p = Player[o_A];

        if(o_A != A && !o_p.Dead && o_p.TimeToLive == 0 && o_A != p.YoshiPlayer)
        {
            double dist = (o_p.Location.X - p.Location.X) * (o_p.Location.X - p.Location.X) + (o_p.Location.Y - p.Location.Y) * (o_p.Location.Y - p.Location.Y);

            if(closest == 0 || closest_dist > dist)
            {
                closest = o_A;
                closest_dist = dist;
            }
        }
    }

    return closest;
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
    }
}

static void s_gameOver()
{
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
    MenuMode = MENU_INTRO;
    MenuCursor = 0;
#ifdef ENABLE_ANTICHEAT_TRAP
    CheaterMustDie = false;
#endif
}

void EveryonesDead()
{
//    int A = 0; // UNUSED
    if(BattleMode)
        return;

    StopMusic();
    LevelMacro = LEVELMACRO_OFF;
    FreezeNPCs = false;

    // Quit to world map if died on sub-hub
    if(!NoMap && IsHubLevel && !FileRecentSubHubLevel.empty())
    {
        FileRecentSubHubLevel.clear();
        ReturnWarp = 0;
        ReturnWarpSaved = 0;
    }

// Play fade effect instead of wait (see ProcessLastDead() above)
    if(!g_config.EnableInterLevelFade)
    {
        XRender::setTargetTexture();
        XRender::clearBuffer();
        XRender::repaint();
//    if(MagicHand)
//        BitBlt frmLevelWindow::vScreen[1].hdc, 0, 0, frmLevelWindow::vScreen[1].ScaleWidth, frmLevelWindow::vScreen[1].ScaleHeight, 0, 0, 0, vbWhiteness;
        if(!g_config.unlimited_framerate)
            PGE_Delay(500);
    }

#ifdef ENABLE_ANTICHEAT_TRAP
    if(CheaterMustDie)
    {
        s_gameOver();
        return;
    }
#endif

    if(g_ClonedPlayerMode)
        gDeathCounter.MarkDeath();

    if(g_config.modern_lives_system)
    {
        g_100s--;

        if(g_100s < 0)
            Score = 0;
    }
    else
        Lives--;

    if(g_config.modern_lives_system || Lives >= 0.f)
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
        s_gameOver();
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

    for(B = 0; B < numSections; B++)
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
        for(B = 0; B < numSections; B++)
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
    if(p.Effect != PLREFF_NO_COLLIDE)
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
                    NewEffect(EFFID_SKID_DUST, tempLocation, 1, 0, ShadowMode);
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
                    NewEffect(EFFID_SKID_DUST, tempLocation, 1, 0, ShadowMode);
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

        if(g_config.fix_climb_bgo_speed_adding && p.VineBGO > 0)
        {
            const Layer_t& layer = Layer[Background[p.VineBGO].Layer];

            doesPlayerMoves = !fEqual(p.Location.SpeedX,  (double)layer.ApplySpeedX) ||
                               p.Location.SpeedY < layer.ApplySpeedY - 0.1;
        }
        else
        {
            doesPlayerMoves = !fEqual(p.Location.SpeedX,  NPC[p.VineNPC].Location.SpeedX) ||
                               p.Location.SpeedY < NPC[p.VineNPC].Location.SpeedY - 0.1;
        }

        if(doesPlayerMoves) // Or .Location.SpeedY > 0.1 Then
        {
            p.FrameCount += 1;
            if(p.FrameCount >= 8)
            {
                p.Frame += 1;
                p.FrameCount = 0;
            }
            PlaySoundSpatial(SFX_Climbing, p.Location);
        }

        if(p.Frame < 25)
            p.Frame = 26;
        else if(p.Frame > 26)
            p.Frame = 25;

        return;
    }

// this finds the players direction
    if(!LevelSelect && p.Effect != PLREFF_WARP_PIPE)
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
                        if(p.Location.SpeedX > 0 && (p.Controls.Left || (p.Direction == -1 && p.Bumped)) && p.Effect == PLREFF_NORMAL && p.Quicksand == 0)
                        {
                            if(!LevelSelect)
                            {
                                if(p.Mount != 2 && !p.WetFrame && !p.Duck)
                                {
                                    PlaySoundSpatial(SFX_Skid, p.Location);
                                    if(p.SlideCounter <= 0)
                                    {
                                        p.SlideCounter = 2 + iRand_round(2);
                                        tempLocation.Y = p.Location.Y + p.Location.Height - 5;
                                        tempLocation.X = p.Location.X + p.Location.Width / 2.0 - 4 + 8 * -p.Direction;
                                        NewEffect(EFFID_SKID_DUST, tempLocation, 1, 0, ShadowMode);
                                    }
                                }

                                p.Frame = 4;
                            }
                        }
                        else if(p.Location.SpeedX < 0 && (p.Controls.Right || (p.Direction == 1 && p.Bumped)) && p.Effect == PLREFF_NORMAL && p.Quicksand == 0)
                        {
                            if(!LevelSelect)
                            {
                                if(p.Mount != 2 && !p.WetFrame && !p.Duck)
                                {
                                    PlaySoundSpatial(SFX_Skid, p.Location);
                                    if(p.SlideCounter <= 0)
                                    {
                                        p.SlideCounter = 2 + iRand_round(2);
                                        tempLocation.Y = p.Location.Y + p.Location.Height - 5;
                                        tempLocation.X = p.Location.X + p.Location.Width / 2.0 - 4 + 8 * -p.Direction;
                                        NewEffect(EFFID_SKID_DUST, tempLocation, 1, 0, ShadowMode);
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
                        p.Effect == PLREFF_NORMAL && !p.Duck)
                    {
                        PlaySoundSpatial(SFX_Skid, p.Location);
                        if(p.SlideCounter <= 0)
                        {
                            p.SlideCounter = 2 + iRand_round(2);
                            tempLocation.Y = p.Location.Y + p.Location.Height - 5;
                            tempLocation.X = p.Location.X + p.Location.Width / 2.0 - 4 + 10 * -p.Direction;
                            NewEffect(EFFID_SKID_DUST, tempLocation, 1, 0, ShadowMode);
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
        // TODO: state-dependent moment
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
            if(p.HoldingNPC == 0 || (p.Effect == PLREFF_WARP_PIPE && p.Character >= 3))
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
                           p.Effect == PLREFF_NORMAL && !p.Duck && p.Quicksand == 0)
                        {
                            if(!LevelSelect)
                            {
                                if(p.Mount != 2 && p.Wet == 0)
                                {
                                    PlaySoundSpatial(SFX_Skid, p.Location);
                                    if(p.SlideCounter <= 0)
                                    {
                                        p.SlideCounter = 2 + iRand_round(2);
                                        tempLocation.Y = p.Location.Y + p.Location.Height - 5;
                                        tempLocation.X = p.Location.X + p.Location.Width / 2.0 - 4 + 6 * -p.Direction;
                                        NewEffect(EFFID_SKID_DUST, tempLocation, 1, 0, ShadowMode);
                                    }
                                }
                                p.Frame = 6;
                            }
                        }
                        else if(p.Location.SpeedX < 0 && (p.Controls.Right || (p.Direction == 1 && p.Bumped)) &&
                                p.Effect == PLREFF_NORMAL && !p.Duck && p.Quicksand == 0)
                        {
                            if(!LevelSelect)
                            {
                                if(p.Mount != 2 && p.Wet == 0)
                                {
                                    PlaySoundSpatial(SFX_Skid, p.Location);
                                    if(p.SlideCounter <= 0)
                                    {
                                        p.SlideCounter = 2 + iRand_round(2);
                                        tempLocation.Y = p.Location.Y + p.Location.Height - 5;
                                        tempLocation.X = p.Location.X + p.Location.Width / 2.0 - 4 + 10 * -p.Direction;
                                        NewEffect(EFFID_SKID_DUST, tempLocation, 1, 0, ShadowMode);
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
                       p.Effect == PLREFF_NORMAL && !p.Duck)
                    {
                        PlaySoundSpatial(SFX_Skid, p.Location);
                        if(p.SlideCounter <= 0)
                        {
                            p.SlideCounter = 2 + iRand_round(2);
                            tempLocation.Y = p.Location.Y + p.Location.Height - 5;
                            tempLocation.X = p.Location.X + p.Location.Width / 2.0 - 4 + 10 * -p.Direction;
                            NewEffect(EFFID_SKID_DUST, tempLocation, 1, 0, ShadowMode);
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
                    if(p.Effect == PLREFF_NORMAL)
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

void UpdatePlayerBonus(const int A, const NPCID B)
{
    auto &p = Player[A];

    // INCORRECT NOTE: I have traced all paths into this code, and it is unreachable if p.Effect != PLREFF_NORMAL
    // NOTE: this is not true if the player touches two bonuses in the same frame.

    // 1 player growing
    // 4 fire flower
    // 5 leaf
    if(p.State != 1 || (p.Effect == PLREFF_TURN_BIG || p.Effect == PLREFF_TURN_FIRE || p.Effect == PLREFF_TURN_LEAF))
    {
        if(B == NPCID_POWER_S3 || B == NPCID_POWER_S4 || B == NPCID_POWER_S1 || B == NPCID_POWER_S5)
        {
            if(p.HeldBonus == 0)
                p.HeldBonus = B;
        }
        else if((p.State == 2 || p.Effect == PLREFF_TURN_BIG) && !(p.Effect == PLREFF_TURN_FIRE || p.Effect == PLREFF_TURN_LEAF))
        {
            if(p.HeldBonus == 0)
            {
                if(p.StateNPC == NPCID_POWER_S1)
                    p.HeldBonus = NPCID_POWER_S1;
                else if(p.StateNPC == NPCID_POWER_S4)
                    p.HeldBonus = NPCID_POWER_S4;
                else
                    p.HeldBonus = NPCID_POWER_S3;
            }
        }
        else
        {
            if(p.State == 3 || p.Effect == PLREFF_TURN_FIRE)
            {
                if(p.StateNPC == NPCID_FIRE_POWER_S4)
                    p.HeldBonus = NPCID_FIRE_POWER_S4;
                else if(p.StateNPC == NPCID_FIRE_POWER_S1)
                    p.HeldBonus = NPCID_FIRE_POWER_S1;
                else
                    p.HeldBonus = NPCID_FIRE_POWER_S3;
            }

            if(p.State == 4 || p.Effect == PLREFF_TURN_LEAF)
                p.HeldBonus = NPCID_LEAF_POWER;

            if(p.State == 5 || p.Effect == PLREFF_TURN_STATUE)
                p.HeldBonus = NPCID_STATUE_POWER;

            if(p.State == 6 || p.Effect == PLREFF_TURN_HEAVY)
                p.HeldBonus = NPCID_HEAVY_POWER;

            if(p.State == 7 || p.Effect == PLREFF_TURN_ICE)
            {
                if(p.StateNPC == NPCID_ICE_POWER_S4)
                    p.HeldBonus = NPCID_ICE_POWER_S4;
                else
                    p.HeldBonus = NPCID_ICE_POWER_S3;
            }

            // TODO: State-dependent moment, extend for states > 7, using the 38A NPCIDs and then our own dedicated range. It's not safe to rely on StateNPC because it doesn't get saved.
        }
    }

    if(p.Character == 3 || p.Character == 4 || p.Character == 5)
        p.HeldBonus = NPCID(0);
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

        for(BlockRef_t block_p : treeFLBlockQuery(tailLoc, SORTMODE_COMPAT))
        {
            Block_t& block = *block_p;
            A = (int)block_p;

            if(!BlockIsSizable[block.Type] && !block.Hidden && (block.Type != 293 || Stab) && !block.Invis && !BlockNoClipping[block.Type])
            {
                if(CheckCollision(tailLoc, block.Location))
                {
                    // if(block.ShakeY == 0 && block.ShakeY2 == 0 && block.ShakeY3 == 0)
                    if(block.ShakeCounter == 0)
                    {
                        if(block.Special > 0 || block.Type == 55 || block.Type == 159 || block.Type == 90)
                            PlaySoundSpatial(SFX_BlockHit, block.Location);
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
                            // if(block.ShakeY != 0)
                            if(block.ShakeCounter != 0)
                            {
                                tempLoc.X = (block.Location.X + tailLoc.X + (block.Location.Width + tailLoc.Width) / 2.0) / 2 - 16;
                                tempLoc.Y = (block.Location.Y + tailLoc.Y + (block.Location.Height + tailLoc.Height) / 2.0) / 2 - 16;
                                NewEffect(EFFID_WHIP, tempLoc);
                            }
                            break;
                        }
                        else
                        {
                            if(StabDir == 2)
                            {
                                if(block.Type == 293 || block.Type == 370
                                    /* || block.ShakeY != 0 || block.ShakeY2 != 0 || block.ShakeY3 != 0 */
                                    || block.ShakeCounter != 0
                                    || block.Hidden || BlockHurts[block.Type])
                                {
                                    if(BlockHurts[block.Type])
                                        PlaySoundSpatial(SFX_Spring, block.Location);
                                    p.Location.Y -= 0.1;
                                    p.Location.SpeedY = Physics.PlayerJumpVelocity;
                                    p.StandingOnNPC = 0;
                                    if(p.Controls.Jump || p.Controls.AltJump)
                                        p.Jump = 10;
                                }
                            }

                            if(block.Type == 370)
                            {
                                PlaySoundSpatial(SFX_HeroGrass, block.Location);
                                block.Hidden = true;
                                block.Layer = LAYER_DESTROYED_BLOCKS;
                                syncLayersTrees_Block(A);
                                NewEffect(EFFID_SMOKE_S3, block.Location);
                                Effect[numEffects].Location.SpeedY = -2;
                            }

                            // allow Char5 to stab gray bricks when it has heavy power
                            if(block.Type == 457 && p.State == 6)
                                KillBlock(A);
                        }
                    }
                }
            }
        }
    }

    int numNPCsMax5 = numNPCs;

    // need this complex loop syntax because Active can be modified within it
    for(int A : NPCQueues::Active.may_erase)
    {
        if(A > numNPCsMax5)
            continue;

        if(NPC[A].Active && NPC[A].Effect == NPCEFF_NORMAL && !(NPCIsAnExit(NPC[A]) || (NPC[A]->IsACoin && !Stab)) &&
            NPC[A].CantHurtPlayer != plr && !(p.StandingOnNPC == A && p.ShellSurf))
        {
            if(NPC[A].Type != NPCID_PLR_FIREBALL && NPC[A].Type != NPCID_PLR_ICEBALL && !(NPC[A].Type == NPCID_BULLET && NPC[A].Projectile) &&
                NPC[A].Type != NPCID_PET_FIRE && NPC[A].Type != NPCID_GOALTAPE && NPC[A].Type != NPCID_CHECKPOINT)
            {
                stabLoc = NPC[A].Location;
                if(NPC[A]->HeightGFX > NPC[A].Location.Height && NPC[A].Type != NPCID_PLANT_S3 && NPC[A].Type != NPCID_MINIBOSS &&
                    NPC[A].Type != NPCID_WALL_BUG && NPC[A].Type != NPCID_POWER_S3 && NPC[A].Type != NPCID_BOTTOM_PLANT && NPC[A].Type != NPCID_SIDE_PLANT &&
                    NPC[A].Type != NPCID_BIG_PLANT && NPC[A].Type != NPCID_PLANT_S1 && NPC[A].Type != NPCID_FIRE_PLANT)
                {
                    stabLoc.Y += stabLoc.Height;
                    stabLoc.Height = NPC[A]->HeightGFX;
                    stabLoc.Y += -stabLoc.Height;
                }

                if(NPC[A].Type == NPCID_ITEM_BURIED && Stab)
                    stabLoc.Y += -stabLoc.Height;

                if(CheckCollision(tailLoc, stabLoc) && NPC[A].Killed == 0 && NPC[A].TailCD == 0 && !(StabDir != 0 && NPC[A].Type == NPCID_ITEM_BURIED))
                {
                    oldNPC = NPC[A];
                    if(Stab)
                    {
                        B = NPC[A].Damage;
                        C = NPC[A].Type;

                        if(StabDir == 2 && ((NPC[A].Type >= NPCID_CARRY_BLOCK_A && NPC[A].Type <= NPCID_CARRY_BLOCK_D) || NPC[A].Type == NPCID_SPRING || NPC[A].Type == NPCID_COIN_SWITCH || NPC[A].Type == NPCID_TIME_SWITCH || NPC[A].Type == NPCID_EARTHQUAKE_BLOCK))
                        {
                        }
                        else
                        {
                            if(NPC[A].Type == NPCID_SLIDE_BLOCK && StabDir != 0)
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
                            NewEffect(EFFID_WHIP, tempLoc);
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

                if(CheckCollision(tailLoc, stabLoc) && Player[A].Effect == PLREFF_NORMAL &&
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
                        PlaySoundSpatial(SFX_HeroHit, Player[A].Location);
                    }
                    else
                    {
                        Player[A].Location.SpeedX = 6 * p.Direction;
                        Player[A].Location.SpeedY = -5;
                        PlaySoundSpatial(SFX_Stomp, Player[A].Location);
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

        if(B != A && p2.Effect == PLREFF_NORMAL && !p2.Dead && p2.TimeToLive == 0 && p2.Mount == 0)
        {
            if(CheckCollision(p.YoshiTongue, p2.Location))
            {
                p.YoshiPlayer = B;
                p2.HoldingNPC = 0;
                return;
            }
        }
    }

    for(int B : treeNPCQuery(p.YoshiTongue, SORTMODE_ID))
    {
        auto &n = NPC[B];
        if(((n->IsACoin && n.Special == 1) || !n->NoYoshi) &&
           n.Active && ((!n->IsACoin || n.Special == 1) || n.Type == 103) &&
           !NPCIsAnExit(n) && !n.Generator && !n.Inert && !NPCIsYoshi(n) &&
            n.Effect != NPCEFF_PET_TONGUE && n.Immune == 0 && n.Type != NPCID_ITEM_BURIED &&
            !(n.Projectile && n.Type == NPCID_BULLET) && n.HoldingPlayer == 0)
        {
            tempLocation = n.Location;
            // dead code
#if 0
            if(n.Type == NPCID_ITEM_BURIED)
                tempLocation.Y = n.Location.Y - 16;
#endif

            if(CheckCollision(p.YoshiTongue, tempLocation))
            {
                // dead code, check n.Type != 91 condition above
#if 0
                if(n.Type == NPCID_ITEM_BURIED)
                {
                    if(!NPCTraits[(int)n.Special].NoYoshi)
                    {
                        PlaySound(SFX_Grab);
                        n.Generator = false;
                        n.Frame = 0;
                        n.Type = n.Special;
                        n.Special = 0;

                        if(NPCIsYoshi(n))
                        {
                            n.Special = n.Type;
                            n.Type = 96;
                        }

                        n.Location.Height = n->THeight;
                        n.Location.Width = n->TWidth;

                        if(!(n.Type == 21 || n.Type == 22 || n.Type == 26 || n.Type == 31 || n.Type == 32 || n.Type == 35 || n.Type == 49 || NPCIsAnExit(n)))
                            n.DefaultType = 0;

                        n.Effect = 5;
                        n.Effect2 = A;
                        p.YoshiNPC = B;

                        NPCQueues::Unchecked.push_back(B);
                        treeNPCUpdate(B);
                    }
                }
                else
#endif
                if(n.Type == NPCID_ITEM_BUBBLE)
                {
                    NPCHit(B, 3, B);
                }
                else
                {
                    n.Effect = NPCEFF_PET_TONGUE;
                    n.Effect2 = A;
                    n.Location.Height = n->THeight;
                    p.YoshiNPC = B;
                    treeNPCUpdate(B);
                }

                if(n.Type == NPCID_VEGGIE_RANDOM)
                {
                    n.Type = NPCID(NPCID_VEGGIE_2 + iRand(9));
                    if(n.Type == NPCID_VEGGIE_RANDOM)
                        n.Type = NPCID_VEGGIE_1;

                    n.Location.X += n.Location.Width / 2.0;
                    n.Location.Y += n.Location.Height / 2.0;
                    n.Location.Width = n->TWidth;
                    n.Location.Height = n->THeight;
                    n.Location.X += -n.Location.Width / 2.0;
                    n.Location.Y += -n.Location.Height / 2.0;

                    NPCQueues::Unchecked.push_back(B);
                    treeNPCUpdate(B);
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
        Player[p.YoshiPlayer].Effect = PLREFF_NORMAL;
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
        PlaySoundSpatial(SFX_SpitBossSpit, p.Location);
    }
    else
    {
        NPC[p.YoshiNPC].RealSpeedX = 0;
        if(NPC[p.YoshiNPC]->IsAShell)
            NPC[p.YoshiNPC].Special = 0;

        if((NPC[p.YoshiNPC]->IsAShell || NPCIsABot(NPC[p.YoshiNPC]) || NPC[p.YoshiNPC].Type == NPCID_RAINBOW_SHELL) &&
           NPC[p.YoshiNPC].Type != NPCID_GLASS_SHELL && p.YoshiRed)
        {
            NPC[p.YoshiNPC].Killed = 9;
            NPCQueues::Killed.push_back(p.YoshiNPC);
            PlaySoundSpatial(SFX_BigFireball, p.Location);
            for(B = 1; B <= 3; B++)
            {
                numNPCs++;
                NPC[numNPCs] = NPC_t();
                NPC[numNPCs].Direction = p.Direction;
                NPC[numNPCs].Type = NPCID_PET_FIRE;
                NPC[numNPCs].Frame = EditorNPCFrame(NPC[numNPCs].Type, NPC[numNPCs].Direction);
                NPC[numNPCs].Active = true;
                NPC[numNPCs].Section = p.Section;
                NPC[numNPCs].TimeLeft = 100;
                NPC[numNPCs].Effect = NPCEFF_NORMAL;
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
            NPC[p.YoshiNPC].Effect = NPCEFF_NORMAL;
            NPC[p.YoshiNPC].Effect2 = 0;
            NPC[p.YoshiNPC].Location.X = p.Location.X + p.YoshiTX + 32 * p.Direction;
            NPC[p.YoshiNPC].Location.Y = p.Location.Y + p.YoshiTY;
            if(p.Duck)
                NPC[p.YoshiNPC].Location.Y -= 8;
            NPC[p.YoshiNPC].Location.Y -= 2;
            NPC[p.YoshiNPC].Location.SpeedX = 0;
            NPC[p.YoshiNPC].Location.SpeedY = 0;

            if(p.YoshiNPC > 0 && p.YoshiNPC <= numNPCs)
            {
                NPCQueues::Active.insert(p.YoshiNPC);
                NPCQueues::Unchecked.push_back(p.YoshiNPC);
                treeNPCUpdate(p.YoshiNPC);
            }



            if(NPC[p.YoshiNPC].Type == NPCID_SLIDE_BLOCK)
                NPC[p.YoshiNPC].Special = 1;

            PlaySoundSpatial(SFX_SpitBossSpit, p.Location);

            if(!p.Controls.Down || (p.Location.SpeedY != 0 && p.StandingOnNPC == 0 && p.Slope == 0))
            {
                if(NPC[p.YoshiNPC]->IsAShell)
                {
                    SoundPause[SFX_ShellHit] = 2;
                    // NPCHit .YoshiNPC, 1, A
                    NPC[p.YoshiNPC].Location.SpeedX = Physics.NPCShellSpeed * p.Direction;
                }
                else if(NPC[p.YoshiNPC].Type == NPCID_SLIDE_BLOCK)
                    NPC[p.YoshiNPC].Location.SpeedX = Physics.NPCShellSpeed * p.Direction;
                else
                {
                    NPC[p.YoshiNPC].Projectile = true;
                    NPC[p.YoshiNPC].Location.SpeedX = 7 * p.Direction;
                    NPC[p.YoshiNPC].Location.SpeedY = -1.3;
                }
            }

            if(NPC[p.YoshiNPC].Type == NPCID_ICE_BLOCK)
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
    // int B = 0;
    Location_t tempLocation;
    Location_t tempLocation2;
    auto &p = Player[A];

    if(p.Location.SpeedY > 3)
    {

        tempLocation.Width = 128;
        tempLocation.X = p.Location.X + p.Location.Width / 2.0 - tempLocation.Width / 2.0;
        tempLocation.Height = 32;
        tempLocation.Y = p.Location.Y + p.Location.Height - 16;

        for(int B : NPCQueues::Active.may_erase)
        {
            if(!NPC[B].Hidden && NPC[B].Active && NPC[B].Effect == NPCEFF_NORMAL)
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
            for(BlockRef_t block : treeBlockQuery(p.Location, SORTMODE_COMPAT))
            {
                Block_t& b = block;
                int B = block;

                if(b.Hidden || b.Invis || BlockNoClipping[b.Type] || BlockIsSizable[b.Type])
                    continue;

                if(g_config.fix_vehicle_char_switch && mount == 2 &&
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
        NewEffect(EFFID_SMOKE_S3, tempLocation);
        Effect[numEffects].Location.SpeedX = -2;
        tempLocation.X = p.Location.X + p.Location.Width / 2.0 - 16 + 16;
        NewEffect(EFFID_SMOKE_S3, tempLocation);
        Effect[numEffects].Location.SpeedX = 2;
        PlaySoundSpatial(SFX_Stone, p.Location);
        if(BreakBlocks && g_config.extra_screen_shake)
            doShakeScreen(0, 4, SHAKE_SEQUENTIAL, 4, 0.2);
    }
}

void PlayerDismount(const int A)
{
    auto &p = Player[A];

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
        PlaySoundSpatial(SFX_Jump, p.Location); // Jump sound
        PlaySoundSpatial(SFX_Boot, p.Location);
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
            NPC[numNPCs].Type = NPCID_GRN_BOOT;
        else if(Player[A].MountType == 2)
            NPC[numNPCs].Type = NPCID_RED_BOOT;
        else if(Player[A].MountType == 3)
            NPC[numNPCs].Type = NPCID_BLU_BOOT;

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
        PlaySoundSpatial(SFX_Jump, p.Location); // Jump sound
        PlaySoundSpatial(SFX_Boot, p.Location);
        Player[A].Jump = Physics.PlayerJumpHeight;
        if(Player[A].Character == 2)
            Player[A].Jump = Player[A].Jump + 3;
        if(Player[A].SpinJump)
            Player[A].Jump = Player[A].Jump - 6;
        Player[A].Mount = 0;
        numNPCs++;
        NPC[numNPCs].Direction = Player[A].Direction;
        if(NPC[numNPCs].Direction == 1)
            NPC[numNPCs].Frame = 4;
        NPC[numNPCs].Frame = NPC[numNPCs].Frame + SpecialFrame[2];
        NPC[numNPCs].Active = true;
        NPC[numNPCs].TimeLeft = 100;
        NPC[numNPCs].Type = NPCID_VEHICLE;
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

            if(Player[B].StandingOnVehiclePlr && (g_ClonedPlayerMode || Player[B].StandingOnVehiclePlr == A))
            {
                Player[B].StandingOnNPC = numNPCs;
                Player[B].StandingOnVehiclePlr = 0;
            }
        }

        for(int B : NPCQueues::Active.no_change)
        {
            if(NPC[B].vehiclePlr == A)
            {
                NPC[B].vehiclePlr = 0;
                NPC[B].Location.SpeedY = 0;
                NPC[B].Location.Y = NPC[numNPCs].Location.Y - 0.1 - NPC[B].vehicleYOffset;
                treeNPCUpdate(B);

                NPC[B].vehicleYOffset = 0;
                if(NPC[B].Type == NPCID_CANNONITEM)
                    NPC[B].Special = 0;
                if(NPC[B].Type == NPCID_TOOTHY)
                {
                    NPC[B].Killed = 9;
                    NPC[B].Special = 0;
                    NPCQueues::Killed.push_back(B);
                }
                else if(NPC[B].Type == NPCID_TOOTHYPIPE)
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
            NPC[numNPCs].Type = NPCID_PET_GREEN;
        else if(Player[A].MountType == 2)
            NPC[numNPCs].Type = NPCID_PET_BLUE;
        else if(Player[A].MountType == 3)
            NPC[numNPCs].Type = NPCID_PET_YELLOW;
        else if(Player[A].MountType == 4)
            NPC[numNPCs].Type = NPCID_PET_RED;
        else if(Player[A].MountType == 5)
            NPC[numNPCs].Type = NPCID_PET_BLACK;
        else if(Player[A].MountType == 6)
            NPC[numNPCs].Type = NPCID_PET_PURPLE;
        else if(Player[A].MountType == 7)
            NPC[numNPCs].Type = NPCID_PET_PINK;
        else if(Player[A].MountType == 8)
            NPC[numNPCs].Type = NPCID_PET_CYAN;

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
            PlaySoundSpatial(SFX_Jump, p.Location); // Jump sound
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

    UpdatableQuery<BlockRef_t> q(p.Location, SORTMODE_COMPAT, QUERY_FLBLOCK);

    for(auto it = q.begin(); it != q.end(); ++it)
    {
        int B = *it;
        Block_t& b = **it;

        if(b.Hidden || BlockIsSizable[b.Type])
            continue;

        // Note: could also apply this for a downwards clip when colliding with an invisible block
        if(g_config.fix_player_filter_bounce && BlockCheckPlayerFilter(B, A))
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
                    {
                        // Note: this vanilla peculiarity (Width should be subtracted) only affects a victim spat rightwards out of a pet's mouth.
                        // If spat rightwards against a wall, the victim teleports to the left through the player who spat them out.
                        // If spat leftwards against a wall (non-bugged behavior), the player who spat them out gets pushed rightwards.
                        p.Location.X = b.Location.X - p.Location.Height - 0.01;
                    }
                    else if(HitSpot == 3)
                        p.Location.Y = b.Location.Y + b.Location.Height + 0.01;
                    else if(HitSpot == 4)
                        p.Location.X = b.Location.X + b.Location.Width + 0.01;

                    q.update(p.Location, it);
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
    // Location_t tempLocation;
    auto &p = Player[A];

    if(p.Mount == 3 && !p.Fairy)
    {
    // Shell Colors
        if(p.MountSpecial == 0)
        {
            if(p.YoshiNPC > 0)
            {
                if(NPC[p.YoshiNPC].Type == NPCID_RED_SHELL_S3 || NPC[p.YoshiNPC].Type == NPCID_RED_TURTLE_S3)
                    p.YoshiRed = true;
                if(NPC[p.YoshiNPC].Type == NPCID_RED_TURTLE_S4 || NPC[p.YoshiNPC].Type == NPCID_RED_SHELL_S4 || NPC[p.YoshiNPC].Type == NPCID_RED_SLIME)
                    p.YoshiRed = true;
                if(NPC[p.YoshiNPC].Type == NPCID_RED_TURTLE_S4 || NPC[p.YoshiNPC].Type == NPCID_RED_SHELL_S4 || NPC[p.YoshiNPC].Type == NPCID_RED_SLIME)
                    p.YoshiRed = true;
                if(NPC[p.YoshiNPC].Type == NPCID_RED_SHELL_S1 || NPC[p.YoshiNPC].Type == NPCID_RED_TURTLE_S1 || NPC[p.YoshiNPC].Type == NPCID_RED_FLY_TURTLE_S1 || NPC[p.YoshiNPC].Type == NPCID_RAINBOW_SHELL)
                    p.YoshiRed = true;

                if(NPC[p.YoshiNPC].Type == NPCID_BLU_TURTLE_S4 || NPC[p.YoshiNPC].Type == NPCID_BLU_SHELL_S4 || NPC[p.YoshiNPC].Type == NPCID_RAINBOW_SHELL || NPC[p.YoshiNPC].Type == NPCID_FLIPPED_RAINBOW_SHELL)
                {
                    if(!p.YoshiBlue)
                    {
                        p.CanFly2 = true;
                        p.CanFly2 = true;
                        p.FlyCount = 300;
                    }
                    p.YoshiBlue = true;
                }

                if(NPC[p.YoshiNPC].Type == NPCID_YEL_TURTLE_S4 || NPC[p.YoshiNPC].Type == NPCID_YEL_SHELL_S4 ||
                   NPC[p.YoshiNPC].Type == NPCID_CYAN_SLIME || NPC[p.YoshiNPC].Type == NPCID_RAINBOW_SHELL ||
                   NPC[p.YoshiNPC].Type == NPCID_FLIPPED_RAINBOW_SHELL)
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
            if(NPC[p.YoshiNPC].Type == NPCID_KEY) // key check
                KeyholeCheck(A, p.Location);
            else if(NPC[p.YoshiNPC].Type == NPCID_SLIDE_BLOCK)
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
                            PlaySoundSpatial(SFX_PetTongue, p.Location);
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

                if(p.YoshiNPC <= numNPCs)
                    treeNPCUpdate(p.YoshiNPC);
            }

            if(p.YoshiPlayer > 0)
            {
                Player[p.YoshiPlayer].Effect = PLREFF_NO_COLLIDE;
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

            if(NPC[p.YoshiNPC].Type == NPCID_GRN_TURTLE_S3 || NPC[p.YoshiNPC].Type == NPCID_GRN_FLY_TURTLE_S3)
                NPC[p.YoshiNPC].Type = NPCID_GRN_SHELL_S3;
            else if(NPC[p.YoshiNPC].Type == NPCID_RED_TURTLE_S3 || NPC[p.YoshiNPC].Type == NPCID_RED_FLY_TURTLE_S3)
                NPC[p.YoshiNPC].Type = NPCID_RED_SHELL_S3;
            else if(NPC[p.YoshiNPC].Type == NPCID_GLASS_TURTLE)
                NPC[p.YoshiNPC].Type = NPCID_GLASS_SHELL;
            else if(NPC[p.YoshiNPC].Type == NPCID_BIG_TURTLE)
                NPC[p.YoshiNPC].Type = NPCID_BIG_SHELL;
            else if(NPC[p.YoshiNPC].Type >= NPCID_GRN_TURTLE_S4 && NPC[p.YoshiNPC].Type <= NPCID_YEL_TURTLE_S4)
                NPC[p.YoshiNPC].Type = NPCID(NPC[p.YoshiNPC].Type + 4);
            else if(NPC[p.YoshiNPC].Type >= NPCID_GRN_FLY_TURTLE_S4 && NPC[p.YoshiNPC].Type <= NPCID_YEL_FLY_TURTLE_S4)
            {
                NPC[p.YoshiNPC].Type = NPCID(NPC[p.YoshiNPC].Type - 8);
                NPC[p.YoshiNPC].Special = 0;
            }
            else if(NPC[p.YoshiNPC].Type == NPCID_GRN_TURTLE_S1 || NPC[p.YoshiNPC].Type == NPCID_GRN_FLY_TURTLE_S1)
            {
                NPC[p.YoshiNPC].Type = NPCID_GRN_SHELL_S1;
                NPC[p.YoshiNPC].Location.Height = 28;
            }
            else if(NPC[p.YoshiNPC].Type == NPCID_RED_TURTLE_S1 || NPC[p.YoshiNPC].Type == NPCID_RED_FLY_TURTLE_S1)
            {
                NPC[p.YoshiNPC].Type = NPCID_RED_SHELL_S1;
                NPC[p.YoshiNPC].Location.Height = 28;
            }

            NPC[p.YoshiNPC].Location.Height = NPC[p.YoshiNPC]->THeight;
            if((NPC[p.YoshiNPC]->IsGrabbable || NPC[p.YoshiNPC]->IsAShell || NPC[p.YoshiNPC].Type == NPCID_SPIT_BOSS_BALL || NPCIsABot(NPC[p.YoshiNPC]) || NPC[p.YoshiNPC].Type == NPCID_RAINBOW_SHELL || NPC[p.YoshiNPC].Type == NPCID_WALK_BOMB_S2 || NPC[p.YoshiNPC].Type == NPCID_WALK_BOMB_S3 || NPC[p.YoshiNPC].Type == NPCID_LIT_BOMB_S3) && (NPC[p.YoshiNPC].Type != NPCID_HIT_CARRY_FODDER))
            {
                if(NPC[p.YoshiNPC].Type == NPCID_WALK_BOMB_S2)
                    NPC[p.YoshiNPC].Special = 450;
                if(NPC[p.YoshiNPC].Type == NPCID_BOMB)
                    NPC[p.YoshiNPC].Special = 250;
                if(NPC[p.YoshiNPC].Type == NPCID_WALK_BOMB_S3 || NPC[p.YoshiNPC].Type == NPCID_LIT_BOMB_S3)
                {
                    NPC[p.YoshiNPC].Special = 250;
                    NPC[p.YoshiNPC].Type = NPCID_LIT_BOMB_S3;
                    NPC[p.YoshiNPC].Location.Height = NPC[p.YoshiNPC]->THeight;
                }

                NPC[p.YoshiNPC].Effect = NPCEFF_PET_INSIDE;
                NPC[p.YoshiNPC].Effect2 = A;

                if(NPC[p.YoshiNPC].Active)
                {
                    NPC[p.YoshiNPC].Active = false;
                    NPCQueues::update(p.YoshiNPC);
                }

                if(NPC[p.YoshiNPC].Type == NPCID_TOOTHYPIPE)
                {
                    NPC[p.YoshiNPC].Special = 0;
                    NPC[p.YoshiNPC].Special2 = 0;
                }
            }
            else if(p.MountType == 7 && !NPC[p.YoshiNPC]->IsABonus)
            {
                B = iRand(9);
                NPC[p.YoshiNPC].Type = NPCID(NPCID_VEGGIE_2 + B);
                if(NPC[p.YoshiNPC].Type == NPCID_VEGGIE_RANDOM)
                    NPC[p.YoshiNPC].Type = NPCID_VEGGIE_1;
                NPC[p.YoshiNPC].Location.X += NPC[p.YoshiNPC].Location.Width / 2.0;
                NPC[p.YoshiNPC].Location.Y += NPC[p.YoshiNPC].Location.Height / 2.0;
                NPC[p.YoshiNPC].Location.Width = NPC[p.YoshiNPC]->TWidth;
                NPC[p.YoshiNPC].Location.Height = NPC[p.YoshiNPC]->THeight;
                NPC[p.YoshiNPC].Location.X += -NPC[p.YoshiNPC].Location.Width / 2.0;
                NPC[p.YoshiNPC].Location.Y += -NPC[p.YoshiNPC].Location.Height / 2.0;
                NPC[p.YoshiNPC].Effect = NPCEFF_PET_INSIDE;
                NPC[p.YoshiNPC].Effect2 = A;

                NPCQueues::Unchecked.push_back(p.YoshiNPC);

                if(NPC[p.YoshiNPC].Active)
                {
                    NPC[p.YoshiNPC].Active = false;
                    NPCQueues::update(p.YoshiNPC);
                }
            }
            else if(p.MountType == 8 && !NPC[p.YoshiNPC]->IsABonus)
            {
                NPC[p.YoshiNPC].Type = NPCID_ICE_BLOCK;
                NPC[p.YoshiNPC].Location.X += NPC[p.YoshiNPC].Location.Width / 2.0;
                NPC[p.YoshiNPC].Location.Y += NPC[p.YoshiNPC].Location.Height / 2.0;
                NPC[p.YoshiNPC].Location.Width = NPC[p.YoshiNPC]->TWidth;
                NPC[p.YoshiNPC].Location.Height = NPC[p.YoshiNPC]->THeight;
                NPC[p.YoshiNPC].Location.X += -NPC[p.YoshiNPC].Location.Width / 2.0;
                NPC[p.YoshiNPC].Location.Y += -NPC[p.YoshiNPC].Location.Height / 2.0;
                NPC[p.YoshiNPC].Effect = NPCEFF_PET_INSIDE;
                NPC[p.YoshiNPC].Effect2 = A;

                NPCQueues::Unchecked.push_back(p.YoshiNPC);

                if(NPC[p.YoshiNPC].Active)
                {
                    NPC[p.YoshiNPC].Active = false;
                    NPCQueues::update(p.YoshiNPC);
                }
            }
            else
            {
                if(NPC[p.YoshiNPC]->IsABonus)
                {
                    TouchBonus(A, p.YoshiNPC);
                    p.YoshiNPC = 0;
                }
                else
                {
                    MoreScore(NPC[p.YoshiNPC]->Score, NPC[p.YoshiNPC].Location, p.Multiplier);
                    NPC[p.YoshiNPC].Killed = 9;
                    NPCQueues::Killed.push_back(p.YoshiNPC);

                    p.YoshiNPC = 0;
                    p.FireBallCD = 30;
                    Coins += 1;

                    if(Coins >= 100)
                        Got100Coins();

                    PlaySoundSpatial(SFX_PetSwallow, p.Location);
                }
            }

            if(p.YoshiNPC != 0)
            {
                NPCQueues::Unchecked.push_back(p.YoshiNPC);

                if(p.YoshiNPC <= numNPCs)
                    treeNPCUpdate(p.YoshiNPC);
            }
        }
        else if(p.MountSpecial == 0 && p.YoshiPlayer > 0)
        {
            Player[p.YoshiPlayer].Effect = PLREFF_PET_INSIDE;
            Player[p.YoshiPlayer].Effect2 = A;
            Player[p.YoshiPlayer].Location.X = p.Location.X + p.Location.Width / 2.0 - Player[p.YoshiPlayer].Location.Width / 2.0;
            Player[p.YoshiPlayer].Location.Y = p.Location.Y + p.Location.Height / 2.0 - Player[p.YoshiPlayer].Location.Height / 2.0;
            p.YoshiTFrameCount = 1;
        }
    }
}

void RespawnPlayer(int A, double Direction, double CenterX, double StopY, const vScreen_t& target_screen)
{
    Player[A].Location.Width = Physics.PlayerWidth[Player[A].Character][Player[A].State];
    Player[A].Location.Height = Physics.PlayerHeight[Player[A].Character][Player[A].State];
    Player[A].Frame = 1;
    Player[A].Direction = Direction;
    Player[A].Dead = false;
    Player[A].Location.SpeedX = 0;
    Player[A].Location.SpeedY = 0;
    Player[A].Effect = PLREFF_RESPAWN;
    // location where player stops flashing
    Player[A].Effect2 = StopY - Player[A].Location.Height;
    Player[A].Location.Y = -target_screen.Y - Player[A].Location.Height;
    Player[A].Location.X = CenterX - Player[A].Location.Width / 2.0;
}

void RespawnPlayerTo(int A, int TargetPlayer)
{
    double CenterX = Player[TargetPlayer].Location.X + Player[TargetPlayer].Location.Width / 2.0;

    // don't lose a player when it targets a player who is already respawning
    double StopY;
    if(Player[TargetPlayer].Effect == PLREFF_RESPAWN)
        StopY = Player[TargetPlayer].Effect2 + Player[TargetPlayer].Location.Height;
    else if(Player[TargetPlayer].Mount == 2)
        StopY = Player[TargetPlayer].Location.Y;
    else
        StopY = Player[TargetPlayer].Location.Y + Player[TargetPlayer].Location.Height;

    // technically this would fix a vanilla bug (possible weird effects after Player 2 dies, Player 1 goes through Warp, Player 2 respawns)
    //   so I will do it where it only affects the new code.
    // Player[A].Section = Player[TargetPlayer].Section;

    // respawn at top of vScreen 1 in SMBX dynamic splitscreen, otherwise at top of target player's vScreen
    const Screen_t& screen = ScreenByPlayer(A);
    const vScreen_t& target_vscreen = (screen.Type == ScreenTypes::Dynamic) ? screen.vScreen(1) : vScreenByPlayer(TargetPlayer);

    RespawnPlayer(A, Player[TargetPlayer].Direction, CenterX, StopY, target_vscreen);

    // if TargetPlayer is scrolling in warp, we can't spawn them directly.
    if(PlayerScrollingInWarp(Player[TargetPlayer]))
    {
        // Give player A wings in shared screen
        if(screen.Type == ScreenTypes::SharedScreen)
        {
            Player[A].Location.Y -= 100;
            Player[A].Dead = true;
            Player[A].Effect = PLREFF_COOP_WINGS;
            Player[A].Effect2 = 0;
        }
        // respawn them to the target player's warp otherwise
        else
        {
            const auto& warp = Warp[Player[TargetPlayer].Warp];
            const auto& warp_exit = (Player[TargetPlayer].WarpBackward) ? warp.Entrance : warp.Exit;
            const auto warp_exit_dir = (Player[TargetPlayer].WarpBackward) ? warp.Direction : warp.Direction2;

            Player[A].Location.X = warp_exit.X + warp_exit.Width / 2 - Player[A].Location.Width / 2;

            if(Player[TargetPlayer].Effect == PLREFF_WARP_PIPE && warp_exit_dir == 1)
                Player[A].Effect2 = warp_exit.Y;
            else
                Player[A].Effect2 = warp_exit.Y + warp_exit.Height - Player[A].Location.Height;

            Player[A].Location.Y = Player[A].Effect2 - target_vscreen.Height * 3 / 4;
        }
    }
}

void StealBonus()
{
    int A = 0;
    int B = 0;
    int C = 0;
    UNUSED(C);

    // dead players steal life
    if(BattleMode || GameMenu || GameOutro || g_ClonedPlayerMode)
        return;

    // NOTE: legacy code here accepted TimeToLive <= 0, CheckLiving() requires TimeToLive == 0. TimeToLive should never be negative.
    int alive = CheckLiving();
    if(!alive)
        return;

    for(A = 1; A <= numPlayers; A++)
    {
        if(Player[A].Dead && Player[A].Effect != PLREFF_COOP_WINGS)
        {
            // find other player
            if((g_config.modern_lives_system || Lives > 0) && LevelMacro == LEVELMACRO_OFF)
            {
                if(Player[A].Controls.Jump || Player[A].Controls.Run)
                {
                    B = CheckNearestLiving(A);

                    if(g_config.modern_lives_system)
                    {
                        g_100s--;

                        if(g_100s < 0)
                            Score = 0;
                    }
                    else
                        Lives -= 1;

                    Player[A].State = 1;
                    Player[A].Hearts = 1;
                    RespawnPlayerTo(A, B);
                    PlaySoundSpatial(SFX_DropItem, Player[B].Location);
                }
            }
        }
    }
}

void ClownCar()
{
    // for when the player is in the clown car
    // int A = 0;
    // int B = 0;
//    int C = 0;
    // NPC_t blankNPC;

    for(int A = 1; A <= numPlayers; A++) // Code for running the Koopa Clown Car
    {
        // commenting out because:
        //   (1) misplaced; (2) doesn't work with abstract controls
        // logic moved to Controls::Update()

        // if(numPlayers > 2 && GameMenu == false && LevelMacro == LEVELMACRO_OFF && nPlay.Online == false)
        //     Player[A].Controls = Player[1].Controls;

        Player_t& p = Player[A];
        Location_t& pLoc = p.Location;

        // this code should only apply to living players on the vehicle mount
        if(!(p.Mount == 2 && p.Dead == false && p.TimeToLive == 0))
            continue;

        // vehicle movement code
        if(p.Effect == PLREFF_NORMAL)
        {
            if(p.Controls.Left)
            {
                pLoc.SpeedX -= 0.1;

                if(pLoc.SpeedX > 0)
                    pLoc.SpeedX -= 0.15;
            }
            else if(p.Controls.Right)
            {
                pLoc.SpeedX += 0.1;

                if(pLoc.SpeedX < 0)
                    pLoc.SpeedX += 0.15;
            }
            else
            {
                if(pLoc.SpeedX > 0.2)
                    pLoc.SpeedX -= 0.05;
                else if(pLoc.SpeedX < -0.2)
                    pLoc.SpeedX += 0.05;
                else
                    pLoc.SpeedX = 0;
            }

            if(p.Controls.Up)
            {
                pLoc.SpeedY -= 0.1;

                if(pLoc.SpeedY > 0)
                    pLoc.SpeedY -= 0.2;
            }
            else if(p.Controls.Down)
            {
                pLoc.SpeedY += 0.2;

                if(pLoc.SpeedY < 0)
                    pLoc.SpeedY += 0.2;
            }
            else
            {
                if(pLoc.SpeedY > 0.1)
                    pLoc.SpeedY -= 0.1;
                else if(pLoc.SpeedY < -0.1)
                    pLoc.SpeedY += 0.1;
                else
                    pLoc.SpeedY = 0;
            }

            if(pLoc.SpeedX > 4)
                pLoc.SpeedX = 4;
            else if(pLoc.SpeedX < -4)
                pLoc.SpeedX = -4;
            if(pLoc.SpeedY > 10)
                pLoc.SpeedY = 10;
            else if(pLoc.SpeedY < -4)
                pLoc.SpeedY = -4;
        }

        // create playerTemp NPC for the Vehicle
        numNPCs++;
        NPC[numNPCs] = NPC_t();
        NPC[numNPCs].playerTemp = true;
        NPC[numNPCs].Type = NPCID_VEHICLE;
        NPC[numNPCs].Variant = A; // newly-added to allow setting StandingOnVehiclePlr
        NPC[numNPCs].Active = true;
        NPC[numNPCs].TimeLeft = 100;
        NPC[numNPCs].Location = pLoc;

        if(p.Effect != PLREFF_NORMAL)
        {
            NPC[numNPCs].Location.SpeedX = 0;
            NPC[numNPCs].Location.SpeedY = 0;
        }

        NPC[numNPCs].Location.Y += NPC[numNPCs].Location.SpeedY;
        NPC[numNPCs].Location.X += NPC[numNPCs].Location.SpeedX;
        NPC[numNPCs].Section = p.Section;
        syncLayers_NPC(numNPCs);

        // update other players' StandingOnNPC
        for(int B = 1; B <= numPlayers; B++)
        {
            if(Player[B].StandingOnVehiclePlr && (g_ClonedPlayerMode || Player[B].StandingOnVehiclePlr == A))
            {
                Player[B].StandingOnNPC = numNPCs;
                Player[B].Location.X += double(p.mountBump);

                if(Player[B].Effect != PLREFF_NORMAL)
                {
                    Player[B].Location.Y = pLoc.Y - Player[B].Location.Height;
                    Player[B].Location.X += pLoc.SpeedX;
                }
            }
        }

        // handle NPCs on the vehicle
        for(int B : NPCQueues::Active.may_insert)
        {
            // only want non-toothy NPCs on A's vehicle
            if(!(NPC[B].vehiclePlr == A && NPC[B].Type != NPCID_TOOTHY))
                continue;

            if(p.Effect == PLREFF_NORMAL)
                NPC[B].Location.X += pLoc.SpeedX + double(p.mountBump);

            NPC[B].TimeLeft = 100;
            NPC[B].Location.SpeedY = pLoc.SpeedY;
            NPC[B].Location.SpeedX = 0;

            if(p.Effect != PLREFF_NORMAL)
                NPC[B].Location.SpeedY = 0;

            NPC[B].Location.Y = pLoc.Y + NPC[B].Location.SpeedY + 0.1 - NPC[B].vehicleYOffset;
            treeNPCUpdate(B);

            // extend toothy pipe
            if(p.Controls.Run && NPC[B].Type == NPCID_TOOTHYPIPE)
            {
                // create toothy if it doesn't exist already
                if(NPC[B].Special == 0)
                {
                    NPC[B].Special = 1;
                    numNPCs++;
                    NPC[numNPCs] = NPC_t();
                    NPC[B].Special2 = numNPCs;

                    NPC[numNPCs].Active = true;
                    NPC[numNPCs].Section = p.Section;
                    NPC[numNPCs].TimeLeft = 100;
                    NPC[numNPCs].Type = NPCID_TOOTHY;
                    NPC[numNPCs].Location.Height = 32;
                    NPC[numNPCs].Location.Width = 48;
                    NPC[numNPCs].Special = A;
                    NPC[numNPCs].Special2 = B;
                    NPC[numNPCs].Direction = NPC[B].Direction;

                    if(NPC[numNPCs].Direction == 1)
                        NPC[numNPCs].Frame = 2;
                    syncLayers_NPC(numNPCs);
                }

                // update toothy's position
                for(int C : NPCQueues::Active.no_change)
                {
                    if(NPC[C].Type == NPCID_TOOTHY && NPC[C].Special == A && NPC[C].Special2 == B)
                    {
                        NPC[C].vehiclePlr = A;
                        NPC[C].Projectile = true;
                        NPC[C].Direction = NPC[B].Direction;

                        if(NPC[C].Direction > 0)
                            NPC[C].Location.X = NPC[B].Location.X + 32;
                        else
                            NPC[C].Location.X = NPC[B].Location.X - NPC[C].Location.Width;

                        NPC[C].Location.Y = NPC[B].Location.Y;
                        NPC[C].TimeLeft = 100;
                        treeNPCUpdate(C);
                        break;
                    }
                }
            }

            // check if NPC should stay on the vehicle
            bool still_on_vehicle = false;
            Location_t query_loc = NPC[B].Location;
            query_loc.Y += query_loc.Height + 0.1;
            query_loc.X += 0.5;
            query_loc.Width -= 1;
            query_loc.Height = 1;

            for(int C : treeNPCQuery(query_loc, SORTMODE_NONE))
            {
                if(B != C && (NPC[C].vehiclePlr == A || NPC[C].playerTemp))
                {
                    if(CheckCollision(query_loc, NPC[C].Location))
                    {
                        still_on_vehicle = true;
                        break;
                    }
                }
            }

            if(!still_on_vehicle)
            {
                NPC[B].vehiclePlr = 0;
                NPC[B].vehicleYOffset = 0;
            }
            else
                NPC[B].Location.SpeedX = 0;
        }
    }
}

void WaterCheck(const int A)
{
    Location_t tempLocation;
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
            NewEffect(EFFID_WATER_SPLASH, tempLocation);
        }
    }

    for(int B : treeWaterQuery(p.Location, SORTMODE_NONE))
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
                        NewEffect(EFFID_WATER_SPLASH, tempLocation);
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
                for(int B : treeWaterQuery(tempLocation, SORTMODE_NONE))
                {
                    if(CheckCollision(Water[B].Location, tempLocation))
                    {
                        NewEffect(EFFID_AIR_BUBBLE, tempLocation, 1, 0, ShadowMode);
                        break;
                    }
                }
            }
            else
                NewEffect(EFFID_AIR_BUBBLE, tempLocation, 1, 1, ShadowMode);
        }
    }
}

// checks if player A is in the mouth of the pet of any player on the screen
bool InOnscreenPet(int plr_A, const Screen_t& screen)
{
    const Player_t& p = Player[plr_A];

    // in the mouth of any player's Pet?
    if(p.Effect == PLREFF_PET_INSIDE)
    {
        // check it really is a player on this screen
        for(int plr_i = 0; plr_i < screen.player_count; plr_i++)
        {
            int B = screen.players[plr_i];

            if(p.Effect2 == B && Player[B].YoshiPlayer == plr_A)
                return true;
        }
    }

    return false;
}

// removes a player from a pet's mouth (so that their effect can be changed successfully)
void RemoveFromPet(int plr_A)
{
    Player_t& p = Player[plr_A];

    if(p.Effect == PLREFF_PET_INSIDE && p.Effect2 > 0 && p.Effect2 <= numPlayers && Player[p.Effect2].YoshiPlayer == plr_A)
        Player[p.Effect2].YoshiPlayer = 0;

    // set to no-collide mode with the other player (but this will be changed at the calling code)
    p.Effect = PLREFF_NO_COLLIDE;
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
           (p2.Effect == PLREFF_NORMAL || p2.Effect == PLREFF_WARP_PIPE) &&
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
                    if(!g_ClonedPlayerMode)
                        PlaySoundSpatial(SFX_Skid, p1.Location);
                    tempLocation = p1.Location;
                    p1.Location.SpeedX = p2.Location.SpeedX;
                    p2.Location.SpeedX = tempLocation.SpeedX;
                    p1.Bumped = true;
                    p2.Bumped = true;
                }
                else if(HitSpot == 1)
                {
                    if(!g_ClonedPlayerMode)
                        PlaySoundSpatial(SFX_Stomp, p1.Location);
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
                    NewEffect(EFFID_WHACK, newLoc(p1.Location.X + p1.Location.Width / 2.0 - 16, p1.Location.Y + p1.Location.Height - 16));
                }
                else if(HitSpot == 3)
                {
                    if(!g_ClonedPlayerMode)
                        PlaySoundSpatial(SFX_Stomp, p1.Location);
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
                    NewEffect(EFFID_WHACK, newLoc(p2.Location.X + p2.Location.Width / 2.0 - 16, p2.Location.Y + p2.Location.Height - 16));
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
        if(NPC[p.StandingOnNPC]->GrabFromTop)
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
                    if(NPC[p.StandingOnNPC]->IsAShell)
                        p.Location.SpeedX = NPC[p.StandingOnNPC].Location.SpeedX;
                    NPC[p.StandingOnNPC].HoldingPlayer = A;
                    NPC[p.StandingOnNPC].CantHurt = Physics.NPCCanHurtWait;
                    NPC[p.StandingOnNPC].CantHurtPlayer = A;
                    NPC[p.StandingOnNPC].Direction = p.Direction;
                    NPCFrames(p.StandingOnNPC);

                    if(NPC[p.StandingOnNPC].Type == NPCID_ITEM_BURIED)
                    {
                        p.Location.SpeedX += NPC[p.StandingOnNPC].Location.SpeedX;
                        NPC[p.StandingOnNPC].Direction = p.Direction;
                        NPC[p.StandingOnNPC].Generator = false;
                        NPC[p.StandingOnNPC].Frame = 0;
                        NPC[p.StandingOnNPC].Frame = EditorNPCFrame(NPC[p.StandingOnNPC].Type, NPC[p.StandingOnNPC].Direction);
                        NPC[p.StandingOnNPC].Type = NPCID(NPC[p.StandingOnNPC].Special);

                        if(NPC[p.StandingOnNPC].Type == NPCID_RANDOM_POWER)
                        {
                            NPC[p.StandingOnNPC].Type = RandomBonus();
                            NPC[p.StandingOnNPC].DefaultSpecial = NPC[p.StandingOnNPC].Type;
                        }

                        CharStuff(p.StandingOnNPC);
                        NPC[p.StandingOnNPC].Special = 0;

                        if(NPCIsYoshi(NPC[p.StandingOnNPC]))
                        {
                            NPC[p.StandingOnNPC].Special = NPC[p.StandingOnNPC].Type;
                            NPC[p.StandingOnNPC].Type = NPCID_ITEM_POD;
                        }

                        if(!(NPC[p.StandingOnNPC].Type == NPCID_CANNONENEMY || NPC[p.StandingOnNPC].Type == NPCID_CANNONITEM || NPC[p.StandingOnNPC].Type == NPCID_SPRING || NPC[p.StandingOnNPC].Type == NPCID_KEY || NPC[p.StandingOnNPC].Type == NPCID_COIN_SWITCH || NPC[p.StandingOnNPC].Type == NPCID_GRN_BOOT || NPC[p.StandingOnNPC].Type == NPCID_RED_BOOT || NPC[p.StandingOnNPC].Type == NPCID_BLU_BOOT || NPC[p.StandingOnNPC].Type == NPCID_TOOTHYPIPE || NPCIsAnExit(NPC[p.StandingOnNPC])))
                        {
                            if(!BattleMode)
                                NPC[p.StandingOnNPC].DefaultType = NPCID_NULL;
                        }

                        NPC[p.StandingOnNPC].Location.Height = NPC[p.StandingOnNPC]->THeight;
                        NPC[p.StandingOnNPC].Location.Width = NPC[p.StandingOnNPC]->TWidth;

                        if(NPC[p.StandingOnNPC].Type == NPCID_VEGGIE_RANDOM)
                        {
                            B = iRand(9);
                            NPC[p.StandingOnNPC].Type = NPCID(NPCID_VEGGIE_2 + B);
                            if(NPC[p.StandingOnNPC].Type == NPCID_VEGGIE_RANDOM)
                                NPC[p.StandingOnNPC].Type = NPCID_VEGGIE_1;
                            NPC[p.StandingOnNPC].Location.X += NPC[p.StandingOnNPC].Location.Width / 2.0;
                            NPC[p.StandingOnNPC].Location.Y += NPC[p.StandingOnNPC].Location.Height / 2.0;
                            NPC[p.StandingOnNPC].Location.Width = NPC[p.StandingOnNPC]->TWidth;
                            NPC[p.StandingOnNPC].Location.Height = NPC[p.StandingOnNPC]->THeight;
                            NPC[p.StandingOnNPC].Location.X += -NPC[p.StandingOnNPC].Location.Width / 2.0;
                            NPC[p.StandingOnNPC].Location.Y += -NPC[p.StandingOnNPC].Location.Height / 2.0;
                        }

                        NPCFrames(p.StandingOnNPC);

                        if(p.StandingOnNPC > 0 && p.StandingOnNPC <= numNPCs)
                        {
                            NPCQueues::Unchecked.push_back(p.StandingOnNPC);
                            treeNPCUpdate(p.StandingOnNPC);
                        }

                        p.StandingOnNPC = 0;
                    }
                }
                else
                {
                    if(p.GrabTime == 0)
                    {
                        if(NPC[p.StandingOnNPC].Type == NPCID_ITEM_BURIED || NPCIsVeggie(NPC[p.StandingOnNPC]))
                            PlaySoundSpatial(SFX_Grab2, p.Location);
                        else
                            PlaySoundSpatial(SFX_Grab, p.Location);
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
        if(NPC[p.HoldingNPC].Type == NPCID_FLY_BLOCK || NPC[p.HoldingNPC].Type == NPCID_FLY_CANNON)
        {
            if(p.Jump == 0)
                NPC[p.HoldingNPC].Special = 1;
            else if(p.Jump > 0 && NPC[p.HoldingNPC].Special == 1)
            {
                p.Jump = p.Jump * 1.5;
                NPC[p.HoldingNPC].Special = 0;
            }
            if(NPC[p.HoldingNPC].Type == NPCID_FLY_CANNON)
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

                        PlaySoundSpatial(SFX_FlameThrower, p.Location);

                    // For B = 1 To 3
                        numNPCs++;
                        NPC[numNPCs] = NPC_t();
                        NPC[numNPCs].CantHurt = 10000;
                        NPC[numNPCs].CantHurtPlayer = A;
                        NPC[numNPCs].BattleOwner = A;
                        NPC[numNPCs].Type = NPCID_PET_FIRE;
                        NPC[numNPCs].Location.Width = NPC[numNPCs]->TWidth;
                        NPC[numNPCs].Location.Height = NPC[numNPCs]->THeight;
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

        if(NPC[p.HoldingNPC].Type == NPCID_ICE_CUBE)
            NPC[p.HoldingNPC].Special3 = 0;
        NPC[p.HoldingNPC].TimeLeft = Physics.NPCTimeOffScreen;
        NPC[p.HoldingNPC].Effect = NPCEFF_NORMAL;
        NPC[p.HoldingNPC].CantHurt = Physics.NPCCanHurtWait;
        NPC[p.HoldingNPC].CantHurtPlayer = A;
        if(NPCIsVeggie(NPC[p.HoldingNPC]))
            NPC[p.HoldingNPC].CantHurt = 1000;

        if(p.Controls.Run || p.ForceHold > 0)
        {
            // fix a graphical bug where the NPC would stutter in the player's hands
            double use_w = (g_config.fix_visual_bugs) ? std::round(NPC[p.HoldingNPC].Location.Width) : NPC[p.HoldingNPC].Location.Width;

        // hold above head
            if(p.Character == 3 || p.Character == 4 || (p.Duck))
            {
                NPC[p.HoldingNPC].Bouce = true;
                NPC[p.HoldingNPC].Location.X = p.Location.X + p.Location.Width / 2.0 - use_w / 2.0;

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
                        if(NPC[p.HoldingNPC].Type == NPCID_PLR_FIREBALL || NPC[p.HoldingNPC].Type == NPCID_PLR_ICEBALL)
                        {
                            NPC[p.HoldingNPC].Location.X = p.Location.X + p.Location.Width / 2.0 - use_w / 2.0 + dRand() * 4 - 2;
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
                    NPC[p.HoldingNPC].Location.X = p.Location.X + p.Location.Width - Physics.PlayerGrabSpotX[p.Character][p.State] - use_w;

                NPC[p.HoldingNPC].Location.Y = p.Location.Y + Physics.PlayerGrabSpotY[p.Character][p.State] + 32 - NPC[p.HoldingNPC].Location.Height;
            }

            if(NPC[p.HoldingNPC].Type == NPCID_TOOTHYPIPE && !FreezeNPCs)
            {
                if(NPC[p.HoldingNPC].Special == 0)
                {
                    NPC[p.HoldingNPC].Special = 1;
                    NPC[p.HoldingNPC].Special2 = numNPCs + 1;
                    numNPCs++;
                    NPC[numNPCs] = NPC_t();
                    NPC[numNPCs].Active = true;
                    NPC[numNPCs].Section = p.Section;
                    NPC[numNPCs].TimeLeft = 100;
                    NPC[numNPCs].Type = NPCID_TOOTHY;
                    NPC[numNPCs].Location.Height = 32;
                    NPC[numNPCs].Location.Width = 48;
                    NPC[numNPCs].Special = A;
                    if(p.Direction > 0)
                        NPC[numNPCs].Frame = 2;
                    syncLayers_NPC(numNPCs);
                }

                for(int B : NPCQueues::Active.no_change)
                {
                    if(NPC[B].Type == NPCID_TOOTHY && NPC[B].Special == A)
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

            if(NPC[p.HoldingNPC].Type == NPCID_HEAVY_THROWER)
                NPCHit(p.HoldingNPC, 3, p.HoldingNPC);

            if(NPC[p.HoldingNPC]->IsACoin && !p.Controls.Down) // Smoke effect for coins
                NewEffect(EFFID_SMOKE_S3, NPC[p.HoldingNPC].Location);

            if(p.Controls.Up && !NPC[p.HoldingNPC]->IsACoin && NPC[p.HoldingNPC].Type != NPCID_BULLET) // Throw the npc up
            {
                if(NPC[p.HoldingNPC]->IsAShell || NPC[p.HoldingNPC].Type == NPCID_SLIDE_BLOCK || NPC[p.HoldingNPC].Type == NPCID_ICE_CUBE)
                {
                    if(p.Controls.Left || p.Controls.Right) // Up and forward
                    {
                        NPC[p.HoldingNPC].Location.SpeedX = Physics.NPCShellSpeed * p.Direction;
                        NPC[p.HoldingNPC].Location.SpeedY = -7;
                        tempLocation.Height = 0;
                        tempLocation.Width = 0;
                        tempLocation.Y = (p.Location.Y + NPC[p.HoldingNPC].Location.Y * 4) / 5;
                        tempLocation.X = (p.Location.X + NPC[p.HoldingNPC].Location.X * 4) / 5;
                        if(NPC[p.HoldingNPC].Type != NPCID_ICE_CUBE)
                            NewEffect(EFFID_STOMP_INIT, tempLocation);
                    }
                    else
                    {
                        NPC[p.HoldingNPC].Location.SpeedY = -Physics.NPCShellSpeedY;
                        tempLocation.Height = 0;
                        tempLocation.Width = 0;
                        tempLocation.Y = (p.Location.Y + NPC[p.HoldingNPC].Location.Y * 4) / 5;
                        tempLocation.X = (p.Location.X + NPC[p.HoldingNPC].Location.X * 4) / 5;
                        if(NPC[p.HoldingNPC].Type != NPCID_ICE_CUBE)
                            NewEffect(EFFID_STOMP_INIT, tempLocation);
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

                if(NPCIsVeggie(NPC[p.HoldingNPC]) || NPC[p.HoldingNPC].Type == NPCID_BLU_GUY || NPC[p.HoldingNPC].Type == NPCID_RED_GUY || NPC[p.HoldingNPC].Type == NPCID_JUMPER_S3 || NPC[p.HoldingNPC].Type == NPCID_BIRD || NPC[p.HoldingNPC].Type == NPCID_RED_SPIT_GUY || NPC[p.HoldingNPC].Type == NPCID_BLU_SPIT_GUY || NPC[p.HoldingNPC].Type == NPCID_GRY_SPIT_GUY || NPC[p.HoldingNPC].Type == NPCID_BOMB || NPC[p.HoldingNPC].Type == NPCID_WALK_BOMB_S2 || NPC[p.HoldingNPC].Type == NPCID_CARRY_BLOCK_A || NPC[p.HoldingNPC].Type == NPCID_CARRY_BLOCK_B || NPC[p.HoldingNPC].Type == NPCID_CARRY_BLOCK_C || NPC[p.HoldingNPC].Type == NPCID_CARRY_BLOCK_D || NPC[p.HoldingNPC].Type == NPCID_SPIT_BOSS_BALL || NPC[p.HoldingNPC].Type == NPCID_TIMER_S2 || NPC[p.HoldingNPC].Type == NPCID_PLR_FIREBALL || NPC[p.HoldingNPC].Type == NPCID_PLR_ICEBALL || NPC[p.HoldingNPC].Type == NPCID_DOOR_MAKER || NPC[p.HoldingNPC].Type == NPCID_CHAR3_HEAVY)
                    PlaySoundSpatial(SFX_Throw, p.Location);
                else
                    PlaySoundSpatial(SFX_ShellHit, p.Location);

                NPC[p.HoldingNPC].Projectile = true;
            }
            else if(p.Controls.Down && NPC[p.HoldingNPC].Type != NPCID_BULLET) // Drop
            {
                tempBool = false;
                if((p.Direction == 1 && p.Location.SpeedX > 3) || (p.Direction == -1 && p.Location.SpeedX < -3))
                    tempBool = true;

                if(tempBool && NPC[p.HoldingNPC].Type == NPCID_FLIPPED_RAINBOW_SHELL)
                {
                    p.Location.SpeedX = 0;
                    NPC[p.HoldingNPC].Location.SpeedX = Physics.NPCShellSpeed * p.Direction;
                    NPC[p.HoldingNPC].Projectile = true;
                    NPC[p.HoldingNPC].CantHurt = 0;
                    NPC[p.HoldingNPC].CantHurtPlayer = 0;
                    NPC[p.HoldingNPC].HoldingPlayer = 0;
                    PlaySoundSpatial(SFX_ShellHit, p.Location);
                    NewEffect(EFFID_WHIP, newLoc(NPC[p.HoldingNPC].Location.X, NPC[p.HoldingNPC].Location.Y + NPC[p.HoldingNPC].Location.Height - 16));
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
                    if(NPC[p.HoldingNPC].Type == NPCID_VINE_BUG)
                        NPC[p.HoldingNPC].Projectile = true;
                    if(p.StandingOnNPC != 0)
                        NPC[p.HoldingNPC].Location.Y += NPC[p.StandingOnNPC].Location.SpeedY;
                }
                if(NPC[p.HoldingNPC].Type == NPCID_PLR_FIREBALL || NPC[p.HoldingNPC].Type == NPCID_PLR_ICEBALL || NPC[p.HoldingNPC].Type == NPCID_CHAR3_HEAVY)
                {
                    NPC[p.HoldingNPC].Location.X = p.Location.X + p.Location.Width / 2.0 - NPC[p.HoldingNPC].Location.Width / 2.0;
                    if(p.State == 1)
                        NPC[p.HoldingNPC].Location.Y = p.Location.Y - NPC[p.HoldingNPC].Location.Height;
                    else
                        NPC[p.HoldingNPC].Location.Y = p.Location.Y - NPC[p.HoldingNPC].Location.Height + 6;
                    PlaySoundSpatial(SFX_Throw, p.Location);
                    NPC[p.HoldingNPC].Location.SpeedX = 0;
                    NPC[p.HoldingNPC].Location.SpeedY = 20;
                }
            }
            else if(!NPC[p.HoldingNPC]->IsAShell &&
                    NPC[p.HoldingNPC].Type != NPCID_SLIDE_BLOCK &&
                    NPC[p.HoldingNPC].Type != NPCID_ICE_CUBE &&
                    !NPC[p.HoldingNPC]->IsACoin) // if not a shell or a coin the kick it up and forward
            {
            // peach
                if(p.Character == 3)
                {
                    if(NPC[p.HoldingNPC].Type == NPCID_PLR_FIREBALL || NPC[p.HoldingNPC].Type == NPCID_PLR_ICEBALL || (p.Location.SpeedY != 0 && p.StandingOnNPC == 0 && p.Slope == 0))
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
                    if(NPC[p.HoldingNPC].Type == NPCID_PLR_FIREBALL || NPC[p.HoldingNPC].Type == NPCID_PLR_ICEBALL || (p.Location.SpeedY != 0 && p.StandingOnNPC == 0 && p.Slope == 0))
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
                if(NPCIsVeggie(NPC[p.HoldingNPC]) || NPC[p.HoldingNPC].Type == NPCID_BLU_GUY || NPC[p.HoldingNPC].Type == NPCID_RED_GUY || NPC[p.HoldingNPC].Type == NPCID_JUMPER_S3 || NPC[p.HoldingNPC].Type == NPCID_BIRD || NPC[p.HoldingNPC].Type == NPCID_RED_SPIT_GUY || NPC[p.HoldingNPC].Type == NPCID_BLU_SPIT_GUY || NPC[p.HoldingNPC].Type == NPCID_GRY_SPIT_GUY || NPC[p.HoldingNPC].Type == NPCID_BOMB || NPC[p.HoldingNPC].Type == NPCID_WALK_BOMB_S2 || NPC[p.HoldingNPC].Type == NPCID_CARRY_BLOCK_A || NPC[p.HoldingNPC].Type == NPCID_CARRY_BLOCK_B || NPC[p.HoldingNPC].Type == NPCID_CARRY_BLOCK_C || NPC[p.HoldingNPC].Type == NPCID_CARRY_BLOCK_D || NPC[p.HoldingNPC].Type == NPCID_SPIT_BOSS_BALL || NPC[p.HoldingNPC].Type == NPCID_TIMER_S2 || NPC[p.HoldingNPC].Type == NPCID_PLR_FIREBALL || NPC[p.HoldingNPC].Type == NPCID_PLR_ICEBALL || NPC[p.HoldingNPC].Type == NPCID_DOOR_MAKER || NPC[p.HoldingNPC].Type == NPCID_CHAR3_HEAVY)
                    PlaySoundSpatial(SFX_Throw, p.Location);
                else if(NPC[p.HoldingNPC].Type == NPCID_BULLET)
                    PlaySoundSpatial(SFX_Bullet, p.Location);
                else
                    PlaySoundSpatial(SFX_ShellHit, p.Location);
            }
            else if(NPC[p.HoldingNPC]->IsAShell)
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
                NewEffect(EFFID_STOMP_INIT, tempLocation);
            }
            else if(NPC[p.HoldingNPC].Type == NPCID_ICE_CUBE)
            {
                PlaySoundSpatial(SFX_ShellHit, p.Location);
                NPC[p.HoldingNPC].Location.SpeedX = Physics.NPCShellSpeed * p.Direction;
                NPC[p.HoldingNPC].CantHurt = Physics.NPCCanHurtWait;
                NPC[p.HoldingNPC].CantHurtPlayer = A;
                NPC[p.HoldingNPC].Projectile = true;
            }
            if(NPC[p.HoldingNPC].Type == NPCID_BOMB && NPC[p.HoldingNPC].Location.SpeedX != 0)
            {
                NPC[p.HoldingNPC].Location.SpeedX += p.Location.SpeedX * 0.5;
                if(p.StandingOnNPC != 0)
                    NPC[p.HoldingNPC].Location.SpeedX += NPC[p.StandingOnNPC].Location.SpeedX;
            }
            if(NPC[p.HoldingNPC].Type == NPCID_PLR_FIREBALL && NPC[p.HoldingNPC].Special == 4) // give toad fireballs a little spunk
            {
                if(NPC[p.HoldingNPC].Location.SpeedY < 0)
                    NPC[p.HoldingNPC].Location.SpeedY = NPC[p.HoldingNPC].Location.SpeedY * 1.1;
            }
            if(NPC[p.HoldingNPC].Type == NPCID_CHAR3_HEAVY)
            {
                if(p.Location.SpeedX != 0 && NPC[p.HoldingNPC].Location.SpeedX != 0)
                    NPC[p.HoldingNPC].Location.SpeedX += p.Location.SpeedX * 0.5;
            }

            // this block was misleadingly left-indented in VB6, but its nesting in C++ is accurate to its nesting in VB6, and now I'm fixing the indentation to match the nesting -- ds-sloth
            if(NPC[p.HoldingNPC].Type == NPCID_CHAR4_HEAVY)
            {
                NPC[p.HoldingNPC].Special5 = A;
                NPC[p.HoldingNPC].Special4 = p.Direction; // Special6 in SMBX 1.3
                NPC[p.HoldingNPC].Location.SpeedY = -8;
                NPC[p.HoldingNPC].Location.SpeedX = 12 * p.Direction + p.Location.SpeedX;
                NPC[p.HoldingNPC].Projectile = true;
            }


            NPC[p.HoldingNPC].HoldingPlayer = 0;
            p.HoldingNPC = 0;
        }

        if(LayerNPC > 0 && LayerNPC <= numNPCs && (NPC[LayerNPC].Location.X != lyrX || NPC[LayerNPC].Location.Y != lyrY))
            treeNPCUpdate(LayerNPC);
    }

    if(LayerNPC > 0)
    {
        int B = NPC[LayerNPC].AttLayer;
        if(B != LAYER_NONE && B != LAYER_DEFAULT)
            SetLayerSpeed(B, NPC[LayerNPC].Location.X - lyrX, NPC[LayerNPC].Location.Y - lyrY, false);
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
                    NewEffect(EFFID_SKID_DUST, tempLocation, 1, 0, ShadowMode);
                }
            }
        }
        return;
    }

    if(!LevelSelect && p.Effect == PLREFF_NORMAL && p.FireBallCD == 0)
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
                PlaySoundSpatial(SFX_HeroDash, p.Location);
                p.SlideCounter = 2 + iRand_round(2);
                tempLocation.Y = p.Location.Y + p.Location.Height - 4;

                if(p.Location.SpeedX < 0)
                    tempLocation.X = p.Location.X + p.Location.Width / 2.0 - 6 - 4;
                else
                    tempLocation.X = p.Location.X + p.Location.Width / 2.0 + 6 - 4;

                NewEffect(EFFID_SKID_DUST, tempLocation, 1, 0, ShadowMode);
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

    if(p.Effect != PLREFF_WAITING && p.Fairy)
    {
        p.Fairy = false;
        SizeCheck(Player[A]);
    }

    p.TailCount = 0;
    // p.Pinched1 = 0;
    // p.Pinched2 = 0;
    // p.Pinched3 = 0;
    // p.Pinched4 = 0;
    // p.NPCPinched = 0;
    p.Pinched.reset_non_strict();
    p.SwordPoke = 0;

    if(!p.YoshiBlue && p.Effect != PLREFF_STONE)
    {
        p.CanFly = false;
        p.CanFly2 = false;
        p.RunCount = 0;
    }

    p.Immune2 = false;

    // in shared screen mode, give wings to players who are above the screen waiting to exit a warp
    if(PlayerWaitingInWarp(p))
    {
        Screen_t& s = ScreenByPlayer(A);
        if(s.Type == ScreenTypes::SharedScreen && (p.Location.Y + p.Location.Height < -vScreenByPlayer(A).Y) && CheckNearestLiving(A))
        {
            SharedScreenAvoidJump_Pre(s);
            p.Dead = true;
            p.Effect = PLREFF_COOP_WINGS;
            p.Effect2 = 0;
            SizeCheck(p);
            SharedScreenAvoidJump_Post(s, 0);
            return;
        }
    }

    if(p.Effect == PLREFF_TURN_BIG) // Player growing effect
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
            p.Effect = PLREFF_NORMAL;
            p.Effect2 = 0;
            p.StandUp = true;
        }
    }
    else if(p.Effect == PLREFF_TURN_SMALL) // Player shrinking effect
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
            p.Effect = PLREFF_NORMAL;
            p.Effect2 = 0;
            // If numPlayers <= 2 Then DropBonus A
        }
    }
    // logic combined across all powerup-to-big effects
    else if(p.Effect == PLREFF_FIRE_TO_BIG || p.Effect == PLREFF_ICE_TO_BIG)
    {
        int prev_state = 7;

        if(p.Effect == PLREFF_FIRE_TO_BIG)
            prev_state = 3;

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
                p.State = prev_state;
            else
                p.State = 2;
        }

        if(p.Effect2 >= 50)
        {
            if(p.State == prev_state)
                p.State = 2;

            p.Immune = 150;
            p.Immune2 = true;
            p.Effect = PLREFF_NORMAL;
            p.Effect2 = 0;
            // If numPlayers <= 2 Then DropBonus A
        }
    }
#if 0
    else if(p.Effect == PLREFF_FIRE_TO_BIG) // Player losing firepower
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
            p.Effect = PLREFF_NORMAL;
            p.Effect2 = 0;
            // If numPlayers <= 2 Then DropBonus A
        }
    }
    else if(p.Effect == PLREFF_ICE_TO_BIG) // Player losing icepower
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
            p.Effect = PLREFF_NORMAL;
            p.Effect2 = 0;
            // If numPlayers <= 2 Then DropBonus A
        }
    }
#endif
    else if(p.Effect == PLREFF_WARP_PIPE) // Warp effect
        PlayerEffectWarpPipe(A);
    else if(p.Effect == PLREFF_WARP_DOOR) // Door effect
        PlayerEffectWarpDoor(A);
    else if(p.Effect == PLREFF_WAITING) // Holding Pattern
    {
        // tracking a player that got an exit
        if(p.Effect2 < 0)
        {
            p.Location.X = Player[-p.Effect2].Location.X;
            p.Location.Y = Player[-p.Effect2].Location.Y;

            if(Player[-p.Effect2].Dead)
                p.Dead = true;
        }
        // temporary immunity and invisibility
        else if(p.Effect2 <= 30)
        {
            p.Effect2 -= 1;
            if(p.Effect2 == 0.0)
            {
                p.Effect = PLREFF_NORMAL;
                p.Effect2 = 0;
            }
        }
        else
            PlayerEffectWarpWait(A);
    }
    // logic combined across all "grow" powerup effects
    else if(p.Effect == PLREFF_TURN_FIRE || p.Effect == PLREFF_TURN_ICE) // Player got fire power
    {
        int target_state = 3;

        if(p.Effect == PLREFF_TURN_ICE)
            target_state = 7;

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
            else if(p.State != target_state)
                p.State = target_state;
            else
                p.State = 2;
        }

        if(p.Effect2 >= 50)
        {
            if(p.State == 2)
                p.State = target_state;

            p.Immune += 50;
            p.Immune2 = true;
            p.Effect = PLREFF_NORMAL;
            p.Effect2 = 0;
            p.StandUp = true;
        }
    }
#if 0
    else if(p.Effect == PLREFF_TURN_FIRE) // Player got fire power
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
            p.Effect = PLREFF_NORMAL;
            p.Effect2 = 0;
            p.StandUp = true;
        }
    }
    else if(p.Effect == PLREFF_TURN_ICE) // Player got ice power
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
            p.Effect = PLREFF_NORMAL;
            p.Effect2 = 0;
            p.StandUp = true;
        }
    }
#endif
    // logic combined across all "transform" powerup effects
    else if(p.Effect == PLREFF_TURN_LEAF || p.Effect == PLREFF_TURN_STATUE || p.Effect == PLREFF_TURN_HEAVY)
    {
        int target_state = 4;

        if(p.Effect != PLREFF_TURN_LEAF)
        {
            p.Immune2 = true;
            target_state = 5 + (p.Effect - PLREFF_TURN_STATUE);
        }

        p.Frame = 1;

        if(p.Effect2 == 0.0)
        {
            if(p.State == 1 && p.Mount == 0)
            {
                p.Location.X += -Physics.PlayerWidth[p.Character][2] * 0.5 + Physics.PlayerWidth[p.Character][1] * 0.5;
                p.Location.Y += -Physics.PlayerHeight[p.Character][2] + Physics.PlayerHeight[p.Character][1];
                p.State = target_state;
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
                p.Location.Height = Physics.PlayerHeight[p.Character][target_state];
            }

            p.State = target_state;
            tempLocation.Width = 32;
            tempLocation.Height = 32;
            tempLocation.X = p.Location.X + p.Location.Width / 2.0 - tempLocation.Width / 2.0;
            tempLocation.Y = p.Location.Y + p.Location.Height / 2.0 - tempLocation.Height / 2.0;
            NewEffect(EFFID_SMOKE_S4, tempLocation, 1, 0, ShadowMode);
        }

        p.Effect2 += 1;

        if(fEqual(p.Effect2, 14))
        {
            p.Immune += 50;
            p.Immune2 = true;
            p.Effect = PLREFF_NORMAL;
            p.Effect2 = 0;
            p.StandUp = true;
        }
    }
#if 0
    else if(p.Effect == PLREFF_TURN_LEAF) // Player got a leaf
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
            NewEffect(EFFID_SMOKE_S4, tempLocation, 1, 0, ShadowMode);
        }

        p.Effect2 += 1;

        if(fEqual(p.Effect2, 14))
        {
            p.Immune += 50;
            p.Immune2 = true;
            p.Effect = PLREFF_NORMAL;
            p.Effect2 = 0;
            p.StandUp = true;
        }
    }
    else if(p.Effect == PLREFF_TURN_STATUE) // Player got a tanooki suit
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
                p.Location.Height = Physics.PlayerHeight[p.Character][5]; // was 4 in SMBX 1.3, but the value was the same for all characters
            }

            p.State = 5;
            tempLocation.Width = 32;
            tempLocation.Height = 32;
            tempLocation.X = p.Location.X + p.Location.Width / 2.0 - tempLocation.Width / 2.0;
            tempLocation.Y = p.Location.Y + p.Location.Height / 2.0 - tempLocation.Height / 2.0;
            NewEffect(EFFID_SMOKE_S4, tempLocation, 1, 0, ShadowMode);
        }

        p.Effect2 += 1;

        if(fEqual(p.Effect2, 14))
        {
            p.Immune += 50;
            p.Immune2 = true;
            p.Effect = PLREFF_NORMAL;
            p.Effect2 = 0;
            p.StandUp = true;
        }
    }
    else if(p.Effect == PLREFF_TURN_HEAVY) // Player got a hammer suit
    {
        p.Frame = 1;
        p.Immune2 = true;

        if(p.Effect2 == 0.0)
        {
            if(p.State == 1 && p.Mount == 0)
            {
                p.Location.X += -Physics.PlayerWidth[p.Character][2] * 0.5 + Physics.PlayerWidth[p.Character][1] * 0.5;
                p.Location.Y += -Physics.PlayerHeight[p.Character][2] + Physics.PlayerHeight[p.Character][1];
                p.State = 6; // was 5 in SMBX 1.3, but this is fine because the dimensions are the same, and it gets set correctly below
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
            NewEffect(EFFID_SMOKE_S4, tempLocation, 1, 0, ShadowMode);
        }

        p.Effect2 += 1;

        if(p.Effect2 == 14.0)
        {
            p.Immune += 50;
            p.Immune2 = true;
            p.Effect = PLREFF_NORMAL;
            p.Effect2 = 0;
            p.StandUp = true;
        }
    }
#endif
    else if(p.Effect == PLREFF_STONE) // Change to / from tanooki
    {
        for(B = 1; B <= 2; B++)
        {
            NewEffect(EFFID_SPARKLE, newLoc(p.Location.X + dRand() * (p.Location.Width + 8) - 8,
                                 p.Location.Y + dRand() * (p.Location.Height + 8) - 4), 1, 0, ShadowMode);
            Effect[numEffects].Location.SpeedX = dRand() * 2 - 1;
            Effect[numEffects].Location.SpeedY = dRand() * 2 - 1;
        }

        if(p.Effect2 == 0.0)
        {
            UnDuck(Player[A]);
            PlaySoundSpatial(SFX_Transform, p.Location);
            tempLocation.Width = 32;
            tempLocation.Height = 32;
            tempLocation.X = p.Location.X + p.Location.Width / 2.0 - tempLocation.Width / 2.0;
            tempLocation.Y = p.Location.Y + p.Location.Height / 2.0 - tempLocation.Height / 2.0;
            NewEffect(EFFID_SMOKE_S3, tempLocation, 1, 0, ShadowMode);

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
            p.Effect = PLREFF_NORMAL;
            p.Immune = 0;
            p.Immune2 = false;
        }
    }
    else if(p.Effect == PLREFF_NO_COLLIDE) // MultiMario
    {
        if(p.HoldingNPC > numNPCs) // Can't hold an NPC that is dead
            p.HoldingNPC = 0;

        if(p.HoldingNPC > 0)
        {
            NPC[p.HoldingNPC].Effect = NPCEFF_NORMAL;
            NPC[p.HoldingNPC].CantHurt = Physics.NPCCanHurtWait;
            NPC[p.HoldingNPC].CantHurtPlayer = A;

            if(p.Direction > 0)
                NPC[p.HoldingNPC].Location.X = p.Location.X + Physics.PlayerGrabSpotX[p.Character][p.State];
            else
                NPC[p.HoldingNPC].Location.X = p.Location.X + p.Location.Width - Physics.PlayerGrabSpotX[p.Character][p.State] - NPC[p.HoldingNPC].Location.Width;

            NPC[p.HoldingNPC].Location.Y = p.Location.Y + Physics.PlayerGrabSpotY[p.Character][p.State] + 32 - NPC[p.HoldingNPC].Location.Height;

            if(p.HoldingNPC <= numNPCs)
                treeNPCUpdate(p.HoldingNPC);
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
            if(B != A && (Player[B].Effect == PLREFF_NORMAL || fEqual(B, p.Effect2)) && !Player[B].Dead && Player[B].TimeToLive == 0 && CheckCollision(p.Location, Player[B].Location))
                tempBool = false;
        }

        if(tempBool)
        {
            p.Effect = PLREFF_NORMAL;
            p.Effect2 = 0;
            p.Immune = 0;
            p.Immune2 = false;
            p.Location.SpeedY = 0.01;
        }
        else if(p.Effect2 > 0)
        {
            D = float(p.Effect2);

            if(Player[D].Effect == PLREFF_NORMAL)
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
    else if(p.Effect == PLREFF_PET_INSIDE) // Yoshi swallow
    {
        p.HoldingNPC = 0;
        p.StandingOnNPC = 0;
        p.Section = Player[p.Effect2].Section;
        p.Location.X = Player[p.Effect2].Location.X + Player[p.Effect2].Location.Width / 2.0 - p.Location.Width / 2.0;
        p.Location.Y = Player[p.Effect2].Location.Y + Player[p.Effect2].Location.Height / 2.0 - p.Location.Height / 2.0;

        if(Player[p.Effect2].YoshiPlayer != A)
        {
            p.Effect = PLREFF_NORMAL;
            p.Effect2 = 0;
        }
    }
    else if(p.Effect == PLREFF_RESPAWN) // player stole a heldbonus
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
                // !Player[B].Dead condition was added to prevent confusing Drop/Add cases where player gets locked in immune state
                if(B != A && Player[B].Effect != PLREFF_RESPAWN && ((!g_config.allow_drop_add && (numPlayers < 3 || g_ClonedPlayerMode)) || (!Player[B].Dead && Player[B].Effect != PLREFF_PET_INSIDE)) && CheckCollision(p.Location, Player[B].Location))
                    tempBool = false;
            }
            if(tempBool)
            {
                p.Effect = PLREFF_NORMAL;
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
                    p.Effect = PLREFF_NORMAL;
                    p.Immune = 50;
                    p.Immune2 = false;
                    p.Location.Y = Player[B].Location.Y - p.Location.Height;
                    p.Location.SpeedY = 0.01;
                }
            }
        }
    }

    if(p.Mount == 3 && p.Effect != PLREFF_NO_COLLIDE)
        PlayerFrame(p);

//    if(Player[A].Effect == 0)
//    {
//        if(nPlay.Online && A == nPlay.MySlot + 1)
//            Netplay::sendData Netplay::PutPlayerControls(nPlay.MySlot) + "1c" + std::to_string(A) + "|" + Player[A].Effect + "|" + Player[A].Effect2 + LB + "1h" + std::to_string(A) + "|" + Player[A].State + LB;
//    }
}

bool PlayerNormal(const Player_t& p)
{
    return p.Effect == PLREFF_NORMAL || p.Effect == PLREFF_WARP_PIPE || p.Effect == PLREFF_NO_COLLIDE || p.Effect == PLREFF_PET_INSIDE || p.Effect == PLREFF_COOP_WINGS;
}

bool AllPlayersNormal()
{
    for(int B = 1; B <= numPlayers; B++)
    {
        if(!PlayerNormal(Player[B]))
            return false;
    }

    return true;
}

// NEW: ensures the players on a screen are nearby if the screen is shared
void PlayersEnsureNearby(const Screen_t& screen)
{
    if(screen.Type != ScreenTypes::SharedScreen)
        return;

    // get extreme bounds on screen players
    double l = 0.0;
    double r = 0.0;
    double t = 0.0;
    double b = 0.0;

    // also get center of alive screen players
    double cx = 0.0;
    double cy = 0.0;
    int c_count = 0;

    // if a player is currently warping, always re-calculate position, and prefer to center players on another player
    bool exists_warping = false;
    bool exists_non_warping = false;

    for(int plr_i = 0; plr_i < screen.player_count; plr_i++)
    {
        int plr_A = screen.players[plr_i];
        const Player_t& p = Player[plr_A];
        const Location_t& pLoc = p.Location;

        double p_x = pLoc.X + pLoc.Width / 2;
        double p_top = (p.Effect == PLREFF_RESPAWN) ? p.Effect2 : pLoc.Y;
        double p_y = p_top + pLoc.Height / 2;

        // dead players don't affect camera
        if(p.Dead)
            continue;

        if(plr_i == 0 || p_x < l)
            l = p_x;

        if(plr_i == 0 || p_x > r)
            r = p_x;

        if(plr_i == 0 || p_y < t)
            t = p_y;

        if(plr_i == 0 || p_y > b)
            b = p_y;

        if(!p.Dead && p.TimeToLive == 0)
        {
            cx += p_x;
            cy += p_y;
            c_count += 1;

            if(p.Effect == PLREFF_WARP_PIPE || p.Effect == PLREFF_WARP_DOOR)
                exists_warping = true;
            else
                exists_non_warping = true;
        }
    }

    // no problem if players are nearby, just return
    if(!exists_warping && r - l <= screen.W && b - t <= screen.H)
        return;

    // need to calculate center and figure out which player is closest
    if(c_count > 0)
    {
        cx /= c_count;
        cy /= c_count;
    }
    else
    {
        cx = (l + r) / 2;
        cy = (t + b) / 2;
    }

    // figure out which player is closest
    int    closest      = 0;
    double closest_dist = 0;

    for(int plr_i = 0; plr_i < screen.player_count; plr_i++)
    {
        int plr_A = screen.players[plr_i];
        const Player_t& p = Player[plr_A];
        const Location_t& pLoc = p.Location;

        // prefer non-warping player
        if(exists_non_warping && (p.Effect == PLREFF_WARP_PIPE || p.Effect == PLREFF_WARP_DOOR))
            continue;

        if(!p.Dead && p.TimeToLive == 0)
        {
            double p_top = (p.Effect == PLREFF_RESPAWN) ? p.Effect2 : pLoc.Y;
            double dist = (pLoc.X - cx) * (pLoc.X - cx) + (p_top - cy) * (p_top - cy);

            if(closest == 0 || closest_dist > dist)
            {
                closest = plr_A;
                closest_dist = dist;
            }
        }
    }

    // if all players dead, choose randomly
    if(closest == 0)
        closest = screen.players[0];

    Player_t& pClosest = Player[closest];
    const Location_t& pClosestLoc = pClosest.Location;

    // if the winner is currently respawning, place them at their target loc
    if(pClosest.Effect == PLREFF_RESPAWN)
        pClosest.Location.Y = pClosest.Effect2;

    // move all players to winning player's location, and set effect if alive
    for(int plr_i = 0; plr_i < screen.player_count; plr_i++)
    {
        int plr_A = screen.players[plr_i];

        if(plr_A == closest)
            continue;

        Player_t& p = Player[plr_A];

        // winged player will naturally return to screen
        if(p.Effect == PLREFF_COOP_WINGS)
            continue;

        Location_t& pLoc = p.Location;

        p.Section = pClosest.Section;

        pLoc.X = pClosestLoc.X + pClosestLoc.Width / 2 - pLoc.Width / 2;
        pLoc.Y = pClosestLoc.Y + pClosestLoc.Height - pLoc.Height;
    }

    bool in_door_scroll = PlayerScrollingInWarp(pClosest);

    for(int plr_i = 0; plr_i < screen.player_count; plr_i++)
    {
        int plr_A = screen.players[plr_i];

        Player_t& p = Player[plr_A];

        if(p.Dead || p.TimeToLive != 0)
            continue;

        if(in_door_scroll)
        {
            p.Effect = pClosest.Effect;
            p.Effect2 = pClosest.Effect2;
            p.Warp = pClosest.Warp;
            p.WarpCD = pClosest.WarpCD;
            p.WarpBackward = pClosest.WarpBackward;
        }
        else
        {
            p.Effect = PLREFF_NORMAL;
            p.Effect2 = 0;
            p.Warp = 0;
            p.WarpCD = 0;
            p.WarpBackward = false;
            p.WarpShooted = false;

            DodgePlayers(plr_A);
        }
    }
}

void KeyholeCheck(const int A, const Location_t& loc)
{
    UNUSED(A);

    for(int B : treeBackgroundQuery(loc, SORTMODE_NONE))
    {
        if(B > numBackground)
            continue;

        if(Background[B].Type == 35)
        {
            SpeedlessLocation_t tempLocation = Background[B].Location;
            tempLocation.Width = 16;
            tempLocation.X += 8;
            tempLocation.Height = 26;
            tempLocation.Y += 2;

            if(CheckCollision(loc, tempLocation))
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

// make a death effect for player and release all items linked to them.
// used for the Die SwapCharacter and for the DropPlayer.
// do this BEFORE changing/erasing any player fields
void PlayerGone(const int A)
{
    PlaySoundSpatial(SFX_PlayerDied2, Player[A].Location);
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

void AddPlayer(int Character, Screen_t& screen)
{
    numPlayers++;

    // add player to screen
    Screens_AssignPlayer(numPlayers, screen);

    Player_t& p = Player[numPlayers];

    p = Player_t();
    p.Character = Character;
    p.State = SavedChar[p.Character].State;
    p.HeldBonus = NPCID(SavedChar[p.Character].HeldBonus);
    p.Mount = SavedChar[p.Character].Mount;
    p.MountType = SavedChar[p.Character].MountType;
    p.Hearts = SavedChar[p.Character].Hearts;

    if(p.State == 0)
        p.State = 1;

    p.Frame = 1;
    if(p.Character == 3)
        p.CanFloat = true;
    p.Direction = 1.;

    SizeCheck(Player[numPlayers]);

    // the rest only matters during level play
    if(LevelSelect)
        return;

    int alivePlayer = CheckLiving();
    if(alivePlayer == 0 || alivePlayer == numPlayers)
        alivePlayer = 1;

    p.Section = Player[alivePlayer].Section;
    RespawnPlayerTo(numPlayers, alivePlayer);

    // the rest only matters during level play
    if(LevelSelect)
        return;

    SetupScreens();
    PlayersEnsureNearby(screen);
}

void DropPlayer(const int A)
{
    if(A < 1 || A > numPlayers)
        return;

    const Screen_t& screen = ScreenByPlayer(A);

    // in levels, make a death effect (and leave behind mount)
    if(!LevelSelect)
        PlayerGone(A);

    // IMPORTANT - removes all references to player A,
    //   decrements all references to higher players

    // NPC player references
    for(int C = 1; C <= numNPCs; C++)
    {
        NPC_t& n = NPC[C];

        // most of these should not be equal because PlayerGone has already been called.
        if(n.vehiclePlr > A && n.vehiclePlr <= numPlayers)
            n.vehiclePlr --;
        else if(n.vehiclePlr == A)
            n.vehiclePlr = 0;

        if(n.HoldingPlayer > A && n.HoldingPlayer <= numPlayers)
            n.HoldingPlayer --;
        else if(n.HoldingPlayer == A)
            n.HoldingPlayer = 0;

        if(n.CantHurtPlayer > A && n.CantHurtPlayer <= numPlayers)
            n.CantHurtPlayer --;
        else if(n.CantHurtPlayer == A)
            n.CantHurtPlayer = 0;

        if(n.BattleOwner > A && n.BattleOwner <= numPlayers)
            n.BattleOwner --;
        else if(n.BattleOwner == A)
            n.BattleOwner = 0;

        // this is not quite right (the vScreen index doesn't necessarily equal the player index, and might not get removed)
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

        if(Player[B].StandingOnVehiclePlr == A)
            Player[B].StandingOnVehiclePlr = 0;
        else if(Player[B].StandingOnVehiclePlr > A)
            Player[B].StandingOnVehiclePlr --;
    }

    // saves player without their mount, but mount is still onscreen and available
    SavedChar[Player[A].Character] = Player[A];
    for(int B = A; B < numPlayers; B++)
    {
        Player[B] = std::move(Player[B + 1]);
        OwedMount[B] = OwedMount[B + 1];
        OwedMountType[B] = OwedMountType[B + 1];
        BattleLives[B] = BattleLives[B + 1];
    }

    numPlayers --;

    // remove player from screens
    Screens_DropPlayer(A);

    // the rest only matters during level play
    if(LevelSelect)
        return;

    SetupScreens();
    PlayersEnsureNearby(screen);
}

void SwapCharacter(int A, int Character, bool FromBlock)
{
    SavedChar[Player[A].Character] = Player[A];

    // the following is identical to the code moved from blocks.cpp
    Player[A].Character = Character;
    auto &p = Player[A];
    p.State = SavedChar[p.Character].State;
    p.HeldBonus = NPCID(SavedChar[p.Character].HeldBonus);
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
        p.Effect = PLREFF_WAITING;
        p.Effect2 = 14;
    }
    // NEW CODE that plays same role as old call to SetupPlayers() in the original world map char swap code
    else
    {
        p.Frame = 1;
        p.FrameCount = 0;

        if(LevelSelect)
            p.Immune = 0;
    }

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
        double saved_respawn_StopY = 0;
        if(p.Effect == PLREFF_RESPAWN)
            saved_respawn_StopY = p.Effect2 + p.Location.Height;

        // make player bottom match old player bottom, to avoid floor glitches
        UnDuck(Player[A]);
        SizeCheck(Player[A]);

        // if player effect is 6 (respawn downwards), update target similarly
        if(p.Effect == PLREFF_RESPAWN)
            p.Effect2 = saved_respawn_StopY - p.Location.Height;
    }

    if(!LevelSelect)
    {
        Location_t tempLocation = p.Location;
        tempLocation.Y = p.Location.Y + p.Location.Height / 2.0 - 16;
        tempLocation.X = p.Location.X + p.Location.Width / 2.0 - 16;
        NewEffect(EFFID_SMOKE_S3, tempLocation);
    }

    UpdateYoshiMusic();
}

// returns whether a player is allowed to swap characters
bool SwapCharAllowed()
{
    if(LevelSelect || GameMenu || (g_config.allow_drop_add && IsHubLevel))
        return true;
    else
        return false;
}
