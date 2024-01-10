/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2024 Vitaly Novichkov <admin@wohlnet.ru>
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

#include "sdl_proxy/sdl_stdinc.h"

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
                    this->cur_loc.cont_axes |= CONT_X;
                    this->cur_loc.cont_axes &= ~CONT_Y | (this->parent.cont_axes & CONT_Y);
                    this->cur_loc.y = this->parent.t;
                }
            }

            return *this;
        }

        inline bool operator!=(const iterator& other) const
        {
            return this->cur_loc.y != other.cur_loc.y || this->cur_loc.x != other.cur_loc.x;
        }

        inline const AugLoc_t& operator*() const
        {
            return cur_loc;
        }
    };

    inline iterator begin() const
    {
        // could consider an optimization where != is only properly implemented for != end(),
        // removing this condition and replacing the `||` in != with `&&`.

        if(t == b || l == r)
            return end();

        return iterator(*this, {l, t, cont_axes});
    }

    inline iterator end() const
    {
        return iterator(*this, {r, b, CONT_XY});
    }
};

struct rect_external
{
    int32_t l, r, t, b;

    rect_external() {}

    rect_external(int32_t l, int32_t r, int32_t t, int32_t b) : l(l), r(r), t(t), b(b) {}

    rect_external(const Location_t& loc)
    {
        l = std::floor(loc.X);
        r = std::ceil(loc.X + loc.Width);
        t = std::floor(loc.Y);
        b = std::ceil(loc.Y + loc.Height);
    }
};

// a screen is 2048x2048.
// it's made up of node_t's, which are 64x64 each.
// soon it will have 33x33 nodes instead of 32x32, so that there's some room for shifting the offset for performance
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
    return obj->LocationGFX();
}

template<>
inline Location_t extract_loc(NPCRef_t obj)
{
    Location_t ret = obj->Location;

    if(ret.Height < NPCHeight[obj->Type])
        ret.Height = NPCHeight[obj->Type];

    if(ret.Width < NPCWidthGFX[obj->Type])
    {
        ret.X -= (NPCWidthGFX[obj->Type] - ret.Width) / 2.0;
        ret.Width = NPCWidthGFX[obj->Type];
    }

    // for tempBlock queries
    if(obj->Type == 26)
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
template<class MyRef_t>
struct table_t
{
    typedef std::vector<screen_t*> screen_ptr_arr_t;
    std::vector<screen_ptr_arr_t> columns;
    std::vector<int> col_first_row_index;
    std::unordered_map<MyRef_t, rect_external> member_rects;
    int first_col_index;

    ~table_t()
    {
        clear();
    }

private:
    void insert(MyRef_t b, const rect_external& rect)
    {
        int lcol = rect.l / 2048;
        if(rect.l < 0 && (rect.l % 2048))
            lcol -= 1;

        int rcol = rect.r / 2048;
        if(rect.r > 0 && (rect.r % 2048))
            rcol += 1;

        int trow = rect.t / 2048;
        if(rect.t < 0 && (rect.t % 2048))
            trow -= 1;

        int brow = rect.b / 2048;
        if(rect.b > 0 && (rect.b % 2048))
            brow += 1;

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

        inner_rect.cont_axes = CONT_NONE;

        for(int col = lcol; col < rcol; col++)
        {
            int internal_col = col - first_col_index;

            int inner_l = 0;
            if(col == lcol)
                inner_l = rect.l - lcol * 2048;

            int inner_r = 2048;
            if(col == rcol - 1)
                inner_r = rect.r - (rcol - 1) * 2048;

            // apply offset if needed
            inner_rect.l = inner_l / 64;
            inner_rect.r = inner_r / 64;
            if(inner_r & 63)
                inner_rect.r += 1;

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

            for(int row = trow; row < brow; row++)
            {
                int internal_row = row - col_first_row_index[internal_col];

                int inner_t = 0;
                if(row == trow)
                    inner_t = rect.t - trow * 2048;

                int inner_b = 2048;
                if(row == brow - 1)
                    inner_b = rect.b - (brow - 1) * 2048;

                // apply offset if needed
                inner_rect.t = inner_t / 64;
                inner_rect.b = inner_b / 64;
                if(inner_b & 63)
                    inner_rect.b += 1;

                columns[internal_col][internal_row]->insert(b, inner_rect);

                inner_rect.cont_axes |= CONT_Y;
            }

            inner_rect.cont_axes = CONT_X;
        }
    }

    void erase(MyRef_t b, const rect_external& rect)
    {
        int lcol = rect.l / 2048;
        if(rect.l < 0 && (rect.l % 2048))
            lcol -= 1;

        int rcol = rect.r / 2048;
        if(rect.r > 0 && (rect.r % 2048))
            rcol += 1;

        int trow = rect.t / 2048;
        if(rect.t < 0 && (rect.t % 2048))
            trow -= 1;

        int brow = rect.b / 2048;
        if(rect.b > 0 && (rect.b % 2048))
            brow += 1;

        int lcol_check = SDL_max(lcol, first_col_index);
        int rcol_check = SDL_min(rcol, first_col_index + (int)columns.size());

        rect_internal inner_rect;

        inner_rect.cont_axes = CONT_NONE;
        if(lcol_check != lcol)
            inner_rect.cont_axes = CONT_X;

        for(int col = lcol_check; col < rcol_check; col++)
        {
            int internal_col = col - first_col_index;

            int inner_l = 0;
            if(col == lcol)
                inner_l = rect.l - lcol * 2048;

            int inner_r = 2048;
            if(col == rcol - 1)
                inner_r = rect.r - (rcol - 1) * 2048;

            // apply offset if needed
            inner_rect.l = inner_l / 64;
            inner_rect.r = inner_r / 64;
            if(inner_r & 63)
                inner_rect.r += 1;

            int trow_check = SDL_max(trow, col_first_row_index[internal_col]);
            int brow_check = SDL_min(brow, col_first_row_index[internal_col] + (int)columns[internal_col].size());
            if(trow_check != trow)
                inner_rect.cont_axes |= CONT_Y;

            for(int row = trow_check; row < brow_check; row++)
            {
                int internal_row = row - col_first_row_index[internal_col];

                int inner_t = 0;
                if(row == trow)
                    inner_t = rect.t - trow * 2048;

                int inner_b = 2048;
                if(row == brow - 1)
                    inner_b = rect.b - (brow - 1) * 2048;

                // apply offset if needed
                inner_rect.t = inner_t / 64;
                inner_rect.b = inner_b / 64;
                if(inner_b & 63)
                    inner_rect.b += 1;

                columns[internal_col][internal_row]->erase(b, inner_rect);

                inner_rect.cont_axes |= CONT_Y;
            }

            inner_rect.cont_axes = CONT_X;
        }
    }

public:
    void query(std::vector<BaseRef_t>& out, const rect_external& rect)
    {
        if(columns.size() == 0 || member_rects.size() == 0)
            return;

        int lcol = rect.l / 2048;
        if(rect.l < 0 && (rect.l % 2048))
            lcol -= 1;

        int rcol = rect.r / 2048;
        if(rect.r > 0 && (rect.r % 2048))
            rcol += 1;

        int trow = rect.t / 2048;
        if(rect.t < 0 && (rect.t % 2048))
            trow -= 1;

        int brow = rect.b / 2048;
        if(rect.b > 0 && (rect.b % 2048))
            brow += 1;

        int lcol_check = SDL_max(lcol, first_col_index);
        int rcol_check = SDL_min(rcol, first_col_index + (int)columns.size());

        rect_internal inner_rect;

        inner_rect.cont_axes = CONT_NONE;
        if(lcol_check != lcol)
            inner_rect.cont_axes = CONT_X;

        for(int col = lcol_check; col < rcol_check; col++)
        {
            int internal_col = col - first_col_index;

            if(columns[internal_col].size() == 0)
            {
                inner_rect.cont_axes |= CONT_X;
                continue;
            }

            int inner_l = 0;
            if(col == lcol)
                inner_l = rect.l - lcol * 2048;

            int inner_r = 2048;
            if(col == rcol - 1)
                inner_r = rect.r - (rcol - 1) * 2048;

            // apply offset if needed
            inner_rect.l = inner_l / 64;
            inner_rect.r = inner_r / 64;
            if(inner_r & 63)
                inner_rect.r += 1;

            int trow_check = SDL_max(trow, col_first_row_index[internal_col]);
            int brow_check = SDL_min(brow, col_first_row_index[internal_col] + (int)columns[internal_col].size());
            if(trow_check != trow)
                inner_rect.cont_axes |= CONT_Y;

            for(int row = trow_check; row < brow_check; row++)
            {
                int internal_row = row - col_first_row_index[internal_col];

                int inner_t = 0;
                if(row == trow)
                    inner_t = rect.t - trow * 2048;

                int inner_b = 2048;
                if(row == brow - 1)
                    inner_b = rect.b - (brow - 1) * 2048;

                // apply offset if needed
                inner_rect.t = inner_t / 64;
                inner_rect.b = inner_b / 64;
                if(inner_b & 63)
                    inner_rect.b += 1;

                columns[internal_col][internal_row]->query(out, inner_rect);

                inner_rect.cont_axes |= CONT_Y;
            }

            inner_rect.cont_axes = CONT_X;
        }
    }

    void insert(MyRef_t b)
    {
        Location_t loc = extract_loc<MyRef_t>(b);

        // ignore improper rects
        if(loc.Width < 0 || loc.Height < 0)
            return;

        rect_external rect(loc);
        member_rects[b] = rect;
        insert(b, rect);
    }

    void insert_layer(MyRef_t b)
    {
        Location_t loc = extract_loc_layer<MyRef_t>(b);

        // ignore improper rects
        if(loc.Width < 0 || loc.Height < 0)
            return;

        rect_external rect(loc);
        member_rects[b] = rect;
        insert(b, rect);
    }

    void erase(MyRef_t b)
    {
        auto it = member_rects.find(b);
        if(it == member_rects.end())
            return;

        erase(b, it->second);
        member_rects.erase(it);
    }

    void update(MyRef_t b)
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
                return;
            }

            erase(b, it->second);
        }

        // ignore improper rects
        if(loc.Width < 0 || loc.Height < 0)
            return;

        member_rects[b] = rect;
        insert(b, rect);
    }

    void update_layer(MyRef_t b)
    {
        auto it = member_rects.find(b);
        if(it != member_rects.end())
            erase(b, it->second);

        insert_layer(b);
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
            erase(p.first, p.second);

        member_rects.clear();
    }
};

#endif // #ifndef BLOCK_TABLE_IMPL_HPP
