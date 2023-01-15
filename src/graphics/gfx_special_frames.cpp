/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2023 Vitaly Novichkov <admin@wohlnet.ru>
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
#include "../graphics.h"

/*Private*/
void SpecialFrames()
{
    SpecialFrameCount[1] += 1;
    if(SpecialFrameCount[1] >= 6)
    {
        SpecialFrame[1] += 1;
        if(SpecialFrame[1] >= 2)
            SpecialFrame[1] = 0;
        SpecialFrameCount[1] = 0;
    }
    SpecialFrameCount[2] += 1;
    if(SpecialFrameCount[2] >= 3)
    {
        SpecialFrame[2] += 1;
        if(SpecialFrame[2] >= 4)
            SpecialFrame[2] = 0;
        SpecialFrameCount[2] = 0;
    }
    SpecialFrameCount[3] += 1;
    if(SpecialFrameCount[3] >= 8)
    {
        SpecialFrame[3] += 1;
        if(SpecialFrame[3] >= 4)
            SpecialFrame[3] = 0;
        SpecialFrameCount[3] = 0;
    }
    SpecialFrameCount[4] += 1;
    if(SpecialFrameCount[4] >= 2.475F)
    {
        SpecialFrame[4] += 1;
        if(SpecialFrame[4] >= 4)
            SpecialFrame[4] = 0;
        SpecialFrameCount[4] = (float)(SpecialFrameCount[4] - 2.475);
    }
    SpecialFrameCount[5] += 1;
    if(SpecialFrameCount[5] < 20)
        SpecialFrame[5] = 1;
    else if(SpecialFrameCount[5] < 25)
        SpecialFrame[5] = 2;
    else if(SpecialFrameCount[5] < 30)
        SpecialFrame[5] = 3;
    else if(SpecialFrameCount[5] < 35)
        SpecialFrame[5] = 4;
    else if(SpecialFrameCount[5] < 40)
        SpecialFrame[5] = 5;
    else if(SpecialFrameCount[5] < 45)
        SpecialFrame[5] = 1;
    else if(SpecialFrameCount[5] < 50)
        SpecialFrame[5] = 0;
    else
        SpecialFrameCount[5] = 0;
    SpecialFrameCount[6] += 1;
    if(SpecialFrameCount[6] >= 12)
    {
        SpecialFrame[6] += 1;
        if(SpecialFrame[6] >= 4)
            SpecialFrame[6] = 0;
        SpecialFrameCount[6] = 0;
    }

    SpecialFrameCount[7] += 1;
    if(SpecialFrameCount[7] < 8)
        SpecialFrame[7] = 0;
    else if(SpecialFrameCount[7] < 16)
        SpecialFrame[7] = 1;
    else if(SpecialFrameCount[7] < 24)
        SpecialFrame[7] = 2;
    else if(SpecialFrameCount[7] < 32)
        SpecialFrame[7] = 3;
    else if(SpecialFrameCount[7] < 40)
        SpecialFrame[7] = 2;
    else if(SpecialFrameCount[7] < 48)
        SpecialFrame[7] = 1;
    else
        SpecialFrameCount[7] = 0;
    SpecialFrameCount[8] += 1;
    if(SpecialFrameCount[8] >= 8)
    {
        SpecialFrame[8] += 1;
        if(SpecialFrame[8] >= 3)
            SpecialFrame[8] = 0;
        SpecialFrameCount[8] = 0;
    }

    SpecialFrameCount[9] += 1; // Fairy frame
    if(SpecialFrameCount[9] >= 8)
    {
        SpecialFrame[9] += 1;
        if(SpecialFrame[9] >= 2)
            SpecialFrame[9] = 0;
        SpecialFrameCount[9] = 0;
    }
}

void LevelFramesNotFrozen()
{
    BackgroundFrameCount[26]++;
    if(BackgroundFrameCount[26] >= 8)
    {
        BackgroundFrame[26]++;
        if(BackgroundFrame[26] >= 8)
            BackgroundFrame[26] = 0;
        BackgroundFrameCount[26] = 0;
    }
    BackgroundFrameCount[18]++;
    if(BackgroundFrameCount[18] >= 12)
    {
        BackgroundFrame[18]++;
        if(BackgroundFrame[18] >= 4)
            BackgroundFrame[18] = 0;
        BackgroundFrame[19] = BackgroundFrame[18];
        BackgroundFrame[20] = BackgroundFrame[18];
        BackgroundFrame[161] = BackgroundFrame[18];
        BackgroundFrameCount[18] = 0;
    }
    BackgroundFrameCount[36] += 1;
    if(BackgroundFrameCount[36] >= 2)
    {
        BackgroundFrame[36] += 1;
        if(BackgroundFrame[36] >= 4)
            BackgroundFrame[36] = 0;
        BackgroundFrameCount[36] = 0;
    }
    BackgroundFrame[68] = BackgroundFrame[36];
    BackgroundFrameCount[65] += 1;
    if(BackgroundFrameCount[65] >= 8)
    {
        BackgroundFrame[65] += 1;
        if(BackgroundFrame[65] >= 4)
            BackgroundFrame[65] = 0;
        BackgroundFrameCount[65] = 0;
    }

    BackgroundFrame[66] = BackgroundFrame[65];

    BackgroundFrame[70] = BackgroundFrame[65];
    BackgroundFrame[100] = BackgroundFrame[65];

    BackgroundFrame[134] = BackgroundFrame[65];
    BackgroundFrame[135] = BackgroundFrame[65];
    BackgroundFrame[136] = BackgroundFrame[65];
    BackgroundFrame[137] = BackgroundFrame[65];
    BackgroundFrame[138] = BackgroundFrame[65];


    BackgroundFrameCount[82] += 1;
    if(BackgroundFrameCount[82] >= 10)
    {
        BackgroundFrame[82] += 1;
        if(BackgroundFrame[82] >= 4)
            BackgroundFrame[82] = 0;
        BackgroundFrameCount[82] = 0;
    }

    BackgroundFrameCount[170] += 1;
    if(BackgroundFrameCount[170] >= 8)
    {
        BackgroundFrame[170] += 1;
        if(BackgroundFrame[170] >= 4)
            BackgroundFrame[170] = 0;
        BackgroundFrame[171] = BackgroundFrame[170];
        BackgroundFrameCount[170] = 0;
    }

    BackgroundFrameCount[125] += 1;
    if(BackgroundFrameCount[125] >= 4)
    {
        if(BackgroundFrame[125] == 0)
            BackgroundFrame[125] = 1;
        else
            BackgroundFrame[125] = 0;
        BackgroundFrameCount[125] = 0;
    }
}

void LevelFramesAlways()
{
    BackgroundFrame[172] = BackgroundFrame[66];
    BackgroundFrameCount[158] += 1;

    if(BackgroundFrameCount[158] >= 6)
    {
        BackgroundFrameCount[158] = 0;
        BackgroundFrame[158] += 1;
        BackgroundFrame[159] += 1;
        if(BackgroundFrame[158] >= 4)
            BackgroundFrame[158] = 0;
        if(BackgroundFrame[159] >= 8)
            BackgroundFrame[159] = 0;
    }

    BackgroundFrameCount[168] += 1;
    if(BackgroundFrameCount[168] >= 8)
    {
        BackgroundFrame[168] += 1;
        if(BackgroundFrame[168] >= 8)
            BackgroundFrame[168] = 0;
        BackgroundFrameCount[168] = 0;
    }

    BackgroundFrameCount[173] += 1;
    if(BackgroundFrameCount[173] >= 8)
    {
        BackgroundFrameCount[173] = 0;
        if(BackgroundFrame[173] == 0)
            BackgroundFrame[173] = 1;
        else
            BackgroundFrame[173] = 0;
    }

    BackgroundFrameCount[187] += 1;
    if(BackgroundFrameCount[187] >= 6)
    {
        BackgroundFrame[187] += 1;
        if(BackgroundFrame[187] >= 4)
            BackgroundFrame[187] = 0;
        BackgroundFrame[188] = BackgroundFrame[187];
        BackgroundFrame[189] = BackgroundFrame[187];
        BackgroundFrame[190] = BackgroundFrame[187];
        BackgroundFrameCount[187] = 0;
    }
}