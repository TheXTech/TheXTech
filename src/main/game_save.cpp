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

#include "../globals.h"
#include "../game_main.h"
#include "config.h"

#include "speedrunner.h"
#ifdef THEXTECH_ENABLE_LUNA_AUTOCODE
#include "../script/luna/lunavarbank.h"
#endif

#include <Utils/files.h>
#include <DirManager/dirman.h>
#include <AppPath/app_path.h>
#include <PGE_File_Formats/file_formats.h>
#include <fmt_format_ne.h>
#include <IniProcessor/ini_processing.h>
#include <script/luna/lunacounter.h>
#include <Logger/logger.h>

#include "main/level_save_info.h"
#include "menu_main.h"
#include "saved_layers.h"

std::string makeGameSavePath(std::string epPath, std::string saveFile)
{
    std::string gameSaveDir = AppPathManager::gameSaveRootDir() + Files::basename(Files::dirname(epPath));
    if(gameSaveDir.back() == ':')
        gameSaveDir.pop_back();

    if(!DirMan::exists(gameSaveDir))
    {
        pLogDebug("Creating directory [%s]", gameSaveDir.c_str());
        DirMan::mkAbsPath(gameSaveDir);
    }

    std::string ret = gameSaveDir + "/" + Files::basename(epPath) + "-" + saveFile;

    pLogDebug("Save data path for ep [%s], file [%s] -> [%s]", epPath.c_str(), saveFile.c_str(), ret.c_str());

    return ret;
}

static void s_LoadCharacter(SavedChar_t& dest, const saveCharState& s)
{
    dest = SavedChar_t();

    if((s.state < 1) || (s.state > numStates))
        dest.State = 1;
    else
        dest.State = int(s.state);

    dest.HeldBonus = int(s.itemID);

    switch(s.mountID)
    {
    default:
    case 0:
        dest.Mount = 0;
        dest.MountType = 0;
        break;
    case 1: case 2: case 3:
        dest.Mount = int(s.mountID);
    }

    dest.MountType = int(s.mountType);
    switch(s.mountID)
    {
    case 1:
        if((s.mountType < 1) || (s.mountType > 3))
            dest.MountType = 1;
        break;
    default:
        break;
    case 3:
        if((s.mountType < 1) || (s.mountType > 8))
            dest.MountType = 1;
        break;
    }

    dest.Hearts = int(s.health);
    dest.Character = int(s.id);
}

static std::string s_legacy_save_path(const SelectWorld_t& w, int save_idx)
{
    const std::string episode = Files::dirname(w.WorldFilePath);
    return fmt::sprintf_ne("%ssave%d.sav", episode.c_str(), save_idx);
}


void FindSaves()
{
//    std::string newInput;
    const auto &w = SelectWorld[selWorld];
    GamesaveData f;

    for(auto A = 1; A <= maxSaveSlots; A++)
    {
        auto& info = SaveSlotInfo[A];

        info = SaveSlotInfo_t();
        info.ConfigDefaults = 0;

        // Modern gamesave file
        std::string saveFile = makeGameSavePath(w.WorldFilePath,
                                                fmt::format_ne("save{0}.savx", A));
        // Old gamesave location at episode's read-only directory
        std::string saveFileOld = s_legacy_save_path(w, A);
        // Gamesave locker to make an illusion of absence of the gamesave
        std::string saveFileOldLocker = makeGameSavePath(w.WorldFilePath,
                                                         fmt::format_ne("save{0}.nosave", A));

        if((Files::fileExists(saveFile) && FileFormats::ReadExtendedSaveFileF(saveFile, f)) ||
           (!Files::fileExists(saveFileOldLocker) && Files::fileExists(saveFileOld) && FileFormats::ReadSMBX64SavFileF(saveFileOld, f)))
        {
            int curActive = 0;
            int maxActive = 0;

            // "game beat flag"
            maxActive++;
            if(f.gameCompleted)
                curActive++;

            // How much paths open
            maxActive += (int)f.visiblePaths.size();
            for(auto &p : f.visiblePaths)
            {
                if(p.second)
                    curActive++;
            }

            // How much levels opened
            maxActive += (int)f.visibleLevels.size();
            for(auto &p : f.visibleLevels)
            {
                if(p.second)
                    curActive++;
            }

            // How many stars collected
            maxActive += (int(f.totalStars) * 4);
            info.Stars = int(f.gottenStars.size());

            curActive += (info.Stars * 4);

            // calculate progress
            if(maxActive > 0)
                info.Progress = curActive * 100 / maxActive;
            else
                info.Progress = 100;

            // load normal stats
            info.Lives = f.lives;
            info.Hundreds = (f.hundreds == 0) ? f.lives : ((f.hundreds > 0) ? f.hundreds - 1 : f.hundreds);
            info.Coins = f.coins;
            info.Score = f.points;

            // load saved chars
            for(int i = 1; i <= 5; i++)
            {
                info.SavedChar[i] = SavedChar_t();
                info.SavedChar[i].Character = i;
            }

            for(auto &s : f.characterStates)
            {
                if((s.id < 1) || (s.id > 5))
                    continue;
                int i = int(s.id);
                s_LoadCharacter(info.SavedChar[i], s);
            }

            // load settings from save file here
            for(auto &s : f.userData.store)
            {
                if(s.name == "_xt_ep_conf" && s.location == saveUserData::DATA_GLOBAL)
                {
                    for(const saveUserData::DataEntry& e : s.data)
                    {
                        if(e.key == "speedrun-mode" && e.value != "0")
                        {
                            info.ConfigDefaults = -atoi(e.value.c_str());
                            break;
                        }
                        // TODO: remove this legacy clause
                        else if(e.key == "enable-bugfixes")
                        {
                            info.ConfigDefaults = (e.value == "none") ? Config_t::MODE_VANILLA : ((e.value == "critical") ? Config_t::MODE_CLASSIC : Config_t::MODE_MODERN);
                            info.ConfigDefaults += 1;
                        }
                        else if(e.key == "playstyle")
                        {
                            info.ConfigDefaults = (e.value == "vanilla") ? Config_t::MODE_VANILLA : ((e.value == "classic") ? Config_t::MODE_CLASSIC : Config_t::MODE_MODERN);
                            info.ConfigDefaults += 1;
                        }
                    }
                    break;
                }
            }

            // default config if not present
            if(info.ConfigDefaults == 0)
            {
                if(f.meta.smbx64strict)
                    info.ConfigDefaults = Config_t::MODE_VANILLA + 1;
                else if(w.bugfixes_on_by_default)
                    info.ConfigDefaults = Config_t::MODE_MODERN + 1;
                else
                    info.ConfigDefaults = Config_t::MODE_CLASSIC + 1;
            }

            // load timer info for existing save
            std::string savePath = makeGameSavePath(w.WorldFilePath,
                                                     fmt::format_ne("timers{0}.ini", A));

            if(Files::fileExists(savePath))
            {
                IniProcessing timer(savePath);
                timer.beginGroup("timers");
                timer.read("total", info.Time, 0);
                timer.endGroup();
            }

            // load fails for existing save
            savePath = makeGameSavePath(w.WorldFilePath,
                                        fmt::format_ne("fails-{0}.rip", A));

            if(Files::fileExists(savePath))
            {
                gDeathCounter.counterFile = savePath;
                gDeathCounter.TryLoadStats();
                gDeathCounter.Recount();
                info.FailsEnabled = true;
                info.Fails = gDeathCounter.mCurTotalDeaths;
                gDeathCounter.quit();
            }
        }
    }
}


void SaveGame()
{
    int A = 0;

    if(Cheater || !selSave)
        return;

    for(A = numPlayers; A >= 1; A--)
        SavedChar[Player[A].Character] = Player[A];

    // Clean-up from invalid entries
    for(auto it = Star.begin(); it != Star.end(); )
    {
        if(it->level.empty())
            it = Star.erase(it);
        else
            ++it;
    }

    numStars = (int)Star.size();

    const auto &w = SelectWorld[selWorld];

    GamesaveData sav;
//    std::string savePath = SelectWorld[selWorld].WorldPath + fmt::format_ne("save{0}.savx", selSave);
    std::string savePath = makeGameSavePath(w.WorldFilePath,
                                            fmt::format_ne("save{0}.savx", selSave));
    std::string legacyGamesaveLocker = makeGameSavePath(w.WorldFilePath,
                                                        fmt::format_ne("save{0}.nosave", selSave));

//    Open SelectWorld[selWorld].WorldPath + "save" + selSave + ".sav" For Output As #1;
    sav.lives = Lives;
    sav.hundreds = (g_100s >= 0) ? g_100s + 1 : g_100s;
    sav.coins = uint32_t(Coins);
    sav.points = uint32_t(Score);
    sav.worldPosX = (int)WorldPlayer[1].Location.X;
    sav.worldPosY = (int)WorldPlayer[1].Location.Y;

    for(A = 1; A <= 5; A++)
    {
        saveCharState c;
        c.id = static_cast<unsigned long>(A);
        c.state = uint32_t(SavedChar[A].State);
        c.itemID = uint32_t(SavedChar[A].HeldBonus);
        c.mountID = uint32_t(SavedChar[A].Mount);
        c.mountType = uint32_t(SavedChar[A].MountType);
        c.health = uint32_t(SavedChar[A].Hearts);
        sav.characterStates.push_back(c);
    }
    sav.musicID = uint32_t(curWorldMusic);
    sav.musicFile = curWorldMusicFile;
    sav.last_hub_warp = static_cast<unsigned long>(ReturnWarpSaved);
    sav.last_hub_level_file = FileRecentSubHubLevel;

    // ABOVE GETS SKIPPED BY FINDSAVES
    sav.gameCompleted = BeatTheGame; // Can only get 99% until you finish the game;

    for(A = 1; A <= numWorldLevels; A++)
        sav.visibleLevels.emplace_back(A, WorldLevel[A].Active);

    for(A = 1; A <= numWorldPaths; A++)
        sav.visiblePaths.emplace_back(A, WorldPath[A].Active);

    for(A = 1; A <= numScenes; A++)
        sav.visibleScenery.emplace_back(A, Scene[A].Active);

    for(const auto& star : Star)
        sav.gottenStars.emplace_back(star.level, star.Section);

    savedLayerSaveEntry savedLayer;
    for(A = 0; ExportSavedLayer(nullptr, savedLayer, A); A++)
        sav.savedLayers.push_back(savedLayer);

    sav.totalStars = uint32_t(MaxWorldStars);

#ifdef THEXTECH_ENABLE_LUNA_AUTOCODE
    gSavedVarBank.WriteBank();
    if(gLunaVarBank.name == "LunaDLL" && !gLunaVarBank.data.empty())
        sav.userData.store.push_back(gLunaVarBank);
#endif

    // save extra settings here
    g_config.SaveEpisodeConfig(sav.userData);

    ExportLevelSaveInfo(sav);

    FileFormats::WriteExtendedSaveFileF(savePath, sav);

    if(Files::fileExists(legacyGamesaveLocker))
        Files::deleteFile(legacyGamesaveLocker); // Remove the gamesave locker of legacy file

    // Also, save the speed-running states
    speedRun_saveStats();

    AppPathManager::syncFs();
}

void LoadGame()
{
    int A = 0;
    size_t i = 0;
    const auto &w = SelectWorld[selWorld];
//    std::string newInput;

    GamesaveData sav;
    std::string savePath = makeGameSavePath(w.WorldFilePath,
                                            fmt::format_ne("save{0}.savx", selSave));
    std::string savePathOld = s_legacy_save_path(w, selSave);
    std::string legacySaveLocker = makeGameSavePath(w.WorldFilePath,
                                                    fmt::format_ne("save{0}.nosave", selSave));

    if(Files::fileExists(savePath))
        FileFormats::ReadExtendedSaveFileF(savePath, sav);
    else if(!Files::fileExists(legacySaveLocker) && Files::fileExists(savePathOld))
        FileFormats::ReadSMBX64SavFileF(savePathOld, sav);
    else
    {
        pLogDebug("Game save file not found: %s", savePath.c_str());
        return;
    }

    if(!sav.meta.ReadFileValid)
    {
        pLogWarning("Invalid game save file: %s [%s]; ", sav.meta.filename.c_str(), sav.meta.ERROR_info.c_str());
        return;
    }

    // load settings from save file here
    g_config.LoadEpisodeConfig(sav.userData);

    Lives = sav.lives;
    g_100s = (sav.hundreds == 0) ? sav.lives : ((sav.hundreds > 0) ? sav.hundreds - 1 : sav.hundreds);
    Coins = int(sav.coins);
    Score = int(sav.points);
    BeatTheGame = sav.gameCompleted;
    WorldPlayer[1].Location.X = sav.worldPosX;
    WorldPlayer[1].Location.Y = sav.worldPosY;

    if(Lives > 99)
        Lives = 99;
    if(Coins > 99)
        Coins = 0;
    if(Score > 9999990)
        Score = 9999990;

    if(g_100s > 9999)
        g_100s = 9999;
    else if(g_100s < -9999)
        g_100s = -9999;

    curWorldMusic = int(sav.musicID);
    curWorldMusicFile = sav.musicFile;

    if(g_config.enable_last_warp_hub_resume)
    {
        ReturnWarp = int(sav.last_hub_warp);
        FileRecentSubHubLevel = sav.last_hub_level_file;
        if(ReturnWarp > maxWarps)
            ReturnWarp = 0; // Invalid value
    }
    else
    {
        // Keep the vanilla behavior, and let players feel the pain!, MuhahahahA! >:-D
        ReturnWarp = 0;
        FileRecentSubHubLevel.clear();
    }

    ReturnWarpSaved = ReturnWarp;

    for(A = 1, i = 0; A <= 5; A++, i++)
    {
        SavedChar[A] = SavedChar_t();
        SavedChar[A].Character = A;
    }

    for(auto &s : sav.characterStates)
    {
        if((s.id < 1) || (s.id > 5))
            continue;
        A = int(s.id);

        s_LoadCharacter(SavedChar[A], s);
    }


    for(auto &p : sav.visiblePaths)
    {
        A = static_cast<int>(p.first);
        if(A > 0 && A <= maxWorldPaths)
            WorldPath[A].Active = p.second;
    }

    for(auto &p : sav.visibleLevels)
    {
        A = static_cast<int>(p.first);
        if(A > 0 && A <= maxWorldLevels)
            WorldLevel[A].Active = p.second;
    }

    for(auto &p : sav.visibleScenery)
    {
        A = static_cast<int>(p.first);
        if(A > 0 && A <= maxScenes)
            Scene[A].Active = p.second;
    }

    if(Star.capacity() < sav.gottenStars.size())
        Star.reserve(sav.gottenStars.size());

    for(auto &p : sav.gottenStars)
    {
        Star_t star;
        star.level = p.first;
        star.Section = p.second;
        Star.push_back(std::move(star));
    }

    for(auto& p : sav.savedLayers)
        LoadSavedLayer(nullptr, p);

    numStars = int(sav.gottenStars.size());

    for(A = 1; A <= numPlayers; A++)
        Player[A] = SavedChar[Player[A].Character];

#ifdef THEXTECH_ENABLE_LUNA_AUTOCODE
    gLunaVarBank = saveUserData::DataSection();
    for(auto &s : sav.userData.store)
    {
        if(s.name == "LunaDLL" && s.location == saveUserData::DATA_GLOBAL)
        {
            gLunaVarBank = s;
            break;
        }
    }

    gSavedVarBank.TryLoadWorldVars();
#endif

    ImportLevelSaveInfo(sav);
}

void ClearGame(bool punnish)
{
    curWorldMusic = 0;
    curWorldMusicFile.clear();

    ReturnWarp = 0;
    ReturnWarpSaved = ReturnWarp;

    IsHubLevel = false;
    FileRecentSubHubLevel.clear();

    WorldPlayer[1].Location.X = -1;
    WorldPlayer[1].Location.Y = -1;

    for(int A = 1, i = 0; A <= 5; A++, i++)
    {
        SavedChar[A].State = 1;
        SavedChar[A].HeldBonus = NPCID(0);
        SavedChar[A].Mount = 0;
        SavedChar[A].MountType = 0;
        SavedChar[A].Hearts = 1;
        SavedChar[A].Character = A;
    }

    for(int A = 1; A <= maxWorldPaths; ++A)
        WorldPath[A].Active = false;

    for(int A = 1; A <= maxWorldLevels; ++A)
    {
        WorldLevel[A].Active = false;
        WorldLevel[A].save_info = LevelSaveInfo_t();
    }

    for(int A = 1; A <= maxScenes; ++A)
        Scene[A].Active = true;

    // maxStars = 0;
    Star.clear();
    numStars = 0;

    LevelWarpSaveEntries.clear();

#ifdef THEXTECH_ENABLE_LUNA_AUTOCODE
    gLunaVarBank = saveUserData::DataSection();
    gSavedVarBank.ClearBank();
#endif

    if(punnish && selSave) // Remove gamesave of user who was used a trap cheat
        DeleteSave(selWorld, selSave);
}


void DeleteSave(int world, int save)
{
    const auto &w = SelectWorld[world];
    std::vector<std::string> deleteList;

#define AddFile(f) \
    deleteList.push_back(makeGameSavePath(w.WorldFilePath,\
                                          fmt::format_ne(f, save)));

    AddFile("save{0}.savx");
    AddFile("timers{0}.ini");
    AddFile("deaths-{0}.rip");
    AddFile("fails-{0}.rip");
    AddFile("demos-{0}.dmo");

    // Clear all files in list
    for(auto &s : deleteList)
    {
        if(Files::fileExists(s))
            Files::deleteFile(s);
    }

    std::string legacySave = s_legacy_save_path(w, save);
    std::string legacySaveLocker = makeGameSavePath(w.WorldFilePath,
                                                    fmt::format_ne("save{0}.nosave", save));

    // If legacy gamesave file exists, make the locker file to make illusion that old file got been removed
    if(Files::fileExists(legacySave))
    {
        auto *f = Files::open_file(legacySaveLocker, "wb");
        if(f)
        {
            const char s[] = "If this file exists, the Vanilla \"save*.sav\" inside the episode folder is ignored.";
            SDL_RWwrite(f, s, 1, sizeof(s) - 1);
            SDL_RWclose(f);
        }
    }

#undef AddFile

#ifdef __EMSCRIPTEN__
    AppPathManager::syncFs();
#endif
}


static void copySaveFile(const SelectWorld_t& w, const char*file_mask, int src, int dst)
{
    std::string filePathSrc = makeGameSavePath(w.WorldFilePath,
                                               fmt::format_ne(file_mask, src));
    std::string filePathDst = makeGameSavePath(w.WorldFilePath,
                                               fmt::format_ne(file_mask, dst));
    Files::copyFile(filePathDst, filePathSrc, true);
}

void CopySave(int world, int src, int dst)
{
    const auto &w = SelectWorld[world];

    std::string savePathSrc = makeGameSavePath(w.WorldFilePath,
                                               fmt::format_ne("save{0}.savx", src));
    std::string savePathDst = makeGameSavePath(w.WorldFilePath,
                                               fmt::format_ne("save{0}.savx", dst));
    std::string legacySaveLocker = makeGameSavePath(w.WorldFilePath,
                                                    fmt::format_ne("save{0}.nosave", dst));

    if(!Files::fileExists(savePathSrc))
    {
        // Attempt to import an old game-save from the episode directory
        std::string savePathOld = s_legacy_save_path(w, src);

        GamesaveData sav;
        bool succ = false;

        if(Files::fileExists(savePathOld))
            succ = FileFormats::ReadSMBX64SavFileF(savePathOld, sav);

        if(succ)
            FileFormats::WriteExtendedSaveFileF(savePathSrc, sav);

        if(Files::fileExists(legacySaveLocker))
            Files::deleteFile(legacySaveLocker);
    }

    Files::copyFile(savePathDst, savePathSrc, true);

    copySaveFile(w, "timers{0}.ini", src, dst);
    copySaveFile(w, "fails-{0}.rip", src, dst);
    copySaveFile(w, "deaths-{0}.rip", src, dst);
    copySaveFile(w, "demos-{0}.dmo", src, dst);

#ifdef __EMSCRIPTEN__
    AppPathManager::syncFs();
#endif
}
