/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2024 Vitaly Novichkov <admin@wohlnet.ru>
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

#ifndef LEVEL_SAVE_INFO_H
#define LEVEL_SAVE_INFO_H

#include <string>
#include <bitset>

#include <PGE_File_Formats/file_formats.h>

#include "globals.h"

struct SaveInfoInit
{
    LevelSaveInfo_t* target = nullptr;
    std::bitset<c_max_track_medals> used_indexes;

    // resets target's fields to begin initialization
    void begin(LevelSaveInfo_t* target);

    // marks target as invalid on load error
    void on_error();

#ifdef PGEFL_CALLBACK_API
    // updates star count from header
    void check_head(const LevelHead& head);
#else
    // updates star count from header
    void check_head(const LevelData& head);
#endif

    // updates medal count from NPC
    void check_npc(const LevelNPC& head);
};

/**
 * \brief convenience function: initialize level save info from fullPath, loading the level into tempData
 *
 * \param fullPath a full path to the level file whose save info will be initialized
 * \param tempData an unloaded LevelData object used to prevent repeated heap allocations while loading multiple levels
 *
 * \return loaded save info
 */
LevelSaveInfo_t InitLevelSaveInfo(const std::string& fullPath, LevelData& tempData);

/**
 * \brief import all level save info into WorldLevel and LevelWarpSaveEntries from a GamesaveData object
 * \param s a PGE-FL GamesaveData object from which level save info will be loaded to global arrays
 */
void ImportLevelSaveInfo(const GamesaveData& s);

/**
 * \brief export all level save info from WorldLevel and LevelWarpSaveEntries to a GamesaveData object
 * \param s a mutable PGE-FL GamesaveData object to which level save info will be stored from global arrays
 */
void ExportLevelSaveInfo(GamesaveData& s);

#endif // #ifndef LEVEL_SAVE_INFO_H
