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

#include "logic/world_map_fog.h"
#include "main/trees.h"
#include "collision.h"
#include "config.h"

#include <algorithm>

void WorldMapFog::Update()
{
    m_ready = false;
    m_active = false;

    if(!LevelSelect || GameMenu)
        return;

    if(g_config.world_map_fog == Config_t::WORLD_MAP_FOG_OFF && !g_config.world_map_smart_pan)
        return;

    bool spoilers = (g_config.world_map_fog == Config_t::WORLD_MAP_FOG_SPOILERS);

    m_path_active.reset();
    m_level_active.reset();

    m_active_paths.clear();
    m_active_levels.clear();

    Location_t tempLocation = WorldPlayer[1].Location;

    // add the locations that the player collides with to the active set
    tempLocation.X += 4;
    tempLocation.Y += 4;
    tempLocation.Width -= 8;
    tempLocation.Height -= 8;
    for(auto *t : treeWorldPathQuery(tempLocation, false))
    {
        WorldPathRef_t path = t - &WorldPath[1] + 1;
        if((spoilers || path->Active) && CheckCollision(tempLocation, path->Location))
        {
            if(!m_path_active[path])
            {
                m_path_active[path] = true;
                m_active_paths.push_back(path);
            }
        }
    }
    for(auto *t : treeWorldLevelQuery(tempLocation, false))
    {
        WorldLevelRef_t level = t - &WorldLevel[1] + 1;
        if((spoilers || level->Active) && CheckCollision(tempLocation, level->Location))
        {
            if(!m_level_active[level])
            {
                m_level_active[level] = true;
                m_active_levels.push_back(level);
            }
        }
    }

    // find the bounds of all locations the player could have
    double left = tempLocation.X + tempLocation.Width / 2;
    double top = tempLocation.Y + tempLocation.Height / 2;
    double right = tempLocation.X + tempLocation.Width / 2;
    double bottom = tempLocation.Y + tempLocation.Height / 2;

    // expand the locations, add the locations that the player collides with to the active set
    unsigned int path_expanded = 0;
    unsigned int level_expanded = 0;

    while(path_expanded != m_active_paths.size() || level_expanded != m_active_levels.size())
    {
        if(path_expanded != m_active_paths.size())
        {
            WorldPathRef_t to_expand = m_active_paths[path_expanded];
            tempLocation = to_expand->Location;
            path_expanded++;
        }
        else
        {
            WorldLevelRef_t to_expand = m_active_levels[level_expanded];
            tempLocation = to_expand->Location;
            level_expanded++;
        }

        // update bounds
        if(tempLocation.X + tempLocation.Width / 2 < left)
            left = tempLocation.X + tempLocation.Width / 2;
        if(tempLocation.X + tempLocation.Width / 2 > right)
            right = tempLocation.X + tempLocation.Width / 2;
        if(tempLocation.Y + tempLocation.Height / 2 < top)
            top = tempLocation.Y + tempLocation.Height / 2;
        if(tempLocation.Y + tempLocation.Height / 2 > bottom)
            bottom = tempLocation.Y + tempLocation.Height / 2;

        // expand in four directions
        for(int B = 1; B <= 4; B++)
        {
            if(B == 1)
                tempLocation.Y -= 32; // Up
            else if(B == 2)
            {
                tempLocation.Y += 32; // Down
                tempLocation.X -= 32; // Left
            }
            else if(B == 3)
            {
                tempLocation.X += 32; // Right
                tempLocation.Y += 32; // Down
            }
            else if(B == 4)
            {
                tempLocation.Y -= 32; // Up
                tempLocation.X += 32; // Right
            }

            for(auto *t : treeWorldPathQuery(tempLocation, false))
            {
                WorldPathRef_t path = t - &WorldPath[1] + 1;
                if((spoilers || path->Active) && CheckCollision(tempLocation, path->Location))
                {
                    if(!m_path_active[path])
                    {
                        m_path_active[path] = true;
                        m_active_paths.push_back(path);
                    }
                }
            }
            for(auto *t : treeWorldLevelQuery(tempLocation, false))
            {
                WorldLevelRef_t level = t - &WorldLevel[1] + 1;
                if((spoilers || level->Active) && CheckCollision(tempLocation, level->Location))
                {
                    if(!m_level_active[level])
                    {
                        m_level_active[level] = true;
                        m_active_levels.push_back(level);
                    }
                }
            }
        }
    }

    // form the alpha map, to the desired resolution
    double alpha_map_left = left - ScreenW / 2;
    alpha_map_left = std::floor(alpha_map_left / m_tile_size) * m_tile_size;
    double alpha_map_right = right + ScreenW / 2;
    alpha_map_right = std::ceil(alpha_map_right / m_tile_size) * m_tile_size;

    double alpha_map_top = top - ScreenH / 2;
    alpha_map_top = std::floor(alpha_map_top / m_tile_size) * m_tile_size;
    double alpha_map_bottom = bottom + ScreenH / 2;
    alpha_map_bottom = std::ceil(alpha_map_bottom / m_tile_size) * m_tile_size;

    m_map_left = alpha_map_left;
    m_map_top = alpha_map_top;

    m_map_cols = (alpha_map_right - alpha_map_left) / m_tile_size;
    m_map_rows = (alpha_map_bottom - alpha_map_top) / m_tile_size;

    m_fog_alpha.resize(m_map_cols * m_map_rows);
    std::fill(m_fog_alpha.begin(), m_fog_alpha.end(), m_fog_levels);

    // for cheating player after using ParkingLot
    RevealLoc(WorldPlayer[1].Location.X + WorldPlayer[1].Location.Width / 2.0, WorldPlayer[1].Location.Y + WorldPlayer[1].Location.Height / 2.0);

    // show all the reachable places
    for(WorldPathRef_t p : m_active_paths)
    {
        RevealLoc(p->Location.X + p->Location.Width / 2.0, p->Location.Y + p->Location.Height / 2.0);
    }

    for(WorldLevelRef_t l : m_active_levels)
    {
        RevealLoc(l->Location.X + l->Location.Width / 2.0, l->Location.Y + l->Location.Height / 2.0);
    }

    // calculate the optimal pans based on player position
    if(g_config.world_map_smart_pan)
    {
        const int pan_cols = m_map_rows / m_tiles_per_zone + 1;
        const int pan_rows = m_map_cols / m_tiles_per_zone + 1;
        const int zone_size = m_tile_size * m_tiles_per_zone;
        m_best_pans.resize(pan_cols * pan_rows);

        for(int row = 0; row < pan_rows; row++)
        {
            double cy = m_map_top + row * zone_size;
            for(int col = 0; col < pan_cols; col++)
            {
                double cx = m_map_left + col * zone_size;
                m_best_pans[row * pan_cols + col] = FindCenterOfVisibleMass(cx, cy);
            }
        }
    }

    m_ready = true;

    if(ScreenW <= 800 && ScreenH <= 600)
        return;

    if(g_config.world_map_fog == Config_t::WORLD_MAP_FOG_OFF)
        return;

    m_active = true;
}

void WorldMapFog::RevealLoc(double x, double y)
{
    // canonically, the horiz. camera when a player is at a loc extends -334 +334
    // canonically, the vert.  camera when a player is at a loc extends -202 +202
    double bound_left = x - 334 - (m_fog_levels * m_tile_size / 2);
    double bound_right = x + 334 + (m_fog_levels * m_tile_size / 2);
    double bound_top = y - 202 - (m_fog_levels * m_tile_size / 2);
    double bound_bottom = y + 202 + (m_fog_levels * m_tile_size / 2);

    int start_col = std::floor((bound_left - m_map_left) / m_tile_size);
    int limit_col = std::ceil((bound_right - m_map_left) / m_tile_size);

    int start_row = std::floor((bound_top - m_map_top) / m_tile_size);
    int limit_row = std::ceil((bound_bottom - m_map_top) / m_tile_size);

    for(int row = start_row; row < limit_row; row++)
    {
        if(row < 0 || row >= m_map_rows)
            continue;

        int row_from_start = row - start_row + 1;
        int row_from_limit = limit_row - row;

        int row_interior = row_from_start < row_from_limit ? row_from_start : row_from_limit;

        if(row_interior > m_fog_levels)
            row_interior = m_fog_levels;

        for(int col = start_col; col < limit_col; col++)
        {
            if(col < 0 || col >= m_map_cols)
                continue;

            int col_from_start = col - start_col + 1;
            int col_from_limit = limit_col - col;

            int col_interior = col_from_start < col_from_limit ? col_from_start : col_from_limit;

            if(col_interior > m_fog_levels)
                col_interior = m_fog_levels;

            int total_interior = row_interior + col_interior;
            total_interior -= m_fog_levels;
            if(total_interior < 0)
                total_interior = 0;

            int8_t fog_alpha = m_fog_levels - total_interior;

            if(m_fog_alpha[row * m_map_cols + col] > fog_alpha)
            {
                m_fog_alpha[row * m_map_cols + col] = fog_alpha;
            }
        }
    }
}

std::pair<double, double> WorldMapFog::FindCenterOfVisibleMass(double cx, double cy)
{
    double start_x = cx - ScreenW + 66 + 66;
    int start_col = std::floor((start_x - m_map_left) / m_tile_size);
    double limit_x = cx + ScreenW - 66 - 66;
    int limit_col = std::ceil((limit_x - m_map_left) / m_tile_size);

    double start_y = cy - (ScreenH - 66 - 130);
    int start_row = std::floor((start_y - m_map_top) / m_tile_size);
    double limit_y = cy + (ScreenH - 66 - 130);
    int limit_row = std::ceil((limit_y - m_map_top) / m_tile_size);

    long cum_row = 0;
    long cum_col = 0;
    long divisor = 0;

    for(int row = start_row; row < limit_row; row++)
    {
        if(row < 0 || row >= m_map_rows)
            continue;

        int row_from_start = row - start_row + 1;
        int row_from_limit = limit_row - row;

        int row_interior = row_from_start < row_from_limit ? row_from_start : row_from_limit;

        if(row_interior > m_fog_levels)
            row_interior = m_fog_levels;

        for(int col = start_col; col < limit_col; col++)
        {
            if(col < 0 || col >= m_map_cols)
                continue;

            int visibility = m_fog_levels - m_fog_alpha[row * m_map_cols + col];

            divisor += visibility;
            cum_row += visibility * row;
            cum_col += visibility * col;
        }
    }

    if(divisor == 0)
    {
        // printf("no way %d %d %d %d %d %d\n", start_row, limit_row, m_map_rows, start_col, limit_col, m_map_cols);
        return {cx, cy};
    }

    double rx = (cum_col / divisor) * m_tile_size + m_map_left;
    double ry = (cum_row / divisor) * m_tile_size + m_map_top;

    return {rx, ry};
}

std::pair<double, double> WorldMapFog::GetPan(double cx, double cy)
{
    if(!m_ready || !g_config.world_map_smart_pan)
        return {cx, cy};

    const int pan_cols = m_map_rows / m_tiles_per_zone + 1;
    const int pan_rows = m_map_cols / m_tiles_per_zone + 1;
    const int zone_size = m_tile_size * m_tiles_per_zone;

    int col_1 = std::floor((cx - m_map_left) / zone_size);
    int col_2 = col_1 + 1;

    int row_1 = std::floor((cy - m_map_top) / zone_size);
    int row_2 = row_1 + 1;

    auto tl = m_best_pans[row_1 * pan_cols + col_1];
    auto tr = m_best_pans[row_1 * pan_cols + col_2];
    auto bl = m_best_pans[row_2 * pan_cols + col_1];
    auto br = m_best_pans[row_2 * pan_cols + col_2];

    if(col_1 < 0 || col_2 >= pan_cols || row_1 < 0 || row_2 >= pan_rows)
        return {cx, cy};

    double col_coord = (cx - m_map_left - col_1 * zone_size) / zone_size;
    double row_coord = (cy - m_map_top - row_1 * zone_size) / zone_size;

    double ret_x = tl.first * (1 - row_coord) * (1 - col_coord)
        + tr.first * (1 - row_coord) * col_coord
        + bl.first * row_coord * (1 - col_coord)
        + br.first * row_coord * col_coord;

    double ret_y = tl.second * (1 - row_coord) * (1 - col_coord)
        + tr.second * (1 - row_coord) * col_coord
        + bl.second * row_coord * (1 - col_coord)
        + br.second * row_coord * col_coord;

    return {ret_x, ret_y};
}

WorldMapFog g_worldMapFog;
