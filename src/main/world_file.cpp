/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2024 Vitaly Novichkov <admin@wohlnet.ru>
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

#include "sdl_proxy/sdl_stdinc.h"
#include "sdl_proxy/sdl_timer.h"

#include <json/json_rwops_input.hpp>
#include <json/json.hpp>

#include "core/render.h"

#include "../globals.h"
#include "../frame_timer.h"
#include "../game_main.h"
#include "../load_gfx.h"
#include "../sound.h"
#include "../custom.h"
#include "config.h"
#include "../main/trees.h"
#include "level_file.h"
#include "world_file.h"
#include "main/level_save_info.h"
#include "main/screen_progress.h"
#include "main/game_strings.h"
#include "translate_episode.h"
#include "fontman/font_manager.h"

#include <Utils/strings.h>
#include <Utils/files.h>
#include <Utils/dir_list_ci.h>
#include <Logger/logger.h>
#include <PGE_File_Formats/file_formats.h>

#include "global_dirs.h"

#include "editor/editor_custom.h"
#include "editor/editor_strings.h"

struct WorldLoad
{
    int FileRelease = 64;
};

bool OpenWorld_Unpack(WorldLoad& load, WorldData& wld);

bool OpenWorld_Post(const WorldLoad& load);

bool OpenWorld(std::string FilePath)
{
    // USE PGE-FL here
    // std::string newInput = "";
    WorldLoad load;
    // long long zCounter = 0;

    ClearWorld();

//    for(A = FilePath.length(); A >= 1; A--)
//    {
//        if(FilePath.substr(A - 1, 1) == "/" || FilePath.substr(A - 1, 1) == "\\")
//            break;
//    }

    // set the file path and load custom configuration
    FileNamePath = Files::dirname(FilePath) + "/";
    g_dirEpisode.setCurDir(FileNamePath);

    FileName = g_dirEpisode.resolveDirCase(Files::basenameNoSuffix(FilePath));
    g_dirCustom.setCurDir(FileNamePath + FileName);

    FileNameFull = Files::basename(FilePath);
    FullFileName = FilePath;

    // Preserve these values for quick restoring when going to the world map
    FileNameFullWorld = FileNameFull;
    FileNameWorld = FileName;
    FileNamePathWorld = FileNamePath;
    FileFormatWorld = FileFormat;

    FontManager::loadCustomFonts();

    LoadCustomConfig();
    FindCustomPlayers();
    LoadCustomGFX(true);

    // bool compatModern = (g_config.compatibility_mode == Config_t::COMPAT_OFF);

    numTiles = 0;
    numScenes = 0;
    numWorldLevels = 0;
    numWorldPaths = 0;
    numWorldMusic = 0;
    numWorldAreas = 0;

    // FileFormats::OpenWorldFile(FilePath, wld);
    {
        WorldData wld;

        PGE_FileFormats_misc::RWopsTextInput in(Files::open_file(FilePath, "r"), FilePath);
        if(!FileFormats::OpenWorldFileT(in, wld))
        {
            pLogWarning("Error of world \"%s\" file loading: %s (line %d).",
                        FilePath.c_str(),
                        wld.meta.ERROR_info.c_str(),
                        wld.meta.ERROR_linenum);
            return false;
        }

        if(!OpenWorld_Unpack(load, wld))
            return false;
    }

    return OpenWorld_Post(load);
}

bool OpenWorld_Head(void* userdata, WorldData& wld)
{
    WorldLoad& load = *static_cast<WorldLoad*>(userdata);

    FileFormat = wld.meta.RecentFormat;
    if(wld.meta.RecentFormat == LevelData::SMBX64)
        load.FileRelease = int(wld.meta.RecentFormatVersion);

    WorldName = wld.EpisodeTitle;

    // cancel block if cheat is active
    if(g_forceCharacter && !LevelEditor && !WorldEditor)
    {
        for(int A = 1; A <= numCharacters; A++)
            blockCharacter[A] = false;
    }
    // load character block
    else
    {
        for(size_t A = 1; A <= numCharacters && A - 1 < wld.nocharacter.size(); A++)
            blockCharacter[A] = wld.nocharacter[A - 1];
    }

    StartLevel = wld.IntroLevel_file;
    addMissingLvlSuffix(StartLevel);
    StartLevel = g_dirEpisode.resolveFileCase(StartLevel);

    NoMap = wld.HubStyledWorld;
    RestartLevel = wld.restartlevel;

    // new:
    WorldStarsShowPolicy = wld.starsShowPolicy;

    MaxWorldStars = int(wld.stars);

    // world extra settings:
    WldxCustomParams.clear();
    SubHubLevels.clear();

    if(!wld.custom_params.empty())
    {
        WldxCustomParams = wld.custom_params;

        try
        {
            const nlohmann::json world_data = nlohmann::json::parse(wld.custom_params);

            if(world_data.contains("sub_hubs_list"))
            {
                for(const nlohmann::json& sub_hub : world_data["sub_hubs_list"])
                    SubHubLevels.push_back(sub_hub.get<std::string>());
            }
        }
        catch(const std::exception &e)
        {
            pLogWarning("Failed to load World %s JSON data: %s", FileNameFull.c_str(), e.what());
        }
    }

    // world credits
    numWorldCredits = 0;
    for(int i = 1; i <= maxWorldCredits; i++)
        WorldCredits[i].clear();

    int B = 0;
    std::vector<std::string> authorsList;
    if(!wld.authors.empty())
    {
        Strings::split(authorsList, wld.authors, "\n");
        for(auto &c : authorsList)
        {
            ++B;
            if(B > maxWorldCredits)
                break;
            WorldCredits[B] = c;
            numWorldCredits = B;
        }
    }

    return true;
}

bool OpenWorld_Tile(void*, WorldTerrainTile& t)
{
    {
        numTiles++;
        if(numTiles > maxTiles)
        {
            numTiles = maxTiles;
            return false;
        }

        auto &terra = Tile[numTiles];

        terra = Tile_t();

        terra.Location.X = t.x;
        terra.Location.Y = t.y;
        terra.Type = int(t.id);
        terra.Location.Width = TileWidth[terra.Type];
        terra.Location.Height = TileHeight[terra.Type];
        // terra.Z = zCounter++;
        treeWorldTileAdd(&terra);

        if(IF_OUTRANGE(terra.Type, 1, maxTileType)) // Drop ID to 1 for Tiles of out of range IDs
        {
            pLogWarning("TILE-%d ID is out of range (max types %d), reset to TILE-1", terra.Type, maxTileType);
            terra.Type = 1;
        }
    }

    return true;
}

bool OpenWorld_Scene(void*, WorldScenery& s)
{
    {
        numScenes++;
        if(numScenes > maxScenes)
        {
            numScenes = maxScenes;
            return false;
        }

        auto &scene = Scene[numScenes];

        scene = Scene_t();

        scene.Type = int(s.id);
        scene.Location.X = s.x;
        scene.Location.Y = s.y;
        scene.Location.Width = SceneWidth[scene.Type];
        scene.Location.Height = SceneHeight[scene.Type];
        scene.Active = true;
        // scene.Z = zCounter++;
        treeWorldSceneAdd(&scene);

        if(IF_OUTRANGE(scene.Type, 1, maxSceneType)) // Drop ID to 1 for Scenery of out of range IDs
        {
            pLogWarning("TILE-%d ID is out of range (max types %d), reset to TILE-1", scene.Type, maxSceneType);
            scene.Type = 1;
        }
    }

    return true;
}

bool OpenWorld_Path(void*, WorldPathTile& p)
{
    {
        numWorldPaths++;
        if(numWorldPaths > maxWorldPaths)
        {
            numWorldPaths = maxWorldPaths;
            return false;
        }

        auto &pp = WorldPath[numWorldPaths];

        pp = WorldPath_t();

        pp.Location.X = p.x;
        pp.Location.Y = p.y;
        pp.Type = int(p.id);
        pp.Location.Width = 32;
        pp.Location.Height = 32;
        pp.Active = false;
        // pp.Z = zCounter++;
        treeWorldPathAdd(&pp);
//        if(LevelEditor == true)
//            pp.Active = true;

        if(IF_OUTRANGE(pp.Type, 1, maxPathType)) // Drop ID to 1 for Path of out of range IDs
        {
            pLogWarning("PATH-%d ID is out of range (max types %d), reset to PATH-1", pp.Type, maxPathType);
            pp.Type = 1;
        }
    }

    return true;
}

bool OpenWorld_Level(void*, WorldLevelTile& l)
{
    {
        numWorldLevels++;
        if(numWorldLevels > maxWorldLevels)
        {
            numWorldLevels = maxWorldLevels;
            return false;
        }

        auto &ll = WorldLevel[numWorldLevels];

        ll = WorldLevel_t();

        ll.Location.X = l.x;
        ll.Location.Y = l.y;
        ll.Type = int(l.id);
        validateLevelName(ll.FileName, l.lvlfile);
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

        // ll.Z = zCounter++;
        treeWorldLevelAdd(&ll);

        if(IF_OUTRANGE(ll.Type, 0, maxLevelType)) // Drop ID to 1 for Levels of out of range IDs
        {
            pLogWarning("PATH-%d ID is out of range (max types %d), reset to PATH-1", ll.Type, maxLevelType);
            ll.Type = 1;
        }
    }

    return true;
}

bool OpenWorld_Music(void*, WorldMusicBox& m)
{
    {
        numWorldMusic++;
        if(numWorldMusic > maxWorldMusic)
        {
            numWorldMusic = maxWorldMusic;
            return false;
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
        // box.Z = zCounter++;
        treeWorldMusicAdd(&box);
    }

    return true;
}

bool OpenWorld_AreaRect(void*, WorldAreaRect& m)
{
    {
        if(!(m.flags & WorldAreaRect::SETUP_SET_VIEWPORT))
            return true;

        numWorldAreas++;
        if(numWorldAreas > maxWorldAreas)
        {
            numWorldAreas = maxWorldAreas;
            return false;
        }

        auto &area = WorldArea[numWorldAreas];

        area = WorldArea_t();

        area.Location.X = m.x;
        area.Location.Y = m.y;
        area.Location.Width = m.w;
        area.Location.Height = m.h;
    }

    return true;
}

bool OpenWorld_Unpack(WorldLoad& load, WorldData& wld)
{
    OpenWorld_Head(&load, wld);

    for(auto &t : wld.tiles)
    {
        if(!OpenWorld_Tile(&load, t))
            break;
    }
    for(auto &s : wld.scenery)
    {
        if(!OpenWorld_Scene(&load, s))
            break;
    }
    for(auto &p : wld.paths)
    {
        if(!OpenWorld_Path(&load, p))
            break;
    }
    for(auto &l : wld.levels)
    {
        if(!OpenWorld_Level(&load, l))
            break;
    }
    for(auto &m : wld.music)
    {
        if(!OpenWorld_Music(&load, m))
            break;
    }
    for(auto &a : wld.arearects)
    {
        if(!OpenWorld_AreaRect(&load, a))
            break;
    }

    return true;
}

bool OpenWorld_Post(const WorldLoad& load)
{
    TranslateEpisode tr;

    if(!LevelEditor)
        tr.loadWorldTranslation(FileNameFull);

    LoadCustomSound();

    // the version targeting below is from SMBX 1.3
    const int FileRelease = load.FileRelease;

    if(!LevelEditor)
    {
        for(int A = 1; A <= numWorldLevels; A++)
        {
            auto &ll = WorldLevel[A];
            if((FileRelease <= 20 && ll.Type == 1) || (FileRelease > 20 && ll.Start))
            {
                WorldPlayer[1].Type = 1;
                WorldPlayer[1].Location = static_cast<Location_t>(WorldLevel[A].Location);
                break;
            }
        }

        for(int A = 1; A <= numWorldLevels; A++)
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
        for(int A = 1; A <= numWorldLevels; A++)
        {
            auto &ll = WorldLevel[A];
            if(FileRelease <= 20 && ll.Type == 1)
                ll.Start = true;
        }

        vScreen[1].X = (XRender::TargetW / 2) - (800 / 2);
        vScreen[1].Y = (XRender::TargetH / 2) - (600 / 2);
    }

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

    WorldPlayer[1] = WorldPlayer_t();

    if(!quick)
    {
#ifdef __16M__
        XRender::clearAllTextures();
#endif

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
    numWorldAreas = 0;
    RestartLevel = false;
    WorldStarsShowPolicy = WorldData::STARS_UNSPECIFIED;
    NoMap = false;
    IsEpisodeIntro = false;
    WldxCustomParams.clear();
    SubHubLevels.clear();
    StartLevel.clear();
    BeatTheGame = false;
    numWorldCredits = 0;
    // default file format if world header is missing
    FileFormat = FileFormats::LVL_PGEX;

    for(int i = 1; i <= maxWorldCredits; i++)
        WorldCredits[i].clear();

    if(LevelEditor)
    {
        vScreen[1].X = 0;
        vScreen[1].Y = 0;
    }
//    if(LevelEditor == true)
//    {
//        frmLevelEditor::optCursor(14).Value = true;
//        frmWorld.txtWorldName = "";
//        frmWorld.txtStartLevel = "";
//        frmWorld::chkNoMap.Value = false;
//        frmWorld.chkRestartLevel = false;
//        vScreen[1].X = 0;
//        vScreen[1].Y = 0;
//        for(A = 1; A <= 5; A++)
//            frmWorld::txtCredits(A).Text = "";
//        frmWorld.txtStars = "";
//        MaxWorldStars = 0;
//    }
}

void FindWldStars()
{
    LevelData tempData;
    uint32_t start_time = SDL_GetTicks();

    bool world_must_show_stars = (WorldStarsShowPolicy == Config_t::MAP_STARS_SHOW);

    for(int A = 1; A <= numWorldLevels; A++)
    {
        IndicateProgress(start_time, (double)A / numWorldLevels, g_gameStrings.messageScanningLevels);

        auto &l = WorldLevel[A];

        if(!l.FileName.empty())
        {
            l.curStars = 0;

            for(const auto& star : Star)
            {
                if(SDL_strcasecmp(star.level.c_str(), Files::basename(l.FileName).c_str()) == 0)
                    l.curStars++;
            }

            bool level_must_show_stars = (l.starsShowPolicy == Config_t::MAP_STARS_SHOW);
            bool level_can_show_stars = (world_must_show_stars && l.starsShowPolicy == Config_t::MAP_STARS_UNSPECIFIED);

            // skip check for max stars and medals if it's already been inited, OR if the star count isn't needed
            if(l.save_info.inited() || !(level_must_show_stars || level_can_show_stars))
                continue;

            std::string lFile = l.FileName;
            addMissingLvlSuffix(lFile);

            std::string fullPath = g_dirEpisode.resolveFileCaseExistsAbs(lFile);

            if(!fullPath.empty())
                l.save_info = InitLevelSaveInfo(fullPath, tempData);
        }
    }
}

#if 0
// Is there any unsupported content for this format in the world?
bool CanConvertWorld(int format, std::string* reasons)
{
    if(format == FileFormats::WLD_PGEX)
        return true;

    if(format == FileFormats::WLD_SMBX38A)
    {
        if(reasons)
        {
            *reasons = g_editorStrings.fileConvert38aUnsupported;
            *reasons += '\n';
        }
        return false;
    }

    if(format != FileFormats::WLD_SMBX64)
    {
        if(reasons)
        {
            *reasons = g_editorStrings.fileConvertFormatUnknown;
            *reasons += '\n';
        }
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
            {
                *reasons = g_editorStrings.fileConvertFeatureCustomWorldMusic;
                *reasons += '\n';
            }
            break;
        }
    }

    if(WorldStarsShowPolicy != WorldData::STARS_UNSPECIFIED)
    {
        can_convert = false;
        if(reasons)
        {
            *reasons = g_editorStrings.fileConvertFeatureWorldStarDisplay;
            *reasons += '\n';
        }
    }

    for(int i = 1; i <= numWorldLevels; i++)
    {
        if(WorldLevel[i].starsShowPolicy != WorldData::STARS_UNSPECIFIED)
        {
            can_convert = false;
            if(reasons)
            {
                *reasons = g_editorStrings.fileConvertFeatureLevelStarDisplay;
                *reasons += '\n';
            }
        }
    }

    if(numWorldAreas > 0)
    {
        can_convert = false;
        if(reasons)
        {
            *reasons = g_editorStrings.fileConvertFeatureLevelStarDisplay;
            *reasons += '\n';
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
        SetS(WorldMusic[i].MusicFile, "");

    WorldStarsShowPolicy = WorldData::STARS_UNSPECIFIED;

    for(int i = 1; i <= numWorldLevels; i++)
        WorldLevel[i].starsShowPolicy = WorldData::STARS_UNSPECIFIED;

    numWorldAreas = 0;
}
#endif
