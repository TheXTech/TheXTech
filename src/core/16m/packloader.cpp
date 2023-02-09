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

#include <cstdio>

#include <string>
#include <array>
#include <unordered_map>

#include <Logger/logger.h>

#include "std_picture.h"
#include "core/16m/packloader.h"
#include "core/msgbox.h"

void logMemUse();

namespace PackLoader
{

struct Pack_t
{
    FILE* f = nullptr;
    uint16_t refcount = 0;
};

static std::unordered_map<std::string, packindex_t> s_pack_map;
static std::array<Pack_t, 255> s_packs;
static std::array<packindex_t, 255> s_freed_packs;
static uint8_t s_alloc_count = 0;
static uint8_t s_free_count = 0;

static inline packindex_t psetup(const std::string& pack_path)
{
    packindex_t pack;
    if(s_free_count > 0)
        pack = s_freed_packs[--s_free_count];
    else
        pack = s_alloc_count++;

    s_packs[pack].f = fopen(pack_path.c_str(), "rb");
    s_packs[pack].refcount = 0;

    pLogDebug("Opened pack #%d at %s", (int)pack, pack_path.c_str());

    return pack;
}

static inline void pfree(packindex_t pack)
{
    pLogDebug("Closing pack #%d", (int)pack);

    if(s_packs[pack].f)
        fclose(s_packs[pack].f);

    s_packs[pack].f = nullptr;
    s_packs[pack].refcount = 0;

    s_freed_packs[s_free_count++] = pack;

    for(auto it = s_pack_map.begin(); it != s_pack_map.end(); ++it)
    {
        if(it->second == pack)
        {
            s_pack_map.erase(it);
            break;
        }
    }
}

void setup(StdPicture& tex, const std::string& pack_path, uint32_t pack_offset)
{
    auto it = s_pack_map.find(pack_path);

    if(it == s_pack_map.end())
    {
        tex.l.pack = psetup(pack_path);
        s_pack_map[pack_path] = tex.l.pack;
    }
    else
    {
        tex.l.pack = it->second;
    }

    s_packs[tex.l.pack].refcount++;
    tex.l.pack_offset = pack_offset;
}

void incref(uint8_t pack)
{
    s_packs[pack].refcount++;
}

void decref(uint8_t pack)
{
    s_packs[pack].refcount--;

    if(s_packs[pack].refcount == 0)
        pfree(pack);
}

FILE* getf(const StdPicture& tex, int index)
{
    // XMsgBox::errorMsgBox("was called", "open");

    if(tex.l.pack == PACK_NONE)
    {
        const std::string&& path = (index == 0) ? tex.l.path       :
                                  ((index == 1) ? tex.l.path + '1' :
                                                  tex.l.path + '2');

        // pLogDebug("Opening %s", path.c_str());

        FILE* f = fopen(path.c_str(), "rb");

        return f;
    }
    else
    {
        uint32_t offset = tex.l.pack_offset;

        // adjust offset as needed
        if(index != 0)
        {
            // palettes for each previous tex
            offset += 32 * index;

            // texture data for previous textures
            int prev_logical_w = tex.w;
            int prev_logical_h = index * 2048;

            prev_logical_w >>= (1 + (tex.d.flags & 15));
            prev_logical_h >>= (1 + (tex.d.flags & 15));

            // 4bpp
            offset += prev_logical_w * prev_logical_h / 2;
        }

        // pLogDebug("Pack #%d, image part %d at offset %lu", (int)tex.l.pack, index, offset);


        FILE* const f = s_packs[tex.l.pack].f;

        if(f && fseek(f, offset, SEEK_SET) == 0)
            return f;
    }

    return nullptr;
}

void finalizef(FILE* f, const StdPicture& tex)
{
    if(tex.l.pack == PACK_NONE)
        fclose(f);
}

}; // namespace PackLoader
