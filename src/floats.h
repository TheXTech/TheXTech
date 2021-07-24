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

#ifndef FLOATS_H
#define FLOATS_H

#include <SDL2/SDL_types.h>
#include <Utils/maths.h>

/**
 * @brief Comparison of two floating point numbers
 * @param a First argument
 * @param b Second argument
 * @return true whe both arguments are equal (almost)
 */
SDL_INLINE bool fEqual(double a, double b)
{
    Sint64 ai = Sint64(Maths::lRound(a * 1000000.0));
    Sint64 bi = Sint64(Maths::lRound(b * 1000000.0));
    return ai == bi;
}

/**
 * @brief Comparison of two floating point numbers
 * @param a First argument
 * @param b Second argument
 * @return true whe both arguments are equal (almost)
 */
SDL_INLINE bool fEqual(float a, float b)
{
    Sint64 ai = Sint64(Maths::lRound(a * 10000.0f));
    Sint64 bi = Sint64(Maths::lRound(b * 10000.0f));
    return ai == bi;
}

SDL_INLINE bool fiEqual(double a, int b)
{
    int ai = Sint64(Maths::iRound(a));
    int bi = b;
    return ai == bi;
}

SDL_INLINE bool fiEqual(float a, int b)
{
    int ai = Sint64(Maths::iRound(a));
    int bi = b;
    return ai == bi;
}

#endif // FLOATS_H
