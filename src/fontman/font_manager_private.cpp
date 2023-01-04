/*
 * Moondust, a free game engine for platform game making
 * Copyright (c) 2014-2023 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * This software is licensed under a dual license system (MIT or GPL version 3 or later).
 * This means you are free to choose with which of both licenses (MIT or GPL version 3 or later)
 * you want to use this software.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You can see text of MIT license in the LICENSE.mit file you can see in Engine folder,
 * or see https://mit-license.org/.
 *
 * You can see text of GPLv3 license in the LICENSE.gpl3 file you can see in Engine folder,
 * or see <http://www.gnu.org/licenses/>.
 */

#include "font_manager_private.h"

/*
 * Index into the table below with the first byte of a UTF-8 sequence to
 * get the number of trailing bytes that are supposed to follow it.
 * Note that *legal* UTF-8 values can't have 4 or 5-bytes. The table is
 * left as-is for anyone who may want to do such conversion, which was
 * allowed in earlier algorithms.
 */
const char trailingBytesForUTF8[256] =
{
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5
};

/*
 * Magic values subtracted from a buffer value during UTF8 conversion.
 * This table contains as many values as there might be trailing bytes
 * in a UTF-8 sequence.
 */
const UTF32 offsetsFromUTF8[6] =
{
    0x00000000UL, 0x00003080UL, 0x000E2080UL,
    0x03C82080UL, 0xFA082080UL, 0x82082080UL
};

char32_t get_utf8_char(const char *str)
{
    const UTF8 *source = reinterpret_cast<const UTF8*>(str);
    UTF32 ch = 0;
    uint16_t extraBytesToRead = static_cast<uint16_t>(trailingBytesForUTF8[*source]);

    switch (extraBytesToRead)
    {
    case 5: ch += *source++; ch <<= 6; /*fallthrough*/ /* remember, illegal UTF-8 */
    case 4: ch += *source++; ch <<= 6; /*fallthrough*/ /* remember, illegal UTF-8 */
    case 3: ch += *source++; ch <<= 6; /*fallthrough*/
    case 2: ch += *source++; ch <<= 6; /*fallthrough*/
    case 1: ch += *source++; ch <<= 6; /*fallthrough*/
    case 0: ch += *source++; /*fallthrough*/
    }
    ch -= offsetsFromUTF8[extraBytesToRead];
    return static_cast<char32_t>(ch);
}

std::u32string std_to_utf32(const std::string &src)
{
    std::u32string dst_tmp;
    dst_tmp.resize(src.size());
    const UTF8* src_begin = reinterpret_cast<const UTF8*>(src.c_str());
    const UTF8* src_end   = src_begin + src.size();
    UTF32*      dst_begin = reinterpret_cast<UTF32*>(&dst_tmp[0]);
    UTF32*      dst_end   = dst_begin + dst_tmp.size();
    PGEFF_ConvertUTF8toUTF32(&src_begin, src_end, &dst_begin, dst_end, strictConversion);
    return std::u32string(reinterpret_cast<char32_t*>(&dst_tmp[0]));
}
