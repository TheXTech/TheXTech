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


#include "globals.h"

extern void treeWorldCleanAll();

typedef std::vector<Tile_t*> TilePtrArr;
extern void treeWorldTileAdd(Tile_t *obj);
extern void treeWorldTileUpdate(Tile_t *obj);
extern void treeWorldTileRemove(Tile_t *obj);
extern void treeWorldTileQuery(double Left, double Top, double Right, double Bottom,
                               TilePtrArr &list,
                               bool z_sort);
extern void treeWorldTileQuery(const Location_t &loc, TilePtrArr &list, bool z_sort);


typedef std::vector<Scene_t*> ScenePtrArr;
extern void treeWorldSceneAdd(Scene_t *obj);
extern void treeWorldSceneUpdate(Scene_t *obj);
extern void treeWorldSceneRemove(Scene_t *obj);
extern void treeWorldSceneQuery(double Left, double Top, double Right, double Bottom,
                               ScenePtrArr &list,
                               bool z_sort);
extern void treeWorldSceneQuery(const Location_t &loc, ScenePtrArr &list, bool z_sort);


typedef std::vector<WorldPath_t*> WorldPathPtrArr;
extern void treeWorldPathAdd(WorldPath_t *obj);
extern void treeWorldPathUpdate(WorldPath_t *obj);
extern void treeWorldPathRemove(WorldPath_t *obj);
extern void treeWorldPathQuery(double Left, double Top, double Right, double Bottom,
                               WorldPathPtrArr &list,
                               bool z_sort);
extern void treeWorldPathQuery(const Location_t &loc, WorldPathPtrArr &list, bool z_sort);


typedef std::vector<WorldLevel_t*> WorldLevelPtrArr;
extern void treeWorldLevelAdd(WorldLevel_t *obj);
extern void treeWorldLevelUpdate(WorldLevel_t *obj);
extern void treeWorldLevelRemove(WorldLevel_t *obj);
extern void treeWorldLevelQuery(double Left, double Top, double Right, double Bottom,
                               WorldLevelPtrArr &list,
                               bool z_sort);
extern void treeWorldLevelQuery(const Location_t &loc, WorldLevelPtrArr &list, bool z_sort);

typedef std::vector<WorldMusic_t*> WorldMusicPtrArr;
extern void treeWorldMusicAdd(WorldMusic_t *obj);
extern void treeWorldMusicUpdate(WorldMusic_t *obj);
extern void treeWorldMusicRemove(WorldMusic_t *obj);
extern void treeWorldMusicQuery(double Left, double Top, double Right, double Bottom,
                               WorldMusicPtrArr &list,
                               bool z_sort);
extern void treeWorldMusicQuery(const Location_t &loc, WorldMusicPtrArr &list, bool z_sort);


extern void blockTileGet(const Location_t &loc, int64_t &fBlock, int64_t &lBlock);
extern void blockTileGet(double x, double w, int64_t &fBlock, int64_t &lBlock);
