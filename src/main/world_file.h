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
#ifndef WORLD_FILE_H
#define WORLD_FILE_H

#include <string>

//! loads the world
bool OpenWorld(std::string FilePath);
void FindWldStars();
//! clears the world.
//  when "quick" is true, does not unload custom assets. this is used only for creating new worlds at menu.
void ClearWorld(bool quick = false);

//! NEW: routines to check if it is possible to convert to legacy file formats and to remove all non-legacy content
bool CanConvertWorld(int format, std::string* reasons);
void ConvertWorld(int format);

#endif // WORLD_FILE_H
