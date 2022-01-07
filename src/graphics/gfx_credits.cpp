/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2021 Vitaly Novichkov <admin@wohlnet.ru>
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
#include "main/game_info.h"
#include "core/render.h"


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
    XRender::renderRect(0, 0, ScreenW, Maths::iRound(CreditChop), 0.f, 0.f, 0.f);
    XRender::renderRect(0, ScreenH - Maths::iRound(CreditChop), ScreenW, std::ceil(CreditChop), 0.f, 0.f, 0.f);

    if(CreditChop > 100)
    {
        float chop = CreditChop- 100;
        float chop_max = (static_cast<float>(ScreenH) / 2.f) - 100;
        float alpha = chop / chop_max;
        XRender::renderRect(0, 0, ScreenW, ScreenH, 0.f, 0.f, 0.f, alpha);
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
