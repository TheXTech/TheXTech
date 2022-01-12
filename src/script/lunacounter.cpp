#include <SDL2/SDL_endian.h>
#include <Utils/files.h>
#include <AppPath/app_path.h>
#include <Logger/logger.h>
#include <fmt_format_ne.h>

#include "luna.h"
#include "lunacounter.h"
#include "lunaplayer.h"
#include "lunarender.h"
#include "renderop_string.h"
#include "globals.h"
#include "graphics.h"
#include "core/render.h"


DeathCounter gDeathCounter;

// CTOR
DeathCounter::DeathCounter()
{
    mStatFileOK = false;
    mEnabled = true;
    mCurTotalDeaths = 0;
    mCurLevelDeaths = 0;
}

void DeathCounter::init()
{
    if(counterFile.empty())
    {
        counterFile = AppPathManager::settingsRoot() + DEATHCT_FNAME;
        if(!TryLoadStats())
            mEnabled = false;
    }
}

// TRY LOAD STATS - Attempts to load stats from stats file. Creates and inits the file if it doesn't exist.
bool DeathCounter::TryLoadStats()
{
    // Try to open the file
    int32_t tempint = 0;
    FILE *statsfile;

    // If file is not exist yet, try to create empty file
    if(!Files::fileExists(counterFile))
    {
        statsfile = Files::utf8_fopen(counterFile.c_str(), "wb");
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

    statsfile = Files::utf8_fopen(counterFile.c_str(), "r+b");

    // If create failed, disable death counter
    if(!statsfile)
    {
        pLogWarning("Unable to open the Demos counter: %s", counterFile.c_str());
        mStatFileOK = false;
        mEnabled = false;
        return false;
    }

    // If size less than 100, init new file
    std::fseek(statsfile, 0, SEEK_END);
    int cursize = (int)std::ftell(statsfile);
    std::fseek(statsfile, 0, SEEK_SET);

    if(cursize < 50)
    {
        InitStatsFile(statsfile);
        std::fflush(statsfile);
        mStatFileOK = true;
        std::fseek(statsfile, 0, SEEK_SET);
    }

//    if(statsfile.good() == false)
//    {
//        mStatFileOK = false;
//        mEnabled = false;
//        return false;
//    }

    // Check version
    std::fread(&tempint, 1, sizeof(int32_t), statsfile);
    tempint = SDL_SwapLE32(tempint);

    if(tempint < 5)
    {
        mStatFileOK = false;
        mEnabled = false;
        return false;
    }
    else
    {
        mStatFileOK = true;
        mEnabled = true;
    }

    ClearRecords();
    ReadRecords(statsfile);

    std::fclose(statsfile);

    return true;
}

// UPDATE DEATHS - Determine if the main player has died and update death counter state if so
void DeathCounter::UpdateDeaths(bool write_save)
{
    std::wstring debuginfo(L"UpdateDeaths");

    Player_t *demo = PlayerF::Get(1);
    if(demo == 0)
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
    if(mEnabled == false)
        return;

    for(auto iter = mDeathRecords.begin(), end = mDeathRecords.end(); iter != end; ++iter)
    {
        if((*iter).LevelName.compare(lvlname) == 0)   // On first name match...
        {
            (*iter).Deaths += amount;      // Inc death count
            return;                        // and exit
        }
    }

    // if no match, create new death record and add it to list
    DeathRecord newrec;
    newrec.LevelName = lvlname;
    newrec.Deaths = amount;
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
    int32_t writeint = SDL_SwapLE32(LUNA_VERSION);

    // Write dll version
    std::fseek(statsfile, 0, SEEK_SET);
    std::fwrite(&writeint, 1, sizeof(writeint), statsfile);

    // Init reserved
    uint8_t writebyte = 0;
    off_t offset = std::ftell(statsfile);
    while(offset < 100)
    {
        std::fwrite(&writebyte, 1, sizeof(writebyte), statsfile);
        offset = std::ftell(statsfile);
    }

    // Write record count at 100 bytes (0 record count)
    writeint = 0;
    std::fwrite(&writeint, 1, sizeof(writeint), statsfile);
}


// READ RECORDS - Add death records from file into death record list
void DeathCounter::ReadRecords(FILE *statsfile)
{
    // Read the record count at 100 bytes
    std::fseek(statsfile, 100, SEEK_SET);
    int32_t tempint = 0;
    std::fread(&tempint, 1, sizeof(tempint), statsfile);

    if(tempint == 0)
        return;

    tempint = SDL_SwapLE32(tempint);

    for(int i = 0; i < tempint; i++)
    {
        DeathRecord newrec;
        newrec.Load(statsfile);
        mDeathRecords.push_back(newrec);
    }
}


// WRITE RECORDS - Writes death record count at pos 100 in the file followed by each record
void DeathCounter::WriteRecords(FILE *statsfile)
{
    int32_t reccount = SDL_SwapLE32((int32_t)mDeathRecords.size());
    std::fseek(statsfile, 100, SEEK_SET);
    std::fwrite(&reccount, 1, sizeof(reccount), statsfile);

    // Write each record, if any exist
    if(mDeathRecords.size() >= 1)
    {
        for(auto iter = mDeathRecords.begin(), end = mDeathRecords.end(); iter != end; ++iter)
            (*iter).Save(statsfile);
    }

}

// TRY SAVE - Externally callable, safe auto-save function
void DeathCounter::TrySave()
{
    if(mStatFileOK && mEnabled)
    {
        FILE *statsfile = Files::utf8_fopen(counterFile.c_str(), "wb");
        if(statsfile)
        {
            Save(statsfile);
            std::fflush(statsfile);
            std::fclose(statsfile);
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
    if(!mEnabled) return;

    int total = 0;
    mCurLevelDeaths = 0;

    for(auto iter = mDeathRecords.begin(), end = mDeathRecords.end(); iter != end; ++iter)
    {
        total += (*iter).Deaths;
        if((*iter).LevelName.compare(FileNameFull) == 0)
            mCurLevelDeaths = (*iter).Deaths;
    }

    mCurTotalDeaths = total;

}

// DRAW - Print the death counter in its current state
void DeathCounter::Draw()
{
    if(!mEnabled)
        return;

    // Format string to print
    std::string printstr = fmt::format_ne("{0} / {1}", mCurLevelDeaths, mCurTotalDeaths);
    float minusoffset = (float)(123 - (printstr.size() * 8));

    XRender::offsetViewportIgnore(true);
    // Print to screen in upper left
    SuperPrint(gDemoCounterTitle, 3, 80, 27);
    SuperPrint(printstr, 3, minusoffset, 48);
    XRender::offsetViewportIgnore(false);
}

// PRINT DEBUG - Prints all death records to the screen
void DeathCounter::PrintDebug()
{
    if(!gLunaEnabled || !gLunaEnabledGlobally)
        return;

    if(mDeathRecords.size() >= 1)
    {
        float y = 300;
        for(auto iter = mDeathRecords.begin(), end = mDeathRecords.end(); iter != end; ++iter)
        {
            Renderer::Get().AddOp(new RenderStringOp(std::to_string((*iter).Deaths), 2, 50, y));
            Renderer::Get().AddOp(new RenderStringOp((*iter).LevelName, 2, 80, y));
            y += 30;
        }
    }
}
