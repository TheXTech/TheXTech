#ifndef RANGE_ARR_HPP
#define RANGE_ARR_HPP

#include <cstddef>
#include <cstring>
#include <cassert>

#define For(A, From, To) for(int A = From; A <= To; ++A)

template <class T, long begin, long end>
class RangeArr
{
    static constexpr long range_diff = begin - end;
    static constexpr size_t size = (range_diff < 0 ? -range_diff : range_diff) + 1;
    static const long offset = -begin;
    T array[size];
public:
    RangeArr()
    {}

    RangeArr(const RangeArr &o)
    {
        for(size_t i = 0; i < size; i++)
            array[i] = o.array[i];
    }

    RangeArr& operator=(const RangeArr &o)
    {
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
        assert(index <= end);
        assert(index >= begin);
        assert(offset + index < static_cast<long>(size));
        assert(offset + index >= 0);
        return array[offset + index];
    }
};

#endif // RANGE_ARR_HPP
