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


#include <vector>
#include <set>

#include "globals.h"

#include "npc/npc_queues.h"

namespace NPCQueues
{

std::vector<NPCRef_t> NoReset;

std::vector<NPCRef_t> Killed;
std::vector<NPCRef_t> Unchecked;

std::vector<NPCRef_t> PlayerTemp;

SafeSet<NPCRef_t> Active;

std::set<NPCRef_t> RespawnDelay;

void clear()
{
    NoReset.clear();

    Killed.clear();
    Unchecked.clear();

    PlayerTemp.clear();

    Active.clear();

    RespawnDelay.clear();
}

void update(NPCRef_t npc)
{
    if((int)npc > numNPCs)
    {
        Active.erase(npc);
        RespawnDelay.erase(npc);

        return;
    }

    // cheaper to do this than to check if it's already there
    NoReset.push_back(npc);
    Unchecked.push_back(npc);

    if(npc->playerTemp)
        PlayerTemp.push_back(npc);

    if(check_active(npc))
        Active.insert(npc);
    else
        Active.erase(npc);

    if(npc->RespawnDelay)
        RespawnDelay.insert(npc);
    else
        RespawnDelay.erase(npc);

    if(npc->Killed)
        Killed.push_back(npc);
}

} // namespace NPCQueues
