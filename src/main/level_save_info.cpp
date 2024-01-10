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

#include <cstdio>
#include <string>
#include <bitset>
#include <PGE_File_Formats/file_formats.h>
#include <Logger/logger.h>
#include <fmt_format_ne.h>

#include "sdl_proxy/sdl_stdinc.h"

#include "npc_id.h"

#include "globals.h"

static bool s_exportSingleSaveInfo(saveLevelInfo& s, const LevelSaveInfo_t& info)
{
    if(!info.inited())
        return false;

    s.max_stars = info.max_stars;
    s.max_medals = info.max_medals;

    int size = SDL_min(static_cast<int>(info.max_medals), c_max_track_medals);

    s.medals_got.resize(size);
    s.medals_best.resize(size);

    for(int i = 0; i < size; ++i)
    {
        s.medals_got[i]  = (info.medals_got  & (1 << i));
        s.medals_best[i] = (info.medals_best & (1 << i));
    }

    return true;
}

static bool s_importSingleSaveInfo(LevelSaveInfo_t& info, const saveLevelInfo& s)
{
    info = LevelSaveInfo_t();

    unsigned max_stars = s.max_stars;
    unsigned max_medals = s.max_medals;

    // validate limits
    static_assert(c_max_track_stars <= 255, "c_max_track_stars used to limit values of uint8_t");
    if(max_stars > c_max_track_stars)
        max_stars = c_max_track_stars;

    static_assert(c_max_track_medals <= 8, "c_max_track_medals used to limit bit indexes into uint8_t");
    if(max_medals > c_max_track_medals)
        max_medals = c_max_track_medals;

    // load maximums
    info.max_stars = max_stars;
    info.max_medals = max_medals;

    // load medals
    for(unsigned i = 0; i < max_medals; ++i)
    {
        if(i < s.medals_got.size() && s.medals_got[i])
            info.medals_got |= (1 << i);

        if(i < s.medals_got.size() && s.medals_best[i])
            info.medals_best |= (1 << i);
    }

    return true;
}

void ImportLevelSaveInfo(const GamesaveData& s)
{
    // reset all world levels' save info
    for(int A = 1; A <= numWorldLevels; A++)
    {
        if(WorldLevel[A].FileName.empty())
            continue;

        WorldLevel[A].save_info = LevelSaveInfo_t();
    }

    LevelWarpSaveEntries.clear();

    for(const saveLevelInfo& e : s.levelInfo)
    {
        LevelSaveInfo_t info;

        if(!s_importSingleSaveInfo(info, e))
            continue;

        D_pLogDebug("Loaded save info (stars %d, medals %d - got %x, best %x) for level [%s]", (int)info.max_stars, (int)info.max_medals, (int)info.medals_got, (int)info.medals_best, e.level_filename.c_str());

        // see if it applies to a world level
        bool worldLevelHit = false;

        for(int A = 1; A <= numWorldLevels; A++)
        {
            WorldLevel_t& l = WorldLevel[A];

            // can skip the string comparison if the level has already been initialized and we are no longer checking whether this save info has any level
            if(l.save_info.inited() && worldLevelHit)
                continue;

            if(l.FileName == e.level_filename)
            {
                // update level save info if not yet initialized
                if(!l.save_info.inited())
                    l.save_info = info;

                // mark save info as paired with a level
                worldLevelHit = true;

                // don't break, in case another level has the same filename
            }
        }

        // otherwise, add a new level warp save entry
        if(!worldLevelHit)
            LevelWarpSaveEntries.push_back(LevelWarpSaveEntry_t{e.level_filename, info});
    }
}

void ExportLevelSaveInfo(GamesaveData& s)
{
    saveLevelInfo tempInfo;

    for(int A = 1; A <= numWorldLevels; A++)
    {
        if(s_exportSingleSaveInfo(tempInfo, WorldLevel[A].save_info))
        {
            tempInfo.level_filename = WorldLevel[A].FileName;
            s.levelInfo.push_back(tempInfo);
        }
    }

    for(const auto& e : LevelWarpSaveEntries)
    {
        if(s_exportSingleSaveInfo(tempInfo, e.save_info))
        {
            tempInfo.level_filename = e.levelPath;
            s.levelInfo.push_back(tempInfo);
        }
    }
}

LevelSaveInfo_t InitLevelSaveInfo(const LevelData& loadedLevel)
{
    LevelSaveInfo_t ret;

    // NOTE: could use code similar to below for stars, if not trusting the level header
    ret.max_stars = loadedLevel.stars;
    ret.max_medals = 0;
    ret.medals_got = 0;
    ret.medals_best = 0;

    // look for medals
    uint8_t medal_count = 0;

    // keep track of the user-specified ones because two medals with the same user index are only counted once
    std::bitset<c_max_track_medals> used_indexes;

    for(const auto &npc : loadedLevel.npc)
    {
        bool is_container = (npc.id == NPCID_ITEM_BURIED || npc.id == NPCID_ITEM_POD ||
                             npc.id == NPCID_ITEM_BUBBLE || npc.id == NPCID_ITEM_THROWER);

        bool contains_medal = is_container && npc.contents == NPCID_MEDAL;

        // allow medals only
        if(npc.id != NPCID_MEDAL && !contains_medal)
            continue;

        // don't count friendly medals (except, thrown medals can be collected)
        if(npc.friendly && npc.id != NPCID_ITEM_THROWER)
            continue;

        uint8_t Variant = static_cast<uint8_t>(npc.special_data);

        // medal won't be counted (at all) if out-of-range
        if(Variant > c_max_track_medals)
            continue;

        // if no index, freely add to medal count (cap at 8 later)
        if(Variant == 0)
        {
            medal_count++;
            continue;
        }

        // if index specified, check that the index hasn't already been used
        if(!used_indexes[Variant - 1])
        {
            medal_count++;
            used_indexes[Variant - 1] = true;
        }
    }

    // cap medal count at 8
    if(medal_count > c_max_track_medals)
        medal_count = c_max_track_medals;

    ret.max_medals = medal_count;

    return ret;
}

LevelSaveInfo_t InitLevelSaveInfo(const std::string& fullPath, LevelData& tempData)
{
    LevelSaveInfo_t ret;

    if(FileFormats::OpenLevelFile(fullPath, tempData))
    {
        ret = InitLevelSaveInfo(tempData);
        pLogDebug("Initing level save data at [%s] with %d stars and %d medals", fullPath.c_str(), (int)ret.max_stars, (int)ret.max_medals);
    }
    else
    {
        pLogWarning("During save info init: error of level \"%s\" file loading: %s (line %d).",
                    fullPath.c_str(),
                    tempData.meta.ERROR_info.c_str(),
                    tempData.meta.ERROR_linenum);
    }

    return ret;
}
