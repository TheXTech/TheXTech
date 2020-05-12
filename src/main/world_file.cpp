/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2020 Vitaly Novichkov <admin@wohlnet.ru>
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

#include "../globals.h"
#include "../game_main.h"
#include "../load_gfx.h"
#include "../sound.h"
#include "level_file.h"

#include <Utils/strings.h>
#include <Utils/files.h>
#include <Utils/dir_list_ci.h>
#include <Logger/logger.h>
#include <PGE_File_Formats/file_formats.h>

void OpenWorld(std::string FilePath)
{
    // USE PGE-FL here
    std::string newInput = "";
    int FileRelease = 64;
    int A = 0;
    int B = 0;
    WorldData wld;
    DirListCI dirEpisode;

    ClearWorld();

    FileFormats::OpenWorldFile(FilePath, wld);

//    for(A = FilePath.length(); A >= 1; A--)
//    {
//        if(FilePath.substr(A - 1, 1) == "/" || FilePath.substr(A - 1, 1) == "\\")
//            break;
//    }

    dirEpisode.setCurDir(wld.meta.path);
    FileNameFull = Files::basename(FilePath);
    FileName = dirEpisode.resolveDirCase(wld.meta.filename); //FilePath.substr(FilePath.length() - (FilePath.length() - A));
    FileNamePath = wld.meta.path + "/"; //FilePath.substr(0, (A));

    if(wld.meta.RecentFormat == LevelData::SMBX64)
        FileRelease = int(wld.meta.RecentFormatVersion);

    LoadWorldCustomGFX();

    numTiles = 0;
    numScenes = 0;
    numWorldLevels = 0;
    numWorldPaths = 0;
    numWorldMusic = 0;

    WorldName = wld.EpisodeTitle;
    wld.charactersToS64();
    blockCharacter[1] = wld.nocharacter1;
    blockCharacter[2] = wld.nocharacter2;
    blockCharacter[3] = wld.nocharacter3;
    blockCharacter[4] = wld.nocharacter4;
    blockCharacter[5] = wld.nocharacter5;
    StartLevel = dirEpisode.resolveFileCase(wld.IntroLevel_file);
    NoMap = wld.HubStyledWorld;
    RestartLevel = wld.restartlevel;

    MaxWorldStars = int(wld.stars);

    addMissingLvlSuffix(StartLevel);

    for(int i = 1; i <= maxWorldCredits; i++)
        WorldCredits[i].clear();

    B = 0;
    std::vector<std::string> authorsList;
    if(!wld.authors.empty())
    {
        Strings::split(authorsList, wld.authors, "\n");
        for(auto &c : authorsList)
        {
            B++;
            if(B > maxWorldCredits)
                break;
            WorldCredits[B] = c;
        }
    }

    for(auto &t : wld.tiles)
    {
        numTiles++;
        if(numTiles > maxTiles)
        {
            numTiles = maxTiles;
            break;
        }

        auto &terra = Tile[numTiles];

        terra = Tile_t();

        terra.Location.X = t.x;
        terra.Location.Y = t.y;
        terra.Type = int(t.id);
        terra.Location.Width = TileWidth[terra.Type];
        terra.Location.Height = TileHeight[terra.Type];

        if(terra.Type > maxTileType) // Drop ID to 1 for Tiles of out of range IDs
        {
            pLogWarning("TILE-%d ID is out of range (max types %d), reset to TILE-1", terra.Type, maxTileType);
            terra.Type = 1;
        }
    }

    for(auto &s : wld.scenery)
    {
        numScenes++;
        if(numScenes > maxScenes)
        {
            numScenes = maxScenes;
            break;
        }

        auto &scene = Scene[numScenes];

        scene = Scene_t();

        scene.Type = int(s.id);
        scene.Location.X = s.x;
        scene.Location.Y = s.y;
        scene.Location.Width = SceneWidth[scene.Type];
        scene.Location.Height = SceneHeight[scene.Type];
        scene.Active = true;

        if(scene.Type > maxSceneType) // Drop ID to 1 for Scenery of out of range IDs
        {
            pLogWarning("TILE-%d ID is out of range (max types %d), reset to TILE-1", scene.Type, maxSceneType);
            scene.Type = 1;
        }
    }

    for(auto &p : wld.paths)
    {
        numWorldPaths++;
        if(numWorldPaths > maxWorldPaths)
        {
            numWorldPaths = maxWorldPaths;
            break;
        }

        auto &pp = WorldPath[numWorldPaths];

        pp = WorldPath_t();

        pp.Location.X = p.x;
        pp.Location.Y = p.y;
        pp.Type = int(p.id);
        pp.Location.Width = 32;
        pp.Location.Height = 32;
        pp.Active = false;
//        if(LevelEditor == true)
//            pp.Active = true;

        if(pp.Type > maxPathType) // Drop ID to 1 for Path of out of range IDs
        {
            pLogWarning("PATH-%d ID is out of range (max types %d), reset to PATH-1", pp.Type, maxPathType);
            pp.Type = 1;
        }
    }

    for(auto &l : wld.levels)
    {
        numWorldLevels++;
        if(numWorldLevels > maxWorldLevels)
        {
            numWorldLevels = maxWorldLevels;
            break;
        }

        auto &ll = WorldLevel[numWorldLevels];

        ll = WorldLevel_t();

        ll.Location.X = l.x;
        ll.Location.Y = l.y;
        ll.Type = int(l.id);
        ll.FileName = dirEpisode.resolveFileCase(l.lvlfile);
        ll.LevelName = l.title;
        ll.LevelExit[1] = l.top_exit;
        ll.LevelExit[2] = l.left_exit;
        ll.LevelExit[3] = l.bottom_exit;
        ll.LevelExit[4] = l.right_exit;
        ll.Location.Width = 32;
        ll.Location.Height = 32;
        ll.StartWarp = int(l.entertowarp);
        ll.Visible = l.alwaysVisible;
        ll.Active = ll.Visible;
        ll.Path = l.pathbg;
        ll.Start = l.gamestart;
        ll.WarpX = l.gotox;
        ll.WarpY = l.gotoy;
        ll.Path2 = l.bigpathbg;

        if(ll.Type > maxLevelType) // Drop ID to 1 for Levels of out of range IDs
        {
            pLogWarning("PATH-%d ID is out of range (max types %d), reset to PATH-1", ll.Type, maxLevelType);
            ll.Type = 1;
        }
    }

    for(auto &m : wld.music)
    {
        numWorldMusic++;
        if(numWorldMusic > maxWorldMusic)
        {
            numWorldMusic = maxWorldMusic;
            break;
        }

        auto &box = WorldMusic[numWorldMusic];

        box = WorldMusic_t();

        box.Location.X = m.x;
        box.Location.Y = m.y;
        box.Type = int(m.id);
        box.MusicFile = dirEpisode.resolveFileCase(m.music_file);

        // In game they are smaller (30x30), in world they are 32x32
        box.Location.Width = 30;
        box.Location.Height = 30;
        box.Location.Y = box.Location.Y + 1;
        box.Location.X = box.Location.X + 1;
    }

    LoadCustomGFX();
    LoadCustomSound();

//    if(LevelEditor == false)
    {
        for(A = 1; A <= numWorldLevels; A++)
        {
            auto &ll = WorldLevel[A];
            if((FileRelease <= 20 && ll.Type == 1) || (FileRelease > 20 && ll.Start))
            {
                WorldPlayer[1].Type = 1;
                WorldPlayer[1].Location = WorldLevel[A].Location;
                break;
            }
        }

        for(A = 1; A <= numWorldLevels; A++)
        {
            auto &ll = WorldLevel[A];
            if((FileRelease <= 20 && ll.Type == 1) || (FileRelease > 20 && ll.Start))
            {
                ll.Active = true;
                LevelPath(A, 5, true);
            }
        }
    }
//    else
//    {
//        for(A = 1; A <= numCharacters; A++)
//        {
//            if(blockCharacter[A] == true)
//                frmWorld::chkChar(A).Value = 1;
//            else
//                frmWorld::chkChar(A).Value = 0;
//        }
//        frmWorld.txtWorldName = WorldName;
//        frmWorld.txtStartLevel = StartLevel;
//        if(NoMap == true)
//            frmWorld::chkNoMap.Value = 1;
//        else
//            frmWorld::chkNoMap.Value = 0;
//        if(RestartLevel == true)
//            frmWorld::chkRestartLevel.Value = 1;
//        else
//            frmWorld::chkRestartLevel.Value = 0;
//        for(A = 1; A <= 5; A++)
//            frmWorld::txtCredits(A).Text = WorldCredits[A];
//        frmWorld.txtStars = MaxWorldStars;
//    }
    overTime = 0;
    GoalTime = SDL_GetTicks() + 1000;
    fpsCount = 0;
    fpsTime = 0;
    cycleCount = 0;
    gameTime = 0;
}

void ClearWorld()
{
    int A = 0;

    for(A = 1; A <= numCharacters; A++)
    {
        blockCharacter[A] = false;
//        if(LevelEditor == true)
//            frmWorld::chkChar(A).Value = 0;
    }

    for(A = 1; A <= numTiles; A++)
        Tile[A] = Tile_t();
    for(A = 1; A <= numWorldPaths; A++)
        WorldPath[A] = WorldPath_t();
    for(A = 1; A <= numScenes; A++)
        Scene[A] = Scene_t();
    for(A = 1; A <= numWorldLevels; A++)
        WorldLevel[A] = WorldLevel_t();
    for(A = 1; A <= numWorldMusic; A++)
        WorldMusic[A] = WorldMusic_t();

    MaxWorldStars = 0;
    numTiles = 0;
    numWorldPaths = 0;
    numScenes = 0;
    numWorldLevels = 0;
    numWorldPaths = 0;
    numWorldMusic = 0;
    RestartLevel = false;
    NoMap = false;
    IsEpisodeIntro = false;
    StartLevel.clear();
    BeatTheGame = false;
    for(int A = 1; A <= maxWorldCredits; A++)
        WorldCredits[A].clear();
    UnloadCustomGFX();
    UnloadWorldCustomGFX();
    UnloadCustomSound();
//    if(LevelEditor == true)
//    {
//        frmLevelEditor::optCursor(14).Value = true;
//        frmWorld.txtWorldName = "";
//        frmWorld.txtStartLevel = "";
//        frmWorld::chkNoMap.Value = false;
//        frmWorld.chkRestartLevel = false;
//        vScreenX[1] = 0;
//        vScreenY[1] = 0;
//        for(A = 1; A <= 5; A++)
//            frmWorld::txtCredits(A).Text = "";
//        frmWorld.txtStars = "";
//        MaxWorldStars = 0;
//    }
}
