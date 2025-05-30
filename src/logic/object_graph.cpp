/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2025 Vitaly Novichkov <admin@wohlnet.ru>
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

#include "sdl_proxy/sdl_stdinc.h"
#include <Logger/logger.h>

#include "logic/object_graph.h"

#include "npc_id.h"
#include "npc.h"
#include "globals.h"

namespace ObjectGraph
{

template<class LocType>
Loc get_center(const LocType& loc)
{
    return {(int)(loc.X + loc.Width / 2), (int)(loc.Y + loc.Height / 2)};
}

static unsigned int s_get_distance(const Loc& loc_1, const Loc& loc_2)
{
    int dx = loc_1.x - loc_2.x;
    int dy = loc_1.y - loc_2.y;
    return SDL_abs(dx) + SDL_abs(dy);
}

void Graph::expand(SearchPriorityQueue& queue, DistMap& dist_map, const Object* node, unsigned int distance, bool warps_reverse) const
{
    if(dist_map.find(node) != dist_map.end())
        return;

    dist_map.emplace(node, distance);

    // expand warps (only)
    if(node->type == Object::Warp)
    {
        // add paths from warp to all nodes to queue
        Loc source_loc = warps_reverse ? node->loc : node->dest;

        for(const Object* next : this->all_nodes)
        {
            Loc dest_loc = (warps_reverse && next->type == Object::Warp) ? next->dest : next->loc;
            queue.emplace(distance + s_get_distance(source_loc, dest_loc), next);
        }
    }
}

void Graph::search(DistMap& dist_map, const Object* origin, bool warps_reverse) const
{
    SearchPriorityQueue queue;
    dist_map.clear();

    // add paths from origin to all nodes to queue
    Loc origin_loc = (!warps_reverse && origin->type == Object::Warp) ? origin->dest : origin->loc;

    for(const Object* next : this->all_nodes)
    {
        Loc next_loc = (warps_reverse && next->type == Object::Warp) ? next->dest : next->loc;
        queue.emplace(s_get_distance(origin_loc, next_loc), next);
    }

    while(!queue.empty())
    {
        QueueEntry entry = queue.top();
        queue.pop();

        unsigned int distance = entry.first;
        const Object* node = entry.second;

        expand(queue, dist_map, node, distance, warps_reverse);
    }
}

void Graph::update()
{
    // refresh the set of nodes
    this->all_nodes.clear();
    for(const Object& node : this->level.warps)
        this->all_nodes.insert(&node);
    // don't process exits for now, because only medals are tracked for now
    // for(const Object& node : this->level.exits)
    //     this->all_nodes.insert(&node);
    for(const Object& node : this->level.items)
        this->all_nodes.insert(&node);

    // create the map of distances from start
    search(start_dist_map, &this->level.player_start, false);

    // check the furthest distance
    this->furthest_dist = 0;
    for(const Object* node : this->all_nodes)
    {
        unsigned int node_dist = this->start_dist_map[node];
        if(node_dist > this->furthest_dist)
            this->furthest_dist = node_dist;
    }
}

unsigned int Graph::distance_from_start(Loc loc)
{
    unsigned int best_distance = s_get_distance(loc, this->level.player_start.loc);

    for(const Object* node : this->all_nodes)
    {
        unsigned int start_to_node = start_dist_map[node];
        if(start_to_node == 0 || start_to_node >= best_distance)
            continue;

        const Loc& node_loc = (node->type == Object::Warp) ? node->dest : node->loc;

        unsigned int node_to_loc = s_get_distance(loc, node_loc);

        if(start_to_node + node_to_loc < best_distance)
            best_distance = start_to_node + node_to_loc;
    }

    return best_distance;
}

void FillGraph(Graph& graph)
{
    // start by filling the level struct
    graph.level = ObjectGraph::Level();

    graph.level.player_start = o(ObjectGraph::Object::PlayerStart,
        get_center(PlayerStart[1]));

    for(int i = 1; i <= numWarps; i++)
    {
        if(Warp[i].LevelEnt)
        {
            // eventually mark it as a possible level start
            continue;
        }

        if(Warp[i].MapWarp || Warp[i].LevelWarp)
        {
            graph.level.exits.push_back(o(ObjectGraph::Object::Exit,
                get_center(Warp[i].Entrance),
                ObjectGraph::Object::G_Warp, i));
        }
        else
        {
            graph.level.warps.push_back(o(ObjectGraph::Object::Warp,
                get_center(Warp[i].Entrance),
                ObjectGraph::Object::G_Warp, i,
                get_center(Warp[i].Exit)));

            if(Warp[i].twoWay)
            {
                graph.level.warps.push_back(o(ObjectGraph::Object::Warp,
                    get_center(Warp[i].Exit),
                    ObjectGraph::Object::G_Warp, i,
                    get_center(Warp[i].Entrance)));
            }
        }
    }

    // add magic doors as warps
    for(int i = 1; i <= numNPCs; ++i)
    {
        NPC_t& n = NPC[i];

        bool is_container = NPCIsContainer(n);

        bool contains_door = is_container && (n.Special == NPCID_DOOR_MAKER || n.Special == NPCID_MAGIC_DOOR);
        bool is_door = (n.Type == NPCID_DOOR_MAKER || n.Type == NPCID_MAGIC_DOOR);

        bool has_target_section = n.Variant <= maxSections;

        // allow doors only
        if(!is_door && !contains_door)
            continue;

        // only count it if it has a target section
        if(!has_target_section)
            continue;

        // add a warp from the NPC's position to the corresponding position in the target section

        // check current section (backing up whatever value was already there)
        uint8_t old_section = n.Section;
        CheckSectionNPC(i);

        int cur_section = n.Section;
        n.Section = old_section;

        Loc npc_center = get_center(n.Location);

        int targetX = npc_center.x - LevelREAL[cur_section].X + LevelREAL[n.Variant].X;
        int targetY = npc_center.y - LevelREAL[cur_section].Y + LevelREAL[n.Variant].Y;

        graph.level.warps.push_back(o(ObjectGraph::Object::Warp,
            npc_center,
            ObjectGraph::Object::G_NPC, i,
            {targetX, targetY}));
    }

    for(int i = 1; i <= numNPCs; i++)
    {
        if(NPC[i].Type == NPCID_GOALTAPE || NPC[i].Type == NPCID_STAR_EXIT || NPC[i].Type == NPCID_FLAG_EXIT
            || NPC[i].Type == NPCID_ITEMGOAL || NPC[i].Type == NPCID_GOALORB_S3 || NPC[i].Type == NPCID_GOALORB_S2)
        {
            graph.level.exits.push_back(o(ObjectGraph::Object::Exit,
                get_center(NPC[i].Location),
                ObjectGraph::Object::G_NPC, i));
        }

        // only track medals for now -- they're all that's used for the time being
        if(NPC[i].Type == NPCID_MEDAL) // || NPC[i].Type == NPCID_STAR_COLLECT || NPC[i].Type == NPCID_KEY)
        {
            graph.level.items.push_back(o(ObjectGraph::Object::Item,
                get_center(NPC[i].Location),
                ObjectGraph::Object::G_NPC, i));
        }
    }

    for(int i = 1; i <= numBackground; i++)
    {
        // keyhole
        if(Background[i].Type == 35)
        {
            graph.level.exits.push_back(o(ObjectGraph::Object::Exit,
                get_center(Background[i].Location),
                ObjectGraph::Object::G_BGO, i));
        }
    }

    // now update the graph (gets all objects' distances from start)
    graph.update();
}

} //namespace ObjectGraph
