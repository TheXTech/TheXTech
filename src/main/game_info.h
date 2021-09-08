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

#pragma once
#ifndef GAME_INFO_H
#define GAME_INFO_H

#include <string>
#include <vector>
#include "global_constants.h"
#include "range_arr.hpp"

struct GameInfo
{
    std::string title;
    std::string titleWindow;
    std::string titleCredits;
    RangeArr<std::string, 1, numCharacters> characterName;

    int creditsFont;
    std::string creditsHomePage;
    std::vector<std::string> creditsGame;

    bool introEnableActivity = true;
    int  introMaxPlayersCount = 6;
};

extern GameInfo g_gameInfo;

extern void initGameInfo();

#endif // GAME_INFO_H
