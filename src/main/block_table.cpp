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

#include <algorithm>

#include "layers.h"
#include "compat.h"

#include "main/block_table.h"
#include "main/block_table.hpp"
#include "main/trees.h"

// all shared utility code for all item types
template<class ItemRef_t>
struct TableInterface
{
    table_t<ItemRef_t> common_table;
    table_t<ItemRef_t> layer_table[maxLayers+1];

    bool layer_table_active[maxLayers+1] = {false};
    int16_t active_tables[maxLayers+1] = {0};
    int num_active_tables = 0;

    // clears all tables and rejoins all layers
    void clear()
    {
        common_table.clear();

        for(int i = 0; i < maxLayers + 1; i++)
        {
            layer_table[i].clear();
            layer_table_active[i] = false;
        }

        num_active_tables = 0;
    }

    const std::set<int>& layer_items(int layer);

    // checks if a layer is currently split from the main table
    bool active(int layer)
    {
        return !layer_table_active[layer];
    }

    // splits a layer from the main table
    void split(int layer)
    {
        if(layer < 0 || layer == LAYER_NONE || layer_table_active[layer])
            return;

        layer_table_active[layer] = true;
        active_tables[num_active_tables] = layer;
        num_active_tables++;

        for(int i : layer_items(layer))
        {
            common_table.erase(i);
            layer_table[layer].insert_layer(i);
        }
    }

    // joins a layer to the main table
    void join(int layer)
    {
        if(layer < 0 || layer == LAYER_NONE || !layer_table_active[layer])
            return;

        layer_table_active[layer] = false;

        // remove from queue of active tables
        num_active_tables--;
        for(int i = 0; i < num_active_tables; i++)
        {
            if(active_tables[i] == layer)
            {
                active_tables[i] = active_tables[num_active_tables];
                break;
            }
        }

        for(int i : layer_items(layer))
        {
            common_table.insert(i);
        }

        layer_table[layer].clear();
    }

    void add(int layer, ItemRef_t item)
    {
        if(layer < 0 || layer == LAYER_NONE || !layer_table_active[layer])
            common_table.insert(item);
        else
            layer_table[layer].insert_layer(item);
    }

    void update(int layer, ItemRef_t item)
    {
        if(layer < 0 || layer == LAYER_NONE || !layer_table_active[layer])
            common_table.update(item);
        else
            layer_table[layer].update_layer(item);
    }

    void erase(int layer, ItemRef_t item)
    {
        if(layer < 0 || layer == LAYER_NONE || !layer_table_active[layer])
            common_table.erase(item);
        else
            layer_table[layer].erase(item);
    }

    TreeResult_Sentinel<ItemRef_t> query(Location_t loc,
                             int sort_mode)
    {
        TreeResult_Sentinel<ItemRef_t> result;

        common_table.query(*result.i_vec, loc);

        for(int i = 0; i < num_active_tables; i++)
        {
            int layer = active_tables[i];

            loc.X -= Layer[layer].OffsetX;
            loc.Y -= Layer[layer].OffsetY;

            layer_table[layer].query(*result.i_vec, loc);

            loc.X += Layer[layer].OffsetX;
            loc.Y += Layer[layer].OffsetY;
        }

        if(sort_mode == SORTMODE_COMPAT)
        {
            if(g_compatibility.emulate_classic_block_order)
                sort_mode = SORTMODE_ID;
            else
                sort_mode = SORTMODE_LOC;
        }

        if(sort_mode == SORTMODE_LOC)
        {
            std::sort(result.i_vec->begin(), result.i_vec->end(),
                [](BaseRef_t a, BaseRef_t b) {
                    return (((ItemRef_t)a)->Location.X < ((ItemRef_t)b)->Location.X
                        || (((ItemRef_t)a)->Location.X == ((ItemRef_t)b)->Location.X
                            && ((ItemRef_t)a)->Location.Y < ((ItemRef_t)b)->Location.Y));
                });
        }
        else if(sort_mode == SORTMODE_ID)
        {
            std::sort(result.i_vec->begin(), result.i_vec->end(),
                [](BaseRef_t a, BaseRef_t b) {
                    return a < b;
                });
        }
        else if(sort_mode == SORTMODE_Z)
        {
            std::sort(result.i_vec->begin(), result.i_vec->end(),
                [](BaseRef_t a, BaseRef_t b) {
                    // not implemented yet, might never be
                    // instead, just sort by the index
                    // (which is currently the same as z-order)
                    return a < b;
                });
        }

        return result;
    }

    TreeResult_Sentinel<ItemRef_t> query(double Left, double Top, double Right, double Bottom,
                             int sort_mode,
                             double margin)
    {
        Location_t loc = newLoc(Left - margin,
           Top - margin,
           (Right - Left) + margin * 2,
           (Bottom - Top) + margin * 2);

        return query(loc, sort_mode);
    }
};

table_t<BlockRef_t> s_temp_block_table;

/* ================= Level blocks ================= */

template<>
const std::set<int>& TableInterface<BlockRef_t>::layer_items(int layer)
{
    return Layer[layer].blocks;
}

TableInterface<BlockRef_t> s_block_tables;

void treeLevelCleanBlockLayers()
{
    s_block_tables.clear();
    s_temp_block_table.clear();
}

// checks if a layer is split from the main block table
bool treeBlockLayerActive(int layer)
{
    return s_block_tables.active(layer);
}

// splits a layer from the main block table
void treeBlockSplitLayer(int layer)
{
    s_block_tables.split(layer);
}

// joins a layer to the main block table
void treeBlockJoinLayer(int layer)
{
    s_block_tables.join(layer);
}

void treeBlockAddLayer(int layer, BlockRef_t block)
{
    s_block_tables.add(layer, block);
}

void treeBlockUpdateLayer(int layer, BlockRef_t block)
{
    s_block_tables.update(layer, block);
}

void treeBlockRemoveLayer(int layer, BlockRef_t block)
{
    s_block_tables.erase(layer, block);
}

TreeResult_Sentinel<BlockRef_t> treeBlockQuery(double Left, double Top, double Right, double Bottom,
                         int sort_mode,
                         double margin)
{
    return s_block_tables.query(Left, Top, Right, Bottom, sort_mode, margin);
}

TreeResult_Sentinel<BlockRef_t> treeBlockQuery(const Location_t &loc,
                         int sort_mode)
{
    return s_block_tables.query(loc, sort_mode);
}

/* ================= Temp blocks ================= */

void treeTempBlockStartFrame()
{
    s_temp_block_table.clear_light();
}

void treeTempBlockAdd(BlockRef_t obj)
{
    s_temp_block_table.insert(obj);
}

void treeTempBlockUpdate(BlockRef_t obj)
{
    s_temp_block_table.update(obj);
}

TreeResult_Sentinel<BlockRef_t> treeTempBlockQuery(const Location_t &loc,
                         int sort_mode)
{
    TreeResult_Sentinel<BlockRef_t> result;

    s_temp_block_table.query(*result.i_vec, loc);

    if(sort_mode == SORTMODE_COMPAT)
    {
        sort_mode = SORTMODE_LOC;
    }

    if(sort_mode == SORTMODE_LOC)
    {
        std::sort(result.i_vec->begin(), result.i_vec->end(),
            [](BaseRef_t a, BaseRef_t b) {
                return (((BlockRef_t)a)->Location.X < ((BlockRef_t)b)->Location.X
                    || (((BlockRef_t)a)->Location.X == ((BlockRef_t)b)->Location.X
                        && ((BlockRef_t)a)->Location.Y < ((BlockRef_t)b)->Location.Y));
            });
    }
    else if(sort_mode == SORTMODE_ID)
    {
        std::sort(result.i_vec->begin(), result.i_vec->end(),
            [](BaseRef_t a, BaseRef_t b) {
                return a.index < b.index;
            });
    }
    else if(sort_mode == SORTMODE_Z)
    {
        std::sort(result.i_vec->begin(), result.i_vec->end(),
            [](BaseRef_t a, BaseRef_t b) {
                // not implemented yet, might never be
                // instead, just sort by the index
                // (which is currently the same as z-order)
                return a.index < b.index;
            });
    }

    return result;
}

TreeResult_Sentinel<BlockRef_t> treeTempBlockQuery(double Left, double Top, double Right, double Bottom,
                         int sort_mode,
                         double margin)
{
    Location_t loc = newLoc(Left - margin,
               Top - margin,
               (Right - Left) + margin * 2,
               (Bottom - Top) + margin * 2);

    return treeTempBlockQuery(loc, sort_mode);
}

/* ================= Level Backgrounds ================= */

template<>
const std::set<int>& TableInterface<BackgroundRef_t>::layer_items(int layer)
{
    return Layer[layer].BGOs;
}

TableInterface<BackgroundRef_t> s_background_tables;

void treeLevelCleanBackgroundLayers()
{
    s_background_tables.clear();
}

// checks if a layer is split from the main background table
bool treeBackgroundLayerActive(int layer)
{
    return s_background_tables.active(layer);
}

// splits a layer from the main Background table
void treeBackgroundSplitLayer(int layer)
{
    s_background_tables.split(layer);
}

// joins a layer to the main Background table
void treeBackgroundJoinLayer(int layer)
{
    s_background_tables.join(layer);
}

void treeBackgroundAddLayer(int layer, BackgroundRef_t Background)
{
    s_background_tables.add(layer, Background);
}

void treeBackgroundUpdateLayer(int layer, BackgroundRef_t Background)
{
    s_background_tables.update(layer, Background);
}

void treeBackgroundRemoveLayer(int layer, BackgroundRef_t Background)
{
    s_background_tables.erase(layer, Background);
}

TreeResult_Sentinel<BackgroundRef_t> treeBackgroundQuery(double Left, double Top, double Right, double Bottom,
                         int sort_mode,
                         double margin)
{
    return s_background_tables.query(Left, Top, Right, Bottom, sort_mode, margin);
}

TreeResult_Sentinel<BackgroundRef_t> treeBackgroundQuery(const Location_t &loc,
                         int sort_mode)
{
    return s_background_tables.query(loc, sort_mode);
}
