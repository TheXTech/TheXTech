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

#include "main/game_strings.h"

GameContent g_gameStrings;

void initGameStrings()
{
    g_gameStrings.msgBoxTitleInfo = "Info";
    g_gameStrings.msgBoxTitleWarning = "Warning!";
    g_gameStrings.msgBoxTitleError = "Error!";

    g_gameStrings.errorOpenFileFailed = "Can't open \"{0}\": file doesn't exist or corrupted.";
#if defined(THEXTECH_INTERPROC_SUPPORTED) || !defined(THEXTECH_DISABLE_LANG_TOOLS)
    g_gameStrings.errorOpenIPCDataFailed = "Can't proceed received file data because of corruption or other errors.";
#endif
    g_gameStrings.errorTooOldEngine = "Content can't be loaded because it requires higher "
                                      "features level ({0}) than this engine can offer ({1}). "
                                      "Please update TheXTech.";
    g_gameStrings.errorTooOldGameAssets = "Content can't be loaded with the current game assets "
                                          "pack because it installs too low feature level ({1}) "
                                          "than is required ({0}). Please upgrade your game assets "
                                          "pack to support higher feature level.";

    g_gameStrings.errorInvalidEnterWarp = "Can't start the level "
                                          "because of an invalid "
                                          "entrance warp {1} was "
                                          "specified.\n"
                                          "Total warp entries: {2}\n"
                                          "\n"
                                          "File: {0}";
    g_gameStrings.errorNoStartPoint = "Can't start the level "
                                      "because of no available "
                                      "start points placed or "
                                      "entrance warp specified.\n"
                                      "\n"
                                      "File: {0}";

#if defined(THEXTECH_INTERPROC_SUPPORTED) || !defined(THEXTECH_DISABLE_LANG_TOOLS)
    g_gameStrings.errorIPCTimeOut = "No responce from the connected Editor. Game will be closed.";
#endif

    g_gameStrings.messageScanningLevels = "Scanning levels...";
    g_gameStrings.formatMinutesSeconds = "{0}m{1}s";

#if defined(THEXTECH_INTERPROC_SUPPORTED) || !defined(THEXTECH_DISABLE_LANG_TOOLS)
    g_gameStrings.ipcStatusWaitingInput = "Waiting for input data...";
    g_gameStrings.ipcStatusDataTransferStarted = "Started data tansfer...";
    g_gameStrings.ipcStatusDataAccepted = "Data accepted, the parsing started...";
    g_gameStrings.ipcStatusDataValid = "Accepted data is valid";
    g_gameStrings.ipcStatusErrorTimeout = "ERROR: Wait time out.";
    g_gameStrings.ipcStatusLoadingDone = "Done. Starting game...";
#endif

#if !defined(NO_WINDOW_FOCUS_TRACKING) || !defined(THEXTECH_DISABLE_LANG_TOOLS)
    g_gameStrings.screenPaused = "Paused";
#endif

    g_gameStrings.warpNeedStarCount = "You need {0} {1} to enter.";

    g_gameStrings.pauseItemContinue = "Continue";
    g_gameStrings.pauseItemRestartLevel = "Restart Level";
    g_gameStrings.pauseItemResetCheckpoints = "Reset Checkpoints";
    g_gameStrings.pauseItemQuitTesting = "Quit Testing";
    g_gameStrings.pauseItemReturnToEditor = "Return to Editor";
    g_gameStrings.pauseItemPlayerSetup = "Player Setup";
    g_gameStrings.pauseItemEnterCode = "Enter Code";
    g_gameStrings.pauseItemSaveAndContinue = "Save and Continue";
    g_gameStrings.pauseItemSaveAndQuit = "Save and Quit";
    g_gameStrings.pauseItemQuit = "Quit";


    g_gameStrings.connectReconnectTitle = "Reconnect";
    g_gameStrings.connectPressAButton = "Press A Button";
    g_gameStrings.connectTestProfile = "Test Profile";
    g_gameStrings.connectDisconnect = "Disconnect";
    g_gameStrings.connectHoldStart = "Hold Start";

    g_gameStrings.connectDropMe = "Drop Me";

    g_gameStrings.connectDropPX = "Drop P{0}";
    g_gameStrings.connectForceResume = "Force Resume";

    g_gameStrings.connectWaitingForInputDevice = "Waiting for input device...";
    g_gameStrings.connectPressSelectForControlsOptions_P1 = "Press Select for";
    g_gameStrings.connectPressSelectForControlsOptions_P2 = "Controls Options";

}
