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


#pragma once

#ifndef NPC_QUEUES_H
#define NPC_QUEUES_H

/*
 * The goal of this file is to maintain certain queues to track which NPCs
 * need to be accessed at which times. Ultimately, the intention is to
 * eliminate all loops over the entire NPC array.
 *
 *
 * All queues except paired write/read vector queues share a common interface:
 *   - C++11 iteration over `NPCRef_t`s
 *   - clear()
 *   - insert(NPCRef_t)
 *   - erase(NPCRef_t)
 *   - size_t size()
 *
 *
 * Each type of queue must be chosen for the optimal use case:
 *
 * - Some queues are write-once/read-once per-frame, like the set of NPCs that
 *   currently have one of the Reset flags disabled. They can be simple
 *   vectors, or pairs of vectors if they are simultaneously read and written.
 *
 * - Other queues are read and written frequently, but persist across frames,
 *   like the set of NPCs that are Active or have TimeLeft > 0.
 *   They should carefully be tuned as sets, well-constructed linked lists,
 *   or indexed sorted vectors, depending on use case.
 *
 * - Some queues need to be sorted, and others don't.
 *
 *
 * When implementing a new queue, it is best to start with the set-backed
 * queue because this one has the simplest implementation. Then if profiling
 * reveals a need, we can change the queue.
 *
 * -- ds-sloth
 */

#include <vector>
#include <set>
#include "npc/safe_set.hpp"

#include "globals.h"

namespace NPCQueues
{

// NPC[A].Reset[X] = false -> NPCQueues::NoReset.push_back(A);
extern std::vector<NPCRef_t> NoReset;

// NPC[A].Killed = X -> NPCQueues::Killed.push_back(A);
extern std::vector<NPCRef_t> Killed;

// NPC[A].playerTemp = true -> NPCQueues::PlayerTemp.push_back(A);
extern std::vector<NPCRef_t> PlayerTemp;

// NPC[A].Location.Width modified or NPC A deactivated -> NPCQueues::Unchecked.push_back(A);
//   Also add if certain fields (RealSpeedX and various counters) are unexpectedly modified while NPC inactive
extern std::vector<NPCRef_t> Unchecked;

// Contained in any of the following cases:
// - NPC[A].Active true
// - NPC[A].Generator true
// - NPC[A].JustActivated true
// - check_active_type(NPC[A]) true -- check when changing NPC[A].Type
// - NPC[A].AttLayer not LAYER_NONE or LAYER_DEFAULT
extern SafeSet<NPCRef_t> Active;

// NPC[A].RespawnDelay modified -> NPCQueues::RespawnDelay.insert(A) / erase(A);
extern std::set<NPCRef_t> RespawnDelay;

inline bool check_active_type(NPCRef_t n)
{
    return n->Type == 57 || n->Type == 60 || n->Type == 62 || n->Type == 64 || n->Type == 66;
}

inline bool check_active(NPCRef_t n)
{
    return n->Active || n->Generator || n->JustActivated || check_active_type(n) || (n->AttLayer != LAYER_NONE && n->AttLayer != LAYER_DEFAULT);
}

void clear();

void update(NPCRef_t npc);

} // namespace NPCQueues

#endif // #ifndef NPC_QUEUES_H
