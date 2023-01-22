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

#include "sdl_proxy/sdl_stdinc.h"

#include "core/events.h"
#include "globals.h"
#include <fmt_format_ne.h>
#include <cmath>
#include <cfenv>

#if defined(VITA) || defined(__3DS__)
#define USE_CUSTOM_TONEAREST
#endif

#ifdef USE_CUSTOM_TONEAREST
#include <pge_tonearest.h>
#endif

bool GameIsActive = false;
std::string AppPath;

int numSavedEvents = 0;
RangeArr<std::string, 1, MaxSavedEvents> SavedEvents;
RangeArrI<bool, 1, 4, false> BlockSwitch;
RangeArrI<bool, 2, 7, false> PowerUpUnlock;
long myBackBuffer = 0;
long myBufferBMP = 0;
int AllCharBlock = 0;
bool StartMenu = false;
int BlockFlash = 0;
bool ScrollRelease = false;
bool TakeScreen = false;
bool ShowOnScreenHUD = true;
bool NewFontRender = true;
std::string LB;
std::string EoT;

std::string Checkpoint;
std::vector<Checkpoint_t> CheckpointsList;
bool MagicHand = false;
RangeArr<Player_t, 1, 2> testPlayer;
bool ClearBuffer = false;
int numLocked = 0;
bool resChanged = false;

// moved into game_loop.cpp
// PauseCode GamePaused = PauseCode::None;

std::string MessageText;
UTF8CharMap_t MessageTextMap;

// int NumSelectWorld  = 0;
// std::vector<SelectWorld_t> SelectWorld;

std::string g_recentWorld1p;
std::string g_recentWorld2p;
std::string g_recentWorldEditor;
bool ShowFPS = false;
double PrintFPS = 0.0;
RangeArr<vScreen_t, 0, 2> vScreen;
int ScreenType = 0;
int DScreenType = 0;
bool LevelEditor = false;
bool WorldEditor = false;
RangeArr<PlayerStart_t, 1, 2> PlayerStart;
RangeArrI<bool, 0, 20, false> blockCharacter;
RangeArrI<int, 0, maxPlayers, 0> OwedMount;
RangeArrI<int, 0, maxPlayers, 0> OwedMountType;
bool AutoUseModern = false;
RangeArr<float, 0, maxSections> AutoX;
RangeArr<float, 0, maxSections> AutoY;
int numStars = 0;
RangeArr<Water_t, 0, maxWater> Water;
int numWater = 0;
RangeArr<Star_t, 1, maxStarsNum> Star;
std::string GoToLevel;
bool GoToLevelNoGameThing = false;
std::string StartLevel;
bool NoMap = false;
bool RestartLevel = false;
int WorldStarsShowPolicy = -1;
float LevelChop[maxSections + 1];
// RangeArr<int, -FLBlocks, FLBlocks> FirstBlock;
// RangeArr<int, -FLBlocks, FLBlocks> LastBlock;
int MidBackground = 1;
int LastBackground = 1;
int iBlocks = 0;
RangeArrI<int, 0, maxBlocks, 0> iBlock;
int numTiles = 0;
int numScenes = 0;
RangeArr<std::string, 0, maxSections> CustomMusic;
int numSections = 0;
RangeArr<Location_t, 0, maxSections> level;
RangeArrI<bool, 0, maxSections, false> LevelWrap;
RangeArrI<bool, 0, maxSections, false> LevelVWrap;
RangeArrI<bool, 0, maxSections, false> OffScreenExit;
RangeArrI<int, 0, maxSections, 0> bgMusic;
RangeArrI<int, 0, maxSections, 0> bgMusicREAL;
RangeArrI<int, 0, maxSections, 0> Background2REAL;
RangeArr<Location_t, 0, maxSections> LevelREAL;
int curMusic = 0;
RangeArrI<long, 0, maxSections, 0> bgColor;
RangeArrI<int, 0, maxSections, 0> Background2;
RangeArr<WorldPath_t, 1, maxWorldPaths> WorldPath;
int numWorldPaths = 0;
int numWarps = 0;
RangeArr<Warp_t, 1, maxWarps> Warp;
RangeArr<Tile_t, 1, maxTiles> Tile;
RangeArr<Scene_t, 1, maxScenes> Scene;
RangeArr<CreditLine_t, 1, maxCreditsLines> Credit;
double CreditOffsetY = 0.0;
double CreditTotalHeight = 0.0;
int numCredits = 0;
int numBlock = 0;
int numBackground = 0;
int numNPCs = 0;
int numEffects = 0;
int numPlayers = 0;
int numWorldLevels = 0;
RangeArr<WorldMusic_t, 1, maxWorldMusic> WorldMusic;
int numWorldMusic = 0;
RangeArr<WorldLevel_t, 1, maxWorldLevels> WorldLevel;
RangeArr<Background_t, 1, (maxBackgrounds + maxWarps)> Background;
RangeArr<Effect_t, 1, maxEffects> Effect;

RangeArr<NPC_t, -128, maxNPCs> NPC;
RangeArr<Block_t, 0, maxBlocks> Block;

RangeArr<Player_t, 0, maxPlayers> Player;
RangeArrI<int, 0, maxPlayerFrames, 0> MarioFrameX;
RangeArrI<int, 0, maxPlayerFrames, 0> MarioFrameY;
RangeArrI<int, 0, maxPlayerFrames, 0> LuigiFrameX;
RangeArrI<int, 0, maxPlayerFrames, 0> LuigiFrameY;
RangeArrI<int, 0, maxPlayerFrames, 0> PeachFrameX;
RangeArrI<int, 0, maxPlayerFrames, 0> PeachFrameY;
RangeArrI<int, 0, maxPlayerFrames, 0> ToadFrameX;
RangeArrI<int, 0, maxPlayerFrames, 0> ToadFrameY;
RangeArrI<int, 0, maxPlayerFrames, 0> LinkFrameX;
RangeArrI<int, 0, maxPlayerFrames, 0> LinkFrameY;
RangeArrI<bool, 0, maxBackgroundType, false> BackgroundFence;
RangeArrI<int, 0, maxNPCType, 0> NPCFrameOffsetX;
RangeArrI<int, 0, maxNPCType, 0> NPCFrameOffsetY;
RangeArrI<int, 0, maxNPCType, 0> NPCWidth;
RangeArrI<int, 0, maxNPCType, 0> NPCHeight;
RangeArrI<int, 0, maxNPCType, 0> NPCWidthGFX;
RangeArrI<int, 0, maxNPCType, 0> NPCHeightGFX;
RangeArr<float, 0, maxNPCType> NPCSpeedvar;

RangeArrI<bool, 0, maxNPCType, false> NPCIsAShell;
RangeArrI<bool, 0, maxNPCType, false> NPCIsABlock;
RangeArrI<bool, 0, maxNPCType, false> NPCIsAHit1Block;
RangeArrI<bool, 0, maxNPCType, false> NPCIsABonus;
RangeArrI<bool, 0, maxNPCType, false> NPCIsACoin;
RangeArrI<bool, 0, maxNPCType, false> NPCIsAVine;
RangeArrI<bool, 0, maxNPCType, false> NPCIsAnExit;
RangeArrI<bool, 0, maxNPCType, false> NPCIsAParaTroopa;
RangeArrI<bool, 0, maxNPCType, false> NPCIsCheep;
RangeArrI<bool, 0, maxNPCType, false> NPCJumpHurt;
RangeArrI<bool, 0, maxNPCType, false> NPCNoClipping;
RangeArrI<int, 0, maxNPCType, 0> NPCScore;
RangeArrI<bool, 0, maxNPCType, false> NPCCanWalkOn;
RangeArrI<bool, 0, maxNPCType, false> NPCGrabFromTop;
RangeArrI<bool, 0, maxNPCType, false> NPCTurnsAtCliffs;
RangeArrI<bool, 0, maxNPCType, false> NPCWontHurt;
RangeArrI<bool, 0, maxNPCType, false> NPCMovesPlayer;
RangeArrI<bool, 0, maxNPCType, false> NPCStandsOnPlayer;
RangeArrI<bool, 0, maxNPCType, false> NPCIsGrabbable;
RangeArrI<bool, 0, maxNPCType, false> NPCIsBoot;
RangeArrI<bool, 0, maxNPCType, false> NPCIsYoshi;
RangeArrI<bool, 0, maxNPCType, false> NPCIsToad;
RangeArrI<bool, 0, maxNPCType, false> NPCNoYoshi;
RangeArrI<bool, 0, maxNPCType, false> NPCForeground;
RangeArrI<bool, 0, maxNPCType, false> NPCIsABot;
RangeArrI<bool, 0, maxNPCType, false> NPCDefaultMovement;
RangeArrI<bool, 0, maxNPCType, false> NPCIsVeggie;
RangeArrI<bool, 0, maxNPCType, false> NPCNoFireBall;
RangeArrI<bool, 0, maxNPCType, false> NPCNoIceBall;
RangeArrI<bool, 0, maxNPCType, false> NPCNoGravity;

RangeArrI<int, 0, maxNPCType, 0> NPCFrame;
RangeArrI<int, 0, maxNPCType, 0> NPCFrameSpeed;
RangeArrI<int, 0, maxNPCType, 0> NPCFrameStyle;

RangeArrI<bool, 0, maxBlockType, false> BlockIsSizable;
RangeArrI<int, 0, maxBlockType, 0> BlockSlope;
RangeArrI<int, 0, maxBlockType, 0> BlockSlope2;
RangeArr<double, 0, maxPlayers> vScreenX;
RangeArr<double, 0, maxPlayers> vScreenY;

RangeArr<double, 0, maxPlayers> qScreenX;
RangeArr<double, 0, maxPlayers> qScreenY;
bool qScreen = false;
RangeArr<vScreen_t, 0, 2> qScreenLoc;

RangeArrI<int, 0, maxBlockType, 0> BlockWidth;
RangeArrI<int, 0, maxBlockType, 0> BlockHeight;
RangeArrI<int, 0, maxBlockType, 0> BonusWidth;
RangeArrI<int, 0, maxBlockType, 0> BonusHeight;
RangeArrI<int, 0, maxBlockType, 0> EffectWidth;
RangeArrI<int, 0, maxBlockType, 0> EffectHeight;

EffectDefaults_t EffectDefaults;
RangeArrI<int, 1, maxSceneType, 0> SceneWidth;
RangeArrI<int, 1, maxSceneType, 0> SceneHeight;
RangeArrI<int, 0, 100, 0> SpecialFrame;
RangeArr<float, 0, 100> SpecialFrameCount;
RangeArrI<bool, 1, maxBackgroundType, false> BackgroundHasNoMask;
RangeArrI<bool, 0, maxBackgroundType, false> Foreground;
RangeArrI<int, 1, maxBackgroundType, 0> BackgroundWidth;
RangeArrI<int, 1, maxBackgroundType, 0> BackgroundHeight;
RangeArrI<int, 1, maxBackgroundType, 0> BackgroundFrame;
RangeArrI<int, 1, maxBackgroundType, 0> BackgroundFrameCount;
RangeArrI<int, 1, maxBlockType, 0> BlockFrame;
RangeArrI<int, 1, maxBlockType, 0> BlockFrame2;

// deprecated
// RangeArrI<int, 1, 1000, 0> sBlockArray;
// int sBlockNum = 0;

RangeArrI<int, 1, maxSceneType, 0> SceneFrame;
RangeArrI<int, 1, maxSceneType, 0> SceneFrame2;
RangeArrI<int, 1, maxTileType, 0> TileWidth;
RangeArrI<int, 1, maxTileType, 0> TileHeight;
RangeArrI<int, 1, maxTileType, 0> TileFrame;
RangeArrI<int, 1, maxTileType, 0> TileFrame2;
RangeArrI<int, 1, 100, 0> LevelFrame;
RangeArrI<int, 1, 100, 0> LevelFrame2;
RangeArrI<bool, 1, maxBlockType, false> BlockHasNoMask;
RangeArrI<bool, 1, 100, false> LevelHasNoMask;
RangeArrI<bool, 0, maxBlockType, false> BlockOnlyHitspot1;
RangeArrI<bool, 0, maxBlockType, false> BlockKills;
RangeArrI<bool, 0, maxBlockType, false> BlockKills2;
RangeArrI<bool, 0, maxBlockType, false> BlockHurts;
RangeArrI<bool, 0, maxBlockType, false> BlockPSwitch;
RangeArrI<bool, 0, maxBlockType, false> BlockNoClipping;
RangeArrI<int, 1, 10, 0> CoinFrame;
RangeArrI<int, 1, 10, 0> CoinFrame2;
EditorCursor_t EditorCursor;
OldEditorControls_t OldEditorControls;
EditorControls_t EditorControls;
SharedControls_t SharedControls;

CursorControls_t SharedCursor;
// RangeArr<CursorControls_t, 1, maxLocalPlayers> PlayerCursor;

RangeArrI<int, 1, numSounds, 0> Sound;
RangeArrI<int, 1, numSounds, 0> SoundPause;
bool ErrorQuit = false;
bool EndLevel = false;
int LevelMacro = LEVELMACRO_OFF;
int LevelMacroWhich = 0;
int LevelMacroCounter = 0;
int numJoysticks = 0;

// information about the currently loaded file
std::string FileName;
std::string FileNameFull;
std::string FullFileName;
std::string FileNamePath;
int FileFormat = 0;

// backup information to restore when returning to world map
std::string FileNameWorld;
std::string FileNameFullWorld;
std::string FileNamePathWorld;
int FileFormatWorld = 0;

bool IsEpisodeIntro = false;
int Coins = 0;
float Lives = 0.0f;
bool EndIntro = false;
bool ExitMenu = false;
bool LevelSelect = false;
bool LevelRestartRequested = false;
RangeArr<WorldPlayer_t, 0, 1> WorldPlayer;
int LevelBeatCode = 0;
int curWorldLevel = 0;
int curWorldMusic = 0;
std::string curWorldMusicFile;
RangeArrI<bool, 0, maxSections, false> NoTurnBack;
RangeArrI<bool, 0, maxSections, false> UnderWater;
bool TestLevel = false;
bool GameMenu = false;
std::string WorldName;
int selWorld = 0;
int selSave = 0;
int PSwitchTime = 0;
int PSwitchStop = 0;
int PSwitchPlayer = 0;

RangeArr<SaveSlotInfo_t, 1, maxSaveSlots> SaveSlotInfo;

int BeltDirection = 0;
bool BeatTheGame = false;
//int cycleCount = 0;
//double fpsTime = 0.0;
//double fpsCount = 0.0;
bool FrameSkip = false;
//double GoalTime = 0.0;
//double overTime = 0.0;
int worldCurs = 0;
int minShow = 0;
int maxShow = 0;

int ReturnWarp = 0;
int ReturnWarpSaved = 0;
int StartWarp = 0;
Physics_t Physics;
int MenuCursor = 0;
int MenuMode = 0;
bool MenuCursorCanMove = false;
bool MenuCursorCanMove2 = false;
bool NextFrame = false;
int StopHit = 0;
bool MouseRelease = false;
bool TestFullscreen = false;
//bool keyDownAlt = false;
//bool keyDownEnter = false;
bool BlocksSorted = false;
int SingleCoop = 0;
//std::string CheatString;
bool GameOutro = false;
bool GameOutroDoQuit = false;
float CreditChop = 0.0f;
int EndCredits = 0;
int curStars = 0;
int maxStars = 0;
bool ShadowMode = false;
bool MultiHop = false;
bool SuperSpeed = false;
bool WalkAnywhere = false;
bool FlyForever = false;
bool FreezeNPCs = false;
bool CaptainN = false;
bool FlameThrower = false;
bool CoinMode = false;
bool WorldUnlock = false;
bool MaxFPS = false;
bool GodMode = false;
bool GrabAll = false;
bool Cheater = false;
RangeArr<std::string, 1, maxWorldCredits> WorldCredits;
int Score = 0;
RangeArrI<int, 1, 13, 0> Points;
int MaxWorldStars = 0;
bool Debugger = false;
RangeArr<SavedChar_t, 0, 10> SavedChar;

bool LoadingInProcess = false;
int LoadCoins = 0;
float LoadCoinsT = 0.0f;

RangeArrI<bool, 1, maxBlockType, false> GFXBlockCustom;
//RangeArrI<long, 1, maxBlockType, 0> GFXBlock;
//RangeArrI<long, 1, maxBlockType, 0> GFXBlockMask;
RangeArr<StdPicture, 1, maxBlockType> GFXBlockBMP;
//RangeArr<StdPicture, 1, maxBlockType> GFXBlockMaskBMP;
RangeArrI<bool, 1, numBackground2, false> GFXBackground2Custom;
//RangeArrI<long, 1, numBackground2, 0> GFXBackground2;
RangeArr<StdPicture, 1, numBackground2> GFXBackground2BMP;
RangeArrI<int, 1, numBackground2, 0> GFXBackground2Height;
RangeArrI<int, 1, numBackground2, 0> GFXBackground2Width;
RangeArrI<bool, 1, maxNPCType, false> GFXNPCCustom;
//RangeArrI<long, 1, maxNPCType, 0> GFXNPC;
//RangeArrI<long, 1, maxNPCType, 0> GFXNPCMask;
RangeArr<StdPicture, 0, maxNPCType> GFXNPCBMP;
//RangeArr<StdPicture, 0, maxNPCType> GFXNPCMaskBMP;
RangeArrI<int, 1, maxNPCType, 0> GFXNPCHeight;
RangeArrI<int, 1, maxNPCType, 0> GFXNPCWidth;
RangeArrI<bool, 1, maxEffectType, false> GFXEffectCustom;
//RangeArrI<long, 1, maxEffectType, 0> GFXEffect;
//RangeArrI<long, 1, maxEffectType, 0> GFXEffectMask;
RangeArr<StdPicture, 1, maxEffectType> GFXEffectBMP;
//RangeArr<StdPicture, 1, maxEffectType> GFXEffectMaskBMP;
RangeArrI<int, 1, maxEffectType, 0> GFXEffectHeight;
RangeArrI<int, 1, maxEffectType, 0> GFXEffectWidth;
RangeArrI<bool, 1, maxBackgroundType, false> GFXBackgroundCustom;
//RangeArrI<long, 1, maxBackgroundType, 0> GFXBackground;
//RangeArrI<long, 1, maxBackgroundType, 0> GFXBackgroundMask;
RangeArr<StdPicture, 1, maxBackgroundType> GFXBackgroundBMP;
//RangeArr<StdPicture, 1, maxBackgroundType> GFXBackgroundMaskBMP;
RangeArrI<int, 1, maxBackgroundType, 0> GFXBackgroundHeight;
RangeArrI<int, 1, maxBackgroundType, 0> GFXBackgroundWidth;

const char *GFXPlayerNames[numCharacters] =
{
    "mario", "luigi", "peach", "toad", "link"
};
RangeArr<StdPicture, 1, 10> *GFXCharacterBMP[numCharacters] =
{
    &GFXMarioBMP, &GFXLuigiBMP, &GFXPeachBMP, &GFXToadBMP, &GFXLinkBMP
};
RangeArrI<int, 1, 10, 0> *GFXCharacterWidth[numCharacters] =
{
    &GFXMarioWidth, &GFXLuigiWidth, &GFXPeachHeight, &GFXToadWidth, &GFXLinkWidth
};
RangeArrI<int, 1, 10, 0> *GFXCharacterHeight[numCharacters] =
{
    &GFXMarioHeight, &GFXLuigiHeight, &GFXPeachHeight, &GFXToadHeight, &GFXLinkHeight
};
RangeArrI<bool, 1, 10, false> *GFXCharacterCustom[numCharacters] =
{
    &GFXMarioCustom, &GFXLuigiCustom, &GFXPeachCustom, &GFXToadCustom, &GFXLinkCustom
};

RangeArrI<bool, 1, 10, false> GFXMarioCustom;
//RangeArrI<long, 1, 10, 0> GFXMario;
//RangeArrI<long, 1, 10, 0> GFXMarioMask;
RangeArr<StdPicture, 1, 10> GFXMarioBMP;
//RangeArr<StdPicture, 1, 10> GFXMarioMaskBMP;
RangeArrI<int, 1, 10, 0> GFXMarioHeight;
RangeArrI<int, 1, 10, 0> GFXMarioWidth;
RangeArrI<bool, 1, 10, false> GFXLuigiCustom;
//RangeArrI<long, 1, 10, 0> GFXLuigi;
//RangeArrI<long, 1, 10, 0> GFXLuigiMask;
RangeArr<StdPicture, 1, 10> GFXLuigiBMP;
//RangeArr<StdPicture, 1, 10> GFXLuigiMaskBMP;
RangeArrI<int, 1, 10, 0> GFXLuigiHeight;
RangeArrI<int, 1, 10, 0> GFXLuigiWidth;
RangeArrI<bool, 1, 10, false> GFXPeachCustom;
//RangeArrI<long, 1, 10, 0> GFXPeach;
//RangeArrI<long, 1, 10, 0> GFXPeachMask;
RangeArr<StdPicture, 1, 10> GFXPeachBMP;
//RangeArr<StdPicture, 1, 10> GFXPeachMaskBMP;
RangeArrI<int, 1, 10, 0> GFXPeachHeight;
RangeArrI<int, 1, 10, 0> GFXPeachWidth;
RangeArrI<bool, 1, 10, false> GFXToadCustom;
//RangeArrI<long, 1, 10, 0> GFXToad;
//RangeArrI<long, 1, 10, 0> GFXToadMask;
RangeArr<StdPicture, 1, 10> GFXToadBMP;
//RangeArr<StdPicture, 1, 10> GFXToadMaskBMP;
RangeArrI<int, 1, 10, 0> GFXToadHeight;
RangeArrI<int, 1, 10, 0> GFXToadWidth;

RangeArrI<bool, 1, 10, false> GFXLinkCustom;
//RangeArrI<long, 1, 10, 0> GFXLink;
//RangeArrI<long, 1, 10, 0> GFXLinkMask;
RangeArr<StdPicture, 1, 10> GFXLinkBMP;
//RangeArr<StdPicture, 1, 10> GFXLinkMaskBMP;
RangeArrI<int, 1, 10, 0> GFXLinkHeight;
RangeArrI<int, 1, 10, 0> GFXLinkWidth;

RangeArrI<bool, 1, maxYoshiGfx, false> GFXYoshiBCustom;
//RangeArrI<long, 1, 10, 0> GFXYoshiB;
//RangeArrI<long, 1, 10, 0> GFXYoshiBMask;
RangeArr<StdPicture, 1, 10> GFXYoshiBBMP;
//RangeArr<StdPicture, 1, 10> GFXYoshiBMaskBMP;
RangeArrI<bool, 1, 10, false> GFXYoshiTCustom;
//RangeArrI<long, 1, 10, 0> GFXYoshiT;
//RangeArrI<long, 1, 10, 0> GFXYoshiTMask;
RangeArr<StdPicture, 1, 10> GFXYoshiTBMP;
//RangeArr<StdPicture, 1, 10> GFXYoshiTMaskBMP;
RangeArrI<bool, 1, maxTileType, false> GFXTileCustom;
//RangeArrI<long, 1, maxTileType, 0> GFXTile;
RangeArr<StdPicture, 1, maxTileType> GFXTileBMP;
RangeArrI<int, 1, maxTileType, 0> GFXTileHeight;
RangeArrI<int, 1, maxTileType, 0> GFXTileWidth;
RangeArrI<bool, 0, maxLevelType, false> GFXLevelCustom;
//RangeArrI<long, 0, maxLevelType, 0> GFXLevel;
//RangeArrI<long, 0, maxLevelType, 0> GFXLevelMask;
RangeArr<StdPicture, 0, maxLevelType> GFXLevelBMP;
//RangeArr<StdPicture, 0, maxLevelType> GFXLevelMaskBMP;
RangeArrI<int, 0, maxLevelType, 0> GFXLevelHeight;
RangeArrI<int, 0, maxLevelType, 0> GFXLevelWidth;
RangeArrI<bool, 0, maxLevelType, false> GFXLevelBig;
RangeArrI<bool, 1, maxSceneType, false> GFXSceneCustom;
//RangeArrI<long, 1, maxSceneType, 0> GFXScene;
//RangeArrI<long, 1, maxSceneType, 0> GFXSceneMask;
RangeArr<StdPicture, 1, maxSceneType> GFXSceneBMP;
//RangeArr<StdPicture, 1, maxSceneType> GFXSceneMaskBMP;
RangeArrI<int, 1, maxSceneType, 0> GFXSceneHeight;
RangeArrI<int, 1, maxSceneType, 0> GFXSceneWidth;
RangeArrI<bool, 1, maxPathType, false> GFXPathCustom;
//RangeArrI<long, 1, maxPathType, 0> GFXPath;
//RangeArrI<long, 1, maxPathType, 0> GFXPathMask;
RangeArr<StdPicture, 1, maxPathType> GFXPathBMP;
//RangeArr<StdPicture, 1, maxPathType> GFXPathMaskBMP;
RangeArrI<int, 1, maxPathType, 0> GFXPathHeight;
RangeArrI<int, 1, maxPathType, 0> GFXPathWidth;

RangeArrI<bool, 1, numCharacters, false> GFXPlayerCustom;
//RangeArrI<long, 1, numCharacters, 0> GFXPlayer;
//RangeArrI<long, 1, numCharacters, 0> GFXPlayerMask;
RangeArr<StdPicture, 1, numCharacters> GFXPlayerBMP;
//RangeArr<StdPicture, 1, numCharacters> GFXPlayerMaskBMP;
RangeArrI<int, 1, numCharacters, 0> GFXPlayerHeight;
RangeArrI<int, 1, numCharacters, 0> GFXPlayerWidth;

int PlayerCharacter = 0;
int PlayerCharacter2 = 0;
// double MenuMouseX = 0.0;
// double MenuMouseY = 0.0;
// Sint32 MenuWheelDelta = 0;
// bool MenuWheelMoved = false;
// bool MenuMouseDown = false;
// bool MenuMouseBack = false;
bool MenuMouseRelease = false;
// bool MenuMouseMove = false;
bool MenuMouseClick = false;

bool ForcedControls = false;
Controls_t ForcedControl;
int SyncCount = 0;
bool noUpdate = false;
//double gameTime = 0.0;
bool noSound = false;
bool neverPause = false;
//double tempTime = 0.0;
bool BattleMode = false;
int BattleWinner = 0;
RangeArrI<int, 1, maxPlayers, 0> BattleLives;
int BattleIntro = 0;
int BattleOutro = 0;
std::string LevelName;

//void DoEvents()
//{
//    g_events->doEvents();
//}

//Uint8 getKeyState(int key)
//{
//    return XEvents::getKeyState(key);
//}

//Uint8 getKeyStateI(int key)
//{
//    if(key < 0)
//        return 0;
//    return XEvents::getKeyState(key);
//}

// const char *getKeyName(int key)
// {
//     if(key < 0)
//         return " ... ";
//     return g_events->getScanCodeName(key);
// }

void initAll()
{
    SavedEvents.fill(std::string());
    BlockSwitch.fill(false);
    PowerUpUnlock.fill(false);
    vScreen.fill(vScreen_t());
    PlayerStart.fill(Location_t());
    blockCharacter.fill(false);
    OwedMount.fill(0);
    OwedMountType.fill(0);
    AutoX.fill(0.f);
    AutoY.fill(0.f);
    Water.fill(Water_t());
    Star.fill(Star_t());
    // FirstBlock.fill(0);
    // LastBlock.fill(0);
    iBlock.fill(0);
    CustomMusic.fill(std::string());
    level.fill(Location_t());
    LevelWrap.fill(false);
    LevelVWrap.fill(false);
    OffScreenExit.fill(false);
    bgMusic.fill(0);
    bgMusicREAL.fill(0);
    Background2REAL.fill(0);
    Background2.fill(0);
    SpecialFrameCount.fill(0.f);
    NPCSpeedvar.fill(0.f);
    vScreenX.fill(0.0);
    vScreenY.fill(0.0);
    qScreenX.fill(0.0);
    qScreenY.fill(0.0);

    Block.fill(Block_t());
    Background.fill(Background_t());
    NPC.fill(NPC_t());
}


const double power10[] =
{
    1.0,
    10.0,
    100.0,
    1000.0,
    10000.0,

    100000.0,
    1000000.0,
    10000000.0,
    100000000.0,
    1000000000.0,

    10000000000.0,
    100000000000.0,
    1000000000000.0,
    10000000000000.0,
    100000000000000.0,

    1000000000000000.0,
    10000000000000000.0,
    100000000000000000.0,
    1000000000000000000.0,
    10000000000000000000.0,

    100000000000000000000.0,
    1000000000000000000000.0
};

int vb6Round(double x)
{
    return static_cast<int>(vb6Round(x, 0));
}

#ifdef USE_CUSTOM_TONEAREST
#   define toNearest pge_toNearest
#else
static SDL_INLINE double toNearest(double x)
{
    int round_old = std::fegetround();
    if(round_old == FE_TONEAREST)
        return std::nearbyint(x);
    else
    {
        std::fesetround(FE_TONEAREST);
        x = std::nearbyint(x);
        std::fesetround(round_old);
        return x;
    }
}
#endif

double vb6Round(double x, int decimals)
{
    double res = x, decmul;

    if(decimals < 0 || decimals >= 22)
        decimals = 0;

    if(SDL_fabs(x) < 1.0e16)
    {
        decmul = power10[decimals];
        res = toNearest(x * decmul) / decmul;
    }

    return res;
}
