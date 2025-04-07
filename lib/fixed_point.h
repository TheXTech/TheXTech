/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2025 Vitaly Novichkov <admin@wohlnet.ru>
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
#ifndef XT_FIXED_POINT_H
#define XT_FIXED_POINT_H

// FIXME: define q_t: x int, / 10000 _r

#if ((-1) >> 1) == 0
#error "TheXTech requires floor semantics (mandated by C++20 and higher) for signed right shift."
#endif

#include <cstddef>
#include <cstdint>
#include <stdexcept>

#include "xt_color.h"

struct q_t
{
    const int16_t num;
    const int16_t den;

    constexpr q_t(int16_t num, int16_t den) : num(num), den(den) {}
};

struct qdec_t
{
    int32_t num;

    explicit constexpr qdec_t(int32_t num) : num(num) {}

    qdec_t& operator=(const qdec_t&) = default;
    qdec_t operator-() const
    {
        return qdec_t(-num);
    }
};

struct qbin_t
{
    const int16_t num;

    explicit constexpr qbin_t(int16_t num) : num(num) {}
    qbin_t operator-() const
    {
        return qbin_t(-num);
    }
};

struct qidec_t
{
    const int32_t num;

    explicit constexpr qidec_t(int32_t num) : num(num) {}
    qidec_t operator-() const
    {
        return qidec_t(-num);
    }
};

struct qibin_t
{
    const int16_t den;

    explicit constexpr qibin_t(int16_t den) : den(den) {}
    qibin_t operator-() const
    {
        return qibin_t(-den);
    }
};

// s8.24 fixed point
struct numf_t
{
public:
    int32_t i;

    numf_t() {}
    constexpr numf_t(int _i) : i((int32_t)_i << 24) {}
    explicit constexpr numf_t(int32_t _i, nullptr_t) : i(_i) {}
    explicit constexpr numf_t(double _i, nullptr_t) : i((int32_t)(_i * (1 << 24))) {}

    explicit constexpr operator bool() const
    {
        return (bool)i;
    }

    explicit constexpr operator unsigned char() const
    {
        return i / (1 << 24);
    }

    explicit constexpr operator unsigned short() const
    {
        return i / (1 << 24);
    }

    explicit constexpr operator int32_t() const
    {
        return i / (1 << 24);
    }

    explicit constexpr operator double() const
    {
        return (double)(i) / (1 << 24);
    }

    constexpr numf_t operator-() const
    {
        return numf_t(-i, nullptr);
    }

    constexpr numf_t operator*(int o) const
    {
        return numf_t(i * o, nullptr);
    }

    constexpr numf_t operator/(int o) const
    {
        return numf_t(i / o, nullptr);
    }

    numf_t& operator+=(numf_t o)
    {
        *this = *this + o;
        return *this;
    }

    numf_t& operator-=(numf_t o)
    {
        *this = *this - o;
        return *this;
    }

    constexpr numf_t operator+(numf_t o) const
    {
        return numf_t(i + o.i, nullptr);
    }

    constexpr numf_t operator-(numf_t o) const
    {
        return numf_t(i - o.i, nullptr);
    }

    numf_t times(numf_t o) const;
    numf_t divided_by(numf_t o) const;

    constexpr bool operator<=(const numf_t& o) const { return i <= o.i; }
    constexpr bool operator<(const numf_t& o) const { return i < o.i; }
    constexpr bool operator>=(const numf_t& o) const { return i >= o.i; }
    constexpr bool operator>(const numf_t& o) const { return i > o.i; }
    constexpr bool operator==(const numf_t& o) const { return i == o.i; }
    constexpr bool operator!=(const numf_t& o) const { return i != o.i; }
};

// s32.32 fixed point
struct num_t
{
public:
    int64_t i;

    num_t() {}
    constexpr num_t(const num_t& o) : i(o.i) {}
    constexpr num_t(short _i) : i((int64_t)_i << 32) {}
    constexpr num_t(int _i) : i((int64_t)_i << 32) {}
    constexpr num_t(long _i) : i((int64_t)_i << 32) {}
    constexpr num_t(long long _i) : i((int64_t)_i << 32) {}
    constexpr num_t(unsigned short _i) : i((int64_t)_i << 32) {}
    constexpr num_t(unsigned int _i) : i((int64_t)_i << 32) {}
    constexpr num_t(unsigned long _i) : i((int64_t)_i << 32) {}
    constexpr num_t(unsigned long long _i) : i((int64_t)_i << 32) {}
    constexpr num_t(double _i) = delete;

    explicit constexpr num_t(int64_t _i, nullptr_t) : i(_i) {}
    explicit constexpr num_t(double _i, nullptr_t) : i((int64_t)(_i * ((int64_t)1 << 32))) {}

    explicit constexpr num_t(numf_t _i) : i(int64_t(_i.i) * 256) {}

    num_t& operator=(const num_t&) = default;

    num_t& operator++()
    {
        i += ((int64_t)1 << 32);
        return *this;
    }

    num_t operator++(int)
    {
        num_t ret = *this;
        i += ((int64_t)1 << 32);
        return ret;
    }

    num_t& operator+=(num_t o)
    {
        *this = *this + o;
        return *this;
    }

    num_t& operator-=(num_t o)
    {
        *this = *this - o;
        return *this;
    }

    // num_t& operator*=(num_t o)
    // {
    //     *this = *this * o;
    //     return *this;
    // }

    num_t& operator*=(int o)
    {
        i *= o;
        return *this;
    }

    num_t& operator/=(int o)
    {
        i /= o;
        return *this;
    }

    constexpr num_t operator+(num_t o) const
    {
        return num_t(i + o.i, nullptr);
    }

    constexpr num_t operator-(num_t o) const
    {
        return num_t(i - o.i, nullptr);
    }

    constexpr num_t operator-() const
    {
        return num_t(-i, nullptr);
    }

    num_t& operator*=(q_t o)
    {
        *this = *this * o;
        return *this;
    }

    num_t& operator*=(qdec_t o)
    {
        *this = *this * o;
        return *this;
    }

    num_t& operator*=(qbin_t o)
    {
        *this = *this * o;
        return *this;
    }

    num_t& operator/=(qidec_t o)
    {
        *this = *this / o;
        return *this;
    }

    num_t& operator/=(qibin_t o)
    {
        *this = *this / o;
        return *this;
    }

    constexpr num_t operator*(q_t o) const
    {
        return num_t(i * o.num / o.den, nullptr);
    }

    constexpr num_t operator/(q_t o) const
    {
        return num_t(i * o.den / o.num, nullptr);
    }

    constexpr num_t operator*(qdec_t o) const
    {
        return num_t(i * o.num / 10000, nullptr);
    }

    constexpr num_t operator/(qidec_t o) const
    {
        return num_t(i * 10000 / o.num, nullptr);
    }

    constexpr num_t operator*(qbin_t o) const
    {
        return num_t(i * o.num / 256, nullptr);
    }

    constexpr num_t operator/(qibin_t o) const
    {
        return num_t(i * o.den / 256, nullptr);
    }

    explicit constexpr operator unsigned char() const
    {
        return i / ((int64_t)1 << 32);
    }

    explicit constexpr operator unsigned short() const
    {
        return i / ((int64_t)1 << 32);
    }

    explicit constexpr operator short() const
    {
        return i / ((int64_t)1 << 32);
    }

    explicit constexpr operator int() const
    {
        return i / ((int64_t)1 << 32);
    }

    explicit constexpr operator long() const
    {
        return i / ((int64_t)1 << 32);
    }

    explicit constexpr operator long long() const
    {
        return i / ((int64_t)1 << 32);
    }

    explicit constexpr operator size_t() const
    {
        return i / ((int64_t)1 << 32);
    }

    explicit constexpr operator double() const
    {
        return (double)(i) / ((int64_t)1 << 32);
    }

    explicit constexpr operator numf_t() const
    {
        // banker's rounding, assume no overflow
        return numf_t((int32_t)(((!(i & 0x17f) && (i & 0x80)) ? i : (i + 0x80)) >> 8), nullptr);
    }

    static inline constexpr num_t from_double(double _i)
    {
        return num_t(_i, nullptr);
    }

    static inline constexpr num_t abs(num_t x)
    {
        return num_t(x.i > 0 ? x.i : -x.i, nullptr);
    }

    static inline constexpr int32_t vb6round(num_t x)
    {
        // first test: does x % 1 == 0.5? second test: does (x + 0.5) % 2 == 1? if both, then subtract 0.5, otherwise add 0.5 and floor
        return ((((x.i + ((int64_t)1 << 31)) & 0xFFFFFFFFLL) == 0) && (((x.i + ((int64_t)1 << 31)) & 0x100000000LL) != 0))
            ? ((x.i - ((int64_t)1 << 31)) >> 32)
            : ((x.i + ((int64_t)1 << 31)) >> 32);
    }

    static inline constexpr int32_t round(num_t x)
    {
        return ((x.i >= 0 ? x.i + ((int64_t)1 << 31) : x.i - ((int64_t)1 << 31)) / ((int64_t)1 << 32));
    }

    static inline constexpr int32_t ceil(num_t x)
    {
        return (x.i + (((int64_t)1 << 32) - 1)) >> 32;
    }

    static inline constexpr int32_t floor(num_t x)
    {
        return (x.i) >> 32;
    }

    static inline constexpr num_t roundn(num_t x)
    {
        return num_t::round(x);
    }

    static inline num_t dist2(num_t dx, num_t dy)
    {
        // FIXME
        return dx.times(dx) + dy.times(dy);
    }

    static inline bool fEqual_f(numf_t x, numf_t y)
    {
        return x == y;
    }

    static inline bool fEqual_f(num_t x, num_t y)
    {
        return x == y;
    }

    static inline bool fEqual_d(num_t x, num_t y)
    {
        return x == y;
    }

    num_t times(num_t o) const;
    num_t divided_by(num_t o) const;

    static num_t dist(num_t dx, num_t dy);
    static num_t idist(num_t dx, num_t dy);

    static num_t cos(num_t dx);
    static num_t sin(num_t dx);
    static num_t atan2(num_t y, num_t x);

    explicit constexpr operator bool() const
    {
        return (bool)i;
    }

    constexpr bool operator<=(const num_t& o) const { return i <= o.i; }
    constexpr bool operator<(const num_t& o) const { return i < o.i; }
    constexpr bool operator>=(const num_t& o) const { return i >= o.i; }
    constexpr bool operator>(const num_t& o) const { return i > o.i; }
    constexpr bool operator==(const num_t& o) const { return i == o.i; }
    constexpr bool operator!=(const num_t& o) const { return i != o.i; }
};

static inline constexpr num_t operator ""_n(long double d)
{
    return num_t((double)d, nullptr);
}

static inline constexpr numf_t operator ""_nf(long double d)
{
    return numf_t((double)d, nullptr);
}

static inline constexpr num_t operator ""_n(unsigned long long i)
{
    return (num_t)(int64_t)i;
}

static inline constexpr numf_t operator ""_nf(unsigned long long i)
{
    return (numf_t)(int)i;
}

#ifdef __cpp_consteval
#define maybe_consteval consteval
#else
#define maybe_consteval constexpr
#endif

static inline maybe_consteval qdec_t operator ""_r(long double d)
{
    return
#ifdef __cpp_consteval
        (d == (int)(d * 256) / 256.0l) ? (throw std::logic_error("_r should not be used with multiples of 1/256")) :
        (1 / d == (int)(1 / d)) ? (throw std::logic_error("_r should not be used with inverses of whole numbers")) :
        (d != (int)(d * 10000 + ((d > 0) ? 0.000001 : -0.000001)) / 10000.0l) ? (throw std::logic_error("_r may only be used with multiples of 0.0001")) :
#endif
        qdec_t((int)(d * 10000 + ((d > 0) ? 0.000001 : -0.000001)));
}

static inline maybe_consteval qbin_t operator ""_rb(long double d)
{
    return
#ifdef __cpp_consteval
        (d != (int)(d * 256) / 256.0l) ? (throw std::logic_error("_rb may only be used with multiples of 1/256")) :
        (d >= 128.0 || d <= -128.0) ? (throw std::logic_error("_rb may only be used with numbers between -128 and 128")) :
#endif
        qbin_t((int)(d * 256));
}

static inline maybe_consteval qidec_t operator ""_ri(long double d)
{
    return
#ifdef __cpp_consteval
        ((1.0 / d) == (int)((1.0 / d) * 256) / 256.0l) ? (throw std::logic_error("_ri should not be used with divisors of 256")) :
        (d != (int)(d * 10000 + ((d > 0) ? 0.000001 : -0.000001)) / 10000.0l) ? (throw std::logic_error("_ri may only be used with multiples of 0.0001")) :
#endif
        qidec_t((int)(d * 10000 + ((d > 0) ? 0.000001 : -0.000001)));
}

static inline maybe_consteval qibin_t operator ""_rib(long double d)
{
    return
#ifdef __cpp_consteval
        ((1.0 / d) != (int)((1.0 / d) * 256) / 256.0l) ? (throw std::logic_error("_rib may only be used with divisors of 256")) :
        ((1.0 / d) >= 128.0 || (1.0 / d) <= -128.0) ? (throw std::logic_error("_rib may only be used with numbers with magnitude greater than 1/128")) :
#endif
        qibin_t((int)((1.0 / d) * 256));
}

static inline constexpr num_t operator +(short a, num_t b) { return (num_t)a + b; }
static inline constexpr num_t operator +(int a, num_t b) { return (num_t)a + b; }
static inline constexpr num_t operator +(long a, num_t b) { return (num_t)a + b; }
static inline constexpr num_t operator +(long long a, num_t b) { return (num_t)a + b; }

static inline constexpr num_t operator +(unsigned short a, num_t b) { return (num_t)a + b; }
static inline constexpr num_t operator +(unsigned int a, num_t b) { return (num_t)a + b; }
static inline constexpr num_t operator +(unsigned long a, num_t b) { return (num_t)a + b; }
static inline constexpr num_t operator +(unsigned long long a, num_t b) { return (num_t)a + b; }

static inline constexpr num_t operator -(short a, num_t b) { return (num_t)a - b; }
static inline constexpr num_t operator -(int a, num_t b) { return (num_t)a - b; }
static inline constexpr num_t operator -(long a, num_t b) { return (num_t)a - b; }
static inline constexpr num_t operator -(long long a, num_t b) { return (num_t)a - b; }

static inline constexpr num_t operator -(unsigned short a, num_t b) { return (num_t)a - b; }
static inline constexpr num_t operator -(unsigned int a, num_t b) { return (num_t)a - b; }
static inline constexpr num_t operator -(unsigned long a, num_t b) { return (num_t)a - b; }
static inline constexpr num_t operator -(unsigned long long a, num_t b) { return (num_t)a - b; }

static inline constexpr num_t operator *(short a, num_t b) { return num_t(b.i * a, nullptr); }
static inline constexpr num_t operator *(int a, num_t b) { return num_t(b.i * a, nullptr); }
static inline constexpr num_t operator *(long a, num_t b) { return num_t(b.i * a, nullptr); }
static inline constexpr num_t operator *(long long a, num_t b) { return num_t((int64_t)(b.i * a), nullptr); }

static inline constexpr num_t operator *(unsigned short a, num_t b) { return num_t(b.i * a, nullptr); }
static inline constexpr num_t operator *(unsigned int a, num_t b) { return num_t(b.i * a, nullptr); }
static inline constexpr num_t operator *(unsigned long a, num_t b) { return num_t((int64_t)(b.i * a), nullptr); }
static inline constexpr num_t operator *(unsigned long long a, num_t b) { return num_t((int64_t)(b.i * a), nullptr); }

static inline constexpr num_t operator *(num_t b, short a) { return num_t(b.i * a, nullptr); }
static inline constexpr num_t operator *(num_t b, int a) { return num_t(b.i * a, nullptr); }
static inline constexpr num_t operator *(num_t b, long a) { return num_t(b.i * a, nullptr); }
static inline constexpr num_t operator *(num_t b, long long a) { return num_t((int64_t)(b.i * a), nullptr); }

static inline constexpr num_t operator *(num_t b, unsigned short a) { return num_t(b.i * a, nullptr); }
static inline constexpr num_t operator *(num_t b, unsigned int a) { return num_t(b.i * a, nullptr); }
static inline constexpr num_t operator *(num_t b, unsigned long a) { return num_t((int64_t)(b.i * a), nullptr); }
static inline constexpr num_t operator *(num_t b, unsigned long long a) { return num_t((int64_t)(b.i * a), nullptr); }

static inline constexpr num_t operator *(qdec_t a, num_t b) { return b * a; }
static inline constexpr num_t operator *(qbin_t a, num_t b) { return b * a; }

static inline constexpr num_t operator /(num_t a, short b) { return num_t(a.i / b, nullptr); }
static inline constexpr num_t operator /(num_t a, int b) { return num_t(a.i / b, nullptr); }
static inline constexpr num_t operator /(num_t a, long b) { return num_t(a.i / b, nullptr); }
static inline constexpr num_t operator /(num_t a, long long b) { return num_t((int64_t)(a.i / b), nullptr); }

static inline constexpr num_t operator /(num_t a, unsigned short b) { return num_t(a.i / b, nullptr); }
static inline constexpr num_t operator /(num_t a, unsigned int b) { return num_t(a.i / b, nullptr); }
static inline constexpr num_t operator /(num_t a, unsigned long b) { return num_t((int64_t)(a.i / b), nullptr); }
static inline constexpr num_t operator /(num_t a, unsigned long long b) { return num_t((int64_t)(a.i / b), nullptr); }

// comparison operators

static inline constexpr num_t operator <(short a, num_t b) { return (num_t)a < b; }
static inline constexpr num_t operator <(int a, num_t b) { return (num_t)a < b; }
static inline constexpr num_t operator <(long a, num_t b) { return (num_t)a < b; }
static inline constexpr num_t operator <(long long a, num_t b) { return (num_t)a < b; }

static inline constexpr num_t operator <(unsigned short a, num_t b) { return (num_t)a < b; }
static inline constexpr num_t operator <(unsigned int a, num_t b) { return (num_t)a < b; }
static inline constexpr num_t operator <(unsigned long a, num_t b) { return (num_t)a < b; }
static inline constexpr num_t operator <(unsigned long long a, num_t b) { return (num_t)a < b; }

static inline constexpr num_t operator <=(short a, num_t b) { return (num_t)a <= b; }
static inline constexpr num_t operator <=(int a, num_t b) { return (num_t)a <= b; }
static inline constexpr num_t operator <=(long a, num_t b) { return (num_t)a <= b; }
static inline constexpr num_t operator <=(long long a, num_t b) { return (num_t)a <= b; }

static inline constexpr num_t operator <=(unsigned short a, num_t b) { return (num_t)a <= b; }
static inline constexpr num_t operator <=(unsigned int a, num_t b) { return (num_t)a <= b; }
static inline constexpr num_t operator <=(unsigned long a, num_t b) { return (num_t)a <= b; }
static inline constexpr num_t operator <=(unsigned long long a, num_t b) { return (num_t)a <= b; }

static inline constexpr num_t operator >(short a, num_t b) { return (num_t)a > b; }
static inline constexpr num_t operator >(int a, num_t b) { return (num_t)a > b; }
static inline constexpr num_t operator >(long a, num_t b) { return (num_t)a > b; }
static inline constexpr num_t operator >(long long a, num_t b) { return (num_t)a > b; }

static inline constexpr num_t operator >(unsigned short a, num_t b) { return (num_t)a > b; }
static inline constexpr num_t operator >(unsigned int a, num_t b) { return (num_t)a > b; }
static inline constexpr num_t operator >(unsigned long a, num_t b) { return (num_t)a > b; }
static inline constexpr num_t operator >(unsigned long long a, num_t b) { return (num_t)a > b; }

static inline constexpr num_t operator >=(short a, num_t b) { return (num_t)a >= b; }
static inline constexpr num_t operator >=(int a, num_t b) { return (num_t)a >= b; }
static inline constexpr num_t operator >=(long a, num_t b) { return (num_t)a >= b; }
static inline constexpr num_t operator >=(long long a, num_t b) { return (num_t)a >= b; }

static inline constexpr num_t operator >=(unsigned short a, num_t b) { return (num_t)a >= b; }
static inline constexpr num_t operator >=(unsigned int a, num_t b) { return (num_t)a >= b; }
static inline constexpr num_t operator >=(unsigned long a, num_t b) { return (num_t)a >= b; }
static inline constexpr num_t operator >=(unsigned long long a, num_t b) { return (num_t)a >= b; }

static inline constexpr num_t operator ==(short a, num_t b) { return (num_t)a == b; }
static inline constexpr num_t operator ==(int a, num_t b) { return (num_t)a == b; }
static inline constexpr num_t operator ==(long a, num_t b) { return (num_t)a == b; }
static inline constexpr num_t operator ==(long long a, num_t b) { return (num_t)a == b; }

static inline constexpr num_t operator ==(unsigned short a, num_t b) { return (num_t)a == b; }
static inline constexpr num_t operator ==(unsigned int a, num_t b) { return (num_t)a == b; }
static inline constexpr num_t operator ==(unsigned long a, num_t b) { return (num_t)a == b; }
static inline constexpr num_t operator ==(unsigned long long a, num_t b) { return (num_t)a == b; }

static inline constexpr num_t operator !=(short a, num_t b) { return (num_t)a != b; }
static inline constexpr num_t operator !=(int a, num_t b) { return (num_t)a != b; }
static inline constexpr num_t operator !=(long a, num_t b) { return (num_t)a != b; }
static inline constexpr num_t operator !=(long long a, num_t b) { return (num_t)a != b; }

static inline constexpr num_t operator !=(unsigned short a, num_t b) { return (num_t)a != b; }
static inline constexpr num_t operator !=(unsigned int a, num_t b) { return (num_t)a != b; }
static inline constexpr num_t operator !=(unsigned long a, num_t b) { return (num_t)a != b; }
static inline constexpr num_t operator !=(unsigned long long a, num_t b) { return (num_t)a != b; }

// this operator should be used to indicate that a num_t contains an int and may be treated as an int if needed
using int_ok = int;

// this is used for temporary values that were Singles in VB6
using tempf_t = num_t;

static inline constexpr XTColor XTAlphaF(num_t a)
{
    return XTColor().with_alpha(static_cast<uint8_t>(a.i >> 24));
}

#endif // #ifndef XT_FIXED_POINT_H
