/*
 * Moondust, a free game engine for platform game making
 * Copyright (c) 2014-2021 Vitaly Novichkov <admin@wohlnet.ru>
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

#include "ttf_font.h"
#include <SDL2/SDL_assert.h>
#include "core/render.h"
#include <Logger/logger.h>
#include <unordered_set>
#include <mutex>

#include "font_manager_private.h"

//! FreeType library descriptor
FT_Library  g_ft = nullptr;

//! Loaded TTF fonts set used as fallback for missing glyphs
static std::unordered_set<TtfFont*> g_loadedFaces;
static std::mutex                   g_loadedFaces_mutex;

bool initializeFreeType()
{
    FT_Error error = FT_Init_FreeType(&g_ft);
    SDL_assert_release(error == 0);
    return true;
}

void closeFreeType()
{
    if(g_ft)
    {
        FT_Done_FreeType(g_ft);
        g_ft = nullptr;
    }
}

// Default dummy glyph
const TtfFont::TheGlyph TtfFont::dummyGlyph = TtfFont::TheGlyph();

TtfFont::TtfFont() : BaseFontEngine()
{
    SDL_assert_release(g_ft);
}

TtfFont::~TtfFont()
{
    for(StdPicture &t : m_texturesBank)
        XRender::deleteTexture(t);
    m_texturesBank.clear();

    g_loadedFaces_mutex.lock();
    if(m_face)
    {
        g_loadedFaces.erase(this);
        FT_Done_Face(m_face);
    }
    g_loadedFaces_mutex.unlock();

    m_face = nullptr;
}

bool TtfFont::loadFont(const std::string &path)
{
    SDL_assert_release(g_ft);
    FT_Error error = FT_New_Face(g_ft, path.c_str(), 0, &m_face);
    SDL_assert(error == 0);
    if(error)
        return false;
    error = FT_Set_Pixel_Sizes(m_face, 0, m_recentPixelSize);
    SDL_assert(error == 0);
    if(error)
        return false;
    error = FT_Select_Charmap(m_face, FT_ENCODING_UNICODE);
    SDL_assert(error == 0);
    if(error)
        return false;

    g_loadedFaces_mutex.lock();
    g_loadedFaces.insert(this);
    g_loadedFaces_mutex.unlock();

    m_fontName.append(m_face->family_name);
    m_fontName.push_back(' ');
    m_fontName.append(m_face->style_name);
    m_isReady = true;
    return true;
}

bool TtfFont::loadFont(const char *mem, size_t size)
{
    SDL_assert_release(g_ft);
    FT_Error error = FT_New_Memory_Face(g_ft, reinterpret_cast<const FT_Byte *>(mem),
                                        static_cast<FT_Long>(size), 0, &m_face);
    SDL_assert(error == 0);
    if(error)
        return false;
    error = FT_Set_Pixel_Sizes(m_face, 0, m_recentPixelSize);
    SDL_assert(error == 0);
    if(error)
        return false;
    error = FT_Select_Charmap(m_face, FT_ENCODING_UNICODE);
    SDL_assert(error == 0);
    if(error)
        return false;

    g_loadedFaces_mutex.lock();
    g_loadedFaces.insert(this);
    g_loadedFaces_mutex.unlock();

    m_isReady = true;
    return true;
}

PGE_Size TtfFont::textSize(const std::string &text,
                           uint32_t max_line_length,
                           bool cut, uint32_t fontSize)
{
    const std::string *t = &text;
    std::string modText;

    SDL_assert_release(g_ft);
    if(text.empty())
        return PGE_Size(0, 0);

    //! index of last found space character
    size_t lastspace = 0;
    //! Count of lines
    size_t count     = 1;
    //! detected maximal width of message
    uint32_t maxWidth     = 0;

    uint32_t widthSumm    = 0;
    uint32_t widthSummMax = 0;

    if(cut)
    {
        modText = text;
        std::string::size_type i = modText.find('\n');
        if(i != std::string::npos)
            modText.erase(i, modText.size() - i);
        t = &modText;
    }

    /****************Word wrap*********************/
    uint32_t x = 0;
    for(size_t i = 0; i < t->size(); i++, x++)
    {
        const char &cx = (*t)[i];
        UTF8 uch = static_cast<unsigned char>(cx);

        switch(cx)
        {
        case '\r':
            break;

        case '\t':
        {
            //Fake tabulation
            size_t space = (4 * fontSize);
            widthSumm += (space - ((widthSumm / space) % 4));
            if(widthSumm > widthSummMax)
                widthSummMax = widthSumm;
            break;
        }

        case '\n':
        {
            lastspace = 0;
            if((maxWidth < x) && (maxWidth < max_line_length))
                maxWidth = x;
            x = 0;
            widthSumm = 0;
            count++;
            break;
        }

        default:
        {
            if(' ' == cx)
                lastspace = x;
            const TheGlyph &glyph = getGlyph(fontSize, get_utf8_char(&cx));
            widthSumm += glyph.width > 0 ? uint32_t(glyph.advance >> 6) : (fontSize >> 2);
            if(widthSumm > widthSummMax)
                widthSummMax = widthSumm;
            break;
        }

        }//Switch

        if((max_line_length > 0) && (x >= max_line_length)) //If lenght more than allowed
        {
            maxWidth = x;

            if(t != &modText)
            {
                modText = text;
                t = &modText;
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
                x = 0;
                count++;
            }
        }
        i += static_cast<size_t>(trailingBytesForUTF8[uch]);
    }

    if(count == 1)
        maxWidth = static_cast<uint32_t>(text.length());

    /****************Word wrap*end*****************/
    return PGE_Size(static_cast<int32_t>(widthSummMax), static_cast<int32_t>((fontSize * 1.5) * count));
}

void TtfFont::printText(const std::string &text,
                        int32_t x, int32_t y,
                        float Red, float Green, float Blue, float Alpha,
                        uint32_t fontSize)
{
    SDL_assert_release(g_ft);
    if(text.empty())
        return;

    uint32_t offsetX = 0;
    uint32_t offsetY = 0;
    bool    doublePixel = false; //ConfigManager::setup_fonts.double_pixled;

    const char *strIt  = text.c_str();
    const char *strEnd = strIt + text.size();
    for(; strIt < strEnd; strIt++)
    {
        const char &cx = *strIt;
        UTF8 ucx = static_cast<unsigned char>(cx);

        switch(cx)
        {
        case '\n':
            offsetX = 0;
            offsetY += static_cast<uint32_t>(fontSize * 1.5);
            continue;

        case '\t':
            //Fake tabulation
            offsetX += offsetX + offsetX % uint32_t(fontSize * 1.5);
            continue;
//        case ' ':
//            offsetX += m_spaceWidth + m_interLetterSpace / 2;
//            continue;
        default:
            break;
        }

        const TheGlyph &glyph = getGlyph(doublePixel ? (fontSize / 2) : fontSize, get_utf8_char(&cx));
        if(glyph.tx)
        {
            // GlRenderer::setTextureColor(Red, Green, Blue, Alpha);
            int32_t glyph_x = x + static_cast<int32_t>(offsetX);
            int32_t glyph_y = y + static_cast<int32_t>(offsetY + fontSize);
            XRender::renderTextureScale(
                static_cast<float>(glyph_x + glyph.left),
                static_cast<float>(glyph_y - glyph.top),
                (doublePixel ? (glyph.width * 2) : glyph.width),
                (doublePixel ? (glyph.height * 2) : glyph.height),
                *glyph.tx,
                Red, Green, Blue, Alpha
            );
//            GlRenderer::renderTexture(glyph.tx,
//                                      static_cast<float>(glyph_x + glyph.left),
//                                      static_cast<float>(glyph_y - glyph.top),
//                                      (doublePixel ? (glyph.width * 2) : glyph.width),
//                                      (doublePixel ? (glyph.height * 2) : glyph.height)
//                                      );
        }
        offsetX += glyph.tx ? uint32_t(glyph.advance >> 6) : (fontSize >> 2);

        strIt += static_cast<size_t>(trailingBytesForUTF8[ucx]);
    }
}

bool TtfFont::isLoaded()
{
    return m_isReady;
}

std::string TtfFont::getFontName()
{
    return m_fontName;
}

uint32_t TtfFont::drawGlyph(const char *u8char,
                           int32_t x, int32_t y, uint32_t fontSize, double scaleSize,
                           float Red, float Green, float Blue, float Alpha)
{
    (void)(scaleSize);

    const TheGlyph &glyph = getGlyph(fontSize, get_utf8_char(u8char));
    if(glyph.tx)
    {
        int32_t glyph_x = x;
        int32_t glyph_y = y + static_cast<int32_t>(fontSize);
        XRender::renderTextureScale(
            static_cast<float>(glyph_x + glyph.left),
            static_cast<float>(glyph_y - glyph.top),
            glyph.width * static_cast<float>(scaleSize),
            glyph.height * static_cast<float>(scaleSize),
            *glyph.tx,
            Red, Green, Blue, Alpha
        );
        return glyph.width;
    }
    return fontSize;
}

TtfFont::TheGlyphInfo TtfFont::getGlyphInfo(const char *u8char, uint32_t fontSize)
{
    TheGlyph glyph = getGlyph(fontSize, get_utf8_char(u8char));
    TheGlyphInfo info;
    info.width = glyph.width;
    info.height = glyph.height;
    info.left = glyph.left;
    info.top = glyph.top;
    info.advance = glyph.advance;
    info.glyph_width = glyph.glyph_width;
    return info;
}

const TtfFont::TheGlyph &TtfFont::getGlyph(uint32_t fontSize, char32_t character)
{
    auto fSize = m_charMap.find(fontSize);
    if(fSize != m_charMap.end())
    {
        auto rc = fSize->second.find(character);
        if(rc != fSize->second.end())
            return rc->second;
        return loadGlyph(fontSize, character);
    }

    return loadGlyph(fontSize, character);
}

const TtfFont::TheGlyph &TtfFont::loadGlyph(uint32_t fontSize, char32_t character)
{
    FT_Error     error = 0;
    FT_UInt      t_glyphIndex = 0;
    TheGlyph     t_glyph;
    FT_Face      cur_font = m_face;

    if(m_recentPixelSize != fontSize)
    {
        g_loadedFaces_mutex.lock();
        error = FT_Set_Pixel_Sizes(cur_font, 0, fontSize);
        SDL_assert_release(error == 0);
        m_recentPixelSize = fontSize;
        g_loadedFaces_mutex.unlock();
    }

    t_glyphIndex = FT_Get_Char_Index(cur_font, character);
    if(t_glyphIndex == 0)//Attempt to find a fallback
    {
        g_loadedFaces_mutex.lock();
        for(TtfFont *fb_font : g_loadedFaces)
        {
            if(fb_font->m_recentPixelSize != fontSize)
            {
                error = FT_Set_Pixel_Sizes(fb_font->m_face, 0, fontSize);
                SDL_assert_release(error == 0);
                fb_font->m_recentPixelSize = fontSize;
            }

            t_glyphIndex = FT_Get_Char_Index(fb_font->m_face, character);
            if(t_glyphIndex != 0)
            {
                // Fallback has been found!
                cur_font = fb_font->m_face;
                break;
            }
        }
        g_loadedFaces_mutex.unlock();
    }

    error = FT_Load_Glyph(cur_font, t_glyphIndex, FT_LOAD_RENDER);
    if(error != 0)
        return dummyGlyph;


    FT_GlyphSlot glyph  = cur_font->glyph;
    FT_Bitmap &bitmap   = glyph->bitmap;
    uint32_t width      = bitmap.width;
    uint32_t height     = bitmap.rows;
    uint32_t pitch      = width * 4;

    if((width == 0) || (height == 0))
        return dummyGlyph;

    auto *image = new uint8_t[4 * width * height];
    if(bitmap.pitch >= 0)
    {
        for(uint32_t w = 0; w < width; w++)
        {
            for(uint32_t h = 0; h < height; h++)
            {
                uint8_t color = bitmap.buffer[static_cast<uint32_t>(bitmap.pitch) * ((height - 1) - h) + w];
                image[(4 * width) * (height - 1 - h) + (4 * w)] = color;
                image[(4 * width) * (height - 1 - h) + (4 * w + 1)] = color;
                image[(4 * width) * (height - 1 - h) + (4 * w + 2)] = color;
                image[(4 * width) * (height - 1 - h) + (4 * w + 3)] = color;
            }
        }
    }
    else if(bitmap.pitch < 0)
    {
        for(uint32_t w = 0; w < width; w++)
        {
            for(uint32_t h = 0; h < height; h++)
            {
                uint8_t color = *(bitmap.buffer - (static_cast<uint32_t>(bitmap.pitch) * (h)) + w);
                image[(4 * width)*h + (4 * w)] = color;
                image[(4 * width)*h + (4 * w + 1)] = color;
                image[(4 * width)*h + (4 * w + 2)] = color;
                image[(4 * width)*h + (4 * w + 3)] = color;
            }
        }
    }

    m_texturesBank.emplace_back();
    StdPicture &texture = m_texturesBank.back();
    texture.w = width;
    texture.h = height;
    texture.frame_w = width;
    texture.frame_h = height;
    texture.inited = true;
    texture.l.w_orig = int(texture.w);
    texture.l.h_orig = int(texture.h);
    texture.l.w_scale = float(width) / float(texture.l.w_orig);
    texture.l.h_scale = float(width) / float(texture.l.h_orig);

    g_render->loadTexture(texture, width, height, image, pitch);
    t_glyph.tx      = &texture;
    t_glyph.width   = width;
    t_glyph.height  = height;
    t_glyph.left    = glyph->bitmap_left;
    t_glyph.top     = glyph->bitmap_top;
    t_glyph.advance = int32_t(glyph->advance.x);
    t_glyph.glyph_width = glyph->advance.x;

    delete [] image;

    auto fSize = m_charMap.find(fontSize);
    if(fSize == m_charMap.end())
    {
        m_charMap.insert({fontSize, CharMap()});
        fSize = m_charMap.find(fontSize);
        SDL_assert_release(fSize != m_charMap.end());
    }

    fSize->second.insert({character, t_glyph});
    auto rc = fSize->second.find(character);
    SDL_assert_release(rc != fSize->second.end());

    return rc->second;
}
