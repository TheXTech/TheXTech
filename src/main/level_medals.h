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

#ifndef LEVEL_MEDALS_H
#define LEVEL_MEDALS_H

struct LevelData;

struct CurLevelMedals_t
{
private:
    uint8_t m_checkpoint = 0;

public:
    uint8_t max = 0;
    uint8_t prev = 0;
    uint8_t got = 0;
    uint8_t life = 0;

    // sets specific coin as obtained
    void get(uint8_t idx);

    // resets this-life medals
    void on_any_death();

    // loads currently got medals from checkpoint, resets this-life medals
    void on_all_dead();

    // saves currently got medals to checkpoint
    void on_checkpoint();

    // resets all player gotten medals for fresh level
    void reset_checkpoint();

    // resets level attributes (keeps checkpoint)
    void reset_lvl();

    // loads maximums for current level
    void prepare_lvl(LevelData& loadedLevel);

    // commits run to current level's save info
    void commit();
};

// sets the Variant attribute of medals in the level
void OrderMedals();

extern CurLevelMedals_t g_curLevelMedals;

#endif // #ifndef LEVEL_MEDALS_H
