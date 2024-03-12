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

#include <bitset>
#include "sdl_proxy/sdl_stdinc.h"

#include "globals.h"
#include "layers.h"
#include "compat.h"
#include "collision.h"

#include "npc_id.h"
#include "npc_traits.h"

#include "main/trees.h"

inline static bool s_Event_SoundOnly(const Events_t& evt, int test_section)
{
    if(!(evt.Text == STRINGINDEX_NONE
        && evt.HideLayer.empty()
        && evt.ShowLayer.empty()
        && evt.ToggleLayer.empty()
        && evt.MoveLayer == LAYER_NONE
        && evt.TriggerEvent == EVENT_NONE
        && evt.EndGame == 0))
    {
        return false;
    }

    bool autoscroll_okay = !AutoUseModern || evt.AutoSection != test_section
        || (g_compatibility.fix_autoscroll_speed
            ? (!(evt.AutoX != 0.0 || evt.AutoY != 0.0)
                || (AutoX[evt.AutoSection] == evt.AutoX && AutoY[evt.AutoSection] == evt.AutoY))
            : (!IF_INRANGE(evt.AutoSection, 0, SDL_min(maxSections, maxEvents))
                || (AutoX[evt.AutoSection] == Events[evt.AutoSection].AutoX
                    && AutoY[evt.AutoSection] == Events[evt.AutoSection].AutoY)));

    if(!autoscroll_okay)
        return false;

    const EventSection_t& s = const_cast<Events_t&>(evt).section[test_section];
    bool section_okay = s.music_id == EventSection_t::LESet_Nothing
        && s.background_id == EventSection_t::LESet_Nothing
        && (int)s.position.X == EventSection_t::LESet_Nothing
        && s.autoscroll == false;

    return section_okay;
}

static bool s_NPC_MustBeCanonical_internal(const NPC_t& n)
{
    return n.Generator
        || n->UseDefaultCam
        || (n->IsFish && Maths::iRound(n.Special) == 2)
        || n.AttLayer != LAYER_NONE
        || (n.TriggerActivate != EVENT_NONE && !s_Event_SoundOnly(Events[n.TriggerActivate], n.Section));
}

bool NPC_MustBeCanonical(NPCRef_t n)
{
    return n->_priv_force_canonical;
}

bool NPC_InactiveIgnore(const NPC_t& n)
{
    return (n->IsFish && Maths::iRound(n.Special) == 2)
        || n->InactiveRender == NPCTraits_t::SKIP;
}

bool NPC_InactiveRender(const NPC_t& n)
{
    return n.Inert
        || n.Stuck
        || n->InactiveRender == NPCTraits_t::SHOW_ALWAYS;
}

bool NPC_InactiveSmoke(const NPC_t& n)
{
    return n->InactiveRender == NPCTraits_t::SMOKE;
}

void NPC_ConstructCanonicalSet()
{
    std::vector<int16_t> to_check;
    to_check.reserve(64);

    for(int16_t n = 1; n <= numNPCs; n++)
    {
        if(s_NPC_MustBeCanonical_internal(NPC[n]))
        {
            NPC[n]._priv_force_canonical = true;
            to_check.push_back(n);
        }
    }

    // perform chain activation
    for(size_t i = 0; i < to_check.size(); i++)
    {
        int16_t n = to_check[i];

        Location_t tempLocation = NPC[n].Location;
        tempLocation.Y -= 32;
        tempLocation.X -= 32;
        tempLocation.Width += 64;
        tempLocation.Height += 64;

        for(int16_t B : treeNPCQuery(tempLocation, SORTMODE_NONE))
        {
            if(B != n && CheckCollision(tempLocation, NPC[B].Location))
            {
                if(!NPC[B]._priv_force_canonical)
                {
                    NPC[B]._priv_force_canonical = true;
                    to_check.push_back(B);
                }
            }
        }
    }
}
