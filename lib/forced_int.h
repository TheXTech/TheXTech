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
#ifndef FORCED_INT_H
#define FORCED_INT_H

template<class t_vbint>
struct forced_int
{
private:
    t_vbint i;
public:
    forced_int(t_vbint _i) : i(_i) {}
    forced_int(double _i) = delete;

    forced_int& operator=(t_vbint o)
    {
        i = o;
        return *this;
    }

    forced_int& operator=(double o) = delete;

    forced_int& operator++()
    {
        i++;
        return *this;
    }

    forced_int operator++(int)
    {
        forced_int ret(i);
        i++;
        return ret;
    }

    forced_int& operator+=(t_vbint o)
    {
        i += o;
        return *this;
    }

    forced_int& operator+=(double o) = delete;

    forced_int& operator-=(t_vbint o)
    {
        i -= o;
        return *this;
    }

    forced_int& operator-=(double o) = delete;

    operator t_vbint() const
    {
        return i;
    }
};

#endif // #ifndef FORCED_INT_H
