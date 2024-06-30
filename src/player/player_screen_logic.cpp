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

#include "globals.h"
#include "player.h"
#include "graphics.h"
#include "screen.h"
#include "config.h"

#include "main/game_globals.h"

void PlayerSharedScreenLogic(int A)
{
    const Screen_t& screen = ScreenByPlayer(A);

    // if(!LevelWrap[Player[A].Section] && LevelMacro == LEVELMACRO_OFF)
    // This section is fully new logic
    if(screen.Type == ScreenTypes::SharedScreen)
    {
        Player_t& p = Player[A];
        const SpeedlessLocation_t& section = level[p.Section];

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
        for(int o_p_i = 0; o_p_i < screen.player_count; o_p_i++)
        {
            const Player_t& o_p = Player[screen.players[o_p_i]];

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
}

void PlayerLevelWrapLogic(int A)
{
    const Screen_t& screen = ScreenByPlayer(A);
    vScreen_t& vscreen = vScreenByPlayer(A);
    Location_t& pLoc = Player[A].Location;
    const SpeedlessLocation_t& section = level[Player[A].Section];

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
                o_p.Effect = PLREFF_RESPAWN;
                o_p.Effect2 = o_p.Location.Y;
            }
        }

        GetvScreenAuto(vscreen);
    }
}

void PlayerOffscreenExitCheck(int A)
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
}
