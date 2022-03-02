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
#include <unordered_map>
#include <SDL2/SDL_assert.h>

extern const std::string g_emptyString;

extern void SaveWorldStrings();
extern void RestoreWorldStrings();
extern void ClearStringsBank();

/*!
 * \brief Get string from the bank by index
 * \param index Index of string
 * \return Const referrence to the actual string
 */
extern const std::string& GetS(stringindex_t index);

/*!
 * \brief Set the string to the index
 * \param index destinition string field
 * \param target Target string data to assign
 */
extern void SetS(stringindex_t& index, const std::string& target);

/*!
 * \brief Get string as pointer from the bank by index
 * \param index Index of string
 * \return Pointer to the actual string
 */
extern std::string* PtrS(stringindex_t& index);

#endif // #ifndef GLOBAL_STRINGS_H
