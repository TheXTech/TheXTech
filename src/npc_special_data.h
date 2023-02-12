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

// the purpose of this header is define the configurable
// Variant values for NPCs in the PGE-X standard.

// these are generally used in TheXTech to indicate unique
// modifications to the NPC's behavior / AI.

#pragma once
#ifndef NPC_SPECIAL_DATA_H
#define NPC_SPECIAL_DATA_H

#include <cstddef>
#include "npc_id.h"

// the first value listed is assumed to be the modern default.
// the string array must be nullptr-terminated.
struct NPC_Variant_Data_t
{
    // strings to describe behaviors in the editor
    // may be two lines of up to 7 characters each
    // (a sequence of constant pointers to constant characters)
    // MUST have an extra nullptr at the end
    const char* const * strings;

    // initial Variant values for each behavior
    const uint8_t* values;

    // first version of VB6 game that did NOT use a legacy behavior
    // special values:
    // 66: modern SMBX (forwards compatible, includes any future experiments)
    // 65: SMBX64
    // -1: modern TheXTech
    // -2: X38A default behavior
    // -3: SMBX2 default behavior
    const int* active_below;

    // returns the limit i (with strings[i] == nullptr) if not found
    inline size_t find_current(uint8_t value) const
    {
        size_t i;

        for(i = 0; strings[i] != nullptr; i++)
        {
            if(values[i] == value)
                return i;
        }

        return i;
    }

    // returns the limit i (with strings[i] == nullptr) if not found
    inline size_t find_legacy(int version) const
    {
        size_t earliest_behavior = 0;

        size_t i;

        for(i = 1; strings[i] != nullptr; i++)
        {
            if(version < active_below[i]
                && active_below[i] < active_below[earliest_behavior])
            {
                earliest_behavior = i;
            }
        }

        // return limit i if none of the versions are above the current version
        if(earliest_behavior == 0 && version >= active_below[earliest_behavior])
            return i;

        return earliest_behavior;
    }
};

// when multiple NPC types share the same NPC_Variant, we don't
//   need to reinitialize Variant when switching between them
struct NPC_Variant_Map_t
{
    int Type;
    const NPC_Variant_Data_t* data;
};

// NPCID_CANNONITEM // billy gun
constexpr const char* cannon_strings[] = {"1.2.1+", "1.2.0", "Pre-1.2", nullptr};
constexpr uint8_t cannon_values[] = {0, 1, 2};
constexpr int cannon_active_below[] = {66, 51, 28};
constexpr NPC_Variant_Data_t Variant_cannon = {cannon_strings, cannon_values, cannon_active_below};

// NPCID_THWOMP_SMB3
constexpr const char* thwomp_strings[] = {"Modern", "Fall\nAlways", nullptr};
constexpr uint8_t thwomp_values[] = {0, 1};
constexpr int thwomp_active_below[] = {66, 9};
constexpr NPC_Variant_Data_t Variant_thwomp = {thwomp_strings, thwomp_values, thwomp_active_below};

// NPCID_BOWSER_SMB3
constexpr const char* bowser3_strings[] = {"Modern", "Section\nExpand", nullptr};
constexpr uint8_t bowser3_values[] = {0, 1};
constexpr int bowser3_active_below[] = {66, 30};
constexpr NPC_Variant_Data_t Variant_bowser3 = {bowser3_strings, bowser3_values, bowser3_active_below};

// NPCID_YELBLOCKS, NPCID_BLUBLOCKS, NPCID_GRNBLOCKS, NPCID_REDBLOCKS, NPCID_PLATFORM_SMB3, NPCID_SAW
constexpr const char* platform_strings[] = {"XTech", "1.3", "Pre-1.2", nullptr};
constexpr uint8_t platform_values[] = {2, 0, 1};
constexpr int platform_active_below[] = {-1, 66, 30};
constexpr NPC_Variant_Data_t Variant_platform = {platform_strings, platform_values, platform_active_below};

constexpr NPC_Variant_Map_t NPC_Variant_map[] =
{
    {NPCID_CANNONITEM, &Variant_cannon},
    {NPCID_THWOMP_SMB3, &Variant_thwomp},
    {NPCID_BOWSER_SMB3, &Variant_bowser3},
    {NPCID_YELBLOCKS, &Variant_platform},
    {NPCID_BLUBLOCKS, &Variant_platform},
    {NPCID_GRNBLOCKS, &Variant_platform},
    {NPCID_REDBLOCKS, &Variant_platform},
    {NPCID_PLATFORM_SMB3, &Variant_platform},
    {NPCID_SAW, &Variant_platform},
};

constexpr size_t NPC_Variant_count = sizeof(NPC_Variant_map) / sizeof(NPC_Variant_Map_t);

inline const NPC_Variant_Data_t* find_Variant_Data(int Type)
{
    for(size_t i = 0; i < NPC_Variant_count; i++)
    {
        if(NPC_Variant_map[i].Type == Type)
            return NPC_Variant_map[i].data;
    }
    return nullptr;
}

inline uint8_t find_modern_Variant(int Type)
{
    const NPC_Variant_Data_t* data = find_Variant_Data(Type);

    if(!data)
        return 0;

    return data->values[0];
}

inline uint8_t find_legacy_Variant(int Type, int version)
{
    const NPC_Variant_Data_t* data = find_Variant_Data(Type);

    if(!data)
        return 0;

    size_t legacy_index = data->find_legacy(version);

    if(data->strings[legacy_index] == nullptr)
        return 0;

    return data->values[legacy_index];
}

#endif // NPC_SPECIAL_DATA_H
