/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2021 Vitaly Novichkov <admin@wohlnet.ru>
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
#include "trees.h"
#include "QuadTree/LooseQuadtree.h"

template<class WorldItemT>
class WorldTree_Extractor
{
public:
    static void ExtractBoundingBox(const WorldItemT *object, loose_quadtree::BoundingBox<double> *bbox)
    {
        bbox->left      = object->Location.X;
        bbox->top       = object->Location.Y;
        bbox->width     = object->Location.Width;
        bbox->height    = object->Location.Height;
    }
};

template<class WorldItemT>
struct WorldTree_private
{
    typedef loose_quadtree::LooseQuadtree<double, WorldItemT, WorldTree_Extractor<WorldItemT>> IndexTreeQ;
    IndexTreeQ tree;
};

const double s_gridSize = 4;

static std::unique_ptr<WorldTree_private<Tile_t>> s_worldTilesTree;
static std::unique_ptr<WorldTree_private<Scene_t>> s_worldSceneTree;
static std::unique_ptr<WorldTree_private<WorldPath_t>> s_worldPathTree;
static std::unique_ptr<WorldTree_private<WorldLevel_t>> s_worldLevelTree;
static std::unique_ptr<WorldTree_private<WorldMusic_t>> s_worldMusicTree;

template<class WorldItemT>
void sortElements(std::vector<WorldItemT*> &list)
{
    if(list.size() <= 1)
        return; //Nothing to sort!

#define S(x) (static_cast<size_t>(x))

    std::vector<int64_t> beg;
    std::vector<int64_t> end;
    beg.reserve(list.size());
    end.reserve(list.size());

    WorldItemT *piv;
    int64_t i = 0;
    int64_t L, R, swapv;
    beg.push_back(0);
    end.push_back(static_cast<int64_t>(list.size()));

    while(i >= 0)
    {
        L = beg[S(i)];
        R = end[S(i)] - 1;

        if(L < R)
        {
            piv = list[S(L)];
            while(L < R)
            {
                while((list[S(R)]->Z >= piv->Z) && (L < R)) R--;
                if(L < R) list[S(L++)] = std::move(list[S(R)]);

                while((list[S(L)]->Z <= piv->Z) && (L < R)) L++;
                if(L < R) list[S(R--)] = std::move(list[S(L)]);
            }

            list[S(L)] = piv;
            beg.push_back(L + 1);
            end.push_back(end[S(i)]);
            end[S(i++)] = (L);

            if((end[i] - beg[S(i)]) > (end[i - 1] - beg[S(i - 1)]))
            {
                swapv = beg[S(i)];
                beg[S(i)] = beg[S(i - 1)];
                beg[S(i - 1)] = swapv;
                swapv = end[S(i)];
                end[S(i)] = end[S(i - 1)];
                end[S(i - 1)] = swapv;
            }
        }
        else
        {
            i--;
            beg.pop_back();
            end.pop_back();
        }
    }

#undef S
}

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


template<class Obj, class Arr>
void treeInsert(Arr &p, Obj*obj)
{
    if(!p.get())
        p.reset(new WorldTree_private<Obj>());
    p->tree.Insert(obj);
}

template<class Obj, class Arr>
void treeUpdate(Arr &p, Obj*obj)
{
    if(p.get())
        p->tree.Update(obj);
}

template<class Obj, class Arr>
void treeRemove(Arr &p, Obj*obj)
{
    if(p.get())
        p->tree.Remove(obj);
}

template<class Obj, class Arr>
void treeWorldQuery(Arr &p, double Left, double Top, double Right, double Bottom, std::vector<Obj *> &list, bool z_sort)
{
    list.clear();

    if(!p.get())
        return;

    auto q = p->tree.QueryIntersectsRegion(loose_quadtree::BoundingBox<double>(Left - s_gridSize,
                                                                               Top - s_gridSize,
                                                                               (Right - Left) + s_gridSize * 2,
                                                                               (Bottom - Top) + s_gridSize * 2));
    while(!q.EndOfQuery())
    {
        auto *item = q.GetCurrent();
        if(item)
            list.push_back(q.GetCurrent());
        q.Next();
    }

    if(z_sort)
        sortElements(list);
}


/* ================= Terrain Tile ================= */

void treeWorldTileAdd(Tile_t *obj)
{
    treeInsert(s_worldTilesTree, obj);
}

void treeWorldTileUpdate(Tile_t *obj)
{
    treeUpdate(s_worldTilesTree, obj);
}

void treeWorldTileRemove(Tile_t *obj)
{
    treeRemove(s_worldTilesTree, obj);
}

void treeWorldTileQuery(double Left, double Top, double Right, double Bottom, TilePtrArr &list, bool z_sort, double margin)
{
    treeWorldQuery(s_worldTilesTree,
                   Left - margin,
                   Top - margin,
                   Right + margin,
                   Bottom + margin,
                   list, z_sort);
}

void treeWorldTileQuery(const Location_t &loc, TilePtrArr &list, bool z_sort, double margin)
{
    treeWorldQuery(s_worldTilesTree,
                   loc.X - margin,
                   loc.Y - margin,
                   loc.X + loc.Width + margin,
                   loc.Y + loc.Height + margin,
                   list, z_sort);
}


/* ================= Scenery ================= */

void treeWorldSceneAdd(Scene_t *obj)
{
    treeInsert(s_worldSceneTree, obj);
}

void treeWorldSceneUpdate(Scene_t *obj)
{
    treeUpdate(s_worldSceneTree, obj);
}

void treeWorldSceneRemove(Scene_t *obj)
{
    treeRemove(s_worldSceneTree, obj);
}

void treeWorldSceneQuery(double Left, double Top, double Right, double Bottom, ScenePtrArr &list, bool z_sort, double margin)
{
    treeWorldQuery(s_worldSceneTree,
                   Left - margin,
                   Top - margin,
                   Right + margin,
                   Bottom + margin,
                   list, z_sort);
}

void treeWorldSceneQuery(const Location_t &loc, ScenePtrArr &list, bool z_sort, double margin)
{
    treeWorldQuery(s_worldSceneTree,
                   loc.X - margin,
                   loc.Y - margin,
                   loc.X + loc.Width + margin,
                   loc.Y + loc.Height + margin,
                   list, z_sort);
}


/* ================= Paths ================= */

void treeWorldPathAdd(WorldPath_t *obj)
{
    treeInsert(s_worldPathTree, obj);
}

void treeWorldPathUpdate(WorldPath_t *obj)
{
    treeUpdate(s_worldPathTree, obj);
}

void treeWorldPathRemove(WorldPath_t *obj)
{
    treeRemove(s_worldPathTree, obj);
}

void treeWorldPathQuery(double Left, double Top, double Right, double Bottom,
                        WorldPathPtrArr &list,
                        bool z_sort, double margin)
{
    treeWorldQuery(s_worldPathTree,
                   Left - margin,
                   Top - margin,
                   Right + margin,
                   Bottom + margin,
                   list, z_sort);
}

void treeWorldPathQuery(const Location_t &loc, WorldPathPtrArr &list, bool z_sort, double margin)
{
    treeWorldQuery(s_worldPathTree,
                   loc.X - margin,
                   loc.Y - margin,
                   loc.X + loc.Width + margin,
                   loc.Y + loc.Height + margin,
                   list, z_sort);
}

/* ================= Levels ================= */

void treeWorldLevelAdd(WorldLevel_t *obj)
{
    treeInsert(s_worldLevelTree, obj);
}

void treeWorldLevelUpdate(WorldLevel_t *obj)
{
    treeUpdate(s_worldLevelTree, obj);
}

void treeWorldLevelRemove(WorldLevel_t *obj)
{
    treeRemove(s_worldLevelTree, obj);
}

void treeWorldLevelQuery(double Left, double Top, double Right, double Bottom,
                         WorldLevelPtrArr &list,
                         bool z_sort, double margin)
{
    treeWorldQuery(s_worldLevelTree,
                   Left - margin,
                   Top - margin,
                   Right + margin,
                   Bottom + margin,
                   list, z_sort);
}

void treeWorldLevelQuery(const Location_t &loc, WorldLevelPtrArr &list, bool z_sort, double margin)
{
    treeWorldQuery(s_worldLevelTree,
                   loc.X - margin,
                   loc.Y - margin,
                   loc.X + loc.Width + margin,
                   loc.Y + loc.Height + margin,
                   list, z_sort);
}


/* ================= Music ================= */

void treeWorldMusicAdd(WorldMusic_t *obj)
{
    treeInsert(s_worldMusicTree, obj);
}

void treeWorldMusicUpdate(WorldMusic_t *obj)
{
    treeUpdate(s_worldMusicTree, obj);
}

void treeWorldMusicRemove(WorldMusic_t *obj)
{
    treeRemove(s_worldMusicTree, obj);
}

void treeWorldMusicQuery(double Left, double Top, double Right, double Bottom,
                         WorldMusicPtrArr &list,
                         bool z_sort,
                         double margin)
{
    treeWorldQuery(s_worldMusicTree,
                   Left - margin,
                   Top - margin,
                   Right + margin,
                   Bottom + margin,
                   list, z_sort);
}

void treeWorldMusicQuery(const Location_t &loc,
                         WorldMusicPtrArr &list,
                         bool z_sort,
                         double margin)
{
    treeWorldQuery(s_worldMusicTree,
                   loc.X - margin,
                   loc.Y - margin,
                   loc.X + loc.Width + margin,
                   loc.Y + loc.Height + margin, list, z_sort);
}



/* ================= Tile block search ================= */
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
