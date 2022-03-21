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

//////////////////// THIS IS CURRENTLY AN UNUSED PROOF OF CONCEPT


//// Four options:
////   force 800x600
////   visibility-based
////   tile-based (no more tiles == no more display)
////   force screen-res

//// plus, force-center (on/off)


#include <vector>
#include <algorithm>

bool do_center = false;

Location_t visible_map;

struct WorldPathRef_t
{
    const int16_t index;

    WorldPathRef_t(int16_t i) : index(i) {}

    operator int16_t() const { return index; }

    WorldPath_t& operator*() const { return WorldPath[index]; }
    WorldPath_t* operator->() const { return &WorldPath[index]; }
};

struct WorldLevelRef_t
{
    const int16_t index;

    WorldLevelRef_t(int16_t i) : index(i) {}

    operator int16_t() const { return index; }

    WorldLevel_t& operator*() const { return WorldLevel[index]; }
    WorldLevel_t* operator->() const { return &WorldLevel[index]; }
};

Location_t CheckVisibleRange(const std::vector<WorldPathRef_t>& paths, const std::vector<WorldLevelRef_t>& levels)
{
    double cX = WorldPlayer[1].Location.X + WorldPlayer[1].Location.Width / 2.0;
    double cY = WorldPlayer[1].Location.Y + WorldPlayer[1].Location.Height / 2.0;

    cX = (std::round(cX / 32. - 0.5) + 0.5) * 32.;
    cY = (std::round(cY / 32. - 0.5) + 0.5) * 32.;

    int nRow = ScreenH / 32 * 2;
    int nCol = ScreenW / 32 * 2;

    int cRow = ScreenH / 32;
    int cCol = ScreenW / 32;

    std::vector<bool> visible(nRow*nCol);

    std::vector<std::pair<double, double>> locs;

    for(WorldPathRef_t p : paths)
    {
        locs.push_back({p->Location.X - p->Location.Width / 2.0, p->Location.Y - p->Location.Height / 2.0});
    }

    for(WorldLevelRef_t l : levels)
    {
        locs.push_back({l->Location.X - l->Location.Width / 2.0, l->Location.Y - l->Location.Height / 2.0});
    }

    for(std::pair<double, double>& loc : locs)
    {
        // canonically, the horiz. camera when a player is at a loc extends -334 +334 (10.4 tiles)
        // canonically, the vert.  camera when a player is at a loc extends -202 +202 ( 6.3 tiles)
        double path_cX = loc.first;
        double path_cY = loc.second;

        int path_row = cRow + (int)(path_cY - cY) / 32;
        int path_col = cCol + (int)(path_cX - cX) / 32;

        int start_col = path_col - 11;
        int lim_col = path_col + 12;

        if(start_col >= nCol)
            continue;
        if(lim_col <= 0)
            continue;

        if(start_col < 0)
            start_col = 0;
        if(lim_col > nCol)
            lim_col = nCol;

        for(int row = path_row - 7; row <= path_row + 7; row++)
        {
            if(row >= nRow || row < 0)
                continue;

            std::fill(visible.begin() + row * nCol + start_col, visible.begin() + row * nCol + lim_col, true);
        }
    }

    // find largest square within visible

    std::vector<int> row_left_limit(nRow);
    std::vector<int> row_right_limit(nRow);
    for(int r = 0; r < nRow; r++)
    {
        auto start_point = visible.begin() + r * nCol + cCol;
        row_right_limit[r] = std::find(start_point, start_point + cCol, false) - start_point;

        auto start_point_r = visible.rbegin() + ((nRow*nCol - 1) - (r * nCol + cCol));
        row_left_limit[r] = std::find(start_point_r, start_point_r + cCol, false) - start_point_r;
    }

    // make them cumulative
    for(int r = cRow - 1; r >= 0; r--)
    {
        if(row_left_limit[r] > row_left_limit[r+1])
            row_left_limit[r] = row_left_limit[r+1];
        if(row_right_limit[r] > row_right_limit[r+1])
            row_right_limit[r] = row_right_limit[r+1];
    }
    for(int r = cRow + 1; r < nRow; r++)
    {
        if(row_left_limit[r] > row_left_limit[r-1])
            row_left_limit[r] = row_left_limit[r-1];
        if(row_right_limit[r] > row_right_limit[r-1])
            row_right_limit[r] = row_right_limit[r-1];
    }

    // pick which rows to include in order to maximize size
    // int best_left_col = cCol - 10;
    // int best_cols = 10;

    int best_top = 0;
    int best_bottom = 0;
    int best_left = 0;
    int best_right = 0;
    int best_area = 0;
    int best_eccen = 0;
    for(int start_row = 0; start_row < nRow; start_row++)
    {
        int left_col = row_left_limit[start_row];
        int right_col = row_right_limit[start_row];
        int top_row;
        int bottom_row;
        if(do_center)
        {
            if(start_row > cRow)
                continue;
            top_row = start_row;
            bottom_row = nRow - top_row;
            if(row_left_limit[bottom_row] < left_col)
                left_col = row_left_limit[bottom_row];
            if(row_right_limit[bottom_row] < right_col)
                right_col = row_right_limit[bottom_row];

            if(left_col < right_col)
                right_col = left_col;
            else
                left_col = right_col;
        }
        else if(start_row <= cRow)
        {
            top_row = start_row;
            for(bottom_row = cRow; bottom_row < nRow; bottom_row++)
            {
                if(row_left_limit[bottom_row] < left_col)
                    break;
                if(row_right_limit[bottom_row] < right_col)
                    break;
            }
            bottom_row--;
        }
        else
        {
            bottom_row = start_row;
            for(top_row = cRow; top_row >= 0; top_row--)
            {
                if(row_left_limit[top_row] < left_col)
                    break;
                if(row_right_limit[top_row] < right_col)
                    break;
            }
            top_row++;
        }

        // convert into pixel coordinates relative to player, make the right and bottom values upper bounds.
        left_col = (left_col - 1) * 32;
        right_col = (right_col - 1) * 32 + 1;
        top_row = (cRow - top_row) * 32;
        bottom_row = (bottom_row - cRow) * 32 + 1;

        int visible_w = ScreenW - 128;
        int visible_h = ScreenH - 192;

        // move viewport to center player if possible
        if(bottom_row + top_row > visible_h)
        {
            if(top_row >= visible_h/2 && bottom_row > visible_h/2)
            {
                top_row = visible_h/2;
                bottom_row = visible_h - top_row;
            }
            else if(bottom_row > visible_h/2)
            {
                bottom_row = visible_h - top_row;
            }
            else if(top_row >= visible_h/2)
            {
                top_row = visible_h - bottom_row;
            }
        }

        if(left_col + right_col > visible_w)
        {
            if(left_col >= visible_w/2 && right_col > visible_w/2)
            {
                left_col = visible_w/2;
                right_col = visible_w - left_col;
            }
            else if(right_col > visible_w/2)
            {
                right_col = visible_w - left_col;
            }
            else if(left_col >= visible_w/2)
            {
                left_col = visible_w - right_col;
            }
        }

        int area = (bottom_row + top_row) * (left_col + right_col);
        int eccen = (top_row - bottom_row) * (top_row - bottom_row)
            + (left_col - right_col) * (left_col - right_col);

        // printf("t, b, l, r, a: %d, %d, %d, %d, %d, %d\n", top_row, bottom_row, left_col, right_col, area, eccen);

        // incentivize player being close to the center also
        if(area - eccen > best_area - best_eccen)
        {
            best_top = top_row;
            best_bottom = bottom_row;
            best_left = left_col;
            best_right = right_col;
            best_area = area;
            best_eccen = eccen;
        }
    }

    // printf("Chose: t, b, l, r, a: %d, %d, %d, %d, %d, %d\n\n", best_top, best_bottom, best_left, best_right, best_area, best_eccen);

#if 0
    for(int r = 0; r < nRow; r++)
    {
        for(int c = 0; c < nCol; c++)
        {
            if(r == cRow && c == cCol)
                printf("x");
            else if((cRow - r)*32 <= best_top && (r - cRow)*32 < best_bottom && (cCol - c)*32 <= best_left && (c - cCol)*32 < best_right)
                printf(":");
            else if(visible[r * nCol + c])
                printf(".");
            else
                printf(" ");
        }
        printf(" %d %d\n", row_left_limit[r], row_right_limit[r]);
    }
#endif

    // for(int left_row)
    return newLoc(cX - best_left, cY - best_top, best_right + best_left, best_bottom + best_top);
}

std::pair<std::vector<WorldPathRef_t>, std::vector<WorldLevelRef_t>> FindPaths()
{
    std::vector<bool> path_in_ret(maxWorldPaths + 1);
    std::vector<bool> level_in_ret(maxWorldLevels + 1);

    std::vector<WorldPathRef_t> path_ret;
    std::vector<WorldLevelRef_t> level_ret;
    int path_expanded = 0;
    int level_expanded = 0;

    Location_t tempLocation = WorldPlayer[1].Location;

    tempLocation.X += 4;
    tempLocation.Y += 4;
    tempLocation.Width -= 8;
    tempLocation.Height -= 8;
    for(auto *t : treeWorldPathQuery(tempLocation, false))
    {
        WorldPathRef_t path = t - &WorldPath[1] + 1;
        if(path->Active && CheckCollision(tempLocation, path->Location))
        {
            if(!path_in_ret[path])
            {
                path_in_ret[path] = true;
                path_ret.push_back(path);
            }
        }
    }
    for(auto *t : treeWorldLevelQuery(tempLocation, false))
    {
        WorldLevelRef_t level = t - &WorldLevel[1] + 1;
        if(level->Active && CheckCollision(tempLocation, level->Location))
        {
            if(!level_in_ret[level])
            {
                level_in_ret[level] = true;
                level_ret.push_back(level);
            }
        }
    }

    while(path_expanded != path_ret.size() || level_expanded != level_ret.size())
    {
        if(path_expanded != path_ret.size())
        {
            WorldPathRef_t to_expand = path_ret[path_expanded];
            tempLocation = to_expand->Location;
            path_expanded++;
        }
        else
        {
            WorldLevelRef_t to_expand = level_ret[level_expanded];
            tempLocation = to_expand->Location;
            level_expanded++;
        }

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
                if(path->Active && CheckCollision(tempLocation, path->Location))
                {
                    if(!path_in_ret[path])
                    {
                        path_in_ret[path] = true;
                        path_ret.push_back(path);
                    }
                }
            }
            for(auto *t : treeWorldLevelQuery(tempLocation, false))
            {
                WorldLevelRef_t level = t - &WorldLevel[1] + 1;
                if(level->Active && CheckCollision(tempLocation, level->Location))
                {
                    if(!level_in_ret[level])
                    {
                        level_in_ret[level] = true;
                        level_ret.push_back(level);
                    }
                }
            }
        }
    }

    return {path_ret, level_ret};
}

Location_t AutoCheckVisibleRange()
{
    auto ret = FindPaths();
    return CheckVisibleRange(ret.first, ret.second);
}

void ResetWorldMapScreen()
{
    visible_map = AutoCheckVisibleRange();
}

void UpdateWorldMapScreen()
{
    Location_t cur_loc = AutoCheckVisibleRange();

    double moverate = 2;

    if(WalkAnywhere || (g_config.worldMapFastMove && g_speedRunnerMode < SPEEDRUN_MODE_2))
        moverate = 4;

    //cur_loc;
    // take the slower option of 2px per second camera (vanilla)
    //   or 2px per second resize, then scale the speed of the faster one to match
    double camRateX = moverate;
    double camRateY = moverate;

    double resizeRateX = moverate;
    double resizeRateY = moverate;

    double camFramesX = std::abs(cur_loc.X - visible_map.X)/camRateX;
    double camFramesY = std::abs(cur_loc.Y - visible_map.Y)/camRateY;
    double resizeFramesX = std::abs(cur_loc.Width - visible_map.Width)/resizeRateX;
    double resizeFramesY = std::abs(cur_loc.Height - visible_map.Height)/resizeRateY;
    double qFramesX = (camFramesX > resizeFramesX ? camFramesX : resizeFramesX);
    double qFramesY = (camFramesY > resizeFramesY ? camFramesY : resizeFramesY);

    // base frames solely on camera
    // qFramesX = camFramesX;
    // qFramesY = camFramesY;
    if(qFramesX < qFramesY)
        qFramesX = qFramesY;
    else
        qFramesY = qFramesX;

    // don't continue after this frame if it would arrive next frame
    // (this is equivalent to the <5 condition in the vanilla game)

    if (qFramesX < 1) qFramesX = 1;
    if (qFramesY < 1) qFramesY = 1;

    camRateX = std::abs(cur_loc.X - visible_map.X)/qFramesX;
    camRateY = std::abs(cur_loc.Y - visible_map.Y)/qFramesY;

    resizeRateX = std::abs(cur_loc.Width - visible_map.Width)/qFramesX;
    resizeRateY = std::abs(cur_loc.Height - visible_map.Height)/qFramesY;

    if(cur_loc.X < visible_map.X - camRateX)
        visible_map.X = visible_map.X - camRateX;
    else if(cur_loc.X > visible_map.X + camRateX)
        visible_map.X = visible_map.X + camRateX;
    else
        visible_map.X = cur_loc.X;
    if(cur_loc.Y < visible_map.Y - camRateY)
        visible_map.Y = visible_map.Y - camRateY;
    else if(cur_loc.Y > visible_map.Y + camRateY)
        visible_map.Y = visible_map.Y + camRateY;
    else
        visible_map.Y = cur_loc.Y;

    if(cur_loc.Width < visible_map.Width - resizeRateX)
        visible_map.Width -= resizeRateX;
    else if(cur_loc.Width > visible_map.Width + resizeRateX)
        visible_map.Width += resizeRateX;
    else
        visible_map.Width = cur_loc.Width;
    if(cur_loc.Height < visible_map.Height - resizeRateY)
        visible_map.Height -= resizeRateY;
    else if(cur_loc.Height > visible_map.Height + resizeRateY)
        visible_map.Height += resizeRateY;
    else
        visible_map.Height = cur_loc.Height;
}