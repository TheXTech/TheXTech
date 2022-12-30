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

#ifndef SAFE_SET_HPP
#define SAFE_SET_HPP

#include <set>

template<class T>
class SafeSet
{
    using PlainSet = std::set<T>;
    using PlainIt = typename std::set<T>::iterator;

    bool invalid = false;
    PlainSet m_set;

public:
    struct InsertEraseSafeIteration
    {
        SafeSet& parent;

        InsertEraseSafeIteration(SafeSet& parent) : parent(parent) {}

        struct iterator
        {
            SafeSet* m_parent;
            PlainIt it;
            bool prev_invalid;

            T last_val;

            inline iterator(SafeSet& parent) : m_parent(&parent), it(parent.m_set.begin()), prev_invalid(parent.invalid)
            {
                if(it != parent.m_set.end())
                    last_val = *it;
                m_parent->invalid = false;
            }

            inline iterator(SafeSet& parent, std::nullptr_t ptr) : m_parent(ptr), it(parent.m_set.end()) {}

            inline ~iterator()
            {
                if(m_parent)
                    m_parent->invalid |= prev_invalid;
            }

            inline bool operator!=(const iterator& o)
            {
                return it != o.it;
            }

            inline bool operator!=(const PlainIt oit)
            {
                return it != oit;
            }

            inline T operator*()
            {
                return last_val;
            }

            inline const iterator& operator++()
            {
                if(m_parent->invalid)
                    it = m_parent->m_set.upper_bound(last_val);
                else
                    ++it;

                last_val = *it;

                return *this;
            }
        };

        iterator begin()
        {
            return iterator(parent);
        }

        iterator end()
        {
            return iterator(parent, nullptr);
        }
    };

    InsertEraseSafeIteration safe{*this};

    std::set<T>& no_change = m_set;
    std::set<T>& may_insert = m_set;
    InsertEraseSafeIteration& may_erase = safe;
    InsertEraseSafeIteration& may_insert_erase = safe;

    inline void clear()
    {
        invalid = true;
        m_set.clear();
    }

    inline void insert(const T& t)
    {
        m_set.insert(t);
    }

    inline void erase(const T& t)
    {
        invalid = true;
        m_set.erase(t);
    }
};

#endif // #ifndef SAFE_SET_HPP
