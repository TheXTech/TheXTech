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

#ifndef LEVEL_MEDALS_H
#define LEVEL_MEDALS_H

#include <string>

struct LevelData;
struct LevelSaveInfo_t;

namespace PGE_FileFormats_misc
{
    class TextInput;
}

struct CurLevelMedals_t
{
protected:
    friend bool OpenLevelData(PGE_FileFormats_misc::TextInput &lvl, const std::string FilePath);
    friend void OpenLevelDataPost();

    /**
     * \brief checks if the current level's save data must be initialized
     * Must be called during OpenLevelData after FileNameFull is set
     *
     * The resulting pointer may be used by a SaveInfoInit object during level load,
     * and remains valid until the next time LevelWarpSaveEntries is modified
     * by this method or by FindStars()
     *
     * \param loadedLevel the loaded LevelData object, used to initialize level save info if needed
     */
    LevelSaveInfo_t* should_initialize() const;

    /**
     * \brief loads maximums from the current level and resets gotten counts
     * Must be called after the level is fully loaded and the save data has been initialized
     */
    void prepare_lvl();

public:
    //! INTEGER for number of medals available (can't exceed 8)
    uint8_t max = 0;

    //! bitfield of medals previously acquired in level (imported as "got" from save data)
    uint8_t prev = 0;
    //! bitfield of medals acquired during current play session (on win, combined with "got" in save data)
    uint8_t got = 0;
    //! bitfield of medals acquired since most recent lost life (on win, compared with and possibly replaces "best" in save data)
    uint8_t life = 0;
    //! bitfield of medals acquired at most recent checkpoint
    uint8_t checkpoint = 0;

    /**
     * \brief sets specific coin as obtained
     * \param idx coin index
     */
    void get(uint8_t idx);

    /**
     * \brief checks whether specific coin has been obtained
     * \param idx coin index
     */
    bool gotten(uint8_t idx) const;

    /**
     * \brief resets this-life medals
     */
    void on_any_death();

    /**
     * \brief loads currently got medals from checkpoint (if appropriate), resets this-life medals
     */
    void on_all_dead();

    /**
     * \brief saves currently got medals to checkpoint
     */
    void on_checkpoint();

    /**
     * \brief resets saved medals in checkpoint
     */
    void reset_checkpoint();

    /**
     * \brief restores saved medals from checkpoint
     */
    void resume_from_checkpoint();

    /**
     * \brief resets level attributes (keeps checkpoint and gotten counts)
     */
    void reset_lvl();

    /**
     * \brief commits run to current level's save info
     */
    void commit();
};

/**
 * \brief sets the Variant attribute of medals in the current level
 */
void OrderMedals();

/**
 * \brief Save an obtained level beat code to the current level's LevelSaveInfo entries
 */
void CommitBeatCode(int beat_code);

extern CurLevelMedals_t g_curLevelMedals;

#endif // #ifndef LEVEL_MEDALS_H
