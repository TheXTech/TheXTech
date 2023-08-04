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

#include "sdl_proxy/sdl_stdinc.h"
#include <Logger/logger.h>

#include "logic/object_graph.h"

#include "npc_id.h"
#include "globals.h"

namespace ObjectGraph
{

double get_distance(const Loc& loc_1, const Loc& loc_2)
{
    double dx = loc_1.x - loc_2.x;
    double dy = loc_1.y - loc_2.y;
    return SDL_abs(dx) + SDL_abs(dy);
    return SDL_sqrt(dx*dx + dy*dy);
}

void Graph::expand(Graph::SearchPriorityQueue& queue, Graph::DistMap& dist_map, const Object* node, double distance, bool warps_reverse) const
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
            queue.emplace(distance + get_distance(source_loc, dest_loc), next);
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
        queue.emplace(get_distance(origin_loc, next_loc), next);
    }

    while(!queue.empty())
    {
        QueueEntry entry = queue.top();
        queue.pop();

        double distance = entry.first;
        const Object* node = entry.second;

        expand(queue, dist_map, node, distance, warps_reverse);
    }
}

void Graph::update()
{
    this->all_nodes.clear();
    for(const Object& node : this->level.warps)
        this->all_nodes.insert(&node);
    for(const Object& node : this->level.exits)
        this->all_nodes.insert(&node);
    for(const Object& node : this->level.items)
        this->all_nodes.insert(&node);

    search(start_dist_map, &this->level.player_start, false);

    this->furthest_dist = 0.0;
    for(const Object* node : this->all_nodes)
    {
        double node_dist = this->start_dist_map[node];
        if(node_dist > this->furthest_dist)
            this->furthest_dist = node_dist;
    }
}

double Graph::place_loc(Loc loc)
{
    if(this->furthest_dist == 0)
        return 0.0;

    double best_distance = get_distance(loc, this->level.player_start.loc);
    for(const Object* node : this->all_nodes)
    {
        double start_to_node = start_dist_map[node];
        if(start_to_node == 0.0 || start_to_node >= best_distance)
            continue;

        double node_to_loc;
        if(node->type == Object::Warp)
            node_to_loc = get_distance(loc, node->dest);
        else
            node_to_loc = get_distance(loc, node->loc);

        if(start_to_node + node_to_loc < best_distance)
            best_distance = start_to_node + node_to_loc;
    }

    return best_distance / this->furthest_dist;
}

void FillGraph(Graph& graph)
{
    graph.level = ObjectGraph::Level();

    graph.level.player_start = o(ObjectGraph::Object::PlayerStart,
        {PlayerStart[1].X, PlayerStart[1].Y});

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
                {Warp[i].Entrance.X, Warp[i].Entrance.Y},
                ObjectGraph::Object::G_Warp, i));
        }
        else
        {
            graph.level.warps.push_back(o(ObjectGraph::Object::Warp,
                {Warp[i].Entrance.X, Warp[i].Entrance.Y},
                ObjectGraph::Object::G_Warp, i,
                {Warp[i].Exit.X, Warp[i].Exit.Y}));

            if(Warp[i].twoWay)
            {
                graph.level.warps.push_back(o(ObjectGraph::Object::Warp,
                    {Warp[i].Exit.X, Warp[i].Exit.Y},
                    ObjectGraph::Object::G_Warp, i,
                    {Warp[i].Entrance.X, Warp[i].Entrance.Y}));
            }
        }
    }

    for(int i = 1; i <= numNPCs; i++)
    {
        if(NPC[i].Type == NPCID_GOALTAPE || NPC[i].Type == NPCID_STAR_EXIT
            || NPC[i].Type == NPCID_ITEMGOAL || NPC[i].Type == NPCID_GOALORB_S3 || NPC[i].Type == NPCID_GOALORB_S2)
        {
            graph.level.exits.push_back(o(ObjectGraph::Object::Exit,
                {NPC[i].Location.X, NPC[i].Location.Y},
                ObjectGraph::Object::G_NPC, i));
        }

        if(NPC[i].Type == NPCID_MEDAL || NPC[i].Type == NPCID_STAR_COLLECT || NPC[i].Type == NPCID_KEY)
        {
            graph.level.items.push_back(o(ObjectGraph::Object::Item,
                {NPC[i].Location.X, NPC[i].Location.Y},
                ObjectGraph::Object::G_NPC, i));
        }
    }

    for(int i = 1; i <= numBackground; i++)
    {
        if(Background[i].Type == 35)
        {
            graph.level.exits.push_back(o(ObjectGraph::Object::Exit,
                {Background[i].Location.X, Background[i].Location.Y},
                ObjectGraph::Object::G_BGO, i));
        }
    }

    graph.update();
}

}; //namespace ObjectGraph
