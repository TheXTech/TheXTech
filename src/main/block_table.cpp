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

#include <algorithm>
#include <sorting/pdqsort.h>

#include "layers.h"
#include "config.h"

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

    const std::vector<vbint_t>& layer_items(int layer);

    // checks if a layer is currently split from the main table
    bool active(int layer)
    {
        return layer_table_active[layer];
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

    inline void query(std::vector<BaseRef_t>& out, const Location_t& _loc, int sort_mode)
    {
        // ignore improper rects
        if(_loc.Width < 0 || _loc.Height < 0)
            return;

        rect_external loc;

        // NOTE: there are extremely rare cases when these margins are not sufficient for full compatibility
        //   (such as, when an item is trapped inside a wall during !BlocksSorted)
        // This should be fixed for players now.
        // Conversely, there may be cases where this is too much when BlocksSorted is true and an FLBlock query was made.
        if(g_config.emulate_classic_block_order)
            loc.query_from_loc_paranoid(_loc);
        else
        {
            int32_t l_int = num_t::floor(_loc.X);
            int32_t r_int = num_t::ceil(_loc.X + _loc.Width);
            int32_t t_int = num_t::floor(_loc.Y);
            int32_t b_int = num_t::ceil(_loc.Y + _loc.Height);

            // workaround to handle common block push behavior of NPCs -- if NPCs eventually use updatable queries, then we can simply use query_from_loc_standard as in other callsites
            if(r_int < l_int + 32)
                r_int = l_int + 32;

            // find which 64x64 tiles contain the query with a 2px margin in each direction
            loc.l = s_floor_div_64(l_int - 2);
            loc.r = s_floor_div_64(r_int + 2 + 63);
            loc.t = s_floor_div_64(t_int - 2);
            loc.b = s_floor_div_64(b_int + 2 + 63);
        }

        common_table.query(out, loc);

        // bigger offset for r/b because of layer offset rounding issues
        if(num_active_tables > 0)
        {
            loc.r += 1;
            loc.b += 1;

            for(int i = 0; i < num_active_tables; i++)
            {
                int layer = active_tables[i];

                int16_t offX = s_floor_div_64(num_t::floor(-Layer[layer].OffsetX));
                int16_t offY = s_floor_div_64(num_t::floor(-Layer[layer].OffsetY));

                rect_external layer_loc{(int16_t)(loc.l + offX), (int16_t)(loc.r + offX), (int16_t)(loc.t + offY), (int16_t)(loc.b + offY)};

                layer_table[layer].query(out, layer_loc);
            }
        }

        if(sort_mode == SORTMODE_COMPAT)
        {
            if(g_config.emulate_classic_block_order)
                sort_mode = SORTMODE_ID;
            else
                sort_mode = SORTMODE_LOC;
        }

        if(sort_mode == SORTMODE_LOC)
            pdqsort(out.begin(), out.end(), Comparisons::Loc<ItemRef_t>);
        else if(sort_mode == SORTMODE_ID)
            trees_sort_by_index(out);
        else if(sort_mode == SORTMODE_Z)
            pdqsort(out.begin(), out.end(), Comparisons::Z<ItemRef_t>);
    }

    void query(std::vector<BaseRef_t>& out, num_t Left, num_t Top, num_t Right, num_t Bottom,
               int sort_mode, num_t margin)
    {
        auto loc = newLoc(Left - margin,
                          Top - margin,
                          (Right - Left) + margin * 2,
                          (Bottom - Top) + margin * 2);

        query(out, loc, sort_mode);
    }

    TreeResult_Sentinel<ItemRef_t> query(Location_t loc,
                             int sort_mode)
    {
        TreeResult_Sentinel<ItemRef_t> result;

        query(*result.i_vec, loc, sort_mode);

        return result;
    }

    TreeResult_Sentinel<ItemRef_t> query(num_t Left, num_t Top, num_t Right, num_t Bottom,
                             int sort_mode, num_t margin)
    {
        auto loc = newLoc(Left - margin,
                          Top - margin,
                          (Right - Left) + margin * 2,
                          (Bottom - Top) + margin * 2);

        return query(loc, sort_mode);
    }
};

table_t<BlockRef_t> s_temp_block_table;
table_t<NPCRef_t> s_npc_table;
bool s_temp_blocks_enabled = false;


/* ================= Level blocks ================= */

template<>
const std::vector<vbint_t>& TableInterface<BlockRef_t>::layer_items(int layer)
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

TreeResult_Sentinel<BlockRef_t> treeFLBlockQuery(num_t Left, num_t Top, num_t Right, num_t Bottom,
                         int sort_mode,
                         num_t margin)
{
    return s_block_tables.query(Left, Top, Right, Bottom, sort_mode, margin);
}

void treeFLBlockQuery(std::vector<BaseRef_t>& out, const Location_t &loc, int sort_mode)
{
    s_block_tables.query(out, loc, sort_mode);
}

TreeResult_Sentinel<BlockRef_t> treeFLBlockQuery(const Location_t &loc,
                         int sort_mode)
{
    return s_block_tables.query(loc, sort_mode);
}

/* ================= Temp blocks ================= */

static void s_NPCsToTempBlocks(std::vector<BaseRef_t>& out, size_t begin)
{
    for(size_t i = begin; i != out.size();)
    {
        // some NPCs' tempBlocks are at a different place than their location,
        // and those tempBlocks get added to the dedicated tree.

        const NPC_t& n = (NPCRef_t) out[i];
        if(!n.tempBlockInTree && n.tempBlock != 0 && n.tempBlock <= numBlock)
        {
            out[i] = n.tempBlock;
            ++i;
        }
        else
        {
            out[i] = out[out.size() - 1];
            out.resize(out.size() - 1);
        }
    }
}

static void s_NPCsToTempBlocks(std::vector<BaseRef_t>& out)
{
    s_NPCsToTempBlocks(out, 0);
}


void treeTempBlockEnable()
{
    s_temp_blocks_enabled = true;
}

void treeTempBlockFullClear()
{
    s_temp_block_table.clear();
    s_temp_blocks_enabled = false;
}

void treeTempBlockClear()
{
    s_temp_block_table.clear_light();
    s_temp_blocks_enabled = false;
}

void treeTempBlockAdd(BlockRef_t obj)
{
    if(!s_temp_blocks_enabled)
        return;

    s_temp_block_table.insert(obj);
}

void treeTempBlockUpdate(BlockRef_t obj)
{
    if(!s_temp_blocks_enabled)
        return;

    s_temp_block_table.update(obj);
}

void treeTempBlockRemove(BlockRef_t obj)
{
    if(!s_temp_blocks_enabled)
        return;

    s_temp_block_table.erase(obj);
}

void treeTempBlockQuery(std::vector<BaseRef_t>& out,
                        const Location_t &_loc,
                        int sort_mode)
{
    if(!s_temp_blocks_enabled)
        return;

    // ignore improper rects
    if(_loc.Width < 0 || _loc.Height < 0)
        return;

    rect_external loc;

    // NOTE: there are extremely rare cases when these margins are not sufficient for full compatibility
    //   (such as, when an item is trapped inside a wall during !BlocksSorted)
    if(g_config.emulate_classic_block_order)
        loc.query_from_loc_paranoid(_loc);
    else
        loc.query_from_loc_standard(_loc);


    s_npc_table.query(out, loc);

    s_NPCsToTempBlocks(out);

    // this table is empty unless an NPC crossed a boundary during this frame
    if(s_temp_block_table.member_rects.size() != 0)
        s_temp_block_table.query(out, loc);


    if(sort_mode == SORTMODE_COMPAT)
    {
        if(g_config.emulate_classic_block_order)
            sort_mode = SORTMODE_ID;
        else
            sort_mode = SORTMODE_LOC;
    }

    if(sort_mode == SORTMODE_LOC)
        pdqsort(out.begin(), out.end(), Comparisons::Loc<BlockRef_t>);
    else if(sort_mode == SORTMODE_ID)
        trees_sort_by_index(out);
    else if(sort_mode == SORTMODE_Z)
        pdqsort(out.begin(), out.end(), Comparisons::Z<BlockRef_t>);
}

TreeResult_Sentinel<BlockRef_t> treeTempBlockQuery(const Location_t &_loc,
                        int sort_mode)
{
    TreeResult_Sentinel<BlockRef_t> result;

    treeTempBlockQuery(*result.i_vec, _loc, sort_mode);

    return result;
}

TreeResult_Sentinel<BlockRef_t> treeTempBlockQuery(num_t Left, num_t Top, num_t Right, num_t Bottom,
                         int sort_mode,
                         num_t margin)
{
    auto loc = newLoc(Left - margin,
                      Top - margin,
                      (Right - Left) + margin * 2,
                      (Bottom - Top) + margin * 2);

    return treeTempBlockQuery(loc, sort_mode);
}

/* ================= Combined Block Query ============== */

TreeResult_Sentinel<BlockRef_t> treeBlockQuery(const Location_t &_loc,
                         int sort_mode)
{
    TreeResult_Sentinel<BlockRef_t> result;

    // ignore improper rects
    if(_loc.Width < 0 || _loc.Height < 0)
        return result;

    Location_t loc = _loc;

    // NOTE: there are extremely rare cases when these margins are not sufficient for full compatibility
    //   (such as, when an item is trapped inside a wall during !BlocksSorted)
    if(g_config.emulate_classic_block_order)
    {
        loc.X -= 32;
        loc.Y -= 32;
        loc.Width += 64;
        loc.Height += 64;
    }
    else
    {
        loc.X -= 2;
        loc.Y -= 2;
        loc.Width += 4;
        loc.Height += 4;
    }


    s_block_tables.common_table.query(*result.i_vec, loc);

    num_t oX = loc.X;
    num_t oY = loc.Y;

    for(int i = 0; i < s_block_tables.num_active_tables; i++)
    {
        int layer = s_block_tables.active_tables[i];

        loc.X -= Layer[layer].OffsetX;
        loc.Y -= Layer[layer].OffsetY;

        s_block_tables.layer_table[layer].query(*result.i_vec, loc);

        loc.X = oX;
        loc.Y = oY;
    }


    // this is where this function differs from the standard TableInterface::query
    if(s_temp_blocks_enabled)
    {
        auto pre_temp_size = result.i_vec->size();

        s_npc_table.query(*result.i_vec, loc);

        s_NPCsToTempBlocks(*result.i_vec, pre_temp_size);

        s_temp_block_table.query(*result.i_vec, loc);
    }


    if(sort_mode == SORTMODE_COMPAT)
    {
        if(g_config.emulate_classic_block_order)
            sort_mode = SORTMODE_ID;
        else
            sort_mode = SORTMODE_LOC;
    }

    if(sort_mode == SORTMODE_LOC)
        pdqsort(result.i_vec->begin(), result.i_vec->end(), Comparisons::Loc<BlockRef_t>);
    else if(sort_mode == SORTMODE_ID)
        trees_sort_by_index(*result.i_vec);
    else if(sort_mode == SORTMODE_Z)
        pdqsort(result.i_vec->begin(), result.i_vec->end(), Comparisons::Z<BlockRef_t>);

    return result;
}

/* ================= Level Backgrounds ================= */

template<>
const std::vector<vbint_t>& TableInterface<BackgroundRef_t>::layer_items(int layer)
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

void treeBackgroundAddLayer(int layer, BackgroundRef_t obj)
{
    s_background_tables.add(layer, obj);
}

void treeBackgroundUpdateLayer(int layer, BackgroundRef_t obj)
{
    s_background_tables.update(layer, obj);
}

void treeBackgroundRemoveLayer(int layer, BackgroundRef_t obj)
{
    s_background_tables.erase(layer, obj);
}

TreeResult_Sentinel<BackgroundRef_t> treeBackgroundQuery(num_t Left, num_t Top, num_t Right, num_t Bottom,
                         int sort_mode,
                         num_t margin)
{
    return s_background_tables.query(Left, Top, Right, Bottom, sort_mode, margin);
}

void treeBackgroundQuery(std::vector<BaseRef_t>& out, const Location_t &loc, int sort_mode)
{
    s_background_tables.query(out, loc, sort_mode);
}

TreeResult_Sentinel<BackgroundRef_t> treeBackgroundQuery(const Location_t &loc, int sort_mode)
{
    return s_background_tables.query(loc, sort_mode);
}


/* ================= Level NPCs ================= */

void treeNPCClear()
{
    s_npc_table.clear();
}

void treeNPCAdd(NPCRef_t obj)
{
    s_npc_table.insert(obj);
}

bool treeNPCUpdate(NPCRef_t obj)
{
    SDL_assert_release((int)obj > 0);

    return s_npc_table.update(obj);
}

void treeNPCSplitTempBlock(NPCRef_t obj)
{
    if(!obj->tempBlockInTree)
    {
        obj->tempBlockInTree = true;
        treeTempBlockAdd(obj->tempBlock);
    }
}

void treeNPCUpdateTempBlock(NPCRef_t obj)
{
    if(!obj->tempBlockInTree)
    {
        obj->tempBlockInTree = true;
        treeTempBlockAdd(obj->tempBlock);
    }
    else
    {
        treeTempBlockUpdate(obj->tempBlock);
    }
}

void treeNPCRemove(NPCRef_t obj)
{
    s_npc_table.erase(obj);
}

void treeNPCQuery(std::vector<BaseRef_t>& out, const Location_t &_loc, int sort_mode)
{
    // ignore improper rects
    if(_loc.Width < 0 || _loc.Height < 0)
        return;

    rect_external loc;

    // NOTE: there are extremely rare cases when these margins are not sufficient for full compatibility
    //   (such as, when an item is trapped inside a wall during !BlocksSorted)
    if(g_config.emulate_classic_block_order)
        loc.query_from_loc_paranoid(_loc);
    else
        loc.query_from_loc_standard(_loc);

    s_npc_table.query(out, loc);

    if(sort_mode == SORTMODE_COMPAT)
        sort_mode = SORTMODE_ID;

    if(sort_mode == SORTMODE_LOC)
        pdqsort(out.begin(), out.end(), Comparisons::Loc<NPCRef_t>);
    else if(sort_mode == SORTMODE_ID)
        trees_sort_by_index(out);
    else if(sort_mode == SORTMODE_Z)
        pdqsort(out.begin(), out.end(), Comparisons::Z<NPCRef_t>);
}

TreeResult_Sentinel<NPCRef_t> treeNPCQuery(const Location_t &_loc,
                         int sort_mode)
{
    TreeResult_Sentinel<NPCRef_t> result;

    treeNPCQuery(*result.i_vec, _loc, sort_mode);

    return result;
}

TreeResult_Sentinel<NPCRef_t> treeNPCQuery(num_t Left, num_t Top, num_t Right, num_t Bottom,
                         int sort_mode,
                         num_t margin)
{
    auto loc = newLoc(Left - margin,
                      Top - margin,
                      (Right - Left) + margin * 2,
                      (Bottom - Top) + margin * 2);

    return treeNPCQuery(loc, sort_mode);
}


/* ================= Level PEZs ================= */

template<>
const std::vector<vbint_t>& TableInterface<WaterRef_t>::layer_items(int layer)
{
    return Layer[layer].waters;
}

TableInterface<WaterRef_t> s_water_tables;

void treeLevelCleanWaterLayers()
{
    s_water_tables.clear();
}

// checks if a layer is split from the main Water table
bool treeWaterLayerActive(int layer)
{
    return s_water_tables.active(layer);
}

// splits a layer from the main Water table
void treeWaterSplitLayer(int layer)
{
    s_water_tables.split(layer);
}

// joins a layer to the main Water table
void treeWaterJoinLayer(int layer)
{
    s_water_tables.join(layer);
}

void treeWaterAddLayer(int layer, WaterRef_t obj)
{
    s_water_tables.add(layer, obj);
}

void treeWaterUpdateLayer(int layer, WaterRef_t obj)
{
    s_water_tables.update(layer, obj);
}

void treeWaterRemoveLayer(int layer, WaterRef_t obj)
{
    s_water_tables.erase(layer, obj);
}

TreeResult_Sentinel<WaterRef_t> treeWaterQuery(num_t Left, num_t Top, num_t Right, num_t Bottom,
                         int sort_mode,
                         num_t margin)
{
    if(numWater == 0)
    {
        TreeResult_Sentinel<WaterRef_t> empty;
        return empty;
    }

    return s_water_tables.query(Left, Top, Right, Bottom, sort_mode, margin);
}

TreeResult_Sentinel<WaterRef_t> treeWaterQuery(const Location_t &loc,
                         int sort_mode)
{
    if(numWater == 0)
    {
        TreeResult_Sentinel<WaterRef_t> empty;
        return empty;
    }

    return s_water_tables.query(loc, sort_mode);
}
