/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2024 Vitaly Novichkov <admin@wohlnet.ru>
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
#ifndef MAIN_H
#define MAIN_H

#include "globals.h"
#include "cmd_line_setup.h"

enum class PauseCode
{
    None,
    PauseScreen,
    Message,
    DropAdd,
    TextEntry,
    Prompt,
};

//Public GamePaused As Boolean 'true if the game is paused
extern PauseCode GamePaused;

void SetupPhysics();

void initAll();

//! NEW: (re)load all file-based assets from the current asset pack directory
void MainLoadAll(bool reload);

int GameMain(const CmdLineSetup_t &setup);
//! Set up object sizes and frame offsets for blocks/npcs/effects
void SetupVars();
//! The loop for the game
void GameLoop();
//! The loop for the menu
void MenuLoop();
//! The loop for the level editor [DUMMY]
void EditorLoop();
//! Cleans up the buffer before ending the program
void KillIt();

// OpenLevel() and ClearLevel() moved into main/level_file.h

void NextLevel();
//! macros mainly used for end of level stuffs. takes over the players controls
void UpdateMacro();

// OpenWorld, FindWldStars, and ClearWorld moved into main/world_file.h

//! Loop for world select
void WorldLoop();

void LevelPath(const WorldLevel_t &Lvl, int Direction, bool Skp = false);

void PlayerPath(WorldPlayer_t &p);

void PathPath(WorldPath_t &Pth, bool Skp = false);

void PathWait();

void FindWorlds();

void LoadSingleWorld(const std::string wPath);

void FindLevels();

void FindSaves();

extern std::string makeGameSavePath(std::string episode, std::string world, std::string saveFile);

void SaveGame();

void LoadGame();
//! Removes gamesave file and restores initial state of all level objects
void ClearGame(bool punnish = false);
void DeleteSave(int world, int save);
void CopySave(int world, int src, int dst);

int PauseGame(PauseCode code, int plr = 0);

void OpenConfig_preSetup();

// reload recent episodes from config (used when asset pack changes)
void ConfigReloadRecentEpisodes();

void OpenConfig();

void SaveConfig();

void NPCyFix();

// void CheatCode(char NewKey);// Moved into "main/cheat_code.h"

//! credit loop
void OutroLoop();

void SetupCredits();

// FindStars() moved into main/level_file.h

// for settings up the game's credits
void AddCredit(const std::string& newCredit);
// calcualtes scores based on the multiplyer and how much the NPC is worth
void MoreScore(int addScore, const Location_t &Loc);
void MoreScore(int addScore, const Location_t &Loc, vbint_t &Multiplier);

// NEW: convenience hook for when a player has collected 100 coins
void Got100Coins();

// sets up player frame offsets so they are displayed correctly on the screen
void SetupPlayerFrames();

void StartEpisode();
void StartBattleMode();

// std::string FixComma(std::string newStr); // USELESS

#endif // MAIN_H
