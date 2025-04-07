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

#include "fixed_point.h"

#include "FixPointCS/Fixed64.h"

num_t num_t::dist(num_t dx, num_t dy)
{
    return num_t(Fixed64::Sqrt(dist2(dx, dy).i), nullptr);
}

num_t num_t::idist(num_t dx, num_t dy)
{
    return num_t(Fixed64::RSqrt(dist2(dx, dy).i), nullptr);
}

numf_t numf_t::times(numf_t o) const
{
    int64_t multiplied_24 = (int64_t)i * (int64_t)o.i;

    // banker's rounding
    int64_t multiplied_round = (multiplied_24 + ((int64_t)1 << 23));
    if((multiplied_round & 0x00FFFFFFLL) == 0)
    {
        if(multiplied_round & 0x1000000LL)
            multiplied_round -= 0x1000000LL;
    }

    int64_t multiplied = multiplied_round >> 24;

    return numf_t((int32_t)multiplied, nullptr);
}

numf_t numf_t::divided_by(numf_t o) const
{
    int64_t this_24 = (int64_t)i << 24;
    int64_t divided = this_24 / o.i;
    return numf_t((int32_t)divided, nullptr);
}

num_t num_t::times(num_t o) const
{
    int32_t whole_i = (int32_t)(i >> 32);
    uint32_t frac_i = (uint64_t)(i - whole_i * ((int64_t)1 << 32));

    int32_t whole_o = (int32_t)(o.i >> 32);
    uint32_t frac_o = (uint64_t)(o.i - whole_o * ((int64_t)1 << 32));

    int64_t whole_i_times_o = whole_i * o.i;

    int64_t frac_i_times_whole_o = (int64_t)frac_i * whole_o;

    uint64_t frac_i_times_frac_o_32 = (uint64_t)frac_i * frac_o;

    // banker's rounding
    int64_t frac_i_times_frac_o_round = (frac_i_times_frac_o_32 + ((int64_t)1 << 31));
    if((frac_i_times_frac_o_round & 0xFFFFFFFFLL) == 0)
    {
        if(frac_i_times_frac_o_round & 0x100000000LL)
            frac_i_times_frac_o_round -= 0x100000000LL;
    }

    int64_t frac_i_times_frac_o = frac_i_times_frac_o_round >> 32;

    return num_t(whole_i_times_o + frac_i_times_whole_o + frac_i_times_frac_o, nullptr);
}

num_t num_t::divided_by(num_t o) const
{
    return num_t(Fixed64::DivPrecise(i, o.i), nullptr);
}

num_t num_t::sin(num_t x)
{
    return num_t(Fixed64::Cos(x.i), nullptr);
}

num_t num_t::cos(num_t x)
{
    return num_t(Fixed64::Cos(x.i), nullptr);
}

num_t num_t::atan2(num_t y, num_t x)
{
    return num_t(Fixed64::Atan2(y.i, x.i), nullptr);
}
