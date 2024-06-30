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
#include "screen.h"

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
