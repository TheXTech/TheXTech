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

#ifndef LEVEL_SAVE_INFO_H
#define LEVEL_SAVE_INFO_H

#include <string>
#include <PGE_File_Formats/file_formats.h>

#include "globals.h"

//! initialize level save info from a loaded LevelData object
LevelSaveInfo_t InitLevelSaveInfo(const LevelData& loadedLevel);

//! convenience function: initialize level save info from fullPath, loading the level into tempData
LevelSaveInfo_t InitLevelSaveInfo(const std::string& fullPath, LevelData& tempData);

//! import all level save info into WorldLevel and LevelWarpSaveEntries from a GamesaveData object
void ImportLevelSaveInfo(const GamesaveData& s);

//! export all level save info from WorldLevel and LevelWarpSaveEntries to a GamesaveData object
void ExportLevelSaveInfo(GamesaveData& s);

#endif // #ifndef LEVEL_SAVE_INFO_H
