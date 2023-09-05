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

#include <Logger/logger.h>
#include <IniProcessor/ini_processing.h>

#include <fmt_format_ne.h>

#include "sdl_proxy/sdl_stdinc.h"
#include "sdl_proxy/sdl_assert.h"

#include "main/translate.h"

#include "editor/editor_custom.h"

#include "sound.h"

namespace EditorCustom
{

static std::vector<ItemFamily*> s_ordered_block_families;
std::vector<ItemFamily> block_families;
std::array<uint8_t, maxBlockType> block_family_by_type;
std::vector<ItemPage_t> block_pages;


static std::vector<ItemFamily*> s_ordered_bgo_families;
std::vector<ItemFamily> bgo_families;
std::array<uint8_t, maxBackgroundType> bgo_family_by_type;
std::vector<ItemPage_t> bgo_pages;


static std::vector<ItemFamily*> s_ordered_npc_families;
std::vector<ItemFamily> npc_families;
std::array<uint8_t, maxNPCType> npc_family_by_type;
std::vector<ItemPage_t> npc_pages;


static std::vector<ItemFamily*> s_ordered_tile_families;
std::vector<ItemFamily> tile_families;
std::array<uint8_t, maxTileType> tile_family_by_type;
std::vector<ItemPage_t> tile_pages;


ItemList_t sound_list;
ItemList_t music_list;
ItemList_t wmusic_list;
ItemList_t bg2_list;

std::vector<std::string> list_level_exit_names(10);

static constexpr uint8_t LOADED_ALL = 7;
uint8_t loaded = 0;


// implementation for ItemType_t
ItemType_t::ItemType_t(int type, const char* sides, slope_t slope, int width, int height, int group)
    : type(type), group(group), temp_flag(false), slope(slope), width(width), height(height)
{
    set_adj(sides);
}

void ItemType_t::set_adj(const char* sides)
{
    has_1 = false;
    has_2 = false;
    has_3 = false;
    has_4 = false;
    has_6 = false;
    has_7 = false;
    has_8 = false;
    has_9 = false;

    if(!sides)
        return;

    for(const char* c = sides; *c != '\0'; c++)
    {
        switch(*c)
        {
        case '1':
            has_1 = true;
            break;
        case '2':
            has_2 = true;
            break;
        case '3':
            has_3 = true;
            break;
        case '4':
            has_4 = true;
            break;
        case '6':
            has_6 = true;
            break;
        case '7':
            has_7 = true;
            break;
        case '8':
            has_8 = true;
            break;
        case '9':
            has_9 = true;
            break;
        default:
            break;
        }
    }
}


// implementation for LayoutPod_t
LayoutPod_t::LayoutPod_t()
{
    x = 0;
    y = 0;
    rows = 0;
    cols = 0;
}

LayoutPod_t::LayoutPod_t(int rows, int cols)
{
    x = 0;
    y = 0;
    resize(rows, cols);
}

void LayoutPod_t::resize(int _rows, int _cols)
{
    types.clear();
    types.resize(_rows * _cols);
    rows = _rows;
    cols = _cols;
}

void LayoutPod_t::compact()
{
    // the cheap one first
    for(int row = rows - 1; row >= 0; row--)
    {
        bool empty = true;
        for(int col = 0; col < cols; col++)
        {
            if(types[row * cols + col])
            {
                empty = false;
                break;
            }
        }

        if(empty)
        {
            types.erase(types.begin() + row * cols, types.begin() + (row + 1) * cols);
            rows--;
        }
    }

    // the expensive one second
    for(int col = cols - 1; col >= 0; col--)
    {
        bool empty = true;
        for(int row = 0; row < rows; row++)
        {
            if(types[row * cols + col])
            {
                empty = false;
                break;
            }
        }

        if(empty)
        {
            for(int row = 0; row < rows; row++)
            {
                for(int col2 = 0; col2 < cols; col2++)
                {
                    // before col-erase, shift everything back [row] elements
                    if(col2 < col)
                        types[row * (cols - 1) + col2] = types[row * cols + col2];
                    // after, shift everything back [row + 1] elements
                    else if(col2 > col)
                        types[row * (cols - 1) + col2 - 1] = types[row * cols + col2];
                }
            }

            cols--;
            types.resize(rows * cols);
        }
    }
}

// implementation for ItemFamily
void ItemFamily::make_layout_pods()
{
    if(is_misc && req_layout_pods.empty())
        return;

    temp_layout_pods.clear();

    for(const LayoutPod_t& pod : req_layout_pods)
    {
        for(int type : pod.types)
        {
            for(ItemType_t& t2 : types)
            {
                if(t2.type == type)
                    t2.temp_flag = true;
            }
        }

        temp_layout_pods.push_back(pod);
    }

    LayoutPod_t current_pod_external;
    LayoutPod_t current_pod_internal;
    LayoutPod_t current_pod_hstrip;
    LayoutPod_t current_pod_vstrip;

    LayoutPod_t trash_pod;

    bool types_left = true;

    // default-initializes to the standard (no group, no slope, width/height 1)
    ItemType_t current_exemplar;

    while(types_left)
    {
        types_left = false;

        if(current_exemplar.slope == ItemType_t::no_slope)
            current_pod_external.resize(3, 3);
        else
            current_pod_external.resize(2, 2);

        current_pod_internal.resize(2, 2);
        current_pod_hstrip.resize(1, 3);
        current_pod_vstrip.resize(3, 1);

        for(ItemType_t& t : types)
        {
            if(t.group != current_exemplar.group || t.slope != current_exemplar.slope
                || t.width != current_exemplar.width || t.height != current_exemplar.height)
            {
                continue;
            }

            if(t.temp_flag)
                continue;

            t.temp_flag = true;
            for(ItemType_t& t2 : types)
            {
                if(t2.type == t.type)
                    t2.temp_flag = true;
            }

            int* pos = nullptr;

            // 4 tiles in the internal pod
            // bottomright
            if(t.has_1 && t.has_2 && !t.has_3 && t.has_4 && t.has_6 && t.has_7 && t.has_8 && t.has_9)
                pos = &current_pod_internal.types[3];
            // bottomleft
            else if(!t.has_1 && t.has_2 && t.has_3 && t.has_4 && t.has_6 && t.has_7 && t.has_8 && t.has_9)
                pos = &current_pod_internal.types[2];
            // topright
            else if(t.has_1 && t.has_2 && t.has_3 && t.has_4 && t.has_6 && t.has_7 && t.has_8 && !t.has_9)
                pos = &current_pod_internal.types[1];
            // topleft
            else if(t.has_1 && t.has_2 && t.has_3 && t.has_4 && t.has_6 && !t.has_7 && t.has_8 && t.has_9)
                pos = &current_pod_internal.types[0];

            if(!pos && current_exemplar.slope == ItemType_t::no_slope)
            {
                // 9 tiles in the external pod

                // topleft
                if(!t.has_1 && t.has_2 && t.has_3 && !t.has_4 && t.has_6 && !t.has_7 && !t.has_8 && !t.has_9)
                    pos = &current_pod_external.types[0];
                // top
                else if(t.has_1 && t.has_2 && t.has_3 && t.has_4 && t.has_6 && !t.has_7 && !t.has_8 && !t.has_9)
                    pos = &current_pod_external.types[1];
                // topright
                else if(t.has_1 && t.has_2 && !t.has_3 && t.has_4 && !t.has_6 && !t.has_7 && !t.has_8 && !t.has_9)
                    pos = &current_pod_external.types[2];
                // midleft
                else if(!t.has_1 && t.has_2 && t.has_3 && !t.has_4 && t.has_6 && !t.has_7 && t.has_8 && t.has_9)
                    pos = &current_pod_external.types[3];
                // mid
                else if(t.has_1 && t.has_2 && t.has_3 && t.has_4 && t.has_6 && t.has_7 && t.has_8 && t.has_9)
                    pos = &current_pod_external.types[4];
                // midright
                else if(t.has_1 && t.has_2 && !t.has_3 && t.has_4 && !t.has_6 && t.has_7 && t.has_8 && !t.has_9)
                    pos = &current_pod_external.types[5];
                // bottomleft
                else if(!t.has_1 && !t.has_2 && !t.has_3 && !t.has_4 && t.has_6 && !t.has_7 && t.has_8 && t.has_9)
                    pos = &current_pod_external.types[6];
                // bottom
                else if(!t.has_1 && !t.has_2 && !t.has_3 && t.has_4 && t.has_6 && t.has_7 && t.has_8 && t.has_9)
                    pos = &current_pod_external.types[7];
                // bottomright
                else if(!t.has_1 && !t.has_2 && !t.has_3 && t.has_4 && !t.has_6 && t.has_7 && t.has_8 && !t.has_9)
                    pos = &current_pod_external.types[8];

                // 3 tiles in the horizontal strip pod
                // left
                else if(!t.has_1 && !t.has_2 && !t.has_3 && !t.has_4 && t.has_6 && !t.has_7 && !t.has_8 && !t.has_9)
                    pos = &current_pod_hstrip.types[0];
                // center
                else if(!t.has_1 && !t.has_2 && !t.has_3 && t.has_4 && t.has_6 && !t.has_7 && !t.has_8 && !t.has_9)
                    pos = &current_pod_hstrip.types[1];
                // right
                else if(!t.has_1 && !t.has_2 && !t.has_3 && t.has_4 && !t.has_6 && !t.has_7 && !t.has_8 && !t.has_9)
                    pos = &current_pod_hstrip.types[2];

                // 3 tiles in the vertical strip pod
                // top
                else if(!t.has_1 && t.has_2 && !t.has_3 && !t.has_4 && !t.has_6 && !t.has_7 && !t.has_8 && !t.has_9)
                    pos = &current_pod_vstrip.types[0];
                // center
                else if(!t.has_1 && t.has_2 && !t.has_3 && !t.has_4 && !t.has_6 && !t.has_7 && t.has_8 && !t.has_9)
                    pos = &current_pod_vstrip.types[1];
                // bottom
                else if(!t.has_1 && !t.has_2 && !t.has_3 && !t.has_4 && !t.has_6 && !t.has_7 && t.has_8 && !t.has_9)
                    pos = &current_pod_vstrip.types[2];
            }
            else if(!pos)
            {
                // topleft
                if(t.has_2 && t.has_3 && !t.has_4 && t.has_6 && !t.has_7 && !t.has_8)
                    pos = &current_pod_external.types[0];
                // topright
                else if(t.has_1 && t.has_2 && t.has_4 && !t.has_6 && !t.has_8 && !t.has_9)
                    pos = &current_pod_external.types[1];
                // bottomleft
                else if(!t.has_1 && !t.has_2 && !t.has_4 && t.has_6 && t.has_8 && t.has_9)
                    pos = &current_pod_external.types[2];
                // bottomright
                else if(!t.has_2 && !t.has_3 && t.has_4 && !t.has_6 && t.has_7 && t.has_8)
                    pos = &current_pod_external.types[3];
            }

            if(pos && *pos == 0)
                *pos = t.type;
            else
                trash_pod.types.push_back(t.type);
        }

        // find a new exemplar (can't do this during, since temp_flag is set for some during)
        for(ItemType_t& t : types)
        {
            if(t.group != current_exemplar.group || t.slope != current_exemplar.slope
                || t.width != current_exemplar.width || t.height != current_exemplar.height)
            {
                if(!t.temp_flag)
                {
                    current_exemplar = t;
                    types_left = true;
                    break;
                }
            }
        }

        current_pod_external.compact();
        if(!current_pod_external.types.empty())
            temp_layout_pods.push_back(current_pod_external);

        current_pod_internal.compact();
        if(!current_pod_internal.types.empty())
            temp_layout_pods.push_back(current_pod_internal);

        current_pod_hstrip.compact();
        if(!current_pod_hstrip.types.empty())
            temp_layout_pods.push_back(current_pod_hstrip);

        current_pod_vstrip.compact();
        if(!current_pod_vstrip.types.empty())
            temp_layout_pods.push_back(current_pod_vstrip);
    }

    trash_pod.rows = 1;
    trash_pod.cols = (int)trash_pod.types.size();

    // order the pods by size
    std::sort(temp_layout_pods.begin(), temp_layout_pods.end(),
    [](const LayoutPod_t& a, const LayoutPod_t& b)
    {
        return a.rows > b.rows || (a.rows == b.rows && a.cols > b.cols);
    });

    if(!trash_pod.types.empty())
    {
        for(int t : trash_pod.types)
        {
            temp_layout_pods.push_back(LayoutPod_t(1, 1));
            temp_layout_pods[temp_layout_pods.size() - 1].types[0] = t;
        }
    }

    for(ItemType_t& t : types)
        t.temp_flag = false;
}

bool ItemFamily::make_layout(int layout_width)
{
    if(temp_layout_pods.empty())
    {
        layout_pod.resize((int)types.size() / layout_width + 1, layout_width);

        layout_pod.types.resize(0);
        for(ItemType_t t : types)
            layout_pod.types.push_back(t.type);
        layout_pod.types.resize((types.size() / layout_width + 1) * layout_width);

        layout_pod.compact();

        return true;
    }

    // make a layout using these pods
    // place them where they fit
    layout_pod.resize(3, layout_width);

    for(LayoutPod_t& pod : temp_layout_pods)
    {
        int x, y;

        bool okay = false;

        int prows = pod.rows;
        int pcols = pod.cols;

        // rotate if needed
        if(layout_width < pcols && prows == 1)
        {
            prows = pcols;
            pcols = 1;
        }


        for(y = 0; y < 10; y++)
        {
            for(x = 0; x + pcols <= layout_width; x++)
            {
                if(y + prows > layout_pod.rows)
                {
                    layout_pod.rows = y + prows;
                    layout_pod.types.resize(layout_pod.rows * layout_pod.cols);
                }

                okay = true;
                for(int y2 = 0; y2 < prows; y2++)
                {
                    for(int x2 = 0; x2 < pcols; x2++)
                    {
                        if(layout_pod.types[(y + y2) * layout_pod.cols + (x + x2)])
                        {
                            okay = false;
                            break;
                        }
                    }
                    if(!okay)
                        break;
                }

                if(okay)
                    break;
            }
            if(okay)
                break;
        }

        if(!okay)
        {
            layout_pod.resize(types.size() / layout_width + 1, layout_width);
            return false;
        }

        for(int y2 = 0; y2 < prows; y2++)
        {
            for(int x2 = 0; x2 < pcols; x2++)
                layout_pod.types[(y + y2) * layout_pod.cols + (x + x2)] = pod.types[y2 * pcols + x2];
        }
    }

    layout_pod.compact();

    return true;
}


// implementation for ItemList_t

void ItemList_t::make_layout(int rows)
{
    // first, remove any existing padding
    for(int i = (int)indices.size() - 1; i >= 0; i--)
    {
        if(indices[i] == -1 && names[i].empty())
        {
            indices.erase(indices.begin() + i);
            names.erase(names.begin() + i);
        }
    }

    // now, add any padding needed
    for(size_t i = 0; i < indices.size(); i++)
    {
        if(indices[i] == -1 && !names[i].empty() && (i + 1) % rows == 0)
        {
            indices.insert(indices.begin() + i, -1);
            names.insert(names.begin() + i, std::string());
        }
    }
}

void ItemList_t::make_translation(XTechTranslate& translate, const char* group, const char* prefix, const std::vector<std::pair<ListItemFamily_t, std::string>>& families)
{
    int header = -1;

    for(size_t i = 0; i < indices.size(); i++)
    {
        if(indices[i] != -1)
        {
            if(header < 0)
                translate.m_assetsMap.insert({fmt::format_ne("editor.{0}.auto.{1}{2}", group, prefix, indices[i]), &names[i]});
            else if(header < (int)families.size())
                translate.m_assetsMap.insert({fmt::format_ne("editor.{0}.{1}.{2}{3}", group, families[header].second, prefix, indices[i]), &names[i]});
            else
                translate.m_assetsMap.insert({fmt::format_ne("editor.{0}.fam{1}.{2}{3}", group, header, prefix, indices[i]), &names[i]});
        }
        else if(!names[i].empty())
        {
            header++;

            if(header < (int)families.size())
                translate.m_assetsMap.insert({fmt::format_ne("editor.{0}.{1}.header", group, families[header].second), &names[i]});
            else
                translate.m_assetsMap.insert({fmt::format_ne("editor.{0}.fam{1}.header", group, header), &names[i]});
        }
    }
}


// general-purpose layout functions

std::vector<ItemFamily*>::iterator make_page(std::vector<ItemFamily*>::iterator begin, std::vector<ItemFamily*>::iterator end,
    int width, int height)
{
    std::sort(begin, end,
    [](const ItemFamily* a, const ItemFamily* b)
    {
        return a->category < b->category
            || (a->category == b->category &&
                a->types.size() > b->types.size());
    });

    int page_category = -1;

    // row - half row, because of titles
    int start_section_row = 0;
    int current_row = 0;
    int main_width = width;

    std::vector<std::array<int, 4>> lost_margins;

    auto unplaced = begin;
    auto orig_end = end;

    while(unplaced != end)
    {
        ItemFamily& family = **unplaced;

        if(page_category == -1)
            page_category = family.category;
        else if(family.category != page_category)
            break;

        bool okay = false;

        if(current_row != start_section_row)
        {
            // try to fill a margin, if it exists
            okay = family.make_layout(width - main_width - 1);
            int r_height = family.layout_pod.rows * 2 + 1;

            if(okay && r_height <= current_row - start_section_row)
            {
                int min_width = family.layout_pod.cols;
                int min_height = family.layout_pod.rows;
                while(min_width > 1)
                {
                    min_width--;
                    okay = family.make_layout(min_width);
                    if(!okay || family.layout_pod.rows > min_height)
                    {
                        min_width++;
                        okay = family.make_layout(min_width);
                        break;
                    }
                }
                r_height = family.layout_pod.rows * 2 + 1;

                family.X = main_width + 1;
                family.Y = start_section_row;

                int margin_size_tall = (width - main_width - 1) * (current_row - start_section_row - r_height);
                int margin_size_wide = (width - main_width - family.layout_pod.cols - 2) * (current_row - start_section_row);

                // pick the bigger continued margin
                if(margin_size_tall >= margin_size_wide)
                {
                    lost_margins.push_back({start_section_row, current_row, main_width + family.layout_pod.cols + 2, width});
                    start_section_row += r_height;
                }
                else
                {
                    lost_margins.push_back({start_section_row + r_height, current_row, main_width + 1, main_width + 1 + family.layout_pod.cols});
                    main_width += family.layout_pod.cols + 1;
                }

                ++unplaced;
                continue;
            }

            // we'll be below the margin

            // pick whether to fill the row (use width) or keep the margin (use main_width)
            family.make_layout(width);
            int height_full = family.layout_pod.rows * 2 + 1;

            okay = family.make_layout(main_width);
            int height_main = family.layout_pod.rows * 2 + 1;

            int margin_size = (width - main_width - 1) * (current_row - start_section_row);

            if(!okay || height_main * main_width > height_full * width + margin_size)
            {
                // discard the margin
                // (added to lost margins later)
                okay = family.make_layout(width);
            }
        }
        else
        {
            okay = family.make_layout(width);
        }

        // figure out the minimum height and width at which it gets this height
        int min_height = family.layout_pod.rows;

        // skip if we can't fit it
        if((!okay || current_row + (min_height * 2 + 1) > height * 2) && current_row != 0)
        {
            std::rotate(unplaced, unplaced + 1, end);
            end--;
            continue;
        }
        // if there isn't enough room for another one below, allow it to be taller
        else if(current_row + (min_height * 2 + 1) + 4 == height * 2)
        {
            min_height += 2;
        }
        // if there isn't enough room for another one below, allow it to be taller
        else if(current_row + (min_height * 2 + 1) + 3 == height * 2)
        {
            min_height++;
        }
        // if there isn't enough room for another one below, allow it to be taller
        else if(current_row + (min_height * 2 + 1) + 2 == height * 2)
        {
            min_height++;
        }

        int min_width = family.layout_pod.cols;
        while(min_width > 1)
        {
            min_width--;
            okay = family.make_layout(min_width);
            if(!okay || family.layout_pod.rows > min_height)
            {
                min_width++;
                okay = family.make_layout(min_width);
                break;
            }
        }

        min_height = family.layout_pod.rows;

        // convert to half-rows, add title
        min_height = min_height * 2 + 1;

        family.X = 0;
        family.Y = current_row;

        // consume the entire rows
        if(min_width >= width - 1)
        {
            if(current_row != start_section_row)
                lost_margins.push_back({start_section_row, current_row, main_width + 1, width});

            current_row += min_height;
            start_section_row = current_row;
            main_width = width;
        }
        // start a margin
        else if(current_row == start_section_row)
        {
            current_row += min_height;
            main_width = min_width;
        }
        // decide what happens to the margin
        else
        {
            // pick the bigger continued margin, or just cancel
            int margin_size_tall = (width - main_width - 1) * (min_height + current_row - start_section_row);
            int margin_size_wide = (width - min_width - 1) * (min_height);

            // just cancel
            // if(margin_size_tall < 4 && margin_size_wide < 4)
            // {
            //     current_row += min_height;
            //     start_section_row = current_row;
            //     main_width = width;
            // }
            // make a wide margin, starting on the current row
            if(margin_size_wide >= margin_size_tall)
            {
                lost_margins.push_back({start_section_row, current_row, main_width + 1, width});

                start_section_row = current_row;
                current_row += min_height;
                main_width = min_width;
            }
            // make a tall margin, keeping start row and width the same
            else
            {
                if(min_width + 1 != main_width)
                    lost_margins.push_back({current_row, current_row + min_height, min_width + 1, main_width});

                current_row += min_height;
            }
        }

        ++unplaced;
    }

    // try to fill lost margins if possible
    if(current_row != start_section_row)
    {
        lost_margins.push_back({start_section_row, height * 2, main_width + 1, width});
    }

    // grow lost margins to screen height
    for(auto& i: lost_margins)
    {
        if(i[1] == current_row)
            i[1] = height * 2;
    }

    // grow any combined margins
    std::sort(lost_margins.begin(), lost_margins.end(),
    [](const std::array<int, 4>& a, const std::array<int, 4>& b)
    {
        return a[2] < b[2];
    });

    for(auto& i : lost_margins)
    {
        for(auto& j : lost_margins)
        {
            if((i[3] == j[2] || i[3] + 1 == j[2]) && i[0] == j[0] && i[1] == j[1])
            {
                i[3] = j[3];
                j[2] = j[3] + 1;
            }
        }
    }

    std::sort(lost_margins.begin(), lost_margins.end(),
    [](const std::array<int, 4>& a, const std::array<int, 4>& b)
    {
        return a[0] < b[0];
    });

    for(auto& i : lost_margins)
    {
        for(auto& j : lost_margins)
        {
            if(i[1] == j[0] && i[2] == j[2] && i[3] == j[3])
            {
                i[1] = j[1];
                j[0] = j[1];
            }
        }
    }

    // remove bad lost margins
    for(size_t x = 0; x < lost_margins.size(); x++)
    {
        const auto& i = lost_margins[x];
        if((i[1] - i[0]) < 3 || (i[3] - i[2]) < 1)
        {
            lost_margins[x] = lost_margins[lost_margins.size() - 1];
            lost_margins.resize(lost_margins.size() - 1);
        }
    }

    // do it!
    end = orig_end;

    std::sort(unplaced, end,
    [](const ItemFamily* a, const ItemFamily* b)
    {
        return a->category < b->category
            || (a->category == b->category &&
                (a->is_misc < b->is_misc
                    || (a->is_misc == b->is_misc && a->types.size() > b->types.size())));
    });

    while(unplaced != end)
    {
        ItemFamily& family = **unplaced;

        if(family.category != page_category)
            break;

        bool placed = false;

        std::sort(lost_margins.begin(), lost_margins.end(),
        [](const std::array<int, 4>& a, const std::array<int, 4>& b)
        {
            return ((a[1] - a[0]) * (a[3] - a[2])) < ((b[1] - b[0]) * (b[3] - b[2]));
        });

        for(size_t x = 0; x < lost_margins.size(); x++)
        {
            auto i = lost_margins[x];
            if((i[1] - i[0]) * (i[3] - i[2]) < (int)family.types.size() * 2)
                continue;

            bool okay = family.make_layout(i[3] - i[2]);

            if(!okay || family.layout_pod.rows * 2 + 1 > i[1] - i[0])
                continue;

            int min_height = family.layout_pod.rows;
            int min_width = family.layout_pod.cols;
            while(min_width > 1)
            {
                min_width--;
                okay = family.make_layout(min_width);
                if(!okay || family.layout_pod.rows > min_height)
                {
                    min_width++;
                    okay = family.make_layout(min_width);
                    break;
                }
            }

            family.X = i[2];
            family.Y = i[0];

            if(family.layout_pod.cols + 1 < i[3] - i[2])
                lost_margins.push_back({i[0], i[1], i[2] + family.layout_pod.cols + 1, i[3]});

            if(family.layout_pod.rows * 2 + 1 < i[1] - i[0])
                lost_margins.push_back({i[0] + family.layout_pod.rows * 2 + 1, i[1], i[2], i[3]});

            lost_margins[x] = lost_margins[lost_margins.size() - 1];
            lost_margins.resize(lost_margins.size() - 1);

            placed = true;
            break;
        }

        if(!placed)
        {
            std::rotate(unplaced, unplaced + 1, end);
            end--;
            continue;
        }

        ++unplaced;
    }

    return unplaced;
}

void make_pages(std::vector<ItemFamily*>& families, std::vector<ItemPage_t>& pages, int width, int height)
{
    pages.clear();

    auto unplaced = families.begin();

    while(unplaced != families.end())
    {
        auto unplaced_new = make_page(unplaced, families.end(), width, height);

        if(unplaced_new == unplaced)
        {
            pLogCritical("Block page layout algorithm failed.");
            break;
        }

        pages.emplace_back();
        ItemPage_t& new_page = pages[pages.size() - 1];
        new_page.begin = unplaced;
        new_page.end = unplaced_new;
        new_page.category = (**unplaced).category;
        new_page.icon = 0;

        for(auto f = new_page.begin; f != new_page.end; f++)
        {
            if((**f).icon)
            {
                new_page.icon = (**f).icon;
                break;
            }
        }

        if(!new_page.icon)
            new_page.icon = (**unplaced).types[0].type;

        for(auto it = new_page.begin; it != new_page.end; ++it)
            (**it).page = (int8_t)pages.size();

        unplaced = unplaced_new;
    }
}


void Load(XTechTranslate* translate)
{
    block_families.clear();
    bgo_families.clear();
    npc_families.clear();
    tile_families.clear();

    sound_list.clear();
    music_list.clear();
    wmusic_list.clear();
    bg2_list.clear();

    loaded = 0;


    pLogDebug("Loading editor.ini...");

    IniProcessing editor(AppPath + "editor.ini");
    IniProcessing sound(AppPath + "sound.ini");
    IniProcessing music(AppPath + "music.ini");

    std::vector<int> temp_ints;
    std::vector<int> temp_layout_pod_indices;
    std::string temp_str;

    // will be added to the global list and then destroyed
    std::vector<std::pair<ListItemFamily_t, std::string>> sound_families;
    std::vector<std::pair<ListItemFamily_t, std::string>> music_families;
    std::vector<std::pair<ListItemFamily_t, std::string>> wmusic_families;
    std::vector<std::pair<ListItemFamily_t, std::string>> bg2_families;

    // needed to form the translations correctly
    std::vector<std::string> block_family_keys;
    std::vector<std::string> bgo_family_keys;
    std::vector<std::string> npc_family_keys;
    std::vector<std::string> tile_family_keys;

    for(const std::string& group : editor.childGroups())
    {
        ItemFamily* f_ptr = nullptr;
        ListItemFamily_t* l_ptr = nullptr;
        const char* prefix = "";
        int prefix_length = 3;
        int max_item_type = 0;
        bool has_slope = false;

        if(SDL_strncasecmp(group.c_str(), "blkfam", 6) == 0)
        {
            block_families.emplace_back();
            f_ptr = &block_families[block_families.size() - 1];
            prefix = "blk";
            max_item_type = maxBlockType;
            has_slope = true;
            block_family_keys.push_back(group.c_str() + prefix_length);
        }
        else if(SDL_strncasecmp(group.c_str(), "bgofam", 6) == 0)
        {
            bgo_families.emplace_back();
            f_ptr = &bgo_families[bgo_families.size() - 1];
            prefix = "bgo";
            max_item_type = maxBackgroundType;
            bgo_family_keys.push_back(group.c_str() + prefix_length);
        }
        else if(SDL_strncasecmp(group.c_str(), "npcfam", 6) == 0)
        {
            npc_families.emplace_back();
            f_ptr = &npc_families[npc_families.size() - 1];
            prefix = "npc";
            max_item_type = maxNPCType;
            npc_family_keys.push_back(group.c_str() + prefix_length);
        }
        else if(SDL_strncasecmp(group.c_str(), "tilefam", 7) == 0)
        {
            tile_families.emplace_back();
            f_ptr = &tile_families[tile_families.size() - 1];
            prefix = "tile";
            prefix_length = 4;
            max_item_type = maxTileType;
            tile_family_keys.push_back(group.c_str() + prefix_length);
        }
        else if(SDL_strncasecmp(group.c_str(), "sndfam", 6) == 0)
        {
            sound_families.emplace_back();
            l_ptr = &sound_families[sound_families.size() - 1].first;
            prefix = "snd";
            sound_families[sound_families.size() - 1].second = group.c_str() + prefix_length;
        }
        else if(SDL_strncasecmp(group.c_str(), "musfam", 6) == 0)
        {
            music_families.emplace_back();
            l_ptr = &music_families[music_families.size() - 1].first;
            prefix = "mus";
            music_families[music_families.size() - 1].second = group.c_str() + prefix_length;
        }
        else if(SDL_strncasecmp(group.c_str(), "wmusfam", 7) == 0)
        {
            wmusic_families.emplace_back();
            l_ptr = &wmusic_families[wmusic_families.size() - 1].first;
            prefix = "wmus";
            prefix_length = 4;
            wmusic_families[wmusic_families.size() - 1].second = group.c_str() + prefix_length;
        }
        else if(SDL_strncasecmp(group.c_str(), "bg2fam", 6) == 0)
        {
            bg2_families.emplace_back();
            l_ptr = &bg2_families[bg2_families.size() - 1].first;
            prefix = "bg2";
            bg2_families[bg2_families.size() - 1].second = group.c_str() + prefix_length;
        }
        else if(SDL_strcasecmp(group.c_str(), "exit-codes") == 0)
        {
            // handled above to ensure we get failsafes
        }
        else
        {
            pLogWarning("Ignoring invalid group %s.", group.c_str());
        }

        // handling for Lists
        if(l_ptr)
        {
            editor.beginGroup(group);

            ListItemFamily_t& f = *l_ptr;

            editor.read("name", f.name, "...");
            editor.read("sort-index", f.sort_index, 0);

            temp_ints.clear();
            editor.read("members", temp_ints, temp_ints);
            for(int i : temp_ints)
            {
                if(i > 0 && i <= 127)
                {
                    f.indices.push_back(i);
                    f.names.push_back(fmt::sprintf_ne("%s %d", prefix, i));
                }
            }

            for(const std::string& key : editor.allKeys())
            {
                if(SDL_strncasecmp(key.c_str(), "name", 4) == 0)
                    continue;

                if(SDL_strncasecmp(key.c_str(), "sort-index", 10) == 0)
                    continue;

                if(SDL_strncasecmp(key.c_str(), "members", 7) == 0)
                    continue;

                if(SDL_strncasecmp(key.c_str(), prefix, prefix_length) != 0)
                {
                    pLogWarning("Ignoring invalid key %s in family %s", key.c_str(), group.c_str());
                    continue;
                }

                const char* startptr = key.c_str() + prefix_length;

                const char* endptr;

                int i = strtol(startptr, const_cast<char**>(&endptr), 0);
                if(endptr == startptr)
                {
                    pLogWarning("Ignoring invalid key %s in family %s", key.c_str(), group.c_str());
                    continue;
                }

                if(i < 0)
                    i = -i;

                if(i < 1 || i > 127)
                {
                    pLogWarning("Ignoring invalid key %s in family %s", key.c_str(), group.c_str());
                    continue;
                }

                if(*endptr == '\0')
                {
                    pLogWarning("Ignoring invalid key %s in family %s", key.c_str(), group.c_str());
                    continue;
                }
                if(*endptr == '-')
                    endptr++;

                if(SDL_strncasecmp(endptr, "name", 4) != 0)
                {
                    pLogWarning("Ignoring invalid key %s in family %s", key.c_str(), group.c_str());
                    continue;
                }

                // find the item
                std::string* str = nullptr;
                for(size_t ind = 0; ind < f.indices.size(); ind++)
                {
                    if(f.indices[ind] == i)
                    {
                        str = &f.names[ind];
                        break;
                    }
                }

                if(!str)
                {
                    f.indices.push_back(i);
                    f.names.push_back(fmt::sprintf_ne("%s %d", prefix, i));
                    str = &f.names[f.names.size() - 1];
                }

                editor.read(key.c_str(), *str, *str);
            }
        }

        // handling for ordinary Families
        if(f_ptr)
        {
            temp_layout_pod_indices.clear();

            editor.beginGroup(group);

            ItemFamily& f = *f_ptr;

            editor.read("name", f.name, "");

            int temp_int;
            editor.read("category", temp_int, 0);
            if(temp_int > -128 && temp_int < 128)
            {
                f.category = temp_int;
            }
            else
            {
                f.category = 0;
                pLogWarning("Ignoring invalid category index %d for family %s (max 127)", temp_int, f.name.c_str());
            }

            editor.read("icon", temp_int, 0);
            if(temp_int >= 0 && temp_int <= max_item_type)
            {
                f.icon = temp_int;
            }
            else
            {
                f.icon = 0;
                pLogWarning("Ignoring invalid icon %d for family %s (max %d)", temp_int, f.name.c_str(), max_item_type);
            }

            f.is_misc = true;

            temp_ints.clear();
            editor.read("members", temp_ints, temp_ints);
            for(int i : temp_ints)
            {
                if(i > 0 && i <= max_item_type)
                    f.types.push_back(ItemType_t(i));
            }

            // first pass for the blocks, second pass to add alternate group and adj values
            for(int alt = 0; alt < 2; alt++)
            {
                for(const std::string& key : editor.allKeys())
                {
                    if(SDL_strncasecmp(key.c_str(), "name", 4) == 0)
                        continue;

                    if(SDL_strncasecmp(key.c_str(), "cate", 4) == 0)
                        continue;

                    if(SDL_strncasecmp(key.c_str(), "memb", 4) == 0)
                        continue;

                    if(SDL_strncasecmp(key.c_str(), "misc", 4) == 0)
                        continue;

                    if(SDL_strncasecmp(key.c_str(), "icon", 4) == 0)
                        continue;

                    if(SDL_strncasecmp(key.c_str(), "behi", 4) == 0)
                        continue;

                    bool pod_mode = false;

                    if(SDL_strncasecmp(key.c_str(), "pod", 3) == 0)
                    {
                        if(alt)
                            continue;
                        else
                            pod_mode = true;
                    }
                    else if(SDL_strncasecmp(key.c_str(), prefix, prefix_length) != 0)
                    {
                        pLogWarning("Ignoring invalid key %s in family %s", key.c_str(), group.c_str());
                        continue;
                    }

                    const char* startptr = key.c_str() + prefix_length;
                    if(pod_mode)
                        startptr = key.c_str() + 3;

                    const char* endptr;

                    int i = strtol(startptr, const_cast<char**>(&endptr), 0);
                    if(endptr == startptr)
                    {
                        pLogWarning("Ignoring invalid key %s in family %s", key.c_str(), group.c_str());
                        continue;
                    }

                    if(i < 0)
                        i = -i;

                    if(!pod_mode && (i < 1 || i > max_item_type))
                        continue;

                    if(*endptr == '\0')
                    {
                        pLogWarning("Ignoring invalid key %s in family %s", key.c_str(), group.c_str());
                        continue;
                    }
                    if(*endptr == '-')
                        endptr++;

                    // handle pods
                    if(pod_mode)
                    {
                        size_t ind;
                        for(ind = 0; ind < temp_layout_pod_indices.size(); ind++)
                        {
                            if(temp_layout_pod_indices[ind] == i)
                                break;
                        }

                        if(ind == temp_layout_pod_indices.size())
                        {
                            temp_layout_pod_indices.push_back(i);
                            f.req_layout_pods.emplace_back();
                        }

                        LayoutPod_t& pod = f.req_layout_pods[ind];

                        if(SDL_strncasecmp(endptr, "mem", 3) == 0)
                        {
                            pod.types.clear();
                            editor.read(key.c_str(), pod.types, pod.types);
                            for(int j : pod.types)
                            {
                                // add to family types if needed
                                if(j > 0 && j <= max_item_type)
                                {
                                    bool found = false;
                                    for(ItemType_t& t_i : f.types)
                                    {
                                        if(t_i.type == j)
                                        {
                                            found = true;
                                            break;
                                        }
                                    }
                                    if(!found)
                                        f.types.push_back(ItemType_t(j));
                                }
                            }

                            if(pod.cols <= 0)
                                pod.cols = 4;

                            pod.rows = (pod.types.size() - 1) / pod.cols + 1;
                            pod.types.resize(pod.rows * pod.cols);
                        }
                        else if(SDL_strncasecmp(endptr, "col", 3) == 0 || SDL_strncasecmp(endptr, "wid", 3) == 0)
                        {
                            int old_cols = pod.cols;

                            editor.read(key.c_str(), pod.cols, pod.cols);

                            if(pod.cols <= 0)
                                pod.cols = old_cols;

                            if(!pod.types.empty() && pod.cols > 0)
                            {
                                pod.rows = pod.types.size() / pod.cols + 1;
                                pod.types.resize(pod.rows * pod.cols);
                            }
                        }
                        else
                        {
                            pLogWarning("Ignoring group %s key %s", group.c_str(), key.c_str());
                            continue;
                        }

                        continue;
                    }

                    // deal with these in the second pass
                    if(SDL_strncasecmp(endptr, "alt", 3) == 0)
                    {
                        if(alt)
                        {
                            endptr += 3;

                            if(*endptr == '\0')
                            {
                                pLogWarning("Ignoring invalid key %s", key.c_str());
                                continue;
                            }
                            if(*endptr == '-')
                                endptr++;
                        }
                        else
                            continue;
                    }
                    else if(alt)
                    {
                        continue;
                    }

                    // find the item
                    ItemType_t* t = nullptr;
                    for(ItemType_t& t_i : f.types)
                    {
                        if(t_i.type == i)
                        {
                            t = &t_i;
                            break;
                        }
                    }

                    if(alt)
                    {
                        if(!t)
                        {
                            pLogWarning("Ignoring alt key %s for non-existent item %d", key.c_str(), i);
                            continue;
                        }
                        else
                        {
                            // add a copy to the end of the array
                            f.types.push_back(*t);
                            t = &f.types[f.types.size() - 1];
                        }
                    }
                    else if(!t)
                    {
                        f.types.push_back(ItemType_t(i));
                        t = &f.types[f.types.size() - 1];
                    }

                    if(SDL_strncasecmp(endptr, "adj", 3) == 0)
                    {
                        f.is_misc = false;

                        editor.read(key.c_str(), temp_str, "");
                        t->set_adj(temp_str.c_str());
                    }
                    else if(SDL_strncasecmp(endptr, "gr", 2) == 0)
                    {
                        int temp;
                        editor.read(key.c_str(), temp, 1);
                        if(temp < 0 || temp >= 16)
                            pLogWarning("Ignoring invalid group %d for item %d (max 15)", temp, i);
                        else
                            t->group = temp;
                    }
                    else if(has_slope && SDL_strncasecmp(endptr, "sl", 2) == 0)
                    {
                        int temp;
                        editor.read(key.c_str(), temp, 0);

                        switch(temp)
                        {
                        case 0:
                            t->slope = ItemType_t::no_slope;
                            break;
                        case 1:
                            t->slope = ItemType_t::slope_1;
                            break;
                        case 2:
                            t->slope = ItemType_t::slope_2;
                            break;
                        case 4:
                            t->slope = ItemType_t::slope_4;
                            break;
                        default:
                            pLogWarning("Ignoring invalid slope %d for item %d", temp, i);
                        }
                    }
                    else if(alt)
                    {
                        pLogWarning("Ignoring invalid alt key %s", key.c_str());
                        continue;
                    }
                    else if(SDL_strncasecmp(endptr, "w", 1) == 0)
                    {
                        int temp;
                        editor.read(key.c_str(), temp, 1);
                        if(temp < 0 || temp >= 8)
                            pLogWarning("Ignoring invalid width %d for item %d (max 7)", temp, i);
                        else
                            t->width = temp;
                    }
                    else if(SDL_strncasecmp(endptr, "h", 1) == 0)
                    {
                        int temp;
                        editor.read(key.c_str(), temp, 1);
                        if(temp < 0 || temp >= 8)
                            pLogWarning("Ignoring invalid height %d for item %d (max 7)", temp, i);
                        else
                            t->height = temp;
                    }
                    else
                    {
                        pLogWarning("Ignoring invalid key %s for item %d in family %s", key.c_str(), i, group.c_str());
                    }
                }
            }

#if 0
            for(ItemType_t& t : f.types)
            {
                printf("Itemtype %d (gp %d, slope %d, wh %d%d)", t.type, t.group, (int)t.slope, t.width, t.height);
                printf(" adj ");
                if(t.has_1) printf("1");
                if(t.has_2) printf("2");
                if(t.has_3) printf("3");
                if(t.has_4) printf("4");
                if(t.has_6) printf("6");
                if(t.has_7) printf("7");
                if(t.has_8) printf("8");
                if(t.has_9) printf("9");
                printf("\n");
            }
#endif

            editor.read("misc", f.is_misc, f.is_misc);
            editor.read("behind", f.behind_mode, false);
            f.make_layout_pods();
        }
    }

    // Process exit codes
    editor.beginGroup("exit-codes");

    editor.read("any", list_level_exit_names[0], "Any");
    editor.read("none", list_level_exit_names[1], "None");

    translate->m_assetsMap.insert({fmt::format_ne("editor.exit-codes.{0}", "any"), &(list_level_exit_names[0])});
    translate->m_assetsMap.insert({fmt::format_ne("editor.exit-codes.{0}", "none"), &(list_level_exit_names[1])});

    for(size_t i = 1; i + 1 < list_level_exit_names.size(); i++)
    {
        const std::string s = fmt::format_ne("code{0}", i);
        editor.read(s.c_str(), list_level_exit_names[i + 1], s);
        translate->m_assetsMap.insert({fmt::format_ne("editor.exit-codes.{0}", i), &(list_level_exit_names[i + 1])});
    }

    editor.endGroup();


    // process Blocks
    if(!block_families.empty())
        loaded++;

    s_ordered_block_families.clear();
    for(ItemFamily& family : block_families)
    {
        s_ordered_block_families.push_back(&family);
    }

    make_pages(s_ordered_block_families, block_pages, 11, 11);

    for(int i = 0; i < maxBlockType; i++)
    {
        block_family_by_type[i] = FAMILY_NONE;
    }

    SDL_assert_release(block_families.size() < 255); // Max of uint8_t

    for(uint8_t family = (uint8_t)block_families.size() - 1; family != FAMILY_NONE; family--)
    {
        for(ItemType_t t : block_families[family].types)
            block_family_by_type[t.type - 1] = family;
    }

    bool blocks_begun = false;
    for(int i = maxBlockType - 1; i >= 0; i--)
    {
        if(block_family_by_type[i] != FAMILY_NONE)
            blocks_begun = true;
        else if(blocks_begun)
            pLogWarning("Can't find family for block type %d", i + 1);
    }

    // add Blocks to translations
    if(translate)
    {
        for(uint8_t i = 0; i < block_families.size(); i++)
            translate->m_assetsMap.insert({fmt::format_ne("editor.block.{0}", block_family_keys[i]), &(block_families[i].name)});
    }


    // process BGOs
    if(!bgo_families.empty())
        loaded++;

    s_ordered_bgo_families.clear();
    for(ItemFamily& family : bgo_families)
    {
        s_ordered_bgo_families.push_back(&family);
    }

    make_pages(s_ordered_bgo_families, bgo_pages, 11, 11);

    for(int i = 0; i < maxBackgroundType; i++)
    {
        bgo_family_by_type[i] = FAMILY_NONE;
    }

    SDL_assert_release(bgo_families.size() < 255); // Max of uint8_t

    for(uint8_t family = (uint8_t)bgo_families.size() - 1; family != FAMILY_NONE; family--)
    {
        for(ItemType_t t : bgo_families[family].types)
            bgo_family_by_type[t.type - 1] = family;
    }

    bool bgos_begun = false;
    for(int i = maxBackgroundType - 1; i >= 0; i--)
    {
        if(bgo_family_by_type[i] != FAMILY_NONE)
            bgos_begun = true;
        // don't worry about locked types
        else if(bgos_begun && i + 1 != 160 && i + 1 != 98)
            pLogWarning("Can't find family for BGO type %d", i + 1);
    }

    // add BGOs to translations
    if(translate)
    {
        for(uint8_t i = 0; i < bgo_families.size(); i++)
            translate->m_assetsMap.insert({fmt::format_ne("editor.bgo.{0}", bgo_family_keys[i]), &(bgo_families[i].name)});
    }


    // process NPCs
    if(!npc_families.empty())
        loaded++;

    s_ordered_npc_families.clear();
    for(ItemFamily& family : npc_families)
    {
        s_ordered_npc_families.push_back(&family);
    }

    make_pages(s_ordered_npc_families, npc_pages, 10, 11);

    for(int i = 0; i < maxNPCType; i++)
    {
        npc_family_by_type[i] = FAMILY_NONE;
    }

    SDL_assert_release(npc_families.size() < 255); // Max of uint8_t

    for(uint8_t family = (uint8_t)npc_families.size() - 1; family != FAMILY_NONE; family--)
    {
        for(ItemType_t t : npc_families[family].types)
            npc_family_by_type[t.type - 1] = family;
    }

#if 0
    // no warnings for NPCs because many are not included

    bool npcs_begun = false;
    for(int i = maxNPCType - 1; i >= 0; i--)
    {
        if(npc_family_by_type[i] != FAMILY_NONE)
            npcs_begun = true;
        else if(npcs_begun)
            pLogWarning("Can't find family for NPC type %d", i + 1);
    }
#endif

    // add NPCs to translations
    if(translate)
    {
        for(uint8_t i = 0; i < npc_families.size(); i++)
            translate->m_assetsMap.insert({fmt::format_ne("editor.npc.{0}", npc_family_keys[i]), &(npc_families[i].name)});
    }


    // process tiles
    if(!tile_families.empty())
        loaded++;

    s_ordered_tile_families.clear();
    for(ItemFamily& family : tile_families)
    {
        s_ordered_tile_families.push_back(&family);
    }

    make_pages(s_ordered_tile_families, tile_pages, 15, 11);

    for(int i = 0; i < maxTileType; i++)
    {
        tile_family_by_type[i] = FAMILY_NONE;
    }

    SDL_assert_release(tile_families.size() < 255); // Max of uint8_t

    for(uint8_t family = (uint8_t)tile_families.size() - 1; family != FAMILY_NONE; family--)
    {
        for(ItemType_t t : tile_families[family].types)
            tile_family_by_type[t.type - 1] = family;
    }

    bool tiles_begun = false;
    for(int i = maxTileType - 1; i >= 0; i--)
    {
        if(tile_family_by_type[i] != FAMILY_NONE)
            tiles_begun = true;
        else if(tiles_begun)
            pLogWarning("Can't find family for tile type %d", i + 1);
    }

    // add tiles to translations
    if(translate)
    {
        for(uint8_t i = 0; i < tile_families.size(); i++)
            translate->m_assetsMap.insert({fmt::format_ne("editor.tile.{0}", tile_family_keys[i]), &(tile_families[i].name)});
    }


    // process bg2s
    if(!bg2_families.empty())
    {
        loaded++;

        std::sort(bg2_families.begin(), bg2_families.end(),
        [](const std::pair<ListItemFamily_t, std::string>& a, const std::pair<ListItemFamily_t, std::string>& b)
        {
            return a.first.sort_index < b.first.sort_index;
        });

        size_t length = 1;
        for(const std::pair<ListItemFamily_t, std::string>& p : bg2_families)
        {
            const ListItemFamily_t& f = p.first;
            length += 1 + f.indices.size();
        }

        bg2_list.indices.reserve(length);
        bg2_list.names.reserve(length);

        bg2_list.indices.push_back(0);
        bg2_list.names.push_back("None");

        for(std::pair<ListItemFamily_t, std::string>& p : bg2_families)
        {
            ListItemFamily_t& f = p.first;

            bg2_list.indices.push_back(-1);
            bg2_list.names.push_back(std::move(f.name));

            for(const int16_t i : f.indices)
                bg2_list.indices.push_back(i);

            for(std::string& s : f.names)
                bg2_list.names.push_back(std::move(s));
        }

        bg2_list.make_layout(10);

        // add to translation engine
        if(translate)
            bg2_list.make_translation(*translate, "background2", "bg2-", bg2_families);
    }


    // process music
    if(!music_families.empty())
    {
        loaded++;

        std::sort(music_families.begin(), music_families.end(),
        [](const std::pair<ListItemFamily_t, std::string>& a, const std::pair<ListItemFamily_t, std::string>& b)
        {
            return a.first.sort_index < b.first.sort_index;
        });

        size_t length = 2;
        for(const std::pair<ListItemFamily_t, std::string>& p : music_families)
        {
            const ListItemFamily_t& f = p.first;
            length += 1 + f.indices.size();
        }

        music_list.indices.reserve(length);
        music_list.names.reserve(length);

        music_list.indices.push_back(0);
        music_list.names.push_back("None");

        music_list.indices.push_back(24);
        music_list.names.push_back("Custom");

        for(std::pair<ListItemFamily_t, std::string>& p : music_families)
        {
            ListItemFamily_t& f = p.first;

            music_list.indices.push_back(-1);
            music_list.names.push_back(std::move(f.name));

            for(const int16_t i : f.indices)
                music_list.indices.push_back(i);

            for(std::string& s : f.names)
                music_list.names.push_back(std::move(s));
        }

        music_list.make_layout(10);

        // add to translation engine
        if(translate)
            music_list.make_translation(*translate, "music", "music", music_families);
    }

    // process world music
    if(!wmusic_families.empty())
    {
        std::sort(wmusic_families.begin(), wmusic_families.end(),
        [](const std::pair<ListItemFamily_t, std::string>& a, const std::pair<ListItemFamily_t, std::string>& b)
        {
            return a.first.sort_index < b.first.sort_index;
        });

        size_t length = 1;
        for(const std::pair<ListItemFamily_t, std::string>& p : wmusic_families)
        {
            const ListItemFamily_t& f = p.first;
            length += 1 + f.indices.size();
        }

        wmusic_list.indices.reserve(length);
        wmusic_list.names.reserve(length);

        wmusic_list.indices.push_back(0);
        wmusic_list.names.push_back("None");

        for(std::pair<ListItemFamily_t, std::string>& p : wmusic_families)
        {
            ListItemFamily_t& f = p.first;

            wmusic_list.indices.push_back(-1);
            wmusic_list.names.push_back(std::move(f.name));

            for(const int16_t i : f.indices)
                wmusic_list.indices.push_back(i);

            for(std::string& s : f.names)
                wmusic_list.names.push_back(std::move(s));
        }

        wmusic_list.make_layout(10);

        // add to translation engine
        if(translate)
            wmusic_list.make_translation(*translate, "worldMusic", "wmusic", wmusic_families);
    }

    // process sounds
    if(!sound_families.empty())
    {
        loaded++;

        std::sort(sound_families.begin(), sound_families.end(),
        [](const std::pair<ListItemFamily_t, std::string>& a, const std::pair<ListItemFamily_t, std::string>& b)
        {
            return a.first.sort_index < b.first.sort_index;
        });

        size_t length = 1;
        for(const std::pair<ListItemFamily_t, std::string>& p : sound_families)
        {
            const ListItemFamily_t& f = p.first;
            length += 1 + f.indices.size();
        }

        sound_list.indices.reserve(length);
        sound_list.names.reserve(length);

        sound_list.indices.push_back(0);
        sound_list.names.push_back("None");

        for(std::pair<ListItemFamily_t, std::string>& p : sound_families)
        {
            ListItemFamily_t& f = p.first;

            sound_list.indices.push_back(-1);
            sound_list.names.push_back(std::move(f.name));

            for(const int16_t i : f.indices)
                sound_list.indices.push_back(i);

            for(std::string& s : f.names)
                sound_list.names.push_back(std::move(s));
        }

        sound_list.make_layout(10);

        // add to translation engine
        if(translate)
            sound_list.make_translation(*translate, "sound", "sound", sound_families);
    }

    // report loading status
    if(loaded == 0)
        pLogWarning("Could not load editor.ini, editor disabled");
    else if(loaded < LOADED_ALL)
        pLogWarning("editor.ini seems incomplete");
}

} // namespace EditorCustom
