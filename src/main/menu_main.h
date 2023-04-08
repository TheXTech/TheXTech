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

#pragma once
#ifndef MENU_MAIN_H
#define MENU_MAIN_H

#include <string>
#include "global_constants.h"
#include "range_arr.hpp"

struct SelectWorld_t;
extern int NumSelectWorld;
extern int NumSelectWorldEditable; // NEW
extern int NumSelectBattle; // NEW
extern std::vector<SelectWorld_t> SelectWorld;
extern std::vector<SelectWorld_t> SelectWorldEditable; // NEW
extern std::vector<SelectWorld_t> SelectBattle; // NEW

// Menu modes
enum
{
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
    // Editor mode episode select
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
    // New character select for main game
    MENU_CHARACTER_SELECT_NEW    = 1000,
    MENU_CHARACTER_SELECT_NEW_BM = 1001,

    MENU_CHARACTER_SELECT_BASE  = MENU_CHARACTER_SELECT_1P,
    MENU_CHARACTER_SELECT_BASE_END = MENU_CHARACTER_SELECT_BM_S2,

    MENU_INPUT_SETTINGS = 30,

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
    std::string mainStartGame;
    std::string main1PlayerGame;
    std::string mainMultiplayerGame;
    std::string mainBattleGame;
    std::string mainEditor;
    std::string mainOptions;
    std::string mainExit;

    std::string loading;

    std::string languageName;

    std::string editorNewWorld;
    std::string editorErrorResolution;
    std::string editorErrorMissingResources;
    std::string editorPromptNewWorldName;

    std::string gameNoEpisodesToPlay;
    std::string gameNoBattleLevels;
    std::string gameBattleRandom;

    std::string gameSlotContinue;
    std::string gameSlotNew;

    std::string gameCopySave;
    std::string gameEraseSave;

    std::string gameSourceSlot;
    std::string gameTargetSlot;
    std::string gameEraseSlot;

    std::string phraseScore;
    std::string phraseTime;

    // Battle
    std::string errorBattleNoLevels;

    // Options
    std::string optionsModeFullScreen;
    std::string optionsModeWindowed;
    std::string optionsViewCredits;

    // ConnectScreen
    std::string connectCharSelTitle;
    std::string connectStartGame;
    std::string selectCharacter;

    // Controls menus
    std::string controlsTitle;
    std::string controlsConnected;
    std::string controlsDeleteKey;
    std::string controlsDeviceTypes;
    std::string controlsInUse;
    std::string controlsNotInUse;

    std::string controlsActivateProfile;
    std::string controlsRenameProfile;
    std::string controlsDeleteProfile;
    std::string controlsPlayerControls;
    std::string controlsCursorControls;
    std::string controlsEditorControls;
    std::string controlsHotkeys;

    std::string controlsOptionRumble;
    std::string controlsOptionGroundPoundButton;
    std::string controlsOptionBatteryStatus;

    std::string wordProfiles;
    std::string wordButtons;
    std::string controlsReallyDeleteProfile;
    std::string controlsNewProfile;

    // General purpose
    std::string wordYes;
    std::string wordNo;
    std::string caseNone;
    std::string wordOn;
    std::string wordOff;
    std::string wordShow;
    std::string wordHide;
    std::string wordPlayer;
    std::string wordProfile;
    std::string wordBack;
    std::string wordResume;
    std::string wordWaiting;
};

extern MainMenuContent g_mainMenu;

extern void initMainMenu();

extern bool mainMenuUpdate();
extern void mainMenuDraw();

void GetMenuPos(int* MenuX, int* MenuY);

#endif // MENU_MAIN_H
