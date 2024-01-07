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

#pragma once
#ifndef RASTER_FONT_H
#define RASTER_FONT_H

#include <string>
#ifdef LOW_MEM
#   include <map>
#else
#   include <unordered_map>
#endif
#include <Utils/vptrlist.h>
#include "std_picture.h"
#include <Graphics/size.h>

#include "font_engine_base.h"

/**
 * @brief The raster fonts engine
 */
class RasterFont final : public BaseFontEngine
{
public:
    RasterFont();
    RasterFont(const RasterFont &rf) = delete;
    RasterFont &operator=(const RasterFont &tf) = delete;
    RasterFont(RasterFont &&tf) = default;

    virtual ~RasterFont() override;

    void  loadFont(const std::string& font_ini);
    void  loadFontMap(const std::string &fontmap_ini);

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
     * \param Alpha Alpha channel level (does not issue render calls if Alpha is 0)
     * \param fontSize The size of the TTF font glyph (unused for raster fonts)
     * \param crop_info nullable pointer to info for crop (marquee) logic
     * \return Width and height of the text block in pixels
     */
    PGE_Size printText(const char* text, size_t text_size,
                       int32_t x, int32_t y,
                       XTColor color = XTColor(),
                       uint32_t fontSize = 0,
                       CropInfo* crop_info = nullptr) override;

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
    XTColor m_ttfOutlinesColour = {0, 0, 0, 255};
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

    /**
     * @brief Raster font glyph
     */
    struct RasChar
    {
        bool        valid = false; //!< Is a valid glyph
        StdPicture* tx     = nullptr; //!< Pointer to the texture that contains this glyph
        uint8_t     padding_left    = 0; //!< Crop left
        uint8_t     padding_right   = 0; //!< Crop right
        int16_t     x = 0; //!< X pixel offset
        int16_t     y = 0; //!< Y pixel offset
    };

#ifdef LOW_MEM
    typedef std::map<char32_t, RasChar > CharMap;
#else
    typedef std::unordered_map<char32_t, RasChar > CharMap;
#endif

    //! Table of available characters
    CharMap m_charMap;

    //! Bank of loaded textures
    VPtrList<StdPicture> m_texturesBank;
};

#endif // RASTER_FONT_H
