/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2022 Vitaly Novichkov <admin@wohlnet.ru>
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
#include "globals.h"
#include "layers.h"
#include "compat.h"
#include "collision.h"

#include "npc_id.h"

static std::bitset<maxNPCs> s_canonicalNPCs;

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
        || n.Type == NPCID_THWOMP_SMB3
        || n.Type == NPCID_THWOMP_SMW
        || n.Type == NPCID_METALBARREL
        || n.Type == NPCID_CANNONENEMY
        || n.Type == NPCID_STATUE_SMB3
        || n.Type == NPCID_STATUE_SMW
        || n.Type == NPCID_RINKAGEN
        || n.Type == NPCID_BLARGG
        || n.Type == NPCID_LAKITU_SMB3
        || n.Type == NPCID_LAKITU_SMW
        || (NPCIsCheep[n.Type] && Maths::iRound(n.Special) == 2)
        || n.AttLayer != LAYER_NONE
        || (n.TriggerActivate != EVENT_NONE && !s_Event_SoundOnly(Events[n.TriggerActivate], n.Section));
}

bool NPC_MustBeCanonical(NPCRef_t n)
{
    return s_canonicalNPCs[n];
}

bool NPC_MustNotRenderInactive(const NPC_t& n)
{
    return (NPCIsCheep[n.Type] && Maths::iRound(n.Special) == 2)
        || n.Type == NPCID_PODOBOO
        || n.Type == NPCID_PIRANHA_SMB3 || n.Type == NPCID_BOTTOMPIRANHA || n.Type == NPCID_SIDEPIRANHA
        || n.Type == NPCID_BIGPIRANHA || n.Type == NPCID_PIRANHA_SMB || n.Type == NPCID_FIREPIRANHA
        || n.Type == NPCID_LONGPIRANHA_UP || n.Type == NPCID_LONGPIRANHA_DOWN || n.Type == NPCID_PIRANHAHEAD
        || n.Type == NPCID_BLARGG;
}

bool NPC_MustRenderInactive(const NPC_t& n)
{
    return n.Inert
        || n.Stuck
        || NPCIsACoin[n.Type]
        || NPCIsABlock[n.Type]
        || NPCIsAVine[n.Type]
        || n.Type == NPCID_CHECKPOINT
        || n.Type == NPCID_BURIEDPLANT
        || n.Type == NPCID_CONVEYOR
        || n.Type == NPCID_THWOMP_SMB3
        || n.Type == NPCID_THWOMP_SMW
        || n.Type == NPCID_CANNONENEMY
        || n.Type == NPCID_STATUE_SMB3
        || n.Type == NPCID_STATUE_SMW
        || n.Type == NPCID_RINKAGEN
        || n.Type == NPCID_YOSHIEGG;
}

void NPC_ConstructCanonicalSet()
{
    std::vector<int16_t> to_check;

    s_canonicalNPCs.reset();

    for(int16_t n = 1; n <= numNPCs; n++)
    {
        if(s_NPC_MustBeCanonical_internal(NPC[n]))
        {
            s_canonicalNPCs[n] = true;
            to_check.push_back(n);
        }
    }

    // perform chain activation
    for(int16_t n : to_check)
    {
        Location_t tempLocation = NPC[n].Location;
        tempLocation.Y -= 32;
        tempLocation.X -= 32;
        tempLocation.Width += 64;
        tempLocation.Height += 64;

        for(int16_t B = 1; B <= numNPCs; B++)
        {
            if(B != n && CheckCollision(tempLocation, NPC[B].Location))
            {
                if(!s_canonicalNPCs[B])
                {
                    s_canonicalNPCs[B] = true;
                    to_check.push_back(B);
                }
            }
        }
    }
}
