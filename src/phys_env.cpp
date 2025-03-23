/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2025 Vitaly Novichkov <admin@wohlnet.ru>
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

#include "globals.h"
#include "collision.h"
#include "blocks.h"
#include "layers.h"

#include "main/trees.h"

#include "phys_env.h"

void PhysEnv_Maze(Location_t& loc, vbint_t& maze_index, uint8_t& maze_state, int npc_A, int plr_A, int speedvar, std::array<bool, 4> controls)
{
    // check if on same page with water
    if(maze_index < 1 || maze_index > numWater || Water[maze_index].Hidden)
    {
        maze_index = 0;
        return;
    }

    Water_t& cur_maze = Water[maze_index];

    double layer_speed_x = Layer[cur_maze.Layer].ApplySpeedX;
    double layer_speed_y = Layer[cur_maze.Layer].ApplySpeedY;

    loc.X += layer_speed_x;
    loc.Y += layer_speed_y;
    loc.SpeedX -= layer_speed_x;
    loc.SpeedY -= layer_speed_y;

    int direction = maze_state % 4;
    bool cleared_to_exit = maze_state & MAZE_CAN_EXIT;

    double center_x = loc.X + loc.Width / 2;
    double center_y = loc.Y + loc.Height / 2;

    double target_speed_x = 0;
    double target_speed_y = 0;

    double space_left = 0;
    double space_to_leave = 0;
    double space_to_cancel = 0;

    double exit_check_x, exit_check_y;

    // direction logic
    if(direction == MAZE_DIR_UP || direction == MAZE_DIR_DOWN)
    {
        if(direction == MAZE_DIR_UP)
        {
            exit_check_y = cur_maze.Location.Y - 16;
            space_left = center_y - cur_maze.Location.Y;
            target_speed_y = -4;
        }
        else
        {
            exit_check_y = cur_maze.Location.Y + cur_maze.Location.Height + 16;
            space_left = cur_maze.Location.Y + cur_maze.Location.Height - center_y;
            target_speed_y = 4;
        }

        space_to_cancel = cur_maze.Location.Height;
        space_to_leave = loc.Height / 2;

        double maze_center_x = cur_maze.Location.X + cur_maze.Location.Width / 2;
        exit_check_x = maze_center_x;
        target_speed_x = (maze_center_x - center_x) / 8;

        if(std::abs(target_speed_x) < 0.25)
        {
            loc.X = maze_center_x - loc.Width / 2;
            loc.SpeedX = 0;
            target_speed_x = 0;
        }
    }
    else
    {
        if(direction == MAZE_DIR_LEFT)
        {
            exit_check_x = cur_maze.Location.X - 16;
            space_left = center_x - cur_maze.Location.X;
            target_speed_x = -4;
        }
        else
        {
            exit_check_x = cur_maze.Location.X + cur_maze.Location.Width + 16;
            space_left = cur_maze.Location.X + cur_maze.Location.Width - center_x;
            target_speed_x = 4;
        }

        space_to_cancel = cur_maze.Location.Width;
        space_to_leave = loc.Width / 2;

        double maze_center_y = cur_maze.Location.Y + cur_maze.Location.Height / 2;
        exit_check_y = maze_center_y;

        if(plr_A && Player[plr_A].Mount == 3)
            maze_center_y += 8;

        target_speed_y = (maze_center_y - center_y) / 8;

        if(std::abs(target_speed_y) < 0.25)
        {
            loc.Y = maze_center_y - loc.Height / 2;
            loc.SpeedY = 0;
            target_speed_y = 0;
        }
    }

    if(cleared_to_exit)
    {
        if(space_left < -space_to_leave)
        {
            maze_index = 0;
            return;
        }

        if(direction == MAZE_DIR_UP)
            target_speed_y *= 2;
    }
    else if(space_left < 32 || space_left > space_to_cancel + 128)
    {
        // need to find a new direction
        Location_t edgeLoc;
        edgeLoc.X = exit_check_x - 16;
        edgeLoc.Y = exit_check_y - 16;
        edgeLoc.Width = 32;
        edgeLoc.Height = 32;

        cleared_to_exit = true;

        for(int B : treeBlockQuery(edgeLoc, SORTMODE_NONE))
        {
            const Block_t& b = Block[B];

            if((!npc_A || b.tempBlockNpcIdx != npc_A) && (!plr_A || !BlockCheckPlayerFilter(B, plr_A)) && !b.Hidden && !b.Invis && !BlockOnlyHitspot1[b.Type] && !BlockIsSizable[b.Type] && !BlockNoClipping[b.Type])
            {
                if(CheckCollision(edgeLoc, b.Location))
                {
                    cleared_to_exit = false;
                    break;
                }
            }
        }

        if(cleared_to_exit)
            maze_state |= MAZE_CAN_EXIT;
        else
        {
            int new_maze[4] = {0, 0, 0, 0};

            for(int W : treeWaterQuery(edgeLoc, SORTMODE_NONE))
            {
                const Water_t& w = Water[W];

                if(!w.Hidden && w.Type == PHYSID_MAZE)
                {
                    if(CheckCollision(edgeLoc, w.Location))
                    {
                        if(w.Location.Y <= exit_check_y - 32 && new_maze[MAZE_DIR_UP] < W)
                            new_maze[MAZE_DIR_UP] = W;
                        if(w.Location.Y + w.Location.Height >= exit_check_y + 32 && new_maze[MAZE_DIR_DOWN] < W)
                            new_maze[MAZE_DIR_DOWN] = W;
                        if(w.Location.X <= exit_check_x - 32 && new_maze[MAZE_DIR_LEFT] < W)
                            new_maze[MAZE_DIR_LEFT] = W;
                        if(w.Location.X + w.Location.Width >= exit_check_x + 32 && new_maze[MAZE_DIR_RIGHT] < W)
                            new_maze[MAZE_DIR_RIGHT] = W;
                    }
                }
            }

            // if things are normal, return to the same maze segment if the other directions are blocked
            if(space_left < 32)
                new_maze[direction ^ MAZE_DIR_FLIP_BIT] = maze_index;

            bool random_choice = iRand(2);
            bool choice_by_control = false;

            // don't allow player to choose to go backwards
            controls[direction ^ MAZE_DIR_FLIP_BIT] = false;

            for(int i = 0; i < 4; i++)
            {
                if(controls[i] && new_maze[i])
                {
                    maze_state = i;
                    choice_by_control = true;
                    break;
                }
            }

            if(choice_by_control)
            {
                // already have a new state
            }
            else if(new_maze[direction])
                maze_state = direction;
            else if(direction == MAZE_DIR_UP || direction == MAZE_DIR_DOWN)
            {
                if(new_maze[MAZE_DIR_LEFT] && new_maze[MAZE_DIR_RIGHT])
                    maze_state = (random_choice) ? MAZE_DIR_LEFT : MAZE_DIR_RIGHT;
                else if(new_maze[MAZE_DIR_LEFT])
                    maze_state = MAZE_DIR_LEFT;
                else if(new_maze[MAZE_DIR_RIGHT])
                    maze_state = MAZE_DIR_RIGHT;
                else
                    maze_state = (direction ^ MAZE_DIR_FLIP_BIT);
            }
            else
            {
                if(new_maze[MAZE_DIR_UP] && new_maze[MAZE_DIR_DOWN])
                    maze_state = (random_choice) ? MAZE_DIR_UP : MAZE_DIR_DOWN;
                else if(new_maze[MAZE_DIR_UP])
                    maze_state = MAZE_DIR_UP;
                else if(new_maze[MAZE_DIR_DOWN])
                    maze_state = MAZE_DIR_DOWN;
                else
                    maze_state = (direction ^ MAZE_DIR_FLIP_BIT);
            }

            maze_index = new_maze[maze_state];
        }
    }

    target_speed_x *= speedvar;
    target_speed_y *= speedvar;
    target_speed_x /= 4;
    target_speed_y /= 4;

    loc.SpeedX = loc.SpeedX / 2 + target_speed_x / 2;
    loc.SpeedY = loc.SpeedY / 2 + target_speed_y / 2;

    loc.X += loc.SpeedX;
    loc.Y += loc.SpeedY;

    loc.SpeedX += Layer[cur_maze.Layer].SpeedX;
    loc.SpeedY += Layer[cur_maze.Layer].SpeedY;
}

void PhysEnv_Maze_PickDirection(const Location_t& loc, vbint_t maze_index, uint8_t& maze_state)
{
    double dx = Water[maze_index].Location.X - loc.X + (Water[maze_index].Location.Width - loc.Width) / 2;
    double dy = Water[maze_index].Location.Y - loc.Y + (Water[maze_index].Location.Height - loc.Height) / 2;

    if(std::abs(dx) > std::abs(dy))
    {
        if(dx > 0)
            maze_state = MAZE_DIR_RIGHT;
        else
            maze_state = MAZE_DIR_LEFT;
    }
    else
    {
        if(dy > 0)
            maze_state = MAZE_DIR_DOWN;
        else
            maze_state = MAZE_DIR_UP;
    }
}
