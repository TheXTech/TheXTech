/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2019-2021 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

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
    RangeArr()
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
    RangeArrI()
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
