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

#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_thread.h>
#include <fmt_format_ne.h>
#include <atomic>

#include <AppPath/app_path.h>
#include <DirManager/dirman.h>
#include <Utils/files.h>
#include <PGE_File_Formats/file_formats.h>

#include "menu_main.h"
#include "game_info.h"
#include "menu_playerselect.h"
#include "menu_controls.h"

#include "speedrunner.h"
#include "../game_main.h"
#include "../sound.h"
#include "../player.h"
#include "../collision.h"
#include "../graphics.h"
#include "../control/joystick.h"
#include "../config.h"
#include "level_file.h"
#include "pge_delay.h"


MainMenuContent g_mainMenu;

static SDL_atomic_t         loading;
static SDL_atomic_t         loadingProgrss;
static SDL_atomic_t         loadingProgrssMax;

static SDL_Thread*          loadingThread = nullptr;


void initMainMenu()
{
    SDL_AtomicSet(&loading, 0);
    SDL_AtomicSet(&loadingProgrss, 0);
    SDL_AtomicSet(&loadingProgrssMax, 0);

    g_mainMenu.mainGame = "Main Game";
    g_mainMenu.main1PlayerGame = "1 Player Game";
    g_mainMenu.main2PlayerGame = "2 Player Game";
    g_mainMenu.mainBattleGame = "Battle Game";
    g_mainMenu.mainOptions = "Options";
    g_mainMenu.mainExit = "Exit";

    g_mainMenu.loading = "Loading...";

    for(int i = 1; i <= numCharacters; ++i)
        g_mainMenu.selectPlayer[i] = fmt::format_ne("{0} game", g_gameInfo.characterName[i]);

    g_mainMenu.playerSelTitle = "Character Select";
    g_mainMenu.wordPlayer = "Player";
    g_mainMenu.wordProfile = "Profile";
    g_mainMenu.playerSelStartGame = "Start Game";
    g_mainMenu.playerSelAttachController = "Press A Button";
    g_mainMenu.rotateProfileMessage = "(L/R Rotate)";

    g_mainMenu.controlsTitle = "Controls";
    g_mainMenu.controlsConnected = "Connected:";
    g_mainMenu.controlsDeleteKey = "(Alt Jump to Delete)";
    g_mainMenu.controlsDeviceTypes = "Device Types";
    g_mainMenu.controlsInUse = "(In Use)";
    g_mainMenu.controlsNotInUse = "(Not In Use)";
    g_mainMenu.wordProfiles = "Profiles";
    g_mainMenu.wordButtons = "Buttons";

    g_mainMenu.controlsReallyDeleteProfile = "Really delete profile?";
    g_mainMenu.controlsNewProfile = "<New Profile>";

    g_mainMenu.wordNo = "No";
    g_mainMenu.wordYes = "Yes";
}


static int ScrollDelay = 0;
static int menuPlayersNum = 0;
static int menuBattleMode = false;

static int menuCopySaveSrc = 0;
static int menuCopySaveDst = 0;

static int FindWorldsThread(void *)
{
    FindWorlds();
    return 0;
}

void FindWorlds()
{
    NumSelectWorld = 0;

    std::vector<std::string> worldRoots;
    worldRoots.push_back(AppPath + "worlds/");
#if (defined(__APPLE__) && defined(USE_BUNDLED_ASSETS)) || defined(FIXED_ASSETS_PATH)
    worldRoots.push_back(AppPathManager::userWorldsRootDir() + "/");
#endif

    SelectWorld.clear();
    SelectWorld.push_back(SelectWorld_t()); // Dummy entry

    SDL_AtomicSet(&loadingProgrss, 0);
    SDL_AtomicSet(&loadingProgrssMax, 0);

    for(const auto &worldsRoot : worldRoots)
    {
        std::vector<std::string> dirs;
        DirMan episodes(worldsRoot);
        episodes.getListOfFolders(dirs);
        SDL_AtomicAdd(&loadingProgrssMax, dirs.size());
    }

    for(const auto &worldsRoot : worldRoots)
    {
        DirMan episodes(worldsRoot);

        std::vector<std::string> dirs;
        std::vector<std::string> files;
        episodes.getListOfFolders(dirs);
        WorldData head;

        for(auto &dir : dirs)
        {
            std::string epDir = worldsRoot + dir + "/";
            DirMan episode(epDir);
            episode.getListOfFiles(files, {".wld", ".wldx"});

            for(std::string &fName : files)
            {
                std::string wPath = epDir + fName;
                if(FileFormats::OpenWorldFileHeader(wPath, head))
                {
                    SelectWorld_t w;
                    w.WorldName = head.EpisodeTitle;
                    head.charactersToS64();
                    w.WorldPath = epDir;
                    w.WorldFile = fName;
                    if(w.WorldName.empty())
                        w.WorldName = fName;
                    w.blockChar[1] = head.nocharacter1;
                    w.blockChar[2] = head.nocharacter2;
                    w.blockChar[3] = head.nocharacter3;
                    w.blockChar[4] = head.nocharacter4;
                    w.blockChar[5] = head.nocharacter5;

                    SelectWorld.push_back(w);
                }
            }

            SDL_AtomicAdd(&loadingProgrss, 1);
        }
    }

    NumSelectWorld = (SelectWorld.size() - 1);

    SDL_AtomicSet(&loading, 0);
}

static int FindLevelsThread(void *)
{
    FindLevels();
    return 0;
}

void FindLevels()
{
    std::vector<std::string> battleRoots;
    battleRoots.push_back(AppPath + "battle/");
#if (defined(__APPLE__) && defined(USE_BUNDLED_ASSETS)) || defined(FIXED_ASSETS_PATH)
    battleRoots.push_back(AppPathManager::userBattleRootDir() + "/");
#endif

    SelectWorld.clear();
    SelectWorld.push_back(SelectWorld_t()); // Dummy entry

    NumSelectWorld = 1;
    SelectWorld.push_back(SelectWorld_t()); // "random level" entry
    SelectWorld[1].WorldName = "Random Level";
    LevelData head;

    SDL_AtomicSet(&loadingProgrss, 0);
    SDL_AtomicSet(&loadingProgrssMax, 0);

    for(const auto &battleRoot : battleRoots)
    {
        std::vector<std::string> files;
        DirMan battleLvls(battleRoot);
        battleLvls.getListOfFiles(files, {".lvl", ".lvlx"});
        SDL_AtomicAdd(&loadingProgrssMax, files.size());
    }

    for(const auto &battleRoot : battleRoots)
    {
        std::vector<std::string> files;
        DirMan battleLvls(battleRoot);
        battleLvls.getListOfFiles(files, {".lvl", ".lvlx"});
        for(std::string &fName : files)
        {
            std::string wPath = battleRoot + fName;
            if(FileFormats::OpenLevelFileHeader(wPath, head))
            {
                SelectWorld_t w;
                w.WorldPath = battleRoot;
                w.WorldFile = fName;
                w.WorldName = head.LevelName;
                if(w.WorldName.empty())
                    w.WorldName = fName;
                SelectWorld.push_back(w);
            }
            SDL_AtomicAdd(&loadingProgrss, 1);
        }
    }

    NumSelectWorld = (SelectWorld.size() - 1);
    SDL_AtomicSet(&loading, 0);
}


static void s_handleMouseMove(int items, int x, int y, int maxWidth, int itemHeight)
{
    For(A, 0, items)
    {
        if(MenuMouseY >= y + A * itemHeight && MenuMouseY <= y + 16 + A * itemHeight)
        {
            if(MenuMouseX >= x && MenuMouseX <= x + maxWidth)
            {
                if(MenuMouseRelease && MenuMouseDown)
                    MenuMouseClick = true;
                if(MenuCursor != A)
                {
                    PlaySoundMenu(SFX_Slide);
                    MenuCursor = A;
                    break;
                }
            }
        }
    }
}



bool mainMenuUpdate()
{
    int B;
    Location_t tempLocation;
    int menuLen;
    Player_t blankPlayer;

    bool upPressed = SharedControls.MenuUp;
    bool downPressed = SharedControls.MenuDown;
    bool leftPressed = SharedControls.MenuLeft;
    bool rightPressed = SharedControls.MenuRight;

    bool menuDoPress = SharedControls.MenuDo;
    bool menuBackPress = SharedControls.MenuBack;

    for(int i = 0; i < maxLocalPlayers; i++)
    {
        Controls_t &c = Player[i+1].Controls;

        menuDoPress |= c.Start || c.Jump;
        menuBackPress |= c.Run;

        upPressed |= c.Up;
        downPressed |= c.Down;
        leftPressed |= c.Left;
        rightPressed |= c.Right;
    }

    {
        if(frmMain.MousePointer != 99)
        {
            frmMain.MousePointer = 99;
            showCursor(0);
        }

        {
            bool k = false;
            k |= menuBackPress;
            k |= menuDoPress;
            k |= upPressed;
            k |= downPressed;
            k |= leftPressed;
            k |= rightPressed;

            if(!k)
                MenuCursorCanMove = true;

        }

        if(!g_pollingInput && (MenuMode != MENU_CHARACTER_SELECT_NEW && MenuMode != MENU_CHARACTER_SELECT_NEW_BM))
        {
            int cursorDelta = 0;

            if(upPressed)
            {
                if(MenuCursorCanMove)
                {
                    MenuCursor -= 1;
                    cursorDelta = -1;
                }

                MenuCursorCanMove = false;
            }
            else if(downPressed)
            {
                if(MenuCursorCanMove)
                {
                    MenuCursor += 1;
                    cursorDelta = +1;
                }
                MenuCursorCanMove = false;
            }

            if(cursorDelta != 0)
            {
                if(MenuMode >= MENU_CHARACTER_SELECT_BASE && MenuMode <= MENU_CHARACTER_SELECT_BASE_END)
                {
                    while((MenuCursor == (PlayerCharacter - 1) &&
                          (MenuMode == MENU_CHARACTER_SELECT_2P_S2 || MenuMode == MENU_CHARACTER_SELECT_BM_S2)) ||
                           blockCharacter[MenuCursor + 1])
                    {
                        MenuCursor += cursorDelta;
                        if(MenuCursor < 0)
                            MenuCursor = numCharacters - 1;
                        else if(MenuCursor >= numCharacters)
                            MenuCursor = 0;
                    }
                }
                PlaySoundMenu(SFX_Slide);
            }

        } // No keyboard/Joystick grabbing active

        if(SDL_AtomicGet(&loading))
        {
            if((menuDoPress && MenuCursorCanMove) || MenuMouseClick)
                PlaySoundMenu(SFX_BlockHit);
            if(MenuCursor != 0)
                MenuCursor = 0;
        }
        // Main Menu
        else if(MenuMode == MENU_MAIN)
        {
            if(MenuMouseMove)
            {
                For(A, 0, 10)
                {
                    if(MenuMouseY >= 350 + A * 30 && MenuMouseY <= 366 + A * 30)
                    {
                        int i = 0;
                        if(g_config.LegacyPlayerSelect && A == i++)
                            menuLen = 18 * g_mainMenu.main1PlayerGame.size() - 2;
                        else if(g_config.LegacyPlayerSelect && A == i++)
                            menuLen = 18 * g_mainMenu.main2PlayerGame.size() - 2;
                        else if(!g_config.LegacyPlayerSelect && A == i++)
                            menuLen = 18 * g_mainMenu.mainGame.size() - 2;
                        else if(A == i++)
                            menuLen = 18 * g_mainMenu.mainBattleGame.size();
                        else if(A == i++)
                            menuLen = 18 * g_mainMenu.mainOptions.size();
                        else if(A == i++)
                            menuLen = 18 * g_mainMenu.mainExit.size();
                        else
                            break;

                        if(MenuMouseX >= 300 && MenuMouseX <= 300 + menuLen)
                        {
                            if(MenuMouseRelease && MenuMouseDown)
                                MenuMouseClick = true;

                            if(MenuCursor != A)
                            {
                                PlaySoundMenu(SFX_Slide);
                                MenuCursor = A;
                            }
                        }
                    }
                }
            }

            if(menuBackPress && MenuCursorCanMove)
            {
                int quitKeyPos;
                if(g_config.LegacyPlayerSelect)
                    quitKeyPos = 4;
                else
                    quitKeyPos = 3;

                if(MenuCursor != quitKeyPos)
                {
                    MenuCursor = quitKeyPos;
                    PlaySoundMenu(SFX_Slide);
                }
            }
            else if((menuDoPress && MenuCursorCanMove) || MenuMouseClick)
            {
                MenuCursorCanMove = false;
                PlayerCharacter = 0;
                PlayerCharacter2 = 0;

                int i = 0;
                if(MenuCursor == i++)
                {
                    PlaySoundMenu(SFX_Do);
                    MenuMode = MENU_1PLAYER_GAME;
                    menuPlayersNum = 1;
                    menuBattleMode = false;
#ifdef __EMSCRIPTEN__
                    FindWorlds();
#else
                    SDL_AtomicSet(&loading, 1);
                    loadingThread = SDL_CreateThread(FindWorldsThread, "FindWorlds", NULL);
#endif
                    MenuCursor = 0;
                }
                else if(g_config.LegacyPlayerSelect && MenuCursor == i++)
                {
                    PlaySoundMenu(SFX_Do);
                    MenuMode = MENU_2PLAYER_GAME;
                    menuPlayersNum = 2;
                    menuBattleMode = false;
#ifdef __EMSCRIPTEN__
                    FindWorlds();
#else
                    SDL_AtomicSet(&loading, 1);
                    loadingThread = SDL_CreateThread(FindWorldsThread, "FindWorlds", NULL);
#endif
                    MenuCursor = 0;
                }
                else if(MenuCursor == i++)
                {
                    PlaySoundMenu(SFX_Do);
                    MenuMode = MENU_BATTLE_MODE;
                    menuPlayersNum = 2;
                    menuBattleMode = true;
#ifdef __EMSCRIPTEN__
                    FindLevels();
#else
                    SDL_AtomicSet(&loading, 1);
                    loadingThread = SDL_CreateThread(FindLevelsThread, "FindLevels", NULL);
#endif
                    MenuCursor = 0;
                }
                else if(MenuCursor == i++)
                {
                    PlaySoundMenu(SFX_Do);
                    MenuMode = MENU_OPTIONS;
                    MenuCursor = 0;
                }
                else if(MenuCursor == i++)
                {
                    PlaySoundMenu(SFX_Do);
                    frmMain.setTargetTexture();
                    frmMain.clearBuffer();
                    StopMusic();
                    frmMain.repaint();
                    DoEvents();
                    PGE_Delay(500);
                    KillIt();
                    return true;
                }

            }


            int quitKeyPos;
            if(g_config.LegacyPlayerSelect)
                quitKeyPos = 4;
            else
                quitKeyPos = 3;
            if(MenuCursor > quitKeyPos)
                MenuCursor = 0;
            if(MenuCursor < 0)
                MenuCursor = quitKeyPos;
        } // Main Menu

        // Character Select
        else if(MenuMode == MENU_CHARACTER_SELECT_NEW ||
                MenuMode == MENU_CHARACTER_SELECT_NEW_BM)
        {
            int ret = menuPlayerSelect_Logic(menuPlayersNum);
            if(ret == -1)
            {
                if(MenuMode == MENU_CHARACTER_SELECT_NEW_BM)
                {
                    MenuCursor = selWorld - 1;
                    MenuMode = MENU_BATTLE_MODE;
                }
                else
                {
                    MenuCursor = selSave - 1;
                    MenuMode = MENU_SELECT_SLOT_1P;
                }
                MenuCursorCanMove = false;
            }
            else if(ret == 1)
            {
                if(MenuMode == MENU_CHARACTER_SELECT_NEW)
                {
                    MenuCursor = 0;
                    StartEpisode();
                    return true;
                }
                else if(MenuMode == MENU_CHARACTER_SELECT_NEW_BM)
                {
                    MenuCursor = 0;
                    StartBattleMode();
                    return true;
                }
            }
        }
        // Character Select
        else if(MenuMode == MENU_CHARACTER_SELECT_1P ||
                MenuMode == MENU_CHARACTER_SELECT_2P_S1 ||
                MenuMode == MENU_CHARACTER_SELECT_2P_S2 ||
                MenuMode == MENU_CHARACTER_SELECT_BM_S1 ||
                MenuMode == MENU_CHARACTER_SELECT_BM_S2)
        {
            if(MenuMouseMove)
            {
                B = 0;
                For(A, 0, 4)
                {
                    if(blockCharacter[A + 1])
                    {
                        B -= 30;
                    }
                    else
                    {
                        if(MenuMouseY >= 350 + A * 30 + B && MenuMouseY <= 366 + A * 30 + B)
                        {
                            if(A >= 0 && A < numCharacters)
                            {
                                menuLen = 18 * g_mainMenu.selectPlayer[A + 1].size();
                                if(A == 0)
                                    menuLen += 2;
                            }
                            else
                            {
                                menuLen = 180;
                            }

                            if(MenuMouseX >= 300 && MenuMouseX <= 300 + menuLen)
                            {
                                if(MenuMouseRelease && MenuMouseDown)
                                    MenuMouseClick = true;
                                if(MenuCursor != A)
                                {
                                    if(
                                        ((MenuMode == MENU_CHARACTER_SELECT_2P_S2 || MenuMode == MENU_CHARACTER_SELECT_BM_S2) && PlayerCharacter - 1 == A) ||
                                        ((blockCharacter[A + 1]) && MenuMouseClick)
                                    )
                                    {
                                        MenuMouseClick = false;
                                    }
                                    else
                                    {
                                        PlaySoundMenu(SFX_Slide);
                                        MenuCursor = A;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            if(MenuCursorCanMove || MenuMouseClick || MenuMouseBack)
            {
                if(menuBackPress || MenuMouseBack)
                {
                    if(MenuMode == MENU_CHARACTER_SELECT_2P_S2)
                    {
                        MenuMode = MENU_CHARACTER_SELECT_2P_S1;
                        MenuCursor = PlayerCharacter - 1;
                    }
                    else if(MenuMode == MENU_CHARACTER_SELECT_BM_S2)
                    {
                        MenuMode = MENU_CHARACTER_SELECT_BM_S1;
                        MenuCursor = PlayerCharacter - 1;
                    }
                    else
                    {
                        MenuCursor = selWorld - 1;
                        MenuMode /= MENU_CHARACTER_SELECT_BASE;
                    }

                    MenuCursorCanMove = false;
                    PlaySoundMenu(SFX_Slide);
                }
                else if(menuDoPress || MenuMouseClick)
                {
                    PlaySoundMenu(SFX_Do);

                    if(MenuMode == MENU_CHARACTER_SELECT_1P)
                    {
                        PlayerCharacter = MenuCursor + 1;
                        MenuMode = MENU_SELECT_SLOT_1P;
                        MenuCursor = 0;
                    }
                    else if(MenuMode == MENU_CHARACTER_SELECT_2P_S1)
                    {
                        PlayerCharacter = MenuCursor + 1;
                        MenuMode = MENU_CHARACTER_SELECT_2P_S2;
                        MenuCursor = PlayerCharacter2;
                    }
                    else if(MenuMode == MENU_CHARACTER_SELECT_2P_S2)
                    {
                        PlayerCharacter2 = MenuCursor + 1;
                        MenuMode = MENU_SELECT_SLOT_2P;
                        MenuCursor = 0;
                    }
                    else if(MenuMode == MENU_CHARACTER_SELECT_BM_S1)
                    {
                        PlayerCharacter = MenuCursor + 1;
                        MenuMode = MENU_CHARACTER_SELECT_BM_S2;
                        MenuCursor = PlayerCharacter2 - 1;
                        if(MenuCursor < 0)
                            MenuCursor = 0;
                    }
                    else if(MenuMode == MENU_CHARACTER_SELECT_BM_S2)
                    {
                        PlayerCharacter2 = MenuCursor + 1;
                        MenuCursor = 0;
                        StartBattleMode();
                        return true;
                    }

                    MenuCursorCanMove = false;
                }
            }

            if(MenuMode > MENU_MAIN)
            {
                if(MenuCursor > numCharacters - 1)
                {
                    MenuCursor = 0;

                    while((MenuCursor == PlayerCharacter - 1 && (MenuMode == MENU_CHARACTER_SELECT_2P_S2 || MenuMode == MENU_CHARACTER_SELECT_BM_S2)) ||
                          blockCharacter[MenuCursor + 1])
                    {
                        MenuCursor += 1;
                    }

                }

                if(MenuCursor < 0)
                {
                    MenuCursor = numCharacters - 1;

                    while((MenuCursor == PlayerCharacter - 1 && (MenuMode == MENU_CHARACTER_SELECT_2P_S2 || MenuMode == MENU_CHARACTER_SELECT_BM_S2)) ||
                          blockCharacter[MenuCursor + 1])
                    {
                        MenuCursor -= 1;
                    }
                }
            }

            while(((MenuMode == MENU_CHARACTER_SELECT_2P_S2 || MenuMode == MENU_CHARACTER_SELECT_BM_S2) && MenuCursor == PlayerCharacter - 1) ||
                   blockCharacter[MenuCursor + 1])
            {
                MenuCursor += 1;
            }

            if(MenuMode >= MENU_CHARACTER_SELECT_BASE && MenuMode <= MENU_CHARACTER_SELECT_BASE_END)
            {
                if(MenuCursor >= numCharacters)
                {
                    MenuCursor = 0;
                }
                else
                {
                    For(A, 1, numPlayers)
                    {
                        Player[A].Character = MenuCursor + 1;
                        SizeCheck(A);
                    }

                    For(A, 1, numNPCs)
                    {
                        if(NPC[A].Type == 13)
                            NPC[A].Special = MenuCursor + 1;
                    }
                }
            }
        } // Character Select

        // World Select
        else if(MenuMode == MENU_1PLAYER_GAME || MenuMode == MENU_2PLAYER_GAME || MenuMode == MENU_BATTLE_MODE)
        {
            if(ScrollDelay > 0)
            {
                MenuMouseMove = true;
                ScrollDelay = ScrollDelay - 1;
            }

            if(MenuMouseMove)
            {
                B = 0;

                For(A, minShow - 1, maxShow - 1)
                {
                    if(MenuMouseY >= 350 + B * 30 && MenuMouseY <= 366 + B * 30)
                    {
                        menuLen = 19 * static_cast<int>(SelectWorld[A + 1].WorldName.size());

                        if(MenuMouseX >= 300 && MenuMouseX <= 300 + menuLen)
                        {
                            if(MenuMouseRelease && MenuMouseDown)
                                MenuMouseClick = true;

                            if(MenuCursor != A && ScrollDelay == 0)
                            {
                                ScrollDelay = 10;
                                PlaySoundMenu(SFX_Slide);
                                MenuCursor = A;
                            }
                        }
                    }

                    B += 1;
                }
            }

            if(MenuCursorCanMove || MenuMouseClick || MenuMouseBack)
            {
                if(menuBackPress || MenuMouseBack)
                {
                    MenuCursor = MenuMode - 1;

                    if(MenuMode == MENU_BATTLE_MODE)
                    {
                        int battleIndex;
                        if(g_config.LegacyPlayerSelect)
                            battleIndex = 2;
                        else
                            battleIndex = 1;
                        MenuCursor = battleIndex;
                    }

                    MenuMode = MENU_MAIN;
//'world select back

                    PlaySoundMenu(SFX_Slide);
                    MenuCursorCanMove = false;
                }
                else if(menuDoPress || MenuMouseClick)
                {
                    PlaySoundMenu(SFX_Do);
                    selWorld = MenuCursor + 1;
                    FindSaves();

                    For(A, 1, numCharacters)
                    {
                        if(MenuMode == MENU_BATTLE_MODE)
                            blockCharacter[A] = false;
                        else
                            blockCharacter[A] = SelectWorld[selWorld].blockChar[A];
                    }

                    if(g_config.LegacyPlayerSelect)
                    {
                        MenuMode *= MENU_CHARACTER_SELECT_BASE;
                        MenuCursor = 0;

                        if(MenuMode == MENU_CHARACTER_SELECT_BM_S1 && PlayerCharacter != 0)
                            MenuCursor = PlayerCharacter - 1;
                    }
                    else
                    {
                        if(MenuMode == MENU_BATTLE_MODE)
                        {
                            MenuMode = MENU_CHARACTER_SELECT_NEW_BM;
                            menuPlayerSelect_Start();
                        }
                        else
                        {
                            MenuMode *= MENU_SELECT_SLOT_BASE;
                            MenuCursor = 0;
                        }
                    }

                    MenuCursorCanMove = false;
                }

            }

            if(MenuMode == MENU_1PLAYER_GAME || MenuMode == MENU_2PLAYER_GAME || MenuMode == MENU_BATTLE_MODE)
            {
                if(MenuCursor >= NumSelectWorld)
                    MenuCursor = 0;
                if(MenuCursor < 0)
                    MenuCursor = NumSelectWorld - 1;
            }
        } // World select

        // Save Select
        else if(MenuMode == MENU_SELECT_SLOT_1P || MenuMode == MENU_SELECT_SLOT_2P)
        {
            if(MenuMouseMove)
                s_handleMouseMove(4, 300, 350, 300, 30);

            if(MenuCursorCanMove || MenuMouseClick || MenuMouseBack)
            {
                if(menuBackPress || MenuMouseBack)
                {
//'save select back
                    if(g_config.LegacyPlayerSelect)
                    {
                        if(AllCharBlock > 0)
                        {
                            MenuMode /= MENU_SELECT_SLOT_BASE;
                            MenuCursor = selWorld - 1;
                        }
                        else
                        {
                            if(MenuMode == MENU_SELECT_SLOT_1P)
                            {
                                MenuCursor = PlayerCharacter - 1;
                                MenuMode = MENU_CHARACTER_SELECT_1P;
                            }
                            else
                            {
                                MenuCursor = PlayerCharacter2 - 1;
                                MenuMode = MENU_CHARACTER_SELECT_2P_S2;
                            }
                        }
                    }
                    else
                    {
                        MenuMode /= MENU_SELECT_SLOT_BASE;
                        MenuCursor = selWorld - 1;
                    }

                    MenuCursorCanMove = false;
                    PlaySoundMenu(SFX_Slide);
                }
                else if(menuDoPress || MenuMouseClick)
                {
                    PlaySoundMenu(SFX_Do);

                    if(MenuCursor >= 0 && MenuCursor <= 2 && g_config.LegacyPlayerSelect) // Select the save slot
                    {
                        selSave = MenuCursor + 1;
                        numPlayers = MenuMode / MENU_SELECT_SLOT_BASE;

                        StartEpisode();

                        return true;
                    }
                    else if(MenuCursor >= 0 && MenuCursor <= 2) // Select the save slot, but still need to select players
                    {
                        selSave = MenuCursor + 1;
                        MenuMode = MENU_CHARACTER_SELECT_NEW;
                        menuPlayerSelect_Start();
                        MenuCursorCanMove = false;
                    }
                    else if(MenuCursor == 3) // Copy the gamesave
                    {
                        MenuCursor = 0;
                        MenuMode += MENU_SELECT_SLOT_COPY_S1_ADD;
                        MenuCursorCanMove = false;
                    }
                    else if(MenuCursor == 4) // Delete the gamesave
                    {
                        MenuCursor = 0;
                        MenuMode += MENU_SELECT_SLOT_DELETE_ADD;
                        MenuCursorCanMove = false;
                    }
                }
            }

            if(MenuMode == MENU_SELECT_SLOT_1P || MenuMode == MENU_SELECT_SLOT_2P)
            {
                if(MenuCursor > 4) MenuCursor = 0;
                if(MenuCursor < 0) MenuCursor = 4;
            }
        } // Save Slot Select

        // Save Select
        else if(MenuMode == MENU_SELECT_SLOT_1P_COPY_S1 || MenuMode == MENU_SELECT_SLOT_2P_COPY_S1 ||
                MenuMode == MENU_SELECT_SLOT_1P_COPY_S2 || MenuMode == MENU_SELECT_SLOT_2P_COPY_S2)
        {
            if(MenuMouseMove)
                s_handleMouseMove(2, 300, 350, 300, 30);

            if(MenuCursorCanMove || MenuMouseClick || MenuMouseBack)
            {
                if(menuBackPress || MenuMouseBack)
                {
//'save select back
                    if(MenuMode == MENU_SELECT_SLOT_1P_COPY_S2 || MenuMode == MENU_SELECT_SLOT_2P_COPY_S2)
                    {
                        MenuMode -= MENU_SELECT_SLOT_COPY_S1_ADD;
                    }
                    else
                    {
                        MenuMode -= MENU_SELECT_SLOT_COPY_S1_ADD;
                        MenuCursor = 3;
                    }

                    MenuCursorCanMove = false;
                    PlaySoundMenu(SFX_Do);
                }
                else if(menuDoPress || MenuMouseClick)
                {
                    SDL_assert_release(IF_INRANGE(MenuCursor, 0, maxSaveSlots - 1));
                    int slot = MenuCursor + 1;

                    if(MenuMode == MENU_SELECT_SLOT_1P_COPY_S1 || MenuMode == MENU_SELECT_SLOT_2P_COPY_S1)
                    {
                        if(SaveSlot[slot] < 0)
                            PlaySoundMenu(SFX_BlockHit);
                        else
                        {
                            PlaySoundMenu(SFX_Do);
                            menuCopySaveSrc = slot;
                            MenuMode += MENU_SELECT_SLOT_COPY_S1_ADD;
                        }
                        MenuCursorCanMove = false;
                    }
                    else if(menuCopySaveSrc == slot)
                    {
                        PlaySoundMenu(SFX_BlockHit);
                        MenuCursorCanMove = false;
                    }
                    else
                    {
                        PlaySoundMenu(SFX_Raccoon);
                        menuCopySaveDst = slot;
                        CopySave(selWorld, menuCopySaveSrc, menuCopySaveDst);
                        FindSaves();
                        MenuMode -= MENU_SELECT_SLOT_COPY_S2_ADD;
                        MenuCursor = 3;
                        MenuCursorCanMove = false;
                    }
                }
            }

            if(MenuMode == MENU_SELECT_SLOT_1P_COPY_S1 || MenuMode == MENU_SELECT_SLOT_2P_COPY_S1 ||
               MenuMode == MENU_SELECT_SLOT_1P_COPY_S2 || MenuMode == MENU_SELECT_SLOT_2P_COPY_S2)
            {
                if(MenuCursor > 2) MenuCursor = 0;
                if(MenuCursor < 0) MenuCursor = 2;
            }
        }

        // Delete gamesave
        else if(MenuMode == MENU_SELECT_SLOT_1P_DELETE || MenuMode == MENU_SELECT_SLOT_1P_DELETE)
        {
            if(MenuMouseMove)
                s_handleMouseMove(2, 300, 350, 300, 30);

            if(MenuCursorCanMove || MenuMouseClick || MenuMouseBack)
            {
                if(menuBackPress || MenuMouseBack)
                {
//'save select back
                    MenuMode -= MENU_SELECT_SLOT_DELETE_ADD;
                    MenuCursor = 4;
                    PlaySoundMenu(SFX_Do);
                    MenuCursorCanMove = false;
                }
                else if(menuDoPress || MenuMouseClick)
                {
                    MenuMode -= MENU_SELECT_SLOT_DELETE_ADD;
                    DeleteSave(selWorld, (MenuCursor + 1));
                    FindSaves();
                    MenuCursor = 4;
                    PlaySoundMenu(SFX_Blaarg);
                    MenuCursorCanMove = false;
                }
            }

            if(MenuMode == MENU_SELECT_SLOT_1P_DELETE || MenuMode == MENU_SELECT_SLOT_1P_DELETE)
            {
                if(MenuCursor > 2) MenuCursor = 0;
                if(MenuCursor < 0) MenuCursor = 2;
            }
        }

        // Options
        else if(MenuMode == MENU_OPTIONS)
        {
#ifndef __ANDROID__
            const int optionsMenuLength = 2;
#else
            const int optionsMenuLength = 1;
#endif

            if(MenuMouseMove)
            {
                For(A, 0, optionsMenuLength)
                {
                    if(MenuMouseY >= 350 + A * 30 && MenuMouseY <= 366 + A * 30)
                    {
                        int i = 0;
                        if(A == i++)
                            menuLen = 18 * g_mainMenu.controlsTitle.size();
#ifndef __ANDROID__
                        else if(A == i++)
                        {
                            if(resChanged)
                                menuLen = 18 * std::strlen("windowed mode");
                            else
                                menuLen = 18 * std::strlen("fullscreen mode");
                        }
#endif
                        else
                            menuLen = 18 * std::strlen("view credits") - 2;

                        if(MenuMouseX >= 300 && MenuMouseX <= 300 + menuLen)
                        {
                            if(MenuMouseRelease && MenuMouseDown)
                                MenuMouseClick = true;

                            if(MenuCursor != A)
                            {
                                PlaySoundMenu(SFX_Slide);
                                MenuCursor = A;
                            }
                        }
                    }
                }
            }

            if(MenuCursorCanMove || MenuMouseClick || MenuMouseBack)
            {
                if(menuBackPress || MenuMouseBack)
                {
                    int optionsIndex;
                    if(g_config.LegacyPlayerSelect)
                        optionsIndex = 3;
                    else
                        optionsIndex = 2;
                    MenuMode = MENU_MAIN;
                    MenuCursor = optionsIndex;
                    MenuCursorCanMove = false;
                    PlaySoundMenu(SFX_Slide);
                }
                else if(menuDoPress || MenuMouseClick)
                {
                    MenuCursorCanMove = false;
                    int i = 0;
                    if(MenuCursor == i++)
                    {
                        MenuCursor = 0;
                        MenuMode = MENU_INPUT_SETTINGS;
                        PlaySoundMenu(SFX_Slide);
                    }
#ifndef __ANDROID__ // on Android run the always full-screen
                    else if(MenuCursor == i++)
                    {
                        PlaySoundMenu(SFX_Do);
                        ChangeScreen();
                    }
#endif
                    else if(MenuCursor == i++)
                    {
                        PlaySoundMenu(SFX_Do);
                        GameMenu = false;
                        GameOutro = true;
                        CreditChop = 300;
                        EndCredits = 0;
                        SetupCredits();
                    }

                }

            }

            if(MenuMode == MENU_OPTIONS)
            {
                if(MenuCursor > optionsMenuLength)
                    MenuCursor = 0;
                if(MenuCursor < 0)
                    MenuCursor = optionsMenuLength;
            }
        }

        // Input Settings
        else if(MenuMode == MENU_INPUT_SETTINGS)
        {
            int ret = menuControls_Logic();
            if(ret == -1)
            {
                MenuCursor = 0;
                MenuMode = MENU_OPTIONS;
                MenuCursorCanMove = false;
            }
        }
    }

//'check for all characters blocked
    if(MenuMode == MENU_CHARACTER_SELECT_1P || MenuMode == MENU_CHARACTER_SELECT_2P_S1 || MenuMode == MENU_CHARACTER_SELECT_2P_S2)
    {
        AllCharBlock = 0;

        For(A, 1, numCharacters)
        {
            if(!blockCharacter[A])
            {
                if(AllCharBlock == 0)
                {
                    AllCharBlock = A;
                }
                else
                {
                    AllCharBlock = 0;
                    break;
                }
            }
        }

        if(AllCharBlock > 0)
        {
            PlayerCharacter = AllCharBlock;
            PlayerCharacter2 = AllCharBlock;

            if(MenuMode == MENU_CHARACTER_SELECT_1P)
            {
                MenuMode = MENU_SELECT_SLOT_1P;
                MenuCursor = 0;
            }
            else if(MenuMode == MENU_CHARACTER_SELECT_2P_S1)
            {
                MenuMode = MENU_CHARACTER_SELECT_2P_S2;
                MenuCursor = PlayerCharacter2;
            }
            else
            {
                MenuMode = MENU_SELECT_SLOT_2P;
                MenuCursor = 0;
            }
        }
    }

    return false;
}

static void s_drawGameTypeTitle(int x, int y)
{
    if(menuBattleMode)
        SuperPrint(g_mainMenu.mainBattleGame, 3, x, y, 0.3f, 0.3f, 1.0f);
    else if(!g_config.LegacyPlayerSelect)
    {
        SuperPrint(g_mainMenu.mainGame, 3, x, y);
    }
    else
    {
        float r = menuPlayersNum == 1 ? 1.f : 0.3f;
        float g = menuPlayersNum == 2 ? 1.f : 0.3f;
        if(menuPlayersNum == 1)
            SuperPrint(g_mainMenu.main1PlayerGame, 3, x, y, r, g, 0.3f);
        else
            SuperPrint(g_mainMenu.main2PlayerGame, 3, x, y, r, g, 0.3f);
    }
}

static void s_drawGameSaves()
{
    int A;

    for(A = 1; A <= maxSaveSlots; A++)
    {
        if(SaveSlot[A] >= 0)
        {
            SuperPrint(fmt::format_ne("SLOT {0} ... {1}%", A, SaveSlot[A]), 3, 300, 320 + (A * 30));
            if(SaveStars[A] > 0)
            {
                frmMain.renderTexture(560, 320 + (A * 30) + 1,
                                      GFX.Interface[5].w, GFX.Interface[5].h,
                                      GFX.Interface[5], 0, 0);
                frmMain.renderTexture(560 + 24, 320 + (A * 30) + 2,
                                      GFX.Interface[1].w, GFX.Interface[1].h,
                                      GFX.Interface[1], 0, 0);
                SuperPrint(fmt::format_ne(" {0}", SaveStars[A]), 3, 588, 320 + (A * 30));
            }
        }
        else
        {
            SuperPrint(fmt::format_ne("SLOT {0} ... NEW GAME", A), 3, 300, 320 + (A * 30));
        }
    }

    if(MenuMode == MENU_SELECT_SLOT_1P || MenuMode == MENU_SELECT_SLOT_2P)
    {
        SuperPrint("COPY SAVE", 3, 300, 320 + (A * 30));
        A++;
        SuperPrint("ERASE SAVE", 3, 300, 320 + (A * 30));
    }
}

void mainMenuDraw()
{
    int A = 0;
    int B = 0;
    int C = 0;

    if(MenuMode != MENU_1PLAYER_GAME && MenuMode != MENU_2PLAYER_GAME && MenuMode != MENU_BATTLE_MODE)
        worldCurs = 0;

    int menuFix = -44; // for Input Settings

    frmMain.renderTexture(0, 0, GFX.MenuGFX[1].w, GFX.MenuGFX[1].h, GFX.MenuGFX[1], 0, 0);
    frmMain.renderTexture(ScreenW / 2 - GFX.MenuGFX[2].w / 2, 70,
            GFX.MenuGFX[2].w, GFX.MenuGFX[2].h, GFX.MenuGFX[2], 0, 0);

    frmMain.renderTexture(ScreenW / 2 - GFX.MenuGFX[3].w / 2, 576,
            GFX.MenuGFX[3].w, GFX.MenuGFX[3].h, GFX.MenuGFX[3], 0, 0);

    if(SDL_AtomicGet(&loading))
    {
        if(SDL_AtomicGet(&loadingProgrssMax) <= 0)
            SuperPrint(g_mainMenu.loading, 3, 300, 350);
        else
        {
            int progress = (SDL_AtomicGet(&loadingProgrss) * 100) / SDL_AtomicGet(&loadingProgrssMax);
            SuperPrint(fmt::format_ne("{0} {1}%", g_mainMenu.loading, progress), 3, 300, 350);
        }
    }

    // Main menu
    else if(MenuMode == MENU_MAIN)
    {
        int i = 0;
        if(g_config.LegacyPlayerSelect)
            SuperPrint(g_mainMenu.main1PlayerGame, 3, 300, 350+30*(i++));
        else
            SuperPrint(g_mainMenu.mainGame, 3, 300, 350+30*(i++));
        if(g_config.LegacyPlayerSelect)
            SuperPrint(g_mainMenu.main2PlayerGame, 3, 300, 350+30*(i++));
        SuperPrint(g_mainMenu.mainBattleGame, 3, 300, 350+30*(i++));
        SuperPrint(g_mainMenu.mainOptions, 3, 300, 350+30*(i++));
        SuperPrint(g_mainMenu.mainExit, 3, 300, 350+30*(i++));
        frmMain.renderTexture(300 - 20, 350 + (MenuCursor * 30), 16, 16, GFX.MCursor[0], 0, 0);
    }

    // Character select
    else if(MenuMode == MENU_CHARACTER_SELECT_NEW ||
            MenuMode == MENU_CHARACTER_SELECT_NEW_BM)
    {
        menuPlayerSelect_Render(menuPlayersNum);
    }
    else if(MenuMode == MENU_CHARACTER_SELECT_1P ||
            MenuMode == MENU_CHARACTER_SELECT_2P_S1 ||
            MenuMode == MENU_CHARACTER_SELECT_2P_S2 ||
            MenuMode == MENU_CHARACTER_SELECT_BM_S1 ||
            MenuMode == MENU_CHARACTER_SELECT_BM_S2)
    {
        A = 0;
        B = 0;
        C = 0;

        s_drawGameTypeTitle(300, 280);
        SuperPrint(SelectWorld[selWorld].WorldName, 3, 300, 310, 0.6f, 1.f, 1.f);

        // TODO: Make a custom playable character names print here
        if(!blockCharacter[1])
            SuperPrint(g_mainMenu.selectPlayer[1], 3, 300, 350);
        else
        {
            A -= 30;
            if(MenuCursor + 1 >= 1)
                B -= 30;
            if(PlayerCharacter >= 1)
                C -= 30;
        }

        if(!blockCharacter[2])
            SuperPrint(g_mainMenu.selectPlayer[2], 3, 300, 380 + A);
        else
        {
            A = A - 30;
            if(MenuCursor + 1 >= 2)
                B = B - 30;
            if(PlayerCharacter >= 2)
                C = C - 30;
        }

        if(!blockCharacter[3])
            SuperPrint(g_mainMenu.selectPlayer[3], 3, 300, 410 + A);
        else
        {
            A -= 30;
            if(MenuCursor + 1 >= 3)
                B -= 30;
            if(PlayerCharacter >= 3)
                C -= 30;
        }

        if(!blockCharacter[4])
            SuperPrint(g_mainMenu.selectPlayer[4], 3, 300, 440 + A);
        else
        {
            A -= 30;
            if(MenuCursor + 1 >= 4)
                B -= 30;
            if(PlayerCharacter >= 4)
                C -= 30;
        }

        if(!blockCharacter[5])
            SuperPrint(g_mainMenu.selectPlayer[5], 3, 300, 470 + A);
        else
        {
            A -= 30;
            if(MenuCursor + 1 >= 5)
                B -= 30;
            if(PlayerCharacter >= 5)
                C -= 30;
        }

        if(MenuMode == MENU_CHARACTER_SELECT_2P_S2 || MenuMode == MENU_CHARACTER_SELECT_BM_S2)
        {
            frmMain.renderTexture(300 - 20, B + 350 + (MenuCursor * 30), GFX.MCursor[3]);
            frmMain.renderTexture(300 - 20, B + 350 + ((PlayerCharacter - 1) * 30), GFX.MCursor[0]);
        }
        else
        {
            frmMain.renderTexture(300 - 20, B + 350 + (MenuCursor * 30), GFX.MCursor[0]);
        }
    }

    // Episode / Level selection
    else if(MenuMode == MENU_1PLAYER_GAME || MenuMode == MENU_2PLAYER_GAME || MenuMode == MENU_BATTLE_MODE)
    {
        s_drawGameTypeTitle(300, 280);
        std::string tempStr;

        minShow = 1;
        maxShow = NumSelectWorld;

        if(NumSelectWorld > 5)
        {
            minShow = worldCurs;
            maxShow = minShow + 4;

            if(MenuCursor <= minShow - 1)
                worldCurs -= 1;

            if(MenuCursor >= maxShow - 1)
                worldCurs += 1;

            if(worldCurs < 1)
                worldCurs = 1;

            if(worldCurs > NumSelectWorld - 4)
                worldCurs = NumSelectWorld - 4;

            if(maxShow >= NumSelectWorld)
            {
                maxShow = NumSelectWorld;
                minShow = NumSelectWorld - 4;
            }

            minShow = worldCurs;
            maxShow = minShow + 4;
        }

        for(auto A = minShow; A <= maxShow; A++)
        {
            B = A - minShow + 1;
            tempStr = SelectWorld[A].WorldName;
            SuperPrint(tempStr, 3, 300, 320 + (B * 30));
        }

        if(minShow > 1)
            frmMain.renderTexture(400 - 8, 350 - 20, GFX.MCursor[1]);


        if(maxShow < NumSelectWorld)
            frmMain.renderTexture(400 - 8, 490, GFX.MCursor[2]);

        B = MenuCursor - minShow + 1;

        if(B >= 0 && B < 5)
            frmMain.renderTexture(300 - 20, 350 + (B * 30), GFX.MCursor[0].w, GFX.MCursor[0].h, GFX.MCursor[0], 0, 0);
    }

    else if(MenuMode == MENU_SELECT_SLOT_1P || MenuMode == MENU_SELECT_SLOT_2P) // Save Select
    {
        s_drawGameTypeTitle(300, 280);
        SuperPrint(SelectWorld[selWorld].WorldName, 3, 300, 310, 0.6f, 1.f, 1.f);
        s_drawGameSaves();
        frmMain.renderTexture(300 - 20, 350 + (MenuCursor * 30), GFX.MCursor[0]);
    }

    else if(MenuMode == MENU_SELECT_SLOT_1P_COPY_S1 || MenuMode == MENU_SELECT_SLOT_2P_COPY_S1 ||
            MenuMode == MENU_SELECT_SLOT_1P_COPY_S2 || MenuMode == MENU_SELECT_SLOT_2P_COPY_S2) // Copy save
    {
        s_drawGameTypeTitle(300, 280);
        SuperPrint(SelectWorld[selWorld].WorldName, 3, 300, 310, 0.6f, 1.f, 1.f);
        s_drawGameSaves();

        if(MenuMode == MENU_SELECT_SLOT_1P_COPY_S1 || MenuMode == MENU_SELECT_SLOT_2P_COPY_S1)
            SuperPrint("Select the source slot", 3, 300, 320 + (5 * 30), 0.7f, 0.7f, 1.0f);
        else if(MenuMode == MENU_SELECT_SLOT_1P_COPY_S2 || MenuMode == MENU_SELECT_SLOT_2P_COPY_S2)
            SuperPrint("Now select the target", 3, 300, 320 + (5 * 30), 0.7f, 1.0f, 0.7f);

        if(MenuMode == MENU_SELECT_SLOT_1P_COPY_S2 || MenuMode == MENU_SELECT_SLOT_2P_COPY_S2)
        {
            frmMain.renderTexture(300 - 20, 350 + ((menuCopySaveSrc - 1) * 30), GFX.MCursor[0]);
            frmMain.renderTexture(300 - 20, 350 + (MenuCursor * 30), GFX.MCursor[3]);
        }
        else
            frmMain.renderTexture(300 - 20, 350 + (MenuCursor * 30), GFX.MCursor[0]);
    }

    else if(MenuMode == MENU_SELECT_SLOT_1P_DELETE || MenuMode == MENU_SELECT_SLOT_2P_DELETE) // Copy save
    {
        s_drawGameTypeTitle(300, 280);
        SuperPrint(SelectWorld[selWorld].WorldName, 3, 300, 310, 0.6f, 1.f, 1.f);
        s_drawGameSaves();

        SuperPrint("Select the slot to erase", 3, 300, 320 + (5 * 30), 1.0f, 0.7f, 0.7f);

        frmMain.renderTexture(300 - 20, 350 + (MenuCursor * 30), GFX.MCursor[0]);
    }

    // Options Menu
    else if(MenuMode == MENU_OPTIONS)
    {
        int i = 0;
        SuperPrint(g_mainMenu.controlsTitle, 3, 300, 350 + 30*i++);
#ifndef __ANDROID__
        if(resChanged)
            SuperPrint("WINDOWED MODE", 3, 300, 350 + 30*i++);
        else
            SuperPrint("FULLSCREEN MODE", 3, 300, 350 + 30*i++);
#endif
        SuperPrint("VIEW CREDITS", 3, 300, 350 + 30*i++);
        frmMain.renderTexture(300 - 20, 350 + (MenuCursor * 30),
                              GFX.MCursor[0].w, GFX.MCursor[0].h, GFX.MCursor[0], 0, 0);
    }

    // Player controls setup
    else if(MenuMode == MENU_INPUT_SETTINGS)
    {
        menuControls_Render();
    }

    // Mouse cursor
    frmMain.renderTexture(int(MenuMouseX), int(MenuMouseY), GFX.ECursor[2]);
}
