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

#ifndef LIMITS_H
#define LIMITS_H

//Public Const MaxSavedEvents As Integer = 200
const int MaxSavedEvents = 200;

const int maxSelectWorlds = 100;

const int maxCreditsLines = 200;

const int maxSaveSlots = 3;

const int maxWorldCredits = 100;

const int maxYoshiGfx = 10;

const int maxStarsNum = 1000;

//Public Const vScreenYOffset As Integer = 0     'Players Y on the screen
const int vScreenYOffset = 0;
//Public Const maxBlocks As Integer = 20000  'Max # of blocks
const int maxBlocks = 50000;
//Public Const maxPlayers As Integer = 200  'Holds the max number of players
const int maxPlayers = 200;
//Public Const maxEffects As Integer = 1000    'Max # of effects
const int maxEffects = 2000;
//Public Const maxNPCs As Integer = 5000    'Max # of NPCs
const int maxNPCs = 8000;
//Public Const maxBackgrounds As Integer = 8000    'Max # of background objects
const int maxBackgrounds = 8000;
//Public Const maxPlayerFrames As Integer = 750 'Maximum number of player frames
const int maxPlayerFrames = 950;
//Public Const maxBlockType As Integer = 700 'Maximum number of block types
const int maxBlockType = 750;
//Public Const maxBackgroundType As Integer = 200 'Maximum number of background types
const int maxBackgroundType = 400;
//Public Const maxSceneType As Integer = 100 'Maximum number of scenetypes
const int maxSceneType = 200;
//Public Const maxNPCType As Integer = 300 'Maximum number of NPC types
const int maxNPCType = 750;
//Public Const maxEffectType As Integer = 200 'Maximum number of effect types
const int maxEffectType = 400;
//Public Const maxWarps As Integer = 200 'Maximum number of warps
const int maxWarps = 4000; // 200
//Public Const numBackground2 As Integer = 100  'Total # of backgrounds
const int numBackground2 = 150;
//Public Const numCharacters As Integer = 5 'Maximum number of player characters
const int numCharacters = 5;
//Public Const numStates As Integer = 7   'Maximum number of player states
const int numStates = 9;
//Public Const maxWater As Integer = 1000
const int maxWater = 2000;
//Public Const maxWorldLevels As Integer = 400   'Maximum number of levels
const int maxWorldLevels = 800;
//Public Const maxWorldPaths As Integer = 2000   'Maximum number of paths
const int maxWorldPaths = 5000;
//Public Const maxWorldMusic As Integer = 1000   'Maximum number of musics
const int maxWorldMusic = 2000;
//Public Const numSounds As Integer = 100
const int numSounds = 100;
//Public Const maxSections As Integer = 20
const int maxSections = 200;
//Public Const maxTileType As Integer = 400
const int maxTileType = 400;
//Public Const maxLevelType As Integer = 100
const int maxLevelType = 100;
//Public Const maxPathType As Integer = 100
const int maxPathType = 100;
//Public Const maxTiles As Integer = 20000
const int maxTiles = 20000;
//Public Const maxScenes As Integer = 5000
const int maxScenes = 5000;

//Public Const ScreenW As Integer = 800  'Game Screen Width
const int ScreenW = 800;
//Public Const ScreenH As Integer = 600  'Game Screen Height
const int ScreenH = 600;

//Public Const frameRate As Double = 15 'for controlling game speed
const int frameRate = 15;

#endif // LIMITS_H
