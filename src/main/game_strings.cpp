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

#include "main/game_strings.h"

GameContent g_gameStrings;

void initGameStrings()
{
    g_gameStrings.controlsPhrasePlayerDisconnected = "P{0} disconnected";
    g_gameStrings.controlsPhrasePlayerConnected = "P{0} connected {1}, Profile: {2}";

    g_gameStrings.errorOpenFileFailed = "ERROR: Can't open \"{0}\": file doesn't exist or corrupted.";

    g_gameStrings.warpNeedStarCount = "You need {0} {1} to enter.";

    g_gameStrings.pauseItemContinue = "Continue";
    g_gameStrings.pauseItemRestartLevel = "Restart Level";
    g_gameStrings.pauseItemResetCheckpoints = "Reset Checkpoints";
    g_gameStrings.pauseItemQuitTesting = "Quit Testing";
    g_gameStrings.pauseItemReturnToEditor = "Return to Editor";
    g_gameStrings.pauseItemDropAddPlayers = "Drop/Add Players";
    g_gameStrings.pauseItemEnterCode = "Enter Code";
    g_gameStrings.pauseItemSaveAndContinue = "Save and Continue";
    g_gameStrings.pauseItemSaveAndQuit = "Save and Quit";
    g_gameStrings.pauseItemQuit = "Quit";
}
