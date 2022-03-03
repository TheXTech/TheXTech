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

#include "global_strings.h"


// utilities for stringindex_t
const std::string g_emptyString = "";
//! Strings storage
static std::vector<std::string> g_LevelString;
//! Number of world strings
static size_t g_numWorldString = 0;

#ifdef STRS_UNIQUENESS_TRACKING
//! Counter of usages per every string
static std::vector<int> g_LevelStringUsages;
//! List of free indexes appearing at the moddle of array
static std::vector<int> g_FreeIndexes;
//! String-To-Index map
static std::unordered_map<std::string, int> g_uniqueStringsIds;
//! Number of world strings
static size_t g_numWorldString = 0;

static std::vector<std::string> g_LevelString_backup;
static std::vector<int> g_LevelStringUsages_backup;
static std::vector<int> g_FreeIndexes_backup;
static std::unordered_map<std::string, int> g_uniqueStringsIds_backup;


void SaveWorldStrings()
{
    g_LevelString_backup = g_LevelString;
    g_LevelStringUsages_backup = g_LevelStringUsages;
    g_FreeIndexes_backup = g_FreeIndexes;
    g_uniqueStringsIds_backup = g_uniqueStringsIds;
    g_numWorldString = g_LevelString.size();
}

void RestoreWorldStrings()
{
    g_LevelString = g_LevelString_backup;
    g_LevelStringUsages = g_LevelStringUsages_backup;
    g_FreeIndexes = g_FreeIndexes_backup;
    g_uniqueStringsIds = g_uniqueStringsIds_backup;
}

void ClearStringsBank()
{
    g_LevelString.clear();
    g_LevelStringUsages.clear();
    g_uniqueStringsIds.clear();
    g_FreeIndexes.clear();
    g_numWorldString = 0;
}

const std::string& GetS(stringindex_t index)
{
    if(index == STRINGINDEX_NONE)
        return g_emptyString;

    SDL_assert_release(index < g_LevelString.size());

    return g_LevelString[index];
}

void SetS(stringindex_t& index, const std::string& target)
{
    if(index == STRINGINDEX_NONE && target.empty())
        return;

    SDL_assert_release(index <= STRINGINDEX_NONE);

    bool stringEmpty = index == STRINGINDEX_NONE && g_LevelString.size() < MaxLevelStrings;

    if(!stringEmpty)
    {
        SDL_assert_release(index < g_LevelString.size());
        if(g_LevelString[index] == target)
            return; // Do nothing, there is an attempt to set the same string

        g_LevelStringUsages[index]--;
        if(g_LevelStringUsages[index] == 0)
        {
            g_uniqueStringsIds.erase(g_LevelString[index]);
            g_LevelString[index].clear();
            g_FreeIndexes.push_back(index);
        }
        stringEmpty = true;
    }

    if(stringEmpty)
    {
        auto f = g_uniqueStringsIds.find(target);
        if(f == g_uniqueStringsIds.end())
        {
            if(g_FreeIndexes.empty())
            {
                index = (stringindex_t)g_LevelString.size();
                g_LevelString.push_back(target);
                g_LevelStringUsages.push_back(1);
            }
            else
            {
                index = g_FreeIndexes.back();
                g_FreeIndexes.pop_back();
                g_LevelString[index] = target;
                g_LevelStringUsages[index] = 1;
            }
            g_uniqueStringsIds[target] = index;
        }
        else
        {
            index = f->second;
            g_LevelStringUsages[index]++;
        }
    }
}

stringindex_t SetS(const std::string& target)
{
    stringindex_t out = STRINGINDEX_NONE;
    SetS(out, target);
    return out;
}

std::string* PtrS(stringindex_t& index)
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
