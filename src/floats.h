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

#pragma once
#ifndef FLOATS_H
#define FLOATS_H

#include <cstdint>
#include <Utils/maths.h>

/**
 * @brief Comparison of two floating point numbers
 * @param a First argument
 * @param b Second argument
 * @return true whe both arguments are equal (almost)
 */
inline bool fEqual(double a, double b)
{
    int64_t ai = int64_t(Maths::lRound(a * 1000000.0));
    int64_t bi = int64_t(Maths::lRound(b * 1000000.0));
    return ai == bi;
}

/**
 * @brief Comparison of two floating point numbers
 * @param a First argument
 * @param b Second argument
 * @return true whe both arguments are equal (almost)
 */
inline bool fEqual(float a, float b)
{
    int64_t ai = int64_t(Maths::lRound(a * 10000.0f));
    int64_t bi = int64_t(Maths::lRound(b * 10000.0f));
    return ai == bi;
}

inline bool fiEqual(double a, int b)
{
    int ai = int64_t(Maths::iRound(a));
    int bi = b;
    return ai == bi;
}

inline bool fiEqual(float a, int b)
{
    int ai = int64_t(Maths::iRound(a));
    int bi = b;
    return ai == bi;
}

#endif // FLOATS_H
