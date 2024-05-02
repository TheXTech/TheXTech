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
#ifndef FRAME_TIMER_H
#define FRAME_TIMER_H

#include <cstdint>
#include <functional>

struct MicroStats
{
public:
    enum Task
    {
        Script,
        Controls,
        Layers,
        NPCs,
        Blocks,
        Effects,
        Player,
        Camera,
        Graphics,
        Events,
        Sound,
        TASK_END
    };

    const char* const task_names[TASK_END] =
    {
        "Sct",
        "Ctl",
        "Lay",
        "NPC",
        "Blk",
        "Eff",
        "Plr",
        "Cam",
        "Gfx",
        "Evt",
        "Snd",
    };

private:
    uint8_t m_cur_task = TASK_END;
    uint8_t m_cur_frame = 0;
    uint64_t m_level_frame = 0;
    uint64_t m_cur_time = 0;
    uint64_t m_cur_timer[TASK_END] = {0};

    uint64_t m_frame_time = 0;
    uint64_t m_slow_frame_time = 0;

public:
    uint64_t level_timer[TASK_END] = {0};
    int view_timer[TASK_END] = {0};
    int view_total = 0;
    int view_slow_frame_time = 0;

    void reset();
    void start_task(Task task);
    void start_sleep();
    void end_frame();

};

struct PerformanceStats_t
{
    // How many objects got drawn in one frame
    int renderedBlocks = 0;
    // int renderedSzBlocks = 0; // combined with normal blocks
    int renderedBGOs = 0;
    int renderedNPCs = 0;
    int renderedEffects = 0;

    // How many objects got scanned to find what to render
    int checkedBlocks = 0;
    // int checkedSzBlocks = 0;
    int checkedBGOs = 0;
    // int checkedNPCs = 0; // use active NPCs
    // int checkedEffects = 0; // use total effects

    int renderedTiles = 0;
    int renderedScenes = 0;
    int renderedPaths = 0;
    int renderedLevels = 0;

    int checkedTiles = 0;
    int checkedScenes = 0;
    int checkedPaths = 0;
    int checkedLevels = 0;

    int page = 0;

    // Displays title of the music OR filename
    std::string currentMusic;
    std::string currentMusicFile;

    void next_page();

    void reset();
    void print_filenames(int x, int y);
    void print_obj_stats(int x, int y);
    void print_cpu_stats(int x, int y);
    // void print_ram_stats();
    void print();
};

extern MicroStats g_microStats;
extern PerformanceStats_t g_stats;

void resetFrameTimer();
void resetTimeBuffer();
bool frameSkipNeeded();

typedef void (*LoopCall_t)(void);

bool canProceedFrame();
void computeFrameTime1();
void computeFrameTime2();
void frameNextInc();
void cycleNextInc();

void frameRenderStart();
void frameRenderEnd();

void runFrameLoop(LoopCall_t doLoopCallbackPre,
                  LoopCall_t doLoopCallbackPost,
                  std::function<bool ()> condition,
                  std::function<bool ()> subCondition = nullptr,
                  std::function<void ()> preTimerExtraPre = nullptr,
                  std::function<void ()> preTimerExtraPost = nullptr);

#endif // FRAME_TIMER_H
