#include "sdl_proxy/sdl_assert.h"

#include "globals.h"
#include "../graphics.h"
#include "core/render.h"
#include "../gfx.h"
#include "../draw_planes.h"
#include "../frame_timer.h"
#include "../config.h"

#include "fontman/font_manager_private.h"
#include "fontman/font_manager.h"

static std::string s_opt_message;
static int s_last_width = 0;
static PGE_Size s_title_dims;
static PGE_Size s_message_dims;


void PrepareMessageDims()
{
    int TextBoxW = GFX.TextBox.w;

    if(!GFX.TextBox.inited || XRender::TargetW < GFX.TextBox.w || (g_MessageType >= MESSAGE_TYPE_SYS_INFO))
        TextBoxW = XRender::TargetW - 50;

    int font = FontManager::fontIdFromSmbxFont(4);

    s_opt_message = MessageText;
    s_message_dims = FontManager::optimizeTextPx(s_opt_message, TextBoxW - 20, font);

    if(!MessageTitle.empty())
        s_title_dims = FontManager::textSize(MessageTitle.c_str(), MessageTitle.size(), font);

    s_last_width = XRender::TargetH;
}

// Now uses the modern systems
void DrawMessage()
{
    if(s_last_width != XRender::TargetH)
        PrepareMessageDims();

    int TextBoxW = GFX.TextBox.w;
    bool UseGFX = true;

    if(!GFX.TextBox.inited || XRender::TargetW < GFX.TextBox.w || (g_MessageType >= MESSAGE_TYPE_SYS_INFO))
    {
        TextBoxW = XRender::TargetW - 50;
        UseGFX = false;
    }

    int BoxY_Start = XRender::TargetH / 2 - 150;

    if(BoxY_Start < 60)
    {
        BoxY_Start = XRender::TargetH / 2 - s_message_dims.h() / 2 - 10;

        if(BoxY_Start > 60)
            BoxY_Start = 60;
    }

    // Draw the background now we know how many lines there are. 10px of padding above and below.
    int totalHeight = s_message_dims.h() + 20;

    XTColor messageColour = {8, 96, 168};
    XTColor titleColour = {255, 255, 255};

    switch(g_MessageType)
    {
    case MESSAGE_TYPE_SYS_INFO:
        messageColour = {0x1D, 0x08, 0x5E};
        titleColour = {0xf5, 0xff, 0x1a};
        break;
    case MESSAGE_TYPE_SYS_WARNING:
        messageColour = {0xBF, 0x63, 0x24};
        titleColour = {0xf5, 0xff, 0x1a};
        break;
    case MESSAGE_TYPE_SYS_ERROR:
    case MESSAGE_TYPE_SYS_FATAL_ASSERT:
        messageColour = {0x61, 0x00, 0x0F};
        titleColour = {0xf5, 0xff, 0x1a};
        break;
    default:
        messageColour = {8, 96, 168};
        break;
    }

    if(g_MessageType >= MESSAGE_TYPE_SYS_INFO && !MessageTitle.empty())
    {
        int titleBoxWidth = TextBoxW > s_title_dims.w() + 12 ? TextBoxW : s_title_dims.w() + 12;
        int titleBoxHeight = s_title_dims.h() + 20;
        int titleBoxX = (XRender::TargetW / 2) - (titleBoxWidth / 2);
        int titleBoxY = BoxY_Start - 40;

        XRender::renderRect(titleBoxX, titleBoxY,
                            titleBoxWidth, titleBoxHeight, {0, 0, 0});
        XRender::renderRect(titleBoxX + 2, titleBoxY + 2,
                            titleBoxWidth - 4, titleBoxHeight - 4, {255, 255, 255});
        XRender::renderRect(titleBoxX + 4, titleBoxY + 4,
                            titleBoxWidth - 8, titleBoxHeight - 8, messageColour);

        SuperPrint(MessageTitle, 4, (XRender::TargetW / 2) - (s_title_dims.w() / 2), titleBoxY + 10, titleColour);
    }

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
                            TextBoxW - 8, totalHeight - 8, messageColour);
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
        XRender::renderTextureBasic(XRender::TargetW / 2 - TextBoxW / 2,
                               BoxY_Start,
                               TextBoxW, 20, GFX.TextBox, 0, 0);

        // loop middle of graphics
        for(int i = 0; i < vertReps; i++)
        {
            if((i + 1) * gfxMidH <= spaceToFill)
                XRender::renderTextureBasic(XRender::TargetW / 2 - TextBoxW / 2,
                                       BoxY_Start + 20 + i * gfxMidH,
                                       TextBoxW, gfxMidH, GFX.TextBox, 0, 20);
            else
                XRender::renderTextureBasic(XRender::TargetW / 2 - TextBoxW / 2,
                                       BoxY_Start + 20 + i * gfxMidH,
                                       TextBoxW, spaceToFill - i * gfxMidH, GFX.TextBox, 0, 20);
        }

        // render bottom of graphics
        XRender::renderTextureBasic(XRender::TargetW / 2 - TextBoxW / 2,
                               BoxY_Start + 20 + spaceToFill,
                               TextBoxW, bottomPaddingHeight, GFX.TextBox, 0, GFX.TextBox.h - bottomPaddingHeight);
    }
#endif

    // PASS TWO: draw the lines
    int font = FontManager::fontIdFromSmbxFont(4);

    int left = XRender::TargetW / 2 - TextBoxW / 2 + 12;

    if(s_message_dims.h() < 30 && s_message_dims.w() < TextBoxW - 100)
        left = XRender::TargetW / 2 - s_message_dims.w() / 2;

    FontManager::printText(s_opt_message.c_str(), s_opt_message.size(),
                            left, BoxY_Start + totalHeight / 2 - s_message_dims.h() / 2,
                            font);
}

void UpdateGraphicsFatalAssert()
{
    cycleNextInc();

    if(g_config.enable_frameskip && !TakeScreen && frameSkipNeeded())
        return;

    XRender::setTargetTexture();
    XRender::resetViewport();
    XRender::setDrawPlane(PLANE_LVL_META);

    if(PrintFPS > 0 && g_config.show_fps)
        SuperPrint(std::to_string(PrintFPS), 1, XRender::TargetOverscanX + 8, 8, {0, 255, 0});

    DrawMessage();

    XRender::repaint();
}
