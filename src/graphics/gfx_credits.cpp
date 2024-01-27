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

    if(t >= XRender::TargetH) // The line at bottom
    {
        return 0.0f;
    }

    if(b > XRender::TargetH) // The line enters the screen
    {
        return 1.0f - static_cast<float>(b - XRender::TargetH) / static_cast<float>(h);
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
    XRender::renderRect(0, 0, XRender::TargetW, Maths::iRound(CreditChop), {0, 0, 0});
    XRender::renderRect(0, XRender::TargetH - Maths::iRound(CreditChop), XRender::TargetW, std::ceil(CreditChop), {0, 0, 0});

    // previously hardcoded to 100
    int shrink = vScreen[1].Top;

    if(CreditChop > shrink)
    {
        int chop = CreditChop - shrink;
        int chop_max = (XRender::TargetH / 2) - shrink;
        uint8_t alpha = 255 * chop / chop_max;
        XRender::renderRect(0, 0, XRender::TargetW, XRender::TargetH, {0, 0, 0, alpha});
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

        if(y > XRender::TargetH)
            break; // Nothing also to draw

        // Printing lines of credits
        SuperPrint(GetS(c.Text),
                   g_gameInfo.creditsFont,
                   static_cast<float>(l.X),
                   y,
                   XTAlphaF(s_alphaFromY(y)));
    }
}
