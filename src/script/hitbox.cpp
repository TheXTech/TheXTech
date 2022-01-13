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

#include "hitbox.h"
#include "csprite.h"

// TEST -- fast rectangle
bool Hitbox::Test(int Left2, int Up2, int iW, int iH) const
{
    if(pParent)
    {
        if(this->CollisionType == 0)   // square aabb collision detection
        {
            // bool rightcol = true;
            // bool leftcol = true;
            // bool upcol = true;
            // bool downcol = true;

            if(CalcRight() < Left2)
                return false;
            if(CalcLeft() > Left2 + iW)
                return false;
            if(CalcTop() > Up2 + iH)
                return false;
            if(CalcBottom() < Up2)
                return false;

            // This condition is ALWAYS FALSE
            //if(!rightcol || !leftcol || !upcol || !downcol)
            //    return false;
            return true;
        }
        else   // circle vs aaab -> convert to circle
        {
            int hW = iW / 2;
            int hH = iH / 2;
            double halfwidth = hW;
            double halfheight = hH;
            return Test((int)(Left2 + halfwidth), (int)(Up2 + halfheight), (int)(halfwidth <= halfheight ? halfwidth : halfheight));
        }
    }

    return false;
}

// TEST -- fast circle/distance
bool Hitbox::Test(int cx, int cy, int radius) const
{
    int radi_total = radius + (W <= H ? W / 2 : H / 2); // Other obj radius + my radius
    radi_total *= radi_total;
    int x_dist = (int)CenterX() - cx;
    int y_dist = (int)CenterY() - cy;
    int sqr_dist = x_dist * x_dist + y_dist * y_dist;

    if(sqr_dist > radi_total)
        return false;

    return true;
}

double Hitbox::CalcLeft() const
{
    if(pParent)
        return pParent->m_Xpos + Left_off;
    return 0;
}

double Hitbox::CalcTop() const
{
    if(pParent)
        return pParent->m_Ypos + Top_off;
    return 0;
}

double Hitbox::CalcBottom() const
{
    if(pParent)
        return pParent->m_Ypos + H;
    return 0;
}

double Hitbox::CalcRight() const
{
    if(pParent)
        return pParent->m_Xpos + W;
    return 0;
}

double Hitbox::CenterX() const
{
    return CalcLeft() + (W / 2.0);
}

double Hitbox::CenterY() const
{
    return CalcTop() + (H / 2.0);
}
