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

#include <bitset>
#include <array>

#include <PGE_File_Formats/file_formats.h>

#include "npc_id.h"

#include "main/level_medals.h"
#include "main/level_save_info.h"

CurLevelMedals_t g_curLevelMedals;

static inline LevelSaveInfo_t* s_findSaveInfo()
{
    for(int i = 1; i <= numWorldLevels; ++i)
    {
        if(WorldLevel[i].FileName == FileNameFull)
            return &WorldLevel[i].save_info;
    }

    for(auto& e : LevelWarpSaveEntries)
    {
        if(e.levelPath == FileNameFull)
            return &e.save_info;
    }

    return nullptr;
}

void CurLevelMedals_t::get(uint8_t idx)
{
    if(InHub())
        return;

    if(idx < 8 && idx < max)
    {
        got |= (1 << idx);
        life |= (1 << idx);
    }
}

void CurLevelMedals_t::on_any_death()
{
    life = 0;
}

void CurLevelMedals_t::on_all_dead()
{
    life = 0;
    got = m_checkpoint;
}

void CurLevelMedals_t::on_checkpoint()
{
    if(InHub())
        return;

    m_checkpoint = got;
}

void CurLevelMedals_t::reset_checkpoint()
{
    got = 0;
    life = 0;
    m_checkpoint = 0;
}

void CurLevelMedals_t::reset_lvl()
{
    max = 0;
    prev = 0;
}

void CurLevelMedals_t::prepare_lvl(LevelData& loadedLevel)
{
    LevelSaveInfo_t* info = s_findSaveInfo();

    if(!info && LevelWarpSaveEntries.size() != 0xFFFF)
    {
        LevelSaveInfo_t info_init = InitLevelSaveInfo(loadedLevel);

        if(info_init.inited())
        {
            LevelWarpSaveEntries.push_back({FileNameFull, info_init});
            info = &LevelWarpSaveEntries[LevelWarpSaveEntries.size() - 1].save_info;
        }
    }

    if(!info)
    {
        reset_lvl();
        return;
    }

    max = info->max_medals;
    prev = info->medals_got;
}

void CurLevelMedals_t::commit()
{
    LevelSaveInfo_t* info = s_findSaveInfo();

    if(!info)
        return;

    info->medals_got |= got;

    int old_best_count = 0;
    int new_best_count = 0;

    for(int i = 0; i < 8 && i < info->max_medals; ++i)
    {
        if(info->medals_best & (1 << i))
            old_best_count++;

        if(life & (1 << i))
            new_best_count++;
    }

    if(new_best_count > old_best_count)
        info->medals_best = life;
}

void OrderMedals()
{
    std::array<int16_t, 8> auto_hits;
    int auto_count = 0;

    std::bitset<8> spec_hits;

    // look for medals
    for(int i = 1; i <= numNPCs; ++i)
    {
        NPC_t& n = NPC[i];

        bool is_container = (n.Type == NPCID_ITEM_BURIED || n.Type == NPCID_ITEM_POD ||
                             n.Type == NPCID_ITEM_BUBBLE || n.Type == NPCID_ITEM_THROWER);

        bool contains_medal = is_container && n.Special == NPCID_MEDAL;

        // allow medals only
        if(n.Type != NPCID_MEDAL && !contains_medal)
            continue;

        // don't count friendly medals (except, thrown medals can be collected)
        if(n.Inert && n.Type != NPCID_ITEM_THROWER)
            continue;

        // medal won't be counted if out-of-range
        if(n.Variant > 8)
            continue;

        if(n.Variant == 0)
        {
            if(auto_count < 8)
            {
                auto_hits[auto_count] = static_cast<int16_t>(i);
                auto_count++;
            }

            // otherwise, Variant will be left as 0 and medal won't be counted

            continue;
        }

        spec_hits[n.Variant - 1] = true;
    }

    for(int auto_i = 0; auto_i < auto_count; ++auto_i)
    {
        NPC_t& n = NPC[auto_hits[auto_i]];

        // try to find an index
        for(int i = 0; i < 8; ++i)
        {
            if(!spec_hits[i])
            {
                n.Variant = i + 1;
                spec_hits[i] = true;
                break;
            }
        }

        // if failed, Variant will be left as 0 and medal won't be counted
    }
}
