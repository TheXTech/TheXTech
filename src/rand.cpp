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

#include <cstdlib>

#include <pcg/pcg_random.hpp>

#include "globals.h"
#include "rand.h"

static pcg32 g_random_engine;
static long g_random_n_calls = 0;

static pcg32 g_random_engine_isolated;

#ifdef DEBUG_RANDOM_CALLS
std::vector<void*> g_random_calls;
#endif

static int last_seed = 310;

void seedRandom(int seed)
{
    last_seed = seed;
    g_random_n_calls = 0;
#ifdef DEBUG_RANDOM_CALLS
    g_random_calls.clear();
#endif
    g_random_engine.seed(seed);
    g_random_engine_isolated.seed(seed);
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

void random_set_ncalls(long ncalls)
{
    g_random_engine.seed(last_seed);

    for(long i = 0; i < ncalls; i++)
        g_random_engine();

    g_random_n_calls = ncalls;
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

    if(max == 0)
    {
        g_random_engine();
        return 0;
    }

    return g_random_engine() % max;
}

int iRand2(int max)
{
    if(max == 0)
    {
        g_random_engine_isolated();
        return 0;
    }

    return g_random_engine_isolated() % max;
}


double dRand()
{
    g_random_n_calls ++;

#ifdef DEBUG_RANDOM_CALLS
    void* stack[2] = {nullptr, nullptr};
    backtrace(stack, 2);
    g_random_calls.push_back(stack[1]);
#endif

    return std::ldexp(g_random_engine(), -32);
}
