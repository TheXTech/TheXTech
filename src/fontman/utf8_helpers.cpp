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

#include "font_manager.h"
#include "font_manager_private.h"

size_t FontManager::utf8_strlen(const char *str)
{
    size_t size = 0;
    while(str && *str != 0)
    {
        size_t  charLen = 1 + static_cast<size_t>(trailingBytesForUTF8[static_cast<UTF8>(*str)]);
        size += charLen;
        str  += charLen;
    }
    return size;
}

size_t FontManager::utf8_strlen(const char *str, size_t len)
{
    size_t utf8_pos = 0;
    while(str && len > 0)
    {
        size_t  charLen = 1 + static_cast<size_t>(trailingBytesForUTF8[static_cast<UTF8>(*str)]);
        str  += charLen;
        len  -= charLen;
        utf8_pos++;
    }
    return utf8_pos;
}

size_t FontManager::utf8_strlen(const std::string &str)
{
    return utf8_strlen(str.c_str(), str.size());
}

size_t FontManager::utf8_substrlen(const std::string &str, size_t utf8len)
{
    size_t pos = 0;
    size_t utf8_pos = 0;
    const char *cstr = str.c_str();
    size_t len = str.length();
    while(cstr && len > 0)
    {
        size_t  charLen = 1 + static_cast<size_t>(trailingBytesForUTF8[static_cast<UTF8>(*cstr)]);
        if(utf8_pos == utf8len)
            return pos;
        cstr += charLen;
        pos  += charLen;
        len  -= charLen;
        utf8_pos++;
    }
    return pos;
}

std::string FontManager::utf8_substr(const std::string &str, size_t utf8_begin, size_t utf8_len)
{
    size_t utf8_pos = 0;
    const char *cstr = str.c_str();
    size_t len = str.length();
    std::string out;
    while(cstr && (len > 0) && (utf8_len > 0))
    {
        size_t  charLen = 1 + static_cast<size_t>(trailingBytesForUTF8[static_cast<UTF8>(*cstr)]);
        if(utf8_pos >= utf8_begin)
        {
            out.append(cstr, charLen);
            utf8_len--;
        }
        cstr += charLen;
        len  -= charLen;
        utf8_pos++;
    }
    return out;
}

const char* FontManager::utf8_skip_begin(const char *str, size_t utf8_begin)
{
    size_t utf8_pos = 0;
    const char *cstr = str;
    const char *out = cstr;
    while(cstr && *cstr != 0)
    {
        size_t  charLen = 1 + static_cast<size_t>(trailingBytesForUTF8[static_cast<UTF8>(*cstr)]);
        if(utf8_pos >= utf8_begin)
            out = cstr;
        cstr += charLen;
        utf8_pos++;
    }

    return out;
}

void FontManager::utf8_pop_back(std::string &str)
{
    size_t pos = 0;
    const char*cstr = str.c_str();
    size_t len = str.length();
    while(cstr && len > 0)
    {
        size_t  charLen = 1 + static_cast<size_t>(trailingBytesForUTF8[static_cast<UTF8>(*cstr)]);
        len  -= charLen;
        if(len == 0)
        {
            str.erase(str.begin() + static_cast<std::string::difference_type>(pos), str.end());
            return;
        }
        cstr += charLen;
        pos  += charLen;
    }
}

void FontManager::utf8_erase_at(std::string &str, size_t begin)
{
    size_t pos = 0;
    size_t utf8_pos = 0;
    const char*cstr = str.c_str();
    size_t len = str.length();
    while(cstr && len > 0)
    {
        size_t  charLen = 1 + static_cast<size_t>(trailingBytesForUTF8[static_cast<UTF8>(*cstr)]);
        if(utf8_pos == begin)
        {
            str.erase(str.begin() + static_cast<std::string::difference_type>(pos), str.end());
            return;
        }
        cstr += charLen;
        pos  += charLen;
        len  -= charLen;
        utf8_pos++;
    }
}

void FontManager::utf8_erase_before(std::string &str, size_t end)
{
    size_t pos = 0;
    size_t utf8_pos = 0;
    const char*cstr = str.c_str();
    size_t len = str.length();
    while(cstr && len > 0)
    {
        size_t  charLen = 1 + static_cast<size_t>(trailingBytesForUTF8[static_cast<UTF8>(*cstr)]);
        if(utf8_pos == end)
        {
            str.erase(str.begin(), str.begin() + static_cast<std::string::difference_type>(pos) + 1);
            return;
        }
        cstr += charLen;
        pos  += charLen;
        len  -= charLen;
        utf8_pos++;
    }
}
