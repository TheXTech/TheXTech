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
#include "speedrunner.h"
#include "game_main.h"

#include <SDL2/SDL.h>
#include <chrono>
#include <fmt_time_ne.h>
#include <fmt_format_ne.h>

#include <cinttypes>
#include <Utils/files.h>
#include <DirManager/dirman.h>
#include <AppPath/app_path.h>
#include <Logger/logger.h>

#ifndef PRId64 /*Workaround*/
#   ifndef __PRI64_PREFIX
#       if __WORDSIZE == 64
#           define __PRI64_PREFIX	"l"
#           define __PRIPTR_PREFIX	"l"
#       else
#           define __PRI64_PREFIX	"ll"
#           define __PRIPTR_PREFIX
#       endif
#   endif
#   define PRId64		__PRI64_PREFIX "d"
#endif

std::string makeRecordPrefix()
{
    auto now = std::chrono::system_clock::now();
    std::time_t in_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm t = fmt::localtime_ne(in_time_t);

    return fmt::sprintf_ne("%s/%s_%s_%04d-%02d-%02d_%02d-%02d-%02d.rec",
                           AppPathManager::gameplayRecordsRootDir(), FileName, SHORT_VERSION,
                           (1900 + t.tm_year), (1 + t.tm_mon), t.tm_mday,
                           t.tm_hour, t.tm_min, t.tm_sec);
}

// std::string makeRecordPrefix()
// {
//     // want to figure out path of filename relative to game engine, then save in the recording root.
//     std::string recordDir = AppPathManager::gameplayRecordsRootDir() + "/"
//         + Files::basename(Files::dirname(FullFileName));
//     if(!DirMan::exists(recordDir))
//         DirMan::mkAbsPath(recordDir);

//     return recordDir + "/" + FileNameFull + ".rec";
// }

namespace Record
{

// public

FILE* record_file = nullptr;
FILE* replay_file = nullptr;

// private

bool         in_level = false;
bool         diverged = false;
int64_t      frame_no = 0;
int64_t      next_record_frame = 0;
uint32_t     last_status_tick = 0;
Controls_t   last_controls[maxPlayers];

void write_header()
{
    // write all necessary state variables!
    fprintf(record_file, "Header\r\n");
    fprintf(record_file, "%s\r\n", LONG_VERSION); // game version / commit
    fprintf(record_file, " %d \r\n", CompatGetLevel()); // compatibility mode
    fprintf(record_file, "%s\r\n", FullFileName.c_str()); // level that was played
    fprintf(record_file, " %d \r\n", readSeed());
    fprintf(record_file, " %d \r\n", (Checkpoint == FullFileName) ? 1 : 0); // update for multipoints?
    fprintf(record_file, " %d \r\n", StartWarp);
    fprintf(record_file, " %d \r\n", ReturnWarp);
    fprintf(record_file, " %d \r\n", (int)Lives);
    fprintf(record_file, " %d \r\n", Coins);
    fprintf(record_file, " %d \r\n", Score);
    fprintf(record_file, " %d \r\n", numStars);

    for(int A = 1; A <= numStars; A++)
    {
        fprintf(record_file, "Star\r\n");
        fprintf(record_file, "%s\r\n", Star[A].level.c_str());
        fprintf(record_file, " %d \r\n", Star[A].Section);
    }

    fprintf(record_file, " %d \r\n", numPlayers);

    for(int A = 1; A <= numPlayers; A++)
    {
        fprintf(record_file, "Player\r\n %d \r\n %d \r\n %d \r\n %d \r\n",
                Player[A].Character, Player[A].State, Player[A].MountType, Player[A].HeldBonus);
    }
}

void read_header()
{
    rewind(replay_file); // fseek(replay_file, 0, SEEK_SET);

    // buffer is a 1024-character buffer used for reading strings, shared with the record_init() function.
    char buffer[1024];

    // n is an integer for some implicit conversions
    int n;

    // read all necessary state variables!
    fgets(buffer, 1024, replay_file); // "Header"
    fgets(buffer, 1024, replay_file); // game version / commit
    fscanf(replay_file, "%d\r\n", &n); // compatibility mode

    CompatSetEnforcedLevel(n);

    if(CompatGetLevel() < COMPAT_SMBX2)
        pLogWarning("compatibility mode is not a long-term support version. Do not expect identical results.");

    fgets(buffer, 1024, replay_file); // level that was played

    for(int i = 0; i < 1024; i++)
        if(buffer[i] == '\r') buffer[i] = '\0'; // clip the newline :(

    // now SET the filename
    FullFileName = buffer;
    // if(SDL_strcasecmp(buffer, FileNameFull.c_str()))
    //     pLogWarning("FileName does not match.");

    fscanf(replay_file, "%d\r\n", &n); // random seed
    seedRandom(n);

    fscanf(replay_file, "%d\r\n", &n); // is there a checkpoint?

    Checkpoint = n ? FullFileName : "";

    fscanf(replay_file, "%d\r\n", &StartWarp);
    fscanf(replay_file, "%d\r\n", &ReturnWarp);
    fscanf(replay_file, "%d\r\n", &n);
    Lives = n;
    fscanf(replay_file, "%d\r\n", &Coins);
    fscanf(replay_file, "%d\r\n", &Score);
    fscanf(replay_file, "%d\r\n", &numStars);

    for(int A = 1; A <= numStars; A++)
    {
        fgets(buffer, 1024, replay_file); // "Star"
        fgets(buffer, 1024, replay_file); // section

        for(int i = 0; i < 1024; i++)
            if(buffer[i] == '\r') buffer[i] = '\0'; // clip the newline :(

        Star[A].level = buffer;
        fscanf(replay_file, "%d\r\n", &Star[A].Section);
    }

    fscanf(replay_file, "%d\r\n", &numPlayers);

    for(int A = 1; A <= numPlayers; A++)
    {
        fscanf(replay_file, "Player\r\n%d\r\n%d\r\n%d\r\n%d\r\n",
            &Player[A].Character, &Player[A].State, &Player[A].MountType, &Player[A].HeldBonus);
    }

    Cheater = true; // important to avoid losing player save data in replay mode.
    TestLevel = false;
    MaxFPS = true;
    ShowFPS = true;
}

void write_end()
{
    fprintf(record_file, " %" PRId64 " \r\nEnd\r\n %d \r\n", frame_no+1, LevelBeatCode);
}

void read_end()
{
    int b;

    if(fscanf(replay_file, "End\r\n%d\r\n", &b) != 1)
    {
        pLogWarning("old gameplay file diverged (invalid end header).");
        diverged = true;
    }

    if(next_record_frame-1 != frame_no)
    {
        pLogWarning("final frame_no diverged (old: %" PRId64 ", new: %" PRId64 ").", next_record_frame-1, frame_no);
        diverged = true;
    }

    if(b != LevelBeatCode)
    {
        pLogWarning("LevelBeatCode diverged (old: %d, new: %d).", b, LevelBeatCode);
        diverged = true;
    }
}

void write_control()
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

void read_control()
{
    int p;
    char mode, key;

    if(fscanf(replay_file, "C%c%d%c\r\n", &mode, &p, &key) != 3)
    {
        return;
    }

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

void write_status()
{
    if(frame_no == 0)
    {
        g_stats.renderedNPCs = 0;
        g_stats.renderedBlocks = 0;
        g_stats.renderedSzBlocks = 0;
        g_stats.renderedBGOs = 0;
    }

    fprintf(record_file, " %" PRId64 " \r\nStatus\r\n", frame_no);
    uint32_t status_tick = SDL_GetTicks();
    fprintf(record_file, " %d \r\n", SDL_GetTicks() - last_status_tick);
    last_status_tick = status_tick;
    fprintf(record_file, " %d \r\n", Score);
    fprintf(record_file, " %d \r\n", numNPCs);

    int numActiveNPCs = 0;
    if(frame_no != 0)
    {
        for(int i = 1; i <= numNPCs; i++)
        {
            if(NPC[i].Active)
                numActiveNPCs ++;
        }
    }

    fprintf(record_file, " %d \r\n", numActiveNPCs);
    fprintf(record_file, " %d \r\n %d \r\n %d \r\n",
        g_stats.renderedNPCs, g_stats.renderedBlocks + g_stats.renderedSzBlocks, g_stats.renderedBGOs);

    for(int i = 1; i <= numPlayers; i++)
    {
        fprintf(record_file, "%lf\r\n%lf\r\n",
            Player[i].Location.X, Player[i].Location.Y);
    }

    fflush(record_file);
}

void read_status()
{
    if(frame_no == 0)
    {
        g_stats.renderedNPCs = 0;
        g_stats.renderedBlocks = 0;
        g_stats.renderedSzBlocks = 0;
        g_stats.renderedBGOs = 0;
    }

    int o_ticks, o_Score, o_numNPCs, o_numActiveNPCs, o_renderedNPCs, o_renderedBlocks, o_renderedBGOs;

    int success = 0;

    fscanf(replay_file, "Status\r\n%n", &success);

    if(!success)
    {
        pLogWarning("old gameplay file diverged (invalid status header) at frame %" PRId64 ".", frame_no);
        diverged = true;
        return;
    }
    if(fscanf(replay_file, "%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n",
        &o_ticks, &o_Score, &o_numNPCs, &o_numActiveNPCs, &o_renderedNPCs, &o_renderedBlocks, &o_renderedBGOs) != 7)
    {
        pLogWarning("old gameplay file diverged (invalid status info) at frame %" PRId64 ".", frame_no);
        diverged = true;
        return;
    }

    if(o_Score != Score)
    {
        pLogWarning("score diverged (old: %d, new: %d) at frame %" PRId64 ".", o_Score, Score, frame_no);
        diverged = true;
    }

    if(o_numNPCs != numNPCs)
    {
        pLogWarning("numNPCs diverged (old: %d, new: %d) at frame %" PRId64 ".", o_numNPCs, numNPCs, frame_no);
        diverged = true;
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
        diverged = true;
    }

    if(o_renderedNPCs != g_stats.renderedNPCs)
    {
        pLogWarning("renderedNPCs diverged (old: %d, new: %d) at frame %" PRId64 ".", o_renderedNPCs, g_stats.renderedNPCs, frame_no);
        diverged = true;
    }

    if(o_renderedBlocks != g_stats.renderedBlocks + g_stats.renderedSzBlocks)
    {
        pLogWarning("renderedBlocks diverged (old: %d, new: %d) at frame %" PRId64 ".", o_renderedNPCs, g_stats.renderedNPCs + g_stats.renderedSzBlocks, frame_no);
        diverged = true;
    }

    if(o_renderedBGOs != g_stats.renderedBGOs)
    {
        pLogWarning("renderedBGOs diverged (old: %d, new: %d) at frame %" PRId64 ".", o_renderedBGOs, g_stats.renderedBGOs, frame_no);
        diverged = true;
    }

    for(int i = 1; i <= numPlayers; i++)
    {
        double px, py;

        if(fscanf(replay_file, "%lf\r\n%lf\r\n", &px, &py) != 2)
        {
            pLogWarning("old gameplay file diverged (invalid player %d info) at frame %" PRId64 ".", i, frame_no);
            diverged = true;
            break;
        }

        // quite non-strict because in a true divergence situation, it will get continually worse
        if(SDL_fabs(px - Player[i].Location.X) > 0.01 || SDL_fabs(py - Player[i].Location.Y) > 0.01)
        {
            pLogWarning("player %d position diverged (old: %f %f, new: %f %f) at frame %" PRId64 ".", i, px, py, Player[i].Location.X, Player[i].Location.Y, frame_no);
            diverged = true;
        }
    }
}

void write_NPCs()
{
    fprintf(record_file, " %" PRId64 " \r\nNPCs\r\n %d \r\n", frame_no, numNPCs);
    for(int i = 1; i <= numNPCs; i++)
    {
        const NPC_t& n = NPC[i];
        fprintf(record_file, "NPC %d\r\n", i);
        fprintf(record_file, "Type %d\r\n", n.Type);
        fprintf(record_file, "Active %d\r\n", n.Active);
        fprintf(record_file, "Dir %lf\r\n", n.Direction);
        fprintf(record_file, "XYWH %lf %lf %lf %lf\r\n", n.Location.X, n.Location.Y, n.Location.Width, n.Location.Height);
        fprintf(record_file, "S %lf %lf %lf %lf %lf %lf\r\n", n.Special, n.Special2, n.Special3, n.Special4, n.Special5, n.Special6);
    }
}

void read_NPCs()
{
    int success = 0;

    fscanf(replay_file, "NPCs\r\n%n", &success);
    int o_numNPCs;
    if(!success || fscanf(replay_file, " %d \r\n", &o_numNPCs) != 1)
        success = 0;
    else if(o_numNPCs != numNPCs)
    {
        pLogWarning("old gameplay file diverged (new numNPCs %d, old %d) at frame %" PRId64 ".", numNPCs, o_numNPCs, frame_no);
        diverged = true;
        return;
    }

    if(success)
    {
        for(int i = 1; i <= o_numNPCs; i++)
        {
            int N, T, A;
            double D, X, Y, W, H, S1, S2, S3, S4, S5, S6;
            if(fscanf(replay_file, "NPC %d\r\nType %d\r\nActive %d\r\n", &N, &T, &A) != 3
                || fscanf(replay_file, "Dir %lf\r\nXYWH %lf %lf %lf %lf\r\nS %lf %lf %lf %lf %lf %lf\r\n",
                    &D, &X, &Y, &W, &H, &S1, &S2, &S3, &S4, &S5, &S6) != 11)
            {
                pLogWarning("old gameplay file diverged (invalid NPC %d data) at frame %" PRId64 ".", i, frame_no);
                diverged = true;
                return;
            }
            if(N != i)
            {
                pLogWarning("old gameplay file diverged (NPC %d index listed as %d) at frame %" PRId64 ".", i, N, frame_no);
                diverged = true;
                continue;
            }
            if(i > numNPCs)
                continue;
            const NPC_t& n = NPC[i];
            if(T != n.Type)
            {
                pLogWarning("old gameplay file diverged (new NPC[%d].Type %d, old %d) at frame %" PRId64 ".", i, n.Type, T, frame_no);
                diverged = true;
            }
            if(A != n.Active)
            {
                pLogWarning("old gameplay file diverged (new NPC[%d].Active %d, old %d) at frame %" PRId64 ".", i, n.Active, A, frame_no);
                diverged = true;
            }
            if(D != n.Direction)
            {
                pLogWarning("old gameplay file diverged (new NPC[%d].Direction %f, old %f) at frame %" PRId64 ".", i, n.Direction, D, frame_no);
                diverged = true;
            }
            if(SDL_fabs(X - n.Location.X) > 0.01 || SDL_fabs(Y - n.Location.Y) > 0.01 || SDL_fabs(W - n.Location.Width) > 0.01 || SDL_fabs(H - n.Location.Height) > 0.01)
            {
                pLogWarning("old gameplay file diverged (new NPC[%d].Location %lf %lf %lf %lf, old %lf %lf %lf %lf) at frame %" PRId64 ".", i,
                    n.Location.X, n.Location.Y, n.Location.Width, n.Location.Height, X, Y, W, H, frame_no);
                diverged = true;
            }
            if(S1 != n.Special || S2 != n.Special2 || S3 != n.Special3 || S4 != n.Special4 || S5 != n.Special5 || S6 != n.Special6)
            {
                pLogWarning("old gameplay file diverged (new NPC[%d].Special* %f %f %f %f %f %f, old %f %f %f %f %f %f) at frame %" PRId64 ".", i,
                    n.Special, n.Special2, n.Special3, n.Special4, n.Special5, n.Special6, S1, S2, S3, S4, S5, S6, frame_no);
                diverged = true;
            }
        }
    }

    if(!success)
    {
        pLogWarning("old gameplay file diverged (invalid NPC header) at frame %" PRId64 ".", frame_no);
        diverged = true;
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
    diverged = false;
    frame_no = 0;
    next_record_frame = -1;
    last_status_tick = SDL_GetTicks();
    g_stats.renderedNPCs = 0;
    g_stats.renderedBlocks = 0;
    g_stats.renderedBGOs = 0;

    std::string filename = makeRecordPrefix();

    if(!record_file)
        record_file = Files::utf8_fopen(filename.c_str(), "wb");

    // start of gameplay data
    seedRandom(iRand());
    if(replay_file)
    {
        read_header();
        if(!fscanf(replay_file, "%" PRId64 "\r\n", &next_record_frame))
        {
            pLogWarning("Replayed recording file has prematurely ended.");
            diverged = true;
            EndRecording();
        }
    }
    if(record_file)
        write_header();

    for(int i = 0; i < numPlayers; i++)
    {
        last_controls[i] = Controls_t();
    }
}

// need to preload level info from the replay to load with proper compat
void LoadReplay(std::string recording_path)
{
    if(LevelEditor || GameMenu || GameOutro)
        return;

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

        if(!diverged)
        {
            pLogDebug("CONGRATULATIONS! Your build's run did not diverge from the old run.");
            printf("CONGRATULATIONS! Your build's run did not diverge from the old run.\n");
            if(record_file)
                fprintf(record_file, "DID NOT diverge from old run.\r\n");
        }

        fclose(replay_file);
        replay_file = nullptr;

        GameIsActive = false;
    }

    if(diverged)
    {
        pLogWarning("I'm sorry, but your build's run DIVERGED from the old run.");
        printf("I'm sorry, but your build's run DIVERGED from the old run.\n");
        if(record_file)
            fprintf(record_file, "DIVERGED from old run.\r\n");
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
                diverged = true;
                EndRecording();
                return;
            }

            if(feof(replay_file) || fscanf(replay_file, "%" PRId64 "\r\n", &next_record_frame) != 1)
            {
                pLogWarning("Replayed recording file has prematurely ended.");
                diverged = true;
                EndRecording();
                return;
            }
        }

        for(int i = 0; i < numPlayers; i++)
        {
            Player[i+1].Controls = last_controls[i];
        }
    }
    if(record_file)
    {
        write_control();

        if(!(frame_no % 60))
        {
            write_status();
        }

        if(!(frame_no % 900))
        {
            write_NPCs();
        }
    }

    frame_no++;
}

}; // namespace Record
