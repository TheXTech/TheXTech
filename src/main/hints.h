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
#ifndef THEXTECH_HINTS_H
#define THEXTECH_HINTS_H

#include <string>

class XTechTranslate;

namespace XHints
{

/**
 * \brief resets hint strings to their English values
 **/
void ResetStrings();

/**
 * \brief registers hint strings for translation
 **/
void InitTranslations(XTechTranslate& translate);

/**
 * \brief select a hint based on the currently loaded level
 **/
void Select();

/**
 * \brief draws a hint chosen with SelectHint() on the screen
 *
 * \param top top y coordinate for hint box
 * \param min_priority required priority for the hint to get drawn
 * \param width width of hint box
 **/
void Draw(int top, int min_priority, int width = 460);

} // namespace XHints

#endif // #ifndef THEXTECH_HINTS_H
