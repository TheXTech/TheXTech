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

#include "globals.h"
#include "global_strings.h"

#include "sdl_proxy/sdl_assert.h"

#define STRS_UNIQUENESS_TRACKING


// utilities for stringindex_t
const std::string g_emptyString = "";
//! Strings storage
static std::vector<std::string> g_LevelString;
//! Number of world strings
static size_t g_numWorldString = 0;
//! List of free indexes appearing at the moddle of array
static std::vector<int> g_FreeIndexes;

#ifdef STRS_UNIQUENESS_TRACKING
//! Counter of usages per every string
static std::vector<int> g_LevelStringUsages;
//! String-To-Index map
static std::unordered_map<std::string, int> g_uniqueStringsIds;
#endif

#ifdef STRS_UNIQUENESS_TRACKING
static std::vector<std::string> g_LevelString_backup;
static std::vector<int> g_LevelStringUsages_backup;
static std::vector<int> g_FreeIndexes_backup;
static std::unordered_map<std::string, int> g_uniqueStringsIds_backup;
#endif



size_t StringsBankSize()
{
    return g_LevelString.size();
}

size_t StringsUnusedEntries()
{
    return g_FreeIndexes.size();
}

void SaveWorldStrings()
{
#ifdef STRS_UNIQUENESS_TRACKING
    g_LevelString_backup = g_LevelString;
    g_LevelStringUsages_backup = g_LevelStringUsages;
    g_FreeIndexes_backup = g_FreeIndexes;
    g_uniqueStringsIds_backup = g_uniqueStringsIds;
#endif

    g_numWorldString = g_LevelString.size();
}

void RestoreWorldStrings()
{
    EditorCursor.ClearStrings();

#ifdef STRS_UNIQUENESS_TRACKING
    g_LevelString = g_LevelString_backup;
    g_LevelStringUsages = g_LevelStringUsages_backup;
    g_FreeIndexes = g_FreeIndexes_backup;
    g_uniqueStringsIds = g_uniqueStringsIds_backup;
#else
    for(auto i = g_FreeIndexes.begin(); i != g_FreeIndexes.end(); )
    {
        if(*i >= (int)g_numWorldString)
            i = g_FreeIndexes.erase(i);
        else
            ++i;
    }
    g_LevelString.resize(g_numWorldString);
#endif
}

void ClearStringsBank()
{
    EditorCursor.ClearStrings();

    g_LevelString.clear();
    g_numWorldString = 0;

#ifdef STRS_UNIQUENESS_TRACKING
    g_LevelStringUsages.clear();
    g_uniqueStringsIds.clear();
    g_FreeIndexes.clear();
#endif
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
#ifdef STRS_UNIQUENESS_TRACKING
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
#else
        SDL_assert_release(index < g_LevelString.size());
        g_LevelString[index] = target;
#endif
    }

    if(stringEmpty)
    {
#ifdef STRS_UNIQUENESS_TRACKING
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
#else
        if(g_FreeIndexes.empty())
        {
            index = (stringindex_t)g_LevelString.size();
            g_LevelString.push_back(target);
        }
        else
        {
            index = g_FreeIndexes.back();
            g_FreeIndexes.pop_back();
            g_LevelString[index] = target;
        }
#endif
    }
}

stringindex_t AllocS(const std::string& target)
{
    stringindex_t out = STRINGINDEX_NONE;
    SetS(out, target);
    return out;
}

void FreeS(stringindex_t& index)
{
    if(index == STRINGINDEX_NONE)
        return;

    SDL_assert_release(index <= STRINGINDEX_NONE);
    SDL_assert_release(index < g_LevelString.size());

#ifdef STRS_UNIQUENESS_TRACKING
    g_LevelStringUsages[index]--;
    if(g_LevelStringUsages[index] == 0)
    {
        g_uniqueStringsIds.erase(g_LevelString[index]);
        g_LevelString[index].clear();
        g_FreeIndexes.push_back(index);
    }
#else
    g_FreeIndexes.push_back(index);
#endif

    index = STRINGINDEX_NONE;
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
