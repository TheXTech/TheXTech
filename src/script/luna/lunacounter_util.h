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
#ifndef LUNACOUNTER_UTIL_H
#define LUNACOUNTER_UTIL_H

#include <cstdio>
#include <cstdint>


namespace LunaCounterUtil
{

inline size_t writeIntLE(FILE *openfile, int32_t inValue)
{
    uint8_t out[4];
    out[0] = 0xFF & (static_cast<uint32_t>(inValue) >> 0);
    out[1] = 0xFF & (static_cast<uint32_t>(inValue) >> 8);
    out[2] = 0xFF & (static_cast<uint32_t>(inValue) >> 16);
    out[3] = 0xFF & (static_cast<uint32_t>(inValue) >> 24);
    return std::fwrite(out, 1, 4, openfile);
}

 inline size_t readIntLE(FILE *openfile, int32_t &outValue)
{
    uint8_t in[4];
    size_t ret = std::fread(in, 1, 4, openfile);

    if(ret != 4)
        return ret;

    outValue = (int32_t)
               ((static_cast<uint32_t>(in[0]) << 0) & 0x000000FF)
             | ((static_cast<uint32_t>(in[1]) << 8) & 0x0000FF00)
             | ((static_cast<uint32_t>(in[2]) << 16) & 0x00FF0000)
             | ((static_cast<uint32_t>(in[3]) << 24) & 0xFF000000);

    return ret;
}

inline size_t writeUIntLE(FILE *openfile, uint32_t inValue)
{
    uint8_t out[4];
    out[0] = 0xFF & (inValue >> 0);
    out[1] = 0xFF & (inValue >> 8);
    out[2] = 0xFF & (inValue >> 16);
    out[3] = 0xFF & (inValue >> 24);
    return std::fwrite(out, 1, 4, openfile);
}

inline size_t readUIntLE(FILE *openfile, uint32_t &outValue)
{
    uint8_t in[4];
    size_t ret = std::fread(in, 1, 4, openfile);

    if(ret != 4)
        return ret;

    outValue = ((static_cast<uint32_t>(in[0]) << 0) & 0x000000FF)
             | ((static_cast<uint32_t>(in[1]) << 8) & 0x0000FF00)
             | ((static_cast<uint32_t>(in[2]) << 16) & 0x00FF0000)
             | ((static_cast<uint32_t>(in[3]) << 24) & 0xFF000000);

    return ret;
}

} // namespace


#endif // LUNACOUNTER_UTIL_H
