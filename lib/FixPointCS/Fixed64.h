//
// FixPointCS
//
// Copyright(c) Jere Sanisalo, Petri Kero
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY INT64_C(C)AIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER INT64_C(DEA)INGS IN THE
// SOFTWARE.
//

//
// GENERATED FILE!!!
//
// Generated from Fixed64.cs, part of the FixPointCS project (MIT license).
//
#pragma once
#ifndef __FIXED64_H
#define __FIXED64_H

// Include numeric types
#include <stdint.h>
#include "FixedUtil.h"


// If FP_ASSERT is not custom-defined, then use the standard one
#ifndef FP_ASSERT
#   include <assert.h>
#   define FP_ASSERT(x) assert(x)
#endif

// If FP_CUSTOM_INVALID_ARGS is defined, then the used is expected to implement the following functions in
// the FixedUtil namespace:
//  void InvalidArgument(const char* funcName, const char* argName, FP_INT argValue);
//  void InvalidArgument(const char* funcName, const char* argName, FP_INT argValue1, FP_INT argValue2);
//	void InvalidArgument(const char* funcName, const char* argName, FP_LONG argValue);
//	void InvalidArgument(const char* funcName, const char* argName, FP_LONG argValue1, FP_LONG argValue2);
// These functions should handle the cases for invalid arguments in any desired way (assert, exception, log, ignore etc).
//#define FP_CUSTOM_INVALID_ARGS

namespace Fixed64
{
    typedef int32_t FP_INT;
    typedef uint32_t FP_UINT;
    typedef int64_t FP_LONG;
    typedef uint64_t FP_ULONG;

    static_assert(sizeof(FP_INT) == 4, "Wrong bytesize for FP_INT");
    static_assert(sizeof(FP_UINT) == 4, "Wrong bytesize for FP_UINT");
    static_assert(sizeof(FP_LONG) == 8, "Wrong bytesize for FP_LONG");
    static_assert(sizeof(FP_ULONG) == 8, "Wrong bytesize for FP_ULONG");



    static const FP_INT Shift = 32;
    static const FP_LONG FractionMask = ( INT64_C(1) << Shift ) - 1; // Space before INT64_C(1) needed because of hacky C++ code generator
    static const FP_LONG IntegerMask = ~FractionMask;

    // Constants
    static const FP_LONG Zero = INT64_C(0);
    static const FP_LONG Neg1 = INT64_C(-1) << Shift;
    static const FP_LONG One = INT64_C(1) << Shift;
    static const FP_LONG Two = INT64_C(2) << Shift;
    static const FP_LONG Three = INT64_C(3) << Shift;
    static const FP_LONG Four = INT64_C(4) << Shift;
    static const FP_LONG Half = One >> 1;
    static const FP_LONG Pi = INT64_C(13493037705); //(FP_LONG)(Math.PI * 65536.0) << 16;
    static const FP_LONG Pi2 = INT64_C(26986075409);
    static const FP_LONG PiHalf = INT64_C(6746518852);
    static const FP_LONG E = INT64_C(11674931555);

    static const FP_LONG MinValue = INT64_MIN;
    static const FP_LONG MaxValue = INT64_MAX;

    // Private constants
    static const FP_LONG RCP_LN2      = INT64_C(0x171547652); // 1.0 / log(2.0) ~= 1.4426950408889634
    static const FP_LONG RCP_LOG2_E   = INT64_C(2977044471);  // 1.0 / log2(e) ~= 0.6931471805599453
    static const FP_INT  RCP_HALF_PI  = 683565276; // 1.0 / (4.0 * 0.5 * Math.PI);  // the 4.0 factor converts directly to s2.30

    /// <summary>
    /// Converts an integer to a fixed-point value.
    /// </summary>
    static FP_LONG FromInt(FP_INT v)
    {
        return (FP_LONG)v << Shift;
    }

    /// <summary>
    /// Converts a double to a fixed-point value.
    /// </summary>
    static FP_LONG FromDouble(double v)
    {
        return (FP_LONG)(v * 4294967296.0);
    }

    /// <summary>
    /// Converts a float to a fixed-point value.
    /// </summary>
    static FP_LONG FromFloat(float v)
    {
        return (FP_LONG)(v * 4294967296.0f);
    }

    /// <summary>
    /// Converts a fixed-point value into an integer by rounding it up to nearest integer.
    /// </summary>
    static FP_INT CeilToInt(FP_LONG v)
    {
        return (FP_INT)((v + (One - 1)) >> Shift);
    }

    /// <summary>
    /// Converts a fixed-point value into an integer by rounding it down to nearest integer.
    /// </summary>
    static FP_INT FloorToInt(FP_LONG v)
    {
        return (FP_INT)(v >> Shift);
    }

    /// <summary>
    /// Converts a fixed-point value into an integer by rounding it to nearest integer.
    /// </summary>
    static FP_INT RoundToInt(FP_LONG v)
    {
        return (FP_INT)((v + Half) >> Shift);
    }

    /// <summary>
    /// Converts a fixed-point value into a double.
    /// </summary>
    static double ToDouble(FP_LONG v)
    {
        return (double)v * (1.0 / 4294967296.0);
    }

    /// <summary>
    /// Converts a FP value into a float.
    /// </summary>
    static float ToFloat(FP_LONG v)
    {
        return (float)v * (1.0f / 4294967296.0f);
    }

    /// <summary>
    /// Converts the value to a human readable string.
    /// </summary>

    /// <summary>
    /// Returns the absolute (positive) value of x.
    /// </summary>
    static FP_LONG Abs(FP_LONG x)
    {
        // \note fails with LONG_MIN
        FP_LONG mask = x >> 63;
        return (x + mask) ^ mask;
    }

    /// <summary>
    /// Negative absolute value (returns -abs(x)).
    /// </summary>
    static FP_LONG Nabs(FP_LONG x)
    {
        return -Abs(x);
    }

    /// <summary>
    /// Round up to nearest integer.
    /// </summary>
    static FP_LONG Ceil(FP_LONG x)
    {
        return (x + FractionMask) & IntegerMask;
    }

    /// <summary>
    /// Round down to nearest integer.
    /// </summary>
    static FP_LONG Floor(FP_LONG x)
    {
        return x & IntegerMask;
    }

    /// <summary>
    /// Round to nearest integer.
    /// </summary>
    static FP_LONG Round(FP_LONG x)
    {
        return (x + Half) & IntegerMask;
    }

    /// <summary>
    /// Returns the fractional part of x. Equal to 'x - floor(x)'.
    /// </summary>
    static FP_LONG Fract(FP_LONG x)
    {
        return x & FractionMask;
    }

    /// <summary>
    /// Returns the minimum of the two values.
    /// </summary>
    static FP_LONG Min(FP_LONG a, FP_LONG b)
    {
        return (a < b) ? a : b;
    }

    /// <summary>
    /// Returns the maximum of the two values.
    /// </summary>
    static FP_LONG Max(FP_LONG a, FP_LONG b)
    {
        return (a > b) ? a : b;
    }

    /// <summary>
    /// Returns the value clamped between min and max.
    /// </summary>
    static FP_LONG Clamp(FP_LONG a, FP_LONG min, FP_LONG max)
    {
        return (a > max) ? max : (a < min) ? min : a;
    }

    /// <summary>
    /// Returns the sign of the value (-1 if negative, 0 if zero, 1 if positive).
    /// </summary>
    static FP_INT Sign(FP_LONG x)
    {
        // https://stackoverflow.com/questions/14579920/fast-sign-of-integer-in-c/14612418#14612418
        return (FP_INT)((x >> 63) | (FP_LONG)(((FP_ULONG)-x) >> 63));
    }

    /// <summary>
    /// Adds the two FP numbers together.
    /// </summary>
    static FP_LONG Add(FP_LONG a, FP_LONG b)
    {
        return a + b;
    }

    /// <summary>
    /// Subtracts the two FP numbers from each other.
    /// </summary>
    static FP_LONG Sub(FP_LONG a, FP_LONG b)
    {
        return a - b;
    }

    /// <summary>
    /// Multiplies two FP values together.
    /// </summary>
    static FP_LONG Mul(FP_LONG a, FP_LONG b)
    {
        FP_LONG ai = a >> Shift;
        FP_LONG af = (a & FractionMask);
        FP_LONG bi = b >> Shift;
        FP_LONG bf = (b & FractionMask);
        return FixedUtil::LogicalShiftRight(af * bf, Shift) + ai * b + af * bi;
    }

    static FP_INT MulIntLongLow(FP_INT a, FP_LONG b)
    {
        FP_ASSERT(a >= 0);
        FP_INT bi = (FP_INT)(b >> Shift);
        FP_LONG bf = b & FractionMask;
        return (FP_INT)FixedUtil::LogicalShiftRight(a * bf, Shift) + a * bi;
    }

    static FP_LONG MulIntLongLong(FP_INT a, FP_LONG b)
    {
        FP_ASSERT(a >= 0);
        FP_LONG bi = b >> Shift;
        FP_LONG bf = b & FractionMask;
        return FixedUtil::LogicalShiftRight(a * bf, Shift) + a * bi;
    }

    /// <summary>
    /// Linearly interpolate from a to b by t.
    /// </summary>
    static FP_LONG Lerp(FP_LONG a, FP_LONG b, FP_LONG t)
    {
        return Mul(a, t) + Mul(b, One - t);
    }

    static FP_INT Nlz(FP_ULONG x)
    {
    #if NET5_0_OR_GREATER
        return System.Numerics.BitOperations.LeadingZeroCount(x);
    #else
        FP_INT n = 0;
        if (x <= INT64_C(0x00000000FFFFFFFF)) { n = n + 32; x = x << 32; }
        if (x <= INT64_C(0x0000FFFFFFFFFFFF)) { n = n + 16; x = x << 16; }
        if (x <= INT64_C(0x00FFFFFFFFFFFFFF)) { n = n + 8; x = x << 8; }
        if (x <= INT64_C(0x0FFFFFFFFFFFFFFF)) { n = n + 4; x = x << 4; }
        if (x <= INT64_C(0x3FFFFFFFFFFFFFFF)) { n = n + 2; x = x << 2; }
        if (x <= INT64_C(0x7FFFFFFFFFFFFFFF)) { n = n + 1; }
        if (x == 0) return 64;
        return n;
    #endif
    }

    /// <summary>
    /// Divides two FP values.
    /// </summary>
    static FP_LONG DivPrecise(FP_LONG arg_a, FP_LONG arg_b)
    {
        // From http://www.hackersdelight.org/hdcodetxt/divlu.c.txt

        FP_LONG sign_dif = arg_a ^ arg_b;

        static const FP_ULONG b = INT64_C(0x100000000); // Number base (32 bits)
        FP_ULONG abs_arg_a = (FP_ULONG)((arg_a < 0) ? -arg_a : arg_a);
        FP_ULONG u1 = abs_arg_a >> 32;
        FP_ULONG u0 = abs_arg_a << 32;
        FP_ULONG v = (FP_ULONG)((arg_b < 0) ? -arg_b : arg_b);

        // Overflow?
        if (u1 >= v)
        {
            //rem = 0;
            return INT64_C(0x7fffffffffffffff);
        }

        // Shift amount for norm
        FP_INT s = Nlz(v); // 0 <= s <= 63
        v = v << s; // Normalize the divisor
        FP_ULONG vn1 = v >> 32; // Break the divisor into two 32-bit digits
        FP_ULONG vn0 = v & INT64_C(0xffffffff);

        FP_ULONG un32 = (u1 << s) | (u0 >> (64 - s)) & (FP_ULONG)((FP_LONG)-s >> 63);
        FP_ULONG un10 = u0 << s; // Shift dividend left

        FP_ULONG un1 = un10 >> 32; // Break the right half of dividend into two digits
        FP_ULONG un0 = un10 & INT64_C(0xffffffff);

        // Compute the first quotient digit, q1
        FP_ULONG q1 = un32 / vn1;
        FP_ULONG rhat = un32 - q1 * vn1;
        do
        {
            if ((q1 >= b) || ((q1 * vn0) > (b * rhat + un1)))
            {
                q1 = q1 - 1;
                rhat = rhat + vn1;
            }
            else break;
        } while (rhat < b);

        FP_ULONG un21 = un32 * b + un1 - q1 * v; // Multiply and subtract

        // Compute the second quotient digit, q0
        FP_ULONG q0 = un21 / vn1;
        rhat = un21 - q0 * vn1;
        do
        {
            if ((q0 >= b) || ((q0 * vn0) > (b * rhat + un0)))
            {
                q0 = q0 - 1;
                rhat = rhat + vn1;
            }
            else break;
        } while (rhat < b);

        // Calculate the remainder
        // FP_ULONG r = (un21 * b + un0 - q0 * v) >> s;
        // rem = (FP_LONG)r;

        FP_ULONG ret = q1 * b + q0;
        return (sign_dif < 0) ? -(FP_LONG)ret : (FP_LONG)ret;
    }

    /// <summary>
    /// Calculates division approximation.
    /// </summary>
    static FP_LONG Div(FP_LONG a, FP_LONG b)
    {
        if (b == MinValue || b == 0)
        {
            FixedUtil::InvalidArgument("Fixed64::Div", "b", b);
            return 0;
        }

        // Handle negative values.
        FP_INT sign = (b < 0) ? -1 : 1;
        b *= sign;

        // Normalize input into [1.0, 2.0( range (convert to s2.30).
        FP_INT offset = 31 - Nlz((FP_ULONG)b);
        FP_INT n = (FP_INT)FixedUtil::ShiftRight(b, offset + 2);
        static const FP_INT ONE = (1 << 30);
        FP_ASSERT(n >= ONE);

        // Polynomial approximation.
        FP_INT res = FixedUtil::RcpPoly4Lut8(n - ONE);

        // Apply exponent, convert back to s32.32.
        FP_LONG y = MulIntLongLong(res, a) << 2;
        return FixedUtil::ShiftRight(sign * y, offset);
    }

    /// <summary>
    /// Calculates division approximation.
    /// </summary>
    static FP_LONG DivFast(FP_LONG a, FP_LONG b)
    {
        if (b == MinValue || b == 0)
        {
            FixedUtil::InvalidArgument("Fixed64::DivFast", "b", b);
            return 0;
        }

        // Handle negative values.
        FP_INT sign = (b < 0) ? -1 : 1;
        b *= sign;

        // Normalize input into [1.0, 2.0( range (convert to s2.30).
        FP_INT offset = 31 - Nlz((FP_ULONG)b);
        FP_INT n = (FP_INT)FixedUtil::ShiftRight(b, offset + 2);
        static const FP_INT ONE = (1 << 30);
        FP_ASSERT(n >= ONE);

        // Polynomial approximation.
        FP_INT res = FixedUtil::RcpPoly6(n - ONE);

        // Apply exponent, convert back to s32.32.
        FP_LONG y = MulIntLongLong(res, a) << 2;
        return FixedUtil::ShiftRight(sign * y, offset);
    }

    /// <summary>
    /// Calculates division approximation.
    /// </summary>
    static FP_LONG DivFastest(FP_LONG a, FP_LONG b)
    {
        if (b == MinValue || b == 0)
        {
            FixedUtil::InvalidArgument("Fixed64::DivFastest", "b", b);
            return 0;
        }

        // Handle negative values.
        FP_INT sign = (b < 0) ? -1 : 1;
        b *= sign;

        // Normalize input into [1.0, 2.0( range (convert to s2.30).
        FP_INT offset = 31 - Nlz((FP_ULONG)b);
        FP_INT n = (FP_INT)FixedUtil::ShiftRight(b, offset + 2);
        static const FP_INT ONE = (1 << 30);
        FP_ASSERT(n >= ONE);

        // Polynomial approximation.
        FP_INT res = FixedUtil::RcpPoly4(n - ONE);

        // Apply exponent, convert back to s32.32.
        FP_LONG y = MulIntLongLong(res, a) << 2;
        return FixedUtil::ShiftRight(sign * y, offset);
    }

    /// <summary>
    /// Divides two FP values and returns the modulus.
    /// </summary>
    static FP_LONG Mod(FP_LONG a, FP_LONG b)
    {
        if (b == 0)
        {
            FixedUtil::InvalidArgument("Fixed64::Mod", "b", b);
            return 0;
        }

        return a % b;
    }

    /// <summary>
    /// Calculates the square root of the given number.
    /// </summary>
    static FP_LONG SqrtPrecise(FP_LONG a)
    {
        // Adapted from https://github.com/chmike/fpsqrt
        if (a <= 0)
        {
            if (a < 0)
                FixedUtil::InvalidArgument("Fixed64::SqrtPrecise", "a", a);
            return 0;
        }

        FP_ULONG r = (FP_ULONG)a;
        FP_ULONG b = INT64_C(0x4000000000000000);
        FP_ULONG q = INT64_C(0);
        while (b > INT64_C(0x40))
        {
            FP_ULONG t = q + b;
            if (r >= t)
            {
                r -= t;
                q = t + b;
            }
            r <<= 1;
            b >>= 1;
        }
        q >>= 16;
        return (FP_LONG)q;
    }

    static FP_LONG Sqrt(FP_LONG x)
    {
        // Return 0 for all non-positive values.
        if (x <= 0)
        {
            if (x < 0)
                FixedUtil::InvalidArgument("Fixed64::Sqrt", "x", x);
            return 0;
        }

        // Constants (s2.30).
        static const FP_INT ONE = (1 << 30);
        static const FP_INT SQRT2 = 1518500249; // sqrt(2.0)

        // Normalize input into [1.0, 2.0( range (as s2.30).
        FP_INT offset = 31 - Nlz((FP_ULONG)x);
        FP_INT n = (FP_INT)(((offset >= 0) ? (x >> offset) : (x << -offset)) >> 2);
        FP_ASSERT(n >= ONE);
        FP_INT y = FixedUtil::SqrtPoly3Lut8(n - ONE);

        // Divide offset by 2 (to get sqrt), compute adjust value for odd exponents.
        FP_INT adjust = ((offset & 1) != 0) ? SQRT2 : ONE;
        offset = offset >> 1;

        // Apply exponent, convert back to s32.32.
        FP_LONG yr = (FP_LONG)FixedUtil::Qmul30(adjust, y) << 2;
        return (offset >= 0) ? (yr << offset) : (yr >> -offset);
    }

    static FP_LONG SqrtFast(FP_LONG x)
    {
        // Return 0 for all non-positive values.
        if (x <= 0)
        {
            if (x < 0)
                FixedUtil::InvalidArgument("Fixed64::SqrtFast", "x", x);
            return 0;
        }

        // Constants (s2.30).
        static const FP_INT ONE = (1 << 30);
        static const FP_INT SQRT2 = 1518500249; // sqrt(2.0)

        // Normalize input into [1.0, 2.0( range (as s2.30).
        FP_INT offset = 31 - Nlz((FP_ULONG)x);
        FP_INT n = (FP_INT)(((offset >= 0) ? (x >> offset) : (x << -offset)) >> 2);
        FP_ASSERT(n >= ONE);
        FP_INT y = FixedUtil::SqrtPoly4(n - ONE);

        // Divide offset by 2 (to get sqrt), compute adjust value for odd exponents.
        FP_INT adjust = ((offset & 1) != 0) ? SQRT2 : ONE;
        offset = offset >> 1;

        // Apply exponent, convert back to s32.32.
        FP_LONG yr = (FP_LONG)FixedUtil::Qmul30(adjust, y) << 2;
        return (offset >= 0) ? (yr << offset) : (yr >> -offset);
    }

    static FP_LONG SqrtFastest(FP_LONG x)
    {
        // Return 0 for all non-positive values.
        if (x <= 0)
        {
            if (x < 0)
                FixedUtil::InvalidArgument("Fixed64::SqrtFastest", "x", x);
            return 0;
        }

        // Constants (s2.30).
        static const FP_INT ONE = (1 << 30);
        static const FP_INT SQRT2 = 1518500249; // sqrt(2.0)

        // Normalize input into [1.0, 2.0( range (as s2.30).
        FP_INT offset = 31 - Nlz((FP_ULONG)x);
        FP_INT n = (FP_INT)(((offset >= 0) ? (x >> offset) : (x << -offset)) >> 2);
        FP_ASSERT(n >= ONE);
        FP_INT y = FixedUtil::SqrtPoly3(n - ONE);

        // Divide offset by 2 (to get sqrt), compute adjust value for odd exponents.
        FP_INT adjust = ((offset & 1) != 0) ? SQRT2 : ONE;
        offset = offset >> 1;

        // Apply exponent, convert back to s32.32.
        FP_LONG yr = (FP_LONG)FixedUtil::Qmul30(adjust, y) << 2;
        return (offset >= 0) ? (yr << offset) : (yr >> -offset);
    }

    /// <summary>
    /// Calculates the reciprocal square root.
    /// </summary>
    static FP_LONG RSqrt(FP_LONG x)
    {
        // Return 0 for invalid values
        if (x <= 0)
        {
            FixedUtil::InvalidArgument("Fixed64::RSqrt", "x", x);
            return 0;
        }

        // Constants (s2.30).
        static const FP_INT ONE = (1 << 30);
        static const FP_INT HALF_SQRT2 = 759250125; // 0.5 * sqrt(2.0)

        // Normalize input into [1.0, 2.0( range (as s2.30).
        FP_INT offset = 31 - Nlz((FP_ULONG)x);
        FP_INT n = (FP_INT)(((offset >= 0) ? (x >> offset) : (x << -offset)) >> 2);
        FP_ASSERT(n >= ONE);
        FP_INT y = FixedUtil::RSqrtPoly3Lut16(n - ONE);

        // Divide offset by 2 (to get sqrt), compute adjust value for odd exponents.
        FP_INT adjust = ((offset & 1) != 0) ? HALF_SQRT2 : ONE;
        offset = offset >> 1;

        // Apply exponent, convert back to s32.32.
        FP_LONG yr = (FP_LONG)FixedUtil::Qmul30(adjust, y) << 2;
        return (offset >= 0) ? (yr >> offset) : (yr << -offset);
    }

    /// <summary>
    /// Calculates the reciprocal square root.
    /// </summary>
    static FP_LONG RSqrtFast(FP_LONG x)
    {
        // Return 0 for invalid values
        if (x <= 0)
        {
            FixedUtil::InvalidArgument("Fixed64::RSqrtFast", "x", x);
            return 0;
        }

        // Constants (s2.30).
        static const FP_INT ONE = (1 << 30);
        static const FP_INT HALF_SQRT2 = 759250125; // 0.5 * sqrt(2.0)

        // Normalize input into [1.0, 2.0( range (as s2.30).
        FP_INT offset = 31 - Nlz((FP_ULONG)x);
        FP_INT n = (FP_INT)(((offset >= 0) ? (x >> offset) : (x << -offset)) >> 2);
        FP_ASSERT(n >= ONE);
        FP_INT y = FixedUtil::RSqrtPoly5(n - ONE);

        // Divide offset by 2 (to get sqrt), compute adjust value for odd exponents.
        FP_INT adjust = ((offset & 1) != 0) ? HALF_SQRT2 : ONE;
        offset = offset >> 1;

        // Apply exponent, convert back to s32.32.
        FP_LONG yr = (FP_LONG)FixedUtil::Qmul30(adjust, y) << 2;
        return (offset >= 0) ? (yr >> offset) : (yr << -offset);
    }

    /// <summary>
    /// Calculates the reciprocal square root.
    /// </summary>
    static FP_LONG RSqrtFastest(FP_LONG x)
    {
        // Return 0 for invalid values
        if (x <= 0)
        {
            FixedUtil::InvalidArgument("Fixed64::RSqrtFastest", "x", x);
            return 0;
        }

        // Constants (s2.30).
        static const FP_INT ONE = (1 << 30);
        static const FP_INT HALF_SQRT2 = 759250125; // 0.5 * sqrt(2.0)

        // Normalize input into [1.0, 2.0( range (as s2.30).
        FP_INT offset = 31 - Nlz((FP_ULONG)x);
        FP_INT n = (FP_INT)(((offset >= 0) ? (x >> offset) : (x << -offset)) >> 2);
        FP_ASSERT(n >= ONE);
        FP_INT y = FixedUtil::RSqrtPoly3(n - ONE);

        // Divide offset by 2 (to get sqrt), compute adjust value for odd exponents.
        FP_INT adjust = ((offset & 1) != 0) ? HALF_SQRT2 : ONE;
        offset = offset >> 1;

        // Apply exponent, convert back to s32.32.
        FP_LONG yr = (FP_LONG)FixedUtil::Qmul30(adjust, y) << 2;
        return (offset >= 0) ? (yr >> offset) : (yr << -offset);
    }

    /// <summary>
    /// Calculates reciprocal approximation.
    /// </summary>
    static FP_LONG Rcp(FP_LONG x)
    {
        if (x == MinValue || x == 0)
        {
            FixedUtil::InvalidArgument("Fixed64::Rcp", "x", x);
            return 0;
        }

        // Handle negative values.
        FP_INT sign = (x < 0) ? -1 : 1;
        x *= sign;

        // Normalize input into [1.0, 2.0( range (convert to s2.30).
        FP_INT offset = 31 - Nlz((FP_ULONG)x);
        FP_INT n = (FP_INT)FixedUtil::ShiftRight(x, offset + 2);
        static const FP_INT ONE = (1 << 30);
        FP_ASSERT(n >= ONE);

        // Polynomial approximation.
        FP_INT res = FixedUtil::RcpPoly4Lut8(n - ONE);
        FP_LONG y = (FP_LONG)(sign * res) << 2;

        // Apply exponent, convert back to s32.32.
        return FixedUtil::ShiftRight(y, offset);
    }

    /// <summary>
    /// Calculates reciprocal approximation.
    /// </summary>
    static FP_LONG RcpFast(FP_LONG x)
    {
        if (x == MinValue || x == 0)
        {
            FixedUtil::InvalidArgument("Fixed64::RcpFast", "x", x);
            return 0;
        }

        // Handle negative values.
        FP_INT sign = (x < 0) ? -1 : 1;
        x *= sign;

        // Normalize input into [1.0, 2.0( range (convert to s2.30).
        FP_INT offset = 31 - Nlz((FP_ULONG)x);
        FP_INT n = (FP_INT)FixedUtil::ShiftRight(x, offset + 2);
        static const FP_INT ONE = (1 << 30);
        FP_ASSERT(n >= ONE);

        // Polynomial approximation.
        FP_INT res = FixedUtil::RcpPoly6(n - ONE);
        FP_LONG y = (FP_LONG)(sign * res) << 2;

        // Apply exponent, convert back to s32.32.
        return FixedUtil::ShiftRight(y, offset);
    }

    /// <summary>
    /// Calculates reciprocal approximation.
    /// </summary>
    static FP_LONG RcpFastest(FP_LONG x)
    {
        if (x == MinValue || x == 0)
        {
            FixedUtil::InvalidArgument("Fixed64::RcpFastest", "x", x);
            return 0;
        }

        // Handle negative values.
        FP_INT sign = (x < 0) ? -1 : 1;
        x *= sign;

        // Normalize input into [1.0, 2.0( range (convert to s2.30).
        static const FP_INT ONE = (1 << 30);
        FP_INT offset = 31 - Nlz((FP_ULONG)x);
        FP_INT n = (FP_INT)FixedUtil::ShiftRight(x, offset + 2);
        //FP_INT n = (FP_INT)(((offset >= 0) ? (x >> offset) : (x << -offset)) >> 2);

        // Polynomial approximation.
        FP_INT res = FixedUtil::RcpPoly4(n - ONE);
        FP_LONG y = (FP_LONG)(sign * res) << 2;

        // Apply exponent, convert back to s32.32.
        return FixedUtil::ShiftRight(y, offset);
    }

    /// <summary>
    /// Calculates the base 2 exponent.
    /// </summary>
    static FP_LONG Exp2(FP_LONG x)
    {
        // Handle values that would under or overflow.
        if (x >= 32 * One) return MaxValue;
        if (x <= -32 * One) return 0;

        // Compute exp2 for fractional part.
        FP_INT k = (FP_INT)((x & FractionMask) >> 2);
        FP_LONG y = (FP_LONG)FixedUtil::Exp2Poly5(k) << 2;

        // Combine integer and fractional result, and convert back to s32.32.
        FP_INT intPart = (FP_INT)(x >> Shift);
        return (intPart >= 0) ? (y << intPart) : (y >> -intPart);
    }

    /// <summary>
    /// Calculates the base 2 exponent.
    /// </summary>
    static FP_LONG Exp2Fast(FP_LONG x)
    {
        // Handle values that would under or overflow.
        if (x >= 32 * One) return MaxValue;
        if (x <= -32 * One) return 0;

        // Compute exp2 for fractional part.
        FP_INT k = (FP_INT)((x & FractionMask) >> 2);
        FP_LONG y = (FP_LONG)FixedUtil::Exp2Poly4(k) << 2;

        // Combine integer and fractional result, and convert back to s32.32.
        FP_INT intPart = (FP_INT)(x >> Shift);
        return (intPart >= 0) ? (y << intPart) : (y >> -intPart);
    }

    /// <summary>
    /// Calculates the base 2 exponent.
    /// </summary>
    static FP_LONG Exp2Fastest(FP_LONG x)
    {
        // Handle values that would under or overflow.
        if (x >= 32 * One) return MaxValue;
        if (x <= -32 * One) return 0;

        // Compute exp2 for fractional part.
        FP_INT k = (FP_INT)((x & FractionMask) >> 2);
        FP_LONG y = (FP_LONG)FixedUtil::Exp2Poly3(k) << 2;

        // Combine integer and fractional result, and convert back to s32.32.
        FP_INT intPart = (FP_INT)(x >> Shift);
        return (intPart >= 0) ? (y << intPart) : (y >> -intPart);
    }

    static FP_LONG Exp(FP_LONG x)
    {
        // e^x == 2^(x / ln(2))
        return Exp2(Mul(x, RCP_LN2));
    }

    static FP_LONG ExpFast(FP_LONG x)
    {
        // e^x == 2^(x / ln(2))
        return Exp2Fast(Mul(x, RCP_LN2));
    }

    static FP_LONG ExpFastest(FP_LONG x)
    {
        // e^x == 2^(x / ln(2))
        return Exp2Fastest(Mul(x, RCP_LN2));
    }

    // Natural logarithm (base e).
    static FP_LONG Log(FP_LONG x)
    {
        // Return 0 for invalid values
        if (x <= 0)
        {
            FixedUtil::InvalidArgument("Fixed64::Log", "x", x);
            return 0;
        }

        // Normalize value to range [1.0, 2.0( as s2.30 and extract exponent.
        static const FP_INT ONE = (1 << 30);
        FP_INT offset = 31 - Nlz((FP_ULONG)x);
        FP_INT n = (FP_INT)(((offset >= 0) ? (x >> offset) : (x << -offset)) >> 2);
        FP_ASSERT(n >= ONE);
        FP_LONG y = (FP_LONG)FixedUtil::LogPoly5Lut8(n - ONE) << 2;

        // Combine integer and fractional parts (into s32.32).
        return (FP_LONG)offset * RCP_LOG2_E + y;
    }

    static FP_LONG LogFast(FP_LONG x)
    {
        // Return 0 for invalid values
        if (x <= 0)
        {
            FixedUtil::InvalidArgument("Fixed64::LogFast", "x", x);
            return 0;
        }

        // Normalize value to range [1.0, 2.0( as s2.30 and extract exponent.
        static const FP_INT ONE = (1 << 30);
        FP_INT offset = 31 - Nlz((FP_ULONG)x);
        FP_INT n = (FP_INT)(((offset >= 0) ? (x >> offset) : (x << -offset)) >> 2);
        FP_ASSERT(n >= ONE);
        FP_LONG y = (FP_LONG)FixedUtil::LogPoly3Lut8(n - ONE) << 2;

        // Combine integer and fractional parts (into s32.32).
        return (FP_LONG)offset * RCP_LOG2_E + y;
    }

    static FP_LONG LogFastest(FP_LONG x)
    {
        // Return 0 for invalid values
        if (x <= 0)
        {
            FixedUtil::InvalidArgument("Fixed64::LogFastest", "x", x);
            return 0;
        }

        // Normalize value to range [1.0, 2.0( as s2.30 and extract exponent.
        static const FP_INT ONE = (1 << 30);
        FP_INT offset = 31 - Nlz((FP_ULONG)x);
        FP_INT n = (FP_INT)(((offset >= 0) ? (x >> offset) : (x << -offset)) >> 2);
        FP_ASSERT(n >= ONE);
        FP_LONG y = (FP_LONG)FixedUtil::LogPoly5(n - ONE) << 2;

        // Combine integer and fractional parts (into s32.32).
        return (FP_LONG)offset * RCP_LOG2_E + y;
    }

    static FP_LONG Log2(FP_LONG x)
    {
        // Return 0 for invalid values
        if (x <= 0)
        {
            FixedUtil::InvalidArgument("Fixed64::Log2", "x", x);
            return 0;
        }

        // Normalize value to range [1.0, 2.0( as s2.30 and extract exponent.
        FP_INT offset = 31 - Nlz((FP_ULONG)x);
        FP_INT n = (FP_INT)(((offset >= 0) ? (x >> offset) : (x << -offset)) >> 2);

        // Polynomial approximation of mantissa.
        static const FP_INT ONE = (1 << 30);
        FP_ASSERT(n >= ONE);
        FP_LONG y = (FP_LONG)FixedUtil::Log2Poly4Lut16(n - ONE) << 2;

        // Combine integer and fractional parts (into s32.32).
        return ((FP_LONG)offset << Shift) + y;
    }

    static FP_LONG Log2Fast(FP_LONG x)
    {
        // Return 0 for invalid values
        if (x <= 0)
        {
            FixedUtil::InvalidArgument("Fixed64::Log2Fast", "x", x);
            return 0;
        }

        // Normalize value to range [1.0, 2.0( as s2.30 and extract exponent.
        FP_INT offset = 31 - Nlz((FP_ULONG)x);
        FP_INT n = (FP_INT)(((offset >= 0) ? (x >> offset) : (x << -offset)) >> 2);

        // Polynomial approximation of mantissa.
        static const FP_INT ONE = (1 << 30);
        FP_ASSERT(n >= ONE);
        FP_LONG y = (FP_LONG)FixedUtil::Log2Poly3Lut16(n - ONE) << 2;

        // Combine integer and fractional parts (into s32.32).
        return ((FP_LONG)offset << Shift) + y;
    }

    static FP_LONG Log2Fastest(FP_LONG x)
    {
        // Return 0 for invalid values
        if (x <= 0)
        {
            FixedUtil::InvalidArgument("Fixed64::Log2Fastest", "x", x);
            return 0;
        }

        // Normalize value to range [1.0, 2.0( as s2.30 and extract exponent.
        FP_INT offset = 31 - Nlz((FP_ULONG)x);
        FP_INT n = (FP_INT)(((offset >= 0) ? (x >> offset) : (x << -offset)) >> 2);

        // Polynomial approximation of mantissa.
        static const FP_INT ONE = (1 << 30);
        FP_ASSERT(n >= ONE);
        FP_LONG y = (FP_LONG)FixedUtil::Log2Poly5(n - ONE) << 2;

        // Combine integer and fractional parts (into s32.32).
        return ((FP_LONG)offset << Shift) + y;
    }

    /// <summary>
    /// Calculates x to the power of the exponent.
    /// </summary>
    static FP_LONG Pow(FP_LONG x, FP_LONG exponent)
    {
        // Return 0 for invalid values
        if (x <= 0)
        {
            if (x < 0)
                FixedUtil::InvalidArgument("Fixed64::Pow", "x", x);
            return 0;
        }

        return Exp(Mul(exponent, Log(x)));
    }

    /// <summary>
    /// Calculates x to the power of the exponent.
    /// </summary>
    static FP_LONG PowFast(FP_LONG x, FP_LONG exponent)
    {
        // Return 0 for invalid values
        if (x <= 0)
        {
            if (x < 0)
                FixedUtil::InvalidArgument("Fixed64::PowFast", "x", x);
            return 0;
        }

        return ExpFast(Mul(exponent, LogFast(x)));
    }

    /// <summary>
    /// Calculates x to the power of the exponent.
    /// </summary>
    static FP_LONG PowFastest(FP_LONG x, FP_LONG exponent)
    {
        // Return 0 for invalid values
        if (x <= 0)
        {
            if (x < 0)
                FixedUtil::InvalidArgument("Fixed64::PowFastest", "x", x);
            return 0;
        }

        return ExpFastest(Mul(exponent, LogFastest(x)));
    }

    static FP_INT UnitSin(FP_INT z)
    {
        // See: http://www.coranac.com/2009/07/sines/

        // Handle quadrants 1 and 2 by mirroring the [1, 3] range to [-1, 1] (by calculating 2 - z).
        // The if condition uses the fact that for the quadrants of interest are 0b01 and 0b10 (top two bits are different).
        if ((z ^ (z << 1)) < 0)
            z = (1 << 31) - z;

        // Now z is in range [-1, 1].
        static const FP_INT ONE = (1 << 30);
        FP_ASSERT((z >= -ONE) && (z <= ONE));

        // Polynomial approximation.
        FP_INT zz = FixedUtil::Qmul30(z, z);
        FP_INT res = FixedUtil::Qmul30(FixedUtil::SinPoly4(zz), z);

        // Return s2.30 value.
        return res;
    }

    static FP_INT UnitSinFast(FP_INT z)
    {
        // See: http://www.coranac.com/2009/07/sines/

        // Handle quadrants 1 and 2 by mirroring the [1, 3] range to [-1, 1] (by calculating 2 - z).
        // The if condition uses the fact that for the quadrants of interest are 0b01 and 0b10 (top two bits are different).
        if ((z ^ (z << 1)) < 0)
            z = (1 << 31) - z;

        // Now z is in range [-1, 1].
        static const FP_INT ONE = (1 << 30);
        FP_ASSERT((z >= -ONE) && (z <= ONE));

        // Polynomial approximation.
        FP_INT zz = FixedUtil::Qmul30(z, z);
        FP_INT res = FixedUtil::Qmul30(FixedUtil::SinPoly3(zz), z);

        // Return s2.30 value.
        return res;
    }

    static FP_INT UnitSinFastest(FP_INT z)
    {
        // See: http://www.coranac.com/2009/07/sines/

        // Handle quadrants 1 and 2 by mirroring the [1, 3] range to [-1, 1] (by calculating 2 - z).
        // The if condition uses the fact that for the quadrants of interest are 0b01 and 0b10 (top two bits are different).
        if ((z ^ (z << 1)) < 0)
            z = (1 << 31) - z;

        // Now z is in range [-1, 1].
        static const FP_INT ONE = (1 << 30);
        FP_ASSERT((z >= -ONE) && (z <= ONE));

        // Polynomial approximation.
        FP_INT zz = FixedUtil::Qmul30(z, z);
        FP_INT res = FixedUtil::Qmul30(FixedUtil::SinPoly2(zz), z);

        // Return s2.30 value.
        return res;
    }

    static FP_LONG Sin(FP_LONG x)
    {
        // Map [0, 2pi] to [0, 4] (as s2.30).
        // This also wraps the values into one period.
        FP_INT z = MulIntLongLow(RCP_HALF_PI, x);

        // Compute sine and convert to s32.32.
        return (FP_LONG)UnitSin(z) << 2;
    }

    static FP_LONG SinFast(FP_LONG x)
    {
        // Map [0, 2pi] to [0, 4] (as s2.30).
        // This also wraps the values into one period.
        FP_INT z = MulIntLongLow(RCP_HALF_PI, x);

        // Compute sine and convert to s32.32.
        return (FP_LONG)UnitSinFast(z) << 2;
    }

    static FP_LONG SinFastest(FP_LONG x)
    {
        // Map [0, 2pi] to [0, 4] (as s2.30).
        // This also wraps the values into one period.
        FP_INT z = MulIntLongLow(RCP_HALF_PI, x);

        // Compute sine and convert to s32.32.
        return (FP_LONG)UnitSinFastest(z) << 2;
    }

    static FP_LONG Cos(FP_LONG x)
    {
        return Sin(x + PiHalf);
    }

    static FP_LONG CosFast(FP_LONG x)
    {
        return SinFast(x + PiHalf);
    }

    static FP_LONG CosFastest(FP_LONG x)
    {
        return SinFastest(x + PiHalf);
    }

    static FP_LONG Tan(FP_LONG x)
    {
        FP_INT z = MulIntLongLow(RCP_HALF_PI, x);
        FP_LONG sinX = (FP_LONG)UnitSin(z) << 32;
        FP_LONG cosX = (FP_LONG)UnitSin(z + (1 << 30)) << 32;
        return Div(sinX, cosX);
    }

    static FP_LONG TanFast(FP_LONG x)
    {
        FP_INT z = MulIntLongLow(RCP_HALF_PI, x);
        FP_LONG sinX = (FP_LONG)UnitSinFast(z) << 32;
        FP_LONG cosX = (FP_LONG)UnitSinFast(z + (1 << 30)) << 32;
        return DivFast(sinX, cosX);
    }

    static FP_LONG TanFastest(FP_LONG x)
    {
        FP_INT z = MulIntLongLow(RCP_HALF_PI, x);
        FP_LONG sinX = (FP_LONG)UnitSinFastest(z) << 32;
        FP_LONG cosX = (FP_LONG)UnitSinFastest(z + (1 << 30)) << 32;
        return DivFastest(sinX, cosX);
    }

    static FP_INT Atan2Div(FP_LONG y, FP_LONG x)
    {
        FP_ASSERT(y >= 0 && x > 0 && x >= y);

        // Normalize input into [1.0, 2.0( range (convert to s2.30).
        static const FP_INT ONE = (1 << 30);
        static const FP_INT HALF = (1 << 29);
        FP_INT offset = 31 - Nlz((FP_ULONG)x);
        FP_INT n = (FP_INT)(((offset >= 0) ? (x >> offset) : (x << -offset)) >> 2);
        FP_INT k = n - ONE;

        // Polynomial approximation of reciprocal.
        FP_INT oox = FixedUtil::RcpPoly4Lut8(k);
        FP_ASSERT(oox >= HALF && oox <= ONE);

        // Apply exponent and multiply.
        FP_LONG yr = (offset >= 0) ? (y >> offset) : (y << -offset);
        return FixedUtil::Qmul30((FP_INT)(yr >> 2), oox);
    }

    static FP_LONG Atan2(FP_LONG y, FP_LONG x)
    {
        // See: https://www.dsprelated.com/showarticle/1052.php

        if (x == 0)
        {
            if (y > 0) return PiHalf;
            if (y < 0) return -PiHalf;

            FixedUtil::InvalidArgument("Fixed64::Atan2", "y, x", y, x);
            return 0;
        }

        // \note these round negative numbers slightly
        FP_LONG nx = x ^ (x >> 63);
        FP_LONG ny = y ^ (y >> 63);
        FP_LONG negMask = ((x ^ y) >> 63);

        if (nx >= ny)
        {
            FP_INT k = Atan2Div(ny, nx);
            FP_INT z = FixedUtil::AtanPoly5Lut8(k);
            FP_LONG angle = negMask ^ ((FP_LONG)z << 2);
            if (x > 0) return angle;
            if (y >= 0) return angle + Pi;
            return angle - Pi;
        }
        else
        {
            FP_INT k = Atan2Div(nx, ny);
            FP_INT z = FixedUtil::AtanPoly5Lut8(k);
            FP_LONG angle = negMask ^ ((FP_LONG)z << 2);
            return ((y > 0) ? PiHalf : -PiHalf) - angle;
        }
    }

    static FP_INT Atan2DivFast(FP_LONG y, FP_LONG x)
    {
        FP_ASSERT(y >= 0 && x > 0 && x >= y);

        // Normalize input into [1.0, 2.0( range (convert to s2.30).
        static const FP_INT ONE = (1 << 30);
        static const FP_INT HALF = (1 << 29);
        FP_INT offset = 31 - Nlz((FP_ULONG)x);
        FP_INT n = (FP_INT)(((offset >= 0) ? (x >> offset) : (x << -offset)) >> 2);
        FP_INT k = n - ONE;

        // Polynomial approximation.
        FP_INT oox = FixedUtil::RcpPoly6(k);
        FP_ASSERT(oox >= HALF && oox <= ONE);

        // Apply exponent and multiply.
        FP_LONG yr = (offset >= 0) ? (y >> offset) : (y << -offset);
        return FixedUtil::Qmul30((FP_INT)(yr >> 2), oox);
    }

    static FP_LONG Atan2Fast(FP_LONG y, FP_LONG x)
    {
        // See: https://www.dsprelated.com/showarticle/1052.php

        if (x == 0)
        {
            if (y > 0) return PiHalf;
            if (y < 0) return -PiHalf;

            FixedUtil::InvalidArgument("Fixed64::Atan2Fast", "y, x", y, x);
            return 0;
        }

        // \note these round negative numbers slightly
        FP_LONG nx = x ^ (x >> 63);
        FP_LONG ny = y ^ (y >> 63);
        FP_LONG negMask = ((x ^ y) >> 63);

        if (nx >= ny)
        {
            FP_INT k = Atan2DivFast(ny, nx);
            FP_INT z = FixedUtil::AtanPoly3Lut8(k);
            FP_LONG angle = negMask ^ ((FP_LONG)z << 2);
            if (x > 0) return angle;
            if (y >= 0) return angle + Pi;
            return angle - Pi;
        }
        else
        {
            FP_INT k = Atan2DivFast(nx, ny);
            FP_INT z = FixedUtil::AtanPoly3Lut8(k);
            FP_LONG angle = negMask ^ ((FP_LONG)z << 2);
            return ((y > 0) ? PiHalf : -PiHalf) - angle;
        }
    }

    static FP_INT Atan2DivFastest(FP_LONG y, FP_LONG x)
    {
        FP_ASSERT(y >= 0 && x > 0 && x >= y);

        // Normalize input into [1.0, 2.0( range (convert to s2.30).
        static const FP_INT ONE = (1 << 30);
        static const FP_INT HALF = (1 << 29);
        FP_INT offset = 31 - Nlz((FP_ULONG)x);
        FP_INT n = (FP_INT)(((offset >= 0) ? (x >> offset) : (x << -offset)) >> 2);
        FP_INT k = n - ONE;

        // Polynomial approximation.
        FP_INT oox = FixedUtil::RcpPoly4(k);
        FP_ASSERT(oox >= HALF && oox <= ONE);

        // Apply exponent and multiply.
        FP_LONG yr = (offset >= 0) ? (y >> offset) : (y << -offset);
        return FixedUtil::Qmul30((FP_INT)(yr >> 2), oox);
    }

    static FP_LONG Atan2Fastest(FP_LONG y, FP_LONG x)
    {
        // See: https://www.dsprelated.com/showarticle/1052.php

        if (x == 0)
        {
            if (y > 0) return PiHalf;
            if (y < 0) return -PiHalf;

            FixedUtil::InvalidArgument("Fixed64::Atan2Fastest", "y, x", y, x);
            return 0;
        }

        // \note these round negative numbers slightly
        FP_LONG nx = x ^ (x >> 63);
        FP_LONG ny = y ^ (y >> 63);
        FP_LONG negMask = ((x ^ y) >> 63);

        if (nx >= ny)
        {
            FP_INT z = Atan2DivFastest(ny, nx);
            FP_INT res = FixedUtil::AtanPoly4(z);
            FP_LONG angle = negMask ^ ((FP_LONG)res << 2);
            if (x > 0) return angle;
            if (y >= 0) return angle + Pi;
            return angle - Pi;
        }
        else
        {
            FP_INT z = Atan2DivFastest(nx, ny);
            FP_INT res = FixedUtil::AtanPoly4(z);
            FP_LONG angle = negMask ^ ((FP_LONG)res << 2);
            return ((y > 0) ? PiHalf : -PiHalf) - angle;
        }
    }

    static FP_LONG Asin(FP_LONG x)
    {
        // Return 0 for invalid values
        if (x < -One || x > One)
        {
            FixedUtil::InvalidArgument("Fixed64::Asin", "x", x);
            return 0;
        }

        return Atan2(x, Sqrt(Mul(One + x, One - x)));
    }

    static FP_LONG AsinFast(FP_LONG x)
    {
        // Return 0 for invalid values
        if (x < -One || x > One)
        {
            FixedUtil::InvalidArgument("Fixed64::AsinFast", "x", x);
            return 0;
        }

        return Atan2Fast(x, SqrtFast(Mul(One + x, One - x)));
    }

    static FP_LONG AsinFastest(FP_LONG x)
    {
        // Return 0 for invalid values
        if (x < -One || x > One)
        {
            FixedUtil::InvalidArgument("Fixed64::AsinFastest", "x", x);
            return 0;
        }

        return Atan2Fastest(x, SqrtFastest(Mul(One + x, One - x)));
    }

    static FP_LONG Acos(FP_LONG x)
    {
        // Return 0 for invalid values
        if (x < -One || x > One)
        {
            FixedUtil::InvalidArgument("Fixed64::Acos", "x", x);
            return 0;
        }

        return Atan2(Sqrt(Mul(One + x, One - x)), x);
    }

    static FP_LONG AcosFast(FP_LONG x)
    {
        // Return 0 for invalid values
        if (x < -One || x > One)
        {
            FixedUtil::InvalidArgument("Fixed64::AcosFast", "x", x);
            return 0;
        }

        return Atan2Fast(SqrtFast(Mul(One + x, One - x)), x);
    }

    static FP_LONG AcosFastest(FP_LONG x)
    {
        // Return 0 for invalid values
        if (x < -One || x > One)
        {
            FixedUtil::InvalidArgument("Fixed64::AcosFastest", "x", x);
            return 0;
        }

        return Atan2Fastest(SqrtFastest(Mul(One + x, One - x)), x);
    }

    static FP_LONG Atan(FP_LONG x)
    {
        return Atan2(x, One);
    }

    static FP_LONG AtanFast(FP_LONG x)
    {
        return Atan2Fast(x, One);
    }

    static FP_LONG AtanFastest(FP_LONG x)
    {
        return Atan2Fastest(x, One);
    }




    #undef FP_ASSERT
};
#endif // __FIXED64_H

