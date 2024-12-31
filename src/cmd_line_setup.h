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
#ifndef CMD_LINE_SETUP_H
#define CMD_LINE_SETUP_H

#include <string>

struct CmdLineSetup_t
{
    //! Is a level testing mode
    bool testLevelMode = false;
    //! Level file to test
    std::string testLevel;
    //! Replay file to run
    std::string testReplay;
    //! Number of players for level test
    int testNumPlayers = 1;
    //! Save slot to use for world test
    int testSave = 0;
    //! Run a test in battle mode
    bool testBattleMode = false;

    //! Custom-specified asset pack name or path
    std::string assetPack;

    //! Enable interprocessing communication with the Moondust Editor
    bool interprocess = false;

    //! Allow playable character grab everything
    bool testGrabAll = false;
    //! Make playable character be inmortal
    bool testGodMode = false;
    //! Enable magic-hand functionality
    bool testMagicHand = false;
    //! Open in editor
    bool testEditor = false;

    //! Force log output into console
    bool verboseLogging = false;
};

#endif // CMD_LINE_SETUP_H
