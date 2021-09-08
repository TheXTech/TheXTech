/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2021 Vitaly Novichkov <admin@wohlnet.ru>
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

#ifndef LIMITS_H
#define LIMITS_H

//Public Const MaxSavedEvents As Integer = 200
const int MaxSavedEvents = 200;

//const int maxSelectWorlds = 100;

const int maxCreditsLines = 200;

const int maxSaveSlots = 3;

const int maxWorldCredits = 100;

const int maxYoshiGfx = 10;

const int maxStarsNum = 1000;

const int maxLocalPlayers = 2;

//Public Const vScreenYOffset As Integer = 0     'Players Y on the screen
const int vScreenYOffset = 0;
//Public Const maxBlocks As Integer = 20000  'Max # of blocks
const int maxBlocks = 20000;
//Public Const maxPlayers As Integer = 200  'Holds the max number of players
const int maxPlayers = 200;
//Public Const maxEffects As Integer = 1000    'Max # of effects
const int maxEffects = 1000;
//Public Const maxNPCs As Integer = 5000    'Max # of NPCs
const int maxNPCs = 5000;
//Public Const maxBackgrounds As Integer = 8000    'Max # of background objects
const int maxBackgrounds = 8000;
//Public Const maxPlayerFrames As Integer = 750 'Maximum number of player frames
const int maxPlayerFrames = 750;
//Public Const maxBlockType As Integer = 700 'Maximum number of block types
const int maxBlockType = 700;
//Public Const maxBackgroundType As Integer = 200 'Maximum number of background types
const int maxBackgroundType = 200;
//Public Const maxSceneType As Integer = 100 'Maximum number of scenetypes
const int maxSceneType = 100;
//Public Const maxNPCType As Integer = 300 'Maximum number of NPC types
const int maxNPCType = 302;
//Public Const maxEffectType As Integer = 200 'Maximum number of effect types
const int maxEffectType = 200;
//Public Const maxWarps As Integer = 200 'Maximum number of warps
const int maxWarps = 2000; // 200
//Public Const numBackground2 As Integer = 100  'Total # of backgrounds
const int numBackground2 = 100;
//Public Const numCharacters As Integer = 5 'Maximum number of player characters
const int numCharacters = 5;
//Public Const numStates As Integer = 7   'Maximum number of player states
const int numStates = 7;
//Public Const maxWater As Integer = 1000
const int maxWater = 1000;
//Public Const maxWorldLevels As Integer = 400   'Maximum number of levels
const int maxWorldLevels = 400;
//Public Const maxWorldPaths As Integer = 2000   'Maximum number of paths
const int maxWorldPaths = 2000;
//Public Const maxWorldMusic As Integer = 1000   'Maximum number of musics
const int maxWorldMusic = 1000;
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

#endif // LIMITS_H
