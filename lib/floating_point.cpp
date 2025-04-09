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

#include "floating_point.h"
#include <cmath>
#include <cfenv>

#if defined(VITA) || defined(__3DS__)
#define USE_CUSTOM_TONEAREST
#endif

#ifdef USE_CUSTOM_TONEAREST
#include <pge_tonearest.h>
#endif

int32_t num_t::vb6round(num_t x)
{
#ifdef USE_CUSTOM_TONEAREST
        return pge_toNearest(x.i);
#else
        int round_old = std::fegetround();
        if(round_old == FE_TONEAREST)
            return std::nearbyint(x.i);
        else
        {
            std::fesetround(FE_TONEAREST);
            int32_t ret = std::nearbyint(x.i);
            std::fesetround(round_old);
            return ret;
        }
#endif
}

num_t num_t::dist(num_t dx, num_t dy)
{
    return num_t(std::sqrt(dx.i * dx.i + dy.i * dy.i), nullptr);
}

num_t num_t::idist(num_t dx, num_t dy)
{
    return num_t(1 / std::sqrt(dx.i * dx.i + dy.i * dy.i), nullptr);
}

num_t num_t::times(num_t o) const
{
    return num_t(i * o.i, nullptr);
}

num_t num_t::divided_by(num_t o) const
{
    return num_t(i / o.i, nullptr);
}

num_t num_t::sqrt(num_t x)
{
    return num_t(std::sqrt(x.i), nullptr);
}

num_t num_t::sin(num_t x)
{
    return num_t(std::sin(x.i), nullptr);
}

num_t num_t::cos(num_t x)
{
    return num_t(std::cos(x.i), nullptr);
}

num_t num_t::atan2(num_t y, num_t x)
{
    return num_t(std::atan2(y.i, x.i), nullptr);
}
