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
#ifndef FONT_MANAGER_H
#define FONT_MANAGER_H

#include "std_picture.h"
#include <Graphics/size.h>
#include <Utils/vptrlist.h>

#include <string>
#include <vector>
#include <unordered_map>

#include "font_engine_base.h"

namespace FontManager
{
/*
 * UTF-8 helpers
 */
size_t       utf8_strlen(const char *str);
size_t       utf8_strlen(const char *str, size_t len);
size_t       utf8_strlen(const std::string &str);
/**
 * @brief Get byte lenght from character lenght from begin to utf8len
 * @param str input UTF-8 string
 * @param utf8len Desireed character lenght
 * @return Byte-lenght of desired character lenght
 */
size_t       utf8_substrlen(const std::string &str, size_t utf8len);
std::string  utf8_substr(const std::string &str, size_t utf8_begin, size_t utf8_len);
const char*  utf8_skip_begin(const char *str, size_t utf8_begin);
void         utf8_pop_back(std::string &str);
void         utf8_erase_at(std::string &str, size_t begin);
void         utf8_erase_before(std::string &str, size_t end);


//! Initialize basic support for font rendering withouth config pack
void initBasic();
//! Full initialization of font renderith include external fonts
void initFull();
//! De-Initialize font manager and clear memory
void quit();


int fontIdFromSmbxFont(int font);

/**
 * @brief Automatical font choice
 */
enum DefaultFont
{
    //! Using default TTF font
    DefaultTTF_Font = -2,
    //! Using default raster font
    DefaultRaster   = -1
};

/**
 * @brief Get pixel metrics of the text block
 * @param text Text fragment to measure
 * @param text_size The byte size of the text fragment to print
 * @param fontID Font-ID
 * @param max_line_lenght max line width
 * @param cut First line only, ignore all next lines
 * @param ttfFontPixelSize Point size for the TTF fonts
 * @return
 */
PGE_Size textSize(const char *text, size_t text_size,
                  int       fontID,
                  uint32_t  max_line_lenght = 0,
                  bool      cut = false,
                  uint32_t  ttfFontSize = 14);

/**
 * @brief Retreive Font-ID from the font name
 * @param fontName Name of the font
 * @return FontID key
 */
int      getFontID(std::string fontName);

/**
 * @brief Print the text fragment to the screen
 * @param text Text fragment to print
 * @param text_size The byte size of the text fragment to print
 * @param x X position on the screen
 * @param y Y position on the screen
 * @param font Font-ID
 * @param Red Red color level from 0.0 to 1.0
 * @param Green Green color level from 0.0 to 1.0
 * @param Blue Blue color level from 0.0 to 1.0
 * @param Alpha Alpha-channel level from 0.0 to 1.0
 * @param ttf_FontSize Point size for the TTF fonts
 */
void printText(const char* text, size_t text_size,
               int x, int y,
               int font = DefaultRaster,
               float Red = 1.0, float Green = 1.0, float Blue = 1.0, float Alpha = 1.0,
               uint32_t ttf_FontSize = 14);

/**
 * @brief Clean-Up text fragment to fit into the character width
 * @param [_inout] text Text to optimize
 * @param [_in]  max_line_lenght max line lenght in characters
 * @param [_out] numLines Total line in the text fragment after optimization
 * @param [_out] numCols Total columns in the text fragment after optimization
 */
void           optimizeText(std::string &text,
                            size_t max_line_lenght,
                            int *numLines = 0,
                            int *numCols = 0);

/**
 * @brief Strip text outing of max count of symbols
 * @param text input text
 * @param max_symbols Maximal character count limit
 * @return Begin of string which is fit into desired lenght limit
 */
std::string     cropText(std::string text, size_t max_symbols);

} //namespace FontManager

#endif // FONT_MANAGER_H
