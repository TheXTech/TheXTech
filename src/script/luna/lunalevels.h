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
#ifndef LUNALEVELS_H
#define LUNALEVELS_H

/*!
 * \brief Initialize level custom codes and attach to current runner
 */
extern void lunaLevelsInit();

/*!
 * \brief Remove the custom code attachment
 */
extern void lunaLevelsClear();

/*!
 * \brief Process one step of level custom code if attached
 */
extern void lunaLevelsDo();

#endif // LUNALEVELS_H
