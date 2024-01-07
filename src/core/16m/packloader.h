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

#ifndef PACKLOADER_H
#define PACKLOADER_H

#include <string>
#include <cstdio>

struct StdPicture_Sub;

namespace PackLoader
{
    using packindex_t = uint8_t;
    constexpr packindex_t PACK_NONE = -1;

    void setup(StdPicture_Sub& tex, const std::string& pack_path, uint32_t pack_offset);
    void incref(packindex_t pack);
    void decref(packindex_t pack);

    FILE* getf(const StdPicture_Sub& tex, int index);
    void finalizef(FILE* f, const StdPicture_Sub& tex);

    class packref_t
    {
        packindex_t i = PACK_NONE;

    public:
        packref_t(packindex_t pack = PACK_NONE)
        {
            i = pack;

            if(i != PACK_NONE)
                incref(i);
        }

        ~packref_t()
        {
            if(i != PACK_NONE)
                decref(i);
        }

        inline operator packindex_t() const
        {
            return i;
        }

        const packref_t& operator=(packindex_t pack)
        {
            if(i != PACK_NONE)
                decref(i);

            i = pack;

            if(i != PACK_NONE)
                incref(i);

            return *this;
        }
    };
};

#endif // #ifndef PACKLOADER_H
