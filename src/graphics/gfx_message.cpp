#include "sdl_proxy/sdl_assert.h"

#include "globals.h"
#include "../graphics.h"
#include "core/render.h"
#include "../gfx.h"

#include "fontman/font_manager_private.h"
#include "fontman/font_manager.h"


void BuildUTF8CharMap(const std::string& SuperText, UTF8CharMap_t& outMap)
{
    const char* mapBuildIt = SuperText.c_str();
    const char* mapBuildEnd = mapBuildIt + SuperText.size();

    outMap.clear();

    // Scan the whole line including the NULL terminator
    for(; mapBuildIt <= mapBuildEnd; mapBuildIt++)
    {
        outMap.push_back(mapBuildIt);
        UTF8 ucx = static_cast<unsigned char>(*mapBuildIt);
        mapBuildIt += static_cast<size_t>(trailingBytesForUTF8[ucx]);
    }
}

void DrawMessage(const std::string& SuperText)
{
    UTF8CharMap_t SuperTextMap;
    BuildUTF8CharMap(SuperText, SuperTextMap);
    DrawMessage(SuperTextMap);
}

// based on Wohlstand's improved algorithm, but screen-size aware
void DrawMessage(const UTF8CharMap_t &SuperTextMap)
{
    int TextBoxW = GFX.TextBox.w;
    bool UseGFX = true;

    if(!GFX.TextBox.inited || XRender::TargetW < GFX.TextBox.w)
    {
        TextBoxW = XRender::TargetW - 50;
        UseGFX = false;
    }

    // possibly, load these data from the fonts engine
    const int charWidth = 18;
    const int lineHeight = 16;

    int BoxY = 0;
    int BoxY_Start = XRender::TargetH / 2 - 150;

    if(BoxY_Start < 60)
        BoxY_Start = 60;

    // Draw background all at once:
    // how many lines are there?
    int lineStart = 0; // start of current line
    int lastWord = 0; // planned start of next line
    int numLines = 0; // n lines
    const int maxChars = ((TextBoxW - 24) / charWidth) + 1; // 27 chars wide by default
    // Text size without a NULL terminator
    const int textSize = static_cast<int>(SuperTextMap.size() - 1);


    // PASS ONE: determine the number of lines
    // Wohlstand's updated algorithm, no substrings, reasonably fast
    while(lineStart < textSize)
    {
        lastWord = lineStart;

        for(int i = lineStart + 1; i <= lineStart + maxChars; i++)
        {
            auto c = *(SuperTextMap[size_t(i) - 1]);

            if((lastWord == lineStart && i == lineStart + maxChars) || i == textSize || c == '\n')
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

    // Draw the background now we know how many lines there are. 10px of padding above and below.
    int totalHeight = numLines * lineHeight + 20;

    if(!UseGFX)
    {
        XRender::renderRect(XRender::TargetW / 2 - TextBoxW / 2,
                            BoxY_Start,
                            TextBoxW, totalHeight, {0, 0, 0});
        XRender::renderRect(XRender::TargetW / 2 - TextBoxW / 2 + 2,
                            BoxY_Start + 2,
                            TextBoxW - 4, totalHeight - 4, {255, 255, 255});
        XRender::renderRect(XRender::TargetW / 2 - TextBoxW / 2 + 4,
                            BoxY_Start + 4,
                            TextBoxW - 8, totalHeight - 8, {8, 96, 168});
    }

#ifndef BUILT_IN_TEXTBOX
    else
    {
        // amount of space to fill
        int spaceToFill = totalHeight - 20 - 20;

        // amount of middle GFX that gets looped. 20px in SMBX64.
        int gfxMidH = GFX.TextBox.h - 20 - 20;

        // number of reps needed to fill space
        int vertReps = spaceToFill / gfxMidH + 1;

        // want 20px of padding at bottom if possible
        int bottomPaddingHeight = 20;

        // special case where the entire message box is under 40px tall
        if(spaceToFill <= 0)
        {
            vertReps = 0;
            bottomPaddingHeight += spaceToFill;
            spaceToFill = 0;
        }

        // render top 20px of graphics
        XRender::renderTexture(XRender::TargetW / 2 - TextBoxW / 2,
                               BoxY_Start,
                               TextBoxW, 20, GFX.TextBox, 0, 0);

        // loop middle of graphics
        for(int i = 0; i < vertReps; i++)
        {
            if((i + 1) * gfxMidH <= spaceToFill)
                XRender::renderTexture(XRender::TargetW / 2 - TextBoxW / 2,
                                       BoxY_Start + 20 + i * gfxMidH,
                                       TextBoxW, gfxMidH, GFX.TextBox, 0, 20);
            else
                XRender::renderTexture(XRender::TargetW / 2 - TextBoxW / 2,
                                       BoxY_Start + 20 + i * gfxMidH,
                                       TextBoxW, spaceToFill - i * gfxMidH, GFX.TextBox, 0, 20);
        }

        // render bottom of graphics
        XRender::renderTexture(XRender::TargetW / 2 - TextBoxW / 2,
                               BoxY_Start + 20 + spaceToFill,
                               TextBoxW, bottomPaddingHeight, GFX.TextBox, 0, GFX.TextBox.h - bottomPaddingHeight);
    }
#endif

    // PASS TWO: draw the lines
    // Wohlstand's updated algorithm
    // modified to not allocate/copy a bunch of strings
    bool firstLine = true;
    BoxY = BoxY_Start + 10;
    lineStart = 0; // start of current line

    while(lineStart < textSize)
    {
        lastWord = lineStart;

        for(int i = lineStart + 1; i <= lineStart + maxChars; i++)
        {
            auto c = *(SuperTextMap[size_t(i) - 1]);

            if((lastWord == lineStart && i == lineStart + maxChars) || i == textSize || c == '\n')
            {
                lastWord = i;
                break;
            }
            else if(c == ' ')
            {
                lastWord = i;
            }
        }

        intptr_t lastWordPtr = intptr_t(SuperTextMap[lastWord]);
        intptr_t lineStartPtr = intptr_t(SuperTextMap[lineStart]);
        std::ptrdiff_t lineLenU = lastWordPtr - lineStartPtr;

        SDL_assert_release(lastWordPtr >= lineStartPtr);

        if(lastWord == textSize && firstLine)
        {
            SuperPrint(lineLenU, SuperTextMap[lineStart],
                       4,
                       XRender::TargetW / 2 - ((lastWord - lineStart) * charWidth) / 2,
                       BoxY);
        }
        else
        {
            SuperPrint(lineLenU, SuperTextMap[lineStart],
                       4,
                       XRender::TargetW / 2 - TextBoxW / 2 + 12,
                       BoxY);
        }

        lineStart = lastWord;
        BoxY += lineHeight;
        firstLine = false;
    }
}
