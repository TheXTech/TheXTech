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
static std::mt19937 mt(rd());
static std::uniform_real_distribution<double> distD(0.0, 1.0);
static std::uniform_real_distribution<float> distF(0.0f, 1.0f);
static std::uniform_int_distribution<int> distI(0, RAND_MAX);


float fRand()
{
    return distF(mt);
}

double dRand()
{
    return distD(mt);
}

int iRand()
{
    return distI(mt);
}
