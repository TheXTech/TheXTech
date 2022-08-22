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

#pragma once
#ifndef TREES_HHHH
#define TREES_HHHH

#include "globals.h"

#define MAX_TREEQUERY_DEPTH 4
extern std::vector<BaseRef_t> treeresult_vec[MAX_TREEQUERY_DEPTH];
extern ptrdiff_t cur_treeresult_vec;

enum SortMode
{
    SORTMODE_NONE = 0,
    SORTMODE_ID = 1,
    SORTMODE_LOC = 2,
    SORTMODE_Z = 2,
};

template<class ItemRef_t>
class TreeResult_Sentinel
{
public:
    struct it
    {
        using iterator_category = std::input_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = typename ItemRef_t::value_type;
        using pointer           = ItemRef_t*;
        using reference         = ItemRef_t;

        reference operator*() const { return (reference)*it_internal; }
        pointer operator->() { return &((reference)*it_internal); }

        // Prefix increment
        it& operator++() { it_internal++; return *this; }

        // Postfix increment
        it operator++(int) { it tmp = *this; ++(*this); return tmp; }

        // Prefix decrement
        it& operator--() { it_internal--; return *this; }

        // Postfix decrement
        it operator--(int) { it tmp = *this; --(*this); return tmp; }

        friend bool operator== (const it& a, const it& b) { return a.it_internal == b.it_internal; };
        friend bool operator!= (const it& a, const it& b) { return a.it_internal != b.it_internal; };
        friend bool operator<= (const it& a, const it& b) { return a.it_internal <= b.it_internal; };
        friend bool operator>= (const it& a, const it& b) { return a.it_internal >= b.it_internal; };
        friend bool operator< (const it& a, const it& b) { return a.it_internal < b.it_internal; };
        friend bool operator> (const it& a, const it& b) { return a.it_internal > b.it_internal; };

        std::vector<BaseRef_t>::iterator it_internal;
    };

    std::vector<BaseRef_t>* i_vec = nullptr;

    TreeResult_Sentinel()
    {
        SDL_assert(cur_treeresult_vec >= 0); // invalid state
        SDL_assert_release(cur_treeresult_vec < MAX_TREEQUERY_DEPTH); // insufficient sentinels: move recursive calls out of sentinel scope
        i_vec = &treeresult_vec[cur_treeresult_vec];
        i_vec->clear();
        cur_treeresult_vec ++;
    }

    TreeResult_Sentinel(const TreeResult_Sentinel& other)
    {
        SDL_assert(false); // can't have two sentinels for same results
    }

    TreeResult_Sentinel(TreeResult_Sentinel&& other)
    {
        i_vec = other.i_vec;
        other.i_vec = nullptr;
    }

    it begin() const
    {
        SDL_assert(i_vec); // invalid use of discarded sentinel
        it ret;
        ret.it_internal = i_vec->begin();
        return ret;
    }

    it end() const
    {
        SDL_assert(i_vec); // invalid use of discarded sentinel
        it ret;
        ret.it_internal = i_vec->end();
        return ret;
    }

    ~TreeResult_Sentinel()
    {
        if(!i_vec)
            return;
        cur_treeresult_vec --;
        SDL_assert(cur_treeresult_vec == i_vec - &treeresult_vec[0]); // scopes have been switched
    }
};

extern void treeWorldCleanAll();
extern void treeLevelCleanBlockLayers();
extern void treeLevelCleanAll();

extern void treeWorldTileAdd(TileRef_t obj);
extern void treeWorldTileUpdate(TileRef_t obj);
extern void treeWorldTileRemove(TileRef_t obj);
extern TreeResult_Sentinel<TileRef_t> treeWorldTileQuery(double Left, double Top, double Right, double Bottom,
                               int sort_mode, double margin = 0.0);
extern TreeResult_Sentinel<TileRef_t> treeWorldTileQuery(const Location_t &loc, int sort_mode, double margin = 0.0);


extern void treeWorldSceneAdd(SceneRef_t obj);
extern void treeWorldSceneUpdate(SceneRef_t obj);
extern void treeWorldSceneRemove(SceneRef_t obj);
extern TreeResult_Sentinel<SceneRef_t> treeWorldSceneQuery(double Left, double Top, double Right, double Bottom,
                               int sort_mode, double margin = 16.0);
extern TreeResult_Sentinel<SceneRef_t> treeWorldSceneQuery(const Location_t &loc, int sort_mode, double margin = 16.0);


extern void treeWorldPathAdd(WorldPathRef_t obj);
extern void treeWorldPathUpdate(WorldPathRef_t obj);
extern void treeWorldPathRemove(WorldPathRef_t obj);
extern TreeResult_Sentinel<WorldPathRef_t> treeWorldPathQuery(double Left, double Top, double Right, double Bottom,
                               int sort_mode, double margin = 16.0);
extern TreeResult_Sentinel<WorldPathRef_t> treeWorldPathQuery(const Location_t &loc, int sort_mode, double margin = 16.0);


extern void treeWorldLevelAdd(WorldLevelRef_t obj);
extern void treeWorldLevelUpdate(WorldLevelRef_t obj);
extern void treeWorldLevelRemove(WorldLevelRef_t obj);
extern TreeResult_Sentinel<WorldLevelRef_t> treeWorldLevelQuery(double Left, double Top, double Right, double Bottom,
                               int sort_mode, double margin = 16.0);
extern TreeResult_Sentinel<WorldLevelRef_t> treeWorldLevelQuery(const Location_t &loc, int sort_mode, double margin = 16.0);

extern void treeWorldMusicAdd(WorldMusicRef_t obj);
extern void treeWorldMusicUpdate(WorldMusicRef_t obj);
extern void treeWorldMusicRemove(WorldMusicRef_t obj);
extern TreeResult_Sentinel<WorldMusicRef_t> treeWorldMusicQuery(double Left, double Top, double Right, double Bottom,
                               int sort_mode, double margin = 16.0);
extern TreeResult_Sentinel<WorldMusicRef_t> treeWorldMusicQuery(const Location_t &loc, int sort_mode, double margin = 16.0);


extern void treeBlockAddLayer(int layer, BlockRef_t obj);
extern void treeBlockRemoveLayer(int layer, BlockRef_t obj);
extern void treeBlockUpdateLayer(int layer, BlockRef_t obj);
extern TreeResult_Sentinel<BlockRef_t> treeBlockQuery(double Left, double Top, double Right, double Bottom,
                               int sort_mode, double margin = 16.0);
extern TreeResult_Sentinel<BlockRef_t> treeBlockQuery(const Location_t &loc, int sort_mode, double margin = 16.0);

extern void treeTempBlockStartFrame();
extern void treeTempBlockAdd(BlockRef_t obj);
extern void treeTempBlockUpdate(BlockRef_t obj);
extern TreeResult_Sentinel<BlockRef_t> treeTempBlockQuery(double Left, double Top, double Right, double Bottom,
                               int sort_mode, double margin = 16.0);
extern TreeResult_Sentinel<BlockRef_t> treeTempBlockQuery(const Location_t &loc, int sort_mode, double margin = 16.0);

// removed in favor of block quadtree

// extern void blockTileGet(const Location_t &loc, int64_t &fBlock, int64_t &lBlock);
// extern void blockTileGet(double x, double w, int64_t &fBlock, int64_t &lBlock);

#endif // TREES_HHHH
