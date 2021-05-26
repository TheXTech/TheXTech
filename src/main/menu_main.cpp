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

#ifndef NO_SDL
#include <SDL2/SDL_timer.h>
#else
#include "SDL_supplement.h"
#endif

#ifndef PGE_NO_THREADING
#include <SDL2/SDL_thread.h>
#include <atomic>
#endif

#ifdef __3DS__
#include "../3ds/second_screen.h"
#include "../3ds/n3ds-clock.h"
#endif

#include <fmt_format_ne.h>

#include <AppPath/app_path.h>
#include <DirManager/dirman.h>
#include <Utils/files.h>
#include <PGE_File_Formats/file_formats.h>

#include "menu_main.h"
#include "game_info.h"

#include "speedrunner.h"
#include "../game_main.h"
#include "../sound.h"
#include "../player.h"
#include "../collision.h"
#include "../graphics.h"
#include "../control/joystick.h"
#include "level_file.h"
#include "pge_delay.h"
#include "../config.h"

#ifdef NEW_EDITOR
#include "../editor/new_editor.h"
const int MainMenuQuitIndex = 5;
const int MainMenuOptionsIndex = 4;
#else
const int MainMenuQuitIndex = 4;
const int MainMenuOptionsIndex = 3;
#endif

MainMenuContent g_mainMenu;

#ifndef PGE_NO_THREADING
static SDL_atomic_t         loading;
static SDL_atomic_t         loadingProgrss;
static SDL_atomic_t         loadingProgrssMax;

static SDL_Thread*          loadingThread = nullptr;
#endif

void initMainMenu()
{
#ifndef PGE_NO_THREADING
    SDL_AtomicSet(&loading, 0);
    SDL_AtomicSet(&loadingProgrss, 0);
    SDL_AtomicSet(&loadingProgrssMax, 0);
#endif

    g_mainMenu.introPressStart = "Press Start";

    g_mainMenu.main1PlayerGame = "1 Player Game";
    g_mainMenu.main2PlayerGame = "2 Player Game";
    g_mainMenu.mainBattleGame = "Battle Game";
    g_mainMenu.mainEditor = "Editor";
    g_mainMenu.mainOptions = "Options";
    g_mainMenu.mainExit = "Exit";

    g_mainMenu.loading = "Loading...";

    for(int i = 1; i <= numCharacters; ++i)
        g_mainMenu.selectPlayer[i] = fmt::format_ne("{0} game", g_gameInfo.characterName[i]);
}


static int ScrollDelay = 0;
static int menuPlayersNum = 0;
static int menuBattleMode = false;

static int menuCopySaveSrc = 0;
static int menuCopySaveDst = 0;

const int menuFix = -44; // for Input Settings

const int TinyScreenH = 400;
const int SmallScreenH = 500;
const int TinyScreenW = 600;

void GetMenuPos(int& MenuX, int& MenuY)
{
    MenuX = ScreenW / 2 - 100;
    MenuY = ScreenH - 250;
    if(ScreenW < TinyScreenW)
    {
        MenuX = ScreenW / 2 - 240;
        if(MenuX < 24)
            MenuX = 24;
    }
    if(ScreenH < TinyScreenH)
        MenuY = 100;
    else if(ScreenH < SmallScreenH)
        MenuY = ScreenH - 180;
    if(MenuMode == MENU_INPUT_SETTINGS_P1 || MenuMode == MENU_INPUT_SETTINGS_P2)
    {
        if(ScreenH < TinyScreenH)
            MenuY += 16;
        else if(ScreenH < SmallScreenH)
            MenuY -= 70 - menuFix;
        else
            MenuY += menuFix;
    }
}

static int FindWorldsThread(void *)
{
    FindWorlds();
    return 0;
}

void FindWorlds()
{
    NumSelectWorld = 0;
    NumSelectWorldEditable = 0;
    bool stillEditable = true;

#ifndef __3DS__
    std::vector<std::string> worldRoots;
    worldRoots.push_back(AppPath + "worlds/");
    const bool canOnlyEditFirstRoot = false;
#else
    const std::vector<std::string>& worldRoots = AppPathManager::worldRootDirs();
    const bool canOnlyEditFirstRoot = true;
#endif

#if !defined(__3DS__) && ((defined(__APPLE__) && defined(USE_BUNDLED_ASSETS)) || defined(FIXED_ASSETS_PATH))
    worldRoots.push_back(AppPathManager::userWorldsRootDir() + "/");
#endif

    SelectWorld.clear();
    SelectWorld.push_back(SelectWorld_t()); // Dummy entry

#ifndef PGE_NO_THREADING
    SDL_AtomicSet(&loadingProgrss, 0);
    SDL_AtomicSet(&loadingProgrssMax, 0);
#endif

    for(const auto &worldsRoot : worldRoots)
    {
        std::vector<std::string> dirs;
        DirMan episodes(worldsRoot);
        episodes.getListOfFolders(dirs);
#ifndef PGE_NO_THREADING
        SDL_AtomicAdd(&loadingProgrssMax, dirs.size());
#endif
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
                    if(stillEditable)
                        SelectWorldEditable.push_back(w);
                }
            }

        // On 3DS, this is after the first and only non-romfs root has been processed.
        if (canOnlyEditFirstRoot)
            stillEditable = false;

#ifndef PGE_NO_THREADING
            SDL_AtomicAdd(&loadingProgrss, 1);
#endif
        }
    }

    std::sort(SelectWorld.begin(), SelectWorld.end(), 
        [](const SelectWorld_t& a, const SelectWorld_t& b) {
            return a.WorldName < b.WorldName;
        });
    std::sort(SelectWorldEditable.begin(), SelectWorldEditable.end(), 
        [](const SelectWorld_t& a, const SelectWorld_t& b) {
            return a.WorldName < b.WorldName;
        });

    NumSelectWorld = (SelectWorld.size() - 1);

    SelectWorld_t createWorld = SelectWorld_t();
    createWorld.WorldName = "Create New";
    SelectWorldEditable.push_back(createWorld);
    NumSelectWorldEditable = (SelectWorldEditable.size() - 1);

    LevelData head;

#ifndef PGE_NO_THREADING
    SDL_AtomicSet(&loading, 0);
#endif
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

    SelectBattle.clear();
    SelectBattle.push_back(SelectWorld_t()); // Dummy entry

    NumSelectBattle = 1;
    SelectBattle.push_back(SelectWorld_t()); // "random level" entry
    SelectBattle[1].WorldName = "Random Level";
    LevelData head;

#ifndef PGE_NO_THREADING
    SDL_AtomicSet(&loadingProgrss, 0);
    SDL_AtomicSet(&loadingProgrssMax, 0);
#endif

    for(const auto &battleRoot : battleRoots)
    {
        std::vector<std::string> files;
        DirMan battleLvls(battleRoot);
        battleLvls.getListOfFiles(files, {".lvl", ".lvlx"});
#ifndef PGE_NO_THREADING
        SDL_AtomicAdd(&loadingProgrssMax, files.size());
#endif
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
#ifndef PGE_NO_THREADING
            SDL_AtomicAdd(&loadingProgrss, 1);
#endif
        }
    }

    NumSelectBattle = (SelectBattle.size() - 1);
#ifndef PGE_NO_THREADING
    SDL_AtomicSet(&loading, 0);
#endif
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
    int MenuX, MenuY;
    GetMenuPos(MenuX, MenuY);

    int B;
    Location_t tempLocation;
    int menuLen;
    Player_t blankPlayer;

#ifndef NO_SDL
    bool altPressed = getKeyState(SDL_SCANCODE_LALT) == KEY_PRESSED ||
                      getKeyState(SDL_SCANCODE_RALT) == KEY_PRESSED;
    bool escPressed = getKeyState(SDL_SCANCODE_ESCAPE) == KEY_PRESSED;
    bool spacePressed = getKeyState(SDL_SCANCODE_SPACE) == KEY_PRESSED;
    bool returnPressed = getKeyState(SDL_SCANCODE_RETURN) == KEY_PRESSED;
    bool upPressed = getKeyState(SDL_SCANCODE_UP) == KEY_PRESSED;
    bool downPressed = getKeyState(SDL_SCANCODE_DOWN) == KEY_PRESSED;
    bool leftPressed = getKeyState(SDL_SCANCODE_LEFT) == KEY_PRESSED;
    bool rightPressed = getKeyState(SDL_SCANCODE_RIGHT) == KEY_PRESSED;
#else // #ifndef NO_SDL
    bool altPressed = false;
    bool returnPressed = false;
    bool spacePressed = false;
    bool escPressed = false;
    bool upPressed = false;
    bool downPressed = false;
    bool leftPressed = false;
    bool rightPressed = false;
#endif // #ifndef NO_SDL ... #else

#ifdef __ANDROID__
    escPressed |= getKeyState(SDL_SCANCODE_AC_BACK) == KEY_PRESSED;
#endif // #ifdef __ANDROID__

    bool menuDoPress = (returnPressed && !altPressed) || spacePressed;
    bool menuBackPress = (escPressed && !altPressed);
    bool menuLeftPress = leftPressed;
    bool menuRightPress = rightPressed;

    {
        Controls_t &c = Player[1].Controls;

        menuDoPress |= (c.Start || c.Jump) && !altPressed;
        menuBackPress |= c.Run && !altPressed;
        menuLeftPress |= c.Left;
        menuRightPress |= c.Right;

        if(frmMain.MousePointer != 99)
        {
            frmMain.MousePointer = 99;
            showCursor(0);
        }

        if(!c.Up && !c.Down && !c.Jump && !c.Run && !c.Start && !c.Left && !c.Right)
        {
            bool k = false;
            k |= menuDoPress;
            k |= upPressed;
            k |= downPressed;
            k |= leftPressed;
            k |= rightPressed;
            k |= escPressed;

            if(!k)
                MenuCursorCanMove = true;

        }

        if(!getNewKeyboard && !getNewJoystick)
        {
            int cursorDelta = 0;

            if(c.Up || upPressed)
            {
                if(MenuCursorCanMove)
                {
                    MenuCursor -= 1;
                    cursorDelta = -1;
                }

                MenuCursorCanMove = false;
            }
            else if(c.Down || downPressed)
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

        if(MenuMode == MENU_INTRO && ScreenH >= TinyScreenH)
            MenuMode = MENU_MAIN;

#ifndef PGE_NO_THREADING
        if(SDL_AtomicGet(&loading))
        {
            if((menuDoPress && MenuCursorCanMove) || MenuMouseClick)
                PlaySoundMenu(SFX_BlockHit);
            if(MenuCursor != 0)
                MenuCursor = 0;
        }
        else
#endif
        // Menu Intro
        if(MenuMode == MENU_INTRO)
        {
            if(MenuMouseRelease && MenuMouseDown)
                MenuMouseClick = true;
            if(escPressed && MenuCursorCanMove)
            {
                MenuMode = MENU_MAIN;
                MenuCursor = MainMenuQuitIndex;
                PlaySoundMenu(SFX_Slide);
            }
            if((menuDoPress && MenuCursorCanMove) || MenuMouseClick)
            {
                MenuCursorCanMove = false;
                MenuMode = MENU_MAIN;
                PlaySoundMenu(SFX_Do);
            }
        }
        // Main Menu
        else if(MenuMode == MENU_MAIN)
        {
            if(MenuMouseMove)
            {
                For(A, 0, MainMenuQuitIndex)
                {
                    if(MenuMouseY >= MenuY + A * 30 && MenuMouseY <= MenuY + A * 30 + 16)
                    {
                        if(A == 0)
                            menuLen = 18 * g_mainMenu.main1PlayerGame.size() - 2;
                        else if(A == 1)
                            menuLen = 18 * g_mainMenu.main2PlayerGame.size() - 2;
                        else if(A == 2)
                            menuLen = 18 * g_mainMenu.mainBattleGame.size();
#ifdef NEW_EDITOR
                        else if(A == 3)
                            menuLen = 18 * g_mainMenu.mainEditor.size();
#endif
                        else if(A == MainMenuOptionsIndex)
                            menuLen = 18 * g_mainMenu.mainOptions.size();
                        else
                            menuLen = 18 * g_mainMenu.mainExit.size();

                        if(MenuMouseX >= MenuX && MenuMouseX <= MenuX + menuLen)
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

            if(escPressed && MenuCursorCanMove)
            {
                if(ScreenH < TinyScreenH)
                {
                    MenuCursorCanMove = false;
                    MenuMode = MENU_INTRO;
                    PlaySoundMenu(SFX_Slide);
                }
                else if(MenuCursor != MainMenuQuitIndex)
                {
                    MenuCursor = MainMenuQuitIndex;
                    PlaySoundMenu(SFX_Slide);
                }
            }
            else if(menuBackPress && MenuCursorCanMove && ScreenH < TinyScreenH)
            {
                MenuCursorCanMove = false;
                MenuMode = MENU_INTRO;
                PlaySoundMenu(SFX_Slide);
            }
            else if((menuDoPress && MenuCursorCanMove) || MenuMouseClick)
            {
                MenuCursorCanMove = false;
                PlayerCharacter = 0;
                PlayerCharacter2 = 0;

                if(MenuCursor == 0)
                {
                    PlaySoundMenu(SFX_Do);
                    MenuMode = MENU_1PLAYER_GAME;
                    menuPlayersNum = 1;
                    menuBattleMode = false;
#if !defined(PRELOAD_LEVELS) && defined(PGE_NO_THREADING)
                    FindWorlds();
#elif !defined(PRELOAD_LEVELS)
                    SDL_AtomicSet(&loading, 1);
                    loadingThread = SDL_CreateThread(FindWorldsThread, "FindWorlds", NULL);
#endif
                    MenuCursor = 0;
                }
                else if(MenuCursor == 1)
                {
                    PlaySoundMenu(SFX_Do);
                    MenuMode = MENU_2PLAYER_GAME;
                    menuPlayersNum = 2;
                    menuBattleMode = false;
#if !defined(PRELOAD_LEVELS) && defined(PGE_NO_THREADING)
                    FindWorlds();
#elif !defined(PRELOAD_LEVELS)
                    SDL_AtomicSet(&loading, 1);
                    loadingThread = SDL_CreateThread(FindWorldsThread, "FindWorlds", NULL);
#endif
                    MenuCursor = 0;
                }
                else if(MenuCursor == 2)
                {
                    PlaySoundMenu(SFX_Do);
                    MenuMode = MENU_BATTLE_MODE;
                    menuPlayersNum = 2;
                    menuBattleMode = true;
#if !defined(PRELOAD_LEVELS) && defined(PGE_NO_THREADING)
                    FindLevels();
#elif !defined(PRELOAD_LEVELS)
                    SDL_AtomicSet(&loading, 1);
                    loadingThread = SDL_CreateThread(FindLevelsThread, "FindLevels", NULL);
#endif
                    MenuCursor = 0;
                }
#ifdef NEW_EDITOR
                else if(MenuCursor == 3)
                {
                    PlaySoundMenu(SFX_Do);
                    MenuMode = MENU_EDITOR;
#if     !defined(PRELOAD_LEVELS) && defined(PGE_NO_THREADING)
                    FindWorlds();
#elif   !defined(PRELOAD_LEVELS)
                    SDL_AtomicSet(&loading, 1);
                    loadingThread = SDL_CreateThread(FindWorldsThread, "FindWorlds", NULL);
#endif // PRELOAD_LEVELS
                    MenuCursor = 0;
                }
#endif
                else if(MenuCursor == MainMenuOptionsIndex)
                {
                    PlaySoundMenu(SFX_Do);
                    MenuMode = MENU_OPTIONS;
                    MenuCursor = 0;
                }
                else
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

            if(MenuCursor > MainMenuQuitIndex)
                MenuCursor = 0;
            if(MenuCursor < 0)
                MenuCursor = MainMenuQuitIndex;
        } // Main Menu

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
                        if(MenuMouseY >= MenuY + A * 30 + B && MenuMouseY <= MenuY + A * 30 + B + 16)
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

                            if(MenuMouseX >= MenuX && MenuMouseX <= MenuX + menuLen)
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
        else if(MenuMode == MENU_1PLAYER_GAME || MenuMode == MENU_2PLAYER_GAME
            || MenuMode == MENU_BATTLE_MODE || MenuMode == MENU_EDITOR)
        {
            const std::vector<SelectWorld_t>& SelectorList
                = (MenuMode == MENU_BATTLE_MODE) ? SelectBattle :
                    ((MenuMode == MENU_EDITOR) ? SelectWorldEditable :
                        SelectWorld);

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
                    if(MenuMouseY >= MenuY + B * 30 && MenuMouseY <= MenuY + B * 30 + 16)
                    {
                        menuLen = 19 * static_cast<int>(SelectorList[A + 1].WorldName.size());

                        if(MenuMouseX >= MenuX && MenuMouseX <= MenuX + menuLen)
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
                        MenuCursor = 2;
                    else if(MenuMode == MENU_EDITOR)
                        MenuCursor = 3;

                    MenuMode = MENU_MAIN;
//'world select back

                    PlaySoundMenu(SFX_Slide);
                    MenuCursorCanMove = false;
                }
                else if(menuDoPress || MenuMouseClick)
                {
                    PlaySoundMenu(SFX_Do);
                    selWorld = MenuCursor + 1;
                    // level editor
                    if (MenuMode == MENU_EDITOR)
                    {
                        GameMenu = false;
                        LevelSelect = false;
                        BattleMode = false;
                        LevelEditor = true;
                        WorldEditor = true;
                        ClearLevel();
                        ClearGame();
                        if(selWorld == NumSelectWorldEditable)
                        {
                            ClearWorld();
                            // do something here...
                            printf("BAD TIMES\n");
                        }
                        OpenWorld(SelectWorldEditable[selWorld].WorldPath
                            + SelectWorldEditable[selWorld].WorldFile);
#ifdef NEW_EDITOR
                        editorScreen.ResetCursor();
#endif
                        return true;
                    }
                    else
                    {
                        FindSaves();

                        For(A, 1, numCharacters)
                        {
                            if(MenuMode == MENU_BATTLE_MODE)
                                blockCharacter[A] = false;
                            else
                                blockCharacter[A] = SelectWorld[selWorld].blockChar[A];
                        }

                        MenuMode *= MENU_CHARACTER_SELECT_BASE;
                        MenuCursor = 0;

                        if(MenuMode == MENU_CHARACTER_SELECT_BM_S1 && PlayerCharacter != 0)
                            MenuCursor = PlayerCharacter - 1;
                    }

                    MenuCursorCanMove = false;
                }

            }

            if(MenuMode != MENU_MAIN && MenuMode < MENU_CHARACTER_SELECT_BASE)
            {
                maxShow = (MenuMode == MENU_BATTLE_MODE) ? NumSelectBattle :
                    ((MenuMode == MENU_EDITOR) ? NumSelectWorldEditable :
                        NumSelectWorld);
                if(MenuCursor >= maxShow)
                    MenuCursor = 0;
                if(MenuCursor < 0)
                    MenuCursor = maxShow - 1;
            }
        } // World select

        // Save Select
        else if(MenuMode == MENU_SELECT_SLOT_1P || MenuMode == MENU_SELECT_SLOT_2P)
        {
            if(MenuMouseMove)
                s_handleMouseMove(4, MenuX, MenuY, 300, 30);

            if(MenuCursorCanMove || MenuMouseClick || MenuMouseBack)
            {
                if(menuBackPress || MenuMouseBack)
                {
//'save select back
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

                    MenuCursorCanMove = false;
                    PlaySoundMenu(SFX_Do);
                }
                else if(menuDoPress || MenuMouseClick)
                {
                    PlaySoundMenu(SFX_Do);

                    if(MenuCursor >= 0 && MenuCursor <= 2) // Select the save slot
                    {
                        numPlayers = MenuMode / MENU_SELECT_SLOT_BASE;

                        For(A, 1, numCharacters)
                        {
                            SavedChar[A] = blankPlayer;
                            SavedChar[A].Character = A;
                            SavedChar[A].State = 1;
                        }

                        Player[1].State = 1;
                        Player[1].Mount = 0;
                        Player[1].Character = 1;
                        Player[1].HeldBonus = 0;
                        Player[1].CanFly = false;
                        Player[1].CanFly2 = false;
                        Player[1].TailCount = 0;
                        Player[1].YoshiBlue = false;
                        Player[1].YoshiRed = false;
                        Player[1].YoshiYellow = false;
                        Player[1].Hearts = 0;
                        Player[2].State = 1;
                        Player[2].Mount = 0;
                        Player[2].Character = 2;
                        Player[2].HeldBonus = 0;
                        Player[2].CanFly = false;
                        Player[2].CanFly2 = false;
                        Player[2].TailCount = 0;
                        Player[2].YoshiBlue = false;
                        Player[2].YoshiRed = false;
                        Player[2].YoshiYellow = false;
                        Player[2].Hearts = 0;

                        if(numPlayers <= 2 && PlayerCharacter > 0)
                        {
                            Player[1].Character = PlayerCharacter;
                            PlayerCharacter = 0;
                        }

                        if(numPlayers == 2 && PlayerCharacter2 > 0)
                        {
                            Player[2].Character = PlayerCharacter2;
                            PlayerCharacter2 = 0;
                        }

                        selSave = MenuCursor + 1;
                        numStars = 0;
                        Coins = 0;
                        Score = 0;
                        Lives = 3;
                        LevelSelect = true;
                        GameMenu = false;
                        frmMain.setTargetTexture();
                        frmMain.clearBuffer();
                        frmMain.repaint();
                        StopMusic();
                        DoEvents();
                        PGE_Delay(500);
                        ClearGame();

                        OpenWorld(SelectWorld[selWorld].WorldPath + SelectWorld[selWorld].WorldFile);

                        if(SaveSlot[selSave] >= 0)
                        {
                            if(!NoMap)
                                StartLevel.clear();
                            LoadGame();
                            speedRun_loadStats();
                        }

                        if(WorldUnlock)
                        {
                            For(A, 1, numWorldPaths)
                            {
                                tempLocation = WorldPath[A].Location;
                                {
                                    Location_t &l =tempLocation;
                                    l.X = l.X + 4;
                                    l.Y = l.Y + 4;
                                    l.Width = l.Width - 8;
                                    l.Height = l.Height - 8;
                                }

                                WorldPath[A].Active = true;

                                For(B, 1, numScenes)
                                {
                                    if(CheckCollision(tempLocation, Scene[B].Location))
                                        Scene[B].Active = false;
                                }
                            }

                            For(A, 1, numWorldLevels)
                                WorldLevel[A].Active = true;
                        }

                        SetupPlayers();

                        if(!StartLevel.empty())
                        {
                            PlaySoundMenu(SFX_LevelSelect);
                            SoundPause[26] = 200;
                            LevelSelect = false;

                            GameThing();
                            ClearLevel();

                            PGE_Delay(1000);
                            std::string levelPath = SelectWorld[selWorld].WorldPath + StartLevel;
                            if(!OpenLevel(levelPath))
                            {
                                MessageText = fmt::format_ne("ERROR: Can't open \"{0}\": file doesn't exist or corrupted.", StartLevel);
                                PauseGame(1);
                                ErrorQuit = true;
                            }
                        }
                        return true;
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
                s_handleMouseMove(4, MenuX, MenuY, 300, 30);

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
                s_handleMouseMove(4, MenuX, MenuY, 300, 30);

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
            int optionsMenuLength = 2; // P1, P2, Credits
#if !defined(__ANDROID__) && !defined(__3DS__)
            optionsMenuLength ++; // Fullscreen
#endif
#ifndef __3DS__
            optionsMenuLength ++; // ScaleMode
#endif
#ifndef FIXED_RES
            optionsMenuLength ++; // resolution
#endif
#ifdef NEW_EDITOR
            optionsMenuLength += 1; // EditorControls
#endif
#ifdef __3DS__
            optionsMenuLength += 1; // debugMode
            if (n3ds_clocked != -1)
                optionsMenuLength ++; // clock speed
#endif

            if(MenuMouseMove)
            {
                For(A, 0, optionsMenuLength)
                {
                    if(MenuMouseY >= MenuY + A * 30 && MenuMouseY <= MenuY + A * 30 + 16)
                    {
                        int i = 0;
                        if(A == i++)
                            menuLen = 18 * std::strlen("player 1 controls") - 4;
                        else if(A == i++)
                            menuLen = 18 * std::strlen("player 2 controls") - 4;
#if !defined(__ANDROID__) && !defined(__3DS__)
                        else if(A == i++)
                        {
                            if(resChanged)
                                menuLen = 18 * std::strlen("windowed mode");
                            else
                                menuLen = 18 * std::strlen("fullscreen mode");
                        }
#endif
#if !defined(__3DS__)
                        else if(A == i++)
                            menuLen = 18 * (7+ScaleMode_strings.at(config_ScaleMode).length());
#endif
#if !defined(FIXED_RES)
                        else if(A == i++)
                            menuLen = 18 * std::strlen("res: WWWxHHH (word)");
#endif
#ifdef NEW_EDITOR
                        else if(A == i++)
                            menuLen = 18 * std::strlen("editor controls") - 2;
#endif
#ifdef __3DS__
                        else if(n3ds_clocked != -1 && A == i++)
                            menuLen = 18 * std::strlen("use n3ds clock speed") - 6;
                        else if(A == i++)
                            menuLen = 18 * std::strlen("show debug screen") - 4;
#endif
                        else
                            menuLen = 18 * std::strlen("view credits") - 2;

                        if(MenuMouseX >= MenuX && MenuMouseX <= MenuX + menuLen)
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

            if(MenuCursorCanMove || MenuMouseClick || MenuMouseBack || (menuLeftPress || menuRightPress) && MenuCursorCanMove)
            {
                if(menuBackPress || MenuMouseBack)
                {
                    SaveConfig();
                    MenuMode = MENU_MAIN;
                    MenuCursor = MainMenuOptionsIndex;
                    MenuCursorCanMove = false;
                    PlaySoundMenu(SFX_Slide);
                }
                else if(menuDoPress || MenuMouseClick || menuLeftPress || menuRightPress)
                {
                    MenuCursorCanMove = false;
                    int i = 0;
                    if(MenuCursor == i++)
                    {
                        MenuCursor = 0;
                        MenuMode = MENU_INPUT_SETTINGS_P1;
                        PlaySoundMenu(SFX_Slide);
                    }
                    else if(MenuCursor == i++)
                    {
                        MenuCursor = 0;
                        MenuMode = MENU_INPUT_SETTINGS_P2;
                        PlaySoundMenu(SFX_Slide);
                    }
#if !defined(__ANDROID__) && !defined(__3DS__) // on Android run the always full-screen
                    else if(MenuCursor == i++)
                    {
                        PlaySoundMenu(SFX_Do);
                        ChangeScreen();
                    }
#endif
#if !defined(__3DS__)
                    else if(MenuCursor == i++)
                    {
                        PlaySoundMenu(SFX_Do);
                        if(!menuLeftPress)
                            config_ScaleMode = (ScaleMode_t)((int)config_ScaleMode + 1);
                        else
                            config_ScaleMode = (ScaleMode_t)((int)config_ScaleMode - 1);
                        if(config_ScaleMode > ScaleMode_t::FIXED_2X)
                            config_ScaleMode = ScaleMode_t::DYNAMIC_INTEGER;
                        if(config_ScaleMode < ScaleMode_t::DYNAMIC_INTEGER)
                            config_ScaleMode = ScaleMode_t::FIXED_2X;
                        frmMain.updateViewport();
                    }
#endif
#if !defined(FIXED_RES)
                    else if(MenuCursor == i++)
                    {
                        PlaySoundMenu(SFX_Do);
                        if(!menuLeftPress)
                        {
                            if (config_InternalW == 0 || config_InternalH == 0)
                                { config_InternalW = 480; config_InternalH = 320; }
                            else if (config_InternalW == 480 && config_InternalH == 320)
                                { config_InternalW = 512; config_InternalH = 384; }
                            else if (config_InternalW == 512 && config_InternalH == 384)
                                { config_InternalW = 512; config_InternalH = 448; }
                            else if (config_InternalW == 512 && config_InternalH == 448)
                                { config_InternalW = 640; config_InternalH = 480; }
                            else if (config_InternalW == 640 && config_InternalH == 480)
                                { config_InternalW = 800; config_InternalH = 480; }
                            else if (config_InternalW == 800 && config_InternalH == 480)
                                { config_InternalW = 800; config_InternalH = 600; }
                            else if (config_InternalW == 800 && config_InternalH == 600)
                                { config_InternalW = 960; config_InternalH = 600; }
                            else if (config_InternalW == 960 && config_InternalH == 600)
                                { config_InternalW = 1066; config_InternalH = 600; }
                            else if (config_InternalW == 1066 && config_InternalH == 600)
                                { config_InternalW = 1200; config_InternalH = 600; }
                            else if (config_InternalW == 1200 && config_InternalH == 600)
                                { config_InternalW = 1280; config_InternalH = 720; }
                            else if (config_InternalW == 1280 && config_InternalH == 720)
                                { config_InternalW = 0; config_InternalH = 0; }
                            else
                                { config_InternalW = 0; config_InternalH = 0; }
                        }
                        else
                        {
                            if (config_InternalW == 0 || config_InternalH == 0)
                                { config_InternalW = 1280; config_InternalH = 720; }
                            else if (config_InternalW == 480 && config_InternalH == 320)
                                { config_InternalW = 0; config_InternalH = 0; }
                            else if (config_InternalW == 512 && config_InternalH == 384)
                                { config_InternalW = 480; config_InternalH = 320; }
                            else if (config_InternalW == 512 && config_InternalH == 448)
                                { config_InternalW = 512; config_InternalH = 384; }
                            else if (config_InternalW == 640 && config_InternalH == 480)
                                { config_InternalW = 512; config_InternalH = 448; }
                            else if (config_InternalW == 800 && config_InternalH == 480)
                                { config_InternalW = 640; config_InternalH = 480; }
                            else if (config_InternalW == 800 && config_InternalH == 600)
                                { config_InternalW = 800; config_InternalH = 480; }
                            else if (config_InternalW == 960 && config_InternalH == 600)
                                { config_InternalW = 800; config_InternalH = 600; }
                            else if (config_InternalW == 1066 && config_InternalH == 600)
                                { config_InternalW = 960; config_InternalH = 600; }
                            else if (config_InternalW == 1200 && config_InternalH == 600)
                                { config_InternalW = 1066; config_InternalH = 600; }
                            else if (config_InternalW == 1280 && config_InternalH == 720)
                                { config_InternalW = 1200; config_InternalH = 600; }
                            else
                                { config_InternalW = 0; config_InternalH = 0; }
                        }
                        frmMain.updateViewport();
                    }
#endif
#ifdef __3DS__
                    //TODO: implement 3DS options
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
        else if(MenuMode == MENU_INPUT_SETTINGS_P1 || MenuMode == MENU_INPUT_SETTINGS_P2)
        {
            if(MenuMouseMove && !getNewJoystick && !getNewKeyboard)
            {
                if(useJoystick[MenuMode - MENU_INPUT_SETTINGS_BASE] == 0)
                {
                    For(A, 0, 11)
                    {
                        if(MenuMouseY >= MenuY - 90 + A * 30 && MenuMouseY <= MenuY - 90 + A * 30 + 16)
                        {
                            auto &ck = conKeyboard[MenuMode - MENU_INPUT_SETTINGS_BASE];
                            switch(A)
                            {
                            default:
                            case 0:
                                menuLen = 18 * std::strlen("INPUT......KEYBOARD");
                                break;
                            case 1:
                                menuLen = 18 * static_cast<int>(fmt::format_ne("UP.........{0}",
                                                        getKeyName(ck.Up)).size());
                                break;
                            case 2:
                                menuLen = 18 * static_cast<int>(fmt::format_ne("UP.........{0}",
                                                        getKeyName(ck.Down)).size());
                                break;
                            case 3:
                                menuLen = 18 * static_cast<int>(fmt::format_ne("UP.........{0}",
                                                        getKeyName(ck.Left)).size());
                                break;
                            case 4:
                                menuLen = 18 * static_cast<int>(fmt::format_ne("UP.........{0}",
                                                        getKeyName(ck.Right)).size());
                                break;
                            case 5:
                                menuLen = 18 * static_cast<int>(fmt::format_ne("UP.........{0}",
                                                        getKeyName(ck.Run)).size());
                                break;
                            case 6:
                                menuLen = 18 * static_cast<int>(fmt::format_ne("UP.........{0}",
                                                        getKeyName(ck.AltRun)).size());
                                break;
                            case 7:
                                menuLen = 18 * static_cast<int>(fmt::format_ne("UP.........{0}",
                                                        getKeyName(ck.Jump)).size());
                                break;
                            case 8:
                                menuLen = 18 * static_cast<int>(fmt::format_ne("UP.........{0}",
                                                        getKeyName(ck.AltJump)).size());
                                break;
                            case 9:
                                menuLen = 18 * static_cast<int>(fmt::format_ne("UP.........{0}",
                                                        getKeyName(ck.Drop)).size());
                                break;
                            case 10:
                                menuLen = 18 * static_cast<int>(fmt::format_ne("UP.........{0}",
                                                        getKeyName(ck.Start)).size());
                                break;
                            case 11:
                                menuLen = 18 * std::strlen("Reset tp default");
                                break;
                            }

                            if(MenuMouseX >= MenuX && MenuMouseX <= MenuX + menuLen)
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
                else
                {
                    For(A, 0, 11)
                    {
                        if(MenuMouseY >= MenuY - 90 + A * 30 && MenuMouseY <= MenuY - 90 + A * 30 + 16)
                        {
                            if(A == 0)
                            {
                                menuLen = 18 * std::strlen("INPUT......JOYSTICK 1") - 2;
                            }
                            else
                            {
                                menuLen = 18 * std::strlen("RUN........_");
                            }
                            if(MenuMouseX >= MenuX && MenuMouseX <= MenuX + menuLen)
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
            }

            if(MenuCursorCanMove || ((!getNewKeyboard && !getNewJoystick) && (MenuMouseClick || MenuMouseBack)))
            {
                if(getNewKeyboard)
                {
                    if(inputKey != 0)
                    {
                        getNewKeyboard = false;
                        MenuCursorCanMove = false;
                        PlaySoundMenu(SFX_Do);
                        setKey(conKeyboard[MenuMode - MENU_INPUT_SETTINGS_BASE], MenuCursor, inputKey);
                    }

                }
                else if(getNewJoystick)
                {
                    int JoyNum = useJoystick[MenuMode - MENU_INPUT_SETTINGS_BASE] - 1;
                    //SDL_JoystickUpdate();
                    KM_Key joyKey;
                    bool gotNewKey = joyPollJoystick(JoyNum, joyKey);

                    //if(!joyIsKeyDown(JoyNum, oldJumpJoy))
                    {
                        oldJumpJoy.type = -1;
                        oldJumpJoy.ctrl_type = -1;
                        auto &cj = conJoystick[MenuMode - MENU_INPUT_SETTINGS_BASE];

                        if(gotNewKey)
                        {
                            if(cj.isGameController)
                                joyKey.type = lastJoyButton.type;
                            PlaySoundMenu(SFX_Do);
                            setKey(cj, MenuCursor, joyKey);
                            // Save the changed state into the common cache
                            joySetByIndex(MenuMode - MENU_INPUT_SETTINGS_BASE, JoyNum, cj);
                            getNewJoystick = false;
                            MenuCursorCanMove = false;
                        }
                        else if(escPressed)
                        {
                            PlaySoundMenu(SFX_BlockHit);
                            setKey(cj, MenuCursor, lastJoyButton);
                            getNewJoystick = false;
                            MenuCursorCanMove = false;
                        }
                    }
                }
                else
                {
                    if(menuBackPress || MenuMouseBack)
                    {
                        SaveConfig();
                        MenuCursor = MenuMode - (MENU_INPUT_SETTINGS_BASE + 1);
                        MenuMode = MENU_OPTIONS;
                        MenuCursorCanMove = false;
                        PlaySoundMenu(SFX_Slide);
                    }
                    else if(menuDoPress || MenuMouseClick)
                    {
                        auto &uj = useJoystick[MenuMode - MENU_INPUT_SETTINGS_BASE];
                        auto &ck = conKeyboard[MenuMode - MENU_INPUT_SETTINGS_BASE];
                        auto &cj = conJoystick[MenuMode - MENU_INPUT_SETTINGS_BASE];

                        if(MenuCursor == 0)
                        {
                            PlaySoundMenu(SFX_Do);
                            uj += 1;
                            if(uj > numJoysticks)
                                uj = 0;

                            if(uj > 0)
                            {
                                int joyNum = uj - 1;
                                // Load the saved state for given joystick
                                joyGetByIndex(MenuMode - MENU_INPUT_SETTINGS_BASE, joyNum, cj);
                            }
                            // Tell player is prefer to use the keyboard than controller
                            wantedKeyboard[MenuMode - MENU_INPUT_SETTINGS_BASE] = (uj == 0);
                        }
                        else
                        if(MenuCursor == 11) // Reset to default
                        {
                            PlaySoundMenu(SFX_NewPath);
                            if(uj == 0)
                            {
                                joyFillDefaults(ck);
                            }
                            else
                            {
                                int JoyNum = uj - 1;
                                if(JoyNum >= 0)
                                {
                                    joyFillDefaults(cj);
                                    joySetByIndex(MenuMode - MENU_INPUT_SETTINGS_BASE, JoyNum, cj);
                                }
                            }
                        }
                        else
                        {
                            if(uj == 0)
                            {
                                getNewKeyboard = true;
                                setKey(ck, MenuCursor, -1);
                                inputKey = 0;
                            }
                            else
                            {
                                auto &key = getKey(cj, MenuCursor);
                                lastJoyButton = key;
                                key.type = -1;
                                key.ctrl_type = -1;
                                if(MenuCursor == 7)
                                    oldJumpJoy = key;
                                getNewJoystick = true;
                                MenuCursorCanMove = false;
                            }
                        }
                        MenuCursorCanMove = false;
                    }
                }
            }

            if(MenuMode != MENU_OPTIONS)
            {
                if(MenuCursor > 11)
                    MenuCursor = 0;
                if(MenuCursor < 0)
                    MenuCursor = 11;
#if 0
                if(useJoystick[MenuMode - 30] == 0)
                {
                    if(MenuCursor > 10)
                        MenuCursor = 0;
                    if(MenuCursor < 0)
                        MenuCursor = 10;
                }
                else
                {
                    if(MenuCursor > 10)
                        MenuCursor = 0;
                    if(MenuCursor < 0)
                        MenuCursor = 10;
                }
#endif
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
    if(MenuMode == MENU_EDITOR)
        SuperPrint(g_mainMenu.mainEditor, 3, x, y);
    else if(menuBattleMode)
        SuperPrint(g_mainMenu.mainBattleGame, 3, x, y, 0.3f, 0.3f, 1.0f);
    else
    {
        float r = menuPlayersNum == 1 ? 1.f : 0.3f;
        float g = menuPlayersNum == 2 ? 1.f : 0.3f;
        if (menuPlayersNum == 1)
            SuperPrint(g_mainMenu.main1PlayerGame, 3, x, y, r, g, 0.3f);
        else if (menuPlayersNum == 2)
            SuperPrint(g_mainMenu.main2PlayerGame, 3, x, y, r, g, 0.3f);
    }
}

static void s_drawGameSaves(int MenuX, int MenuY)
{
    int A;

    for(A = 1; A <= maxSaveSlots; A++)
    {
        if(SaveSlot[A] >= 0)
        {
            SuperPrint(fmt::format_ne("SLOT {0} ... {1}%", A, SaveSlot[A]), 3, MenuX, MenuY - 30 + (A * 30));
            if(SaveStars[A] > 0)
            {
                frmMain.renderTexture(MenuX + 260, MenuY - 30 + (A * 30) + 1,
                                      GFX.Interface[5].w, GFX.Interface[5].h,
                                      GFX.Interface[5], 0, 0);
                frmMain.renderTexture(MenuX + 260 + 24, MenuY - 30 + (A * 30) + 2,
                                      GFX.Interface[1].w, GFX.Interface[1].h,
                                      GFX.Interface[1], 0, 0);
                SuperPrint(fmt::format_ne(" {0}", SaveStars[A]), 3, MenuX + 288, MenuY - 30 + (A * 30));
            }
        }
        else
        {
            SuperPrint(fmt::format_ne("SLOT {0} ... NEW GAME", A), 3, MenuX, MenuY - 30 + (A * 30));
        }
    }

    if(MenuMode == MENU_SELECT_SLOT_1P || MenuMode == MENU_SELECT_SLOT_2P)
    {
        SuperPrint("COPY SAVE", 3, MenuX, MenuY - 30 + (A * 30));
        A++;
        SuperPrint("ERASE SAVE", 3, MenuX, MenuY - 30 + (A * 30));
    }
}

void mainMenuDraw()
{
    int MenuX, MenuY;
    GetMenuPos(MenuX, MenuY);

    int A = 0;
    int B = 0;
    int C = 0;

    if(MenuMode != MENU_1PLAYER_GAME && MenuMode != MENU_2PLAYER_GAME && MenuMode != MENU_BATTLE_MODE)
        worldCurs = 0;

#ifdef __3DS__
    frmMain.setLayer(2);
#endif

    // correction to loop the original asset properly
    A = GFX.MenuGFX[1].w;
    if (A == 800)
        A = 768;
    // horizReps
    B = ScreenW / A + 1;
    for (C = 0; C < B; C++)
        frmMain.renderTexture(A * C, 0, A, GFX.MenuGFX[1].h, GFX.MenuGFX[1], 0, 0);
    // URL
    if(ScreenH >= SmallScreenH)
        frmMain.renderTexture(ScreenW / 2 - GFX.MenuGFX[3].w / 2, ScreenH - 24,
                GFX.MenuGFX[3].w, GFX.MenuGFX[3].h, GFX.MenuGFX[3], 0, 0);

    // game logo
    int LogoMode = 0;
    if (ScreenH < SmallScreenH &&
        (MenuMode == MENU_INPUT_SETTINGS_P1 || MenuMode == MENU_INPUT_SETTINGS_P2))
    {
        LogoMode = 2;
    }
    else if (ScreenH >= TinyScreenH || MenuMode == MENU_INTRO)
    {
        LogoMode = 1;
    }
    else if (MenuMode == MENU_MAIN || MenuMode == MENU_OPTIONS)
        LogoMode = 2;

    if(LogoMode == 1)
    {
#ifdef __3DS__
        frmMain.setLayer(3);
#endif
        if(ScreenH < SmallScreenH)
            frmMain.renderTexture(ScreenW / 2 - GFX.MenuGFX[2].w / 2, 30,
                    GFX.MenuGFX[2].w, GFX.MenuGFX[2].h, GFX.MenuGFX[2], 0, 0);
        else
            frmMain.renderTexture(ScreenW / 2 - GFX.MenuGFX[2].w / 2, 70,
                    GFX.MenuGFX[2].w, GFX.MenuGFX[2].h, GFX.MenuGFX[2], 0, 0);
    }
    else if (LogoMode == 2)
    {
        SuperPrint(g_gameInfo.title, 3, ScreenW/2 - g_gameInfo.title.length()*9, 30);
    }

#ifndef PGE_NO_THREADING
    if(SDL_AtomicGet(&loading))
    {
        if(SDL_AtomicGet(&loadingProgrssMax) <= 0)
            SuperPrint(g_mainMenu.loading, 3, MenuX, MenuY);
        else
        {
            int progress = (SDL_AtomicGet(&loadingProgrss) * 100) / SDL_AtomicGet(&loadingProgrssMax);
            SuperPrint(fmt::format_ne("{0} {1}%", g_mainMenu.loading, progress), 3, MenuX, MenuY);
        }
    }
    else
#endif

    // Menu Intro
    if(MenuMode == MENU_INTRO)
    {
        BlockFlash += 1;

        if(BlockFlash >= 90)
            BlockFlash = 0;

        if(BlockFlash < 45)
            SuperPrint(g_mainMenu.introPressStart, 3, ScreenW/2 - g_mainMenu.introPressStart.length()*9, ScreenH - 40);
    }
    // Main menu
    else if(MenuMode == MENU_MAIN)
    {
        A = 0;
        SuperPrint(g_mainMenu.main1PlayerGame, 3, MenuX, MenuY + 30 * (A++));
        SuperPrint(g_mainMenu.main2PlayerGame, 3, MenuX, MenuY + 30 * (A++));
        SuperPrint(g_mainMenu.mainBattleGame, 3, MenuX, MenuY + 30 * (A++));
#ifdef NEW_EDITOR
        SuperPrint(g_mainMenu.mainEditor, 3, MenuX, MenuY + 30 * (A++));
#endif
        SuperPrint(g_mainMenu.mainOptions, 3, MenuX, MenuY + 30 * (A++));
        SuperPrint(g_mainMenu.mainExit, 3, MenuX, MenuY + 30 * (A++));
        frmMain.renderTexture(MenuX - 20, MenuY + (MenuCursor * 30), 16, 16, GFX.MCursor[0], 0, 0);
    }

    // Character select
    else if(MenuMode == MENU_CHARACTER_SELECT_1P ||
            MenuMode == MENU_CHARACTER_SELECT_2P_S1 ||
            MenuMode == MENU_CHARACTER_SELECT_2P_S2 ||
            MenuMode == MENU_CHARACTER_SELECT_BM_S1 ||
            MenuMode == MENU_CHARACTER_SELECT_BM_S2)
    {
        const std::vector<SelectWorld_t>& SelectorList
            = (MenuMode == MENU_CHARACTER_SELECT_BM_S1 || MenuMode == MENU_CHARACTER_SELECT_BM_S2)
                ? SelectBattle : SelectWorld;

        A = 0;
        B = 0;
        C = 0;

        s_drawGameTypeTitle(MenuX, MenuY - 70);
        SuperPrint(SelectorList[selWorld].WorldName, 3, MenuX, MenuY - 40, 0.6f, 1.f, 1.f);

        // TODO: Make a custom playable character names print here
        if(!blockCharacter[1])
            SuperPrint(g_mainMenu.selectPlayer[1], 3, MenuX, MenuY);
        else
        {
            A -= 30;
            if(MenuCursor + 1 >= 1)
                B -= 30;
            if(PlayerCharacter >= 1)
                C -= 30;
        }

        if(!blockCharacter[2])
            SuperPrint(g_mainMenu.selectPlayer[2], 3, MenuX, MenuY + 30 + A);
        else
        {
            A = A - 30;
            if(MenuCursor + 1 >= 2)
                B = B - 30;
            if(PlayerCharacter >= 2)
                C = C - 30;
        }

        if(!blockCharacter[3])
            SuperPrint(g_mainMenu.selectPlayer[3], 3, MenuX, MenuY + 60 + A);
        else
        {
            A -= 30;
            if(MenuCursor + 1 >= 3)
                B -= 30;
            if(PlayerCharacter >= 3)
                C -= 30;
        }

        if(!blockCharacter[4])
            SuperPrint(g_mainMenu.selectPlayer[4], 3, MenuX, MenuY + 90 + A);
        else
        {
            A -= 30;
            if(MenuCursor + 1 >= 4)
                B -= 30;
            if(PlayerCharacter >= 4)
                C -= 30;
        }

        if(!blockCharacter[5])
            SuperPrint(g_mainMenu.selectPlayer[5], 3, MenuX, MenuY + 120 + A);
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
            frmMain.renderTexture(MenuX - 20, B + MenuY + (MenuCursor * 30), GFX.MCursor[3]);
            frmMain.renderTexture(MenuX - 20, C + MenuY + ((PlayerCharacter - 1) * 30), GFX.MCursor[0]);
        }
        else
        {
            frmMain.renderTexture(MenuX - 20, B + MenuY + (MenuCursor * 30), GFX.MCursor[0]);
        }
    }

    // Episode / Level selection
    else if(MenuMode == MENU_1PLAYER_GAME || MenuMode == MENU_2PLAYER_GAME
        || MenuMode == MENU_BATTLE_MODE || MenuMode == MENU_EDITOR)
    {
        s_drawGameTypeTitle(MenuX, MenuY - 70);
        std::string tempStr;

        minShow = 1;
        maxShow = (MenuMode == MENU_BATTLE_MODE) ? NumSelectBattle :
            ((MenuMode == MENU_EDITOR) ? NumSelectWorldEditable :
                NumSelectWorld);
        const std::vector<SelectWorld_t>& SelectorList
            = (MenuMode == MENU_BATTLE_MODE) ? SelectBattle :
                ((MenuMode == MENU_EDITOR) ? SelectWorldEditable :
                    SelectWorld);
        int original_maxShow = maxShow;

        if(maxShow > 5)
        {
            minShow = worldCurs;
            maxShow = minShow + 4;

            if(MenuCursor <= minShow - 1)
                worldCurs -= 1;

            if(MenuCursor >= maxShow - 1)
                worldCurs += 1;

            if(worldCurs < 1)
                worldCurs = 1;

            if(worldCurs > original_maxShow - 4)
                worldCurs = original_maxShow - 4;

            if(maxShow >= original_maxShow)
            {
                maxShow = original_maxShow;
                minShow = original_maxShow - 4;
            }

            minShow = worldCurs;
            maxShow = minShow + 4;
        }

        for(auto A = minShow; A <= maxShow; A++)
        {
            B = A - minShow + 1;
            tempStr = SelectorList[A].WorldName;
            SuperPrint(tempStr, 3, MenuX, MenuY - 30 + (B * 30));
        }

        if(minShow > 1)
            frmMain.renderTexture(ScreenW/2 - 8, MenuY - 20, GFX.MCursor[1]);

        if(maxShow < original_maxShow)
            frmMain.renderTexture(ScreenW/2 - 8, MenuY + 140, GFX.MCursor[2]);

        B = MenuCursor - minShow + 1;

        if(B >= 0 && B < 5)
            frmMain.renderTexture(MenuX - 20, MenuY + (B * 30), GFX.MCursor[0].w, GFX.MCursor[0].h, GFX.MCursor[0], 0, 0);
    }

    else if(MenuMode == MENU_SELECT_SLOT_1P || MenuMode == MENU_SELECT_SLOT_2P) // Save Select
    {
        s_drawGameTypeTitle(MenuX, MenuY - 70);
        SuperPrint(SelectWorld[selWorld].WorldName, 3, MenuX, MenuY - 40, 0.6f, 1.f, 1.f);
        s_drawGameSaves(MenuX, MenuY);
        frmMain.renderTexture(MenuX - 20, MenuY + (MenuCursor * 30), GFX.MCursor[0]);
    }

    else if(MenuMode == MENU_SELECT_SLOT_1P_COPY_S1 || MenuMode == MENU_SELECT_SLOT_2P_COPY_S1 ||
            MenuMode == MENU_SELECT_SLOT_1P_COPY_S2 || MenuMode == MENU_SELECT_SLOT_2P_COPY_S2) // Copy save
    {
        s_drawGameTypeTitle(MenuX, MenuY - 70);
        SuperPrint(SelectWorld[selWorld].WorldName, 3, MenuX, MenuY - 40, 0.6f, 1.f, 1.f);
        s_drawGameSaves(MenuX, MenuY);

        if(MenuMode == MENU_SELECT_SLOT_1P_COPY_S1 || MenuMode == MENU_SELECT_SLOT_2P_COPY_S1)
            SuperPrint("Select the source slot", 3, MenuX, MenuY - 30 + (5 * 30), 0.7f, 0.7f, 1.0f);
        else if(MenuMode == MENU_SELECT_SLOT_1P_COPY_S2 || MenuMode == MENU_SELECT_SLOT_2P_COPY_S2)
            SuperPrint("Now select the target", 3, MenuX, MenuY - 30 + (5 * 30), 0.7f, 1.0f, 0.7f);

        if(MenuMode == MENU_SELECT_SLOT_1P_COPY_S2 || MenuMode == MENU_SELECT_SLOT_2P_COPY_S2)
        {
            frmMain.renderTexture(MenuX - 20, MenuY + ((menuCopySaveSrc - 1) * 30), GFX.MCursor[0]);
            frmMain.renderTexture(MenuX - 20, MenuY + (MenuCursor * 30), GFX.MCursor[3]);
        }
        else
            frmMain.renderTexture(MenuX - 20, MenuY + (MenuCursor * 30), GFX.MCursor[0]);
    }

    else if(MenuMode == MENU_SELECT_SLOT_1P_DELETE || MenuMode == MENU_SELECT_SLOT_2P_DELETE) // Copy save
    {
        s_drawGameTypeTitle(MenuX, MenuY - 70);
        SuperPrint(SelectWorld[selWorld].WorldName, 3, MenuX, MenuY - 40, 0.6f, 1.f, 1.f);
        s_drawGameSaves(MenuX, MenuY);

        SuperPrint("Select the slot to erase", 3, MenuX, MenuY - 30 + (5 * 30), 1.0f, 0.7f, 0.7f);

        frmMain.renderTexture(MenuX - 20, MenuY + (MenuCursor * 30), GFX.MCursor[0]);
    }

    // Options Menu
    else if(MenuMode == MENU_OPTIONS)
    {
        SuperPrint("PLAYER 1 CONTROLS", 3, MenuX, MenuY);
        SuperPrint("PLAYER 2 CONTROLS", 3, MenuX, MenuY + 30);
        A = 2;
#if !defined(__ANDROID__) && !defined(__3DS__)
        if(resChanged)
            SuperPrint("WINDOWED MODE", 3, MenuX, MenuY + 30*A);
        else
            SuperPrint("FULLSCREEN MODE", 3, MenuX, MenuY + 30*A);
        A ++;
#endif
#ifndef __3DS__
        SuperPrint("SCALE: "+ScaleMode_strings.at(config_ScaleMode), 3, MenuX, MenuY + 30*A);
        A ++;
#endif
#ifndef FIXED_RES
        std::string resString = fmt::format_ne("RES: {0}x{1}", config_InternalW, config_InternalH);
        if (config_InternalW == 480 && config_InternalH == 320)
            resString += " (GBA)";
        else if (config_InternalW == 512 && config_InternalH == 384)
            resString += " (NDS)";
        else if (config_InternalW == 512 && config_InternalH == 448)
            resString += " (SNES)";
        else if (config_InternalW == 640 && config_InternalH == 480)
            resString += " (VGA)";
        else if (config_InternalW == 800 && config_InternalH == 480)
            resString += " (3DS)";
        else if (config_InternalW == 800 && config_InternalH == 600)
            resString += " (SMBX)";
        else if (config_InternalW == 960 && config_InternalH == 600)
            resString += " (16:10)";
        else if (config_InternalW == 1066 && config_InternalH == 600)
            resString += " (16:9)";
        else if (config_InternalW == 1200 && config_InternalH == 600)
            resString += " (18:9)";
        else if (config_InternalW == 1280 && config_InternalH == 720)
            resString += " (HD)";
        else if (config_InternalW == 0 || config_InternalH == 0)
            resString = "RES: DYNAMIC";
        else
            resString += " (CUSTOM)";
        SuperPrint(resString, 3, MenuX, MenuY + 30*A);
        A ++;
#endif
#ifdef __3DS__
        SuperPrint("EDITOR CONTROLS", 3, MenuX, MenuY + 30*A);
        A++;
        if (n3ds_clocked != -1)
        {
            if (n3ds_clocked)
                SuperPrint("USE O3DS CLOCK SPEED", 3, MenuX, MenuY + 30*A);
            else
                SuperPrint("USE N3DS CLOCK SPEED", 3, MenuX, MenuY + 30*A);
            A++;
        }
        if (debugMode)
            SuperPrint("HIDE DEBUG SCREEN", 3, MenuX, MenuY + 30*A);
        else
            SuperPrint("SHOW DEBUG SCREEN", 3, MenuX, MenuY + 30*A);
        A++;
#endif
        SuperPrint("VIEW CREDITS", 3, MenuX, MenuY + 30*A);
        frmMain.renderTexture(MenuX - 20, MenuY + (MenuCursor * 30),
                              GFX.MCursor[0].w, GFX.MCursor[0].h, GFX.MCursor[0], 0, 0);
    }

    // Player controls setup
    else if(MenuMode == MENU_INPUT_SETTINGS_P1 || MenuMode == MENU_INPUT_SETTINGS_P2)
    {
        if(useJoystick[MenuMode - MENU_INPUT_SETTINGS_BASE] == 0)
        {
            auto &ck = conKeyboard[MenuMode - MENU_INPUT_SETTINGS_BASE];
            SuperPrint("INPUT......KEYBOARD", 3, MenuX, MenuY - 90);
            SuperPrint(fmt::format_ne("UP.........{0}", getKeyName(ck.Up)), 3, MenuX, MenuY - 60);
            SuperPrint(fmt::format_ne("DOWN.......{0}", getKeyName(ck.Down)), 3, MenuX, MenuY - 30);
            SuperPrint(fmt::format_ne("LEFT.......{0}", getKeyName(ck.Left)), 3, MenuX, MenuY);
            SuperPrint(fmt::format_ne("RIGHT......{0}", getKeyName(ck.Right)), 3, MenuX, MenuY + 30);
            SuperPrint(fmt::format_ne("RUN........{0}", getKeyName(ck.Run)), 3, MenuX, MenuY + 60);
            SuperPrint(fmt::format_ne("ALT RUN....{0}", getKeyName(ck.AltRun)), 3, MenuX, MenuY + 90);
            SuperPrint(fmt::format_ne("JUMP.......{0}", getKeyName(ck.Jump)), 3, MenuX, MenuY + 120);
            SuperPrint(fmt::format_ne("ALT JUMP...{0}", getKeyName(ck.AltJump)), 3, MenuX, MenuY + 150);
            SuperPrint(fmt::format_ne("DROP ITEM..{0}", getKeyName(ck.Drop)), 3, MenuX, MenuY + 180);
            SuperPrint(fmt::format_ne("PAUSE......{0}", getKeyName(ck.Start)), 3, MenuX, MenuY + 210);
            SuperPrint("Reset to default", 3, MenuX, MenuY + 240);
        }
        else
        {
            auto &cj = conJoystick[MenuMode - MENU_INPUT_SETTINGS_BASE];
            SuperPrint("INPUT......" + joyGetName(useJoystick[MenuMode - MENU_INPUT_SETTINGS_BASE] - 1), 3, MenuX, MenuY - 90);
            SuperPrint(fmt::format_ne("UP.........{0}", getJoyKeyName(cj.isGameController, cj.Up)), 3, MenuX, MenuY - 60);
            SuperPrint(fmt::format_ne("DOWN.......{0}", getJoyKeyName(cj.isGameController, cj.Down)), 3, MenuX, MenuY - 30);
            SuperPrint(fmt::format_ne("LEFT.......{0}", getJoyKeyName(cj.isGameController, cj.Left)), 3, MenuX, MenuY);
            SuperPrint(fmt::format_ne("RIGHT......{0}", getJoyKeyName(cj.isGameController, cj.Right)), 3, MenuX, MenuY + 30);
            SuperPrint(fmt::format_ne("RUN........{0}", getJoyKeyName(cj.isGameController, cj.Run)), 3, MenuX, MenuY + 60);
            SuperPrint(fmt::format_ne("ALT RUN....{0}", getJoyKeyName(cj.isGameController, cj.AltRun)), 3, MenuX, MenuY + 90);
            SuperPrint(fmt::format_ne("JUMP.......{0}", getJoyKeyName(cj.isGameController, cj.Jump)), 3, MenuX, MenuY + 120);
            SuperPrint(fmt::format_ne("ALT JUMP...{0}", getJoyKeyName(cj.isGameController, cj.AltJump)), 3, MenuX, MenuY + 150);
            SuperPrint(fmt::format_ne("DROP ITEM..{0}", getJoyKeyName(cj.isGameController, cj.Drop)), 3, MenuX, MenuY + 180);
            SuperPrint(fmt::format_ne("PAUSE......{0}", getJoyKeyName(cj.isGameController, cj.Start)), 3, MenuX, MenuY + 210);
            SuperPrint("Reset to default", 3, MenuX, MenuY + 240);
        }

        frmMain.renderTexture(MenuX - 20, MenuY - 90 + (MenuCursor * 30),
                              GFX.MCursor[0].w, GFX.MCursor[0].h, GFX.MCursor[0], 0, 0);
    }

    // Mouse cursor
    frmMain.renderTexture(int(MenuMouseX), int(MenuMouseY), GFX.ECursor[2]);
}
