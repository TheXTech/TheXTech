/*
 * Moondust, a free game engine for platform game making
 * Copyright (c) 2014-2024 Vitaly Novichkov <admin@wohlnet.ru>
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
#include "sdl_proxy/sdl_stdinc.h"
#include "sdl_proxy/sdl_assert.h"
#if !defined(PGE_NO_THREADING)
#   ifdef PGE_SDL_MUTEX
#       include <SDL2/SDL_mutex.h>
#   else
#       include <mutex>
#   endif
#endif
#include "core/render.h"
#include <Logger/logger.h>
#include <unordered_set>

#include "font_manager_private.h"

//! FreeType library descriptor
FT_Library  g_ft = nullptr;

//! Loaded TTF fonts set used as fallback for missing glyphs
static std::unordered_set<TtfFont*> g_loadedFaces;

#ifndef PGE_NO_THREADING
#   ifdef PGE_SDL_MUTEX
static SDL_mutex                   *g_loadedFaces_mutex = nullptr;
#   define TTF_MUTEX_LOCK()         SDL_LockMutex(g_loadedFaces_mutex)
#   define TTF_MUTEX_UNLOCK()       SDL_UnlockMutex(g_loadedFaces_mutex)
#   else
static std::mutex                   g_loadedFaces_mutex;
#   define TTF_MUTEX_LOCK()         g_loadedFaces_mutex.lock()
#   define TTF_MUTEX_UNLOCK()       g_loadedFaces_mutex.unlock()
#   endif
#else
#   define TTF_MUTEX_LOCK()         (void)0
#   define TTF_MUTEX_UNLOCK()       (void)0
#endif


bool initializeFreeType()
{
    FT_Error error = FT_Init_FreeType(&g_ft);
    SDL_assert_release(error == 0);
#if !defined(PGE_NO_THREADING) && defined(PGE_SDL_MUTEX)
    if(!g_loadedFaces_mutex)
        g_loadedFaces_mutex = SDL_CreateMutex();
#endif
    return true;
}

void closeFreeType()
{
    if(g_ft)
    {
        FT_Done_FreeType(g_ft);
        g_ft = nullptr;
    }

#if !defined(PGE_NO_THREADING) && defined(PGE_SDL_MUTEX)
    if(g_loadedFaces_mutex)
    {
        SDL_DestroyMutex(g_loadedFaces_mutex);
        g_loadedFaces_mutex = nullptr;
    }
#endif
}

// Default dummy glyph
const TtfFont::TheGlyph TtfFont::dummyGlyph = TtfFont::TheGlyph();

TtfFont::TtfFont() : BaseFontEngine()
{
    SDL_assert_release(g_ft);
}

TtfFont::~TtfFont()
{
    TTF_MUTEX_LOCK();
    if(m_face)
    {
        g_loadedFaces.erase(this);
        FT_Done_Face(m_face);
    }
    TTF_MUTEX_UNLOCK();

    m_face = nullptr;
}

bool TtfFont::loadFont(const std::string &path)
{
    SDL_assert_release(g_ft);
    FT_Error error = FT_New_Face(g_ft, path.c_str(), 0, &m_face);
    if(error)
    {
        pLogWarning("Failed to load the font: %s", FT_Error_String(error));
        return false;
    }

    error = FT_Set_Pixel_Sizes(m_face, 0, m_recentPixelSize);
    if(error)
        pLogWarning("Failed to set the pixel sizes %u for the font %s: %s", m_recentPixelSize, path.c_str(), FT_Error_String(error));

    error = FT_Select_Charmap(m_face, FT_ENCODING_UNICODE);
    if(error)
    {
        pLogWarning("Failed to select the charmap for the font %s: %s", path.c_str(), FT_Error_String(error));
        FT_Done_Face(m_face);
        m_face = nullptr;
        return false;
    }

    TTF_MUTEX_LOCK();
    g_loadedFaces.insert(this);
    TTF_MUTEX_UNLOCK();

    m_fontName.append(m_face->family_name);
    m_fontName.push_back(' ');
    m_fontName.append(m_face->style_name);
    m_isReady = true;

    pLogDebug("Loaded TTF font with name: [%s]", m_fontName.c_str());

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

    TTF_MUTEX_LOCK();
    g_loadedFaces.insert(this);
    TTF_MUTEX_UNLOCK();

    m_isReady = true;
    return true;
}

void TtfFont::setAntiAlias(bool enable)
{
    m_enableAntialias = enable;
}

bool TtfFont::antiAlias() const
{
    return m_enableAntialias;
}

void TtfFont::setBitmapSize(int size)
{
    m_bitmapSize = size;
}

int TtfFont::bitmapSize() const
{
    return m_bitmapSize;
}

void TtfFont::setDoublePixel(bool enable)
{
    m_doublePixel = enable;
}

bool TtfFont::doublePixel() const
{
    return m_doublePixel;
}

PGE_Size TtfFont::glyphSize(const char* utf8char, uint32_t charNum, uint32_t fontSize)
{
    PGE_Size ret(0, static_cast<int32_t>(fontSize * 1.5));

    const char &cx = utf8char[0];

    switch(cx)
    {
    case '\n':
    case '\r':
        break;

    case '\t':
    {
        //Fake tabulation
        size_t space = (4 * fontSize);
        if(space == 0)
            space = 1; // Don't divide by zero
        ret.setWidth(space - ((charNum / space) % 4));
        break;
    }

    default:
    {
        const TheGlyph &glyph = getGlyph(fontSize, get_utf8_char(&cx));
        ret.setWidth(glyph.width > 0 ? uint32_t(glyph.advance >> 6) : (fontSize >> 2));
        break;
    }

    }//Switch

    return ret;
}

PGE_Size TtfFont::printText(const char *text, size_t text_size,
                            int32_t x, int32_t y,
                            XTColor color,
                            uint32_t fontSize,
                            CropInfo* crop_info)
{
    SDL_assert_release(g_ft);
    if(!text || text_size == 0)
        return PGE_Size(0, 0);

    int32_t  offsetX = (crop_info) ? -crop_info->offset : 0;
    uint32_t offsetY = 0;

    int32_t  offsetX_max = 0;

    uint8_t letter_alpha = color.a;

    const char *strIt  = text;
    const char *strEnd = strIt + text_size;
    for(; strIt < strEnd; strIt++)
    {
        const char &cx = *strIt;
        UTF8 ucx = static_cast<unsigned char>(cx);

        switch(cx)
        {
        case '\n':
            if(offsetX > offsetX_max)
                offsetX_max = offsetX;

            offsetX = (crop_info) ? -crop_info->offset : 0;
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

        const TheGlyph &glyph = getGlyph(m_doublePixel ? (fontSize / 2) : fontSize, get_utf8_char(&cx));
        if(glyph.tx)
        {
            if(crop_info)
            {
                if(!crop_info->letter_alpha(letter_alpha, color.a, offsetX, offsetX + (glyph.advance >> 6)))
                    break;
            }

            if(letter_alpha != 0)
            {
                int32_t glyph_x = x + static_cast<int32_t>(offsetX);
                int32_t glyph_y = y + static_cast<int32_t>(offsetY + fontSize);
                XRender::renderTextureScale(
                    static_cast<float>(glyph_x + glyph.left),
                    static_cast<float>(glyph_y - glyph.top),
                    (m_doublePixel ? (glyph.width * 2) : glyph.width),
                    (m_doublePixel ? (glyph.height * 2) : glyph.height),
                    *glyph.tx,
                    color.with_alpha(letter_alpha)
                );
            }
        }
        offsetX += glyph.tx ? uint32_t(glyph.advance >> 6) : (fontSize >> 2);

        strIt += static_cast<size_t>(trailingBytesForUTF8[ucx]);
    }

    if(offsetX > offsetX_max)
        offsetX_max = offsetX;

    if(offsetX > 0)
        offsetY += static_cast<uint32_t>(fontSize);

    return PGE_Size(offsetX_max, offsetY);
}

bool TtfFont::isLoaded() const
{
    return m_isReady;
}

void TtfFont::setFontName(const std::string& name)
{
    m_fontName = name;
}

std::string TtfFont::getFontName() const
{
    return m_fontName;
}

BaseFontEngine::FontType TtfFont::getFontType() const
{
    return FONT_TTF;
}

uint32_t TtfFont::drawGlyph(const char *u8char,
                            int32_t x, int32_t y, uint32_t fontSize, double scaleSize,
                            bool drawOutlines,
                            XTColor color,
                            XTColor OL_color)
{
    const TheGlyph &glyph = getGlyph(fontSize, get_utf8_char(u8char));
    if(glyph.tx)
    {
        int32_t glyph_x = x;
        int32_t glyph_y = y + static_cast<int32_t>(fontSize);

        if(drawOutlines)
        {
            const double offsets[4][2] =
            {
                {-scaleSize, 0.0},
                { scaleSize, 0.0},
                {0.0, -scaleSize},
                {0.0,  scaleSize}
            };

            // take square of Alpha to match blend of normal text
            uint8_t scaled_a = XTColor::mul(color.a, color.a);

            for(size_t i = 0; i < 4; ++i)
            {
                XRender::renderTextureScale(
                    static_cast<float>(glyph_x + glyph.left + offsets[i][0]),
                    static_cast<float>(glyph_y - glyph.top + offsets[i][1]),
                    glyph.width * static_cast<float>(scaleSize),
                    glyph.height * static_cast<float>(scaleSize),
                    *glyph.tx,
                    color.with_alpha(scaled_a) * OL_color
                );
            }
        }

        XRender::renderTextureScale(
            static_cast<float>(glyph_x + glyph.left),
            static_cast<float>(glyph_y - glyph.top),
            glyph.width * static_cast<float>(scaleSize),
            glyph.height * static_cast<float>(scaleSize),
            *glyph.tx,
            color
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
        {
            // reload if the renderer has unloaded the glyph
            if(rc->second.tx && !rc->second.tx->d.hasTexture())
                return loadGlyph(*(rc->second.tx), fontSize, character);

            return rc->second;
        }
        return loadGlyph(fontSize, character);
    }

    return loadGlyph(fontSize, character);
}

const TtfFont::TheGlyph &TtfFont::loadGlyph(uint32_t fontSize, char32_t character)
{
    m_texturesBank.emplace_back();
    StdPicture &texture = m_texturesBank.back();

    const TtfFont::TheGlyph &ret = loadGlyph(texture, fontSize, character);

    if(ret.tx != &texture)
        m_texturesBank.pop_back();

    return ret;
}

const TtfFont::TheGlyph &TtfFont::loadGlyph(StdPicture &texture, uint32_t fontSize, char32_t character)
{
    FT_Error     error = 0;
    FT_UInt      t_glyphIndex = 0;
    TheGlyph     t_glyph;
    FT_Face      cur_font = m_face;

    if(m_recentPixelSize != fontSize)
    {
        TTF_MUTEX_LOCK();
        error = FT_Set_Pixel_Sizes(cur_font, 0, fontSize);
        if(error)
            pLogWarning("TheGlyph::loadGlyph (1) Failed to set the pixel sizes %u for the font %s: %s", fontSize, cur_font->family_name, FT_Error_String(error));
        m_recentPixelSize = fontSize;
        TTF_MUTEX_UNLOCK();
    }

    t_glyphIndex = FT_Get_Char_Index(cur_font, character);

    if(t_glyphIndex == 0)//Attempt to find a fallback
    {
        TTF_MUTEX_LOCK();
        for(TtfFont *fb_font : g_loadedFaces)
        {
            if(fb_font->m_recentPixelSize != fontSize)
            {
                error = FT_Set_Pixel_Sizes(fb_font->m_face, 0, fontSize);
                if(error)
                    pLogWarning("TheGlyph::loadGlyph (2) Failed to set the pixel sizes %u for the font %s: %s", fontSize, cur_font->family_name, FT_Error_String(error));
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
        TTF_MUTEX_UNLOCK();
    }

    FT_Int32 ftLoadFlags = FT_LOAD_RENDER;

    if(!m_enableAntialias)
        ftLoadFlags |= FT_LOAD_MONOCHROME | FT_LOAD_TARGET_MONO;

    error = FT_Load_Glyph(cur_font, t_glyphIndex, ftLoadFlags);
    if(error != 0)
        return dummyGlyph;


    FT_GlyphSlot glyph  = cur_font->glyph;
    FT_Bitmap &bitmap   = glyph->bitmap;
    uint32_t width      = bitmap.width;
    uint32_t height     = bitmap.rows;
    uint32_t pitch      = width * 4;

    if((width == 0) || (height == 0))
        return dummyGlyph;

    SDL_assert_release(bitmap.buffer); // Buffer must NOT be null

    uint8_t *image = nullptr;

    try
    {
        size_t image_size = 4 * width * height;
        image = new uint8_t[image_size];
        SDL_memset(image, 0, image_size);
    }
    catch(const std::bad_alloc &e)
    {
        pLogCritical("TtfFont::TheGlyph: Out of memory: %s", e.what());
        return dummyGlyph;
    }

    switch(bitmap.pixel_mode)
    {
    case FT_PIXEL_MODE_NONE:
        break;

    case FT_PIXEL_MODE_MONO:
    {
        uint32_t bsize = (width + 7) / 8;

        if(bitmap.pitch >= 0)
        {
            for(uint32_t h = 0; h < height; ++h)
            {
                for(uint32_t b = 0; b < bsize; ++b)
                {
                    uint8_t block = bitmap.buffer[(static_cast<uint32_t>(bitmap.pitch) * ((height - 1) - h)) + b];
                    uint32_t* dest = reinterpret_cast<uint32_t*>(&(image[((4 * width) * (height - 1 - h)) + (32 * b)]));
                    for(int x = 0; x < 8 && (b * 8 + x) < width; x++)
                    {
                        if(block & (1 << (7 - x)))
                            dest[x] = 0xFFFFFFFF;
                        else
                            dest[x] = 0;
                    }
                }
            }
        }
        else if(bitmap.pitch < 0)
        {
            for(uint32_t h = 0; h < height; ++h)
            {
                for(uint32_t b = 0; b < bsize; ++b)
                {
                    uint8_t block = *(bitmap.buffer - (static_cast<uint32_t>(bitmap.pitch) * h) + b);
                    uint32_t* dest = reinterpret_cast<uint32_t*>(&(image[((4 * width) * h) + (32 * b)]));
                    for(int x = 0; x < 8 && (b * 8 + x) < width; x++)
                    {
                        if(block & (1 << (7 - x)))
                            dest[x] = 0xFFFFFFFF;
                        else
                            dest[x] = 0;
                    }
                }
            }
        }
        break;
    }
    case FT_PIXEL_MODE_GRAY:
    {
        if(bitmap.pitch >= 0)
        {
            for(uint32_t h = 0; h < height; ++h)
            {
                for(uint32_t w = 0; w < width; ++w)
                {
                    uint8_t color = bitmap.buffer[static_cast<uint32_t>(bitmap.pitch) * ((height - 1) - h) + w];
                    size_t hp = (4 * width) * (height - 1 - h);
                    uint32_t *dst = reinterpret_cast<uint32_t*>(image + hp + (4 * w));
                    *dst = (uint32_t(color) << 0) |
                           (uint32_t(color) << 8) |
                           (uint32_t(color) << 16) |
                           (uint32_t(color) << 24);
                }
            }
        }
        else if(bitmap.pitch < 0)
        {
            for(uint32_t h = 0; h < height; ++h)
            {
                for(uint32_t w = 0; w < width; ++w)
                {
                    uint8_t color = *(bitmap.buffer - (static_cast<uint32_t>(bitmap.pitch) * h) + w);
                    size_t hp = (4 * width) * h;
                    uint32_t *dst = reinterpret_cast<uint32_t*>(image + hp + (4 * w));
                    *dst = (uint32_t(color) << 0) |
                           (uint32_t(color) << 8) |
                           (uint32_t(color) << 16) |
                           (uint32_t(color) << 24);
                }
            }
        }
        break;
    }

    default:
        pLogWarning("TtfFont::TheGlyph: FIXME: The pixel mode %d is not supported yet!", bitmap.pixel_mode);
        break;
    }

    if(m_doublePixel)
    {
        texture.w = width * 2;
        texture.h = height * 2;
    }
    else
    {
        texture.w = width;
        texture.h = height;
    }

    XRender::loadTexture(texture, width, height, image, pitch);

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
