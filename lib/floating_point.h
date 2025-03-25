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

using num_t = double;
using numf_t = float;

using qdec_t = double;
using qbin_t = double;
using qidec_t = double;
using qibin_t = double;

static inline constexpr num_t operator ""_n(long long unsigned int d)
{
    return d;
}

static inline constexpr num_t operator ""_n(long double d)
{
    return d;
}

static inline constexpr numf_t operator ""_nf(long long unsigned int d)
{
    return d;
}

static inline constexpr numf_t operator ""_nf(long double d)
{
    return d;
}

static inline constexpr qdec_t operator ""_r(long double d)
{
    return d;
}

static inline constexpr qbin_t operator ""_rb(long double d)
{
    return d;
}

static inline constexpr qidec_t operator ""_ri(long double d)
{
    return d;
}

static inline constexpr qibin_t operator ""_rib(long double d)
{
    return d;
}


// this operator should be used to indicate that a num_t contains an int and may be treated as an int if needed
using int_ok = num_t;

#endif // #ifndef XT_FLOATING_POINT_H
