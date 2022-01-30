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
#ifndef LUNAMISC_H
#define LUNAMISC_H

#include <map>
#include <string>

extern void InitIfMissing(std::map<std::string, double>* pMap, const std::string& sought_key, double init_val);

extern bool FastTestCollision(int Left1, int Up1, int Right1, int Down1, int Left2, int Up2, int Right2, int Down2);

// Compute the current SMBX level section for the given coords, or -1 if invalid
extern int ComputeLevelSection(int x, int y);

extern void RandomPointInRadius(double* ox, double* oy, double cx, double cy, int radius);

extern std::string resolveIfNotAbsolutePath(const std::string &filename);

#endif // LUNAMISC_H
