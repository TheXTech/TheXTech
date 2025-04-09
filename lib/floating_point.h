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
#ifndef XT_FLOATING_POINT_H
#define XT_FLOATING_POINT_H

#include <stdint.h>
#include <stddef.h>
#include <math.h>

struct num_t;

struct num_muldiv_t
{
public:
    double i;

    explicit constexpr num_muldiv_t(long double _i) : i(_i) {}
    explicit constexpr num_muldiv_t(num_t n);

    constexpr num_muldiv_t operator-() const
    {
        return num_muldiv_t(-i);
    }
};

using qdec_t = num_muldiv_t;
using qbin_t = num_muldiv_t;
using qidec_t = num_muldiv_t;
using qibin_t = num_muldiv_t;

struct numf_t
{
public:
    float i;

    numf_t() {}
    constexpr numf_t(int _i) : i(_i) {}
    constexpr numf_t(long _i) : i(_i) {}
    constexpr numf_t(long long _i) : i(_i) {}
    constexpr numf_t(float _i) = delete;

    explicit constexpr numf_t(float _i, nullptr_t) : i(_i) {}

    numf_t& operator=(const numf_t&) = default;

    constexpr bool operator<=(const numf_t& o) const { return i <= o.i; }
    constexpr bool operator<(const numf_t& o) const { return i < o.i; }
    constexpr bool operator>=(const numf_t& o) const { return i >= o.i; }
    constexpr bool operator>(const numf_t& o) const { return i > o.i; }
    constexpr bool operator==(const numf_t& o) const { return i == o.i; }
    constexpr bool operator!=(const numf_t& o) const { return i != o.i; }

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

    numf_t& operator*=(int o)
    {
        i *= o;
        return *this;
    }

    numf_t& operator/=(int o)
    {
        i /= o;
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

    explicit constexpr operator unsigned char() const
    {
        return i;
    }

    explicit constexpr operator unsigned short() const
    {
        return i;
    }

    explicit constexpr operator short() const
    {
        return i;
    }

    explicit constexpr operator int() const
    {
        return i;
    }

    explicit constexpr operator long() const
    {
        return i;
    }

    explicit constexpr operator long long() const
    {
        return i;
    }

    explicit constexpr operator double() const
    {
        return i;
    }

    explicit constexpr operator bool() const
    {
        return (bool)i;
    }

    numf_t times(numf_t o) const
    {
        return numf_t(i * o.i, nullptr);
    }

    numf_t divided_by(numf_t o) const
    {
        return numf_t(i / o.i, nullptr);
    }
};

struct num_t
{
public:
    double i;

    num_t() {}
    constexpr num_t(const num_t& o) : i(o.i) {}
    constexpr num_t(short _i) : i(_i) {}
    constexpr num_t(int _i) : i(_i) {}
    constexpr num_t(long _i) : i(_i) {}
    constexpr num_t(long long _i) : i(_i) {}
    constexpr num_t(unsigned short _i) : i(_i) {}
    constexpr num_t(unsigned int _i) : i(_i) {}
    constexpr num_t(unsigned long _i) : i(_i) {}
    constexpr num_t(unsigned long long _i) : i(_i) {}
    constexpr num_t(double _i) = delete;

    constexpr num_t(int64_t _i, nullptr_t) : i(_i) {}
    explicit constexpr num_t(long double _i, nullptr_t) : i(_i) {}
    explicit constexpr num_t(double _i, nullptr_t) : i(_i) {}

    explicit constexpr num_t(numf_t _i) : i(_i.i) {}

    num_t& operator=(const num_t&) = default;

    num_t& operator++()
    {
        i += 1;
        return *this;
    }

    num_t operator++(int)
    {
        num_t ret = *this;
        i += 1;
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

    num_t& operator*=(num_muldiv_t o)
    {
        *this = *this * o;
        return *this;
    }

    num_t& operator/=(num_muldiv_t o)
    {
        *this = *this / o;
        return *this;
    }

    constexpr num_t operator*(num_muldiv_t o) const
    {
        return num_t(i * o.i, nullptr);
    }

    constexpr num_t operator/(num_muldiv_t o) const
    {
        return num_t(i / o.i, nullptr);
    }

    explicit constexpr operator unsigned char() const
    {
        return i;
    }

    explicit constexpr operator unsigned short() const
    {
        return i;
    }

    explicit constexpr operator short() const
    {
        return i;
    }

    explicit constexpr operator int() const
    {
        return i;
    }

    explicit constexpr operator long() const
    {
        return i;
    }

    explicit constexpr operator long long() const
    {
        return i;
    }

    explicit constexpr operator numf_t() const
    {
        return numf_t(i, nullptr);
    }

    explicit constexpr operator double() const
    {
        return i;
    }

    explicit constexpr operator size_t() const
    {
        return i;
    }

    static inline constexpr num_t from_double(double _i)
    {
        return num_t(_i, nullptr);
    }

    static inline constexpr num_t abs(num_t x)
    {
        return num_t(x.i > 0 ? x.i : -x.i, nullptr);
    }

    static inline constexpr int32_t round(num_t x)
    {
        return std::round(x.i);
    }

    static inline constexpr int32_t ceil(num_t x)
    {
        return std::ceil(x.i);
    }

    static inline constexpr int32_t floor(num_t x)
    {
        return std::floor(x.i);
    }

    static inline constexpr num_t roundn(num_t x)
    {
        return num_t(std::round(x.i), nullptr);
    }

    static inline constexpr num_t PI()
    {
        return num_t(3.14159265358979323846264338327950288, nullptr);
    }

    static inline num_t dist2(num_t dx, num_t dy)
    {
        return dx.times(dx) + dy.times(dy);
    }

    static inline bool fEqual_d(num_t x, num_t y)
    {
        return (int64_t)std::round(x.i * 1000000) == (int64_t)std::round(y.i * 1000000);
    }

    static inline bool fEqual_f(num_t x, num_t y)
    {
        return (int64_t)std::round(x.i * 10000) == (int64_t)std::round(y.i * 10000);
    }

    static inline bool fEqual_f(numf_t x, numf_t y)
    {
        return (int64_t)std::round(x.i * 10000) == (int64_t)std::round(y.i * 10000);
    }

    num_t times(num_t o) const;
    num_t divided_by(num_t o) const;

    static int32_t vb6round(num_t x);

    static num_t dist(num_t dx, num_t dy);
    static num_t idist(num_t dx, num_t dy);

    static num_t sqrt(num_t dx);

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

constexpr num_muldiv_t::num_muldiv_t(num_t n) : i(n.i) {}


static inline constexpr num_t operator ""_n(long double d)
{
    return num_t(d, nullptr);
}

static inline constexpr numf_t operator ""_nf(long double d)
{
    return numf_t(d, nullptr);
}

static inline constexpr num_t operator ""_n(unsigned long long i)
{
    return (num_t)(int64_t)i;
}

static inline constexpr numf_t operator ""_nf(unsigned long long i)
{
    return (numf_t)(int)i;
}

static inline constexpr num_muldiv_t operator ""_r(long double d)
{
    return num_muldiv_t(d);
}

static inline constexpr qbin_t operator ""_rb(long double d)
{
    return num_muldiv_t(d);
}

static inline constexpr qidec_t operator ""_ri(long double d)
{
    return num_muldiv_t(d);
}

static inline constexpr qibin_t operator ""_rib(long double d)
{
    return num_muldiv_t(d);
}

static inline constexpr numf_t operator *(int a, numf_t b) { return numf_t(b.i * a, nullptr); }

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
static inline constexpr num_t operator *(long long a, num_t b) { return num_t(b.i * a, nullptr); }

static inline constexpr num_t operator *(unsigned short a, num_t b) { return num_t(b.i * a, nullptr); }
static inline constexpr num_t operator *(unsigned int a, num_t b) { return num_t(b.i * a, nullptr); }
static inline constexpr num_t operator *(unsigned long a, num_t b) { return num_t(b.i * a, nullptr); }
static inline constexpr num_t operator *(unsigned long long a, num_t b) { return num_t(b.i * a, nullptr); }

static inline constexpr num_t operator *(num_t b, short a) { return num_t(b.i * a, nullptr); }
static inline constexpr num_t operator *(num_t b, int a) { return num_t(b.i * a, nullptr); }
static inline constexpr num_t operator *(num_t b, long a) { return num_t(b.i * a, nullptr); }
static inline constexpr num_t operator *(num_t b, long long a) { return num_t(b.i * a, nullptr); }

static inline constexpr num_t operator *(num_t b, unsigned short a) { return num_t(b.i * a, nullptr); }
static inline constexpr num_t operator *(num_t b, unsigned int a) { return num_t(b.i * a, nullptr); }
static inline constexpr num_t operator *(num_t b, unsigned long a) { return num_t(b.i * a, nullptr); }
static inline constexpr num_t operator *(num_t b, unsigned long long a) { return num_t(b.i * a, nullptr); }

static inline constexpr num_t operator *(num_muldiv_t a, num_t b) { return b * a; }

static inline constexpr num_t operator /(num_t a, short b) { return num_t(a.i / b, nullptr); }
static inline constexpr num_t operator /(num_t a, int b) { return num_t(a.i / b, nullptr); }
static inline constexpr num_t operator /(num_t a, long b) { return num_t(a.i / b, nullptr); }
static inline constexpr num_t operator /(num_t a, long long b) { return num_t(a.i / b, nullptr); }

static inline constexpr num_t operator /(num_t a, unsigned short b) { return num_t(a.i / b, nullptr); }
static inline constexpr num_t operator /(num_t a, unsigned int b) { return num_t(a.i / b, nullptr); }
static inline constexpr num_t operator /(num_t a, unsigned long b) { return num_t(a.i / b, nullptr); }
static inline constexpr num_t operator /(num_t a, unsigned long long b) { return num_t(a.i / b, nullptr); }


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
using int_ok = num_muldiv_t;

// this is used for temporary values that were Singles in VB6
using tempf_t = numf_t;

#endif // #ifndef XT_FLOATING_POINT_H
