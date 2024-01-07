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
#ifndef LANGUAGE_HHHH
#define LANGUAGE_HHHH

#include <string>

namespace XLanguage
{


/*!
 * \brief Finds present and non-empty language translations (called during engine load)
 */
void findLanguages();


/*!
 * \brief Detect system language and set global variables
 */
void resolveLanguage(const std::string& requestedLanguage);


/*!
 * \brief Replaces string with next available language (slow)
 * Uses global state: CurrentLanguage, CurrentLangDialect
 */
void rotateLanguage(std::string& nextLanguage, int step = 1);


void splitRegion(char delimiter);


const std::string &getEngineFile();


const std::string &getAssetsFile();


} // namespace XLanguage

#endif // LANGUAGE_HHHH
