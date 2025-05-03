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

#ifndef BLOCK_TABLE_IMPL_HPP
#define BLOCK_TABLE_IMPL_HPP

#include <iterator>
#include <array>
#include <set>
#include <unordered_map>

#include "globals.h"
#include "layers.h"
#include "npc_traits.h"

#include "sdl_proxy/sdl_stdinc.h"

static inline int32_t s_floor_div_32(int32_t x)
{
    static_assert(((-1) >> 5) == -1 && (31 >> 5) == 0 && (-3232 >> 5) == -101, "block_table.hpp requires signed right shift to have floor semantics");
    return (int16_t)(x >> 5);
}

static inline int32_t s_floor_div_64(int32_t x)
{
    static_assert(((-1) >> 6) == -1 && (63 >> 6) == 0 && (-6464 >> 6) == -101, "block_table.hpp requires signed right shift to have floor semantics");
    return (int16_t)(x >> 6);
}

enum ContinuedRect
{
    CONT_NONE = 0,
    CONT_X = 1,
    CONT_Y = 2,
    CONT_XY = CONT_X | CONT_Y,
};

struct AugBaseRef_t
{
    BaseRef_t ref;
    uint8_t cont_axes;
};

struct node_t
{
    // not safe to change this
    static constexpr int node_size = 4;

    node_t* next = nullptr;
    int16_t refs[node_size];
    uint8_t filled = 0;
    uint8_t cont_axes = 0;

    inline node_t()
    {
        next = nullptr;
        filled = 0;
        cont_axes = 0;
    }

    inline ~node_t()
    {
        if(next)
            delete next;
    }

    struct iterator
    {
        node_t* parent;
        uint16_t i;

        constexpr inline iterator(std::nullptr_t parent) : parent(parent), i(0) {}

        inline void check_linkage()
        {
            while(this->parent && this->i == this->parent->filled)
            {
                this->parent = this->parent->next;
                this->i = 0;
            }
        }

        inline iterator(node_t* parent, size_t i): parent(parent), i((uint16_t)i)
        {
            this->check_linkage();
        }

        inline iterator operator++()
        {
            if(this->parent)
            {
                this->i++;
                if(this->i == this->parent->filled)
                {
                    this->parent = this->parent->next;
                    this->i = 0;
                    this->check_linkage();
                }
            }
            return *this;
        }

        inline bool operator!=(const iterator& other) const
        {
            return this->parent != other.parent || this->i != other.i;
        }

        inline AugBaseRef_t operator*() const
        {
            return {this->parent->refs[this->i], (uint8_t)((this->parent->cont_axes >> (this->i * 2)) & 3)};
        }
    };

    inline iterator begin()
    {
        return iterator(this, 0);
    }

    static constexpr inline iterator end()
    {
        return iterator(nullptr);
    }

    inline void insert(AugBaseRef_t b)
    {
        if(this->filled < node_size)
        {
            this->refs[this->filled] = b.ref;
            this->cont_axes |= (b.cont_axes & 3) << (this->filled * 2);
            this->filled++;
        }
        else
        {
            if(!this->next)
                this->next = new node_t;
            this->next->insert(b);
        }
    }

    inline void erase(iterator& it)
    {
        it.parent->filled--;
        it.parent->refs[it.i] = it.parent->refs[it.parent->filled];
        it.parent->cont_axes &= ~(3 << (it.i * 2));
        it.parent->cont_axes |= ((it.parent->cont_axes >> (it.parent->filled * 2)) & 3) << (it.i * 2);
        it.parent->cont_axes &= ~(3 << (it.parent->filled * 2));
    }

    inline void erase(BaseRef_t o)
    {
        iterator it = this->begin();

        while(it != this->end())
        {
            if((*it).ref == o)
                break;
            ++it;
        }

        if(it != this->end())
            this->erase(it);
    }
};

struct AugLoc_t
{
    int16_t x, y;
    uint8_t cont_axes;
};

struct rect_internal
{
    int16_t t, l, b, r;
    uint8_t cont_axes;

    class iterator
    {
        friend struct rect_internal;
    private:
        const rect_internal& parent;
        AugLoc_t cur_loc;

    public:
        inline iterator(const rect_internal& parent, AugLoc_t cur_loc): parent(parent), cur_loc(cur_loc)
        {}

        inline iterator operator++()
        {
            this->cur_loc.y++;
            this->cur_loc.cont_axes |= CONT_Y;

            if(this->cur_loc.y == this->parent.b)
            {
                this->cur_loc.x++;

                if(this->cur_loc.x != this->parent.r)
                {
                    // CONT_X for certain, CONT_Y if it's set by the parent
                    this->cur_loc.cont_axes = CONT_X | this->parent.cont_axes;
                    this->cur_loc.y = this->parent.t;
                }
            }

            return *this;
        }

        inline bool operator!=(const iterator& end) const
        {
            // only check for inequality on X axis; that signals iteration is over
            return this->cur_loc.x != end.cur_loc.x;
        }

        inline const AugLoc_t& operator*() const
        {
            return cur_loc;
        }
    };

    inline iterator begin() const
    {
        iterator ret(*this, {l, t, cont_axes});

        // if range is empty, return end instead
        if(t == b)
            ret.cur_loc.x = r;

        return ret;
    }

    inline iterator end() const
    {
        return iterator(*this, {r, b, CONT_XY});
    }
};

struct rect_external
{
    int16_t l, r, t, b;

    rect_external() {}

    rect_external(int16_t l, int16_t r, int16_t t, int16_t b) : l(l), r(r), t(t), b(b) {}

    rect_external(const Location_t& loc)
    {
        // find which 64x64 tiles contain the object
        l = s_floor_div_64((int32_t)num_t::floor(loc.X));
        r = s_floor_div_64((int32_t)num_t::ceil(loc.X + loc.Width) + 63);
        t = s_floor_div_64((int32_t)num_t::floor(loc.Y));
        b = s_floor_div_64((int32_t)num_t::ceil(loc.Y + loc.Height) + 63);
    }

    void query_from_loc_paranoid(const Location_t& loc)
    {
        // find which 64x64 tiles contain the query with a 32px margin in each direction
        l = s_floor_div_64((int32_t)num_t::floor(loc.X) - 32);
        r = s_floor_div_64((int32_t)num_t::ceil(loc.X + loc.Width) + 32 + 63);
        t = s_floor_div_64((int32_t)num_t::floor(loc.Y) - 32);
        b = s_floor_div_64((int32_t)num_t::ceil(loc.Y + loc.Height) + 32 + 63);
    }

    void query_from_loc_standard(const Location_t& loc)
    {
        // find which 64x64 tiles contain the query with a 2px margin in each direction
        l = s_floor_div_64((int32_t)num_t::floor(loc.X) - 2);
        r = s_floor_div_64((int32_t)num_t::ceil(loc.X + loc.Width) + 2 + 63);
        t = s_floor_div_64((int32_t)num_t::floor(loc.Y) - 2);
        b = s_floor_div_64((int32_t)num_t::ceil(loc.Y + loc.Height) + 2 + 63);
    }
};

// a screen is 2048x2048.
// it's made up of node_t's, which are 64x64 each.
struct screen_t
{
    std::array<node_t, 1024> nodes;

    screen_t()
    {}

    void query(std::vector<BaseRef_t>& out, const rect_internal& rect)
    {
        for(const AugLoc_t& loc : rect)
        {
            for(AugBaseRef_t obj : nodes[loc.x * 32 + loc.y])
            {
                // only want objs that are new on all continued axes
                if((obj.cont_axes & loc.cont_axes) == CONT_NONE)
                    out.push_back(obj.ref);
            }
        }
    }

    void insert(BaseRef_t obj, const rect_internal& rect)
    {
        for(const AugLoc_t& loc : rect)
            nodes[loc.x * 32 + loc.y].insert({obj, loc.cont_axes});
    }

    void erase(BaseRef_t obj, const rect_internal& rect)
    {
        for(const AugLoc_t& loc : rect)
            nodes[loc.x * 32 + loc.y].erase(obj);
    }
};

template<class MyRef_t>
inline Location_t extract_loc(MyRef_t obj)
{
    return static_cast<Location_t>(obj->Location);
}

template<>
inline Location_t extract_loc(WorldLevelRef_t obj)
{
    return obj->LocationOnscreen();
}

template<>
inline Location_t extract_loc(NPCRef_t obj)
{
    Location_t ret = obj->Location;

    if(ret.Height < NPCHeight(obj->Type))
        ret.Height = NPCHeight(obj->Type);
    else if(ret.Height <= 0)
        ret.Height = 1;

    if(ret.Width < NPCWidthGFX(obj->Type))
    {
        ret.X -= (NPCWidthGFX(obj->Type) - ret.Width) / 2;
        ret.Width = NPCWidthGFX(obj->Type);
    }
    else if(ret.Width <= 0)
        ret.Width = 1;

    // for tempBlock queries
    if(obj->Type == NPCID_SPRING)
    {
        ret.Y -= 16;
        ret.Height += 16;
    }

    return ret;
}

template<class MyRef_t>
inline Location_t extract_loc_layer(MyRef_t obj)
{
    Location_t loc = static_cast<Location_t>(obj->Location);

    if(obj->Layer != LAYER_NONE)
    {
        loc.X -= Layer[obj->Layer].OffsetX;
        loc.Y -= Layer[obj->Layer].OffsetY;
    }

    return loc;
}

// it's a bunch of stacks of screens, which are 2048x2048.
// will be reallocated as needed
struct base_table_t
{
    typedef std::vector<screen_t*> screen_ptr_arr_t;
    std::vector<screen_ptr_arr_t> columns;
    std::vector<int> col_first_row_index;
    std::unordered_map<BaseRef_t, rect_external> member_rects;
    int first_col_index;

    ~base_table_t()
    {
        clear();
    }

protected:
    void insert(BaseRef_t b, const rect_external& rect)
    {
        int lcol = s_floor_div_32(rect.l); // each column contains 32 cells
        int rcol = s_floor_div_32(rect.r + 31); // ceiling, this column won't get checked

        int trow = s_floor_div_32(rect.t);
        int brow = s_floor_div_32(rect.b + 31);

        if(columns.size() == 0)
        {
            columns.emplace_back(screen_ptr_arr_t());
            col_first_row_index.resize(1);
            first_col_index = lcol;
        }

        if(lcol < first_col_index)
        {
            int to_add = first_col_index - lcol;
            for(int i = 0; i < to_add; i++)
                columns.emplace_back(screen_ptr_arr_t());

            std::rotate(columns.rbegin(), columns.rbegin() + to_add, columns.rend());

            col_first_row_index.resize(columns.size() + to_add);
            std::rotate(col_first_row_index.rbegin(), col_first_row_index.rbegin() + to_add, col_first_row_index.rend());

            first_col_index = lcol;
        }

        if(rcol > first_col_index + (int)columns.size())
        {
            for(int i = first_col_index + (int)columns.size(); i < rcol; i++)
                columns.emplace_back(screen_ptr_arr_t());

            col_first_row_index.resize(rcol - first_col_index);
        }

        rect_internal inner_rect;

        inner_rect.l = rect.l - lcol * 32;
        inner_rect.r = 32;
        inner_rect.cont_axes = CONT_NONE;

        for(int col = lcol; col < rcol; col++)
        {
            int internal_col = col - first_col_index;

            if(col == rcol - 1)
                inner_rect.r = rect.r - (rcol - 1) * 32;

            if(columns[internal_col].size() == 0)
            {
                columns[internal_col].push_back(new screen_t);
                col_first_row_index[internal_col] = trow;
            }

            if(trow < col_first_row_index[internal_col])
            {
                int to_add = col_first_row_index[internal_col] - trow;
                for(int i = 0; i < to_add; i++)
                    columns[internal_col].push_back(new screen_t);
                std::rotate(columns[internal_col].rbegin(), columns[internal_col].rbegin() + to_add, columns[internal_col].rend());
                col_first_row_index[internal_col] = trow;
            }

            if(brow > col_first_row_index[internal_col] + (int)columns[internal_col].size())
            {
                for(int i = col_first_row_index[internal_col] + (int)columns[internal_col].size(); i < brow; i++)
                    columns[internal_col].push_back(new screen_t);
            }

            inner_rect.t = rect.t - trow * 32;
            inner_rect.b = 32;

            for(int row = trow; row < brow; row++)
            {
                int internal_row = row - col_first_row_index[internal_col];

                if(row == brow - 1)
                    inner_rect.b = rect.b - (brow - 1) * 32;

                columns[internal_col][internal_row]->insert(b, inner_rect);

                inner_rect.t = 0;
                inner_rect.cont_axes |= CONT_Y;
            }

            inner_rect.l = 0;
            inner_rect.cont_axes = CONT_X;
        }
    }

    void erase(BaseRef_t b, const rect_external& rect)
    {
        int lcol = s_floor_div_32(rect.l); // each column contains 32 cells
        int rcol = s_floor_div_32(rect.r + 31); // ceiling, this column won't get checked

        int trow = s_floor_div_32(rect.t);
        int brow = s_floor_div_32(rect.b + 31);

        int lcol_check = SDL_max(lcol, first_col_index);
        int rcol_check = SDL_min(rcol, first_col_index + (int)columns.size());

        rect_internal inner_rect;

        // initialize l/r bounds of inner query rect
        inner_rect.l = rect.l - lcol * 32;
        inner_rect.r = 32;
        inner_rect.cont_axes = CONT_NONE;

        if(lcol_check != lcol)
        {
            inner_rect.l = 0;
            inner_rect.cont_axes = CONT_X;
        }

        for(int col = lcol_check; col < rcol_check; col++)
        {
            int internal_col = col - first_col_index;

            // it's 32 until this point
            if(col == rcol - 1)
                inner_rect.r = rect.r - (rcol - 1) * 32;

            // initialize t/b bounds of inner query rect
            inner_rect.t = rect.t - trow * 32;
            inner_rect.b = 32;

            int trow_check = SDL_max(trow, col_first_row_index[internal_col]);
            int brow_check = SDL_min(brow, col_first_row_index[internal_col] + (int)columns[internal_col].size());
            if(trow_check != trow)
            {
                inner_rect.t = 0;
                inner_rect.cont_axes |= CONT_Y;
            }

            for(int row = trow_check; row < brow_check; row++)
            {
                int internal_row = row - col_first_row_index[internal_col];

                if(row == brow - 1)
                    inner_rect.b = rect.b - (brow - 1) * 32;

                columns[internal_col][internal_row]->erase(b, inner_rect);

                inner_rect.t = 0;
                inner_rect.cont_axes |= CONT_Y;
            }

            inner_rect.l = 0;
            inner_rect.cont_axes = CONT_X;
        }
    }

public:
    void query(std::vector<BaseRef_t>& out, const rect_external& rect)
    {
        if(columns.size() == 0 || member_rects.size() == 0)
            return;

        int lcol = s_floor_div_32(rect.l); // each column contains 32 cells
        int rcol = s_floor_div_32(rect.r + 31); // ceiling, this column won't get checked

        int trow = s_floor_div_32(rect.t);
        int brow = s_floor_div_32(rect.b + 31);

        int lcol_check = SDL_max(lcol, first_col_index);
        int rcol_check = SDL_min(rcol, first_col_index + (int)columns.size());

        rect_internal inner_rect;

        // initialize l/r bounds of inner query rect
        inner_rect.l = rect.l - lcol * 32;
        inner_rect.r = 32;
        inner_rect.cont_axes = CONT_NONE;

        if(lcol_check != lcol)
        {
            inner_rect.l = 0;
            inner_rect.cont_axes = CONT_X;
        }

        for(int col = lcol_check; col < rcol_check; col++)
        {
            int internal_col = col - first_col_index;

            if(columns[internal_col].size() == 0)
            {
                inner_rect.l = 0;
                inner_rect.cont_axes = CONT_X;
                continue;
            }

            // it's 32 until this point
            if(col == rcol - 1)
                inner_rect.r = rect.r - (rcol - 1) * 32;

            // initialize t/b bounds of inner query rect
            inner_rect.t = rect.t - trow * 32;
            inner_rect.b = 32;

            int trow_check = SDL_max(trow, col_first_row_index[internal_col]);
            int brow_check = SDL_min(brow, col_first_row_index[internal_col] + (int)columns[internal_col].size());
            if(trow_check != trow)
            {
                inner_rect.t = 0;
                inner_rect.cont_axes |= CONT_Y;
            }

            for(int row = trow_check; row < brow_check; row++)
            {
                int internal_row = row - col_first_row_index[internal_col];

                if(row == brow - 1)
                    inner_rect.b = rect.b - (brow - 1) * 32;

                columns[internal_col][internal_row]->query(out, inner_rect);

                inner_rect.t = 0;
                inner_rect.cont_axes |= CONT_Y;
            }

            inner_rect.l = 0;
            inner_rect.cont_axes = CONT_X;
        }
    }

    void clear()
    {
        // can optimize later
        for(auto col : columns)
        {
            for(screen_t* screen : col)
                delete screen;
        }

        columns.clear();
        col_first_row_index.clear();
        member_rects.clear();
    }

    void clear_light()
    {
        for(const auto& p : member_rects)
            base_table_t::erase(p.first, p.second);

        member_rects.clear();
    }
};

template<class MyRef_t>
struct table_t : public base_table_t
{
    void insert(MyRef_t b)
    {
        Location_t loc = extract_loc<MyRef_t>(b);

        // ignore improper rects
        if(loc.Width < 0 || loc.Height < 0)
            return;

        rect_external rect(loc);
        member_rects[b] = rect;
        base_table_t::insert(b, rect);
    }

    void insert_layer(MyRef_t b)
    {
        Location_t loc = extract_loc_layer<MyRef_t>(b);

        // ignore improper rects
        if(loc.Width < 0 || loc.Height < 0)
            return;

        rect_external rect(loc);
        member_rects[b] = rect;
        base_table_t::insert(b, rect);
    }

    void erase(MyRef_t b)
    {
        auto it = member_rects.find(b);
        if(it == member_rects.end())
            return;

        base_table_t::erase(b, it->second);
        member_rects.erase(it);
    }

    bool update(MyRef_t b)
    {
        Location_t loc = extract_loc<MyRef_t>(b);

        rect_external rect(loc);

        auto it = member_rects.find(b);
        if(it != member_rects.end())
        {
            // no-change optimization
            if(it->second.l == rect.l
                && it->second.r == rect.r
                && it->second.t == rect.t
                && it->second.b == rect.b)
            {
                return false;
            }

            base_table_t::erase(b, it->second);
        }

        // ignore improper rects
        if(loc.Width < 0 || loc.Height < 0)
            return true;

        member_rects[b] = rect;
        base_table_t::insert(b, rect);

        return true;
    }

    void update_layer(MyRef_t b)
    {
        auto it = member_rects.find(b);
        if(it != member_rects.end())
            base_table_t::erase(b, it->second);

        insert_layer(b);
    }
};

inline void trees_sort_by_index(std::vector<BaseRef_t>& i_vec)
{
    pdqsort(i_vec.begin(), i_vec.end(),
        [](BaseRef_t a, BaseRef_t b) {
            return a < b;
        });
}

#endif // #ifndef BLOCK_TABLE_IMPL_HPP
