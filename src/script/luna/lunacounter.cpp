/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2026 Vitaly Novichkov <admin@wohlnet.ru>
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

#include <Utils/files.h>
#include <Logger/logger.h>
#include <fmt_format_ne.h>

#include "luna.h"
#include "lunacounter.h"
#include "lunacounter_util.h"
#include "lunaplayer.h"
#include "lunarender.h"
#include "renderop_string.h"
#include "globals.h"
#include "config.h"
#include "graphics.h"
#include "config.h"
#include "game_main.h"
#include "core/render.h"
#include "main/game_save.h"
#include "main/game_info.h"
#include "main/menu_main.h"
#include "main/level_save_info.h"

static char* s_escaped_strcpy(char* dest, char* end, const std::string& src)
{
    for(char c : src)
    {
        if(dest == end - 1 || dest == end)
            return nullptr;

        switch(c)
        {
        case '\n':
            *(dest++) = '\\';
            *(dest++) = 'n';
            break;
        case '\r':
            *(dest++) = '\\';
            *(dest++) = 'r';
            break;
        case '\"':
        case ';':
        case ':':
        case '[':
        case ']':
        case ',':
        case '%':
        case '\\':
            *(dest++) = '\\';
            *(dest++) = c;
            break;
        default:
            *(dest++) = c;
            break;
        }
    }

    return dest;
}


int g_totalFails = 0;
DeathCounter gDeathCounter;

// CTOR
DeathCounter::DeathCounter() noexcept
{
    mEnabled = false;
    g_totalFails = 0;
    mCurLevelDeaths = 0;

    // Print Demos counter with a font 3
    m_print.font = 3;
}

DeathCounter::~DeathCounter() noexcept
{
}

void DeathCounter::init()
{
    mEnabled = (g_totalFails != -1);
}

void DeathCounter::quit()
{
    mEnabled = false;
    mCurLevelDeaths = 0;
}

// TRY LOAD STATS - Attempts to load stats from legacy stats file.
bool DeathCounter::TryLoadStats()
{
    if(g_totalFails != -1)
        return true;

    // prevent a segfault
    if(TestLevel || selWorld < 0 || selWorld >= (int)SelectWorld.size() || BattleMode || LevelEditor || selSave <= 0)
        return false;

    std::string oldFile = makeGameSavePath(SelectWorld[selWorld].WorldFilePath,
                                           fmt::format_ne("demos-{0}.dmo", selSave));

    std::string oldFile2 = makeGameSavePath(SelectWorld[selWorld].WorldFilePath,
                                           fmt::format_ne("deaths-{0}.rip", selSave));

    std::string counterFile = makeGameSavePath(SelectWorld[selWorld].WorldFilePath,
                                   fmt::format_ne("fails-{0}.rip", selSave));

    if(Files::fileExists(oldFile)) // Rename old file ino the new name
        Files::moveFile(counterFile, oldFile);
    else if(Files::fileExists(oldFile2)) // Rename old file ino the new name
        Files::moveFile(counterFile, oldFile2);

    // Try to open the file
    int32_t tempint = 0;
    size_t got;

    SDL_RWops* openFile = Files::open_file(counterFile, "rb");

    // If create failed, disable death counter
    if(!openFile)
    {
        pLogWarning("Unable to restore from the legacy Demos counter: %s", counterFile.c_str());
        return false;
    }

    // Check version
    got = LunaCounterUtil::readIntLE(openFile, tempint);
    if(got != sizeof(int32_t) || tempint < 5)
    {
        if(got != sizeof(int32_t))
            pLogWarning("Fails counter: Failed to read version number at the %s file", counterFile.c_str());

        mEnabled = false;

        SDL_RWclose(openFile);
        openFile = nullptr;

        return false;
    }

    ReadRecords(openFile);

    return (g_totalFails != -1);
}

// mark that a death occurred in the current level
void DeathCounter::MarkDeath(bool write_save)
{
    bool dcAllow = (gEnableDemoCounterByLC || g_config.enable_fails_tracking);

    if(!dcAllow)
        return;

    LevelSaveInfo_t* info = CurSaveInfo();

    if(info)
        info->fails++;

    g_totalFails++;
    mCurLevelDeaths++;

    if(write_save)
    {
        GamesaveAccess save_access;
        if(save_access.savefile)
        {
            bool failed = false;
            std::array<char, 384> buf;

            // update total fail count
            int size = snprintf(buf.data(), 384, "SAVE_HEADER\nTF:%d;\nSAVE_HEADER_END\n", g_totalFails);
            if((int)SDL_RWwrite(save_access.savefile, &buf[0], 1, size) != size)
                failed = true;

            if(!failed && info)
            {
                strcpy(buf.data(), "LEVEL_INFO\nL:\"");
                char* dest = &buf[14];
                char* buf_end = &buf[0] + buf.size();
                dest = s_escaped_strcpy(dest, buf_end, FileNameFull);
                if(dest)
                    dest += snprintf(dest, buf_end - dest, "\";F:%d;\nLEVEL_INFO_END\n", info->fails);
                size = dest - &buf[0];
            }

            if(failed || (int)SDL_RWwrite(save_access.savefile, &buf[0], 1, size) != size)
                pLogCritical("Could not append fail to save file.");
        }
    }
}

// READ RECORDS - Add death records from legacy file into death record list
void DeathCounter::ReadRecords(SDL_RWops *statsfile)
{
    int32_t tempint = 0;
    size_t got;

    // Read the record count at 100 bytes
    SDL_RWseek(statsfile, 100, RW_SEEK_SET);
    got = LunaCounterUtil::readIntLE(statsfile, tempint);

    if(got != sizeof(tempint))
    {
        pLogWarning("Fails counter: Failed to read the number of records");
        return;
    }

    if(tempint == 0)
        return;

    g_totalFails = 0;

    DeathRecord newrec;
    for(int i = 0; i < tempint; i++)
    {
        if(newrec.Load(statsfile))
        {
            g_totalFails += newrec.m_deaths;

            for(auto& e : LevelSaveEntries)
            {
                if(e.levelPath == newrec.m_levelName)
                    e.save_info.fails = newrec.m_deaths;
            }
        }
    }
}


// CLEAR RECORDS - Reset the death records for every level
void DeathCounter::ClearRecords()
{
    g_totalFails = 0;
    for(auto& e : LevelSaveEntries)
        e.save_info.fails = 0;

    mCurLevelDeaths = 0;

    // must save game to prevent conflicts with updates
    SaveGame();
}

// RECOUNT - Recount and relist the death count for the current level
void DeathCounter::Recount()
{
    if(!mEnabled)
        return;

    LevelSaveInfo_t* info = CurSaveInfo();

    if(info)
        mCurLevelDeaths = info->fails;
    else
        mCurLevelDeaths = 0;
}

// DRAW - Print the death counter in its current state
void DeathCounter::Draw(int screenZ)
{
    if(!mEnabled)
        return;

    // Format string to print
    m_print.syncCache(mCurLevelDeaths, g_totalFails);
    m_print.syncCache(g_gameInfo.fails_counter_title);

    const vScreen_t& vscreen = vScreen[screenZ];
    const Screen_t& screen = Screens[vscreen.screen_ref];

    // HUD is very wide in >2P shared screen
    bool wide_hud = (screen.Type == ScreenTypes::SharedScreen && screen.player_count > 2);

    int ScreenTop = 0;
    if(vscreen.Height > 600)
        ScreenTop = vscreen.Height / 2 - 300;
    int HUDLeft = vscreen.Width / 2 - 400;

    // With normal res, print to screen in upper left
    if(vscreen.Width >= 800 && !wide_hud)
    {
        int title_X   = 123 - m_print.titlePixLen / 2;
        int counter_X = 123 - m_print.counterPixLen / 2;

        // make even
        title_X &= ~1;
        counter_X &= ~1;

        SuperPrint(g_gameInfo.fails_counter_title, m_print.font, HUDLeft + title_X, ScreenTop + 26);
        SuperPrint(m_print.counterOut, m_print.font, HUDLeft + counter_X, ScreenTop + 48);
    }
    // At low res, print to top of screen
    else
    {
        int total_W = m_print.titlePixLen + 16 + m_print.counterPixLen;
        int title_X = vscreen.Width / 2 - total_W / 2;

        if(vscreen.Width < 640)
            title_X -= 64;

        int counter_X = title_X + m_print.titlePixLen + 16;

        // make even
        title_X &= ~1;
        counter_X &= ~1;

        SuperPrint(g_gameInfo.fails_counter_title, m_print.font, title_X, ScreenTop);
        SuperPrint(m_print.counterOut, m_print.font, counter_X, ScreenTop);
    }
}

void DeathCounter::CachedPrint::syncCache(int curLevel, int total)
{
    if(curLevel != counterLevel || total != counterTotal)
    {
        counterLevel = curLevel;
        counterTotal = total;
        counterOut = fmt::format_ne("{0} / {1}", curLevel, total);
        counterPixLen = SuperTextPixLen(counterOut.c_str(), font);
    }
}

void DeathCounter::CachedPrint::syncCache(const std::string &title)
{
    intptr_t ptr = reinterpret_cast<intptr_t>(title.c_str());

    if(titlePointer != ptr || titleSize != title.size())
    {
        titlePointer = ptr;
        titleSize = title.size();
        titlePixLen = SuperTextPixLen(title.c_str(), font);
    }
}
