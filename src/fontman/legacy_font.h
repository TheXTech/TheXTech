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
#ifndef LEGACYFONT_H
#define LEGACYFONT_H

#ifdef LOW_MEM
#   include <map>
#else
#   include <unordered_map>
#endif
#include "font_engine_base.h"

struct StdPicture;

/**
 * @brief Implementation of the legacy SMBX font engine as a part of the modern font engine
 */
class LegacyFont final : public BaseFontEngine
{
public:
    explicit LegacyFont(int legacyFontId);
    LegacyFont(const LegacyFont &rf) = delete;
    LegacyFont &operator=(const LegacyFont &tf) = delete;
    LegacyFont(LegacyFont &&tf) = default;

    virtual ~LegacyFont() override;

    void loadFont(int fontId);

    /*!
     * \brief Measure the size of the multiline text block in pixels
     * \param text Multi-line text string
     * \param text_size The byte size of the text string to print
     * \param max_line_length maximum length of the line in characters
     * \param cut Cut the part of string after the line length and put the "..." to the end (Note: input string will remina unmodified)
     * \param fontSize The size of the TTF font glyph (unused for raster fonts)
     * \return Width and height of the text block in pixels
     */
    PGE_Size textSize(const char* text, size_t text_size,
                      uint32_t max_line_lenght = 0,
                      bool  cut = false, uint32_t fontSize = 14) override;

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
     * \param fontSize The size of the TTF font glyph (unused for raster fonts)
     */
    void printText(const char* text, size_t text_size,
                   int32_t x, int32_t y,
                   float Red=1.f, float Green=1.f, float Blue=1.f, float Alpha=1.f,
                   uint32_t fontSize = 0) override;

    bool isLoaded() const override;

    std::string getFontName() const override;
    FontType getFontType() const override;

private:
    //! font is fine
    bool m_isReady = false;

    //! Handalable name of the font
    std::string m_fontName;

    //! Bank of legacy font textures
    StdPicture *m_textures[10];

    /**
     * @brief Raster font glyph
     */
    struct RasChar
    {
        uint8_t  texId = 0; //!< Texture index in m_textures array
        bool     valid = false; //!< Is a valid glyph
        uint16_t tX = 0; //!< Horizontal pixel offset on texture
        uint16_t tY = 0; //!< Vertical pixel offset on texture
        uint8_t  tW = 0; //!< Pixels width of the texture fragment
        uint8_t  tH = 0; //!< Pixels height of the texture fragment
        uint8_t  width = 0; //!< Width of glyph
    };

    //! Fallback width of glyph
    uint32_t m_glyphWidth = 0;

    //! Height of glyph
    uint32_t m_glyphHeight = 0;

#ifdef LOW_MEM
    typedef std::map<char32_t, RasChar > CharMap;
#else
    typedef std::unordered_map<char32_t, RasChar > CharMap;
#endif

    //! Table of available characters
    CharMap m_charMap;

};

#endif // LEGACYFONT_H
