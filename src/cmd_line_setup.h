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
#ifndef CMD_LINE_SETUP_H
#define CMD_LINE_SETUP_H

#include <string>

struct CmdLineSetup_t
{
    //! Disable game sound
    bool noSound = false;
    //! Skip frames when laggy rendering is
    bool frameSkip = false;
    //! Don't pause game while window is not active
    bool neverPause = false;
    //! Allow background input for joysticks
    bool allowBgInput = false;
    //! TYpe of a render
    int renderType = -1;
    //! Is a level testing mode
    bool testLevelMode = false;
    //! Level file to test
    std::string testLevel;
    //! Replay file to run
    std::string testReplay;
    //! Number of players for level test
    int testNumPlayers = 1;
    //! Run a test in battle mode
    bool testBattleMode = false;

    //! Enable interprocessing communication with the Moondust Editor
    bool interprocess = false;

    //! Allow playable character grab everything
    bool testGrabAll = false;
    //! Make playable character be inmortal
    bool testGodMode = false;
    //! Show FPS during level test
    bool testShowFPS = false;
    //! Do not limit framerate
    bool testMaxFPS = false;
    //! Enable magic-hand functionality
    bool testMagicHand = false;
    //! Open in editor
    bool testEditor = false;

    //! Force log output into console
    bool verboseLogging = false;

    //! Enforce spefic compatibiltiy level (If Speed-Run mode is enabled, this will be overwritten)
    int  compatibilityLevel = 0;

    //! Enable the speed-run mode
    int  speedRunnerMode = 0;
    //! Draw the speed-run timer semi-transparently
    bool speedRunnerSemiTransparent = false;
    //! Speed-run timer blink effect policy
    int  speedRunnerBlinkEffect = 0;

    bool showControllerState = false;
};

#endif // CMD_LINE_SETUP_H
