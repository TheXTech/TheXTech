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
#include "../game_main.h"
#include "main/game_info.h"


static float s_alphaFromY(double y)
{
    const int h = 32; // An approximate height of one text line
    const int tb = 16;
    int b = static_cast<int>(y + h);
    int t = static_cast<int>(y);

    if(t >= ScreenH) // The line at bottom
    {
        return 0.0f;
    }

    if(b > ScreenH) // The line enters the screen
    {
        return 1.0f - static_cast<float>(b - ScreenH) / static_cast<float>(h);
    }

    if(b <= tb) // The line at the top
    {
        return 0.0f;
    }

    if(t < tb) // The line quits the screen
    {
        return static_cast<float>(b - tb) / static_cast<float>(h);
    }

    return 1.0f;
}

void DrawCredits()
{
    frmMain.renderRect(0, 0, ScreenW, Maths::iRound(CreditChop), 0.f, 0.f, 0.f);
    frmMain.renderRect(0, ScreenH - Maths::iRound(CreditChop), ScreenW, std::ceil(CreditChop), 0.f, 0.f, 0.f);

    if(CreditChop > 100)
    {
        float chop = CreditChop- 100;
        float chop_max = (static_cast<float>(ScreenH) / 2.f) - 100;
        float alpha = chop / chop_max;
        frmMain.renderRect(0, 0, ScreenW, ScreenH, 0.f, 0.f, 0.f, alpha);
    }

    int A;

    // Find the highest
    for(A = 1; A <= numCredits; A++)
    {
        auto &c = Credit[A];
        auto &l = c.Location;
        auto bottom = static_cast<float>(l.Y) + l.Height + CreditOffsetY;
        if(bottom >= 0)
            break; // found!
    }

    // Draw that actually visible
    for(; A <= numCredits; A++)
    {
        auto &c = Credit[A];
        auto &l = c.Location;
        auto y = static_cast<float>(l.Y) + CreditOffsetY;

        if(y > ScreenH)
            break; // Nothing also to draw

        // Printing lines of credits
        SuperPrint(c.Text,
                   g_gameInfo.creditsFont,
                   static_cast<float>(l.X),
                   y,
                   1.0f, 1.0f, 1.0f, s_alphaFromY(y));
    }
}
