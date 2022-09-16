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

table_t<BlockRef_t> s_common_block_table;
table_t<BlockRef_t> s_temp_block_table;
table_t<BlockRef_t> s_layer_block_table[maxLayers+1];

bool g_layer_block_table_active[maxLayers+1] = {false};

int16_t s_active_block_tables[maxLayers+1] = {0};
int s_num_active_block_tables = 0;


table_t<BackgroundRef_t> background_table[maxLayers+2];

/* ================= Level blocks ================= */

void treeLevelCleanBlockLayers()
{
    s_common_block_table.clear();
    s_temp_block_table.clear();

    for(int i = 0; i < maxLayers + 1; i++)
    {
        s_layer_block_table[i].clear();
        g_layer_block_table_active[i] = false;
    }

    s_num_active_block_tables = 0;

    for(int i = 0; i < maxLayers + 1; i++)
    {
        treeBlockSplitLayer(i);
    }
}

// splits a layer from the main block table
void treeBlockSplitLayer(int layer)
{
    if(layer < 0 || layer == LAYER_NONE || g_layer_block_table_active[layer])
        return;

    g_layer_block_table_active[layer] = true;
    s_active_block_tables[s_num_active_block_tables] = layer;
    s_num_active_block_tables++;

    for(int b : Layer[layer].blocks)
    {
        s_common_block_table.erase(b);
        s_layer_block_table[layer].insert_layer(b);
    }
}

// joins a layer to the main block table
void treeBlockJoinLayer(int layer)
{
    if(layer < 0 || layer == LAYER_NONE || !g_layer_block_table_active[layer])
        return;

    g_layer_block_table_active[layer] = false;
    s_num_active_block_tables--;
    s_active_block_tables[layer] = s_active_block_tables[s_num_active_block_tables];

    for(int b : Layer[layer].blocks)
    {
        s_common_block_table.insert(b);
    }

    s_layer_block_table[layer].clear();
}

void treeBlockAddLayer(int layer, BlockRef_t block)
{
    if(layer < 0 || layer == LAYER_NONE || !g_layer_block_table_active[layer])
        s_common_block_table.insert(block);
    else
        s_layer_block_table[layer].insert_layer(block);
}

void treeBlockUpdateLayer(int layer, BlockRef_t block)
{
    if(layer < 0 || layer == LAYER_NONE || !g_layer_block_table_active[layer])
        s_common_block_table.update(block);
    else
        s_layer_block_table[layer].update_layer(block);
}

void treeBlockRemoveLayer(int layer, BlockRef_t block)
{
    if(layer < 0 || layer == LAYER_NONE || !g_layer_block_table_active[layer])
        s_common_block_table.erase(block);
    else
        s_layer_block_table[layer].erase(block);
}

TreeResult_Sentinel<BlockRef_t> treeBlockQuery(double Left, double Top, double Right, double Bottom,
                         int sort_mode,
                         double margin)
{
    TreeResult_Sentinel<BlockRef_t> result;

    Location_t loc = newLoc(Left - margin,
       Top - margin,
       (Right - Left) + margin * 2,
       (Bottom - Top) + margin * 2);

    s_common_block_table.query(*result.i_vec, loc);

    for(int i = 0; i < s_num_active_block_tables; i++)
    {
        int layer = s_active_block_tables[i];

        loc.X -= Layer[layer].OffsetX;
        loc.Y -= Layer[layer].OffsetY;

        s_layer_block_table[layer].query(*result.i_vec, loc);

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
                return (((BlockRef_t)a)->Location.X < ((BlockRef_t)b)->Location.X
                    || (((BlockRef_t)a)->Location.X == ((BlockRef_t)b)->Location.X
                        && ((BlockRef_t)a)->Location.Y < ((BlockRef_t)b)->Location.Y));
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

TreeResult_Sentinel<BlockRef_t> treeBlockQuery(const Location_t &loc,
                         int sort_mode,
                         double margin)
{
    return treeBlockQuery(loc.X,
                   loc.Y,
                   loc.X + loc.Width,
                   loc.Y + loc.Height, sort_mode, margin);
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

TreeResult_Sentinel<BlockRef_t> treeTempBlockQuery(double Left, double Top, double Right, double Bottom,
                         int sort_mode,
                         double margin)
{
    TreeResult_Sentinel<BlockRef_t> result;

    Location_t loc = newLoc(Left - margin,
               Top - margin,
               (Right - Left) + margin * 2,
               (Bottom - Top) + margin * 2);
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

TreeResult_Sentinel<BlockRef_t> treeTempBlockQuery(const Location_t &loc,
                         int sort_mode,
                         double margin)
{
    return treeTempBlockQuery(loc.X,
                   loc.Y,
                   loc.X + loc.Width,
                   loc.Y + loc.Height, sort_mode, margin);
}

/* ================= Level Backgrounds ================= */

void treeLevelCleanBackgroundLayers()
{
    for(int i = 0; i < maxLayers+2; i++)
        background_table[i].clear();
}

void treeBackgroundAddLayer(int layer, BackgroundRef_t bgo)
{
    if(layer < 0 || layer == LAYER_NONE)
        layer = maxLayers + 1;
    background_table[layer].insert(bgo);
}

void treeBackgroundUpdateLayer(int layer, BackgroundRef_t bgo)
{
    if(layer < 0 || layer == LAYER_NONE)
        layer = maxLayers + 1;
    background_table[layer].update(bgo);
}

void treeBackgroundRemoveLayer(int layer, BackgroundRef_t bgo)
{
    if(layer < 0 || layer == LAYER_NONE)
        layer = maxLayers + 1;
    background_table[layer].erase(bgo);
}

TreeResult_Sentinel<BackgroundRef_t> treeBackgroundQuery(double Left, double Top, double Right, double Bottom,
                         int sort_mode,
                         double margin)
{
    TreeResult_Sentinel<BackgroundRef_t> result;

    for(int layer = 0; layer < maxLayers + 2; layer++)
    {
        // skip empty layers except LAYER_NONE
        if(layer > numLayers && layer != maxLayers + 1)
            layer = maxLayers + 1;

        double OffsetX, OffsetY;
        if(layer == maxLayers + 1)
        {
            OffsetX = OffsetY = 0.0;
        }
        else
        {
            OffsetX = Layer[layer].OffsetX;
            OffsetY = Layer[layer].OffsetY;
        }

        Location_t loc = newLoc(Left - OffsetX - margin,
           Top - OffsetY - margin,
           (Right - Left) + margin * 2,
           (Bottom - Top) + margin * 2);

        background_table[layer].query(*result.i_vec, loc);
    }

    if(sort_mode == SORTMODE_LOC)
    {
        std::sort(result.i_vec->begin(), result.i_vec->end(),
            [](BaseRef_t a, BaseRef_t b) {
                return (((BackgroundRef_t)a)->Location.X < ((BackgroundRef_t)b)->Location.X
                    || (((BackgroundRef_t)a)->Location.X == ((BackgroundRef_t)b)->Location.X
                        && ((BackgroundRef_t)a)->Location.Y < ((BackgroundRef_t)b)->Location.Y));
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

TreeResult_Sentinel<BackgroundRef_t> treeBackgroundQuery(const Location_t &loc,
                         int sort_mode,
                         double margin)
{
    return treeBackgroundQuery(loc.X,
                   loc.Y,
                   loc.X + loc.Width,
                   loc.Y + loc.Height, sort_mode, margin);
}
