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

#include "../globals.h"
#include "../game_main.h"
#include "../compat.h"
#include "speedrunner.h"
#ifdef THEXTECH_ENABLE_LUNA_AUTOCODE
#include "../script/luna/lunavarbank.h"
#endif

#include <Utils/files.h>
#include <DirManager/dirman.h>
#include <AppPath/app_path.h>
#include <PGE_File_Formats/file_formats.h>
#include <fmt_format_ne.h>

#include "menu_main.h"

std::string makeGameSavePath(std::string episode, std::string world, std::string saveFile)
{
    std::string gameSaveDir = AppPathManager::gameSaveRootDir() + Files::basename(Files::dirname(episode + world));

    if(!DirMan::exists(gameSaveDir))
    {
        pLogDebug("Creating directory [%s]", gameSaveDir.c_str());
        DirMan::mkAbsPath(gameSaveDir);
    }

    std::string ret = gameSaveDir + "/"+ world + "-" + saveFile;

    pLogDebug("Save data path for ep [%s], wld [%s], file [%s] -> [%s]", episode.c_str(), world.c_str(), saveFile.c_str(), ret.c_str());

    return ret;
}

void SaveGame()
{
    int A = 0;

    if(Cheater)
        return;

    for(A = numPlayers; A >= 1; A--)
        SavedChar[Player[A].Character] = Player[A];
    for(A = numStars; A >= 1; A--)
    {
        if(Star[A].level.empty())
        {
            if(numStars > A)
            {
                Star[A] = Star[numStars];
                Star[numStars].level.clear();
                Star[numStars].Section = 0;
            }
            numStars -= 1;
        }
    }

    const auto &w = SelectWorld[selWorld];

    GamesaveData sav;
//    std::string savePath = SelectWorld[selWorld].WorldPath + fmt::format_ne("save{0}.savx", selSave);
    std::string savePath = makeGameSavePath(w.WorldPath,
                                            w.WorldFile,
                                            fmt::format_ne("save{0}.savx", selSave));
    std::string legacyGamesaveLocker = makeGameSavePath(w.WorldPath,
                                                        w.WorldFile,
                                                        fmt::format_ne("save{0}.nosave", selSave));

//    Open SelectWorld[selWorld].WorldPath + "save" + selSave + ".sav" For Output As #1;
    sav.lives = int(Lives);
    sav.coins = uint32_t(Coins);
    sav.points = uint32_t(Score);
    sav.worldPosX = WorldPlayer[1].Location.X;
    sav.worldPosY = WorldPlayer[1].Location.Y;

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

    // ABOVE GETS SKIPPED BY FINDSAVES
    sav.gameCompleted = BeatTheGame; // Can only get 99% until you finish the game;

    for(A = 1; A <= numWorldLevels; A++)
        sav.visibleLevels.emplace_back(A, WorldLevel[A].Active);

    for(A = 1; A <= numWorldPaths; A++)
        sav.visiblePaths.emplace_back(A, WorldPath[A].Active);

    for(A = 1; A <= numScenes; A++)
        sav.visibleScenery.emplace_back(A, Scene[A].Active);

    for(A = 1; A <= numStars; A++)
        sav.gottenStars.emplace_back(Star[A].level, Star[A].Section);

    sav.totalStars = uint32_t(MaxWorldStars);

#ifdef THEXTECH_ENABLE_LUNA_AUTOCODE
    gSavedVarBank.WriteBank();
    if(gLunaVarBank.name == "LunaDLL" && !gLunaVarBank.data.empty())
        sav.userData.store.push_back(gLunaVarBank);
#endif

    FileFormats::WriteExtendedSaveFileF(savePath, sav);

    if(Files::fileExists(legacyGamesaveLocker))
        Files::deleteFile(legacyGamesaveLocker); // Remove the gamesave locker of legacy file

    // Also, save the speed-running states
    speedRun_saveStats();

#ifdef __EMSCRIPTEN__
    AppPathManager::syncFs();
#endif
}

void LoadGame()
{
    int A = 0;
    size_t i = 0;
//    std::string newInput;

    GamesaveData sav;
    std::string savePath = makeGameSavePath(SelectWorld[selWorld].WorldPath,
                                            SelectWorld[selWorld].WorldFile,
                                            fmt::format_ne("save{0}.savx", selSave));
    std::string savePathOld = SelectWorld[selWorld].WorldPath + fmt::format_ne("save{0}.savx", selSave);
    std::string savePathAncient = SelectWorld[selWorld].WorldPath + fmt::format_ne("save{0}.sav", selSave);

    if(Files::fileExists(savePath))
        FileFormats::ReadExtendedSaveFileF(savePath, sav);
    else if(Files::fileExists(savePathOld))
        FileFormats::ReadExtendedSaveFileF(savePathOld, sav);
    else if(Files::fileExists(savePathAncient))
        FileFormats::ReadSMBX64SavFileF(savePathAncient, sav);
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

    Lives = float(sav.lives);
    Coins = int(sav.coins);
    Score = int(sav.points);
    BeatTheGame = sav.gameCompleted;
    WorldPlayer[1].Location.X = double(sav.worldPosX);
    WorldPlayer[1].Location.Y = double(sav.worldPosY);

    if(Lives > 99)
        Lives = 99;
    if(Coins > 99)
        Coins = 0;
    if(Score > 9999990)
        Score = 9999990;

    curWorldMusic = int(sav.musicID);
    curWorldMusicFile = sav.musicFile;

    if(g_compatibility.enable_last_warp_hub_resume)
    {
        ReturnWarp = int(sav.last_hub_warp);
        if(ReturnWarp > maxWarps)
            ReturnWarp = 0; // Invalid value
    }
    else
    {
        // Keep the vanilla behavior, and let players feel the pain!, MuhahahahA! >:-D
        ReturnWarp = 0;
    }

    ReturnWarpSaved = ReturnWarp;

    for(A = 1, i = 0; A <= 5; A++, i++)
    {
        SavedChar[A].State = 1;
        SavedChar[A].HeldBonus = 0;
        SavedChar[A].Mount = 0;
        SavedChar[A].MountType = 0;
        SavedChar[A].Hearts = 1;
        SavedChar[A].Character = A;
    }

    for(auto &s : sav.characterStates)
    {
        if(s.id < 1 || s.id > 5)
            continue;
        A = int(s.id);

        if(s.state < 1 || s.state > 10)
            SavedChar[A].State = 1;
        else
            SavedChar[A].State = int(s.state);

        SavedChar[A].HeldBonus = int(s.itemID);

        switch(s.mountID)
        {
        default:
        case 0:
            SavedChar[A].Mount = 0;
            SavedChar[A].MountType = 0;
            break;
        case 1: case 2: case 3:
            SavedChar[A].Mount = int(s.mountID);
        }

        SavedChar[A].MountType = int(s.mountType);
        switch(s.mountID)
        {
        case 1:
            if(s.mountType < 1 || s.mountType > 3)
                SavedChar[A].MountType = 1;
            break;
        default:
            break;
        case 3:
            if(s.mountType < 1 || s.mountType > 8)
                SavedChar[A].MountType = 1;
            break;
        }

        SavedChar[A].Hearts = int(s.health);
        SavedChar[A].Character = A;
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

    A = 1;
    for(auto &p : sav.gottenStars)
    {
        Star[A].level = p.first;
        Star[A].Section = p.second;
        A++;
    }

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
}

void ClearGame(bool punnish)
{
    curWorldMusic = 0;
    curWorldMusicFile.clear();

    ReturnWarp = 0;
    ReturnWarpSaved = ReturnWarp;

    WorldPlayer[1].Location.X = -1;
    WorldPlayer[1].Location.Y = -1;

    for(int A = 1, i = 0; A <= 5; A++, i++)
    {
        SavedChar[A].State = 1;
        SavedChar[A].HeldBonus = 0;
        SavedChar[A].Mount = 0;
        SavedChar[A].MountType = 0;
        SavedChar[A].Hearts = 1;
        SavedChar[A].Character = A;
    }

    for(int A = 1; A <= maxWorldPaths; ++A)
        WorldPath[A].Active = false;

    for(int A = 1; A <= maxWorldLevels; ++A)
        WorldLevel[A].Active = false;

    for(int A = 1; A <= maxScenes; ++A)
        Scene[A].Active = true;

    for(int A = 1; A <= maxStarsNum; ++A)
    {
        Star[A].level.clear();
        Star[A].Section = 0;
    }

    maxStars = 0;
    numStars = 0;

#ifdef THEXTECH_ENABLE_LUNA_AUTOCODE
    gLunaVarBank = saveUserData::DataSection();
    gSavedVarBank.ClearBank();
#endif

    if(punnish) // Remove gamesave of user who was used a trap cheat
        DeleteSave(selWorld, selSave);
}
