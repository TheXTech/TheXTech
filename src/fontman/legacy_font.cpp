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

#include <algorithm>
#include "sdl_proxy/sdl_stdinc.h"

#include "legacy_font.h"
#include "gfx.h"
#include "std_picture.h"
#include "core/render.h"
#include "font_manager_private.h"

#if defined(_MSC_VER) && _MSC_VER <= 1900 // Workaround for MSVC 2015
namespace std
{
    using ::toupper;
}
#endif


LegacyFont::LegacyFont(int legacyFontId)
{
    loadFont(legacyFontId);
}

LegacyFont::~LegacyFont()
{
    m_charMap.clear();
    SDL_memset(m_textures, 0, sizeof(m_textures));
}

void LegacyFont::loadFont(int fontId)
{
    m_charMap.clear();
    SDL_memset(m_textures, 0, sizeof(m_textures));

    switch(fontId)
    {
    default:
    case 1: // Numbers font
        m_glyphWidth = 18;
        m_glyphHeight = 16;

        for(int i = 0; i < 9; ++i)
        {
            char uchar[5] = {static_cast<char>('0' + i), 0, 0, 0, 0};
            m_textures[i] = &GFX.Font1[i];

            RasChar c;
            c.texId = i;
            c.tX = 0;
            c.tY = 0;
            c.tW = 16;
            c.tH = 14;
            c.width = 18;
            c.valid = true;

            m_charMap.insert({get_utf8_char(uchar), std::move(c)});
        }

        m_isReady = true;
        m_fontName = "numeric";
        break;

    case 2: // World map level title font
        m_glyphWidth = 16;
        m_glyphHeight = 18;

        m_textures[0] = &GFX.Font2[1];
        m_textures[1] = &GFX.Font2S;

        for(char c = 33; c <= 125; ++c)
        {
            char uchar[5] = {c, 0, 0, 0, 0};
            RasChar cc;
            cc.texId = 0;
            cc.tY = 0;
            cc.tW = 15;
            cc.tH = 17;
            cc.width = 16;

            if(c >= 48 && c <= 57)
            {
                cc.tX = (c - 48) * 16;
                cc.texId = 0;
            }
            else if(c >= 65 && c <= 90)
            {
                cc.tX = (c - 55) * 16;
                cc.texId = 0;
            }
            else if(c >= 97 && c <= 122)
            {
                cc.tX = (c - 61) * 16;
                cc.texId = 0;
            }
            else if(c >= 33 && c <= 47)
            {
                cc.tX = (c - 33) * 16;
                cc.texId = 1;
            }
            else if(c >= 58 && c <= 64)
            {
                cc.tX = (c - 58 + 15) * 16;
                cc.texId = 1;
            }
            else if(c >= 91 && c <= 96)
            {
                cc.tX = (c - 91 + 22) * 16;
                cc.texId = 1;
            }
            else if(c >= 123 && c <= 125)
            {
                cc.tX = (c - 123 + 28) * 16;
                cc.texId = 1;
            }

            cc.valid = true;

            m_charMap.insert({get_utf8_char(uchar), std::move(cc)});
        }

        m_isReady = true;
        m_fontName = "font3";
        break;

    case 3: // Menu font
        m_glyphWidth = 16;
        m_glyphHeight = 18;

        m_textures[0] = &GFX.Font2[2];

        for(char c = 33; c <= 126; ++c)
        {
            char ucharH[5] = {c, 0, 0, 0, 0};
            char ucharL[5] = {static_cast<char>(std::tolower(c)), 0, 0, 0, 0};

            RasChar cc;
            cc.texId = 0;
            cc.tX = 2;
            cc.tY = (c - 33) * 32;
            cc.tW = 18;
            cc.tH = 16;
            cc.width = 18;

            if(c == 'M')
                cc.width += 2;

            cc.valid = true;

            m_charMap.insert({get_utf8_char(ucharH), cc});
            m_charMap.insert({get_utf8_char(ucharL), std::move(cc)});
        }

        m_isReady = true;
        m_fontName = "font1";
        break;

    case 4: // Message box font
        m_glyphWidth = 18;
        m_glyphHeight = 18;

        m_textures[0] = &GFX.Font2[3];

        for(char c = 33; c <= 126; ++c)
        {
            char ucharL[5] = {c, 0, 0, 0, 0};

            RasChar cc;
            cc.texId = 0;
            cc.tX = 0;
            cc.tY = (c - 33) * 16;
            cc.tW = 18;
            cc.tH = 16;
            cc.width = 18;

            if(c == 'M')
                cc.width += 2;

            cc.valid = true;

            m_charMap.insert({get_utf8_char(ucharL), cc});
        }

        m_isReady = true;
        m_fontName = "font2";
        break;
    }
}

PGE_Size LegacyFont::textSize(const char* text, size_t text_size,
                              uint32_t max_line_lenght, bool cut,
                              uint32_t /*fontSize*/)
{
    const char* t = text;
    size_t t_size = text_size;
    // TODO: Get rid this string: use integer hints instead of making a modded string copy when performing a cut behaviour
    std::string modText;

    if(!text || text_size == 0)
        return PGE_Size(0, 0);

    size_t lastspace = 0; //!< index of last found space character
    size_t count     = 1; //!< Count of lines
    uint32_t maxWidth     = 0; //!< detected maximal width of message

    uint32_t widthSumm    = 0;
    uint32_t widthSummMax = 0;

    if(cut)
    {
        modText = std::string(text, text_size);
        std::string::size_type i = modText.find('\n');
        if(i != std::string::npos)
            modText.erase(i, modText.size() - i);
        t = modText.c_str();
        t_size = modText.size();
    }

    /****************Word wrap*********************/
    uint32_t x = 0;
    for(size_t i = 0; i < t_size; i++, x++)
    {
        const char &cx = t[i];
        UTF8 uch = static_cast<UTF8>(cx);

        switch(cx)
        {
        case '\r':
            break;

        case '\t':
        {
            lastspace = i;
            size_t spaceSize = m_glyphWidth;
            if(spaceSize == 0)
                spaceSize = 1; // Don't divide by zero
            size_t tabMult = 4 - ((widthSumm / spaceSize) % 4);
            widthSumm += static_cast<size_t>(m_glyphWidth) * tabMult;
            if(widthSumm > widthSummMax)
                widthSummMax = widthSumm;
            break;
        }

        case ' ':
        {
            lastspace = i;
            widthSumm += m_glyphWidth;
            if(widthSumm > widthSummMax)
                widthSummMax = widthSumm;
            break;
        }

        case '\n':
        {
            lastspace = 0;
            if((maxWidth < x) && (maxWidth < max_line_lenght))
                maxWidth = x;
            x = 0;
            widthSumm = 0;
            count++;
            break;
        }

        default:
        {
            CharMap::iterator rc = m_charMap.find(get_utf8_char(&cx));
            if(rc != m_charMap.end())
            {
                RasChar &rch = rc->second;
                widthSumm += rch.valid ? rch.width : m_glyphWidth;
            }
            else
                widthSumm += m_glyphWidth;

            if(widthSumm > widthSummMax)
                widthSummMax = widthSumm;

            break;
        }

        }//Switch

        if((max_line_lenght > 0) && (x >= max_line_lenght)) //If lenght more than allowed
        {
            maxWidth = x;
            if(t != modText.c_str())
            {
                modText = text;
                t = modText.c_str();
                t_size = modText.size();
            }

            if(lastspace > 0)
            {
                modText[lastspace] = '\n';
                i = lastspace - 1;
                lastspace = 0;
            }
            else
            {
                modText.insert(i, 1, '\n');
                t = modText.c_str();
                t_size = modText.size();
                x = 0;
                count++;
            }
        }
        i += static_cast<size_t>(trailingBytesForUTF8[uch]);
    }

    // Unused later
    //    if(count == 1)
    //        maxWidth = static_cast<uint32_t>(t_size);

    /****************Word wrap*end*****************/
    return PGE_Size(static_cast<int32_t>(widthSummMax), static_cast<int32_t>(m_glyphHeight * count));
}

PGE_Size LegacyFont::glyphSize(const char* utf8char, uint32_t charNum, uint32_t /*fontSize*/)
{
    PGE_Size ret(0, m_glyphHeight);
    const char &cx = utf8char[0];

    switch(cx)
    {
    case '\n':
    case '\r':
        break; // Has no lenght

    case '\t':
    {
        size_t spaceSize = m_glyphWidth;
        if(spaceSize == 0)
            spaceSize = 1; // Don't divide by zero
        size_t tabMult = 4 - ((charNum / spaceSize) % 4);
        ret.setWidth(static_cast<size_t>(m_glyphWidth) * tabMult);
        break;
    }

    case ' ':
    {
        ret.setWidth(m_glyphWidth);
        break;
    }

    default:
    {
        CharMap::iterator rc = m_charMap.find(get_utf8_char(&cx));
        if(rc != m_charMap.end())
        {
            RasChar &rch = rc->second;
            ret.setWidth(rch.valid ? rch.width : m_glyphWidth);
        }
        else
            ret.setWidth(m_glyphWidth);

        break;
    }

    }//Switch

    return ret;
}

void LegacyFont::printText(const char* text, size_t text_size, int32_t x, int32_t y,
                           float Red, float Green, float Blue, float Alpha,
                           uint32_t /*fontSize*/)
{
    if(m_charMap.empty() || !text || text_size == 0)
        return;

    uint32_t offsetX = 0;
    uint32_t offsetY = 0;

    const char *strIt  = text;
    const char *strEnd = strIt + text_size;
    for(; strIt < strEnd; strIt++)
    {
        const char &cx = *strIt;
        const char *fch = "?";
        UTF8 ucx = static_cast<unsigned char>(cx);

        switch(cx)
        {
        case '\n':
            offsetX = 0;
            offsetY += m_glyphHeight;
            continue;

        case '\t':
            // Fake tabulation
            offsetX += offsetX + offsetX % m_glyphWidth;
            continue;

        case ' ':
            offsetX += m_glyphWidth;
            continue;
        }

        auto rch_f = m_charMap.find(get_utf8_char(strIt));
        if(rch_f == m_charMap.end())
            rch_f = m_charMap.find(get_utf8_char(fch));

        if(rch_f != m_charMap.end() && rch_f->second.valid)
        {
            const RasChar &rch = rch_f->second;
            XRender::renderTexture(x + static_cast<int32_t>(offsetX),
                                   y + static_cast<int32_t>(offsetY),
                                   rch.tW, rch.tH,
                                   *m_textures[rch.texId],
                                   rch.tX, rch.tY,
                                   Red, Green, Blue, Alpha);
            offsetX += rch.width;
        }
        else
            offsetX += m_glyphWidth;

        strIt += static_cast<size_t>(trailingBytesForUTF8[ucx]);
    }
}

bool LegacyFont::isLoaded() const
{
    return m_isReady;
}

std::string LegacyFont::getFontName() const
{
    return m_fontName;
}

BaseFontEngine::FontType LegacyFont::getFontType() const
{
    return FONT_LEGACY;
}
