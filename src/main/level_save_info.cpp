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

#include <cstdio>
#include <string>
#include <bitset>
#include <PGE_File_Formats/file_formats.h>
#include <Logger/logger.h>
#include <fmt_format_ne.h>

#include "npc_id.h"

#include "globals.h"

static bool s_serializeSaveInfo(std::string& s, const LevelSaveInfo_t& info)
{
    if(!info.inited())
        return false;

    char medals_got[9];
    char medals_best[9];

    for(int i = 0; i < info.max_medals && i < 8; ++i)
    {
        medals_got[i]  = (info.medals_got  & (1 << i)) ? '+' : '.';
        medals_best[i] = (info.medals_best & (1 << i)) ? '+' : '.';
    }

    for(int i = info.max_medals; i < 8; ++i)
    {
        medals_got[i] = ' ';
        medals_best[i] = ' ';
    }

    medals_got[8] = '\0';
    medals_best[8] = '\0';

    unsigned max_stars = info.max_stars;
    unsigned max_medals = info.max_medals;

    if(max_medals > 8)
        max_medals = 8;

    s = fmt::sprintf_ne("V:1;Sm:%3u;Mm:%1u;Mg:%s;Mb:%s;", (unsigned)max_stars, (unsigned)max_medals, medals_got, medals_best);

    return true;
}

static bool s_parseSaveInfo(LevelSaveInfo_t& info, const std::string& s)
{
    info = LevelSaveInfo_t();

    unsigned version = 0;
    int end_pos = 0;

    if(sscanf(s.c_str(), "V:%2u;%n", &version, &end_pos) != 1 || end_pos < 1)
    {
        pLogWarning("Invalid level save info string [%s]", s.c_str());
        return false;
    }

    if(version != 1)
    {
        pLogWarning("Supported level save info string version %u, full string [%s]", version, s.c_str());
        return false;
    }

    unsigned max_stars = 0;
    unsigned max_medals = 0;
    char medals_got[9];
    char medals_best[9];

    if(sscanf(s.c_str(), "V:%2u;Sm:%3u;Mm:%1u;Mg:%8[+. ];Mb:%8[+. ];%n", &version, &max_stars, &max_medals, medals_got, medals_best, &end_pos) != 5 || end_pos != (int)s.size())
    {
        pLogWarning("Invalid level save info string [%s]", s.c_str());
        return false;
    }

    // validate limits
    if(max_stars > 255)
        max_stars = 255;

    if(max_medals > 8)
        max_medals = 8;

    // load maximums
    info.max_stars = max_stars;
    info.max_medals = max_medals;

    // parse strings
    for(unsigned i = 0; i < max_medals; ++i)
    {
        if(medals_got[i] == '+')
            info.medals_got |= (1 << i);

        if(medals_best[i] == '+')
            info.medals_best |= (1 << i);
    }

    return true;
}

void ImportLevelSaveInfo(const saveUserData::DataSection& s)
{
    for(int A = 1; A <= numWorldLevels; A++)
    {
        if(WorldLevel[A].FileName.empty())
            continue;

        WorldLevel[A].save_info = LevelSaveInfo_t();
    }

    LevelWarpSaveEntries.clear();

    for(const saveUserData::DataEntry& e : s.data)
    {
        LevelSaveInfo_t info;

        if(!s_parseSaveInfo(info, e.value))
            continue;

        D_pLogDebug("Loaded save info %d %d %d %d for level [%s]", (int)info.max_stars, (int)info.max_medals, (int)info.medals_got, (int)info.medals_best, e.key.c_str());

        // see if it applies to a world level
        bool worldLevelHit = false;

        for(int A = 1; A <= numWorldLevels; A++)
        {
            WorldLevel_t& l = WorldLevel[A];

            if(l.save_info.inited())
                continue;

            if(l.FileName == e.key)
            {
                l.save_info = info;
                worldLevelHit = true;
                // don't break, in case another level has the same filename
            }
        }

        if(!worldLevelHit)
        {
            // add a new level warp save entry
            LevelWarpSaveEntries.push_back(LevelWarpSaveEntry_t{e.key, info});
        }
    }
}

void ExportLevelSaveInfo(saveUserData::DataSection& s)
{
    std::string buf;

    for(int A = 1; A <= numWorldLevels; A++)
    {
        if(s_serializeSaveInfo(buf, WorldLevel[A].save_info))
            s.data.push_back({WorldLevel[A].FileName, buf});
    }

    for(const auto& e : LevelWarpSaveEntries)
    {
        if(s_serializeSaveInfo(buf, e.save_info))
            s.data.push_back({e.levelPath, buf});
    }
}

LevelSaveInfo_t InitLevelSaveInfo(LevelData& loadedLevel)
{
    LevelSaveInfo_t ret;

    ret.max_stars = loadedLevel.stars;
    ret.max_medals = 0;
    ret.medals_got = 0;
    ret.medals_best = 0;

    uint8_t full_count = 0;
    std::bitset<8> spec_hits;

    // look for medals
    for(auto &npc : loadedLevel.npc)
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

        if(Variant > 8)
            continue;

        if(Variant == 0)
        {
            if(full_count < 8)
                full_count++;

            continue;
        }

        if(!spec_hits[Variant - 1])
        {
            full_count++;
            spec_hits[Variant - 1] = true;
        }
    }

    ret.max_medals = full_count;

    return ret;
}

LevelSaveInfo_t InitLevelSaveInfo(const std::string& fullPath, LevelData& tempData)
{
    LevelSaveInfo_t ret;

    if(FileFormats::OpenLevelFile(fullPath, tempData))
    {
        ret = InitLevelSaveInfo(tempData);
        pLogDebug("Initing level save data at [%s] with %d stars", fullPath.c_str(), (int)ret.max_stars);
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
