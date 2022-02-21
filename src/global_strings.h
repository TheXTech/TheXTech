/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2022 Vitaly Novichkov <admin@wohlnet.ru>
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

// utilities for stringindex_t
// everything here is defined in `globals.cpp`

#pragma once
#ifndef GLOBAL_STRINGS_H
#define GLOBAL_STRINGS_H

#include "global_constants.h"
#include <vector>
#include <string>
#include <SDL2/SDL_assert.h>

extern const std::string g_emptyString;
extern std::vector<std::string> g_LevelString;
extern size_t g_numWorldString;

inline const std::string& GetS(stringindex_t index)
{
    if(index == STRINGINDEX_NONE)
        return g_emptyString;

    SDL_assert_release(index < g_LevelString.size());

    return g_LevelString[index];
}

inline void SetS(stringindex_t& index, const std::string& target)
{
    if(index == STRINGINDEX_NONE && target.empty())
        return;

    if(index == STRINGINDEX_NONE && g_LevelString.size() < MaxLevelStrings)
    {
        index = (stringindex_t)g_LevelString.size();
        g_LevelString.push_back(target);
    }
    else
    {
        SDL_assert_release(index < g_LevelString.size());
        g_LevelString[index] = target;
    }
}

inline std::string* PtrS(stringindex_t& index)
{
    if(index == STRINGINDEX_NONE)
    {
        if(g_LevelString.size() >= MaxLevelStrings)
            return nullptr;
        index = (stringindex_t)g_LevelString.size();
        g_LevelString.push_back(std::string());
    }

    SDL_assert_release(index < g_LevelString.size());

    return &g_LevelString[index];
}

#endif // #ifndef GLOBAL_STRINGS_H
