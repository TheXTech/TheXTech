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

#include <random>
#include <cstdlib>

#include "rand.h"

static std::random_device rd;
static std::mt19937 engine(rd());

void seedRandom(int seed)
{
    engine.seed(seed);
}

double dRand()
{
    return (double)(engine()) / (double)0x100000000;
}

float fRand()
{
    return (float)dRand();
}

// NEVER USED IN SMBX
int iRand()
{
    return engine() & 0x7fffffff;
}

// this is how the original VB6 code does it
int iRand(int max)
{
    return (int)(dRand() * max);
}

// Also note that many VB6 calls use dRand * x
// and then assign the result to an Integer.
// The result is NOT iRand(x) but rather vb6RRound(dRand()*x),
// which has a different probability distribution
// (prob 1/(2x) of being 0 or x and 1/x of being each number in between)