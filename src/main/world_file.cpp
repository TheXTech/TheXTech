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

#include "core/sdl.h"

#include "../globals.h"
#include "../frame_timer.h"
#include "../game_main.h"
#include "../load_gfx.h"
#include "../sound.h"
#include "../custom.h"
#include "../compat.h"
#include "../main/trees.h"
#include "level_file.h"
#include "world_file.h"

#include <Utils/strings.h>
#include <Utils/files.h>
#include <Utils/dir_list_ci.h>
#include <Logger/logger.h>
#include <PGE_File_Formats/file_formats.h>

#include "global_dirs.h"

#include "editor/editor_custom.h"

bool OpenWorld(std::string FilePath)
{
    // USE PGE-FL here
    // std::string newInput = "";
    int FileRelease = 64;
    bool compatModern = (CompatGetLevel() == COMPAT_MODERN);
    int A = 0;
    int B = 0;
    long long zCounter = 0;
    WorldData wld;

    ClearWorld();

    // FileFormats::OpenWorldFile(FilePath, wld);
    if(!FileFormats::OpenWorldFile(FilePath, wld))
    {
        pLogWarning("Error of world \"%s\" file loading: %s (line %d).",
                    FilePath.c_str(),
                    wld.meta.ERROR_info.c_str(),
                    wld.meta.ERROR_linenum);
        return false;
    }

//    for(A = FilePath.length(); A >= 1; A--)
//    {
//        if(FilePath.substr(A - 1, 1) == "/" || FilePath.substr(A - 1, 1) == "\\")
//            break;
//    }

    g_dirEpisode.setCurDir(wld.meta.path);
    FileFormat = wld.meta.RecentFormat;
    FileName = g_dirEpisode.resolveDirCase(wld.meta.filename); //FilePath.substr(FilePath.length() - (FilePath.length() - A));
    FileNamePath = wld.meta.path + "/"; //FilePath.substr(0, (A));
    g_dirCustom.setCurDir(FileNamePath + FileName);

    if(!FilePath.empty())
    {
        FileNameFull = Files::basename(FilePath);
        FullFileName = FilePath;
    }
    else if(FileFormat == FileFormats::LVL_SMBX64 || FileFormat == FileFormats::LVL_SMBX38A)
    {
        FileNameFull = FileName + ".lvl";
        FullFileName = FileNamePath + FileName + ".lvl";
    }
    else
    {
        FileNameFull = FileName + ".lvlx";
        FullFileName = FileNamePath + FileName + ".lvlx";
    }

    // Preserve these values for quick restoring when going to the world map
    FileNameFullWorld = FileNameFull;
    FileNameWorld = FileName;
    FileNamePathWorld = FileNamePath;
    FileFormatWorld = FileFormat;

    if(wld.meta.RecentFormat == LevelData::SMBX64)
        FileRelease = int(wld.meta.RecentFormatVersion);

    LoadCustomCompat();
    FindCustomPlayers();
    LoadCustomGFX(true);

    if(LevelEditor || WorldEditor)
        EditorCustom::Load();

    numTiles = 0;
    numScenes = 0;
    numWorldLevels = 0;
    numWorldPaths = 0;
    numWorldMusic = 0;

    WorldName = wld.EpisodeTitle;
    wld.charactersToS64();

    blockCharacter[1] = wld.nocharacter1;
    blockCharacter[2] = wld.nocharacter2;

    if(FileRelease >= 30 || !compatModern)
    {
        blockCharacter[3] = wld.nocharacter3;
        blockCharacter[4] = wld.nocharacter4;
        blockCharacter[5] = wld.nocharacter5;
    }
    else
    {
        blockCharacter[3] = true;
        blockCharacter[4] = true;
        blockCharacter[5] = true;
    }

    StartLevel = wld.IntroLevel_file;
    addMissingLvlSuffix(StartLevel);
    StartLevel = g_dirEpisode.resolveFileCase(StartLevel);

    NoMap = wld.HubStyledWorld;
    RestartLevel = wld.restartlevel;

    // new:
    WorldStarsShowPolicy = wld.starsShowPolicy;

    MaxWorldStars = int(wld.stars);


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

    zCounter = 0;
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
        terra.Z = zCounter++;
        treeWorldTileAdd(&terra);

        if(IF_OUTRANGE(terra.Type, 1, maxTileType)) // Drop ID to 1 for Tiles of out of range IDs
        {
            pLogWarning("TILE-%d ID is out of range (max types %d), reset to TILE-1", terra.Type, maxTileType);
            terra.Type = 1;
        }
    }

    zCounter = 0;
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
        scene.Z = zCounter++;
        treeWorldSceneAdd(&scene);

        if(IF_OUTRANGE(scene.Type, 1, maxSceneType)) // Drop ID to 1 for Scenery of out of range IDs
        {
            pLogWarning("TILE-%d ID is out of range (max types %d), reset to TILE-1", scene.Type, maxSceneType);
            scene.Type = 1;
        }
    }

    zCounter = 0;
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

        pp.index = numWorldPaths;
        pp.Location.X = p.x;
        pp.Location.Y = p.y;
        pp.Type = int(p.id);
        pp.Location.Width = 32;
        pp.Location.Height = 32;
        pp.Active = false;
        pp.Z = zCounter++;
        treeWorldPathAdd(&pp);
//        if(LevelEditor == true)
//            pp.Active = true;

        if(IF_OUTRANGE(pp.Type, 1, maxPathType)) // Drop ID to 1 for Path of out of range IDs
        {
            pLogWarning("PATH-%d ID is out of range (max types %d), reset to PATH-1", pp.Type, maxPathType);
            pp.Type = 1;
        }
    }

    zCounter = 0;
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

        ll.index = numWorldLevels;
        ll.Location.X = l.x;
        ll.Location.Y = l.y;
        ll.Type = int(l.id);
        ll.FileName = g_dirEpisode.resolveFileCase(l.lvlfile);
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

        // new:
        ll.starsShowPolicy = l.starsShowPolicy;

        ll.Z = zCounter++;
        treeWorldLevelAdd(&ll);

        if(IF_OUTRANGE(ll.Type, 0, maxLevelType)) // Drop ID to 1 for Levels of out of range IDs
        {
            pLogWarning("PATH-%d ID is out of range (max types %d), reset to PATH-1", ll.Type, maxLevelType);
            ll.Type = 1;
        }
    }

    zCounter = 0;
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

        // new:
        std::string music_file = g_dirEpisode.resolveFileCase(m.music_file);
        if(!music_file.empty())
        {
            SetS(box.MusicFile, music_file); // adds to LevelString
        }

        // In game they are smaller (30x30), in world they are 32x32
        box.Location.Width = 30;
        box.Location.Height = 30;
        box.Location.Y += 1;
        box.Location.X += 1;
        box.Z = zCounter++;
        treeWorldMusicAdd(&box);
    }

    LoadCustomSound();

    if(!LevelEditor)
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
                LevelPath(WorldLevel[A], 5, true);
            }
        }
    }
    else
    {
        for(A = 1; A <= numWorldLevels; A++)
        {
            auto &ll = WorldLevel[A];
            if(FileRelease <= 20 && ll.Type == 1)
                ll.Start = true;
        }

        vScreenX[1] = (ScreenW / 2 - 800 / 2);
        vScreenY[1] = (ScreenH / 2 - 600 / 2);
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
    SaveWorldStrings();
    resetFrameTimer();

    return true;
}

void ClearWorld(bool quick)
{
    int A = 0;

    for(A = 1; A <= numCharacters; A++)
    {
        blockCharacter[A] = false;
//        if(LevelEditor == true)
//            frmWorld::chkChar(A).Value = 0;
    }

    treeWorldCleanAll();

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

    if(!quick)
    {
        ClearStringsBank();
        UnloadCustomGFX();
        UnloadWorldCustomGFX();
        UnloadCustomSound();
        LoadPlayerDefaults();
    }

    MaxWorldStars = 0;
    numTiles = 0;
    numWorldPaths = 0;
    numScenes = 0;
    numWorldLevels = 0;
    numWorldPaths = 0;
    numWorldMusic = 0;
    RestartLevel = false;
    WorldStarsShowPolicy = WorldData::STARS_UNSPECIFIED;
    NoMap = false;
    IsEpisodeIntro = false;
    StartLevel.clear();
    BeatTheGame = false;
    for(int A = 1; A <= maxWorldCredits; A++)
        WorldCredits[A].clear();
    if(LevelEditor)
    {
        vScreenX[1] = 0;
        vScreenY[1] = 0;
    }
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

void FindWldStars()
{
    LevelData head;

    for(int A = 1; A <= numWorldLevels; A++)
    {
        auto &l = WorldLevel[A];

        if(!l.FileName.empty())
        {
            std::string lFile = l.FileName;
            addMissingLvlSuffix(lFile);

            std::string fullPath = g_dirEpisode.resolveFileCaseExistsAbs(lFile);

            if(!fullPath.empty())
            {
                if(FileFormats::OpenLevelFileHeader(fullPath, head))
                {
                    l.maxStars = head.stars;
                    l.curStars = 0;

                    for(int B = 1; B <= numStars; B++)
                    {
                        if(SDL_strcasecmp(Star[B].level.c_str(), l.FileName.c_str()) == 0)
                            l.curStars++;
                    }
                }
            }

        }
    }
}

// Is there any unsupported content for this format in the world?
bool CanConvertWorld(int format, std::string* reasons)
{
    if(format == FileFormats::WLD_PGEX)
        return true;

    if(format == FileFormats::WLD_SMBX38A)
    {
        if(reasons)
            *reasons = "The SMBX38-A format is not supported at this time.\n";
        return false;
    }

    if(format != FileFormats::WLD_SMBX64)
    {
        if(reasons)
            *reasons = "Requested format is unknown.\n";
        return false;
    }

    bool can_convert = true;
    if(reasons)
        reasons->clear();

    for(int i = 1; i <= numWorldMusic; i++)
    {
        if(!GetS(WorldMusic[i].MusicFile).empty())
        {
            can_convert = false;
            if(reasons)
                *reasons += "Uses custom world music file.\n";
            break;
        }
    }

    if(WorldStarsShowPolicy != WorldData::STARS_UNSPECIFIED)
    {
        can_convert = false;
        if(reasons)
            *reasons += "Uses world setting for star display.\n";
    }

    for(int i = 1; i <= numWorldLevels; i++)
    {
        if(WorldLevel[i].starsShowPolicy != WorldData::STARS_UNSPECIFIED)
        {
            can_convert = false;
            if(reasons)
                *reasons += "Uses per-level setting for star display.\n";
        }
    }

    return can_convert;
}

// Strips all unsupported content from the world.
void ConvertWorld(int format)
{
    FileFormat = format;
    if(format == FileFormats::LVL_SMBX64 || format == FileFormats::LVL_SMBX38A)
    {
        if(!FileNameFull.empty() && FileNameFull.back() == 'x')
            FileNameFull.resize(FileNameFull.size() - 1);
        if(!FullFileName.empty() && FullFileName.back() == 'x')
            FullFileName.resize(FullFileName.size() - 1);
    }
    else
    {
        if(!FileNameFull.empty() && FileNameFull.back() != 'x')
            FileNameFull += "x";
        if(!FullFileName.empty() && FullFileName.back() != 'x')
            FullFileName += "x";
    }

    if(format != FileFormats::WLD_SMBX64)
        return;

    for(int i = 1; i <= numWorldMusic; i++)
    {
        SetS(WorldMusic[i].MusicFile, "");
    }

    WorldStarsShowPolicy = WorldData::STARS_UNSPECIFIED;

    for(int i = 1; i <= numWorldLevels; i++)
    {
        WorldLevel[i].starsShowPolicy = WorldData::STARS_UNSPECIFIED;
    }
}
