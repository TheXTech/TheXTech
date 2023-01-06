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
#ifndef RASTER_FONT_H
#define RASTER_FONT_H

#include <string>
#include <unordered_map>
#include <Utils/vptrlist.h>
#include "std_picture.h"
#include <Graphics/rect.h>
#include <Graphics/size.h>

#include "font_engine_base.h"

class RasterFont : public BaseFontEngine
{
public:
    RasterFont();
    RasterFont(const RasterFont &rf) = delete;
    RasterFont &operator=(const RasterFont &tf) = delete;
    RasterFont(RasterFont &&tf) = default;

    virtual ~RasterFont() override;

    void  loadFont(std::string font_ini);
    void  loadFontMap(std::string fontmap_ini);

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
    //! The fallback TTF name to prefer to render a missing character [all unknown characters will be rendered as TTF]
    std::string m_ttfFallback;
    //! Enable outline borders on backup ttf font render
    bool m_ttfOutlines = false;
    uint32_t m_ttfOutLinesColour = 0x000000FF;
    float    m_ttfOutlinesColourF[4] = {0.f, 0.f, 0.f, 1.f};
    //! The fallback TTF size of the glyph to request
    int  m_ttfSize = -1;
    //! Width of one letter
    uint32_t m_letterWidth = 0;
    //! Height of one letter
    uint32_t m_letterHeight = 0;
    //! Space between printing letters
    uint32_t m_interLetterSpace = 0;
    //! Width of space symbol
    uint32_t m_spaceWidth = 0;
    //! Distance between top of one line and top of next
    uint32_t m_newlineOffset = 0;

    //! Offset all characters by X
    int32_t m_glyphOffsetX = 0;
    //! Offset all characters by Y
    int32_t m_glyphOffsetY = 0;

    //! Width of font matrix
    uint32_t m_matrixWidth = 0;
    //! Width of font matrix
    uint32_t m_matrixHeight = 0;

    //! Handalable name of the font
    std::string m_fontName;

    struct RasChar
    {
        bool valid = false;
        StdPicture* tx     = nullptr;
        uint32_t padding_left    = 0;  //!< Crop left
        uint32_t padding_right   = 0; //!< Crop right
        int32_t x = 0;//!< X pixel offset
        int32_t y = 0;//!< Y pixel offset
    };

    typedef std::unordered_map<char32_t, RasChar > CharMap;

    //! Table of available characters
    CharMap m_charMap;

    //! Bank of loaded textures
    VPtrList<StdPicture > m_texturesBank;
};

#endif // RASTER_FONT_H
