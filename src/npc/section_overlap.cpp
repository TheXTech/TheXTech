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

#include <Logger/logger.h>

#include "globals.h"

#include "npc/section_overlap.h"

std::array<uint8_t, maxSections + 1> g_SectionFirstOverlap;

//! Check if two section bounds collide
static bool s_SectionCollide(const Location_t& bounds, const Location_t& other_bounds)
{
    // remember that section bounds are stored "incorrectly" with "Width" / "Height" referring to the Right / Bottom
    return (bounds.Width >= other_bounds.X
        && bounds.X <= other_bounds.Width
        && bounds.Height >= other_bounds.Y
        && bounds.Y <= other_bounds.Height);
}

//! Find the first section (from start) that overlaps with S, and store it into g_SectionFirstOverlap[S]
static void s_FindFirstOverlap(int S, int start)
{
    g_SectionFirstOverlap[S] = S;

    for(int O = start; O < S; O++)
    {
        if(s_SectionCollide(level[S], level[O]))
        {
            g_SectionFirstOverlap[S] = O;
            D_pLogDebug("Note: section %d is the first section overlapping section %d", O, S);
            break;
        }
    }
}

//! Find the first section that overlaps with each section
void CalculateSectionOverlaps()
{
    for(int S = 0; S <= numSections; S++)
        s_FindFirstOverlap(S, 0);
}

void UpdateSectionOverlaps(int S, bool shrink)
{
    // if not a shrink, find first section overlapping S
    if(!shrink)
        s_FindFirstOverlap(S, 0);
    // if a shrink, only check sections starting from current first overlap
    else if(g_SectionFirstOverlap[S] != S)
        s_FindFirstOverlap(S, g_SectionFirstOverlap[S]);

    // update all sections after S (for which S might be first overlap)
    for(int O = S + 1; O <= numSections; O++)
    {
        // if S is the current overlap, check that it still applies
        if(g_SectionFirstOverlap[O] == S)
        {
            if(!s_SectionCollide(level[S], level[O]))
                s_FindFirstOverlap(O, S + 1);
        }
        // if S is earlier than the current overlap (and not a shrink), check if S is the new first overlap
        else if(!shrink && g_SectionFirstOverlap[O] > S)
        {
            if(s_SectionCollide(level[S], level[O]))
                g_SectionFirstOverlap[O] = S;
        }
    }
}
