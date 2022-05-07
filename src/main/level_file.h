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


#pragma once
#ifndef LEVEL_FILE_H
#define LEVEL_FILE_H

#include <string>
#include <PGE_File_Formats/lvl_filedata.h>

struct Background_t;

extern void bgoApplyZMode(Background_t *bgo, int smbx64sp);

extern void addMissingLvlSuffix(std::string &fileName);

//! loads the level
bool OpenLevel(std::string FilePath);
bool OpenLevelData(LevelData &lvl, const std::string FilePath = std::string());
//! Reset everything to zero
void ClearLevel();

//! checks for stars in warps the lead to another level
void FindStars();

//! NEW: routines to check if it is possible to convert to legacy file formats and to remove all non-legacy content
bool CanConvertLevel(int format, std::string* reasons);
void ConvertLevel(int format);

#endif // LEVEL_FILE_H

