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

#pragma once
#ifndef MENU_MAIN_H
#define MENU_MAIN_H

#include <string>
#include <memory>
#include <stdint.h>

#include "global_constants.h"
#include "std_picture.h"
#include "range_arr.hpp"

//Public Type SelectWorld
struct SelectWorld_t
{
//    WorldName As String
    std::string WorldName;
//    WorldPath As String
    // std::string WorldPath;
//    WorldFile As String (NEW: Full filepath, or <URL for remote assets)
    std::string WorldFilePath;

    // NEW: repo annotations
    std::string CreatorName;
    std::string Description;
    std::unique_ptr<StdPicture> screenshot;
    uint32_t ReleaseDate = 0; // reserved for now

//    blockChar(1 To numCharacters) As Boolean
    RangeArrI<bool, 1, numCharacters, false> blockChar;
// EXTRA:
    bool bugfixes_on_by_default = false;
    bool editable = false;
    bool disabled = false;
    bool probably_incompatible = false;

#ifdef THEXTECH_ENABLE_SDL_NET
    // content hash of packed episode
    uint32_t lz4_content_hash = 0;
#endif
//End Type
};

extern int NumSelectWorld;
extern int NumSelectWorldEditable; // NEW
extern int NumSelectBattle; // NEW
extern std::vector<SelectWorld_t> SelectWorld;
extern std::vector<SelectWorld_t> SelectBattle; // NEW

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
#ifdef THEXTECH_ENABLE_SDL_NET
    // NetPlay welcome menu
    MENU_NETPLAY = 5,
    // NetPlay episode select
    MENU_NETPLAY_WORLD_SELECT = 6,
#endif
    // Options menu
    // MENU_OPTIONS = 3, // FULLY UNUSED
    // New options menu
    MENU_NEW_OPTIONS = 8,
    // Editor mode episode select
    MENU_EDITOR = 9,

#if 0
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
#endif

    // New character select for main game
    MENU_CHARACTER_SELECT_NEW    = 1000,
    MENU_CHARACTER_SELECT_NEW_BM = 1001,

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

    MENU_SELECT_SLOT_1P_ADVMODE = 14,
    MENU_SELECT_SLOT_2P_ADVMODE = 24,
    MENU_SELECT_SLOT_ADVMODE_ADD = 4,

    MENU_SELECT_SLOT_END = 30,
};

struct MainMenuContent
{
    std::string introPressStart;

    std::string mainPlayEpisode;
    std::string main1PlayerGame;
    std::string mainMultiplayerGame;
    std::string mainBattleGame;
    std::string mainEditor;
    std::string mainOptions;
    std::string mainExit;

    std::string loading;

    std::string languageName;
    std::string pluralRules;

    std::string editorBattles;
    std::string editorNewWorld;
    std::string editorMakeFor;
    std::string editorErrorMissingResources;
    std::string editorPromptNewWorldName;

    std::string gameNoEpisodesToPlay;
    std::string gameNoBattleLevels;
    std::string gameBattleRandom;

    std::string warnEpCompat;

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
    std::string optionsRestartEngine;

    // ConnectScreen
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
    std::string controlsOptionBatteryStatus;

    std::string wordProfiles;
    std::string wordButtons;
    std::string controlsReallyDeleteProfile;
    std::string controlsNewProfile;

    // General purpose
    std::string wordYes;
    std::string wordNo;
    std::string wordOkay;
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
    std::string abbrevMilliseconds;

#ifdef THEXTECH_ENABLE_SDL_NET
    // NetPlay
    std::string mainNetplay;
    std::string netplayRoomKey;
    std::string netplayJoinRoom;
    std::string netplayCreateRoom;
    std::string netplayLeaveRoom;
    std::string netplayServer;
    std::string netplayNickname;
#endif
};

extern MainMenuContent g_mainMenu;

extern void initMainMenu();

extern bool mainMenuUpdate();
extern void mainMenuDraw();

extern int mainMenuPlaystyle();

void GetMenuPos(int* MenuX, int* MenuY);

#endif // MENU_MAIN_H
