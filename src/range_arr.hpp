/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2022 Vitaly Novichkov <admin@wohlnet.ru>
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
#ifndef RANGE_ARR_HPP
#define RANGE_ARR_HPP

#include <cstddef>
#include <cstring>
#include <SDL2/SDL_assert.h>

#define For(A, From, To) for(int A = From; A <= To; ++A)

template <class T, long begin, long end>
class RangeArr
{
    static constexpr long range_diff = begin - end;
    static constexpr size_t size = (range_diff < 0 ? -range_diff : range_diff) + 1;
    static const long offset = -begin;
#ifdef RANGE_ARR_USE_HEAP
    T *array = nullptr;
#else
    T array[size];
#endif

public:
    RangeArr() noexcept
    {
#ifdef RANGE_ARR_USE_HEAP
        array = new T[size];
#endif
    }

#ifdef RANGE_ARR_USE_HEAP
    ~RangeArr()
    {
        delete[] array;
    }
#endif

    RangeArr(const RangeArr &o)
    {
#ifdef RANGE_ARR_USE_HEAP
        array = new T[size];
#endif
        for(size_t i = 0; i < size; i++)
            array[i] = o.array[i];
    }

    RangeArr& operator=(const RangeArr &o)
    {
#ifdef RANGE_ARR_USE_HEAP
        array = new T[size];
#endif
        for(size_t i = 0; i < size; i++)
            array[i] = o.array[i];
        return *this;
    }

    void fill(const T &o)
    {
        for(size_t i = 0; i < size; i++)
            array[i] = o;
    }

    T& operator[](long index)
    {
#ifdef RANGE_ARR_USE_HEAP
        SDL_assert_release(array); // When array won't initialize
#endif
        SDL_assert_release(index <= end);
        SDL_assert_release(index >= begin);
        SDL_assert_release(offset + index < static_cast<long>(size));
        SDL_assert_release(offset + index >= 0);
        return array[offset + index];
    }
};

template <class T, long begin, long end, T defaultValue>
class RangeArrI
{
    static constexpr long range_diff = begin - end;
    static constexpr size_t size = (range_diff < 0 ? -range_diff : range_diff) + 1;
    static const long offset = -begin;
#ifdef RANGE_ARR_USE_HEAP
    T *array = nullptr;
#else
    T array[size];
#endif

public:
    RangeArrI() noexcept
    {
#ifdef RANGE_ARR_USE_HEAP
        array = new T[size];
#endif
        for(size_t i = 0; i < size; i++)
            array[i] = defaultValue;
    }

#ifdef RANGE_ARR_USE_HEAP
    ~RangeArrI()
    {
        delete[] array;
    }
#endif

    RangeArrI(const RangeArrI &o)
    {
#ifdef RANGE_ARR_USE_HEAP
        array = new T[size];
#endif
        for(size_t i = 0; i < size; i++)
            array[i] = o.array[i];
    }

    RangeArrI& operator=(const RangeArrI &o)
    {
#ifdef RANGE_ARR_USE_HEAP
        array = new T[size];
#endif
        for(size_t i = 0; i < size; i++)
            array[i] = o.array[i];
        return *this;
    }

    void fill(const T &o)
    {
        for(size_t i = 0; i < size; i++)
            array[i] = o;
    }

    T& operator[](long index)
    {
#ifdef RANGE_ARR_USE_HEAP
        SDL_assert_release(array); // When array won't initialize
#endif
        SDL_assert_release(index <= end);
        SDL_assert_release(index >= begin);
        SDL_assert_release(offset + index < static_cast<long>(size));
        SDL_assert_release(offset + index >= 0);
        return array[offset + index];
    }
};

#endif // RANGE_ARR_HPP
