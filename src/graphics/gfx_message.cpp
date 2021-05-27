#include "globals.h"
#include "../graphics.h"

void DrawMessage()
{
#ifdef __3DS__
    const int TextBoxW = 500;
    const bool UseGFX = false;
#else
    int TextBoxW = GFX.TextBox.w;
    bool UseGFX = true;
    if(ScreenW < GFX.TextBox.w)
    {
        TextBoxW = ScreenW - 50;
        UseGFX = false;
    }
#endif

    const int charWidth = 18;
    const int lineHeight = 16;
    // based on Wohlstand's improved algorithm, but screen-size aware
    static std::string SuperText;
    static std::string tempText;
    SuperText = MessageText;
    int BoxY = 0;
    int BoxY_Start = ScreenH/2 - 150;
    if(BoxY_Start < 60)
        BoxY_Start = 60;
    // Draw background all at once:
    // how many lines are there?
    int lineStart = 0; // start of current line
    int lastWord = 0; // planned start of next line
    int numLines = 0; // n lines
    int maxChars = (TextBoxW-24)/charWidth+1; // 27 by default

    // PASS ONE
    // Wohlstand's updated algorithm, no substrings, reasonably fast
    while(lineStart < int(SuperText.size()))
    {
        lastWord = lineStart;
        for(int i = lineStart + 1; i <= lineStart+maxChars; i++)
        {
            auto c = SuperText[size_t(i) - 1];

            if((lastWord == lineStart && i == lineStart+maxChars) || i == int(SuperText.size()) || c == '\n')
            {
                lastWord = i;
                break;
            }
            else if(c == ' ')
            {
                lastWord = i;
            }
        }

        numLines ++;
        lineStart = lastWord;
    }

    // Draw the background now we know how many lines there are.
    if(!UseGFX)
    {
        frmMain.renderRect(ScreenW/2 - TextBoxW / 2 ,
                              BoxY_Start,
                              TextBoxW, numLines*lineHeight + 20, 0.f, 0.f, 0.f, 1.f);
        frmMain.renderRect(ScreenW/2 - TextBoxW / 2 + 2,
                              BoxY_Start + 2,
                              TextBoxW - 4, numLines*lineHeight + 20 - 4, 1.f, 1.f, 1.f, 1.f);
        frmMain.renderRect(ScreenW/2 - TextBoxW / 2 + 4,
                              BoxY_Start + 4,
                              TextBoxW - 8, numLines*lineHeight + 20 - 8, 8.f/255.f, 96.f/255.f, 168.f/255.f, 1.f);
    }
#ifndef __3DS__
    else
    {
        // carefully render the background image...
        frmMain.renderTexture(ScreenW / 2 - TextBoxW / 2,
                              BoxY_Start,
                              TextBoxW, 20, GFX.TextBox, 0, 0);
        int rndMidH = numLines*lineHeight + 20 - 20 - 20;
        int gfxMidH = GFX.TextBox.h - 20 - 20;
        int vertReps = rndMidH / gfxMidH + 1;
        for (int i = 0; i < vertReps; i++)
        {
            if ((i+1) * gfxMidH <= rndMidH)
                frmMain.renderTexture(ScreenW / 2 - TextBoxW / 2,
                                      BoxY_Start + 20 + i*gfxMidH,
                                      TextBoxW, gfxMidH, GFX.TextBox, 0, 20);
            else
                frmMain.renderTexture(ScreenW / 2 - TextBoxW / 2,
                                      BoxY_Start + 20 + i*gfxMidH,
                                      TextBoxW, rndMidH - i*gfxMidH, GFX.TextBox, 0, 20);
        }
        frmMain.renderTexture(ScreenW / 2 - TextBoxW / 2,
                              BoxY_Start + 20 + rndMidH,
                              TextBoxW, 20, GFX.TextBox, 0, GFX.TextBox.h - 20);
    }
#endif
    // PASS TWO
    // Wohlstand's updated algorithm
    bool firstLine = true;
    BoxY = BoxY_Start + 10;
    do
    {
        // find last word break
        lastWord = 0;
        for(int i = 1; i <= maxChars; i++)
        {
            auto c = SuperText[size_t(i) - 1];

            if((lastWord == 0 && i == maxChars) || i == int(SuperText.size()) || c == '\n')
            {
                lastWord = i;
                break;
            }
            else if(c == ' ')
            {
                lastWord = i;
            }
        }

        tempText = SuperText.substr(0, size_t(lastWord));
        SuperText.erase(0, size_t(lastWord));
        if(SuperText.length() == 0 && firstLine)
        {
            SuperPrint(tempText,
                       4,
                       ScreenW/2 - (tempText.length() * charWidth)/2,
                       BoxY);
        }
        else
        {
            SuperPrint(tempText, 4, ScreenW/2 - TextBoxW / 2 + 12, BoxY);
        }
        BoxY += lineHeight;
        firstLine = false;
    } while(!SuperText.empty());
}