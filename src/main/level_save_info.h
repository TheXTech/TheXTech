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

LevelSaveInfo_t InitLevelSaveInfo(LevelData& loadedLevel);
LevelSaveInfo_t InitLevelSaveInfo(const std::string& fullPath, LevelData& tempData);
void ImportLevelSaveInfo(const GamesaveData& s);
void ExportLevelSaveInfo(GamesaveData& s);

#endif // #ifndef LEVEL_SAVE_INFO_H
