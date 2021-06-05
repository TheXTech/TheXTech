// this module handles particular the control recording and playback functions
// and the gameplay stats recording functions

#include "../version.h"

#define LONG_VERSION "TheXTech branch " V_BUILD_BRANCH " commit " V_BUILD_VER
#define SHORT_VERSION V_BUILD_VER

#include "../globals.h"
#include "../rand.h"
#include "../frame_timer.h"
#include "record.h"
#include "speedrunner.h"
#include "game_main.h"

#include <Utils/files.h>
#include <DirManager/dirman.h>
#include <AppPath/app_path.h>
#include <Logger/logger.h>

std::string makeRecordPrefix(int index)
{
    // want to figure out path of filename relative to game engine, then save in the recording root.
    std::string recordDir = AppPathManager::gameplayRecordsRootDir() + "/"
        + Files::basename(Files::dirname(FullFileName));
    if(!DirMan::exists(recordDir))
        DirMan::mkAbsPath(recordDir);

    return recordDir + "/" + FileNameFull + "." + std::to_string(index);
}

std::string findRecordPrefix(int index)
{
    std::string recordPath = AppPathManager::gameplayRecordsRootDir()
        + "/" + Files::basename(Files::dirname(FullFileName))
        + "/" + FileNameFull + "." + std::to_string(index);

    FILE* e;
    if((e = Files::utf8_fopen((recordPath + ".c.txt").c_str(), "rb")))
    {
        fclose(e);
        return recordPath;
    }

    return "";
}

bool g_recordControlReplay = false;
bool g_recordControlRecord = false;
int g_recordReplayId = -1;

// private
static FILE* s_recordControlFile = nullptr;
static FILE* s_recordGameplayFile = nullptr;
static FILE* s_recordOldGameplayFile = nullptr;

static bool in_level = false;
static bool diverged = false;
static int64_t frame_no = 0;
static int64_t next_delta_frame = 0;
static Controls_t last_controls[maxPlayers];

void record_writestate()
{
    // write all necessary state variables!
    fprintf(s_recordGameplayFile, "Init\r\n");
    fprintf(s_recordGameplayFile, "%s\r\n", LONG_VERSION); // game version / commit
    fprintf(s_recordGameplayFile, " %d \r\n", g_speedRunnerMode); // compatibility mode
    fprintf(s_recordGameplayFile, "%s\r\n", FileNameFull.c_str()); // level that was played
    fprintf(s_recordGameplayFile, " %d \r\n", (Checkpoint == FullFileName) ? 1 : 0);
    fprintf(s_recordGameplayFile, " %d \r\n", StartWarp);
    fprintf(s_recordGameplayFile, " %d \r\n", ReturnWarp);
    fprintf(s_recordGameplayFile, " %d \r\n", (int)Lives);
    fprintf(s_recordGameplayFile, " %d \r\n", Coins);
    fprintf(s_recordGameplayFile, " %d \r\n", Score);
    fprintf(s_recordGameplayFile, " %d \r\n", numStars);

    for(int A = 1; A <= numStars; A++)
    {
        fprintf(s_recordGameplayFile, "Star\r\n");
        fprintf(s_recordGameplayFile, "%s\r\n", Star[A].level.c_str());
        fprintf(s_recordGameplayFile, " %d \r\n", Star[A].Section);
    }

    fprintf(s_recordGameplayFile, " %d \r\n", numPlayers);

    for(int A = 1; A <= numPlayers; A++)
    {
        fprintf(s_recordGameplayFile, "Player\r\n %d \r\n %d \r\n %d \r\n %d \r\n",
                Player[A].Character, Player[A].State, Player[A].MountType, Player[A].HeldBonus);
    }
}

void record_readstate()
{
    // buffer is a 1024-character buffer used for reading strings, shared with the record_init() function.
    char buffer[1024];

    // n is an integer for some implicit conversions
    int n;

    // read all necessary state variables!
    fgets(buffer, 1024, s_recordOldGameplayFile); // "Init"
    fgets(buffer, 1024, s_recordOldGameplayFile); // game version / commit
    fscanf(s_recordOldGameplayFile, "%d\r\n", &g_speedRunnerMode); // compatibility mode

    if(g_speedRunnerMode < 2)
        pLogWarning("compatibility mode is not a long-term support version. Do not expect identical results.");

    fgets(buffer, 1024, s_recordOldGameplayFile); // level that was played

    for(int i = 0; i < 1024; i++)
        if(buffer[i] == '\r') buffer[i] = '\0'; // clip the newline :(

    if(strcasecmp(buffer, FileNameFull.c_str()))
        pLogWarning("FileName does not match.");

    fscanf(s_recordOldGameplayFile, "%d\r\n", &n); // is there a checkpoint?

    Checkpoint = n ? FullFileName : "";
    /*if(n)
        Checkpoint = n ? FullFileName : "";
    else
        Checkpoint = "";*/

    fscanf(s_recordOldGameplayFile, "%d\r\n", &StartWarp);
    fscanf(s_recordOldGameplayFile, "%d\r\n", &ReturnWarp);
    fscanf(s_recordOldGameplayFile, "%d\r\n", &n);
    Lives = n;
    fscanf(s_recordOldGameplayFile, "%d\r\n", &Coins);
    fscanf(s_recordOldGameplayFile, "%d\r\n", &Score);
    fscanf(s_recordOldGameplayFile, "%d\r\n", &numStars);

    for(int A = 1; A <= numStars; A++)
    {
        fgets(buffer, 1024, s_recordOldGameplayFile); // "Star"
        fgets(buffer, 1024, s_recordOldGameplayFile); // section

        for(int i = 0; i < 1024; i++)
            if(buffer[i] == '\r') buffer[i] = '\0'; // clip the newline :(

        Star[A].level = buffer;
        fscanf(s_recordOldGameplayFile, "%d\r\n", &Star[A].Section);
    }

    fscanf(s_recordOldGameplayFile, "%d\r\n", &numPlayers);

    for(int A = 1; A <= numPlayers; A++)
    {
        fscanf(s_recordOldGameplayFile, "Player\r\n%d\r\n%d\r\n%d\r\n%d\r\n",
            &Player[A].Character, &Player[A].State, &Player[A].MountType, &Player[A].HeldBonus);
    }

    Cheater = true; // important to avoid losing player save data in replay mode.
}

void record_init()
{
    if(!g_recordControlReplay && !g_recordControlRecord)
        return;

    in_level = true;
    diverged = false;
    frame_no = 0;
    next_delta_frame = -1;
    g_stats.renderedNPCs = 0;
    g_stats.renderedBlocks = 0;
    g_stats.renderedBGOs = 0;

    seedRandom(310);

    // figure out how many runs have already happened
    int next_run;
    if(g_recordReplayId != -1)
    {
        next_run = g_recordReplayId + 1;
    }
    else
    {
        next_run = 0;
        while(!findRecordPrefix(next_run).empty())
        {
            next_run ++;
        }
    }

    if(g_recordControlRecord)
    {
        std::string filename = makeRecordPrefix(next_run)+".c.txt";

        if(!s_recordControlFile)
        {
            s_recordControlFile = Files::utf8_fopen(filename.c_str(), "wb");
        }

        if(!s_recordGameplayFile)
        {
            filename.end()[-5] = 'g';
            s_recordGameplayFile = Files::utf8_fopen(filename.c_str(), "wb");
        }
    }
    else if(g_recordControlReplay)
    {
        std::string filename = findRecordPrefix(next_run-1);

        if(filename.empty())
        {
            // nothing to reproduce
            return;
        }

        filename += ".c.txt";

        if(!s_recordControlFile)
            s_recordControlFile = Files::utf8_fopen(filename.c_str(), "rb");

        if(!s_recordOldGameplayFile)
        {
            filename.end()[-5] = 'g';
            s_recordOldGameplayFile = Files::utf8_fopen(filename.c_str(), "rb");
            if(s_recordOldGameplayFile)
            {
                record_readstate();
            }
        }

        if(s_recordControlFile && !s_recordGameplayFile)
        {
            filename.replace(filename.end()-5, filename.end(), "r." SHORT_VERSION ".txt");
            s_recordGameplayFile = Files::utf8_fopen(filename.c_str(), "wb");
        }
    }

    // start of gameplay data
    if(s_recordGameplayFile)
        record_writestate();

    for(int i = 0; i < numPlayers; i++)
    {
        last_controls[i] = Controls_t();
    }
}

// need to preload level info from the replay to load with proper compat
void record_preload()
{
    if(!g_recordControlReplay && !g_recordControlRecord)
        return;

    if(LevelEditor || GameMenu || GameOutro)
        return;

    // figure out how many runs have already happened
    int next_run;

    if(g_recordReplayId != -1)
    {
        next_run = g_recordReplayId + 1;
    }
    else
    {
        next_run = 0;
        while(!findRecordPrefix(next_run).empty())
            next_run ++;
    }

    if(!g_recordControlRecord && g_recordControlReplay)
    {
        std::string filename = findRecordPrefix(next_run-1);
        if(filename.empty())
        {
            // nothing to reproduce
            return;
        }

        if(!s_recordOldGameplayFile)
        {
            filename += ".g.txt";
            s_recordOldGameplayFile = Files::utf8_fopen(filename.c_str(), "rb");
            if(s_recordOldGameplayFile)
            {
                record_readstate();
                fclose(s_recordOldGameplayFile);
                s_recordOldGameplayFile = nullptr;
            }
        }
    }
}

void record_finish()
{
    in_level = false;

    if(s_recordControlFile)
    {
        fclose(s_recordControlFile);
        s_recordControlFile = nullptr;
    }

    if(s_recordGameplayFile)
        fprintf(s_recordGameplayFile, "End\r\n %ld \r\n %d \r\n", frame_no, LevelBeatCode);

    if(s_recordOldGameplayFile)
    {
        int64_t f;
        int b;

        if(fscanf(s_recordOldGameplayFile, "End\r\n%ld\r\n%d\r\n", &f, &b) != 2)
        {
            pLogWarning("old gameplay file diverged (invalid end header).");
            diverged = true;
        }

        if(f != frame_no)
        {
            pLogWarning("final frame_no diverged (old: %ld, new: %ld).", f, frame_no);
            diverged = true;
        }

        if(b != LevelBeatCode)
        {
            pLogWarning("LevelBeatCode diverged (old: %d, new: %d).", b, LevelBeatCode);
            diverged = true;
        }

        if(!diverged)
        {
            pLogDebug("CONGRATULATIONS! Your build's run did not diverge from the old run.");
            printf("CONGRATULATIONS! Your build's run did not diverge from the old run.\n");
            if(s_recordGameplayFile)
                fprintf(s_recordGameplayFile, "DID NOT diverge from old run.\n");
        }

        fclose(s_recordOldGameplayFile);
        s_recordOldGameplayFile = nullptr;
    }

    if(diverged)
    {
        pLogWarning("I'm sorry, but your build's run DIVERGED from the old run.");
        printf("I'm sorry, but your build's run DIVERGED from the old run.\n");
        if(s_recordGameplayFile)
            fprintf(s_recordGameplayFile, "DIVERGED from old run.\n");
    }

    if(s_recordGameplayFile)
    {
        fclose(s_recordGameplayFile);
        s_recordGameplayFile = nullptr;
    }

    in_level = false;

    if(TestLevel)
    {
        GameIsActive = false;
    }
}

void record_sync()
{
    if(!in_level)
        return;
    if(!g_recordControlReplay && !g_recordControlRecord)
        return;
    if(!s_recordControlFile && !s_recordGameplayFile)
        return; // Do nothing

    if(g_recordControlReplay)
    {
        if(next_delta_frame == -1 && s_recordControlFile)
        {
            if(!fscanf(s_recordControlFile, "%ld\r\n", &next_delta_frame))
            {
                fclose(s_recordControlFile);
                s_recordControlFile = nullptr;
            }
        }

        while(next_delta_frame == frame_no && s_recordControlFile)
        {
            int p;
            char mode, key;

            if(fscanf(s_recordControlFile, "%c%d%c\r\n", &mode, &p, &key) != 3)
            {
                fclose(s_recordControlFile);
                s_recordControlFile = nullptr;
                break;
            }

            bool set = (mode != '-');
            /*if(mode == '-')
                set = false;
            else
                set = true;*/

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

            if(!fscanf(s_recordControlFile, "%ld\r\n", &next_delta_frame))
            {
                fclose(s_recordControlFile);
                s_recordControlFile = nullptr;
            }
        }

        for(int i = 0; i < numPlayers; i++)
        {
            Player[i+1].Controls = last_controls[i];
        }
    }
    else if(g_recordControlRecord && s_recordControlFile)
    {
        for(int i = 0; i < numPlayers; i++)
        {
            const Controls_t& keys = Player[i+1].Controls;

            if(!last_controls[i].Up && keys.Up)
                fprintf(s_recordControlFile, "%ld\r\n+%dU\r\n", frame_no, i+1);
            else if(last_controls[i].Up && !keys.Up)
                fprintf(s_recordControlFile, "%ld\r\n-%dU\r\n", frame_no, i+1);

            if(!last_controls[i].Down && keys.Down)
                fprintf(s_recordControlFile, "%ld\r\n+%dD\r\n", frame_no, i+1);
            else if(last_controls[i].Down && !keys.Down)
                fprintf(s_recordControlFile, "%ld\r\n-%dD\r\n", frame_no, i+1);

            if(!last_controls[i].Left && keys.Left)
                fprintf(s_recordControlFile, "%ld\r\n+%dL\r\n", frame_no, i+1);
            else if(last_controls[i].Left && !keys.Left)
                fprintf(s_recordControlFile, "%ld\r\n-%dL\r\n", frame_no, i+1);

            if(!last_controls[i].Right && keys.Right)
                fprintf(s_recordControlFile, "%ld\r\n+%dR\r\n", frame_no, i+1);
            else if(last_controls[i].Right && !keys.Right)
                fprintf(s_recordControlFile, "%ld\r\n-%dR\r\n", frame_no, i+1);

            if(!last_controls[i].Start && keys.Start)
                fprintf(s_recordControlFile, "%ld\r\n+%dS\r\n", frame_no, i+1);
            else if(last_controls[i].Start && !keys.Start)
                fprintf(s_recordControlFile, "%ld\r\n-%dS\r\n", frame_no, i+1);

            if(!last_controls[i].Drop && keys.Drop)
                fprintf(s_recordControlFile, "%ld\r\n+%dI\r\n", frame_no, i+1);
            else if(last_controls[i].Drop && !keys.Drop)
                fprintf(s_recordControlFile, "%ld\r\n-%dI\r\n", frame_no, i+1);

            if(!last_controls[i].Jump && keys.Jump)
                fprintf(s_recordControlFile, "%ld\r\n+%dA\r\n", frame_no, i+1);
            else if(last_controls[i].Jump && !keys.Jump)
                fprintf(s_recordControlFile, "%ld\r\n-%dA\r\n", frame_no, i+1);

            if(!last_controls[i].Run && keys.Run)
                fprintf(s_recordControlFile, "%ld\r\n+%dB\r\n", frame_no, i+1);
            else if(last_controls[i].Run && !keys.Run)
                fprintf(s_recordControlFile, "%ld\r\n-%dB\r\n", frame_no, i+1);

            if(!last_controls[i].AltJump && keys.AltJump)
                fprintf(s_recordControlFile, "%ld\r\n+%dX\r\n", frame_no, i+1);
            else if(last_controls[i].AltJump && !keys.AltJump)
                fprintf(s_recordControlFile, "%ld\r\n-%dX\r\n", frame_no, i+1);

            if(!last_controls[i].AltRun && keys.AltRun)
                fprintf(s_recordControlFile, "%ld\r\n+%dY\r\n", frame_no, i+1);
            else if(last_controls[i].AltRun && !keys.AltRun)
                fprintf(s_recordControlFile, "%ld\r\n-%dY\r\n", frame_no, i+1);

            last_controls[i] = keys;
        }
    }

    if(s_recordGameplayFile && !(frame_no % 60))
    {
        if(frame_no == 0)
        {
            g_stats.renderedNPCs = 0;
            g_stats.renderedBlocks = 0;
            g_stats.renderedSzBlocks = 0;
            g_stats.renderedBGOs = 0;
        }

        fprintf(s_recordGameplayFile, "Frame\r\n %ld \r\n",
            frame_no);
        fprintf(s_recordGameplayFile, " %d \r\n", Score);
        fprintf(s_recordGameplayFile, " %d \r\n", numNPCs);

        int numActiveNPCs = 0;
        if(frame_no != 0)
        {
            for(int i = 1; i <= numNPCs; i++)
            {
                if(NPC[i].Active)
                    numActiveNPCs ++;
            }
        }

        fprintf(s_recordGameplayFile, " %d \r\n", numActiveNPCs);
        fprintf(s_recordGameplayFile, " %d \r\n %d \r\n %d \r\n",
            g_stats.renderedNPCs, g_stats.renderedBlocks + g_stats.renderedSzBlocks, g_stats.renderedBGOs);

        for(int i = 1; i <= numPlayers; i++)
        {
            fprintf(s_recordGameplayFile, "%lf\r\n%lf\r\n",
                Player[i].Location.X, Player[i].Location.Y);
        }

        if(s_recordOldGameplayFile)
        {
            int64_t f;
            int o_Score, o_numNPCs, o_numActiveNPCs, o_renderedNPCs, o_renderedBlocks, o_renderedBGOs;

            if(!fscanf(s_recordOldGameplayFile, "Frame\r\n%ld\r\n", &f) || f != frame_no)
            {
                pLogWarning("old gameplay file diverged (invalid frame header) at frame %ld.", frame_no);
                diverged = true;
                fclose(s_recordOldGameplayFile);
                s_recordOldGameplayFile = nullptr;
                KillIt();
            }
            else if(fscanf(s_recordOldGameplayFile, "%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n",
                &o_Score, &o_numNPCs, &o_numActiveNPCs, &o_renderedNPCs, &o_renderedBlocks, &o_renderedBGOs) != 6)
            {
                pLogWarning("old gameplay file diverged (invalid frame info) at frame %ld.", frame_no);
                diverged = true;
                fclose(s_recordOldGameplayFile);
                s_recordOldGameplayFile = nullptr;
            }
            else
            {
                if(o_Score != Score)
                {
                    pLogWarning("score diverged (old: %d, new: %d) at frame %ld.", o_Score, Score, frame_no);
                    diverged = true;
                }

                if(o_numNPCs != numNPCs)
                {
                    pLogWarning("numNPCs diverged (old: %d, new: %d) at frame %ld.", o_numNPCs, numNPCs, frame_no);
                    diverged = true;
                }

                if(o_numActiveNPCs != numActiveNPCs)
                {
                    pLogWarning("numActiveNPCs diverged (old: %d, new: %d) at frame %ld.", o_numActiveNPCs, numActiveNPCs, frame_no);
                    diverged = true;
                }

                if(o_renderedNPCs != g_stats.renderedNPCs)
                {
                    pLogWarning("renderedNPCs diverged (old: %d, new: %d) at frame %ld.", o_renderedNPCs, g_stats.renderedNPCs, frame_no);
                    diverged = true;
                }

                if(o_renderedBlocks != g_stats.renderedBlocks + g_stats.renderedSzBlocks)
                {
                    pLogWarning("renderedBlocks diverged (old: %d, new: %d) at frame %ld.", o_renderedNPCs, g_stats.renderedNPCs + g_stats.renderedSzBlocks, frame_no);
                    diverged = true;
                }

                if(o_renderedBGOs != g_stats.renderedBGOs)
                {
                    pLogWarning("renderedBGOs diverged (old: %d, new: %d) at frame %ld.", o_renderedBGOs, g_stats.renderedBGOs, frame_no);
                    diverged = true;
                }

                for(int i = 1; i <= numPlayers; i++)
                {
                    double px, py;

                    if(fscanf(s_recordOldGameplayFile, "%lf\r\n%lf\r\n", &px, &py) != 2)
                    {
                        pLogWarning("old gameplay file diverged (invalid player %d info) at frame %ld.", i, frame_no);
                        diverged = true;
                        fclose(s_recordOldGameplayFile);
                        s_recordOldGameplayFile = nullptr;
                        break;
                    }

                    // quite non-strict because in a true divergence situation, it will get continually worse
                    if(fabs(px - Player[i].Location.X) > 0.01 || fabs(py - Player[i].Location.Y) > 0.01)
                    {
                        pLogWarning("player %d position diverged (old: %f %f, new: %f %f) at frame %ld.", i, px, py, Player[i].Location.X, Player[i].Location.Y, frame_no);
                        diverged = true;
                    }
                }
            }
        }
    }

    frame_no++;
}
