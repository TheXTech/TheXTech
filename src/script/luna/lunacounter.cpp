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
#include "graphics.h"
#include "game_main.h"
#include "core/render.h"
#include "main/menu_main.h"


DeathCounter gDeathCounter;

// CTOR
DeathCounter::DeathCounter() noexcept
{
    mStatFileOK = false;
    mEnabled = false;
    mCurTotalDeaths = 0;
    mCurLevelDeaths = 0;

    // Print Demos counter with a font 3
    m_print.font = 3;
}

DeathCounter::~DeathCounter() noexcept
{
    if(m_openFile)
        std::fclose(m_openFile);
}

void DeathCounter::init()
{
    if(counterFile.empty())
    {
        // prevent a segfault
        if(TestLevel || selWorld < 0 || selWorld >= (int)SelectWorld.size() || BattleMode || LevelEditor || selSave <= 0)
        {
            mEnabled = false;
            return;
        }

        std::string oldFile = makeGameSavePath(SelectWorld[selWorld].WorldPath,
                                               SelectWorld[selWorld].WorldFile,
                                               fmt::format_ne("demos-{0}.dmo", selSave));

        std::string oldFile2 = makeGameSavePath(SelectWorld[selWorld].WorldPath,
                                               SelectWorld[selWorld].WorldFile,
                                               fmt::format_ne("deaths-{0}.rip", selSave));

        counterFile = makeGameSavePath(SelectWorld[selWorld].WorldPath,
                                       SelectWorld[selWorld].WorldFile,
                                       fmt::format_ne("fails-{0}.rip", selSave));

        if(Files::fileExists(oldFile)) // Rename old file ino the new name
            Files::moveFile(counterFile, oldFile);
        else if(Files::fileExists(oldFile2)) // Rename old file ino the new name
            Files::moveFile(counterFile, oldFile2);

        if(!TryLoadStats())
            mEnabled = false;
    }
}

void DeathCounter::quit()
{
    counterFile.clear();
    mStatFileOK = false;
    mEnabled = false;
    mCurTotalDeaths = 0;
    mCurLevelDeaths = 0;
    mDeathRecords.clear();

    if(m_openFile)
    {
        std::fclose(m_openFile);
        m_openFile = nullptr;
    }
}

// TRY LOAD STATS - Attempts to load stats from stats file. Creates and inits the file if it doesn't exist.
bool DeathCounter::TryLoadStats()
{
    // Try to open the file
    int32_t tempint = 0;
    size_t got;

    // If file is not exist yet, try to create empty file
    if(!Files::fileExists(counterFile))
    {
        FILE *statsfile = Files::utf8_fopen(counterFile.c_str(), "wb");
        if(!statsfile)
        {
            mStatFileOK = false;
            mEnabled = false;
            pLogWarning("Unable to initialize Demos counter: %s", counterFile.c_str());
            return false;
        }

        std::fwrite(&tempint, 1, sizeof(int), statsfile);
        std::fflush(statsfile);
        std::fclose(statsfile);
    }

    // close old open file if present
    if(m_openFile)
    {
        std::fclose(m_openFile);
        m_openFile = nullptr;
    }

    m_openFile = Files::utf8_fopen(counterFile.c_str(), "r+b");

    // If create failed, disable death counter
    if(!m_openFile)
    {
        pLogWarning("Unable to open the Demos counter: %s", counterFile.c_str());
        mStatFileOK = false;
        mEnabled = false;
        return false;
    }

    // If size less than 100, init new file
    std::fseek(m_openFile, 0, SEEK_END);
    int cursize = (int)std::ftell(m_openFile);
    std::fseek(m_openFile, 0, SEEK_SET);

    if(cursize < 50)
    {
        InitStatsFile(m_openFile);
        std::fflush(m_openFile);
        mStatFileOK = true;
        std::fseek(m_openFile, 0, SEEK_SET);
    }

//    if(statsfile.good() == false)
//    {
//        mStatFileOK = false;
//        mEnabled = false;
//        return false;
//    }

    // Check version
    got = LunaCounterUtil::readIntLE(m_openFile, tempint);
    if(got != sizeof(int32_t) || tempint < 5)
    {
        if(got != sizeof(int32_t))
            pLogWarning("Demos counter: Failed to read version number at the %s file", counterFile.c_str());

        mStatFileOK = false;
        mEnabled = false;

        std::fclose(m_openFile);
        m_openFile = nullptr;

        return false;
    }

    mStatFileOK = true;
    mEnabled = true;

    ClearRecords();
    ReadRecords(m_openFile);

    return true;
}

// UPDATE DEATHS - Determine if the main player has died and update death counter state if so
void DeathCounter::UpdateDeaths(bool write_save)
{
    Player_t *demo = PlayerF::Get(1);
    if(!demo)
        return;

    // For now, we'll assume the player died if player 1's death timer is at exactly 50 frames
    if(demo->TimeToLive == 50)
    {
        AddDeath(FileNameFull, 1);
        if(write_save)
        {
            TrySave();
            Recount();
        }
    }
}

// ADD DEATH
void DeathCounter::AddDeath(const std::string &lvlname, int amount)
{
    if(!mEnabled)
        return;

    for(auto &iter : mDeathRecords)
    {
        if(iter.m_levelName == lvlname)   // On first name match...
        {
            iter.m_deaths += amount;      // Inc death count
            return;                        // and exit
        }
    }

    // if no match, create new death record and add it to list
    DeathRecord newrec;
    newrec.m_levelName = lvlname;
    newrec.m_deaths = amount;
    mDeathRecords.push_back(newrec);
}

// INIT STATS FILE
void DeathCounter::InitStatsFile(FILE *statsfile)
{
    WriteHeader(statsfile);
}


// WRITE HEADER - Write the death counter file header at beginning of file
void DeathCounter::WriteHeader(FILE *statsfile)
{
    // Write dll version
    std::fseek(statsfile, 0, SEEK_SET);
    LunaCounterUtil::writeIntLE(statsfile, LUNA_VERSION);

    // Init reserved
    uint8_t writebyte = 0;
    off_t offset = std::ftell(statsfile);
    while(offset < 100)
    {
        std::fwrite(&writebyte, 1, sizeof(writebyte), statsfile);
        offset = std::ftell(statsfile);
    }

    // Write record count at 100 bytes (0 record count)
    LunaCounterUtil::writeIntLE(statsfile, 0);
}

// READ RECORDS - Add death records from file into death record list
void DeathCounter::ReadRecords(FILE *statsfile)
{
    int32_t tempint = 0;
    size_t got;

    // Read the record count at 100 bytes
    std::fseek(statsfile, 100, SEEK_SET);
    got = LunaCounterUtil::readIntLE(statsfile, tempint);

    if(got != sizeof(tempint))
    {
        pLogWarning("Demos counter: Failed to read the number of records");
        return;
    }

    if(tempint == 0)
        return;

    for(int i = 0; i < tempint; i++)
    {
        DeathRecord newrec;
        if(newrec.Load(statsfile))
            mDeathRecords.push_back(newrec);
    }
}


// WRITE RECORDS - Writes death record count at pos 100 in the file followed by each record
void DeathCounter::WriteRecords(FILE *statsfile)
{
    int32_t reccount = (int32_t)mDeathRecords.size();
    std::fseek(statsfile, 100, SEEK_SET);
    LunaCounterUtil::writeIntLE(statsfile, reccount);

    // Write each record, if any exist
    if(!mDeathRecords.empty())
    {
        for(auto &mDeathRecord : mDeathRecords)
            mDeathRecord.Save(statsfile);
    }
}

// TRY SAVE - Externally callable, safe auto-save function
void DeathCounter::TrySave()
{
    if(mStatFileOK && mEnabled && m_openFile)
    {
        if(std::fseek(m_openFile, 0, SEEK_SET))
        {
            // attempt to reopen the file
            std::fclose(m_openFile);
            m_openFile = Files::utf8_fopen(counterFile.c_str(), "wb");
        }

        if(m_openFile)
        {
            Save(m_openFile);
            std::fflush(m_openFile);
        }
    }
}

// SAVE
void DeathCounter::Save(FILE *statsfile)
{
    if(mStatFileOK && mEnabled)
    {
        WriteHeader(statsfile);
        WriteRecords(statsfile);
    }
}


// CLEAR RECORDS - Clear the death record list and dealloc its records
void DeathCounter::ClearRecords()
{
    mDeathRecords.clear();
}

// RECOUNT - Recount and relist the death count for the current level and the total deathcount
void DeathCounter::Recount()
{
    if(!mEnabled)
        return;

    int total = 0;
    mCurLevelDeaths = 0;

    for(const auto &iter : mDeathRecords)
    {
        total += iter.m_deaths;
        if(iter.m_levelName == FileNameFull)
            mCurLevelDeaths = iter.m_deaths;
    }

    mCurTotalDeaths = total;

}

// DRAW - Print the death counter in its current state
void DeathCounter::Draw(int screenZ)
{
    if(!mEnabled)
        return;

    // Format string to print
    m_print.syncCache(mCurLevelDeaths, mCurTotalDeaths);
    m_print.syncCache(gDemoCounterTitle);

    XRender::offsetViewportIgnore(true);

    const vScreen_t& vscreen = vScreen[screenZ];

    // With normal res, print to screen in upper left
    if(vscreen.Width >= 800)
    {
        int title_X   = 123 - m_print.titlePixLen / 2;
        int counter_X = 123 - m_print.counterPixLen / 2;

        // make even
        title_X &= ~1;
        counter_X &= ~1;

        SuperPrint(gDemoCounterTitle, m_print.font, title_X, 26);
        SuperPrint(m_print.counterOut, m_print.font, counter_X, 48);
    }
    // At low res, print to top of screen
    else
    {
        int total_W = m_print.titlePixLen + 16 + m_print.counterPixLen;
        int title_X = vscreen.Width / 2 - total_W / 2;
        int counter_X = title_X + m_print.titlePixLen + 16;

        // make even
        title_X &= ~1;
        counter_X &= ~1;

        SuperPrint(gDemoCounterTitle, m_print.font, title_X, 0);
        SuperPrint(m_print.counterOut, m_print.font, counter_X, 0);
    }

    XRender::offsetViewportIgnore(false);
}

// PRINT DEBUG - Prints all death records to the screen
void DeathCounter::PrintDebug() const
{
    if(!gLunaEnabled || !gLunaEnabledGlobally)
        return;

    if(!mDeathRecords.empty())
    {
        float y = 300;
        for(const auto &iter : mDeathRecords)
        {
            Renderer::Get().AddOp(new RenderStringOp(fmt::format_ne("{0}", iter.m_deaths), 2, 50, y));
            Renderer::Get().AddOp(new RenderStringOp(iter.m_levelName, 2, 80, y));
            y += 30;
        }
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
