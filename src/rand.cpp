/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2021 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
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
