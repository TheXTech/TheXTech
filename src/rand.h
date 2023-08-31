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

#pragma once
#ifndef RAND_H
#define RAND_H

#include <cmath>

// supported only on gcc
// #define DEBUG_RANDOM_CALLS
#ifdef DEBUG_RANDOM_CALLS
#   include <execinfo.h>
#   include <vector>
extern std::vector<void*> g_random_calls;
#endif

/**
 * @brief Seeds the random number generator with argument seed for reproducible results
 */
extern void seedRandom(int seed);

/**
 * @brief Reads the most recently set seed and resets the seed to that seed
 * @return current seed
 */
extern int readSeed();

/**
 * @brief Reads the number of calls to random functions since the seed was set
 * @return number of calls
 */
extern long random_ncalls();

/**
 * @brief Resets the random state to simulate the requested number of calls since the seed was set
 */
extern void random_set_ncalls(long ncalls);

/**
 * @brief Random number generator in double format, between 0.0 to 1.0 (exclusive)
 * @return random double value
 */
extern double dRand();

/**
 * @brief Random number generator in integer format, between 0 to argument max (exclusive)
 * Distribution equivalent to `Int(dRand() * max)`
 * @return random integer value
 */
extern int iRand(int max);

template<class T>
int iRandT(T max)
{
    return iRand(static_cast<int>(max));
}

/**
 * @brief Random number generator in integer format, between 0 to argument max (exclusive)
 * Distribution equivalent to `Int(dRand() * max)`
 *
 * Note: It's the second isolated randomizer
 * @return random integer value
 */
extern int iRand2(int max);

template<class T>
int iRand2T(T max)
{
    return iRand2(static_cast<int>(max));
}

/**
 * @brief Random number generator in integer format, between 0 to argument max (inclusive)
 * Each midpoint has probability 1/max. Each endpoint has probability 1/2max.
 * Distribution equivalent to implicitly casting `dRand() * max` to an Int in vb6
 * @return random integer value
 */
inline int iRand_round(int max)
{
    // let 0 represent top endpoint, otherwise use iRand(max*2)/2.
    int i = iRand(max*2);
    if(i == 0)
        return max;
    return i/2;
}

#endif // RAND_H
