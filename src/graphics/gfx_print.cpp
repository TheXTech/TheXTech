/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2020 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <algorithm>

#include "../globals.h"
#include "../graphics.h"


void SuperPrint(std::string SuperWords, int Font, float X, float Y,
                float r, float g, float b, float a)
{
//    int A = 0;
    int B = 0;
    int C = 0;

    if(Font == 1)
    {
        for(auto c : SuperWords)
        {
            if(c >= '0' && c <= '9')
                frmMain.renderTexture(int(X + B), int(Y), 16, 14, GFX.Font1[c - '0'], 0, 0, r, g, b, a);
            B += 18;
        }
    }
    else if(Font == 2)
    {
        for(auto c : SuperWords)
        {
            if(c >= 48 && c <= 57) {
                C = (c - 48) * 16;
                frmMain.renderTexture(int(X + B), int(Y), 15, 17, GFX.Font2[1], C, 0, r, g, b, a);
                B += 16;
            } else if(c >= 65 && c <= 90) {
                C = (c - 55) * 16;
                frmMain.renderTexture(int(X + B), int(Y), 15, 17, GFX.Font2[1], C, 0, r, g, b, a);
                B += 16;
            } else if(c >= 97 && c <= 122) {
                C = (c - 61) * 16;
                frmMain.renderTexture(int(X + B), int(Y), 15, 17, GFX.Font2[1], C, 0, r, g, b, a);
                B += 16;
            } else if(c >= 33 && c <= 47) {
                C = (c - 33) * 16;
                frmMain.renderTexture(int(X + B), int(Y), 15, 17, GFX.Font2S, C, 0, r, g, b, a);
                B += 16;
            } else if(c >= 58 && c <= 64) {
                C = (c - 58 + 15) * 16;
                frmMain.renderTexture(int(X + B), int(Y), 15, 17, GFX.Font2S, C, 0, r, g, b, a);
                B += 16;
            } else if(c >= 91 && c <= 96) {
                C = (c - 91 + 22) * 16;
                frmMain.renderTexture(int(X + B), int(Y), 15, 17, GFX.Font2S, C, 0, r, g, b, a);
                B += 16;
            } else if(c >= 123 && c <= 125) {
                C = (c - 123 + 28) * 16;
                frmMain.renderTexture(int(X + B), int(Y), 15, 17, GFX.Font2S, C, 0, r, g, b, a);
                B += 16;
            } else {
                B += 16;
            }
        }

    }
    else if (Font == 3)
    {
        std::string Words = SuperWords;
        std::transform(Words.begin(), Words.end(), Words.begin(), [](unsigned char c){ return std::toupper(c); });
//        Do While Len(Words) > 0
        for(auto c : Words)
        {
//            If Asc(Left(Words, 1)) >= 33 And Asc(Left(Words, 1)) <= 126 Then
            if(c >= 33 && c <= 126)
            {
//                C = (Asc(Left(Words, 1)) - 33) * 32
                C = (c - 33) * 32;
//                BitBlt myBackBuffer, X + B, Y, 18, 16, GFX.Font2Mask(2).hdc, 2, C, vbSrcAnd
//                BitBlt myBackBuffer, X + B, Y, 18, 16, GFX.Font2(2).hdc, 2, C, vbSrcPaint
                frmMain.renderTexture(int(X + B), int(Y), 18, 16, GFX.Font2[2], 2, C, r, g, b, a);
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
//            End If
//            Words = Right(Words, Len(Words) - 1)
//        Loop
        }
//    ElseIf Font = 4 Then
    }
    else if(Font == 4)
    {
//        Do While Len(Words) > 0
        for(auto c : SuperWords)
        {
//            If Asc(Left(Words, 1)) >= 33 And Asc(Left(Words, 1)) <= 126 Then
            if(c >= 33 && c <= 126)
            {
//                C = (Asc(Left(Words, 1)) - 33) * 16
                C = (c - 33) * 16;
//                BitBlt myBackBuffer, X + B, Y, 18, 16, GFX.Font2(3).hdc, 2, C, vbSrcPaint
                frmMain.renderTexture(int(X + B), int(Y), 18, 16, GFX.Font2[3], 2, C, r, g, b, a);
//                B = B + 18
                B += 18;
//            Else
            } else {
//                B = B + 18
                B += 18;
//            End If
            }
//            Words = Right(Words, Len(Words) - 1)
//        Loop
        }
//    End If
    }
}
