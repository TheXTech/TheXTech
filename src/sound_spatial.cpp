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

        int vscreen_l = -(int)vscreen.X;
        int vscreen_r = vscreen_l + vscreen.Width;
        int vscreen_c = vscreen_l + vscreen.Width / 2;
        int vscreen_t = -(int)vscreen.Y;
        int vscreen_b = vscreen_t + vscreen.Height;

        int l_dist = (r < vscreen_l) ? vscreen_l - r
                   : (l > vscreen_c) ? l - vscreen_c
                   : 0;

        int r_dist = (r < vscreen_c) ? vscreen_c - r
                   : (l > vscreen_r) ? l - vscreen_r
                   : 0;

        int x_dist = (r < vscreen_l) ? vscreen_l - r
                   : (l > vscreen_r) ? l - vscreen_r
                   : 0;

        int y_dist = (b < vscreen_t) ? vscreen_t - b
                   : (t > vscreen_b) ? t - vscreen_b
                   : 0;

        // decay constant: a sound 200px away should be 10dB quieter
        int64_t x_num = 200 * 200;
        int64_t y_num = 200 * 200;

        if(x_num <= 0)
            x_num = 1;
        if(y_num <= 0)
            y_num = 1;

        int64_t l_div = l_dist * l_dist;
        int64_t r_div = r_dist * r_dist;

        int64_t c_div = x_dist * x_dist;
        int64_t y_div = y_dist * y_dist;

        int l_calc = 127 * (x_num + y_num) / (l_div + y_div + x_num + y_num);
        int r_calc = 127 * (x_num + y_num) / (r_div + y_div + x_num + y_num);

        int c_calc = 128 * (x_num + y_num) / (c_div + y_div + x_num + y_num);

        l_calc = l_calc + c_calc;
        r_calc = r_calc + c_calc;

        if(l_calc > 255)
            l_calc = 255;
        if(r_calc > 255)
            r_calc = 255;

        if(vscreen.ScreenLeft >= screen.W / 2)
            l_calc = (l_calc * 3) / 4;
        else if(vscreen.ScreenLeft + vscreen.Width <= screen.W / 2)
            r_calc = (r_calc * 3) / 4;

        if(l_calc > (int)left)
            left = l_calc;
        if(r_calc > (int)right)
            right = r_calc;
    }
}
