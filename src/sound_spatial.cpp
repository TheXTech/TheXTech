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

#include "screen.h"
#include "sound.h"

void Sound_ResolveSpatialMod(uint8_t& left, uint8_t& right, int l, int t, int r, int b)
{
    const Screen_t& screen = *l_screen;

    left = 0;
    right = 0;

    for(int vscreen_i = screen.active_begin(); vscreen_i != screen.active_end(); vscreen_i++)
    {
        const vScreen_t& vscreen = vScreen[screen.vScreen_refs[vscreen_i]];

        int vscreen_l = -vscreen.X;
        int vscreen_r = -vscreen.X + vscreen.Width;
        int vscreen_t = -vscreen.Y;
        int vscreen_b = -vscreen.Y + vscreen.Height;

        int l_dist = (r < vscreen_l) ? vscreen_l - r
                   : (l > vscreen_l) ? l - vscreen_l
                   : 0;

        int r_dist = (r < vscreen_r) ? vscreen_r - r
                   : (l > vscreen_r) ? l - vscreen_r
                   : 0;

        int y_dist = (b < vscreen_t) ? vscreen_t - b
                   : (t > vscreen_b) ? t - vscreen_b
                   : 0;

        int x_num = vscreen.Width * vscreen.Width;
        int y_num = vscreen.Height * vscreen.Height;

        int l_div = l_dist * l_dist;
        int r_div = r_dist * r_dist;
        int y_div = y_dist * y_dist;

        int l_calc = 255 * (x_num + y_num) / (l_div + y_div + x_num + y_num);
        int r_calc = 255 * (x_num + y_num) / (r_div + y_div + x_num + y_num);

        if(l_calc > 255)
            l_calc = 255;
        if(r_calc > 255)
            r_calc = 255;

        if(l_calc > (int)left)
            left = l_calc;
        if(r_calc > (int)right)
            right = r_calc;
    }
}
