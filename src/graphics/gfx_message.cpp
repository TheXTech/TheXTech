#include "globals.h"
#include "../graphics.h"
#include "core/render.h"
#include "../gfx.h"

// based on Wohlstand's improved algorithm, but screen-size aware
void DrawMessage(const std::string& SuperText)
{
#ifdef BUILT_IN_TEXTBOX
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

    // PASS ONE: determine the number of lines
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
    int totalHeight = numLines*lineHeight + 20;

    if(!UseGFX)
    {
        XRender::renderRect(ScreenW/2 - TextBoxW / 2 ,
                              BoxY_Start,
                              TextBoxW, totalHeight, 0.f, 0.f, 0.f, 1.f);
        XRender::renderRect(ScreenW/2 - TextBoxW / 2 + 2,
                              BoxY_Start + 2,
                              TextBoxW - 4, totalHeight - 4, 1.f, 1.f, 1.f, 1.f);
        XRender::renderRect(ScreenW/2 - TextBoxW / 2 + 4,
                              BoxY_Start + 4,
                              TextBoxW - 8, totalHeight - 8, 8.f/255.f, 96.f/255.f, 168.f/255.f, 1.f);
    }
#ifndef BUILT_IN_TEXTBOX
    else
    {
        // carefully render the background image...
        XRender::renderTexture(ScreenW / 2 - TextBoxW / 2,
                              BoxY_Start,
                              TextBoxW, 20, GFX.TextBox, 0, 0);
        int rndMidH = totalHeight - 20 - 20;
        int gfxMidH = GFX.TextBox.h - 20 - 20;
        int vertReps = rndMidH / gfxMidH + 1;

        for (int i = 0; i < vertReps; i++)
        {
            if ((i+1) * gfxMidH <= rndMidH)
                XRender::renderTexture(ScreenW / 2 - TextBoxW / 2,
                                      BoxY_Start + 20 + i*gfxMidH,
                                      TextBoxW, gfxMidH, GFX.TextBox, 0, 20);
            else
                XRender::renderTexture(ScreenW / 2 - TextBoxW / 2,
                                      BoxY_Start + 20 + i*gfxMidH,
                                      TextBoxW, rndMidH - i*gfxMidH, GFX.TextBox, 0, 20);
        }

        XRender::renderTexture(ScreenW / 2 - TextBoxW / 2,
                              BoxY_Start + 20 + rndMidH,
                              TextBoxW, 20, GFX.TextBox, 0, GFX.TextBox.h - 20);
    }
#endif

    // PASS TWO: draw the lines
    // Wohlstand's updated algorithm
    // modified to not allocate/copy a bunch of strings
    bool firstLine = true;
    BoxY = BoxY_Start + 10;
    lineStart = 0; // start of current line

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

        if(lastWord == int(SuperText.size()) && firstLine)
        {
            SuperPrint(SuperText.c_str() + size_t(lineStart), size_t(lastWord) - size_t(lineStart),
                4,
                ScreenW/2 - ((lastWord - lineStart) * charWidth)/2,
                BoxY);
        }
        else
        {
            SuperPrint(SuperText.c_str() + size_t(lineStart), size_t(lastWord) - size_t(lineStart),
                4,
                ScreenW/2 - TextBoxW / 2 + 12,
                BoxY);
        }

        lineStart = lastWord;
        BoxY += lineHeight;
        firstLine = false;
    }
}
