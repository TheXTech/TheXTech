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

#include "globals.h"

struct BlockRef_t
{
    int16_t index;

    // conversions to and from underlying integer type
    BlockRef_t(int16_t index): index(index) {}
    operator int16_t() const { return index; }

    Block_t& operator*() const { return Block[index]; }
    Block_t* operator->() const { return &Block[index]; }
};

template<size_t node_size>
struct block_node_t
{
    block_node_t<node_size>* next;
    int16_t index[node_size];
    uint16_t filled;

    inline block_node_t()
    {
        next = nullptr;
        filled = 0;
    }

    inline ~block_node_t()
    {
        if(next)
            delete next;
    }

    struct iterator
    {
        block_node_t<node_size>* parent;
        uint16_t i;

        inline void check_linkage()
        {
            if(this->parent && this->i == this->parent->filled)
            {
                this->parent = this->parent->next;
                this->i = 0;
            }
        }
        inline iterator(block_node_t<node_size>* parent, size_t i): parent(parent), i(i)
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
        inline int16_t operator*() const
        {
            return this->parent->index[this->i];
        }
    };

    inline iterator begin()
    {
        return iterator(this, 0);
    }

    constexpr inline iterator end() const
    {
        return iterator(nullptr, 0);
    }

    inline void insert(int16_t i)
    {
        if(this->filled != node_size)
        {
            this->index[this->filled] = i;
            this->filled++;
        }
        else
        {
            if(!this->next)
                this->next = new block_node_t<node_size>;
            this->next->insert(i);
        }
    }

    inline void erase(iterator& it)
    {
        it.parent->filled--;
        it.parent->index[it.i] = it.parent->index[it.parent->filled];
    }

    inline void erase(int16_t i)
    {
        iterator it = this->begin();
        while(it != this->end())
        {
            if(*it == i)
                break;
            ++it;
        }
        if(it != this->end())
            this->erase(it);
    }
};

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

struct BlockTable
{
    static constexpr int32_t max_size = 320000.;
    static constexpr int32_t zone_size = 32768;
    static constexpr int32_t screen_size = 1024;
    static constexpr int32_t area_size = 64;

    static constexpr size_t num_zones_per_sign = ((max_size - 1) / zone_size) + 1;
    static constexpr size_t num_zones = num_zones_per_sign * 2;
    static constexpr size_t num_screens = zone_size / screen_size;
    static constexpr size_t num_areas = screen_size / area_size;
    static constexpr size_t total_num_areas = num_zones * num_screens * num_areas;

    using Area = std::set<BlockRef_t>;

    struct Screen
    {
        std::array<Area*, num_areas * num_areas> areas{};
    };

    struct Zone
    {
        std::array<Screen*, num_screens * num_screens> screens{};
    };

    std::array<Zone*, num_zones * num_zones> zones{};

    struct loc_1d
    {
        uint16_t i;
        constexpr uint16_t zone()
        {
            return i / (num_screens*num_areas);
        }
        constexpr uint16_t screen()
        {
            return (i / num_areas) % num_screens;
        }
        constexpr uint16_t area()
        {
            return i % num_areas;
        }
        inline bool operator!=(const loc_1d& other) const
        {
            return this->i != other.i;
        }
    };

    struct loc_2d
    {
        loc_1d x;
        loc_1d y;

        inline bool operator!=(const loc_2d& other) const
        {
            return this->x != other.x || this->y != other.y;
        }
    };

    struct rect_2d
    {
        loc_2d tl;
        loc_2d br;
        bool has_trash;

        class iterator
        {
        private:
            const rect_2d& parent;
            loc_2d cur_loc;
        public:
            inline iterator(const rect_2d& parent, loc_2d cur_loc): parent(parent), cur_loc(cur_loc) {}
            inline iterator operator++()
            {
                this->cur_loc.x.i ++;
                if(this->cur_loc.x.i == this->parent.br.x.i)
                {
                    this->cur_loc.y.i ++;
                    if(this->cur_loc.y.i != this->parent.br.y.i)
                        this->cur_loc.x.i = this->parent.tl.x.i;
                }
                return *this;
            }
            inline bool operator!=(const iterator& other) const
            {
                return this->cur_loc != other.cur_loc;
            }
            inline const loc_2d& operator*() const
            {
                return this->cur_loc;
            }
        };

        inline iterator begin() const
        {
            return iterator(*this, this->tl);
        }

        inline iterator end() const
        {
            return iterator(*this, this->br);
        }
    };

    std::set<BlockRef_t> members;
    std::array<rect_2d, 30000> member_rects;

    std::set<BlockRef_t> trash_bin;

    BlockTable()
    {
        // table.resize(total_num_areas*total_num_areas);
    }

    static rect_2d loc_to_rect(const Location_t& loc)
    {
        int32_t left = loc.X;
        int32_t top = loc.Y;
        if(left <= 0)
            left -= 1;
        if(top <= 0)
            top -= 1;
        int32_t right = left + (int32_t)loc.Width + 2;
        int32_t bottom = top + (int32_t)loc.Height + 2;

        int32_t x_area_start = (left + max_size) / area_size;
        int32_t x_area_limit = ((right + max_size - 1) / area_size) + 1;

        int32_t y_area_start = (top + max_size) / area_size;
        int32_t y_area_limit = ((bottom + max_size - 1) / area_size) + 1;

        bool has_trash = false;
        if(x_area_start < 0)
        {
            has_trash = true;
            x_area_start = 0;
        }
        if(y_area_start < 0)
        {
            has_trash = true;
            y_area_start = 0;
        }
        if(x_area_limit > (int32_t)total_num_areas)
        {
            has_trash = true;
            x_area_limit = total_num_areas;
        }
        if(y_area_limit > (int32_t)total_num_areas)
        {
            has_trash = true;
            y_area_limit = total_num_areas;
        }

        rect_2d ret;
        ret.tl.x.i = x_area_start;
        ret.br.x.i = x_area_limit;
        ret.tl.y.i = y_area_start;
        ret.br.y.i = y_area_limit;
        ret.has_trash = has_trash;

        return ret;
    }

    void insert(BlockRef_t member)
    {
        this->members.insert(member);
        const rect_2d& rect = member_rects[member] = loc_to_rect(member->Location);

        for(const loc_2d& loc : rect)
        {
            Zone*& z = this->zones[loc.y.zone() * num_zones + loc.x.zone()];
            if(!z)
                z = new Zone;
            Screen*& s = z->screens[loc.y.screen() * num_screens + loc.x.screen()];
            if(!s)
                s = new Screen;
            Area*& a = s->areas[loc.y.area() * num_areas + loc.x.area()];
            if(!a)
                a = new Area;
            a->insert(member);
        }

        if(rect.has_trash)
            this->trash_bin.insert(member);
    }

    void hard_erase(BlockRef_t member)
    {
        this->members.erase(member);
        const rect_2d& rect = member_rects[member];

        for(const loc_2d& loc : rect)
        {
            Zone*& z = this->zones[loc.y.zone() * num_zones + loc.x.zone()];
            if(!z)
                continue;
            Screen*& s = z->screens[loc.y.screen() * num_screens + loc.x.screen()];
            if(!s)
                continue;
            Area*& a = s->areas[loc.y.area() * num_areas + loc.x.area()];
            if(!a)
                continue;
            a->erase(member);
        }

        if(rect.has_trash)
            this->trash_bin.erase(member);
    }

    void erase(BlockRef_t member)
    {
        if(this->members.find(member) != this->members.end())
            this->hard_erase(member);
    }

    void update(BlockRef_t member)
    {
        this->erase(member);
        this->insert(member);
    }

    void clear()
    {
        for(BlockRef_t member : this->members)
        {
            const rect_2d& rect = member_rects[member];

            for(const loc_2d& loc : rect)
            {
                Zone*& z = this->zones[loc.y.zone() * num_zones + loc.x.zone()];
                // theoretically, should be able to safely remove these checks
                if(!z)
                    continue;
                Screen*& s = z->screens[loc.y.screen() * num_screens + loc.x.screen()];
                if(!s)
                    continue;
                Area*& a = s->areas[loc.y.area() * num_areas + loc.x.area()];
                if(!a)
                    continue;
                a->erase(member);
            }

            if(rect.has_trash)
                this->trash_bin.erase(member);
        }

        this->members.clear();
    }

    void query(const Location_t& loc, std::set<BlockRef_t>& out)
    {
        const rect_2d rect = loc_to_rect(loc);

        loc_2d l = rect.tl;
        while(true)
        {
            if(l.x.i >= rect.br.x.i)
            {
                l.y.i += 1;
                if(l.y.i == rect.br.y.i)
                    break;
                l.x.i = rect.tl.x.i;
            }
            Zone*& z = this->zones[l.y.zone() * num_zones + l.x.zone()];
            if(!z)
            {
                l.x.i = (l.x.zone() + 1) * num_screens * num_areas;
                continue;
            }
            Screen*& s = z->screens[l.y.screen() * num_screens + l.x.screen()];
            if(!s)
            {
                l.x.i = (l.x.zone() * num_screens + l.x.screen() + 1) * num_areas;
                continue;
            }
            Area*& a = s->areas[l.y.area() * num_areas + l.x.area()];
            if(!a)
            {
                l.x.i += 1;
                continue;
            }

            for(BlockRef_t member : *a)
                out.insert(member);

            l.x.i += 1;
        }

        if(rect.has_trash)
        {
            for(BlockRef_t member : this->trash_bin)
                out.insert(member);
        }
    }
};

#endif // #ifndef BLOCK_TABLE_H