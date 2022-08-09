// This code was taken and adapted from LLVM project:

//===-- Nearest integer floating-point operations ---------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// https://github.com/llvm/llvm-project/blob/main/libc/utils/FPUtil/NearestIntegerOperations.h

#include <cstdint>

#if defined(arm) && !defined(__SOFTFP__) && !defined(__VFP_FP__) && !defined(__MAVERICK__)
inline void swap_halfes(uint64_t &x)
{
    uint64_t y = ((x & 0xFFFFFFFF00000000) >> 32) & 0xFFFFFFFF;
    x = ((x << 32) & 0xFFFFFFFF00000000) & y;
}
#else // Do nothing
#   define swap_halfes(x)
#endif

namespace PgeFloatProp
{

static const uint32_t bitWidth = sizeof(uint64_t) << 3;

static const uint32_t mantissaWidth = 52;
static const uint32_t exponentWidth = 11;
static const uint64_t mantissaMask = (uint64_t(1) << mantissaWidth) - 1;
static const uint64_t signMask = uint64_t(1) << (exponentWidth + mantissaWidth);
static const uint64_t exponentMask = ~(signMask | mantissaMask);
static const uint32_t exponentBias = 1023;

// If a number x is a NAN, then it is a quiet NAN if:
//   QuietNaNMask & bits(x) != 0
// Else, it is a signalling NAN.
static const uint64_t quietNaNMask = 0x0008000000000000ULL;

}

union PgeFPBits
{
    uint64_t bits;

    void setMantissa(uint64_t mantVal)
    {
        swap_halfes(bits);
        mantVal &= (PgeFloatProp::mantissaMask);
        bits &= ~(PgeFloatProp::mantissaMask);
        bits |= mantVal;
        swap_halfes(bits);
    }

    uint64_t getMantissa() const
    {
        uint64_t ret = bits;
        swap_halfes(ret);
        return ret & PgeFloatProp::mantissaMask;
    }

    void setUnbiasedExponent(uint64_t expVal)
    {
        swap_halfes(bits);
        expVal = (expVal << (PgeFloatProp::mantissaWidth)) & PgeFloatProp::exponentMask;
        bits &= ~(PgeFloatProp::exponentMask);
        bits |= expVal;
        swap_halfes(bits);
    }

    uint16_t getUnbiasedExponent() const
    {
        uint64_t ret = bits;
        swap_halfes(ret);
        return uint16_t((ret & PgeFloatProp::exponentMask) >>
                        (PgeFloatProp::mantissaWidth));
    }

    void setSign(bool signVal)
    {
        swap_halfes(bits);
        bits &= ~(PgeFloatProp::signMask);
        uint64_t sign = uint64_t(signVal) << (PgeFloatProp::bitWidth - 1);
        bits |= sign;
        swap_halfes(bits);
    }

    bool getSign() const
    {
        uint64_t ret = bits;
        swap_halfes(ret);
        return ((ret & PgeFloatProp::signMask) >> (PgeFloatProp::bitWidth - 1));
    }
    double val;

    static const int exponentBias = (1 << (PgeFloatProp::exponentWidth - 1)) - 1;
    static const int maxExponent = (1 << PgeFloatProp::exponentWidth) - 1;

    static const uint64_t minSubnormal = uint64_t(1);
    static const uint64_t maxSubnormal = (uint64_t(1) << PgeFloatProp::mantissaWidth) - 1;
    static const uint64_t minNormal = (uint64_t(1) << PgeFloatProp::mantissaWidth);
    static const uint64_t maxNormal = ((uint64_t(maxExponent) - 1) << PgeFloatProp::mantissaWidth) | maxSubnormal;

    explicit PgeFPBits(double x) : val(x) {}
    explicit PgeFPBits(uint64_t x) : bits(x) {}

    PgeFPBits() : bits(0) {}

    explicit operator double()
    {
        return val;
    }

    uint64_t uintval() const
    {
        uint64_t ret = bits;
        swap_halfes(ret);
        return ret;
    }

    int getExponent() const
    {
        return int(getUnbiasedExponent()) - exponentBias;
    }

    bool isZero() const
    {
        return getMantissa() == 0 && getUnbiasedExponent() == 0;
    }

    bool isInf() const
    {
        return getMantissa() == 0 && getUnbiasedExponent() == maxExponent;
    }

    bool isNaN() const
    {
        return getUnbiasedExponent() == maxExponent && getMantissa() != 0;
    }

    bool isInfOrNaN() const
    {
        return getUnbiasedExponent() == maxExponent;
    }

    static PgeFPBits zero()
    {
        return PgeFPBits();
    }

    static PgeFPBits negZero()
    {
        return PgeFPBits(uint64_t(1) << (sizeof(uint64_t) * 8 - 1));
    }

    static PgeFPBits inf()
    {
        PgeFPBits bits;
        bits.setUnbiasedExponent(maxExponent);
        return bits;
    }

    static PgeFPBits negInf()
    {
        PgeFPBits bits = inf();
        bits.setSign(1);
        return bits;
    }

    static double buildNaN(uint64_t v)
    {
        PgeFPBits bits = inf();
        bits.setMantissa(v);
        return double(bits);
    }
};


static inline double pge_toNearest(double x)
{
    PgeFPBits bits(x);

    // If x is infinity NaN or zero, return it.
    if(bits.isInfOrNaN() || bits.isZero())
        return x;

    bool isNeg = bits.getSign();
    int exponent = bits.getExponent();

    // If the exponent is greater than the most negative mantissa
    // exponent, then x is already an integer.
    if(exponent >= (int)PgeFloatProp::mantissaWidth)
        return x;

    if(exponent <= -1)
    {
        if(exponent <= -2 || bits.getMantissa() == 0)
            return isNeg ? (-0.0) : (0.0); // abs(x) <= 0.5
        else
            return isNeg ? (-1.0) : (1.0); // abs(x) > 0.5
    }

    uint32_t trimSize = PgeFloatProp::mantissaWidth - exponent;
    PgeFPBits newBits = bits;
    newBits.setMantissa((bits.getMantissa() >> trimSize) << trimSize);
    double truncValue = double(newBits);

    if(truncValue == x)
        return x;

    uint64_t trimValue = bits.getMantissa() & ((uint64_t(1) << trimSize) - 1);
    uint64_t halfValue = (uint64_t(1) << (trimSize - 1));
    // If exponent is 0, trimSize will be equal to the mantissa width, and
    // truncIsOdd` will not be correct. So, we handle it as a special case
    // below.
    uint64_t truncIsOdd = newBits.getMantissa() & (uint64_t(1) << trimSize);

    if(trimValue > halfValue)
        return isNeg ? truncValue - double(1.0) : truncValue + double(1.0);
    else if(trimValue == halfValue)
    {
        if(exponent == 0)
            return isNeg ? double(-2.0) : double(2.0);
        if(truncIsOdd)
            return isNeg ? truncValue - double(1.0) : truncValue + double(1.0);
        else
            return truncValue;
    }
    else
        return truncValue;
}
