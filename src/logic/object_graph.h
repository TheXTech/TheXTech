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

#pragma once

#ifndef OBJECT_GRAPH_H
#define OBJECT_GRAPH_H

#include <unordered_map>
#include <vector>
#include <set>
#include <queue>

//! a set of features designed to support warp-aware (but not block-aware) distance checks between objects
namespace ObjectGraph
{

struct Loc
{
    double x;
    double y;
};

//! represents a single game object (does not attempt to track a single NPC if its ID changes)
struct Object
{
    enum Object_Type
    {
        PlayerStart,
        Warp,
        Item,
        Exit
    };
    enum Game_Object_Type
    {
        G_None,
        G_Warp,
        G_Block,
        G_NPC,
        G_BGO
    };

    Object_Type type;
    Loc loc;
    Game_Object_Type g_type = G_None;
    int game_index = -1;
    Loc dest;
};

inline Object o(Object::Object_Type type, Loc loc)
{
    Object o;
    o.type = type;
    o.loc = loc;
    return o;
}

inline Object o(Object::Object_Type type, Loc loc, Object::Game_Object_Type g_type, int game_index)
{
    Object o;
    o.type = type;
    o.loc = loc;
    o.g_type = g_type;
    o.game_index = game_index;
    return o;
}

inline Object o(Object::Object_Type type, Loc loc, Object::Game_Object_Type g_type, int game_index, Loc dest)
{
    Object o;
    o.type = type;
    o.loc = loc;
    o.g_type = g_type;
    o.game_index = game_index;
    o.dest = dest;
    return o;
}

struct Level
{
    Object player_start;
    std::vector<Object> exits;
    std::vector<Object> warps;
    std::vector<Object> items;
};

class Graph
{
public:
    // map of distances to/from objects
    using DistMap = std::unordered_map<const Object*, double>;

    Level level;

    // set of all objects in the graph
    std::set<const Object*> all_nodes;

    // map of distances from P1 start TO each object
    DistMap start_dist_map;

    //! furthest distance of anything from player start
    double furthest_dist = 0.0;

    //! update the object graph's nodes and start_dist_map based on its level substruct
    void update();

    //! get the distance (using warps if needed) from P1 start to loc
    double distance_from_start(Loc loc);

private:
    // search algorithm support functions
    using QueueEntry = std::pair<double, const Object*>;
    using SearchPriorityQueue = std::priority_queue<QueueEntry, std::vector<QueueEntry>, std::greater<QueueEntry>>;

    // Expand a single object in the queue, updating the dist_map and adding new queue entries to additional objects.
    void expand(SearchPriorityQueue& queue, DistMap& dist_map, const Object* node, double distance, bool warps_reverse) const;

    // Fill dist_map with distances from origin. If warps_reverse is true, then find paths from objects to origin.
    void search(DistMap& dist_map, const Object* origin, bool warps_reverse) const;
};

//! create an object graph based on the currently loaded level
void FillGraph(Graph& graph);

} //namespace ObjectGraph

#endif // #ifndef OBJECT_GRAPH_H
