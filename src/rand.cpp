/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2021 Vitaly Novichkov <admin@wohlnet.ru>
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

#include <cstdlib>

#include <cstdio>
#include <Utils/files.h>
#include "main/record.h"

#include <pcg/pcg_random.hpp>

#include "globals.h"
#include "rand.h"

static pcg32 g_random_engine;
static pcg32 g_random_engine_isolated;
static long g_random_n_calls = 0;

#ifdef DEBUG_RANDOM_CALLS
std::vector<void*> g_random_calls;
#endif

static int last_seed = 310;

static int s_last_int_random = 0;
static int s_last_int_random_max = 0;
static int s_last_round_int_random = 0;
static int s_last_round_int_random_max = 0;
static double s_last_double_random = 0.0;
static FILE *s_track = nullptr;

void start_rand_track(const char *filePath)
{
    if(!s_track)
        s_track = Files::utf8_fopen(filePath, "wb");
}

void stop_rand_track()
{
    if(s_track)
        std::fclose(s_track);
    s_track = nullptr;
}

static void dump_random_i()
{
    long long fno = (long long)Record::getFrameNo();
    if(s_track)
        std::fprintf(s_track, "%lld: (%ld) I=%d (max=%d)\r\n", fno, g_random_n_calls,
                     s_last_int_random, s_last_int_random_max);
}

static void dump_random_r()
{
    long long fno = (long long)Record::getFrameNo();
    if(s_track)
        std::fprintf(s_track, "%lld: (%ld) R=%d (max=%d)\r\n", fno, g_random_n_calls,
                     s_last_round_int_random, s_last_round_int_random_max);
}

static void dump_random_d()
{
    long long fno = (long long)Record::getFrameNo();
    if(s_track)
        std::fprintf(s_track, "%lld: (%ld) D=%d\r\n", fno, g_random_n_calls,
                     int(s_last_double_random * 1000000));
}

void seedRandom(int seed)
{
    last_seed = seed;
    g_random_n_calls = 0;
#ifdef DEBUG_RANDOM_CALLS
    g_random_calls.clear();
#endif
    g_random_engine.seed(seed);
}

int readSeed()
{
    g_random_engine.seed(last_seed);
    return last_seed;
}

long random_ncalls()
{
    return g_random_n_calls;
}

// Also note that many VB6 calls use dRand * x
// and then assign the result to an Integer.
// The result is NOT iRand(x) but rather vb6Round(dRand()*x),
// iRand_round, which has a different probability distribution
// (prob 1/(2x) of being 0 or x and 1/x of being each number in between)

int iRand(int max)
{
    g_random_n_calls ++;
#ifdef DEBUG_RANDOM_CALLS
    void* stack[2] = {nullptr, nullptr};
    backtrace(stack, 2);
    g_random_calls.push_back(stack[1]);
#endif

    s_last_int_random_max = max;

    if(max == 0)
    {
        g_random_engine();
        s_last_int_random = 0;
        dump_random_i();
        return 0;
    }

    s_last_int_random = g_random_engine() % (max + 1);
    dump_random_i();
    return s_last_int_random;
}

int iRand2(int max)
{
    if(max == 0)
    {
        g_random_engine_isolated();
        return 0;
    }

    return g_random_engine_isolated() % (max + 1);
}

double dRand()
{
    g_random_n_calls ++;

#ifdef DEBUG_RANDOM_CALLS
    void* stack[2] = {nullptr, nullptr};
    backtrace(stack, 2);
    g_random_calls.push_back(stack[1]);
#endif

    s_last_double_random = std::ldexp(g_random_engine(), -32);
    dump_random_d();
    return s_last_double_random;
}

double dRand2()
{
    return std::ldexp(g_random_engine_isolated(), -32);
}

int iRand_round(int max)
{
    g_random_n_calls ++;
    // let 0 represent top endpoint, otherwise use iRand(max*2)/2.
    int i;

    s_last_round_int_random_max = max;

    if(max == 0)
    {
        g_random_engine();
        i = 0;
    }
    else
        i = g_random_engine() % ((max + 1) * 2);

    if(i == 0)
        s_last_round_int_random = max;
    else
        s_last_round_int_random = i / 2;

    dump_random_r();
    return s_last_round_int_random;
}

int iRand_round2(int max)
{
    // let 0 represent top endpoint, otherwise use iRand(max*2)/2.
    int i = iRand2(max * 2);
    if(i == 0)
        return max;
    return i / 2;
}
