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
#include <fmt_format_ne.h>

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


MainMenuContent g_mainMenu;

void initMainMenu()
{
    g_mainMenu.main1PlayerGame = "1 Player Game";
    g_mainMenu.main2PlayerGame = "2 Player Game";
    g_mainMenu.mainBattleGame = "Battle Game";
    g_mainMenu.mainOptions = "Options";
    g_mainMenu.mainExit = "Exit";

    for(int i = 1; i <= numCharacters; ++i)
        g_mainMenu.selectPlayer[i] = fmt::format_ne("{0} game", g_gameInfo.characterName[i]);
}


static int ScrollDelay = 0;
static int menuPlayersNum = 0;
static int menuBattleMode = false;

bool mainMenuUpdate()
{
    int B;
    Location_t tempLocation;
    int menuLen;
    Player_t blankPlayer;

    bool altPressed = getKeyState(SDL_SCANCODE_LALT) == KEY_PRESSED ||
                      getKeyState(SDL_SCANCODE_RALT) == KEY_PRESSED;
    bool escPressed = getKeyState(SDL_SCANCODE_ESCAPE) == KEY_PRESSED;
#ifdef __ANDROID__
    escPressed |= getKeyState(SDL_SCANCODE_AC_BACK) == KEY_PRESSED;
#endif
    bool spacePressed = getKeyState(SDL_SCANCODE_SPACE) == KEY_PRESSED;
    bool returnPressed = getKeyState(SDL_SCANCODE_RETURN) == KEY_PRESSED;
    bool upPressed = getKeyState(SDL_SCANCODE_UP) == KEY_PRESSED;
    bool downPressed = getKeyState(SDL_SCANCODE_DOWN) == KEY_PRESSED;

    bool menuDoPress = (returnPressed && !altPressed) || spacePressed;
    bool menuBackPress = (escPressed && !altPressed);

    {
        Controls_t &c = Player[1].Controls;

        menuDoPress |= (c.Start || c.Jump) && !altPressed;
        menuBackPress |= c.Run && !altPressed;

        if(frmMain.MousePointer != 99)
        {
            frmMain.MousePointer = 99;
            showCursor(0);
        }

        if(!c.Up && !c.Down && !c.Jump && !c.Run && !c.Start)
        {
            bool k = false;
            k |= menuDoPress;
            k |= upPressed;
            k |= downPressed;
            k |= escPressed;

            if(!k)
                MenuCursorCanMove = true;

        }

        if(!getNewKeyboard && !getNewJoystick)
        {
            if(c.Up || upPressed)
            {
                if(MenuCursorCanMove)
                {
                    MenuCursor -= 1;

                    if(MenuMode >= MENU_CHARACTER_SELECT_BASE)
                    {
                        while((MenuCursor == (PlayerCharacter - 1) &&
                              (MenuMode == MENU_CHARACTER_SELECT_2P_S2 || MenuMode == MENU_CHARACTER_SELECT_BM_S2)) ||
                               blockCharacter[MenuCursor + 1])
                        {
                            MenuCursor -= 1;
                            if(MenuCursor < 0)
                                MenuCursor = numCharacters - 1;
                        }
                    }
                    PlaySoundMenu(SFX_Slide);
                }

                MenuCursorCanMove = false;
            }
            else if(c.Down || downPressed)
            {
                if(MenuCursorCanMove)
                {
                    MenuCursor += 1;

                    if(MenuMode >= MENU_CHARACTER_SELECT_BASE)
                    {
                        while((MenuCursor == (PlayerCharacter - 1) &&
                              (MenuMode == MENU_CHARACTER_SELECT_2P_S2 || MenuMode == MENU_CHARACTER_SELECT_BM_S2)) ||
                               blockCharacter[MenuCursor + 1])
                        {
                            MenuCursor += 1;
                            if(MenuCursor >= numCharacters)
                                MenuCursor = 0;
                        }
                    }
                    PlaySoundMenu(SFX_Slide);
                }

                MenuCursorCanMove = false;
            }
        }

        // Main Menu
        if(MenuMode == MENU_MAIN)
        {
            if(MenuMouseMove)
            {
                For(A, 0, 4)
                {
                    if(MenuMouseY >= 350 + A * 30 && MenuMouseY <= 366 + A * 30)
                    {
                        if(A == 0)
                            menuLen = 18 * g_mainMenu.main1PlayerGame.size() - 2;
                        else if(A == 1)
                            menuLen = 18 * g_mainMenu.main2PlayerGame.size() - 2;
                        else if(A == 2)
                            menuLen = 18 * g_mainMenu.mainBattleGame.size();
                        else if(A == 3)
                            menuLen = 18 * g_mainMenu.mainOptions.size();
                        else
                            menuLen = 18 * g_mainMenu.mainExit.size();

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

            if(escPressed && MenuCursorCanMove)
            {
                if(MenuCursor != 4)
                {
                    MenuCursor = 4;
                    PlaySoundMenu(SFX_Slide);
                }
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
                    FindWorlds();
                    MenuCursor = 0;
                }
                else if(MenuCursor == 1)
                {
                    PlaySoundMenu(SFX_Do);
                    MenuMode = MENU_2PLAYER_GAME;
                    menuPlayersNum = 2;
                    menuBattleMode = false;
                    FindWorlds();
                    MenuCursor = 0;
                }
                else if(MenuCursor == 2)
                {
                    PlaySoundMenu(SFX_Do);
                    MenuMode = MENU_BATTLE_MODE;
                    menuPlayersNum = 2;
                    menuBattleMode = true;
                    FindLevels();
                    MenuCursor = 0;
                }
                else if(MenuCursor == 3)
                {
                    PlaySoundMenu(SFX_Do);
                    MenuMode = MENU_OPTIONS;
                    MenuCursor = 0;
                }
                else if(MenuCursor == 4)
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

            if(MenuCursor > 4)
                MenuCursor = 0;
            if(MenuCursor < 0)
                MenuCursor = 4;
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

            if(MenuMode >= MENU_CHARACTER_SELECT_BASE)
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
        }

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
                        MenuCursor = 2;

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

                    MenuMode *= MENU_CHARACTER_SELECT_BASE;
                    MenuCursor = 0;

                    if(MenuMode == MENU_CHARACTER_SELECT_BM_S1 && PlayerCharacter != 0)
                        MenuCursor = PlayerCharacter - 1;

                    MenuCursorCanMove = false;
                }

            }

            if(MenuMode < MENU_CHARACTER_SELECT_BASE)
            {
                if(MenuCursor >= NumSelectWorld)
                    MenuCursor = 0;
                if(MenuCursor < 0)
                    MenuCursor = NumSelectWorld - 1;
            }
        }

        // Save Select
        else if(MenuMode == MENU_SELECT_SLOT_1P || MenuMode == MENU_SELECT_SLOT_2P)
        {
            if(MenuMouseMove)
            {
                For(A, 0, 2)
                {
                    if(MenuMouseY >= 350 + A * 30 && MenuMouseY <= 366 + A * 30)
                    {
                        menuLen = 18 * std::strlen("slot 1 empty") - 2;

                        if(SaveSlot[A + 1] >= 0)
                            menuLen = 18 * std::strlen("slot ... 100") - 2;

                        if(SaveStars[A + 1] > 0)
                            menuLen = 288 + 2/*sizeof(short) == 2 in VB6*/ * 18;

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
            }

            if(MenuMode < MENU_CHARACTER_SELECT_BASE)
            {
                if(MenuCursor > 2) MenuCursor = 0;
                if(MenuCursor < 0) MenuCursor = 2;
            }
        }

        // Options
        else if(MenuMode == MENU_OPTIONS)
        {
#ifndef __ANDROID__
            const int optionsMenuLength = 3;
#else
            const int optionsMenuLength = 2;
#endif

            if(MenuMouseMove)
            {
                For(A, 0, optionsMenuLength)
                {
                    if(MenuMouseY >= 350 + A * 30 && MenuMouseY <= 366 + A * 30)
                    {
                        if(A == 0)
                            menuLen = 18 * std::strlen("player 1 controls") - 4;
                        else if(A == 1)
                            menuLen = 18 * std::strlen("player 2 controls") - 4;
#ifndef __ANDROID__
                        else if(A == 2)
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
                    MenuMode = MENU_MAIN;
                    MenuCursor = 3;
                    MenuCursorCanMove = false;
                    PlaySoundMenu(SFX_Slide);
                }
                else if(menuDoPress || MenuMouseClick)
                {
                    MenuCursorCanMove = false;
                    if(MenuCursor == 0)
                    {
                        MenuCursor = 0;
                        MenuMode = MENU_INPUT_SETTINGS_P1;
                        PlaySoundMenu(SFX_Slide);
                    }
                    else if(MenuCursor == 1)
                    {
                        MenuCursor = 0;
                        MenuMode = MENU_INPUT_SETTINGS_P2;
                        PlaySoundMenu(SFX_Slide);
#ifndef __ANDROID__ // on Android run the always full-screen
                    }
                    else if(MenuCursor == 2)
                    {
                        PlaySoundMenu(SFX_Do);
                        ChangeScreen();
                    }
                    else if(MenuCursor == 3)
                    {
#else
                    }
                    else if(MenuCursor == 2)
                    {
#endif
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
                        if(MenuMouseY >= 260 - 44 + A * 30 && MenuMouseY <= 276 - 44 + A * 30)
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
                else
                {
                    For(A, 0, 11)
                    {
                        if(MenuMouseY >= 260 - 44 + A * 30 && MenuMouseY <= 276 + A * 30 - 44)
                        {
                            if(A == 0)
                            {
                                menuLen = 18 * std::strlen("INPUT......JOYSTICK 1") - 2;
                            }
                            else
                            {
                                menuLen = 18 * std::strlen("RUN........_");
                            }
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
    if(menuBattleMode)
        SuperPrint("Battle game", 3, x, y, 0.3f, 0.3f, 1.0f);
    else
    {
        float r = menuPlayersNum == 1 ? 1.f : 0.3f;
        float g = menuPlayersNum == 2 ? 1.f : 0.3f;
        SuperPrint(fmt::format_ne("{0} Player game", menuPlayersNum), 3, x, y, r, g, 0.3f);
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

    // Main menu
    if(MenuMode == MENU_MAIN)
    {
        SuperPrint(g_mainMenu.main1PlayerGame, 3, 300, 350);
        SuperPrint(g_mainMenu.main2PlayerGame, 3, 300, 380);
        SuperPrint(g_mainMenu.mainBattleGame, 3, 300, 410);
        SuperPrint(g_mainMenu.mainOptions, 3, 300, 440);
        SuperPrint(g_mainMenu.mainExit, 3, 300, 470);
        frmMain.renderTexture(300 - 20, 350 + (MenuCursor * 30), 16, 16, GFX.MCursor[0], 0, 0);
    }

    // Character select
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

        for(auto A = 1; A <= maxSaveSlots; A++)
        {
            if(SaveSlot[A] >= 0)
            {
                SuperPrint(fmt::format_ne("SLOT {0} ... {1}", A, SaveSlot[A]), 3, 300, 320 + (A * 30));
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
                SuperPrint(fmt::format_ne("SLOT {0} ... EMPTY", A), 3, 300, 320 + (A * 30));
            }
        }
        frmMain.renderTexture(300 - 20, 350 + (MenuCursor * 30), GFX.MCursor[0]);
    }

    // Options Menu
    else if(MenuMode == MENU_OPTIONS)
    {
        SuperPrint("PLAYER 1 CONTROLS", 3, 300, 350);
        SuperPrint("PLAYER 2 CONTROLS", 3, 300, 380);
#ifdef __ANDROID__
        SuperPrint("VIEW CREDITS", 3, 300, 410);
#else
        if(resChanged)
            SuperPrint("WINDOWED MODE", 3, 300, 410);
        else
            SuperPrint("FULLSCREEN MODE", 3, 300, 410);

        SuperPrint("VIEW CREDITS", 3, 300, 440);
#endif
        frmMain.renderTexture(300 - 20, 350 + (MenuCursor * 30),
                              GFX.MCursor[0].w, GFX.MCursor[0].h, GFX.MCursor[0], 0, 0);
    }

    // Player controls setup
    else if(MenuMode == MENU_INPUT_SETTINGS_P1 || MenuMode == MENU_INPUT_SETTINGS_P2)
    {
        if(useJoystick[MenuMode - MENU_INPUT_SETTINGS_BASE] == 0)
        {
            auto &ck = conKeyboard[MenuMode - MENU_INPUT_SETTINGS_BASE];
            SuperPrint("INPUT......KEYBOARD", 3, 300, 260 + menuFix);
            SuperPrint(fmt::format_ne("UP.........{0}", getKeyName(ck.Up)), 3, 300, 290 + menuFix);
            SuperPrint(fmt::format_ne("DOWN.......{0}", getKeyName(ck.Down)), 3, 300, 320 + menuFix);
            SuperPrint(fmt::format_ne("LEFT.......{0}", getKeyName(ck.Left)), 3, 300, 350 + menuFix);
            SuperPrint(fmt::format_ne("RIGHT......{0}", getKeyName(ck.Right)), 3, 300, 380 + menuFix);
            SuperPrint(fmt::format_ne("RUN........{0}", getKeyName(ck.Run)), 3, 300, 410 + menuFix);
            SuperPrint(fmt::format_ne("ALT RUN....{0}", getKeyName(ck.AltRun)), 3, 300, 440 + menuFix);
            SuperPrint(fmt::format_ne("JUMP.......{0}", getKeyName(ck.Jump)), 3, 300, 470 + menuFix);
            SuperPrint(fmt::format_ne("ALT JUMP...{0}", getKeyName(ck.AltJump)), 3, 300, 500 + menuFix);
            SuperPrint(fmt::format_ne("DROP ITEM..{0}", getKeyName(ck.Drop)), 3, 300, 530 + menuFix);
            SuperPrint(fmt::format_ne("PAUSE......{0}", getKeyName(ck.Start)), 3, 300, 560 + menuFix);
            SuperPrint("Reset to default", 3, 300, 590 + menuFix);
        }
        else
        {
            auto &cj = conJoystick[MenuMode - MENU_INPUT_SETTINGS_BASE];
            SuperPrint("INPUT......" + joyGetName(useJoystick[MenuMode - MENU_INPUT_SETTINGS_BASE] - 1), 3, 300, 260 + menuFix);
            SuperPrint(fmt::format_ne("UP.........{0}", getJoyKeyName(cj.isGameController, cj.Up)), 3, 300, 290 + menuFix);
            SuperPrint(fmt::format_ne("DOWN.......{0}", getJoyKeyName(cj.isGameController, cj.Down)), 3, 300, 320 + menuFix);
            SuperPrint(fmt::format_ne("LEFT.......{0}", getJoyKeyName(cj.isGameController, cj.Left)), 3, 300, 350 + menuFix);
            SuperPrint(fmt::format_ne("RIGHT......{0}", getJoyKeyName(cj.isGameController, cj.Right)), 3, 300, 380 + menuFix);
            SuperPrint(fmt::format_ne("RUN........{0}", getJoyKeyName(cj.isGameController, cj.Run)), 3, 300, 410 + menuFix);
            SuperPrint(fmt::format_ne("ALT RUN....{0}", getJoyKeyName(cj.isGameController, cj.AltRun)), 3, 300, 440 + menuFix);
            SuperPrint(fmt::format_ne("JUMP.......{0}", getJoyKeyName(cj.isGameController, cj.Jump)), 3, 300, 470 + menuFix);
            SuperPrint(fmt::format_ne("ALT JUMP...{0}", getJoyKeyName(cj.isGameController, cj.AltJump)), 3, 300, 500 + menuFix);
            SuperPrint(fmt::format_ne("DROP ITEM..{0}", getJoyKeyName(cj.isGameController, cj.Drop)), 3, 300, 530 + menuFix);
            SuperPrint(fmt::format_ne("PAUSE......{0}", getJoyKeyName(cj.isGameController, cj.Start)), 3, 300, 560 + menuFix);
            SuperPrint("Reset to default", 3, 300, 590 + menuFix);
        }

        frmMain.renderTexture(300 - 20, 260 + (MenuCursor * 30) + menuFix,
                              GFX.MCursor[0].w, GFX.MCursor[0].h, GFX.MCursor[0], 0, 0);
    }

    // Mouse cursor
    frmMain.renderTexture(int(MenuMouseX), int(MenuMouseY), GFX.ECursor[2]);
}
