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

#endif // FLOATS_H
