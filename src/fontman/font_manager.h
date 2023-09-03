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

//! Full initialization of font renderith include external fonts
void initFull();
//! De-Initialize font manager and clear memory
void quit();

/**
 * @brief Attempts to load custom fonts for the episode and for the level
 */
void loadCustomFonts();

/**
 * @brief Removes all custom fonts completely
 */
void clearAllCustomFonts();

/**
 * @brief Removes all level-wide fonts, but keeps episode-wide fonts being loaded
 */
void clearLevelFonts();

/*!
 * \brief Checks if font manager works
 * \return true when manager actually works
 */
bool isInitied();

/*!
 * \brief Checks if font manager uses legacy fonts fallback
 * \return true when manager uses legacy fonts fallback
 */
bool isLegacy();

int fontIdFromSmbxFont(int font);
uint32_t fontSizeFromSmbxFont(int font);

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
 * @brief Get the size of one glyph
 * @param utf8char The UTF-8 character to measure
 * @param charNum The row number of the character in the line (affects the size of tabulation)
 * @param fontId Font ID
 * @param ttf_fontSize Point size for the TTF fonts
 * @return
 */
PGE_Size glyphSize(const char *utf8char, uint32_t charNum, int fontId, uint32_t ttf_fontSize = 14);

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
 * @param outline Render using added outline
 */
void printText(const char* text, size_t text_size,
               int x, int y,
               int font = DefaultRaster,
               float Red = 1.0, float Green = 1.0, float Blue = 1.0, float Alpha = 1.0,
               uint32_t ttf_FontSize = 14, bool outline = false,
               float outline_r = 0.0, float outline_g = 0.0, float outline_b = 0.0);

/**
 * @brief Optimize and print the text fragment to the screen
 * @param text Text fragment to print
 * @param x X position on the screen
 * @param y Y position on the screen
 * @param max_columns The maximum line length in characters (columns)
 * @param font The Font-ID
 * @param Red Red color level from 0.0 to 1.0
 * @param Green Green color level from 0.0 to 1.0
 * @param Blue Blue color level from 0.0 to 1.0
 * @param Alpha Alpha-channel level from 0.0 to 1.0
 * @param ttf_FontSize Point size for the TTF fonts
 * @return The size of text block in columns/rows
 */
PGE_Size printTextOptiCol(std::string text,
                          int x, int y,
                          size_t max_columns,
                          int font = DefaultRaster,
                          float Red = 1.0, float Green = 1.0, float Blue = 1.0, float Alpha = 1.0,
                          uint32_t ttf_FontSize = 14);

/**
 * @brief Optimize and print the text fragment to the screen
 * @param text Text fragment to print
 * @param x X position on the screen
 * @param y Y position on the screen
 * @param max_pixels_lenght The maximum line length in pixels
 * @param font The Font-ID
 * @param Red Red color level from 0.0 to 1.0
 * @param Green Green color level from 0.0 to 1.0
 * @param Blue Blue color level from 0.0 to 1.0
 * @param Alpha Alpha-channel level from 0.0 to 1.0
 * @param ttf_FontSize Point size for the TTF fonts
 * @return The size of text block in pixels
 */
PGE_Size printTextOptiPx(std::string text,
                         int x, int y,
                         size_t max_pixels_lenght,
                         int font = DefaultRaster,
                         float Red = 1.0, float Green = 1.0, float Blue = 1.0, float Alpha = 1.0,
                         uint32_t ttf_FontSize = 14);

/**
 * @brief Optimise the given text fragment to fit it into the given columns count.
 * @param [_inout] text Text to optimize
 * @param [_in]  max_columns The maximum line length in characters (columns)
 * @return Computed size of the printed text in columns and lines
 *
 * Suitable with monospace fonts.
 */
PGE_Size       optimizeText(std::string &text, size_t max_columns);

/**
 * @brief Optimise the given text fragment to fit it into the given pixels width
 * @param [_inout] text Text to optimize (the result will be writtein back to here)
 * @param [_in]  max_pixels_lenght The maximum line length in pixels
 * @param [_in]  fontId font ID
 * @return Computed pixels size of the printed text fragment
 *
 * Can be used with any kind of the font
 */
PGE_Size    optimizeTextPx(std::string &text,
                           size_t max_pixels_lenght,
                           int fontId,
                           uint32_t ttf_FontSize = 14);

/**
 * @brief Strip text outing of max count of symbols
 * @param text input text
 * @param max_symbols Maximal character count limit
 * @return Begin of string which is fit into desired lenght limit
 */
std::string     cropText(std::string text, size_t max_symbols);

} //namespace FontManager

#endif // FONT_MANAGER_H
