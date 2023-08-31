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

#pragma once
#ifndef TTF_FONT_H
#define TTF_FONT_H


#ifdef LOW_MEM
#   include <map>
#else
#   include <unordered_map>
#endif
#include <Utils/vptrlist.h>
#include "std_picture.h"

#ifdef HAVE_STDINT_H
#undef HAVE_STDINT_H //Avoid a warning about HAVE_STDINT_H's redefinition
#endif
#include <freetype2/ft2build.h>
#include <freetype2/freetype/freetype.h>
#include <freetype2/freetype/ftglyph.h>
#include <freetype2/freetype/ftoutln.h>
#include <freetype2/freetype/fttrigon.h>

#include "font_engine_base.h"

//! FreeType library descriptor
extern FT_Library  g_ft;

extern bool initializeFreeType();
extern void closeFreeType();

/**
 * @brief The FreeType-based font engine
 */
class TtfFont final : public BaseFontEngine
{
public:
    TtfFont();
    TtfFont(const TtfFont &tf) = delete;
    TtfFont &operator=(const TtfFont &tf) = delete;
    TtfFont(TtfFont &&tf) = default;
    virtual ~TtfFont() override;

    bool loadFont(const std::string &path);
    bool loadFont(const char* mem, size_t size);

    void setAntiAlias(bool enable);
    bool antiAlias() const;

    void setBitmapSize(int size);
    int bitmapSize() const;

    void setDoublePixel(bool enable);
    bool doublePixel() const;

    /*!
     * \brief Measure the size of the multiline text block in pixels
     * \param text Multi-line text string
     * \param text_size The byte size of the text string to print
     * \param max_line_length maximum length of the line in characters
     * \param cut Cut the part of string after the line length and put the "..." to the end (Note: input string will remina unmodified)
     * \param fontSize The size of the TTF font glyph
     * \return Width and height of the text block in pixels
     */
    PGE_Size textSize(const char *text, size_t text_size,
                      uint32_t max_line_length = 0,
                      bool cut = false, uint32_t fontSize = 14) override;

    /*!
     * \brief Get a size of one glyph
     * \param utf8char Pointer to one UTF-8 character
     * \param charNum Character number in the line (required to compute the size of the taboluation)
     * \param fontSize Size of TTF font glyph
     * \return Width and height of one glyph in pixels
     */
    PGE_Size glyphSize(const char *utf8char, uint32_t charNum = 0, uint32_t fontSize = 14) override;

    /*!
     * \brief Print the multiline text block on the screen
     * \param text Multi-line text string
     * \param text_size The byte size of the text string to print
     * \param x Hotizontal screen position (at left-top corner of the block)
     * \param y Vertical screen position (at left-top corner of the block)
     * \param Red Red channel colour
     * \param Green Green channel colour
     * \param Blue Blue channel colour
     * \param Alpha Alpha channel level
     * \param fontSize The size of the TTF font glyph
     */
    void printText(const char *text, size_t text_size,
                   int32_t x, int32_t y,
                   float Red = 1.f, float Green = 1.f, float Blue = 1.f, float Alpha = 1.f,
                   uint32_t fontSize = 14) override;

    bool isLoaded() const override;

    void setFontName(const std::string &name);

    std::string getFontName() const override;
    FontType getFontType() const override;

    /**
     * @brief Draw a single glyph
     * @param u8char Pointer to UTF8 multi-byte character
     * @param x X position to draw
     * @param y Y position do draw
     * @param fontSize Size of font
     * @param scaleSize Scale rendered texture
     * @param drawOutlines Draw the outline at every glyph
     * @param Red Red color level from 0.0 to 1.0
     * @param Green Green color level from 0.0 to 1.0
     * @param Blue Blue color level from 0.0 to 1.0
     * @param Alpha Transparency level from 0.0 to 1.0
     * @return Width of the glypth
     */
    uint32_t drawGlyph(const char* u8char,
                       int32_t x, int32_t y, uint32_t fontSize, double scaleSize = 1.0,
                       bool drawOutlines = false,
                       float Red = 1.f, float Green = 1.f, float Blue = 1.f, float Alpha = 1.f,
                       float OL_Red = 0.f, float OL_Green = 0.f, float OL_Blue = 0.f, float OL_Alpha = 1.f);

    struct TheGlyphInfo
    {
        uint32_t width  = 0;
        uint32_t height = 0;
        int32_t  left   = 0;
        int32_t  top    = 0;
        int32_t  advance = 0;
        FT_Pos   glyph_width = 0;
    };

    TheGlyphInfo getGlyphInfo(const char *u8char, uint32_t fontSize);

private:
    //! font is fine
    bool m_isReady = false;
    //! Handalable name of the font
    std::string m_fontName;

    //! Recently used pixel size of the font
    uint32_t     m_recentPixelSize = 10;

    //! Loaded font face
    FT_Face      m_face = nullptr;

    //! Load anti-aliased glyphs
    bool         m_enableAntialias = true;

    //! Make pixelized glyphs: load them with a smaller size, and then, upscale them during a render
    bool         m_doublePixel     = false;

    //! Font face preferred bitmap size, 0 to disable
    int          m_bitmapSize = 0;

    struct TheGlyph
    {
        TheGlyph() = default;
        StdPicture *tx     = nullptr;
        uint32_t width  = 0;
        uint32_t height = 0;
        int32_t  left   = 0;
        int32_t  top    = 0;
        int32_t  advance = 0;
        FT_Pos   glyph_width = 0;
    };

    //! Default dummy glyph
    static const TheGlyph dummyGlyph;

    const TheGlyph &getGlyph(uint32_t fontSize, char32_t character);

    // version allocating a new texture
    const TheGlyph &loadGlyph(uint32_t fontSize, char32_t character);

    // version using an existing texture
    const TheGlyph &loadGlyph(StdPicture &texture, uint32_t fontSize, char32_t character);

#ifdef LOW_MEM
    typedef std::map<char32_t, TheGlyph> CharMap;
    typedef std::map<uint32_t, CharMap>  SizeCharMap;
#else
    typedef std::unordered_map<char32_t, TheGlyph> CharMap;
    typedef std::unordered_map<uint32_t, CharMap>  SizeCharMap;
#endif

    SizeCharMap m_charMap;
    VPtrList<StdPicture > m_texturesBank;
};

#endif // TTF_FONT_H
