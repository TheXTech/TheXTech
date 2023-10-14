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

#include "lunalevel.h"
#include "globals.h"

#include "npc/section_overlap.h"


void LevelF::PushSectionBoundary(int section, int which_boundary_UDLR, double push_val)
{
    Location_t &boundarray = level[section];

    switch(which_boundary_UDLR)
    {
    case 0:     // U
        boundarray.Y += push_val;
        break;

    case 1:     // D
        boundarray.Height += push_val;
        break;

    case 2:     // L
        boundarray.X += push_val;
        break;

    case 3:     // R
        boundarray.Width += push_val;
        break;

    default:
        break;
    }

    UpdateSectionOverlaps(section);
}

void LevelF::SetSectionBounds(int section, double left_bound, double top_bound, double right_bound, double bot_bound)
{
    Location_t &boundarray = level[section];
    boundarray.X = left_bound;
    boundarray.Y = top_bound;
    boundarray.Height = bot_bound;
    boundarray.Width = right_bound;
    UpdateSectionOverlaps(section);
}

double LevelF::GetBoundary(int section, int which_boundary_UDLR)
{
    const Location_t &boundarray = level[section];

    switch(which_boundary_UDLR)
    {
    case 0:     // U
        return boundarray.Y;
    case 1:     // D
        return boundarray.Height;
    case 2:     // L
        return boundarray.X;
    case 3:     // R
        return boundarray.Width;
    default:
        break;
    }

    return 0;
}

void LevelF::GetBoundary(LunaRect *rect, int section)
{
    if(section >= 0 && section < numSections)
    {
        const Location_t &boundarray = level[section];
        rect->top = (int)boundarray.Y;
        rect->bottom = (int)boundarray.Height;
        rect->left = (int)boundarray.X;
        rect->right = (int)boundarray.Width;
    }
}

std::string LevelF::GetName()
{
    return LevelName;
}
