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
#ifndef LUNALEVEL_H
#define LUNALEVEL_H

#include <string>
#include "lunadefs.h"

namespace LevelF
{

double GetBoundary(int section, int which_boundary_UDLR);
void GetBoundary(LunaRect* rectangle, int section);
void PushSectionBoundary(int section, int which_boundary_UDLR, double push_val);
void SetSectionBounds(int section, double left_bound, double top_bound, double right_bound, double bot_bound);

std::string GetName();

}

#endif // LUNALEVEL_H
