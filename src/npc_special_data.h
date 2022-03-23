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

// the purpose of this header is define the configurable
// Special7 values for NPCs in the PGE-X standard.

// these are generally used in TheXTech to indicate unique
// modifications to the NPC's behavior / AI.

#pragma once
#ifndef NPC_SPECIAL_DATA_H
#define NPC_SPECIAL_DATA_H

#include <cstddef>
#include "npc_id.h"

// the first value listed is assumed to be the modern default.
// the string array must be nullptr-terminated.
struct NPC_Special7_Data_t
{
    // a sequence of constant pointers to constant characters
    const char* const * strings;
    const double* values;
    // const int* max_supported_versions;

    inline size_t find_current(double value) const
    {
        size_t i = 0;

        for(; strings[i] != nullptr; i++)
        {
            if(values[i] == value)
                return i;
        }

        return i;
    }

#if 0
    // One of us can implement code to automate the level-load decision
    //   of which Special7 to pick based on level version

    inline size_t find_default(int version) const
    {
        size_t lowest_supported = 0;

        size_t i = 1;

        for(; strings[i] != nullptr; i++)
        {
            if(version <= max_supported_versions[i]
                && max_supported_versions[i] < max_supported_versions[lowest_supported])
            {
                lowest_supported = i;
            }
        }

        return lowest_supported;
    }
#endif
};

// when multiple NPC types share the same NPC_Special7, we don't
//   need to reinitialize Special7 when switching between them
struct NPC_Special7_Map_t
{
    int Type;
    const NPC_Special7_Data_t* data;
};

// NPCID_CANNONITEM // billy gun
constexpr const char* cannon_strings[] = {"1.2.1+", "1.2.0", "Pre-1.2", nullptr};
constexpr double cannon_values[] = {0., 1., 2.};
constexpr NPC_Special7_Data_t Special7_cannon = {cannon_strings, cannon_values};

// NPCID_THWOMP_SMB3
constexpr const char* thwomp_strings[] = {"Modern", "Fall\nAlways", nullptr};
constexpr double thwomp_values[] = {0., 1.};
constexpr NPC_Special7_Data_t Special7_thwomp = {thwomp_strings, thwomp_values};

// NPCID_BOWSER_SMB3
constexpr const char* bowser3_strings[] = {"Modern", "Section\nExpand", nullptr};
constexpr double bowser3_values[] = {0., 1.};
constexpr NPC_Special7_Data_t Special7_bowser3 = {bowser3_strings, bowser3_values};

// NPCID_YELBLOCKS, NPCID_BLUBLOCKS, NPCID_GRNBLOCKS, NPCID_REDBLOCKS, NPCID_PLATFORM_SMB3, NPCID_SAW
constexpr const char* platform_strings[] = {"XTech", "1.3", "Pre-1.2", nullptr};
constexpr double platform_values[] = {2., 0., 1.};
constexpr NPC_Special7_Data_t Special7_platform = {platform_strings, platform_values};

constexpr NPC_Special7_Map_t NPC_Special7_map[] =
{
    {NPCID_CANNONITEM, &Special7_cannon},
    {NPCID_THWOMP_SMB3, &Special7_thwomp},
    {NPCID_BOWSER_SMB3, &Special7_bowser3},
    {NPCID_YELBLOCKS, &Special7_platform},
    {NPCID_BLUBLOCKS, &Special7_platform},
    {NPCID_GRNBLOCKS, &Special7_platform},
    {NPCID_REDBLOCKS, &Special7_platform},
    {NPCID_PLATFORM_SMB3, &Special7_platform},
    {NPCID_SAW, &Special7_platform},
};

constexpr size_t NPC_Special7_count = sizeof(NPC_Special7_map) / sizeof(NPC_Special7_Map_t);

inline const NPC_Special7_Data_t* find_Special7_Data(int Type)
{
    for(size_t i = 0; i < NPC_Special7_count; i++)
    {
        if(NPC_Special7_map[i].Type == Type)
            return NPC_Special7_map[i].data;
    }
    return nullptr;
}

inline double find_modern_Special7(int Type)
{
    const NPC_Special7_Data_t* data = find_Special7_Data(Type);

    if(!data)
        return 0.;

    return data->values[0];
}

#endif // NPC_SPECIAL_DATA_H
