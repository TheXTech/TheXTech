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
    RasterFont(const RasterFont &rf);
    virtual ~RasterFont();

    void  loadFont(std::string font_ini);
    void  loadFontMap(std::string fontmap_ini);

    PGE_Size textSize(const std::string& text,
                      uint32_t max_line_lenght = 0,
                      bool  cut = false, uint32_t fontSize = 14);

    void printText(const std::string &text,
                   int32_t x, int32_t y,
                   float Red=1.f, float Green=1.f, float Blue=1.f, float Alpha=1.f,
                   uint32_t fontSize = 0);

    bool isLoaded();

    std::string getFontName();

private:
    //! font is fine
    bool m_isReady;
    //! Enable borders on backup ttf font render [all unknown characters will be rendered as TTF]
    bool m_ttfBorders;
    //! Width of one letter
    uint32_t m_letterWidth;
    //! Height of one letter
    uint32_t m_letterHeight;
    //! Space between printing letters
    uint32_t m_interLetterSpace;
    //! Width of space symbol
    uint32_t m_spaceWidth;
    //! Distance between top of one line and top of next
    uint32_t m_newlineOffset;

    //! Width of font matrix
    uint32_t m_matrixWidth;
    //! Width of font matrix
    uint32_t m_matrixHeight;

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
        float l = 1.0f;//!< left
        float t = 1.0f;//!< top
        float b = 1.0f;//!< bottom
        float r = 1.0f;//!< right
    };

    typedef std::unordered_map<char32_t, RasChar > CharMap;

    //! Table of available characters
    CharMap m_charMap;

    //! Bank of loaded textures
    VPtrList<StdPicture > m_texturesBank;
};

#endif // RASTER_FONT_H
