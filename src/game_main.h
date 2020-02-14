#ifndef MAIN_H
#define MAIN_H

#include "globals.h"

bool FileExists(const std::string &fileName);

void SetupPhysics();

int GameMain(int argc, char**argv);
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
//! loads the level
void OpenLevel(std::string FilePath);
//! Reset everything to zero
void ClearLevel();

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

void SaveGame();

void LoadGame();

void PauseGame(int plr);

void InitControls();

void OpenConfig();

void SaveConfig();

void NPCyFix();

void CheatCode(char NewKey);
//! credit loop
void OutroLoop();

void SetupCredits();
//! checks for stars in warps the lead to another level
void FindStars();
// for settings up the game's credits
void AddCredit(std::string newCredit);
// creates a temp Location
Location_t newLoc(double X, double Y, double Width = 0, double Height = 0);
// calcualtes scores based on the multiplyer and how much the NPC is worth
void MoreScore(int addScore, Location_t Loc, int Multiplier = 0);
// sets up player frame offsets so they are displayed correctly on the screen
void SetupPlayerFrames();

void StartBattleMode();

std::string FixComma(std::string newStr);

#endif // MAIN_H
