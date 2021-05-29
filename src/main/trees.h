/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2021 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef TREES_H
#define TREES_H

#include "../globals.h"
#include "../layers.h"

extern std::vector<void*> treeresult_vec[4];
extern ptrdiff_t cur_treeresult_vec;

template<class ItemT>
class TreeResult_Sentinel
{
public:
    struct it
    {
        using iterator_category = std::input_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = ItemT;
        using pointer           = ItemT*&;
        using reference         = ItemT*;

        reference operator*() const { return (ItemT*)*it_internal; }
        pointer operator->() { return &((ItemT*)*it_internal); }

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

        std::vector<void*>::iterator it_internal;
    };

    std::vector<void*>* i_vec = nullptr;

    TreeResult_Sentinel()
    {
        SDL_assert(cur_treeresult_vec >= 0); // invalid state
        SDL_assert(cur_treeresult_vec < 4); // insufficient sentinels: move recursive calls out of sentinel scope
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

extern void treeWorldTileAdd(Tile_t *obj);
extern void treeWorldTileUpdate(Tile_t *obj);
extern void treeWorldTileRemove(Tile_t *obj);
extern TreeResult_Sentinel<Tile_t> treeWorldTileQuery(double Left, double Top, double Right, double Bottom,
                               bool z_sort, double margin = 0.0);
extern TreeResult_Sentinel<Tile_t> treeWorldTileQuery(const Location_t &loc, bool z_sort, double margin = 0.0);


extern void treeWorldSceneAdd(Scene_t *obj);
extern void treeWorldSceneUpdate(Scene_t *obj);
extern void treeWorldSceneRemove(Scene_t *obj);
extern TreeResult_Sentinel<Scene_t> treeWorldSceneQuery(double Left, double Top, double Right, double Bottom,
                               bool z_sort, double margin = 16.0);
extern TreeResult_Sentinel<Scene_t> treeWorldSceneQuery(const Location_t &loc, bool z_sort, double margin = 16.0);


extern void treeWorldPathAdd(WorldPath_t *obj);
extern void treeWorldPathUpdate(WorldPath_t *obj);
extern void treeWorldPathRemove(WorldPath_t *obj);
extern TreeResult_Sentinel<WorldPath_t> treeWorldPathQuery(double Left, double Top, double Right, double Bottom,
                               bool z_sort, double margin = 16.0);
extern TreeResult_Sentinel<WorldPath_t> treeWorldPathQuery(const Location_t &loc, bool z_sort, double margin = 16.0);


extern void treeWorldLevelAdd(WorldLevel_t *obj);
extern void treeWorldLevelUpdate(WorldLevel_t *obj);
extern void treeWorldLevelRemove(WorldLevel_t *obj);
extern TreeResult_Sentinel<WorldLevel_t> treeWorldLevelQuery(double Left, double Top, double Right, double Bottom,
                               bool z_sort,
                               double margin = 16.0);
extern TreeResult_Sentinel<WorldLevel_t> treeWorldLevelQuery(const Location_t &loc, bool z_sort, double margin = 16.0);

extern void treeWorldMusicAdd(WorldMusic_t *obj);
extern void treeWorldMusicUpdate(WorldMusic_t *obj);
extern void treeWorldMusicRemove(WorldMusic_t *obj);
extern TreeResult_Sentinel<WorldMusic_t> treeWorldMusicQuery(double Left, double Top, double Right, double Bottom,
                               bool z_sort, double margin = 16.0);
extern TreeResult_Sentinel<WorldMusic_t> treeWorldMusicQuery(const Location_t &loc, bool z_sort, double margin = 16.0);


extern void treeBlockAddLayer(int layer, Block_t *obj);
extern void treeBlockRemoveLayer(int layer, Block_t *obj);
extern void treeBlockUpdateLayer(int layer, Block_t *obj);
extern TreeResult_Sentinel<Block_t> treeBlockQuery(double Left, double Top, double Right, double Bottom,
                               bool z_sort, double margin = 16.0);
extern TreeResult_Sentinel<Block_t> treeBlockQuery(const Location_t &loc, bool z_sort, double margin = 16.0);


extern void blockTileGet(const Location_t &loc, int64_t &fBlock, int64_t &lBlock);
extern void blockTileGet(double x, double w, int64_t &fBlock, int64_t &lBlock);

#endif // #ifndef TREES_H