/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2023 Vitaly Novichkov <admin@wohlnet.ru>
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
#ifndef REF_TYPE_H
#define REF_TYPE_H

#include <cstdint>
#include <functional>
#include <type_traits>


struct BaseRef_t
{
    int16_t index;

    inline BaseRef_t() : index{INT16_MIN} {}
    inline BaseRef_t(int16_t i) : index{i} {}
    inline BaseRef_t(const BaseRef_t& r) : index(r.index) {}

    inline operator int() const { return index; }
    inline operator int16_t() const { return index; }
    inline operator size_t() const { return index; }
    inline operator bool() const { return *this != nullptr; }

    inline BaseRef_t& operator=(const BaseRef_t& o) { index = o.index; return *this; }
    inline BaseRef_t& operator=(std::nullptr_t /* o */) { index = INT16_MIN; return *this; }

    inline bool operator==(std::nullptr_t /* o */) const { return index == INT16_MIN; }
    inline bool operator!=(std::nullptr_t /* o */) const { return index != INT16_MIN; }

    inline bool operator==(const BaseRef_t& o) const { return index == o.index; }
    inline bool operator!=(const BaseRef_t& o) const { return index != o.index; }
    inline bool operator<(const BaseRef_t& o) const { return index < o.index; }
    inline bool operator>(const BaseRef_t& o) const { return index > o.index; }
    inline bool operator<=(const BaseRef_t& o) const { return index <= o.index; }
    inline bool operator>=(const BaseRef_t& o) const { return index >= o.index; }
};

#if defined(_MSC_VER)
#    define XTECH_TYPEOF(x) typename std::remove_reference<decltype(x)>::type
#else
#    define XTECH_TYPEOF(x) __typeof__(x)
#endif

template<class _target, _target& target>
struct Ref_t : public BaseRef_t
{
    using T = XTECH_TYPEOF(target[1]);
    using value_type = T;

    inline Ref_t() : BaseRef_t{} {}
    inline Ref_t(int16_t i) : BaseRef_t{i} {}
    inline Ref_t(const BaseRef_t& r) : BaseRef_t{r.index} {}
    inline Ref_t(const Ref_t& r) : BaseRef_t{r.index} {}
    inline Ref_t(T* p) : BaseRef_t{(int16_t)(p - &target[1] + 1)} {}
    inline Ref_t(T& p) : BaseRef_t{(int16_t)(&p - &target[1] + 1)} {}

    inline operator int() const { return index; }
    inline operator int16_t() const { return index; }
    inline operator size_t() const { return index; }
    inline operator bool() const { return *this != nullptr; }
    inline operator T*() const { return &target[index]; }
    inline operator T&() const { return target[index]; }

    inline T& operator*() const { return target[index]; }
    inline T* operator->() const { return &target[index]; }

    inline Ref_t& operator=(const Ref_t& o) { index = o.index; return *this; }
    inline BaseRef_t& operator=(std::nullptr_t /* o */) { index = INT16_MIN; return *this; }

    inline bool operator==(std::nullptr_t /* o */) const { return index == INT16_MIN; }
    inline bool operator!=(std::nullptr_t /* o */) const { return index != INT16_MIN; }

    inline bool operator==(const Ref_t& o) const { return index == o.index; }
    inline bool operator!=(const Ref_t& o) const { return index != o.index; }
    inline bool operator<(const Ref_t& o)  const { return index <  o.index; }
    inline bool operator>(const Ref_t& o)  const { return index >  o.index; }
    inline bool operator<=(const Ref_t& o) const { return index <= o.index; }
    inline bool operator>=(const Ref_t& o) const { return index >= o.index; }
};

#undef XTECH_TYPEOF

namespace std
{
    template <>
    struct hash<BaseRef_t>
    {
        uint16_t operator()(const BaseRef_t& k) const noexcept
        {
            return (uint16_t)k.index;
        }
    };

    template<class _target, _target& target>
    struct hash<Ref_t<_target, target>>
    {
        uint16_t operator()(const Ref_t<_target, target>& k) const noexcept
        {
            return (uint16_t)k.index;
        }
    };
}

#define DECLREF_T(target) using target ## Ref_t = Ref_t<decltype(target), target>

#endif // REF_TYPE_H
