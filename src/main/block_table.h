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

#ifndef BLOCK_TABLE_H
#define BLOCK_TABLE_H

#include <iterator>
#include <array>
#include <set>
#include <unordered_map>

#include "globals.h"

enum ContinuedRect
{
    CONT_NONE = 0,
    CONT_X = 1,
    CONT_Y = 2,
    CONT_XY = CONT_X | CONT_Y,
};

struct AugBlockRef_t
{
    BlockRef_t block;
    uint8_t cont_axes;
};

struct node_t
{
    static constexpr int node_size = 4;

    node_t* next = nullptr;
    int16_t block[node_size];
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

        constexpr inline iterator(nullptr_t parent) : parent(parent), i(0) {}

        inline void check_linkage()
        {
            while(this->parent && this->i == this->parent->filled)
            {
                this->parent = this->parent->next;
                this->i = 0;
            }
        }
        inline iterator(node_t* parent, size_t i): parent(parent), i(i)
        {
            this->check_linkage();
        }
        inline iterator operator++()
        {
            if(this->parent)
            {
                this->i += 1;
                this->check_linkage();
            }
            return *this;
        }
        inline bool operator!=(const iterator& other) const
        {
            return this->parent != other.parent || this->i != other.i;
        }
        inline AugBlockRef_t operator*() const
        {
            return {this->parent->block[this->i], (uint8_t)((this->parent->cont_axes >> (this->i * 2)) & 3)};
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

    inline void insert(AugBlockRef_t b)
    {
        if(this->filled < node_size)
        {
            this->block[this->filled] = b.block;
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
        it.parent->block[it.i] = it.parent->block[it.parent->filled];
        it.parent->cont_axes &= ~(3 << (it.i * 2));
        it.parent->cont_axes |= ((it.parent->cont_axes >> (it.parent->filled * 2)) & 3) << (it.i * 2);
        it.parent->cont_axes &= ~(3 << (it.parent->filled * 2));
    }

    inline void erase(BlockRef_t b)
    {
        iterator it = this->begin();
        while(it != this->end())
        {
            if((*it).block == b)
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
        inline iterator(const rect_internal& parent, AugLoc_t cur_loc): parent(parent), cur_loc(cur_loc) {}
        inline iterator operator++()
        {
            this->cur_loc.x++;
            this->cur_loc.cont_axes |= CONT_X;
            if(this->cur_loc.x == this->parent.r)
            {
                this->cur_loc.y ++;

                if(this->cur_loc.y != this->parent.b)
                {
                    this->cur_loc.cont_axes |= CONT_Y;
                    this->cur_loc.cont_axes &= ~CONT_X | (this->parent.cont_axes & CONT_X);
                    this->cur_loc.x = this->parent.l;
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
    int32_t t, l, b, r;

    rect_external() {}

    rect_external(const Location_t& loc)
    {
        l = std::floor(loc.X);
        t = std::floor(loc.Y);
        b = std::ceil(loc.Y + loc.Height);
        r = std::ceil(loc.X + loc.Width);
    }
};

// a screen is 2048x2048.
// it's made up of node_t's, which are 64x64 each.
// soon it will have 33x33 nodes instead of 32x32, so that there's some room for shifting the offset for performance
struct screen_t
{
    std::array<node_t, 1024> nodes;

    screen_t()
    {
    }

    void query(std::vector<BaseRef_t>& out, const rect_internal& rect)
    {
        for(const AugLoc_t& loc : rect)
        {
            for(AugBlockRef_t b : nodes[loc.x * 32 + loc.y])
            {
                // only want blocks that are new on all continued axes
                if((b.cont_axes & loc.cont_axes) == CONT_NONE)
                    out.push_back(b.block);
            }
        }
    }

    void insert(BlockRef_t b, const rect_internal& rect)
    {
        for(const AugLoc_t& loc : rect)
            nodes[loc.x * 32 + loc.y].insert({b, loc.cont_axes});
    }

    void erase(BlockRef_t b, const rect_internal& rect)
    {
        for(const AugLoc_t& loc : rect)
            nodes[loc.x * 32 + loc.y].erase(b);
    }
};

// it's a bunch of stacks of screens, which are 2048x2048.
// will be reallocated as needed
struct table_t
{
    std::vector<std::vector<screen_t*>> columns;
    std::vector<int32_t> col_first_row_index;
    std::unordered_map<BlockRef_t, rect_external> member_rects;
    int32_t first_col_index;

    void query(std::vector<BaseRef_t>& out, const rect_external& rect)
    {
        if(columns.size() == 0)
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
                inner_rect.cont_axes |= CONT_Y;
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

    void insert(BlockRef_t b)
    {
        rect_external rect(b->LocationInLayer);
        member_rects[b] = rect;
        insert(b, rect);
    }

    void insert(BlockRef_t b, const rect_external& rect)
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
            columns.emplace_back();
            col_first_row_index.resize(1);
            first_col_index = lcol;
        }

        if(lcol < first_col_index)
        {
            int to_add = first_col_index - lcol;
            for(int i = 0; i < to_add; i++)
                columns.emplace_back();

            std::rotate(columns.rbegin(), columns.rbegin() + to_add, columns.rend());

            col_first_row_index.resize(columns.size() + to_add);
            std::rotate(col_first_row_index.rbegin(), col_first_row_index.rbegin() + to_add, col_first_row_index.rend());

            first_col_index = lcol;
        }

        if(rcol > first_col_index + (int)columns.size())
        {
            for(int i = first_col_index + (int)columns.size(); i < rcol; i++)
                columns.emplace_back();

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

    void erase(BlockRef_t b)
    {
        auto it = member_rects.find(b);
        if(it == member_rects.end())
            return;

        erase(b, it->second);
        member_rects.erase(it);
    }

    void erase(BlockRef_t b, const rect_external& rect)
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

    void update(BlockRef_t b)
    {
        auto it = member_rects.find(b);
        if(it != member_rects.end())
        {
            erase(b, it->second);
        }

        insert(b);
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

/// UPDATED TO HERE

#if 0
template<typename t>
class BasicAllocator
{
    std::vector<t> alloc_instances;
    std::vector<t*> free_instances;
    size_t used_instances = 0;

    t* alloc()
    {
        t* ret;

        if(!free_instances.empty())
        {
            ret = free_instances[free_instances.size() - 1];
            free_instances.pop_back();
        }
        else if(used_instances < alloc_instances.size())
        {
            ret = alloc_instances[used_instances];
            used_instances++;
        }
        else
        {
            alloc_instances.emplace_back();
            ret = alloc_instances[alloc_instances.size() - 1];
            used_instances++;
        }
        return ret;
    }

    void free(t* instance)
    {
        free_instances.push_back(instance);
    }

    void clear()
    {
        free_instances.clear();
        used_instances = 0;
    }
};
#endif // #if 0

#endif // #ifndef BLOCK_TABLE_H
