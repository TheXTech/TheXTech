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

#ifndef MATHS_H
#define MATHS_H
#include <type_traits>
#include <cmath>
#include <cfloat>
#include <cstdlib>
#include <cstdint>

namespace Maths
{
    int8_t   rand();
    uint8_t  urand8();
    int16_t  rand16();
    uint16_t urand16();
    int32_t  rand32();
    uint32_t urand32();
    int64_t  rand64();
    uint64_t urand64();
    float    frand();
    double   drand();

    long    roundTo(long src, long grid);
    double  roundTo(double src, double grid);

    inline void clearPrecision(double &n)
    {
        n = float(n);
    }

    inline double clearPrecisionRet(double n)
    {
        return float(n);
    }

    inline double roundToUp(double numToRound, double multiple)
    {
        if (multiple == 0)
            return numToRound;

        double remainder = std::fmod(std::fabs(numToRound), multiple);
        if (remainder == 0)
            return numToRound;

        if (numToRound < 0)
            return -(std::fabs(numToRound) - remainder);
        else
            return numToRound + multiple - remainder;
    }

    inline double roundToDown(double num, double multto)
    {
        if(multto == 0.0)
            return num;
        double remind = std::fmod(num, multto);
        if(remind == 0.0)
            return num;
        return num - remind;
    }

    inline int iRound(double d)
    {
        return d >= 0.0 ? int(d + 0.5) : int(d - double(int(d - 1)) + 0.5) + int(d - 1);
    }
    inline unsigned int uRound(double d)
    {
        return d >= 0.0 ?
               static_cast<unsigned int>(d + 0.5) :
               static_cast<unsigned int>(std::abs(d) + 0.5);
    }
    inline long lRound(double d)
    {
        return d >= 0.0 ? long(d + 0.5) : long(d - double(long(d - 1)) + 0.5) + long(d - 1);
    }

    inline int iRound(float d)
    {
        return d >= 0.0f ? int(d + 0.5f) : int(d - float(int(d - 1)) + 0.5f) + int(d - 1);
    }

    inline long lRound(float d)
    {
        return d >= 0.0f ? long(d + 0.5f) : long(d - float(long(d - 1)) + 0.5f) + long(d - 1);
    }

    inline bool equals(long double a, long double b, long double epsilon = LDBL_EPSILON)
    {
        return std::abs(a - b) < epsilon;
    }
    inline bool equals(double a, double b, double epsilon = DBL_EPSILON)
    {
        return std::abs(a - b) < epsilon;
    }
    inline bool equals(float a, float b, float epsilon = FLT_EPSILON)
    {
        return std::abs(a - b) < epsilon;
    }

    template <typename T>
    T max(T n1, T n2)
    {
        static_assert(std::is_arithmetic<T>::value, "The value for \"max\" must be arithemtic");
        return (n1 > n2) ? n1 : n2;
    }

    template <typename T>
    T min(T n1, T n2)
    {
        static_assert(std::is_arithmetic<T>::value, "The value for \"min\" must be arithemtic");
        return (n1 < n2) ? n1 : n2;
    }
    //    template <typename T>
    //    static int sgn(T val) {
    //        static_assert(std::is_arithmetic<T>::value, "The value for \"val\" must be arithemtic");
    //        return int(T(0) < val) - int(val < T(0));
    //    }
    template <typename T>
    T sgn(T val)
    {
        return (T(0) < val) - (val < T(0));
    }
} // namespace Maths

#endif // MATHS_H
