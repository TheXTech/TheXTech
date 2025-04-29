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

#ifndef THEXTECH_TINY_SORT_H
#define THEXTECH_TINY_SORT_H

#include <functional>
#include <iterator>

// an extremely simple introsort implementation intended for correctness as a stable sort, and code size.
// DO NOT USE IF PERFORMANCE IS IMPORTANT.
template<class it, class compare_func>
inline void tinysort(it begin, it end, compare_func comp)
{
    if(begin == end)
        return;

    typedef typename std::iterator_traits<it>::value_type T;

    for(it i = begin + 1; i != end; ++i)
    {
        T x = std::move(*i);

        it j;
        for(j = i; j > begin && comp(x, *(j - 1)); --j)
            *j = std::move(*(j - 1));

        *j = std::move(x);
    }
}

template<class it>
inline void tinysort(it begin, it end)
{
    typedef typename std::iterator_traits<it>::value_type T;
    tinysort(begin, end, std::less<T>());
}

#endif // #ifndef THEXTECH_TINY_SORT_H
