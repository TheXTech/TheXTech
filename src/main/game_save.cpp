/*
 * A2xTech - A platform game engine ported from old source code for VB6
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

#include <Utils/files.h>
#include <PGE_File_Formats/file_formats.h>
#include <fmt_format_ne.h>

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
            numStars = numStars - 1;
        }
    }

    GamesaveData sav;
    std::string savePath = SelectWorld[selWorld].WorldPath + fmt::format_ne("save{0}.savx", selSave);

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

    FileFormats::WriteExtendedSaveFileF(savePath, sav);
}

void LoadGame()
{
    int A = 0;
    size_t i = 0;
    std::string newInput;

    GamesaveData sav;
    std::string savePath = SelectWorld[selWorld].WorldPath + fmt::format_ne("save{0}.savx", selSave);
    std::string savePathOld = SelectWorld[selWorld].WorldPath + fmt::format_ne("save{0}.sav", selSave);

    if(!Files::fileExists(savePath) && Files::fileExists(savePathOld))
        FileFormats::ReadSMBX64SavFileF(savePathOld, sav);
    else if(Files::fileExists(savePath))
        FileFormats::ReadExtendedSaveFileF(savePath, sav);
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

    ReturnWarp = int(sav.last_hub_warp);
    if(ReturnWarp > maxWarps)
        ReturnWarp = 0; // Invalid value
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
}
