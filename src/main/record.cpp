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

// needed because there are a lot of writes / scans whose failure is detected at the end of the function
// updated versions of glibc don't even seem to trigger this warning in this context
#pragma GCC diagnostic ignored "-Wunused-result"


// this module handles particular the control recording and playback functions
// and the gameplay stats recording functions

#include "../version.h"

#define LONG_VERSION "TheXTech branch " V_BUILD_BRANCH " commit " V_BUILD_VER
#define SHORT_VERSION V_BUILD_VER

#include "../globals.h"
#include "../rand.h"
#include "../frame_timer.h"
#include "../compat.h"
#include "../config.h"
#include "record.h"

#include "core/std.h"

#include <chrono>
#include <fmt_time_ne.h>
#include <fmt_format_ne.h>

#include <cstdint>
#include <cinttypes>
#include <Utils/files.h>
#include <DirManager/dirman.h>
#include <AppPath/app_path.h>
#include <Logger/logger.h>
#include <md5tools.hpp>


#ifndef PRId64 /*Workaround*/
#   ifndef __PRI64_PREFIX
#       if THEXTECH_WORDSIZE == 64
#           ifdef _WIN32
#               define __PRI64_PREFIX   "I64"
#               define __PRIPTR_PREFIX  "I64"
#               ifdef __MINGW32__
#                   pragma GCC diagnostic push
#                   pragma GCC diagnostic ignored "-Wformat="
#                   define X_GCC_NO_WARNING_FORMAT
#               endif
#           else
#               define __PRI64_PREFIX   "l"
#               define __PRIPTR_PREFIX  "l"
#           endif
#       else
#           define __PRI64_PREFIX   "ll"
#           define __PRIPTR_PREFIX
#       endif
#   endif
#   define PRId64   __PRI64_PREFIX "d"
#endif

static std::string makeRecordPrefix()
{
    auto now = std::chrono::system_clock::now();
    std::time_t in_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm t = fmt::localtime_ne(in_time_t);

    return fmt::sprintf_ne("%s%s_%s_%04d-%02d-%02d_%02d-%02d-%02d.rec",
                           AppPathManager::gameplayRecordsRootDir(), FileName, SHORT_VERSION,
                           (1900 + t.tm_year), (1 + t.tm_mon), t.tm_mday,
                           t.tm_hour, t.tm_min, t.tm_sec);
}

static void clipNewLine(char *buffer, size_t maxSize)
{
    for(size_t i = 0; i < maxSize; i++)
    {
        if(buffer[i] == '\r' || buffer[i] == '\n')
        {
            buffer[i] = '\0';
            break;
        }
    }
}


namespace Record
{

// public
FILE* record_file = nullptr;
FILE* replay_file = nullptr;

//! Externally providen level file path for the replay
static std::string replayLevelFilePath;

static const int c_recordVersion = 3;

// private

static bool         in_level = false;
static bool         diverged_major = false;
static bool         diverged_minor = false;
static int64_t      frame_no = 0;
static int64_t      next_record_frame = 0;
static uint32_t     last_status_tick = 0;
static Controls_t   last_controls[maxPlayers];


static void write_header()
{
    // write all necessary state variables!
    fprintf(record_file, "Header\r\n");
    fprintf(record_file, "RecordVersion %d\r\n", c_recordVersion); // Version of record file
    fprintf(record_file, "Version %s\r\n", LONG_VERSION); // game version / commit
    fprintf(record_file, "CompatLevel %d\r\n", CompatGetLevel()); // compatibility mode
    if(FullFileName.compare(0, AppPath.size(), AppPath) == 0)
        fprintf(record_file, "%s\r\n", FullFileName.c_str()+AppPath.size()); // level that was played
    else
        fprintf(record_file, "%s\r\n", FullFileName.c_str()); // level that was played

    std::string md5sum = md5::file_to_hashGC(FullFileName);
    fprintf(record_file, "SumMD5 %s\r\n", md5sum.c_str()); // level that was played
    fprintf(record_file, "Seed %d\r\n", readSeed());
    fprintf(record_file, "Checkpoint %d\r\n", (Checkpoint == FullFileName) ? 1 : 0);

    if(g_compatibility.fix_vanilla_checkpoints && Checkpoint == FullFileName)
    {
        fprintf(record_file, "Multipoints %d: ", (int)CheckpointsList.size());

        for(const Checkpoint_t& cp : CheckpointsList)
            fprintf(record_file, "%d,", cp.id);

        fprintf(record_file, "\r\n");
    }

    fprintf(record_file, "StartWarp %d\r\n", StartWarp);
    fprintf(record_file, "ReturnWarp %d\r\n", ReturnWarp);
    fprintf(record_file, "Lives %d\r\n", (int)Lives);
    fprintf(record_file, "Coins %d\r\n", Coins);
    fprintf(record_file, "Score %d\r\n", Score);
    fprintf(record_file, "Stars %d\r\n", numStars);

    for(int A = 1; A <= numStars; A++)
    {
        fprintf(record_file, "Star\r\n");
        fprintf(record_file, "%s\r\n", Star[A].level.c_str());
        fprintf(record_file, "Section %d\r\n", Star[A].Section);
    }

    fprintf(record_file, "Players %d\r\n", numPlayers);

    for(int A = 1; A <= numPlayers; A++)
    {
        fprintf(record_file,
                "Player\r\n"
                "Char %d\r\n"
                "State %d\r\n"
                "Mount %d\r\n"
                "MountType %d\r\n"
                "HeldBonus %d\r\n",
                Player[A].Character, Player[A].State, Player[A].Mount, Player[A].MountType, Player[A].HeldBonus);
    }
}

// FIXME: Implement the error returning and on-failure abortation with leading abortation of record replaying startup

static void read_header()
{
    rewind(replay_file); // fseek(replay_file, 0, SEEK_SET);

    // buffer is a 1024-character buffer used for reading strings, shared with the record_init() function.
    char buffer[1024];
    char md5hash[1024];
    std::string thisHash;

    // n is an integer for some implicit conversions
    int n;
    // Version of record file
    int recordVersion = 0;

    // read all necessary state variables!
    fgets(buffer, 1024, replay_file); // "Header"
    fscanf(replay_file, "RecordVersion %d\r\n", &recordVersion);

    pLogDebug("Loading recording version %d", recordVersion);

    if(recordVersion < 2)
        pLogCritical("Record file is invalid! (version below than minimally supported: %d)", recordVersion);

    fgets(buffer, 1024, replay_file); // game version / commit
    fscanf(replay_file, "CompatLevel %d\r\n", &n); // compatibility mode

    CompatSetEnforcedLevel(n);

    if(CompatGetLevel() < COMPAT_SMBX2)
        pLogWarning("compatibility mode is not a long-term support version. Do not expect identical results.");

    fgets(buffer, 1024, replay_file); // level that was played

    clipNewLine(buffer, 1024); // clip the newline :(

    // now SET the filename
    FullFileName = replayLevelFilePath.empty() ? buffer : replayLevelFilePath;
    // if(XStd::strcasecmp(buffer, FilefNameFull.c_str()))
    //     pLogWarning("FileName does not match.");

    pLogDebug("Attempt to load level file %s for the replay", FullFileName.c_str());

    thisHash = md5::file_to_hashGC(FullFileName);
    if(thisHash.empty())
        pLogCritical("Failed to retrieve the MD5 hash for %s file (probably, it doesn't exist)", FullFileName.c_str());

    XStd::memset(md5hash, 0, sizeof(md5hash));
    fscanf(replay_file, "SumMD5 %s\r\n", md5hash); // File's hash
    pLogDebug("Replay file (loaded %s, expected %s)", thisHash.c_str(), md5hash);
    int hashCmp = thisHash.compare(md5hash);
    if(hashCmp != 0)
        pLogCritical("Loaded level file is not matched to expected (check sum missmatch %d)", hashCmp);

    fscanf(replay_file, "Seed %d\r\n", &n); // random seed
    seedRandom(n);

    fscanf(replay_file, "Checkpoint %d\r\n", &n); // is there a checkpoint?

    Checkpoint = n ? FullFileName : std::string();

    if(g_compatibility.fix_vanilla_checkpoints && Checkpoint == FullFileName)
    {
        CheckpointsList.clear();
        fscanf(replay_file, "Multipoints %d: ", &n);
        CheckpointsList.resize(n);

        for(int i = 0; i < n; i++)
            fscanf(replay_file, "%d,", &CheckpointsList[i].id);

        fscanf(replay_file, "\r\n");
    }

    fscanf(replay_file, "StartWarp %d\r\n", &StartWarp);
    fscanf(replay_file, "ReturnWarp %d\r\n", &ReturnWarp);
    fscanf(replay_file, "Lives %d\r\n", &n);
    Lives = n;
    fscanf(replay_file, "Coins %d\r\n", &Coins);
    fscanf(replay_file, "Score %d\r\n", &Score);
    fscanf(replay_file, "Stars %d\r\n", &numStars);

    for(int A = 1; A <= numStars; A++)
    {
        fgets(buffer, 1024, replay_file); // "Star"
        fgets(buffer, 1024, replay_file); // level

        for(int i = 0; i < 1024; i++)
            if(buffer[i] == '\r') buffer[i] = '\0'; // clip the newline :(

        Star[A].level = buffer;
        fscanf(replay_file, "Section %d\r\n", &Star[A].Section);
    }

    fscanf(replay_file, "Players %d\r\n", &numPlayers);

    for(int A = 1; A <= numPlayers; A++)
    {
        if(recordVersion < 3)
            fscanf(replay_file,
                   "Player\r\n"
                   "Char %d\r\n"
                   "State %d\r\n"
                   "MountType %d\r\n"
                   "HeldBonus %d\r\n",
                &Player[A].Character, &Player[A].State, &Player[A].MountType, &Player[A].HeldBonus);
        else
            fscanf(replay_file,
                   "Player\r\n"
                   "Char %d\r\n"
                   "State %d\r\n"
                   "Mount %d\r\n"
                   "MountType %d\r\n"
                   "HeldBonus %d\r\n",
                &Player[A].Character, &Player[A].State, &Player[A].Mount, &Player[A].MountType, &Player[A].HeldBonus);
    }

    Cheater = true; // important to avoid losing player save data in replay mode.
    TestLevel = false;
    MaxFPS = true;
    ShowFPS = true;
    FrameSkip = false;
}

static void write_end()
{
    fprintf(record_file, " %" PRId64 " \r\nEnd\r\nLevelBeatCode %d\r\n", frame_no+1, LevelBeatCode);
}

static void read_end()
{
    int b;

    if(fscanf(replay_file, "End\r\nLevelBeatCode %d\r\n", &b) != 1)
    {
        pLogWarning("old gameplay file diverged (invalid end header).");
        diverged_major = true;
    }

    if(next_record_frame-1 != frame_no)
    {
        pLogWarning("final frame_no diverged (old: %" PRId64 ", new: %" PRId64 ").", next_record_frame-1, frame_no);
        diverged_major = true;
    }

    if(b != LevelBeatCode && !(b == -1 && LevelBeatCode == 0))
    {
        pLogWarning("LevelBeatCode diverged (old: %d, new: %d).", b, LevelBeatCode);
        diverged_major = true;
    }
}

static void write_control()
{
    for(int i = 0; i < numPlayers; i++)
    {
        const Controls_t& keys = Player[i+1].Controls;

        if(!last_controls[i].Up && keys.Up)
            fprintf(record_file, " %" PRId64 "\r\nC+%dU\r\n", frame_no, i+1);
        else if(last_controls[i].Up && !keys.Up)
            fprintf(record_file, " %" PRId64 "\r\nC-%dU\r\n", frame_no, i+1);

        if(!last_controls[i].Down && keys.Down)
            fprintf(record_file, " %" PRId64 "\r\nC+%dD\r\n", frame_no, i+1);
        else if(last_controls[i].Down && !keys.Down)
            fprintf(record_file, " %" PRId64 "\r\nC-%dD\r\n", frame_no, i+1);

        if(!last_controls[i].Left && keys.Left)
            fprintf(record_file, " %" PRId64 "\r\nC+%dL\r\n", frame_no, i+1);
        else if(last_controls[i].Left && !keys.Left)
            fprintf(record_file, " %" PRId64 "\r\nC-%dL\r\n", frame_no, i+1);

        if(!last_controls[i].Right && keys.Right)
            fprintf(record_file, " %" PRId64 "\r\nC+%dR\r\n", frame_no, i+1);
        else if(last_controls[i].Right && !keys.Right)
            fprintf(record_file, " %" PRId64 "\r\nC-%dR\r\n", frame_no, i+1);

        if(!last_controls[i].Start && keys.Start)
            fprintf(record_file, " %" PRId64 "\r\nC+%dS\r\n", frame_no, i+1);
        else if(last_controls[i].Start && !keys.Start)
            fprintf(record_file, " %" PRId64 "\r\nC-%dS\r\n", frame_no, i+1);

        if(!last_controls[i].Drop && keys.Drop)
            fprintf(record_file, " %" PRId64 "\r\nC+%dI\r\n", frame_no, i+1);
        else if(last_controls[i].Drop && !keys.Drop)
            fprintf(record_file, " %" PRId64 "\r\nC-%dI\r\n", frame_no, i+1);

        if(!last_controls[i].Jump && keys.Jump)
            fprintf(record_file, " %" PRId64 "\r\nC+%dA\r\n", frame_no, i+1);
        else if(last_controls[i].Jump && !keys.Jump)
            fprintf(record_file, " %" PRId64 "\r\nC-%dA\r\n", frame_no, i+1);

        if(!last_controls[i].Run && keys.Run)
            fprintf(record_file, " %" PRId64 "\r\nC+%dB\r\n", frame_no, i+1);
        else if(last_controls[i].Run && !keys.Run)
            fprintf(record_file, " %" PRId64 "\r\nC-%dB\r\n", frame_no, i+1);

        if(!last_controls[i].AltJump && keys.AltJump)
            fprintf(record_file, " %" PRId64 "\r\nC+%dX\r\n", frame_no, i+1);
        else if(last_controls[i].AltJump && !keys.AltJump)
            fprintf(record_file, " %" PRId64 "\r\nC-%dX\r\n", frame_no, i+1);

        if(!last_controls[i].AltRun && keys.AltRun)
            fprintf(record_file, " %" PRId64 "\r\nC+%dY\r\n", frame_no, i+1);
        else if(last_controls[i].AltRun && !keys.AltRun)
            fprintf(record_file, " %" PRId64 "\r\nC-%dY\r\n", frame_no, i+1);

        last_controls[i] = keys;
    }

    fflush(record_file);
}

static void read_control()
{
    int p;
    char mode, key;

    if(fscanf(replay_file, "C%c%d%c\r\n", &mode, &p, &key) != 3)
        return;

    bool set = (mode != '-');

    if(key == 'U')
        last_controls[p-1].Up = set;
    else if(key == 'D')
        last_controls[p-1].Down = set;
    else if(key == 'L')
        last_controls[p-1].Left = set;
    else if(key == 'R')
        last_controls[p-1].Right = set;
    else if(key == 'S')
        last_controls[p-1].Start = set;
    else if(key == 'I')
        last_controls[p-1].Drop = set;
    else if(key == 'A')
        last_controls[p-1].Jump = set;
    else if(key == 'B')
        last_controls[p-1].Run = set;
    else if(key == 'X')
        last_controls[p-1].AltJump = set;
    else if(key == 'Y')
        last_controls[p-1].AltRun = set;

    // replicate the controls changes in the new recording
    if(record_file)
    {
        fprintf(record_file, " %" PRId64 "\r\nC%c%d%c\r\n", frame_no, mode, p, key);
    }
}

static void write_status()
{
    if(frame_no == 0)
    {
        g_stats.renderedNPCs = 0;
        g_stats.renderedBlocks = 0;
        g_stats.renderedSzBlocks = 0;
        g_stats.renderedBGOs = 0;
    }

    fprintf(record_file, " %" PRId64 " \r\nStatus\r\n", frame_no);
    uint32_t status_tick = XStd::GetTicks();
    fprintf(record_file, "Ticks %lu\r\n", (long unsigned)(XStd::GetTicks() - last_status_tick));
    last_status_tick = status_tick;
    fprintf(record_file, "randCalls %ld\r\n", random_ncalls());
    fprintf(record_file, "Score %d\r\n", Score);
    fprintf(record_file, "numNPCs %d\r\n", numNPCs);

    int numActiveNPCs = 0;
    if(frame_no != 0)
    {
        for(int i = 1; i <= numNPCs; i++)
        {
            if(NPC[i].Active)
                numActiveNPCs ++;
        }
    }

    fprintf(record_file, "numActiveNPCs %d\r\n", numActiveNPCs);
    fprintf(record_file, "numRenderNPCs %d\r\nnumRenderBlocks %d\r\nnumRenderBGOs %d\r\n",
        g_stats.renderedNPCs, g_stats.renderedBlocks + g_stats.renderedSzBlocks, g_stats.renderedBGOs);

    for(int i = 1; i <= numPlayers; i++)
    {
        fprintf(record_file, "p%dx %lf\r\np%dy %lf\r\n",
            i, Player[i].Location.X, i, Player[i].Location.Y);
    }

    fflush(record_file);
}

static void read_status()
{
    if(frame_no == 0)
    {
        g_stats.renderedNPCs = 0;
        g_stats.renderedBlocks = 0;
        g_stats.renderedSzBlocks = 0;
        g_stats.renderedBGOs = 0;
    }

    int o_ticks, o_Score, o_numNPCs, o_numActiveNPCs, o_renderedNPCs, o_renderedBlocks, o_renderedBGOs;
    long o_randCalls;

    int success = 0;

    fscanf(replay_file, "Status\r\n%n", &success);

    if(!success)
    {
        pLogWarning("old gameplay file diverged (invalid status header) at frame %" PRId64 ".", frame_no);
        diverged_major = true;
        return;
    }

    if(fscanf(replay_file,
              "Ticks %d\r\n"
              "randCalls %ld\r\n"
              "Score %d\r\n"
              "numNPCs %d\r\n"
              "numActiveNPCs %d\r\n"
              "numRenderNPCs %d\r\n"
              "numRenderBlocks %d\r\n"
              "numRenderBGOs %d\r\n",
        &o_ticks, &o_randCalls, &o_Score, &o_numNPCs, &o_numActiveNPCs, &o_renderedNPCs, &o_renderedBlocks, &o_renderedBGOs) != 8)
    {
        pLogWarning("old gameplay file diverged (invalid status info) at frame %" PRId64 ".", frame_no);
        diverged_major = true;
        return;
    }

    if(o_randCalls != random_ncalls())
    {
        pLogWarning("randCalls diverged (old: %d, new: %ld) at frame %" PRId64 ".", o_randCalls, random_ncalls(), frame_no);
        diverged_minor = true;
#ifdef DEBUG_RANDOM_CALLS
        for(int i = 0; i < g_random_calls.size(); i++)
            printf("%p\n", g_random_calls[i]);
#endif
    }
#ifdef DEBUG_RANDOM_CALLS
    g_random_calls.clear();
#endif

    if(o_Score != Score)
    {
        pLogWarning("score diverged (old: %d, new: %d) at frame %" PRId64 ".", o_Score, Score, frame_no);
        diverged_major = true;
    }

    if(o_numNPCs != numNPCs)
    {
        pLogWarning("numNPCs diverged (old: %d, new: %d) at frame %" PRId64 ".", o_numNPCs, numNPCs, frame_no);
        diverged_major = true;
    }

    int numActiveNPCs = 0;
    if(frame_no != 0)
    {
        for(int i = 1; i <= numNPCs; i++)
        {
            if(NPC[i].Active)
                numActiveNPCs ++;
        }
    }

    if(o_numActiveNPCs != numActiveNPCs)
    {
        pLogWarning("numActiveNPCs diverged (old: %d, new: %d) at frame %" PRId64 ".", o_numActiveNPCs, numActiveNPCs, frame_no);
        diverged_minor = true;
    }

    if(o_renderedNPCs != g_stats.renderedNPCs)
    {
        pLogWarning("renderedNPCs diverged (old: %d, new: %d) at frame %" PRId64 ".", o_renderedNPCs, g_stats.renderedNPCs, frame_no);
        diverged_minor = true;
    }

    if(o_renderedBlocks != g_stats.renderedBlocks + g_stats.renderedSzBlocks)
    {
        pLogWarning("renderedBlocks diverged (old: %d, new: %d) at frame %" PRId64 ".", o_renderedBlocks, g_stats.renderedBlocks + g_stats.renderedSzBlocks, frame_no);
        diverged_minor = true;
    }

    if(o_renderedBGOs != g_stats.renderedBGOs)
    {
        pLogWarning("renderedBGOs diverged (old: %d, new: %d) at frame %" PRId64 ".", o_renderedBGOs, g_stats.renderedBGOs, frame_no);
        diverged_minor = true;
    }

    for(int i = 1; i <= numPlayers; i++)
    {
        double px, py;

        if(fscanf(replay_file, "p%dx %lf\r\np%dy %lf\r\n", &success, &px, &success, &py) != 4)
        {
            pLogWarning("old gameplay file diverged (invalid player %d info) at frame %" PRId64 ".", i, frame_no);
            diverged_major = true;
            break;
        }

        // quite non-strict because in a true divergence situation, it will get continually worse
        if(XStd::fabs(px - Player[i].Location.X) > 0.01 ||
           XStd::fabs(py - Player[i].Location.Y) > 0.01)
        {
            pLogWarning("player %d position diverged (old x=%f new x=%f, old y=%f new y=%f) at frame %" PRId64 ".",
                        i,
                        px, Player[i].Location.X,
                        py, Player[i].Location.Y, frame_no);
            diverged_minor = true;
            if(XStd::fabs(px - Player[i].Location.X) > 1 ||
               XStd::fabs(py - Player[i].Location.Y) > 1)
            {
                pLogWarning("  this is a major divergence.");
                diverged_major = true;
            }
        }
    }
}

static void write_NPCs()
{
    fprintf(record_file, " %" PRId64 " \r\nNPCs\r\nnumNPCs %d\r\n", frame_no, numNPCs);
    for(int i = 1; i <= numNPCs; i++)
    {
        const NPC_t& n = NPC[i];
        fprintf(record_file, "NPC %d\r\n", i);
        fprintf(record_file, "Type %d\r\n", n.Type);
        fprintf(record_file, "Active %d\r\n", n.Active);
        fprintf(record_file, "Dir %lf\r\n", n.Direction);
        fprintf(record_file, "XYWH %lf %lf %lf %lf\r\n", n.Location.X, n.Location.Y, n.Location.Width, n.Location.Height);
        fprintf(record_file, "S %lf %lf %lf %lf %lf %lf %lf\r\n", n.Special, n.Special2, n.Special3, n.Special4, n.Special5, n.Special6, n.Special7);
    }
}

static void read_NPCs()
{
    int success = 0;

    fscanf(replay_file, "NPCs\r\n%n", &success);

    // will only possibly be used in the cases where it is initialized by fscanf
    int o_numNPCs;

    if(!success || fscanf(replay_file, "numNPCs %d\r\n", &o_numNPCs) != 1)
        success = 0;
    else if(o_numNPCs != numNPCs)
    {
        pLogWarning("numNPCs diverged (old %d, new %d) at frame %" PRId64 ".", o_numNPCs, numNPCs, frame_no);
        diverged_major = true;
    }

    if(success)
    {
        for(int i = 1; i <= o_numNPCs; i++)
        {
            int N, T, A;
            double D, X, Y, W, H, S1, S2, S3, S4, S5, S6, S7;
            bool invalid = false;

            invalid |= (fscanf(replay_file,
                               "NPC %d\r\n"
                               "Type %d\r\n"
                               "Active %d\r\n",
                               &N, &T, &A) != 3);
            invalid |= fscanf(replay_file,
                              "Dir %lf\r\n"
                              "XYWH %lf %lf %lf %lf\r\n"
                              "S %lf %lf %lf %lf %lf %lf",
                              &D, &X, &Y, &W, &H, &S1, &S2, &S3, &S4, &S5, &S6) != 11;

            if(invalid)
            {
                pLogWarning("old gameplay file diverged (invalid NPC %d data) at frame %" PRId64 ".", i, frame_no);
                diverged_major = true;
                return;
            }

            // either '\r' (no S7) or ' ' (S7)
            if(fgetc(replay_file) == ' ')
            {
                fscanf(replay_file, "%lf\r\n", &S7);
            }
            else
            {
                S7 = 0;
                fgetc(replay_file); // '\n'
            }

            if(N != i)
            {
                pLogWarning("old gameplay file diverged (NPC %d index listed as %d) at frame %" PRId64 ".", i, N, frame_no);
                diverged_major = true;
                continue;
            }

            if(i > numNPCs)
                continue;

            const NPC_t& n = NPC[i];

            if(T != n.Type)
            {
                pLogWarning("NPC[%d].Type diverged (old %d, new %d) at frame %" PRId64 ".", i, T, n.Type, frame_no);
                diverged_major = true;
            }

            if((bool)A != n.Active)
            {
                pLogWarning("NPC[%d].Active diverged (old %d, new %d; type %d) at frame %" PRId64 ".", i, A, n.Active, n.Type, frame_no);
                diverged_minor = true;
            }

            if(!fEqual((float)D, n.Direction))
            {
                pLogWarning("NPC[%d].Direction diverged (old %f, new %f; type %d) at frame %" PRId64 ".", i, D, n.Direction, n.Type, frame_no);
                diverged_minor = true;
            }

            if(XStd::fabs(X - n.Location.X) > 0.01 ||
               XStd::fabs(Y - n.Location.Y) > 0.01 ||
               XStd::fabs(W - n.Location.Width) > 0.01 ||
               XStd::fabs(H - n.Location.Height) > 0.01)
            {
                pLogWarning("NPC[%d].Location diverged (old %lf %lf %lf %lf, new %lf %lf %lf %lf; type %d) at frame %" PRId64 ".", i,
                    X, Y, W, H, n.Location.X, n.Location.Y, n.Location.Width, n.Location.Height, n.Type, frame_no);
                diverged_minor = true;
            }

            double sOld[] = {S1, S2, S3, S4, S5, S6, S7};
            double sNew[] = {n.Special, n.Special2, n.Special3, n.Special4, n.Special5, n.Special6, n.Special7};

            for(int s = 0; s < 7; ++s)
            {
                if(!fEqual(sOld[s], sNew[s]))
                {
                    pLogWarning("NPC[%d].Special%d diverged (old %f => new %f; type %d) at frame %" PRId64 ".",
                                i, s + 1, sOld[s], sNew[s], n.Type, frame_no);
                    diverged_minor = true;
                }
            }

//            if(S1 != n.Special || S2 != n.Special2 || S3 != n.Special3 || S4 != n.Special4 || S5 != n.Special5 || S6 != n.Special6 || S7 != n.Special7)
//            {
//                pLogWarning("NPC[%d].Special* diverged (old %f %f %f %f %f %f %f, new %f %f %f %f %f %f %f; type %d) at frame %" PRId64 ".", i,
//                    S1, S2, S3, S4, S5, S6, S7, n.Special, n.Special2, n.Special3, n.Special4, n.Special5, n.Special6, n.Special7, n.Type, frame_no);
//                diverged_minor = true;
//            }
        }
    }

    if(!success)
    {
        pLogWarning("old gameplay file diverged (invalid NPC header) at frame %" PRId64 ".", frame_no);
        diverged_major = true;
        return;
    }
}

void InitRecording()
{
    if(LevelEditor || GameMenu || GameOutro)
        return;

    if(!g_config.RecordGameplayData && !replay_file)
        return;

    in_level = true;
    diverged_major = false;
    diverged_minor = false;
    frame_no = 0;
    next_record_frame = -1;
    last_status_tick = XStd::GetTicks();
    g_stats.renderedNPCs = 0;
    g_stats.renderedBlocks = 0;
    g_stats.renderedBGOs = 0;

    std::string filename = makeRecordPrefix();

    if(!record_file)
        record_file = Files::utf8_fopen(filename.c_str(), "wb");

    // start of gameplay data
    seedRandom(iRand(32767));

    if(replay_file)
    {
        read_header();
        if(!fscanf(replay_file, "%" PRId64 "\r\n", &next_record_frame))
        {
            pLogWarning("Replayed recording file has prematurely ended.");
            diverged_major = true;
            EndRecording();
        }
    }

    if(record_file)
        write_header();

    for(int i = 0; i < numPlayers; i++)
        last_controls[i] = Controls_t();
}

// need to preload level info from the replay to load with proper compat
void LoadReplay(const std::string &recording_path, const std::string &level_path)
{
    if(LevelEditor || GameMenu || GameOutro)
        return;

    replayLevelFilePath = level_path;

    // figure out how many runs have already happened
    if(!replay_file)
    {
        replay_file = Files::utf8_fopen(recording_path.c_str(), "rb");
        if(replay_file)
            read_header();
    }
}

void EndRecording()
{
    if(!record_file && !replay_file)
        return;

    in_level = false;

    if(record_file)
        write_end();

    if(replay_file)
    {
        read_end();

        if(!diverged_minor && !diverged_major)
        {
            pLogDebug("CONGRATULATIONS! Your build's run did not diverge from the old run.");
            printf("CONGRATULATIONS! Your build's run did not diverge from the old run.\n");

            if(record_file)
                fprintf(record_file, "DID NOT diverge from old run.\r\n");
        }
        else if(!diverged_major)
        {
            pLogDebug("Your build's run only had MINOR divergence from the old run.");
            printf("Your build's run only had MINOR divergence from the old run.\n");

            if(record_file)
                fprintf(record_file, "MINOR divergence from old run.\r\n");
        }
        else
        {
            pLogWarning("I'm sorry, but your build's run DIVERGED from the old run.");
            printf("I'm sorry, but your build's run DIVERGED from the old run.\n");
            if(record_file)
                fprintf(record_file, "DIVERGED from old run.\r\n");
        }

        fclose(replay_file);
        replay_file = nullptr;

        GameIsActive = false;
    }

    if(record_file)
    {
        fclose(record_file);
        record_file = nullptr;
    }
}

void Sync()
{
    if(!record_file && !replay_file)
        return; // Do nothing

    if(replay_file)
    {
        while(next_record_frame == frame_no && replay_file)
        {
            int type = fgetc(replay_file);
            fseek(replay_file, -1, SEEK_CUR);

            if(type == 'S')
                read_status();
            else if(type == 'E')
                read_end();
            else if(type == 'N')
                read_NPCs();
            else if(type == 'C')
                read_control();
            else if(!feof(replay_file))
            {
                pLogWarning("Invalid record type %c in replayed recording file.", type);
                diverged_major = true;
                EndRecording();
                return;
            }

            if(feof(replay_file) || fscanf(replay_file, "%" PRId64 "\r\n", &next_record_frame) != 1)
            {
                pLogWarning("Replayed recording file has prematurely ended.");
                diverged_major = true;
                EndRecording();
                return;
            }
        }

        for(int i = 0; i < numPlayers; i++)
            Player[i+1].Controls = last_controls[i];
    }

    if(record_file)
    {
        write_control();

        if(!(frame_no % 60))
            write_status();

        if(!(frame_no % 900))
            write_NPCs();
    }

    frame_no++;
}

} // namespace Record

#ifdef X_GCC_NO_WARNING_FORMAT
#   pragma GCC diagnostic pop
#endif
