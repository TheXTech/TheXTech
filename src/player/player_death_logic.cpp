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

#include "globals.h"
#include "player.h"
#include "config.h"
#include "message.h"
#include "script/luna/lunacounter.h"

// updates the position of a player that has just died
void UpdatePlayerTimeToLive(int A)
{
    Player[A].TimeToLive += 1;

    if(Player[A].TimeToLive == 50 && !g_ClonedPlayerMode)
        gDeathCounter.MarkDeath();

    const Screen_t& screen = ScreenByPlayer(A);
    bool dynamic_screen = (screen.Type == ScreenTypes::Dynamic);
    bool shared_screen = (screen.Type == ScreenTypes::SharedScreen);
    bool split_screen = (screen.active_end() - screen.active_begin() > 1);
    bool normal_multiplayer = (dynamic_screen || shared_screen || split_screen || XMessage::GetStatus() != XMessage::Status::local);

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
    else if((!BattleMode || B == 0) && player_timer_done) // ScreenType = 1
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

// updates the position of a player that is fully dead
void UpdatePlayerDead(int A)
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
