/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2020 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "../globals.h"
#include "../graphics.h"

/*Private*/
void SpecialFrames()
{
    SpecialFrameCount[1] = SpecialFrameCount[1] + 1;
    if(SpecialFrameCount[1] >= 6)
    {
        SpecialFrame[1] = SpecialFrame[1] + 1;
        if(SpecialFrame[1] >= 2)
            SpecialFrame[1] = 0;
        SpecialFrameCount[1] = 0;
    }
    SpecialFrameCount[2] = SpecialFrameCount[2] + 1;
    if(SpecialFrameCount[2] >= 3)
    {
        SpecialFrame[2] = SpecialFrame[2] + 1;
        if(SpecialFrame[2] >= 4)
            SpecialFrame[2] = 0;
        SpecialFrameCount[2] = 0;
    }
    SpecialFrameCount[3] = SpecialFrameCount[3] + 1;
    if(SpecialFrameCount[3] >= 8)
    {
        SpecialFrame[3] = SpecialFrame[3] + 1;
        if(SpecialFrame[3] >= 4)
            SpecialFrame[3] = 0;
        SpecialFrameCount[3] = 0;
    }
    SpecialFrameCount[4] = SpecialFrameCount[4] + 1;
    if(SpecialFrameCount[4] >= 2.475F)
    {
        SpecialFrame[4] = SpecialFrame[4] + 1;
        if(SpecialFrame[4] >= 4)
            SpecialFrame[4] = 0;
        SpecialFrameCount[4] = (float)(SpecialFrameCount[4] - 2.475);
    }
    SpecialFrameCount[5] = SpecialFrameCount[5] + 1;
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
    SpecialFrameCount[6] = SpecialFrameCount[6] + 1;
    if(SpecialFrameCount[6] >= 12)
    {
        SpecialFrame[6] = SpecialFrame[6] + 1;
        if(SpecialFrame[6] >= 4)
            SpecialFrame[6] = 0;
        SpecialFrameCount[6] = 0;
    }

    SpecialFrameCount[7] = SpecialFrameCount[7] + 1;
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
    SpecialFrameCount[8] = SpecialFrameCount[8] + 1;
    if(SpecialFrameCount[8] >= 8)
    {
        SpecialFrame[8] = SpecialFrame[8] + 1;
        if(SpecialFrame[8] >= 3)
            SpecialFrame[8] = 0;
        SpecialFrameCount[8] = 0;
    }

    SpecialFrameCount[9] = SpecialFrameCount[9] + 1; // Fairy frame
    if(SpecialFrameCount[9] >= 8)
    {
        SpecialFrame[9] = SpecialFrame[9] + 1;
        if(SpecialFrame[9] >= 2)
            SpecialFrame[9] = 0;
        SpecialFrameCount[9] = 0;
    }
}

