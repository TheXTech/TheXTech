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

#include <memory>
#include <algorithm>

#include "trees.h"
#include "layers.h"

#include "QuadTree/LooseQuadtree.h"


std::vector<BaseRef_t> treeresult_vec[MAX_TREEQUERY_DEPTH] = {std::vector<BaseRef_t>(400), std::vector<BaseRef_t>(400), std::vector<BaseRef_t>(50), std::vector<BaseRef_t>(50)};
ptrdiff_t cur_treeresult_vec = 0;

template<class ItemRef_t>
class Tree_Extractor
{
public:
    static void ExtractBoundingBox(const ItemRef_t object, loose_quadtree::BoundingBox<double> *bbox)
    {
        bbox->left      = object->Location.X;
        bbox->top       = object->Location.Y;
        bbox->width     = object->Location.Width;
        bbox->height    = object->Location.Height;
    }
};


template<>
class Tree_Extractor<BlockRef_t>
{
public:
    static void ExtractBoundingBox(const BlockRef_t object, loose_quadtree::BoundingBox<double> *bbox)
    {
        bbox->left      = object->Location.X;
        bbox->top       = object->Location.Y;
        bbox->width     = object->Location.Width;
        bbox->height    = object->Location.Height;

        if(object->Layer != LAYER_NONE)
        {
            bbox->left -= Layer[object->Layer].OffsetX;
            bbox->top -= Layer[object->Layer].OffsetY;
        }
    }
};

template<class ItemRef_t>
struct Tree_private
{
    typedef loose_quadtree::LooseQuadtree<double, ItemRef_t, Tree_Extractor<ItemRef_t>> IndexTreeQ;
    IndexTreeQ tree;
};

const double s_gridSize = 4;

static std::unique_ptr<Tree_private<TileRef_t>> s_worldTilesTree;
static std::unique_ptr<Tree_private<SceneRef_t>> s_worldSceneTree;
static std::unique_ptr<Tree_private<WorldPathRef_t>> s_worldPathTree;
static std::unique_ptr<Tree_private<WorldLevelRef_t>> s_worldLevelTree;
static std::unique_ptr<Tree_private<WorldMusicRef_t>> s_worldMusicTree;
static std::unique_ptr<Tree_private<BlockRef_t>> s_levelBlockTrees[maxLayers+2];
static std::unique_ptr<Tree_private<BlockRef_t>> s_tempBlockTree;

template<class Q>
void clearTree(Q &tree)
{
    if(tree.get())
    {
        tree->tree.Clear();
        tree.reset();
    }
}

void treeWorldCleanAll()
{
    clearTree(s_worldTilesTree);
    clearTree(s_worldSceneTree);
    clearTree(s_worldPathTree);
    clearTree(s_worldLevelTree);
    clearTree(s_worldMusicTree);
}

void treeLevelCleanAll()
{
    treeLevelCleanBlockLayers();
    treeLevelCleanBackgroundLayers();
    treeLevelCleanWaterLayers();
}

template<class ItemRef_t, class Arr>
void treeInsert(Arr &p, ItemRef_t obj)
{
    if(!p.get())
        p.reset(new Tree_private<ItemRef_t>());
    p->tree.Insert(obj);
}

template<class ItemRef_t, class Arr>
void treeUpdate(Arr &p, ItemRef_t obj)
{
    if(p.get())
        p->tree.Update(obj);
}

template<class ItemRef_t, class Arr>
void treeRemove(Arr &p, ItemRef_t obj)
{
    if(p.get())
        p->tree.Remove(obj);
}

template<class ItemRef_t>
TreeResult_Sentinel<ItemRef_t> treeWorldQuery(std::unique_ptr<Tree_private<ItemRef_t>> &p,
    double Left, double Top, double Right, double Bottom, int sort_mode)
{
    TreeResult_Sentinel<ItemRef_t> result;

    if(!p.get())
        return result;

    auto q = p->tree.QueryIntersectsRegion(loose_quadtree::BoundingBox<double>(Left - s_gridSize,
                                                                               Top - s_gridSize,
                                                                               (Right - Left) + s_gridSize * 2,
                                                                               (Bottom - Top) + s_gridSize * 2));

    while(!q.EndOfQuery())
    {
        ItemRef_t item = q.GetCurrent();
        result.i_vec->push_back((BaseRef_t)item);
        q.Next();
    }

    if(sort_mode == SORTMODE_ID)
    {
        std::sort(result.i_vec->begin(), result.i_vec->end(),
            [](BaseRef_t a, BaseRef_t b) {
                return a < b;
            });
    }
    else if(sort_mode == SORTMODE_LOC)
    {
        std::sort(result.i_vec->begin(), result.i_vec->end(),
            [](BaseRef_t a, BaseRef_t b) {
                return (((ItemRef_t)a)->Location.X < ((ItemRef_t)b)->Location.X
                    || (((ItemRef_t)a)->Location.X == ((ItemRef_t)b)->Location.X
                        && ((ItemRef_t)a)->Location.Y < ((ItemRef_t)b)->Location.Y));
            });
    }
    else if(sort_mode == SORTMODE_Z)
    {
        std::sort(result.i_vec->begin(), result.i_vec->end(),
            [](BaseRef_t a, BaseRef_t b) {
                return ((ItemRef_t)a)->Z < ((ItemRef_t)b)->Z;
            });
    }

    return result;
}


/* ================= Terrain Tile ================= */

void treeWorldTileAdd(TileRef_t obj)
{
    treeInsert(s_worldTilesTree, obj);
}

void treeWorldTileUpdate(TileRef_t obj)
{
    treeUpdate(s_worldTilesTree, obj);
}

void treeWorldTileRemove(TileRef_t obj)
{
    treeRemove(s_worldTilesTree, obj);
}

TreeResult_Sentinel<TileRef_t> treeWorldTileQuery(double Left, double Top, double Right, double Bottom, int sort_mode, double margin)
{
    return treeWorldQuery(s_worldTilesTree,
                   Left - margin,
                   Top - margin,
                   Right + margin,
                   Bottom + margin,
                   sort_mode);
}

TreeResult_Sentinel<TileRef_t> treeWorldTileQuery(const Location_t &loc, int sort_mode, double margin)
{
    return treeWorldQuery(s_worldTilesTree,
                   loc.X - margin,
                   loc.Y - margin,
                   loc.X + loc.Width + margin,
                   loc.Y + loc.Height + margin,
                   sort_mode);
}


/* ================= Scenery ================= */

void treeWorldSceneAdd(SceneRef_t obj)
{
    treeInsert(s_worldSceneTree, obj);
}

void treeWorldSceneUpdate(SceneRef_t obj)
{
    treeUpdate(s_worldSceneTree, obj);
}

void treeWorldSceneRemove(SceneRef_t obj)
{
    treeRemove(s_worldSceneTree, obj);
}

TreeResult_Sentinel<SceneRef_t> treeWorldSceneQuery(double Left, double Top, double Right, double Bottom, int sort_mode, double margin)
{
    return treeWorldQuery(s_worldSceneTree,
                   Left - margin,
                   Top - margin,
                   Right + margin,
                   Bottom + margin,
                   sort_mode);
}

TreeResult_Sentinel<SceneRef_t> treeWorldSceneQuery(const Location_t &loc, int sort_mode, double margin)
{
    return treeWorldQuery(s_worldSceneTree,
                   loc.X - margin,
                   loc.Y - margin,
                   loc.X + loc.Width + margin,
                   loc.Y + loc.Height + margin,
                   sort_mode);
}


/* ================= Paths ================= */

void treeWorldPathAdd(WorldPathRef_t obj)
{
    treeInsert(s_worldPathTree, obj);
}

void treeWorldPathUpdate(WorldPathRef_t obj)
{
    treeUpdate(s_worldPathTree, obj);
}

void treeWorldPathRemove(WorldPathRef_t obj)
{
    treeRemove(s_worldPathTree, obj);
}

TreeResult_Sentinel<WorldPathRef_t> treeWorldPathQuery(double Left, double Top, double Right, double Bottom,
                        int sort_mode, double margin)
{
    return treeWorldQuery(s_worldPathTree,
                   Left - margin,
                   Top - margin,
                   Right + margin,
                   Bottom + margin,
                   sort_mode);
}

TreeResult_Sentinel<WorldPathRef_t> treeWorldPathQuery(const Location_t &loc, int sort_mode, double margin)
{
    return treeWorldQuery(s_worldPathTree,
                   loc.X - margin,
                   loc.Y - margin,
                   loc.X + loc.Width + margin,
                   loc.Y + loc.Height + margin,
                   sort_mode);
}

/* ================= Levels ================= */

void treeWorldLevelAdd(WorldLevelRef_t obj)
{
    treeInsert(s_worldLevelTree, obj);
}

void treeWorldLevelUpdate(WorldLevelRef_t obj)
{
    treeUpdate(s_worldLevelTree, obj);
}

void treeWorldLevelRemove(WorldLevelRef_t obj)
{
    treeRemove(s_worldLevelTree, obj);
}

TreeResult_Sentinel<WorldLevelRef_t> treeWorldLevelQuery(double Left, double Top, double Right, double Bottom,
                         int sort_mode, double margin)
{
    return treeWorldQuery(s_worldLevelTree,
                   Left - margin,
                   Top - margin,
                   Right + margin,
                   Bottom + margin,
                   sort_mode);
}

TreeResult_Sentinel<WorldLevelRef_t> treeWorldLevelQuery(const Location_t &loc, int sort_mode, double margin)
{
    return treeWorldQuery(s_worldLevelTree,
                   loc.X - margin,
                   loc.Y - margin,
                   loc.X + loc.Width + margin,
                   loc.Y + loc.Height + margin,
                   sort_mode);
}


/* ================= Music ================= */

void treeWorldMusicAdd(WorldMusicRef_t obj)
{
    treeInsert(s_worldMusicTree, obj);
}

void treeWorldMusicUpdate(WorldMusicRef_t obj)
{
    treeUpdate(s_worldMusicTree, obj);
}

void treeWorldMusicRemove(WorldMusicRef_t obj)
{
    treeRemove(s_worldMusicTree, obj);
}

TreeResult_Sentinel<WorldMusicRef_t> treeWorldMusicQuery(double Left, double Top, double Right, double Bottom,
                         int sort_mode,
                         double margin)
{
    return treeWorldQuery(s_worldMusicTree,
                   Left - margin,
                   Top - margin,
                   Right + margin,
                   Bottom + margin,
                   sort_mode);
}

TreeResult_Sentinel<WorldMusicRef_t> treeWorldMusicQuery(const Location_t &loc,
                         int sort_mode,
                         double margin)
{
    return treeWorldQuery(s_worldMusicTree,
                   loc.X - margin,
                   loc.Y - margin,
                   loc.X + loc.Width + margin,
                   loc.Y + loc.Height + margin,
                   sort_mode);
}

/* ================= Tile block search ================= */
// removed in favor of block quadtree

#if 0
void blockTileGet(const Location_t &loc, int64_t &fBlock, int64_t &lBlock)
{
    int f = vb6Round(loc.X / 32) - 1;
    int l = vb6Round((loc.X + loc.Width) / 32.0) + 1;
    fBlock = FirstBlock[f < -FLBlocks ? -FLBlocks : f];
    lBlock = LastBlock[l > FLBlocks ? FLBlocks : l];
}

void blockTileGet(double x, double w, int64_t &fBlock, int64_t &lBlock)
{
    int f = vb6Round(x / 32) - 1;
    int l = vb6Round((x + w) / 32.0) + 1;
    fBlock = FirstBlock[f < -FLBlocks ? -FLBlocks : f];
    lBlock = LastBlock[l > FLBlocks ? FLBlocks : l];
}
#endif
