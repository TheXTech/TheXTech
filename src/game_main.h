/*
 * TheXTech - A platform game engine ported from old source code for VB6
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

#ifndef MAIN_H
#define MAIN_H

#include "globals.h"
#include "cmd_line_setup.h"

void SetupPhysics();

void initAll();

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
//! loads the world
void OpenWorld(std::string FilePath);
//! Loop for world select
void WorldLoop();

void LevelPath(int Lvl, int Direction, bool Skp = false);

void PathPath(int Pth, bool Skp = false);

void PathWait();

void ClearWorld();

void FindWorlds();

void FindLevels();

void FindSaves();

extern std::string makeGameSavePath(std::string episode, std::string world, std::string saveFile);

void SaveGame();

void LoadGame();
//! Removes gamesave file and restores initial state of all level objects
void ClearGame(bool punnish = false);

void PauseGame(int plr);

void InitControls();

void OpenConfig_preSetup();

void OpenConfig();

void SaveConfig();

void NPCyFix();

void CheatCode(char NewKey);
//! credit loop
void OutroLoop();

void SetupCredits();

// FindStars() moved into main/level_file.h

// for settings up the game's credits
void AddCredit(std::string newCredit);
// calcualtes scores based on the multiplyer and how much the NPC is worth
void MoreScore(int addScore, Location_t Loc);
void MoreScore(int addScore, Location_t Loc, int &Multiplier);
// sets up player frame offsets so they are displayed correctly on the screen
void SetupPlayerFrames();

void StartBattleMode();

// std::string FixComma(std::string newStr); // USELESS

#endif // MAIN_H
