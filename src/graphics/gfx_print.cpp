/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2023 Vitaly Novichkov <admin@wohlnet.ru>
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

#include <algorithm>

#include <Logger/logger.h>
#include "sdl_proxy/sdl_stdinc.h"
#include "sdl_proxy/sdl_assert.h"
#include "core/render.h"

#include "globals.h"
#include "graphics.h"
#include "gfx.h"
#include "../fontman/font_manager.h"
#include "../fontman/font_manager_private.h"


#if defined(_MSC_VER) && _MSC_VER <= 1900 // Workaround for MSVC 2015
namespace std
{
    using ::toupper;
}
#endif

int SuperTextPixLen(int SuperN, const char* SuperChars, int Font)
{
    if(Font == 5)
        Font = 4;

    int len = 0;
    int dFont = NewFontRender ? FontManager::fontIdFromSmbxFont(Font) : -1;

    if(dFont >= 0)
        return FontManager::textSize(SuperChars, SuperN, dFont, 0, false, FontManager::fontSizeFromSmbxFont(Font)).w();

    switch(Font)
    {
    default:
    case 1:
    case 4:
    {
        //len = SuperN * 18;
        for(int i = 0; i < SuperN; i++)
        {
            len += 18;
            i += static_cast<size_t>(trailingBytesForUTF8[static_cast<UTF8>(SuperChars[i])]);
        }
        break;
    }
    case 2:
    {
        //len = SuperN * 16;
        for(int i = 0; i < SuperN; i++)
        {
            len += 16;
            i += static_cast<size_t>(trailingBytesForUTF8[static_cast<UTF8>(SuperChars[i])]);
        }
        break;
    }
    case 3:
    {
        int B = 0;
        for(int i = 0; i < SuperN; i++)
        {
            char c = SuperChars[i];
            c = std::toupper(c);
            if(c >= 33 && c <= 126)
            {
                B += 18;
                if(c == 'M')
                    B += 2;
            } else {
                B += 16;
            }
            i += static_cast<size_t>(trailingBytesForUTF8[static_cast<UTF8>(c)]);
        }
        len = B;
        break;
    }
    }

    return len;
}

void SuperPrintRightAlign(int SuperN, const char* SuperChars, int Font, float X, float Y, float r, float g, float b, float a)
{
    int RealFont = Font;
    bool outline = false;

    if(Font == 5)
    {
        Font = 4;
        outline = true;
    }

    int dFont = NewFontRender ? FontManager::fontIdFromSmbxFont(Font) : -1;

    if(dFont >= 0)
    {
        X -= FontManager::textSize(SuperChars, SuperN, dFont, 0, false, FontManager::fontSizeFromSmbxFont(Font)).w();
        FontManager::printText(SuperChars, SuperN, X, Y, dFont, r, g, b, a, FontManager::fontSizeFromSmbxFont(Font), outline);
        return;
    }

    X -= SuperTextPixLen(SuperN, SuperChars, Font);
    SuperPrint(SuperN, SuperChars, RealFont, X, Y, r, g, b, a);
}

void SuperPrintCenter(int SuperN, const char* SuperChars, int Font, float X, float Y, float r, float g, float b, float a)
{
    int RealFont = Font;
    bool outline = false;

    if(Font == 5)
    {
        Font = 4;
        outline = true;
    }

    int dFont = NewFontRender ? FontManager::fontIdFromSmbxFont(Font) : -1;

    if(dFont >= 0)
    {
        X -= FontManager::textSize(SuperChars, SuperN, dFont, 0, false, FontManager::fontSizeFromSmbxFont(Font)).w() / 2;
        FontManager::printText(SuperChars, SuperN, X, Y, dFont, r, g, b, a, FontManager::fontSizeFromSmbxFont(Font), outline);
        return;
    }

    X -= SuperTextPixLen(SuperN, SuperChars, Font) / 2;
    SuperPrint(SuperN, SuperChars, RealFont, X, Y, r, g, b, a);
}

void SuperPrintScreenCenter(int SuperN, const char* SuperChars, int Font, float Y, float r, float g, float b, float a)
{
    int RealFont = Font;
    bool outline = false;

    if(Font == 5)
    {
        Font = 4;
        outline = true;
    }

    int dFont = NewFontRender ? FontManager::fontIdFromSmbxFont(Font) : -1;

    if(dFont >= 0)
    {
        float X = (ScreenW / 2) - (FontManager::textSize(SuperChars, SuperN, dFont, 0, false, FontManager::fontSizeFromSmbxFont(Font)).w() / 2);
        FontManager::printText(SuperChars, SuperN, X, Y, dFont, r, g, b, a, FontManager::fontSizeFromSmbxFont(Font), outline);
        return;
    }

    float X = (ScreenW / 2) - (SuperTextPixLen(SuperN, SuperChars, Font) / 2);
    SuperPrint(SuperN, SuperChars, RealFont, X, Y, r, g, b, a);
}

void SuperPrint(int SuperN, const char* SuperChars, int Font, float X, float Y,
                float r, float g, float b, float a)
{
    bool outline = false;

    if(Font == 5)
    {
        Font = 4;
        outline = true;
    }

    int dFont = NewFontRender ? FontManager::fontIdFromSmbxFont(Font) : -1;

    if(dFont >= 0)
    {
        FontManager::printText(SuperChars, SuperN, X, Y, dFont, r, g, b, a, FontManager::fontSizeFromSmbxFont(Font), outline);
        return;
    }

    if(outline)
    {
        // take square of a to match blend of normal text
        float outline_a = a * a;
        SuperPrint(SuperN, SuperChars, Font, X - 2, Y, 0, 0, 0, outline_a);
        SuperPrint(SuperN, SuperChars, Font, X + 2, Y, 0, 0, 0, outline_a);
        SuperPrint(SuperN, SuperChars, Font, X, Y - 2, 0, 0, 0, outline_a);
        SuperPrint(SuperN, SuperChars, Font, X, Y + 2, 0, 0, 0, outline_a);
    }

//    int A = 0;
    int B = 0;
    int C = 0;

    if(Font == 1)
    {
        for(int i = 0; i < SuperN; i++)
        {
            char c = SuperChars[i];
            if(c >= '0' && c <= '9')
                XRender::renderTexture(int(X + B), int(Y), 16, 14, GFX.Font1[c - '0'], 0, 0, r, g, b, a);
            B += 18;
            if(c == '\n')
            {
                B = 0;
                Y += 16;
            }
            i += static_cast<size_t>(trailingBytesForUTF8[static_cast<UTF8>(c)]);
        }
    }
    else if(Font == 2)
    {
        for(int i = 0; i < SuperN; i++)
        {
            char c = SuperChars[i];
            if(c >= 48 && c <= 57) {
                C = (c - 48) * 16;
                XRender::renderTexture(int(X + B), int(Y), 15, 17, GFX.Font2[1], C, 0, r, g, b, a);
                B += 16;
            } else if(c >= 65 && c <= 90) {
                C = (c - 55) * 16;
                XRender::renderTexture(int(X + B), int(Y), 15, 17, GFX.Font2[1], C, 0, r, g, b, a);
                B += 16;
            } else if(c >= 97 && c <= 122) {
                C = (c - 61) * 16;
                XRender::renderTexture(int(X + B), int(Y), 15, 17, GFX.Font2[1], C, 0, r, g, b, a);
                B += 16;
            } else if(c >= 33 && c <= 47) {
                C = (c - 33) * 16;
                XRender::renderTexture(int(X + B), int(Y), 15, 17, GFX.Font2S, C, 0, r, g, b, a);
                B += 16;
            } else if(c >= 58 && c <= 64) {
                C = (c - 58 + 15) * 16;
                XRender::renderTexture(int(X + B), int(Y), 15, 17, GFX.Font2S, C, 0, r, g, b, a);
                B += 16;
            } else if(c >= 91 && c <= 96) {
                C = (c - 91 + 22) * 16;
                XRender::renderTexture(int(X + B), int(Y), 15, 17, GFX.Font2S, C, 0, r, g, b, a);
                B += 16;
            } else if(c >= 123 && c <= 125) {
                C = (c - 123 + 28) * 16;
                XRender::renderTexture(int(X + B), int(Y), 15, 17, GFX.Font2S, C, 0, r, g, b, a);
                B += 16;
            } else {
                B += 16;
            }

            if(c == '\n')
            {
                B = 0;
                Y += 18;
            }

            i += static_cast<size_t>(trailingBytesForUTF8[static_cast<UTF8>(c)]);
        }

    }
    else if(Font == 3)
    {
//        Do While Len(Words) > 0
        for(int i = 0; i < SuperN; i++)
        {
            char c = SuperChars[i];
            c = std::toupper(c);
//            If Asc(Left(Words, 1)) >= 33 And Asc(Left(Words, 1)) <= 126 Then
            if(c >= 33 && c <= 126)
            {
//                C = (Asc(Left(Words, 1)) - 33) * 32
                C = (c - 33) * 32;
//                BitBlt myBackBuffer, X + B, Y, 18, 16, GFX.Font2Mask(2).hdc, 2, C, vbSrcAnd
//                BitBlt myBackBuffer, X + B, Y, 18, 16, GFX.Font2(2).hdc, 2, C, vbSrcPaint
                XRender::renderTexture(int(X + B), int(Y), 18, 16, GFX.Font2[2], 2, C, r, g, b, a);
//                B = B + 18
                B += 18;
//                If Left(Words, 1) = "M" Then B = B + 2
                if(c == 'M')
                    B += 2;
//            Else
            } else {
//                B = B + 16
                B += 16;
            }

            if(c == '\n')
            {
                B = 0;
                Y += 18;
            }

            i += static_cast<size_t>(trailingBytesForUTF8[static_cast<UTF8>(c)]);
//            End If
//            Words = Right(Words, Len(Words) - 1)
//        Loop
        }
//    ElseIf Font = 4 Then
    }
    else if(Font == 4)
    {
        // D_pLogDebug("Trying to draw: [%s]", SuperChars);
//        Do While Len(Words) > 0
        for(int i = 0; i < SuperN; i++)
        {
            char c = SuperChars[i];
            SDL_assert_release(c != 0);
//            If Asc(Left(Words, 1)) >= 33 And Asc(Left(Words, 1)) <= 126 Then
            if(c >= 33 && c <= 126)
            {
//                C = (Asc(Left(Words, 1)) - 33) * 16
                C = (c - 33) * 16;
//                BitBlt myBackBuffer, X + B, Y, 18, 16, GFX.Font2(3).hdc, 2, C, vbSrcPaint
                XRender::renderTexture(int(X + B), int(Y), 18, 16, GFX.Font2[3], 2, C, r, g, b, a);
//                B = B + 18
                B += 18;
//            Else
            }
            else if(c != ' ' && c != '\n' && c != '\0')
            {
                C = ('?' - 33) * 16;
                XRender::renderTexture(int(X + B), int(Y), 18, 16, GFX.Font2[3], 2, C, r, g, b, a);
//                B = B + 18
                B += 18;
//            End If
            }
            else
                B += 18;

            if(c == '\n')
            {
                B = 0;
                Y += 18;
            }

            i += static_cast<size_t>(trailingBytesForUTF8[static_cast<UTF8>(c)]);
//            Words = Right(Words, Len(Words) - 1)
//        Loop
        }
//    End If
    }
}

// const char* versions

int SuperTextPixLen(const char* SuperChars, int Font)
{
    int len = (int)SDL_strlen(SuperChars);
    return SuperTextPixLen(len, SuperChars, Font);
}

void SuperPrintRightAlign(const char* SuperChars, int Font, float X, float Y, float r, float g, float b, float a)
{
    int len = (int)SDL_strlen(SuperChars);
    SuperPrintRightAlign(len, SuperChars, Font, X, Y, r, g, b, a);
}

void SuperPrintCenter(const char* SuperChars, int Font, float X, float Y, float r, float g, float b, float a)
{
    int len = (int)SDL_strlen(SuperChars);
    SuperPrintCenter(len, SuperChars, Font, X, Y, r, g, b, a);
}

void SuperPrintScreenCenter(const char* SuperChars, int Font, float Y, float r, float g, float b, float a)
{
    int len = (int)SDL_strlen(SuperChars);
    SuperPrintScreenCenter(len, SuperChars, Font, Y, r, g, b, a);
}

void SuperPrint(const char* SuperChars, int Font, float X, float Y, float r, float g, float b, float a)
{
    int len = (int)SDL_strlen(SuperChars);
    SuperPrint(len, SuperChars, Font, X, Y, r, g, b, a);
}


// const std::string& versions

int SuperTextPixLen(const std::string& SuperWords, int Font)
{
    return SuperTextPixLen((int)SuperWords.size(), SuperWords.c_str(), Font);
}

void SuperPrintRightAlign(const std::string& SuperWords, int Font, float X, float Y, float r, float g, float b, float a)
{
    SuperPrintRightAlign((int)SuperWords.size(), SuperWords.c_str(), Font, X, Y, r, g, b, a);
}

void SuperPrintCenter(const std::string& SuperWords, int Font, float X, float Y, float r, float g, float b, float a)
{
    SuperPrintCenter((int)SuperWords.size(), SuperWords.c_str(), Font, X, Y, r, g, b, a);
}

void SuperPrintScreenCenter(const std::string& SuperWords, int Font, float Y, float r, float g, float b, float a)
{
    SuperPrintScreenCenter((int)SuperWords.size(), SuperWords.c_str(), Font, Y, r, g, b, a);
}

void SuperPrint(const std::string& SuperWords, int Font, float X, float Y, float r, float g, float b, float a)
{
    SuperPrint((int)SuperWords.size(), SuperWords.c_str(), Font, X, Y, r, g, b, a);
}
