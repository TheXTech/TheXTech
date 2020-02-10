/*
 * A small set of additional math functions and templates
 *
 * Copyright (c) 2017-2020 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies
 * or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
 * FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "maths.h"
#include <cmath>
#include <assert.h>

#include <chrono>
#include <random>

static double osRandom()
{
    auto t = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    std::default_random_engine e;//Seed engine with timed value.
    e.seed(static_cast<std::default_random_engine::result_type>(t));
    std::uniform_real_distribution<double> distribution(0.0, 1.0);
    return distribution(e);
}

int8_t Maths::rand()
{
    return static_cast<int8_t>(((osRandom() * 2.0) - 1.0) * INT8_MAX);
}

uint8_t Maths::urand8()
{
    return static_cast<uint8_t>(osRandom() * UINT8_MAX);
}

int16_t Maths::rand16()
{
    return static_cast<int16_t>(((osRandom() * 2.0) - 1.0) * INT16_MAX);
}

uint16_t Maths::urand16()
{
    return static_cast<uint16_t>(osRandom() * UINT16_MAX);
}

int32_t Maths::rand32()
{
    return static_cast<int32_t>(((osRandom() * 2.0) - 1.0) * INT32_MAX);
}

uint32_t Maths::urand32()
{
    return static_cast<uint32_t>(osRandom() * UINT32_MAX);
}

int64_t Maths::rand64()
{
    return static_cast<int64_t>(((osRandom() * 2.0) - 1.0) * INT64_MAX);
}

uint64_t Maths::urand64()
{
    return static_cast<uint64_t>(osRandom() * UINT64_MAX);
}

float Maths::frand()
{
    return static_cast<float>(osRandom());
}

double Maths::drand()
{
    return osRandom();
}

long Maths::roundTo(long src, long gridSize)
{
    long gridX;

    if(gridSize > 0)
    {
        //ATTACH TO GRID
        gridX = (src - src % gridSize);

        if(src < 0)
        {
            if(src < gridX - static_cast<long>(gridSize / 2.0))
                gridX -= gridSize;
        }
        else
        {
            if(src > gridX + static_cast<long>(gridSize / 2.0))
                gridX += gridSize;
        }

        return gridX;
    }
    else
        return src;
}

double Maths::roundTo(double src, double gridSize)
{
    double gridX;
    src = floor(src);

    if(gridSize > 0)
    {
        //ATTACH TO GRID
        gridX = src - std::fmod(src, gridSize);

        if(src < 0)
        {
            if(src < gridX - floor(gridSize / 2.0))
                gridX -= gridSize;
        }
        else
        {
            if(src > gridX + floor(gridSize / 2.0))
                gridX += gridSize;
        }

        return gridX;
    }
    else
        return src;
}
