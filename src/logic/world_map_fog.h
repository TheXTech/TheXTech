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

#ifndef WORLD_MAP_FOG_H
#define WORLD_MAP_FOG_H

#include <bitset>
#include <vector>
#include <utility>

#include "globals.h"

class WorldMapFog
{
private:
    std::bitset<maxWorldPaths + 1> m_path_active;
    std::bitset<maxWorldLevels + 1> m_level_active;

    std::vector<WorldPathRef_t> m_active_paths;
    std::vector<WorldLevelRef_t> m_active_levels;

    std::vector<std::pair<double, double>> m_best_pans;

    void RevealLoc(double x, double y);
    std::pair<double, double> FindCenterOfVisibleMass(double x, double y);

public:
    double m_map_left = 0.;
    double m_map_top = 0.;
    int m_map_cols = 0;
    int m_map_rows = 0;

    const int m_tile_size = 16;
    const int m_tiles_per_zone = 16;
    const int8_t m_fog_levels = 6;

    bool m_ready = false;
    bool m_active = false;

    std::vector<int8_t> m_fog_alpha;

    void Update();
    std::pair<double, double> GetPan(double cx, double cy);
};

// defined in world_map_visibility.cpp
extern WorldMapFog g_worldMapFog;

#endif // #ifndef WORLD_MAP_FOG_H
