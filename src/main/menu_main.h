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

#ifndef MENU_MAIN_H
#define MENU_MAIN_H

#include <string>
#include "global_constants.h"
#include "range_arr.hpp"

// Menu modes
enum
{
    // Main intro
    MENU_INTRO = -1,
    // Main menu
    MENU_MAIN = 0,
    // One player game episode select
    MENU_1PLAYER_GAME = 1,
    // Two player game episode select
    MENU_2PLAYER_GAME = 2,
    // Battle mode level select
    MENU_BATTLE_MODE = 4,
    // Options menu
    MENU_OPTIONS = 3,
    // Editor menu
    MENU_EDITOR = 9,

    // Character select for single player game
    MENU_CHARACTER_SELECT_1P    = 100,
    // Character select for two player game, step 1
    MENU_CHARACTER_SELECT_2P_S1 = 200,
    // Character select for two player game, step 2
    MENU_CHARACTER_SELECT_2P_S2 = 300,
    // Character select for battle game, step 1
    MENU_CHARACTER_SELECT_BM_S1 = 400,
    // Character select for battle game, step 2
    MENU_CHARACTER_SELECT_BM_S2 = 500,

    MENU_CHARACTER_SELECT_BASE  = MENU_CHARACTER_SELECT_1P,
    MENU_CHARACTER_SELECT_BASE_END = MENU_CHARACTER_SELECT_BM_S2,

    MENU_INPUT_SETTINGS_BASE = 30,
    // Set up controls for player 1
    MENU_INPUT_SETTINGS_P1 = 31,
    // Set up controls for player 2
    MENU_INPUT_SETTINGS_P2 = 32,

    MENU_SELECT_SLOT_BASE = 10,
    // Select game slot for single-player game
    MENU_SELECT_SLOT_1P = 10,
    // Select game slot for two player game
    MENU_SELECT_SLOT_2P = 20,

    MENU_SELECT_SLOT_1P_COPY_S1 = 11,
    MENU_SELECT_SLOT_2P_COPY_S1 = 21,
    MENU_SELECT_SLOT_COPY_S1_ADD = 1,
    MENU_SELECT_SLOT_1P_COPY_S2 = 12,
    MENU_SELECT_SLOT_2P_COPY_S2 = 22,
    MENU_SELECT_SLOT_COPY_S2_ADD = 2,

    MENU_SELECT_SLOT_1P_DELETE = 13,
    MENU_SELECT_SLOT_2P_DELETE = 23,
    MENU_SELECT_SLOT_DELETE_ADD = 3,
};

struct MainMenuContent
{
    std::string introPressStart;

    std::string main1PlayerGame;
    std::string main2PlayerGame;
    std::string mainBattleGame;
    std::string mainOptions;
    std::string mainExit;

    std::string loading;

    RangeArr<std::string, 1, numCharacters> selectPlayer;

};

extern MainMenuContent g_mainMenu;

extern void initMainMenu();

extern bool mainMenuUpdate();
extern void mainMenuDraw();

#endif // MENU_MAIN_H
