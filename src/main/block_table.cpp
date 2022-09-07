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

#include "block_table.h"
#include "trees.h"
#include "layers.h"

BlockTable block_table[maxLayers+2];

void treeLevelCleanBlockLayers()
{
    for(int i = 0; i < maxLayers+2; i++)
        block_table[i].clear();
}

void treeBlockAddLayer(int layer, BlockRef_t block)
{
    if(layer < 0)
        layer = maxLayers + 1;
    block_table[layer].insert(block);
}

void treeBlockUpdateLayer(int layer, BlockRef_t block)
{
    if(layer < 0)
        layer = maxLayers + 1;
    block_table[layer].update(block);
}

void treeBlockRemoveLayer(int layer, BlockRef_t block)
{
    if(layer < 0)
        layer = maxLayers + 1;
    block_table[layer].erase(block);
}

TreeResult_Sentinel<BlockRef_t> treeBlockQuery(double Left, double Top, double Right, double Bottom,
                         int sort_mode,
                         double margin)
{
    TreeResult_Sentinel<BlockRef_t> result;

    for(int layer = 0; layer < maxLayers+2; layer++)
    {
        // skip empty layers except the tempBlock layer
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

        block_table[layer].query(loc, result);
    }

    if(sort_mode == SORTMODE_LOC)
    {
        std::sort(result.i_vec->begin(), result.i_vec->end(),
            [](void* a, void* b) {
                return (((BlockRef_t*)a)->Location.X < ((BlockRef_t*)b)->Location.X
                    || (((BlockRef_t*)a)->Location.X == ((BlockRef_t*)b)->Location.X
                        && ((BlockRef_t*)a)->Location.Y < ((BlockRef_t*)b)->Location.Y));
            });
    }
    else if(sort_mode == SORTMODE_ID)
    {
        std::sort(result.i_vec->begin(), result.i_vec->end(),
            [](void* a, void* b) {
                return a < b;
            });
    }
    else if(sort_mode == SORTMODE_Z)
    {
        std::sort(result.i_vec->begin(), result.i_vec->end(),
            [](void* a, void* b) {
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
