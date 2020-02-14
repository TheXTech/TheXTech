#include "../globals.h"
#include "../game_main.h"

#include <Utils/files.h>
#include <PGE_File_Formats/file_formats.h>
#include <fmt_format_ne.h>

void SaveGame()
{
    int A = 0;
    if(Cheater == true)
        return;
    for(A = numPlayers; A >= 1; A--)
        SavedChar[Player[A].Character] = Player[A];
    for(A = numStars; A >= 1; A--)
    {
        if(Star[A].level == "")
        {
            if(numStars > A)
            {
                Star[A] = Star[numStars];
                Star[numStars].level = "";
                Star[numStars].Section = 0;
            }
            numStars = numStars - 1;
        }
    }

    GamesaveData sav;
    std::string savePath = SelectWorld[selWorld].WorldPath + fmt::format_ne("save{0}.savx", selSave);

//    Open SelectWorld[selWorld].WorldPath + "save" + selSave + ".sav" For Output As #1;
    sav.lives = Lives;
    sav.coins = Coins;
    sav.worldPosX = WorldPlayer[1].Location.X;
    sav.worldPosY = WorldPlayer[1].Location.Y;

    for(A = 1; A <= 5; A++)
    {
        saveCharState c;
        c.state = SavedChar[A].State;
        c.itemID = SavedChar[A].HeldBonus;
        c.mountID = SavedChar[A].Mount;
        c.mountType = SavedChar[A].MountType;
        c.health = SavedChar[A].Hearts;
        sav.characterStates.push_back(c);
    }
    sav.musicID = curWorldMusic;

    // ABOVE GETS SKIPPED BY FINDSAVES
    sav.gameCompleted = BeatTheGame; // Can only get 99% until you finish the game;

    for(A = 1; A <= numWorldLevels; A++)
        sav.visibleLevels.push_back({A, WorldLevel[A].Active});

    for(A = 1; A <= numWorldPaths; A++)
        sav.visiblePaths.push_back({A, WorldPath[A].Active});

    for(A = 1; A <= numScenes; A++)
        sav.visibleScenery.push_back({A, Scene[A].Active});

    for(A = 1; A <= numStars; A++)
        sav.gottenStars.push_back({Star[A].level, Star[A].Section});

    sav.totalStars = MaxWorldStars;

    FileFormats::WriteExtendedSaveFileF(savePath, sav);
}

void LoadGame()
{
    int A = 0;
    size_t i = 0;
    std::string newInput = "";

    GamesaveData sav;
    std::string savePath = SelectWorld[selWorld].WorldPath + fmt::format_ne("save{0}.savx", selSave);
    std::string savePathOld = SelectWorld[selWorld].WorldPath + fmt::format_ne("save{0}.sav", selSave);

    if(!Files::fileExists(savePath) && Files::fileExists(savePathOld))
        FileFormats::ReadSMBX64SavFileF(savePathOld, sav);
    else if(Files::fileExists(savePath))
        FileFormats::ReadSMBX64SavFileF(savePath, sav);
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

    Lives = sav.lives;
    Coins = sav.coins;
    WorldPlayer[1].Location.X = sav.worldPosX;
    WorldPlayer[1].Location.Y = sav.worldPosY;

    for(A = 1, i = 0; A <= 5; A++, i++)
    {
        if(i < sav.characterStates.size())
        {
            auto &s = sav.characterStates[i];
            SavedChar[A].State = s.state;
            SavedChar[A].HeldBonus = s.itemID;
            SavedChar[A].Mount = s.mountID;
            SavedChar[A].MountType = s.mountType;
            SavedChar[A].Hearts = s.health;
            SavedChar[A].Character = A;
        }
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

    numStars = sav.totalStars;

    for(A = 1; A <= numPlayers; A++)
        Player[A] = SavedChar[Player[A].Character];
}
