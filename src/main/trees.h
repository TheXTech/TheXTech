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


#include "globals.h"

extern void treeWorldCleanAll();

typedef std::vector<Tile_t*> TilePtrArr;
extern void treeWorldTileAdd(Tile_t *obj);
extern void treeWorldTileUpdate(Tile_t *obj);
extern void treeWorldTileRemove(Tile_t *obj);
extern void treeWorldTileQuery(double Left, double Top, double Right, double Bottom,
                               TilePtrArr &list,
                               bool z_sort, double margin = 0.0);
extern void treeWorldTileQuery(const Location_t &loc, TilePtrArr &list, bool z_sort, double margin = 0.0);


typedef std::vector<Scene_t*> ScenePtrArr;
extern void treeWorldSceneAdd(Scene_t *obj);
extern void treeWorldSceneUpdate(Scene_t *obj);
extern void treeWorldSceneRemove(Scene_t *obj);
extern void treeWorldSceneQuery(double Left, double Top, double Right, double Bottom,
                               ScenePtrArr &list,
                               bool z_sort, double margin = 16.0);
extern void treeWorldSceneQuery(const Location_t &loc, ScenePtrArr &list, bool z_sort, double margin = 16.0);


typedef std::vector<WorldPath_t*> WorldPathPtrArr;
extern void treeWorldPathAdd(WorldPath_t *obj);
extern void treeWorldPathUpdate(WorldPath_t *obj);
extern void treeWorldPathRemove(WorldPath_t *obj);
extern void treeWorldPathQuery(double Left, double Top, double Right, double Bottom,
                               WorldPathPtrArr &list,
                               bool z_sort, double margin = 16.0);
extern void treeWorldPathQuery(const Location_t &loc, WorldPathPtrArr &list, bool z_sort, double margin = 16.0);


typedef std::vector<WorldLevel_t*> WorldLevelPtrArr;
extern void treeWorldLevelAdd(WorldLevel_t *obj);
extern void treeWorldLevelUpdate(WorldLevel_t *obj);
extern void treeWorldLevelRemove(WorldLevel_t *obj);
extern void treeWorldLevelQuery(double Left, double Top, double Right, double Bottom,
                               WorldLevelPtrArr &list,
                               bool z_sort,
                               double margin = 16.0);
extern void treeWorldLevelQuery(const Location_t &loc, WorldLevelPtrArr &list, bool z_sort, double margin = 16.0);

typedef std::vector<WorldMusic_t*> WorldMusicPtrArr;
extern void treeWorldMusicAdd(WorldMusic_t *obj);
extern void treeWorldMusicUpdate(WorldMusic_t *obj);
extern void treeWorldMusicRemove(WorldMusic_t *obj);
extern void treeWorldMusicQuery(double Left, double Top, double Right, double Bottom,
                               WorldMusicPtrArr &list,
                               bool z_sort, double margin = 16.0);
extern void treeWorldMusicQuery(const Location_t &loc, WorldMusicPtrArr &list, bool z_sort, double margin = 16.0);


extern void blockTileGet(const Location_t &loc, int64_t &fBlock, int64_t &lBlock);
extern void blockTileGet(double x, double w, int64_t &fBlock, int64_t &lBlock);
