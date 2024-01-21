/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2024 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <Logger/logger.h>
#include "sdl_proxy/sdl_stdinc.h"

#include "core/render.h"

#include "globals.h"
#include "graphics.h"
#include "fontman/font_manager.h"
#include "fontman/font_manager_private.h"


int SuperTextPixLen(int SuperN, const char* SuperChars, int Font)
{
    if(Font == 5)
        Font = 4;

    int dFont = FontManager::fontIdFromSmbxFont(Font);

    if(dFont < 0)
    {
        int len = 0;
        pLogWarning("SuperTextPixLen: Invalid font %d is specified", Font);

        for(int i = 0; i < SuperN; ++i)
        {
            len += 18;
            i += static_cast<size_t>(trailingBytesForUTF8[static_cast<UTF8>(SuperChars[i])]);
        }

        return len;
    }

    return FontManager::textSize(SuperChars, SuperN, dFont, FontManager::fontSizeFromSmbxFont(Font)).w();
}

void SuperPrintRightAlign(int SuperN, const char* SuperChars, int Font, float X, float Y, XTColor color)
{
    int RealFont = Font;
    bool outline = false;

    if(Font == 5)
    {
        Font = 4;
        outline = true;
    }

    int dFont = FontManager::fontIdFromSmbxFont(Font);

    if(dFont >= 0)
    {
        X -= FontManager::textSize(SuperChars, SuperN, dFont, FontManager::fontSizeFromSmbxFont(Font)).w();
        FontManager::printText(SuperChars, SuperN, X, Y, dFont, color, FontManager::fontSizeFromSmbxFont(Font), outline);
        return;
    }

    X -= SuperTextPixLen(SuperN, SuperChars, Font);
    SuperPrint(SuperN, SuperChars, RealFont, X, Y, color);
}

void SuperPrintCenter(int SuperN, const char* SuperChars, int Font, float X, float Y, XTColor color)
{
    int RealFont = Font;
    bool outline = false;

    if(Font == 5)
    {
        Font = 4;
        outline = true;
    }

    int dFont = FontManager::fontIdFromSmbxFont(Font);

    if(dFont >= 0)
    {
        X -= FontManager::textSize(SuperChars, SuperN, dFont, FontManager::fontSizeFromSmbxFont(Font)).w() / 2;
        FontManager::printText(SuperChars, SuperN, X, Y, dFont, color, FontManager::fontSizeFromSmbxFont(Font), outline);
        return;
    }

    X -= SuperTextPixLen(SuperN, SuperChars, Font) / 2;
    SuperPrint(SuperN, SuperChars, RealFont, X, Y, color);
}

void SuperPrintScreenCenter(int SuperN, const char* SuperChars, int Font, float Y, XTColor color)
{
    int RealFont = Font;
    bool outline = false;

    if(Font == 5)
    {
        Font = 4;
        outline = true;
    }

    int dFont = FontManager::fontIdFromSmbxFont(Font);

    if(dFont >= 0)
    {
        float X = (XRender::TargetW / 2) - (FontManager::textSize(SuperChars, SuperN, dFont, FontManager::fontSizeFromSmbxFont(Font)).w() / 2);
        FontManager::printText(SuperChars, SuperN, X, Y, dFont, color, FontManager::fontSizeFromSmbxFont(Font), outline);
        return;
    }

    float X = (XRender::TargetW / 2) - (SuperTextPixLen(SuperN, SuperChars, Font) / 2);
    SuperPrint(SuperN, SuperChars, RealFont, X, Y, color);
}

void SuperPrint(int SuperN, const char* SuperChars, int Font, float X, float Y,
                XTColor color)
{
    bool outline = false;

    if(Font == 5)
    {
        Font = 4;
        outline = true;
    }

    int dFont = FontManager::fontIdFromSmbxFont(Font);
    if(dFont < 0)
    {
        pLogWarning("SuperPrint: Invalid font %d is specified", Font);
        return; // Invalid font specified
    }

    FontManager::printText(SuperChars, SuperN, X, Y, dFont, color, FontManager::fontSizeFromSmbxFont(Font), outline);
}

// const char* versions

int SuperTextPixLen(const char* SuperChars, int Font)
{
    int len = (int)SDL_strlen(SuperChars);
    return SuperTextPixLen(len, SuperChars, Font);
}

void SuperPrintRightAlign(const char* SuperChars, int Font, float X, float Y, XTColor color)
{
    int len = (int)SDL_strlen(SuperChars);
    SuperPrintRightAlign(len, SuperChars, Font, X, Y, color);
}

void SuperPrintCenter(const char* SuperChars, int Font, float X, float Y, XTColor color)
{
    int len = (int)SDL_strlen(SuperChars);
    SuperPrintCenter(len, SuperChars, Font, X, Y, color);
}

void SuperPrintScreenCenter(const char* SuperChars, int Font, float Y, XTColor color)
{
    int len = (int)SDL_strlen(SuperChars);
    SuperPrintScreenCenter(len, SuperChars, Font, Y, color);
}

void SuperPrint(const char* SuperChars, int Font, float X, float Y, XTColor color)
{
    int len = (int)SDL_strlen(SuperChars);
    SuperPrint(len, SuperChars, Font, X, Y, color);
}


// const std::string& versions

int SuperTextPixLen(const std::string& SuperWords, int Font)
{
    return SuperTextPixLen((int)SuperWords.size(), SuperWords.c_str(), Font);
}

void SuperPrintRightAlign(const std::string& SuperWords, int Font, float X, float Y, XTColor color)
{
    SuperPrintRightAlign((int)SuperWords.size(), SuperWords.c_str(), Font, X, Y, color);
}

void SuperPrintCenter(const std::string& SuperWords, int Font, float X, float Y, XTColor color)
{
    SuperPrintCenter((int)SuperWords.size(), SuperWords.c_str(), Font, X, Y, color);
}

void SuperPrintScreenCenter(const std::string& SuperWords, int Font, float Y, XTColor color)
{
    SuperPrintScreenCenter((int)SuperWords.size(), SuperWords.c_str(), Font, Y, color);
}

void SuperPrint(const std::string& SuperWords, int Font, float X, float Y, XTColor color)
{
    SuperPrint((int)SuperWords.size(), SuperWords.c_str(), Font, X, Y, color);
}
