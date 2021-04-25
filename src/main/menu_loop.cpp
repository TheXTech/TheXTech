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

#include <DirManager/dirman.h>
#include <AppPath/app_path.h>
#include <Utils/files.h>
#include <PGE_File_Formats/file_formats.h>
#include <pge_delay.h>
#include <fmt_format_ne.h>

#include "../globals.h"
#include "../game_main.h"
#include "../sound.h"
#include "../control/joystick.h"
#include "../effect.h"
#include "../graphics.h"
#include "../blocks.h"
#include "../npc.h"
#include "../layers.h"
#include "../player.h"
#include "../collision.h"
#include "level_file.h"
#include "menu_main.h"
#include "speedrunner.h"

#include "../pseudo_vb.h"

static int ScrollDelay = 0;

void MenuLoop()
{
    int B;
    Location_t tempLocation;
    bool tempBool;
    int menuLen;
    Player_t blankPlayer;

    UpdateControls();

    SingleCoop = 0;


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
                    FindWorlds();
                    MenuCursor = 0;
                }
                else if(MenuCursor == 1)
                {
                    PlaySoundMenu(SFX_Do);
                    MenuMode = MENU_2PLAYER_GAME;
                    FindWorlds();
                    MenuCursor = 0;
                }
                else if(MenuCursor == 2)
                {
                    PlaySoundMenu(SFX_Do);
                    MenuMode = MENU_BATTLE_MODE;
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
                        return;
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
                    return;
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

    if(CheckLiving() == 0)
    {
        ShowLayer("Destroyed Blocks");

        For(A, 1, numNPCs)
        {
            if(NPC[A].DefaultType == 0)
            {
                if(NPC[A].TimeLeft > 10) NPC[A].TimeLeft = 10;
            }
        }
    }

    For(A, 1, numPlayers)
    {
        Player_t &p = Player[A];
        if(p.TimeToLive > 0)
        {
            p.TimeToLive = 0;
            p.Dead = true;
        }

        p.Controls.Down = false;
        p.Controls.Drop = false;
        p.Controls.Right = true;
        p.Controls.Left = false;
        p.Controls.Run = true;
        p.Controls.Up = false;
        p.Controls.AltRun = false;
        p.Controls.AltJump = false;

        if(p.Jump == 0 || p.Location.Y < level[0].Y + 200)
            p.Controls.Jump = false;

        if(p.Location.SpeedX < 0.5)
        {
            p.Controls.Jump = true;
            if(p.Slope > 0 || p.StandingOnNPC > 0 || p.Location.SpeedY == 0.0)
                p.CanJump = true;
        }

        if(p.HoldingNPC ==0)
        {
            if((p.State ==3 || p.State == 6 || p.State == 7) && (dRand() * 100.0) > 90.0)
            {
                if(p.FireBallCD == 0 && !p.RunRelease)
                    p.Controls.Run = false;
            }

            if((p.State == 4 || p.State == 5) && p.TailCount == 0 && !p.RunRelease)
            {
                tempLocation.Width = 24;
                tempLocation.Height = 20;
                tempLocation.Y = p.Location.Y + p.Location.Height - 22;
                tempLocation.X = p.Location.X + p.Location.Width;

                For(B, 1, numNPCs)
                {
                    if(NPC[B].Active && !NPCIsABonus[NPC[B].Type] &&
                       !NPCWontHurt[NPC[B].Type] && NPC[B].HoldingPlayer == 0)
                    {
                        if(CheckCollision(tempLocation, NPC[B].Location))
                            p.Controls.Run = false;
                    }
                }
            }

            if(p.StandingOnNPC > 0)
            {
                if(NPCGrabFromTop[NPC[p.StandingOnNPC].Type])
                {
                    p.Controls.Down = true;
                    p.Controls.Run = true;
                    p.RunRelease = true;
                }
            }
        }

        if(p.Character == 5)
        {
            if(p.FireBallCD == 0 && !p.RunRelease)
            {
                tempLocation.Width = 38 + p.Location.SpeedX * 0.5;
                tempLocation.Height = p.Location.Height - 8;
                tempLocation.Y = p.Location.Y + 4;
                tempLocation.X = p.Location.X + p.Location.Width;

                For(B, 1, numNPCs)
                {
                    if(NPC[B].Active && !NPCIsABonus[NPC[B].Type] &&
                      !NPCWontHurt[NPC[B].Type] && NPC[B].HoldingPlayer == 0)
                    {
                        if(CheckCollision(tempLocation, NPC[B].Location))
                        {
                            p.RunRelease = true;
                            if(NPC[B].Location.Y > p.Location.Y + p.Location.Height / 2)
                                p.Controls.Down = true;
                            break;
                        }
                    }
                }
            }

            if(p.Slope == 0 && p.StandingOnNPC == 0)
            {
                if(p.Location.SpeedY < 0)
                {
                    tempLocation.Width = 200;
                    tempLocation.Height = p.Location.Y - level[0].Y + p.Location.Height;
                    tempLocation.Y = level[0].Y;
                    tempLocation.X = p.Location.X;

                    For(B, 1, numNPCs)
                    {
                        if(NPC[B].Active && !NPCIsABonus[NPC[B].Type] &&
                           !NPCWontHurt[NPC[B].Type] && NPC[B].HoldingPlayer == 0)
                        {
                            if(CheckCollision(tempLocation, NPC[B].Location))
                            {
                                p.Controls.Up = true;
                                break;
                            }
                        }
                    }

                }
                else if(p.Location.SpeedY > 0)
                {
                    tempLocation.Width = 200;
                    tempLocation.Height = level[0].Height - p.Location.Y;
                    tempLocation.Y = p.Location.Y;
                    tempLocation.X = p.Location.X;

                    For(B, 1, numNPCs)
                    {
                        if(NPC[B].Active && !NPCIsABonus[NPC[B].Type] &&
                           !NPCWontHurt[NPC[B].Type] && NPC[B].HoldingPlayer == 0)
                        {
                            if(CheckCollision(tempLocation, NPC[B].Location))
                            {
                                p.Controls.Down = true;
                                break;
                            }
                        }
                    }
                }
            }
        }

        if(p.Location.X < -vScreenX[1] - p.Location.Width && !(-vScreenX[1] <= level[0].X))
            p.Dead = true;

        if(p.Location.X > -vScreenX[1] + 1000)
            p.Dead = true;

        if(p.Location.X > -vScreenX[1] + 600 && -vScreenX[1] + 850 < level[0].Width)
            p.Controls.Run = false;

        if(-vScreenX[1] <= level[0].X && (p.Dead || p.TimeToLive > 0))
        {
            p.ForceHold = 65;
            p.State = (iRand() % 6) + 2;
            p.CanFly = false;
            p.CanFly2 = false;
            p.TailCount = 0;
            p.Dead = false;
            p.TimeToLive = 0;
            p.Character = (iRand() % 5) + 1;

            if(A >= 1 && A <= 5)
                p.Character = A;

            p.HeldBonus = 0;
            p.Section = 0;
            p.Mount = 0;
            p.MountType = 0;
            p.YoshiBlue = false;
            p.YoshiRed = false;
            p.YoshiYellow = false;
            p.YoshiNPC = 0;
            p.Wet = 0;
            p.WetFrame = false;
            p.YoshiPlayer = 0;
            p.Bumped = false;
            p.Bumped2 = 0;
            p.Direction = 1;
            p.Dismount = 0;
            p.Effect = 0;
            p.Effect2 = 0;
            p.FireBallCD = 0;
            p.ForceHold = 0;
            p.Warp = 0;
            p.WarpCD = 0;
            p.GroundPound = false;
            p.Immune = 0;
            p.Frame = 0;
            p.Slope = 0;
            p.Slide = false;
            p.SpinJump = false;
            p.FrameCount = 0;
            p.TailCount = 0;
            p.Duck = false;
            p.GroundPound = false;
            p.Hearts = 3;

            PlayerFrame(A);

            p.Location.Height = Physics.PlayerHeight[p.Character][p.State];
            p.Location.Width = Physics.PlayerWidth[p.Character][p.State];
            p.Location.X = level[p.Section].X - A * 48;
            p.Location.SpeedX = double(Physics.PlayerRunSpeed);
            p.Location.Y = level[p.Section].Height - p.Location.Height - 33;

            do
            {
                tempBool = true;
                For(B, 1, numBlock)
                {
                    if(CheckCollision(p.Location, Block[B].Location))
                    {
                        p.Location.Y = Block[B].Location.Y - p.Location.Height - 0.1;
                        tempBool = false;
                    }
                }
            } while(!tempBool);

            if(!UnderWater[p.Section])
            {
                if((iRand() % 25) + 1 == 25)
                {
                    p.Mount = 1;
                    p.MountType = (iRand() % 3) + 1;
                    if(p.State == 1)
                    {
                        p.Location.Height = Physics.PlayerHeight[1][2];
                        p.Location.Y = p.Location.Y - Physics.PlayerHeight[1][2] + Physics.PlayerHeight[p.Character][1];
                    }
                }
            }

            if(p.Mount == 0 && p.Character <= 2)
            {
                if((iRand() % 15) + 1 == 15)
                {
                    p.Mount = 3;
                    p.MountType = (iRand() % 7) + 1;
                    p.Location.Y = p.Location.Y + p.Location.Height;
                    p.Location.Height = Physics.PlayerHeight[2][2];
                    p.Location.Y = p.Location.Y - p.Location.Height - 0.01;
                }
            }

            p.CanFly = false;
            p.CanFly2 = false;
            p.RunCount = 0;

            if(p.Mount == 0 && p.Character != 5)
            {
                numNPCs += 1;
                p.HoldingNPC = numNPCs;
                p.ForceHold = 120;

                {
                    NPC_t &n = NPC[numNPCs];
                    do
                    {
                        do
                        {
                            n.Type = (iRand() % 286) + 1;
                        } while(n.Type == 11 || n.Type == 16 || n.Type == 18 || n.Type == 15 ||
                                n.Type == 21 || n.Type == 12 || n.Type == 13 || n.Type == 30 ||
                                n.Type == 17 || n.Type == 31 || n.Type == 32 ||
                                (n.Type >= 37 && n.Type <= 44) || n.Type == 46 || n.Type == 47 ||
                                n.Type == 50 || (n.Type >= 56 && n.Type <= 70) || n.Type == 8 ||
                                n.Type == 74 || n.Type == 51 || n.Type == 52 || n.Type == 75 ||
                                n.Type == 34 || NPCIsToad[n.Type] || NPCIsAnExit[n.Type] ||
                                NPCIsYoshi[n.Type] || (n.Type >= 78 && n.Type <= 87) ||
                                n.Type == 91 || n.Type == 93 || (n.Type >= 104 && n.Type <= 108) ||
                                n.Type == 125 || n.Type == 133 || (n.Type >= 148 && n.Type <= 151) ||
                                n.Type == 159 || n.Type == 160 || n.Type == 164 || n.Type == 168 ||
                                (n.Type >= 154 && n.Type <= 157) || n.Type == 159 || n.Type == 160 ||
                                n.Type == 164 || n.Type == 165 || n.Type == 171 || n.Type == 178 ||
                                n.Type == 197 || n.Type == 180 || n.Type == 181 || n.Type == 190 ||
                                n.Type == 192 || n.Type == 196 || n.Type == 197 ||
                                (UnderWater[0] == true && NPCIsBoot[n.Type] == true) ||
                                (n.Type >= 198 && n.Type <= 228) || n.Type == 234);

                    } while(n.Type == 235 || n.Type == 231 || n.Type == 179 || n.Type == 49 ||
                            n.Type == 237 || n.Type == 238 || n.Type == 239 || n.Type == 240 ||
                            n.Type == 245 || n.Type == 246 || n.Type == 248 || n.Type == 254 ||
                            n.Type == 255 || n.Type == 256 || n.Type == 257 || n.Type == 259 ||
                            n.Type == 260 || n.Type == 262 || n.Type == 263 || n.Type == 265 ||
                            n.Type == 266 || (n.Type >= 267 && n.Type <= 272) ||
                            n.Type == 275 || n.Type == 276 ||
                            (n.Type >= 280 && n.Type <= 284) || n.Type == 241);

                    n.Active = true;
                    n.HoldingPlayer = A;
                    n.Location.Height = NPCHeight[n.Type];
                    n.Location.Width = NPCWidth[n.Type];
                    n.Location.Y = Player[A].Location.Y;  // level[n.Section].Height + 1000
                    n.Location.X = Player[A].Location.X; // level[n.Section].X + 1000
                    n.TimeLeft = 100;
                    n.Section = Player[A].Section;
                }
            }

        }
        else if(p.Location.X > level[p.Section].Width + 64)
        {
            p.Dead = true;
        }

        if(p.WetFrame)
        {
            if(p.Location.SpeedY == 0.0 || p.Slope > 0)
                p.CanJump = true;
            if((dRand() * 100) > 98.0 || p.Location.SpeedY == 0.0 || p.Slope > 0)
                p.Controls.Jump = true;
        }

        if((dRand() * 100) > 95.0 && Player[A].HoldingNPC == 0 && Player[A].Slide == false && Player[A].CanAltJump == true && Player[A].Mount == 0)
            Player[A].Controls.AltJump = true;
        if(dRand() * 1000 >= 999 && Player[A].CanFly2 == false)
            Player[A].Controls.Run = false;
        if(Player[A].Mount == 3 && dRand() * 100 >= 98 && Player[A].RunRelease == false)
            Player[A].Controls.Run = false;
        if(NPC[Player[A].HoldingNPC].Type == 22 || NPC[Player[A].HoldingNPC].Type == 49)
            Player[A].Controls.Run = true;
        if(Player[A].Slide == true && Player[A].CanJump == true)
        {
            if(Player[A].Location.SpeedX > -2 && Player[A].Location.SpeedX < 2)
                Player[A].Controls.Jump = true;
        }
        if(Player[A].CanFly == false && Player[A].CanFly2 == false && (Player[A].State == 4 || Player[A].State == 5) && Player[A].Slide == false)
            Player[A].Controls.Jump = true;
        if(Player[A].Quicksand > 0)
        {
            Player[A].CanJump = true;
            Player[A].Controls.Jump = true;
        }
        if(Player[A].FloatTime > 0 || (Player[A].CanFloat == true && Player[A].FloatRelease == true && Player[A].Jump == 0 && Player[A].Location.SpeedY > 0 && (dRand() * 100) > 95.0))
            Player[A].Controls.Jump = true;
        if(NPC[Player[A].HoldingNPC].Type == 13 && (dRand() * 100) > 95.0)
        {
            Player[A].Controls.Run = false;
            if(iRand() % 2 == 1)
                Player[A].Controls.Up = true;
            if(iRand() % 2 == 1)
                Player[A].Controls.Right = false;
        }

        if(Player[A].Slide == false && (Player[A].Slope > 0 || Player[A].StandingOnNPC > 0 || Player[A].Location.SpeedY == 0.0))
        {
            tempLocation = Player[A].Location;
            tempLocation.Width = 95;
            tempLocation.Height = tempLocation.Height - 1;
            for(auto B = 1; B <= numBlock; B++)
            {
                if(BlockSlope[Block[B].Type] == 0 && BlockIsSizable[Block[B].Type] == false && BlockOnlyHitspot1[Block[B].Type] == false && Block[B].Hidden == false)
                {
                    if(CheckCollision(Block[B].Location, tempLocation) == true)
                    {
                        Player[A].CanJump = true;
                        Player[A].SpinJump = false;
                        Player[A].Controls.Jump = true;
                        break;
                    }
                }
            }
        }
        if(Player[A].Slope == 0 && Player[A].Slide == false && Player[A].StandingOnNPC == 0 && (Player[A].Slope > 0 || Player[A].Location.SpeedY == 0.0))
        {
            tempBool = false;
            tempLocation = Player[A].Location;
            tempLocation.Width = 16;
            tempLocation.Height = 16;
            tempLocation.X = Player[A].Location.X + Player[A].Location.Width;
            tempLocation.Y = Player[A].Location.Y + Player[A].Location.Height;
            for(auto B = 1; B <= numBlock; B++)
            {
                if((BlockIsSizable[Block[B].Type] == false || Block[B].Location.Y > Player[A].Location.Y + Player[A].Location.Height - 1) && BlockOnlyHitspot1[Block[B].Type] == false && Block[B].Hidden == false)
                {
                    if(CheckCollision(Block[B].Location, tempLocation) == true)
                    {
                        tempBool = true;
                        break;
                    }
                }
            }
            if(tempBool == false)
            {
                Player[A].CanJump = true;
                Player[A].SpinJump = false;
                Player[A].Controls.Jump = true;
            }
        }
        if(Player[A].Character == 5 && Player[A].Controls.Jump == true)
        {
            Player[A].Controls.AltJump = true;
            // .Controls.Jump = False
        }
    }

    if(LevelMacro > LEVELMACRO_OFF)
        UpdateMacro();

    UpdateLayers();
    UpdateNPCs();
    UpdateBlocks();
    UpdateEffects();
    UpdatePlayer();
    UpdateGraphics();
    UpdateSound();
    UpdateEvents();

    if(MenuMouseDown)
    {
        if(dRand() * 100 > 40.0)
        {
            NewEffect(80, newLoc(MenuMouseX - vScreenX[1], MenuMouseY - vScreenY[1]));
            Effect[numEffects].Location.SpeedX = dRand() * 4 - 2;
            Effect[numEffects].Location.SpeedY = dRand() * 4 - 2;
        }

        For(A, 1, numNPCs)
        {
            if(NPC[A].Active)
            {
                if(CheckCollision(newLoc(MenuMouseX - vScreenX[1], MenuMouseY - vScreenY[1]), NPC[A].Location))
                {
                    if(!NPCIsACoin[NPC[A].Type])
                    {
                        NPC[0] = NPC[A];
                        NPC[0].Location.X = MenuMouseX - vScreenX[1];
                        NPC[0].Location.Y = MenuMouseY - vScreenY[1];
                        NPCHit(A, 3, 0);
                    }
                    else
                    {
                        NewEffect(78, NPC[A].Location);
                        NPC[A].Killed = 9;
                    }
                }
            }
        }

        For(A, 1, numBlock)
        {
            if(!Block[A].Hidden)
            {
                if(CheckCollision(newLoc(MenuMouseX - vScreenX[1], MenuMouseY - vScreenY[1]), Block[A].Location))
                {
                    BlockHit(A);
                    BlockHitHard(A);
                }
            }
        }
    }

    MenuMouseMove = false;
    MenuMouseClick = false;

    if(MenuMouseDown)
        MenuMouseRelease = false;
    else
        MenuMouseRelease = true;

    MenuMouseBack = false;
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
        }
    }

    NumSelectWorld = (SelectWorld.size() - 1);
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
        }
    }

    NumSelectWorld = (SelectWorld.size() - 1);
}

void FindSaves()
{
    std::string newInput = "";

    std::string episode = SelectWorld[selWorld].WorldPath;
    GamesaveData f;
    for(auto A = 1; A <= maxSaveSlots; A++)
    {
        SaveSlot[A] = -1;
        SaveStars[A] = 0;

        std::string saveFile = makeGameSavePath(episode,
                                                SelectWorld[selWorld].WorldFile,
                                                fmt::format_ne("save{0}.savx", A));
        std::string saveFileOld = episode + fmt::format_ne("save{0}.savx", A);
        std::string saveFileAncient = episode + fmt::format_ne("save{0}.sav", A);

        if((Files::fileExists(saveFile) && FileFormats::ReadExtendedSaveFileF(saveFile, f)) ||
           (Files::fileExists(saveFileOld) && FileFormats::ReadExtendedSaveFileF(saveFileOld, f)) ||
           (Files::fileExists(saveFileAncient) && FileFormats::ReadSMBX64SavFileF(saveFileAncient, f)))
        {
            int curActive = 0;
            int maxActive = 0;

            // "game beat flag"
            maxActive++;
            if(f.gameCompleted)
                curActive++;

            // How much paths open
            maxActive += f.visiblePaths.size();
            for(auto &p : f.visiblePaths)
            {
                if(p.second)
                    curActive++;
            }

            // How much levels opened
            maxActive += f.visibleLevels.size();
            for(auto &p : f.visibleLevels)
            {
                if(p.second)
                    curActive++;
            }

            // How many stars collected
            maxActive += (int(f.totalStars) * 4);
            SaveStars[A] = int(f.gottenStars.size());

            curActive += (SaveStars[A] * 4);

            if(maxActive > 0)
                SaveSlot[A] = int((float(curActive) / float(maxActive)) * 100);
            else
                SaveSlot[A] = 100;
        }
    }
}
