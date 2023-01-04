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
#ifndef EDITOR_CUSTOM_H
#define EDITOR_CUSTOM_H

#include <vector>
#include <array>

#include "globals.h"

namespace EditorCustom
{

struct ItemType_t
{
    enum slope_t
    {
        no_slope = 0,
        slope_1 = 1,
        slope_2 = 2,
        slope_4 = 3
    };

    int type : 11;
    int group : 4;
    bool temp_flag : 1;

    // uses numpad coordinates
    bool has_1 : 1;
    bool has_2 : 1;
    bool has_3 : 1;
    bool has_4 : 1;
    bool has_6 : 1;
    bool has_7 : 1;
    bool has_8 : 1;
    bool has_9 : 1;

    slope_t slope : 2;
    unsigned width : 3;
    unsigned height : 3;

    ItemType_t(int type = 0, const char* sides = nullptr, slope_t slope = no_slope, int width = 1, int height = 1, int special = 0);

    void set_adj(const char* sides);
};

struct LayoutPod_t
{
    std::vector<int> types;
    int x;
    int y;
    int rows;
    int cols;

    LayoutPod_t();
    LayoutPod_t(int rows, int cols);
    void resize(int rows, int cols);
    void compact();
};

class ItemFamily
{
public:
    // user-supplied
    std::string name;

    std::vector<ItemType_t> types;
    std::vector<LayoutPod_t> req_layout_pods;

    // automatically generated
    std::vector<LayoutPod_t> temp_layout_pods;
    LayoutPod_t layout_pod;

    int8_t page = 0;
    int8_t X = 0;
    int8_t Y = 0;

    // user-supplied
    int16_t icon = 0;
    int8_t category = 0;
    bool is_misc = true;


    void make_layout_pods();
    bool make_layout(int layout_width);
};

struct ItemPage_t
{
    std::vector<ItemFamily*>::iterator begin;
    std::vector<ItemFamily*>::iterator end;

    int16_t icon;
    int8_t category;
};

struct ListItemFamily_t
{
    std::string name;
    int sort_index = 0;

    std::vector<int16_t> indices;
    std::vector<std::string> names;
};

struct ItemList_t
{
    std::vector<int16_t> indices;
    std::vector<std::string> names;

    inline void clear()
    {
        indices.clear();
        names.clear();
    }

    void make_layout(int rows);
};

void Load();

constexpr uint8_t PAGE_NONE = 255;
constexpr uint8_t FAMILY_NONE = 255;

extern std::vector<ItemFamily> block_families;
extern std::vector<ItemPage_t> block_pages;
extern std::array<uint8_t, maxBlockType> block_family_by_type;


extern std::vector<ItemFamily> bgo_families;
extern std::vector<ItemPage_t> bgo_pages;
extern std::array<uint8_t, maxBackgroundType> bgo_family_by_type;

extern std::vector<ItemFamily> npc_families;
extern std::vector<ItemPage_t> npc_pages;
extern std::array<uint8_t, maxNPCType> npc_family_by_type;

extern std::vector<ItemFamily> tile_families;
extern std::vector<ItemPage_t> tile_pages;
extern std::array<uint8_t, maxTileType> tile_family_by_type;


extern ItemList_t sound_list;
extern ItemList_t music_list;
extern ItemList_t wmusic_list;
extern ItemList_t bg2_list;

} // namespace EditorCustom

#endif // EDITOR_CUSTOM_H
