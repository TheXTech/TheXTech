/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2021 Vitaly Novichkov <admin@wohlnet.ru>
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
#ifndef CHEAT_CODE_H
#define CHEAT_CODE_H

#include <string>

/*!
 * \brief Reset all customized cheats state into default
 */
extern void cheats_reset();


enum CheatsScope
{
    CHEAT_SCOPE_GLOBAL = 0,
    CHEAT_SCOPE_WORLD,
    CHEAT_SCOPE_LEVEL
};

/*!
 * \brief Add alias to the existing cheat code
 * \param scope Cheats scope
 * \param source Source name of the cheat
 * \param alias New alias name
 */
extern void cheats_addAlias(CheatsScope scope,
                            const std::string &source,
                            const std::string &alias);

/*!
 * \brief Rename existing cheat into something other
 * \param scope Cheats scope
 * \param source Original name of cheat to rename
 * \param alias New name for the cheat
 */
extern void cheats_rename(CheatsScope scope,
                          const std::string &source,
                          const std::string &alias);

/*!
 * \brief Erase cheat code by name
 * \param scope Cheats scope
 * \param source Cheat to erase
 */
extern void cheats_erase(CheatsScope scope, const std::string &source);


extern void cheats_setBuffer(const std::string &line);

extern void cheats_clearBuffer();

/*!
 * \brief Adds one charracter into the cheat buffer and executes cheat if buffer has enough
 * \param sym ASCII character to add into the cheat buffer
 */
extern void CheatCode(char sym);

#endif // CHEAT_CODE_H
