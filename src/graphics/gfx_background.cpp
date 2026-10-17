/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2026 Vitaly Novichkov <admin@wohlnet.ru>
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

#include "../globals.h"
#include "../graphics.h"
#include "../collision.h"
#include "../core/render.h"
#include "config.h"
#include "graphics/gfx_background_xt.h"

void DrawBackgroundColor(int A, int Z, bool lower = false)
{
    XRender::lazyPreLoad(GFXBackground2[A]);
    if(lower)
    {
        XRender::renderRect(0, 0, vScreen[Z].Width, vScreen[Z].Height,
            GFXBackground2[A].ColorLower);
    }
    else
    {
        XRender::renderRect(0, 0, vScreen[Z].Width, vScreen[Z].Height,
            GFXBackground2[A].ColorUpper);
    }
}

// draws backgrounds _, _, _, _, _
void DrawTopAnchoredBackground(int S, int Z, int A, int offset = 32, int expected_height = 0, int tile_bottom = 0, int h_num = 2)
{
    DrawBackgroundColor(A, Z, true);

    uint16_t top_anchor_layer_flags = (XTBackgroundLayer::REPEAT_LEFT | XTBackgroundLayer::REPEAT_RIGHT);
    XTBackgroundLayer layer{0, 0, 0, 0, 0, 0, 0, 0, (uint8_t)(100 - h_num * 25), 100, 0, 0, 0, 0, 0, 0, top_anchor_layer_flags};

    if(GameMenu)
    {
        int section_height = LevelREAL[S].Height - LevelREAL[S].Y;
        int screen_offset = vScreen[Z].Height - section_height;
        if(screen_offset > 0)
            offset = screen_offset;
    }

    layer.offsetY = -offset;
    DrawXTBackgroundLayer(GFXBackground2[A], layer, S, Z);

    if(g_config.disable_background2_tiling)
        return;

    if(expected_height != 0 && GFXBackground2[A].h != expected_height)
        return;

    layer.offsetY += GFXBackground2[A].h;

    if(tile_bottom != 0)
    {
        if(GFXBackground2[A].h != expected_height)
            return;
        layer.sourceY = GFXBackground2[A].h - tile_bottom;
        layer.sourceHeight = tile_bottom;
    }

    layer.flags |= XTBackgroundLayer::REPEAT_BOTTOM;
    DrawXTBackgroundLayer(GFXBackground2[A], layer, S, Z);
}

void DrawCenterAnchoredBackground(int S, int Z, int A, int expected_height = 0, int tile_bottom = 0, int tile_top = 0, bool flip_tile = false, int h_num = 2, bool anim = false)
{
    uint16_t center_anchor_layer_flags = (XTBackgroundLayer::ALIGN_CENTERY | XTBackgroundLayer::ALIGN_BOTTOM_OR_FITY
        | XTBackgroundLayer::FALLBACK_BOTTOM | XTBackgroundLayer::REPEAT_LEFT | XTBackgroundLayer::REPEAT_RIGHT);

    if(flip_tile)
        center_anchor_layer_flags |= XTBackgroundLayer::REPEAT_FLIPY;

    XTBackgroundLayer layer{0, 0, 0, 0, 0, 0, 0, 0, (uint8_t)(100 - h_num * 25), 100, 0, 0, 0, 0, 0, 0, center_anchor_layer_flags};

    // decide whether to tile
    bool no_tiling = g_config.disable_background2_tiling;
    if((tile_bottom != 0 || tile_top != 0 || expected_height != 0) && GFXBackground2[A].h != expected_height)
    {
        // HACK: don't cancel the tiling if we have the slightly incorrect original asset
        if(!(A == 42 && GFXBackground2[A].h == expected_height - 1))
            no_tiling = true;
    }

    // use the remaining information from the tempLocation (Y and Height, never tiled)
    if(no_tiling)
    {
        XRender::renderRect(0, 0, vScreen[Z].Width, vScreen[Z].Height / 2,
            GFXBackground2[A].ColorUpper);
        XRender::renderRect(0, vScreen[Z].Height / 2, vScreen[Z].Width, vScreen[Z].Height / 2,
            GFXBackground2[A].ColorLower);
    }

    // in this case, duplicate top line of frame 3
    bool bg42_workaround = (A == 42 && GFXBackground2[A].h == expected_height - 1 && anim);

    // handle animation
    unsigned frameH = (unsigned)GFXBackground2[A].h;
    // HACK: align non-rounded pictures (there was Redigit's original with the 3455 pixels height,
    // but it must be 3456. There are lot of custom resources that using the 3455 height by mistake)
    // in the original image, the fourth frame is missing its top line.
    if(A == 42 && GFXBackground2[A].h == expected_height - 1 && anim)
        frameH = expected_height / 4;
    else if(anim)
        frameH = (unsigned)GFXBackground2[A].h / 4;

    if(!tile_bottom)
        tile_bottom = frameH;

    if(!tile_top)
        tile_top = frameH;


    // draw the main frame first
    layer.sourceHeight = frameH;
    layer.unitHeight = frameH;
    layer.sourceY = 0;

    if(anim)
        layer.sourceY += frameH * SpecialFrame[3];

    // duplicate top row if needed
    if(bg42_workaround && SpecialFrame[3] == 3)
    {
        layer.sourceHeight = 1;
        DrawXTBackgroundLayer(GFXBackground2[A], layer, S, Z);
        layer.inUnitOffsetY = 1;
        layer.sourceHeight = frameH - 1;
    }

    DrawXTBackgroundLayer(GFXBackground2[A], layer, S, Z);

    // now, draw the top portion
    if(!no_tiling && tile_top)
    {
        layer.sourceHeight = tile_top;
        layer.flags = center_anchor_layer_flags | XTBackgroundLayer::REPEAT_TOP | XTBackgroundLayer::REPEAT_STRIPY;

        // skip top row if needed
        if(bg42_workaround)
        {
            layer.inUnitOffsetY = 1;
            layer.sourceHeight -= 1;
            if(SpecialFrame[3] != 3)
                layer.sourceY += 1;
        }

        DrawXTBackgroundLayer(GFXBackground2[A], layer, S, Z);
    }

    // now, draw the bottom portion
    if(!no_tiling && tile_bottom)
    {
        if(bg42_workaround)
            layer.sourceY -= 1;

        layer.sourceHeight = tile_bottom;
        layer.sourceY += (frameH - tile_bottom);
        layer.inUnitOffsetY = (frameH - tile_bottom);
        layer.flags = center_anchor_layer_flags | XTBackgroundLayer::REPEAT_BOTTOM | XTBackgroundLayer::REPEAT_STRIPY;

        DrawXTBackgroundLayer(GFXBackground2[A], layer, S, Z);
    }
}

void DrawBottomAnchoredBackground(int S, int Z, int A, int offset = 0, int expected_height = 0, int tile_top = 0, int h_parallax_num = 2, bool no_bg = false, bool anim = false)
{
    if(!no_bg)
        DrawBackgroundColor(A, Z, false);

    uint16_t bottom_anchor_layer_flags = (XTBackgroundLayer::ALIGN_BOTTOM_OR_FITY | XTBackgroundLayer::REPEAT_LEFT | XTBackgroundLayer::REPEAT_RIGHT);

    XTBackgroundLayer layer{0, 0, 0, 0, 0, 0, 0, 0, (uint8_t)(100 - h_parallax_num * 25), 100, 0, 0, 0, 8, 0, 0, bottom_anchor_layer_flags};

    if(anim)
        layer.frames = 4;

    layer.offsetY = -offset;

    DrawXTBackgroundLayer(GFXBackground2[A], layer, S, Z);

    if(g_config.disable_background2_tiling)
        return;

    if(expected_height != 0 && GFXBackground2[A].h != expected_height)
        return;

    if(tile_top != 0 && GFXBackground2[A].h != expected_height)
        return;

    int frameH = GFXBackground2[A].h;
    if(anim)
        frameH /= 4;

    layer.offsetY += -frameH;
    layer.frameStride = frameH;
    if(tile_top != 0)
        layer.sourceHeight = tile_top;

    layer.flags |= XTBackgroundLayer::REPEAT_TOP;

    DrawXTBackgroundLayer(GFXBackground2[A], layer, S, Z);
}

void DrawBackground(int S, int Z)
{
    const Screen_t& screen = Screens[vScreen[Z].screen_ref];

    int Eff_ScreenH = vScreen[Z].Height;
    if(screen.Type == ScreenTypes::Dynamic && (screen.DType == DScreenTypes::TopBottom || screen.DType == DScreenTypes::BottomTop))
        Eff_ScreenH *= 2;

    if(Background2[S] == 0)
        XRender::renderRect(0, 0, vScreen[Z].Width, vScreen[Z].Height, {0, 0, 0});
    else if(Background2[S] == 1)
    {
        // Clouds
        DrawBottomAnchoredBackground(S, Z, 2, 500, 0, 0, 3);
        // Blocks
        DrawBottomAnchoredBackground(S, Z, 1, 0, -1, 0, 2, true);
    }
    else if(Background2[S] == 13)
    {
        // Clouds
        DrawCenterAnchoredBackground(S, Z, 2, 0, 0, 0, false, 3);
    }
    else if(Background2[S] == 2)
    {
        // Clouds
        DrawBottomAnchoredBackground(S, Z, 2, 500, 0, 0, 3);
        // Hills
        DrawBottomAnchoredBackground(S, Z, 3, 0, -1, 0, 2, true);
    }
    else if(Background2[S] == 3)
    {
        // Castle
        DrawCenterAnchoredBackground(S, Z, 4);
    }
    else if(Background2[S] == 4)
    {
        // Pipes
        const uint16_t layer_flags = (XTBackgroundLayer::ALIGN_CENTERX | XTBackgroundLayer::ALIGN_RIGHT_OR_FITX
            | XTBackgroundLayer::REPEAT_TOP | XTBackgroundLayer::REPEAT_BOTTOM | XTBackgroundLayer::REPEAT_LEFT | XTBackgroundLayer::REPEAT_RIGHT);
        const XTBackgroundLayer bg4_layer{(uint16_t)(GFXBackground2[5].h & ~1), 0, 0, 0, 0, -32, 0, 0, 0, 50, 0, 0, 0, 0, 0, 0, layer_flags};

        DrawXTBackgroundLayer(GFXBackground2[5], bg4_layer, S, Z);
    }
    else if(Background2[S] == 5)
    {
        // Trees
        DrawCenterAnchoredBackground(S, Z, 6, -1);
    }
    else if(Background2[S] == 6)
    {
        // Bonus
        DrawCenterAnchoredBackground(S, Z, 7);
    }
    else if(Background2[S] == 7)
    {
        // Underground (S1)
        DrawTopAnchoredBackground(S, Z, 8);
    }
    else if(Background2[S] == 8)
    {
        // Night
        DrawCenterAnchoredBackground(S, Z, 9, -1, 0, 0, false, 3);
    }
    else if(Background2[S] == 9)
    {
        // Night
        DrawCenterAnchoredBackground(S, Z, 9, -1, 0, 0, false, 3);
        // Night 2
        DrawBottomAnchoredBackground(S, Z, 10, 0, -1, 0, 2, true);
    }
    else if(Background2[S] == 10)
    {
        // Overworld
        DrawCenterAnchoredBackground(S, Z, 11, -1);
    }
    else if(Background2[S] == 11)
    {
        // Hills (S4)
        DrawCenterAnchoredBackground(S, Z, 12, -1);
    }
    else if(Background2[S] == 12)
    {
        // Trees (S4)
        DrawCenterAnchoredBackground(S, Z, 13, 800, 0, 0, true);
    }
    else if(Background2[S] == 14)
    {
        // Desert (S3)
        // could possibly tile
        DrawCenterAnchoredBackground(S, Z, 14, -1);
    }
    else if(Background2[S] == 15)
    {
        // Dungeon (S3)
        DrawCenterAnchoredBackground(S, Z, 15);
    }
    else if(Background2[S] == 16)
    {
        // Crateria
        DrawCenterAnchoredBackground(S, Z, 16);
    }
    else if(Background2[S] == 17)
    {
        // S3 ship
        // top segment is 254px tall
        DrawTopAnchoredBackground(S, Z, 17, 32, 858, 858-254);
    }
    else if(Background2[S] == 18)
    {
        // ghost house (S4)
        DrawCenterAnchoredBackground(S, Z, 18, 0, 0, 0, false, 2, true);
    }
    else if(Background2[S] == 19)
    {
        // forest (S4)
        DrawCenterAnchoredBackground(S, Z, 19, 1200, 0, 0, true);
    }
    else if(Background2[S] == 20)
    {
        // forest (S3)
        DrawCenterAnchoredBackground(S, Z, 20, -1);
    }
    else if(Background2[S] == 21)
    {
        // battle game (S3)
        DrawCenterAnchoredBackground(S, Z, 21);
    }
    else if(Background2[S] == 22)
    {
        // Clouds
        DrawBottomAnchoredBackground(S, Z, 2, 500, 0, 0, 3);
        // Waterfall (S3)
        DrawBottomAnchoredBackground(S, Z, 22, 0, -1, 0, 2, true, true);
    }
    else if(Background2[S] == 23)
    {
        // Tank (S3)
        DrawCenterAnchoredBackground(S, Z, 23, -1);
    }
    else if(Background2[S] == 24)
    {
        // villain's castle (S3)
        DrawTopAnchoredBackground(S, Z, 24, 32, 1504, 384);
    }
    else if(Background2[S] == 25)
    {
        // Underground (S2)
        const uint16_t layer_flags = (XTBackgroundLayer::ALIGN_CENTERX | XTBackgroundLayer::ALIGN_RIGHT_OR_FITX
            | XTBackgroundLayer::REPEAT_TOP | XTBackgroundLayer::REPEAT_BOTTOM | XTBackgroundLayer::REPEAT_LEFT | XTBackgroundLayer::REPEAT_RIGHT);
        static const XTBackgroundLayer bg25_layer{0, 0, 0, 0, 0, -32, 0, 0, 0, 50, 0, 0, 0, 0, 0, 0, layer_flags};

        DrawXTBackgroundLayer(GFXBackground2[25], bg25_layer, S, Z);
    }
    else if(Background2[S] == 26)
    {
        // Civilian's House
        // top segment is 244px tall
        DrawTopAnchoredBackground(S, Z, 26, 32, 1396, 1396-244);
    }
    else if(Background2[S] == 27)
    {
        // Castle (S3)
        // can be tiled; top section is 1000px
        DrawCenterAnchoredBackground(S, Z, 27, -1);
    }
    else if(Background2[S] == 28)
    {
        // Bonus (S4)
        DrawCenterAnchoredBackground(S, Z, 28);
    }
    else if(Background2[S] == 29)
    {
        // Night (S4)
        DrawCenterAnchoredBackground(S, Z, 29, -1, 0, 0, false, 2, true);
    }
    else if(Background2[S] == 30)
    {
        // Cave (S4)
        DrawCenterAnchoredBackground(S, Z, 30, 3456, 0, 0, true, 2, true);
    }
    else if(Background2[S] == 31)
    {
        // Hills 2 (S4)
        DrawCenterAnchoredBackground(S, Z, 31, -1);
    }
    else if(Background2[S] == 32)
    {
        // Clouds (S4)
        DrawCenterAnchoredBackground(S, Z, 32, -1);
    }
    else if(Background2[S] == 33)
    {
        // Snow (S4)
        DrawCenterAnchoredBackground(S, Z, 33, -1);
    }
    else if(Background2[S] == 34)
    {
        // Hills 3 (S4)
        DrawCenterAnchoredBackground(S, Z, 34, -1);
    }
    else if(Background2[S] == 35)
    {
        // Snow Clouds
        DrawBottomAnchoredBackground(S, Z, 36, 500, 0, 0, 3);
        // Snow Trees (S3)
        DrawBottomAnchoredBackground(S, Z, 35, 0, -1, 0, 2, true);
    }
    else if(Background2[S] == 36)
    {
        // Snow Clouds
        DrawCenterAnchoredBackground(S, Z, 36, 0, 0, 0, false, 3);
    }
    else if(Background2[S] == 37)
    {
        // Snow Clouds
        DrawBottomAnchoredBackground(S, Z, 36, 500, 0, 0, 3);
        // Snow Hills (S3)
        DrawBottomAnchoredBackground(S, Z, 37, 0, -1, 0, 2, true);
    }
    else if(Background2[S] == 38)
    {
        // Cave with Sky (S3)
        // each of the lower cave segments are 428px
        DrawTopAnchoredBackground(S, Z, 38, 20, 1500, 428*2);
    }
    else if(Background2[S] == 39)
    {
        // Cave no Sky (S3)
        DrawCenterAnchoredBackground(S, Z, 39);
    }
    else if(Background2[S] == 40)
    {
        // Secret Mine
        if(g_config.disable_background2_tiling)
        {
            XRender::lazyPreLoad(GFXBackground2[40]);
            XRender::renderRect(0, 0, vScreen[Z].Width, vScreen[Z].Height,
                GFXBackground2[40].ColorUpper);
        }

        const uint16_t layer_flags = (XTBackgroundLayer::ALIGN_CENTERY | XTBackgroundLayer::ALIGN_BOTTOM_OR_FITY | XTBackgroundLayer::FALLBACK_BOTTOM
            | XTBackgroundLayer::REPEAT_TOP | XTBackgroundLayer::REPEAT_BOTTOM | XTBackgroundLayer::REPEAT_LEFT | XTBackgroundLayer::REPEAT_RIGHT);
        static const std::array<XTBackgroundLayer, 16> bg40_layers =
        {
            XTBackgroundLayer{47, 0, 953, 0, 0, 0, 1000, 953, 50, 0, 0, 0, 0, 0, 0, 0, layer_flags},
            XTBackgroundLayer{37, 0, 916, 0, 0, 0, 1000, 916, 40, 0, 0, 0, 0, 0, 0, 0, layer_flags},
            XTBackgroundLayer{67, 0, 849, 0, 0, 0, 1000, 849, 30, 0, 0, 0, 0, 0, 0, 0, layer_flags},
            XTBackgroundLayer{34, 0, 815, 0, 0, 0, 1000, 815, 20, 0, 0, 0, 0, 0, 0, 0, layer_flags},
            XTBackgroundLayer{106, 0, 709, 0, 0, 0, 1000, 709, 10, 0, 0, 0, 0, 0, 0, 0, layer_flags},
            XTBackgroundLayer{45, 0, 664, 0, 0, 0, 1000, 664, 15, 0, 0, 0, 0, 0, 0, 0, layer_flags},
            XTBackgroundLayer{50, 0, 614, 0, 0, 0, 1000, 614, 20, 0, 0, 0, 0, 0, 0, 0, layer_flags},
            XTBackgroundLayer{74, 0, 540, 0, 0, 0, 1000, 540, 25, 0, 0, 0, 0, 0, 0, 0, layer_flags},
            XTBackgroundLayer{132, 0, 408, 0, 0, 0, 1000, 408, 30, 0, 0, 0, 0, 0, 0, 0, layer_flags},
            XTBackgroundLayer{75, 0, 333, 0, 0, 0, 1000, 333, 25, 0, 0, 0, 0, 0, 0, 0, layer_flags},
            XTBackgroundLayer{55, 0, 278, 0, 0, 0, 1000, 278, 20, 0, 0, 0, 0, 0, 0, 0, layer_flags},
            XTBackgroundLayer{43, 0, 235, 0, 0, 0, 1000, 235, 15, 0, 0, 0, 0, 0, 0, 0, layer_flags},
            XTBackgroundLayer{112, 0, 123, 0, 0, 0, 1000, 123, 10, 0, 0, 0, 0, 0, 0, 0, layer_flags},
            XTBackgroundLayer{38, 0, 85, 0, 0, 0, 1000, 85, 20, 0, 0, 0, 0, 0, 0, 0, layer_flags},
            XTBackgroundLayer{37, 0, 48, 0, 0, 0, 1000, 48, 30, 0, 0, 0, 0, 0, 0, 0, layer_flags},
            XTBackgroundLayer{48, 0, 0, 0, 0, 0, 1000, 0, 40, 0, 0, 0, 0, 0, 0, 0, layer_flags},
        };

        for(const XTBackgroundLayer& layer : bg40_layers)
            DrawXTBackgroundLayer(GFXBackground2[40], layer, S, Z);

    }
    else if(Background2[S] == 41)
    {
        // Castle (S1)
        DrawCenterAnchoredBackground(S, Z, 41);
    }
    else if(Background2[S] == 42)
    {
        // Castle (S4)
        // safe to tile top 546px and bottom 160px of vanilla asset
        DrawCenterAnchoredBackground(S, Z, 42, 3456, 160, 546, false, 2, true);
    }
    else if(Background2[S] == 43)
    {
        // Castle 2 (S4)
        DrawCenterAnchoredBackground(S, Z, 43);
    }
    else if(Background2[S] == 44)
    {
        // Castle (S2)
        DrawCenterAnchoredBackground(S, Z, 44);
    }
    else if(Background2[S] == 45)
    {
        // "Brinstar"
        DrawCenterAnchoredBackground(S, Z, 45);
    }
    else if(Background2[S] == 46)
    {
        // "Transport"
        DrawCenterAnchoredBackground(S, Z, 46);
    }
    else if(Background2[S] == 47)
    {
        // "Transport"
        DrawCenterAnchoredBackground(S, Z, 47);
    }
    else if(Background2[S] == 48)
    {
        // Blouds (S2)
        DrawCenterAnchoredBackground(S, Z, 48, -1);
    }
    else if(Background2[S] == 49)
    {
        // Desert Night
        // use a simple color fill for the sky above and sand below the texture
        XRender::lazyPreLoad(GFXBackground2[49]);
        XRender::renderRect(0, 0, vScreen[Z].Width, vScreen[Z].Height / 2,
            GFXBackground2[49].ColorUpper);
        XRender::renderRect(0, vScreen[Z].Height / 2, vScreen[Z].Width, vScreen[Z].Height / 2,
            GFXBackground2[49].ColorLower);

        const uint16_t layer_flags = (XTBackgroundLayer::ALIGN_CENTERY | XTBackgroundLayer::ALIGN_BOTTOM_OR_FITY
            | XTBackgroundLayer::FALLBACK_BOTTOM | XTBackgroundLayer::REPEAT_LEFT | XTBackgroundLayer::REPEAT_RIGHT);
        static const std::array<XTBackgroundLayer, 9> bg49_layers =
        {
            XTBackgroundLayer{450, 0, 280, 0, 0, 0, 730, 280, 50, 0, 0, 0, 0, 0, 0, 0, layer_flags},
            XTBackgroundLayer{12, 0, 268, 0, 0, 0, 730, 268, 10, 0, 0, 0, 0, 0, 0, 0, layer_flags},
            XTBackgroundLayer{24, 0, 244, 0, 0, 0, 730, 244, 11, 0, 0, 0, 0, 0, 0, 0, layer_flags},
            XTBackgroundLayer{16, 0, 228, 0, 0, 0, 730, 228, 12, 0, 0, 0, 0, 0, 0, 0, layer_flags},
            XTBackgroundLayer{32, 0, 196, 0, 0, 0, 730, 196, 13, 0, 0, 0, 0, 0, 0, 0, layer_flags},
            XTBackgroundLayer{32, 0, 164, 0, 0, 0, 730, 164, 14, 0, 0, 0, 0, 0, 0, 0, layer_flags},
            XTBackgroundLayer{48, 0, 116, 0, 0, 0, 730, 116, 15, 0, 0, 0, 0, 0, 0, 0, layer_flags},
            XTBackgroundLayer{58, 0, 58, 0, 0, 0, 730, 58, 16, 0, 0, 0, 0, 0, 0, 0, layer_flags},
            XTBackgroundLayer{58, 0, 0, 0, 0, 0, 730, 0, 17, 0, 0, 0, 0, 0, 0, 0, layer_flags},
        };

        for(const XTBackgroundLayer& layer : bg49_layers)
            DrawXTBackgroundLayer(GFXBackground2[49], layer, S, Z);
    }
    else if(Background2[S] == 50)
    {
        // Shrooms
        // use a simple color fill for the sky above and sand below the texture
        XRender::lazyPreLoad(GFXBackground2[50]);
        XRender::renderRect(0, 0, vScreen[Z].Width, vScreen[Z].Height / 2,
            GFXBackground2[50].ColorUpper);
        XRender::renderRect(0, vScreen[Z].Height / 2, vScreen[Z].Width, vScreen[Z].Height / 2,
            GFXBackground2[50].ColorLower);

        const uint16_t layer_flags = (XTBackgroundLayer::ALIGN_CENTERY | XTBackgroundLayer::ALIGN_BOTTOM_OR_FITY
            | XTBackgroundLayer::FALLBACK_BOTTOM | XTBackgroundLayer::REPEAT_LEFT | XTBackgroundLayer::REPEAT_RIGHT);
        static const std::array<XTBackgroundLayer, 4> bg50_layers =
        {
            // XTBackgroundLayer{32, 0, 724, 0, 0, 0, 756, 724, 50, 0, 0, 0, 0, 0, 0, 0, layer_flags | XTBackgroundLayer::REPEAT_BOTTOM | XTBackgroundLayer::REPEAT_STRIPY | XTBackgroundLayer::REPEAT_FLIPY},
            XTBackgroundLayer{378, 0, 378, 0, 0, 0, 756, 378, 50, 0, 0, 0, 0, 0, 0, 0, layer_flags},
            XTBackgroundLayer{158, 0, 220, 0, 0, 0, 756, 220, 40, 0, 0, 0, 0, 0, 0, 0, layer_flags},
            XTBackgroundLayer{220, 0, 0, 0, 0, 0, 756, 0, 35, 0, 0, 0, 0, 0, 0, 0, layer_flags | XTBackgroundLayer::REPEAT_TOP | XTBackgroundLayer::REPEAT_STRIPY},
        };

        for(const XTBackgroundLayer& layer : bg50_layers)
            DrawXTBackgroundLayer(GFXBackground2[50], layer, S, Z);
    }
    else if(Background2[S] == 51)
    {
        // Desert (S1)
        // use a simple color fill for the sky above and sand below the texture
        XRender::lazyPreLoad(GFXBackground2[51]);
        XRender::renderRect(0, vScreen[Z].Height / 2, vScreen[Z].Width, vScreen[Z].Height / 2,
            GFXBackground2[51].ColorLower);

        uint16_t layer_flags = (XTBackgroundLayer::REPEAT_LEFT | XTBackgroundLayer::REPEAT_RIGHT);
        if(GFXBackground2[51].h >= Eff_ScreenH)
            layer_flags |= (XTBackgroundLayer::ALIGN_CENTERY | XTBackgroundLayer::ALIGN_BOTTOM_OR_FITY | XTBackgroundLayer::FALLBACK_BOTTOM);

        const std::array<XTBackgroundLayer, 2> bg51_layers =
        {
            XTBackgroundLayer{350, 0, 0, 0, 0, 0, (uint16_t)GFXBackground2[51].h, 0, 25, 0, 0, 0, 0, 0, 0, 0, layer_flags},
            XTBackgroundLayer{(uint16_t)(GFXBackground2[51].h - 350), 0, 350, 0, 0, 0, (uint16_t)GFXBackground2[51].h, 350, 50, 0, 0, 0, 0, 0, 0, 0, layer_flags},
        };

        for(const XTBackgroundLayer& layer : bg51_layers)
            DrawXTBackgroundLayer(GFXBackground2[51], layer, S, Z);
    }
    else if(Background2[S] == 52)
    {
        // Desert Night (S2)
        // use a simple color fill for the sky above and sand below the texture
        XRender::lazyPreLoad(GFXBackground2[52]);
        XRender::renderRect(0, 0, vScreen[Z].Width, vScreen[Z].Height / 2,
            GFXBackground2[52].ColorUpper);
        XRender::renderRect(0, vScreen[Z].Height / 2, vScreen[Z].Width, vScreen[Z].Height / 2,
            GFXBackground2[52].ColorLower);

        const uint16_t layer_flags = (XTBackgroundLayer::ALIGN_CENTERY | XTBackgroundLayer::ALIGN_BOTTOM_OR_FITY
            | XTBackgroundLayer::FALLBACK_BOTTOM | XTBackgroundLayer::REPEAT_LEFT | XTBackgroundLayer::REPEAT_RIGHT | XTBackgroundLayer::REPEAT_STRIPY);
        const uint16_t bg52h = (uint16_t)GFXBackground2[52].h;
        const uint16_t lastH = bg52h - 280;
        const std::array<XTBackgroundLayer, 12> bg52_layers =
        {
            XTBackgroundLayer{lastH, 0, 280, 0, 0, -2, bg52h, 280, 50, 0, 0, 0, 0, 0, 0, 0, layer_flags | XTBackgroundLayer::REPEAT_BOTTOM | XTBackgroundLayer::REPEAT_FLIPY},
            XTBackgroundLayer{lastH, 0, 280, 0, 0, 0, bg52h, 280, 50, 0, 0, 0, 0, 0, 0, 0, layer_flags},
            XTBackgroundLayer{12, 0, 268, 0, 0, 0, bg52h, 268, 10, 0, 0, 0, 0, 0, 0, 0, layer_flags},
            XTBackgroundLayer{24, 0, 244, 0, 0, 0, bg52h, 244, 11, 0, 0, 0, 0, 0, 0, 0, layer_flags},
            XTBackgroundLayer{16, 0, 228, 0, 0, 0, bg52h, 228, 12, 0, 0, 0, 0, 0, 0, 0, layer_flags},
            XTBackgroundLayer{32, 0, 196, 0, 0, 0, bg52h, 196, 13, 0, 0, 0, 0, 0, 0, 0, layer_flags},
            XTBackgroundLayer{32, 0, 164, 0, 0, 0, bg52h, 164, 14, 0, 0, 0, 0, 0, 0, 0, layer_flags},
            XTBackgroundLayer{48, 0, 116, 0, 0, 0, bg52h, 116, 15, 0, 0, 0, 0, 0, 0, 0, layer_flags},
            XTBackgroundLayer{58, 0, 58, 0, 0, 0, bg52h, 58, 16, 0, 0, 0, 0, 0, 0, 0, layer_flags},
            XTBackgroundLayer{58, 0, 0, 0, 0, 0, bg52h, 0, 17, 0, 0, 0, 0, 0, 0, 0, layer_flags},
            XTBackgroundLayer{58, 0, 0, 0, 3, -58, bg52h, 0, 19, 0, 0, 0, 0, 0, 0, 0, layer_flags},
            XTBackgroundLayer{58, 0, 0, 0, 5, -116, bg52h, 0, 21, 0, 0, 0, 0, 0, 0, 0, layer_flags | XTBackgroundLayer::REPEAT_TOP},
        };

        for(const XTBackgroundLayer& layer : bg52_layers)
            DrawXTBackgroundLayer(GFXBackground2[52], layer, S, Z);
    }
    else if(Background2[S] == 53)
    {
        // Cliffs
        DrawCenterAnchoredBackground(S, Z, 53, -1);
    }
    else if(Background2[S] == 54)
    {
        // Warehouse
        DrawCenterAnchoredBackground(S, Z, 54);
    }
    else if(Background2[S] == 55)
    {
        // Water (S4)
        DrawCenterAnchoredBackground(S, Z, 55, 0, 0, 0, false, 2, true);
    }
    else if(Background2[S] == 56)
    {
        // Water (S3)
        // use a simple color fill for the sky above and sand below the texture
        XRender::lazyPreLoad(GFXBackground2[56]);
        XRender::renderRect(0, 0, vScreen[Z].Width, vScreen[Z].Height / 2,
            GFXBackground2[56].ColorUpper);
        XRender::renderRect(0, vScreen[Z].Height / 2, vScreen[Z].Width, vScreen[Z].Height / 2,
            GFXBackground2[56].ColorLower);

        const uint16_t layer_flags = (XTBackgroundLayer::ALIGN_CENTERY | XTBackgroundLayer::ALIGN_BOTTOM_OR_FITY | XTBackgroundLayer::FALLBACK_BOTTOM
            | XTBackgroundLayer::REPEAT_LEFT | XTBackgroundLayer::REPEAT_RIGHT);
        const uint16_t bg56h = (uint16_t)GFXBackground2[56].h;
        const uint16_t lastH = bg56h - 455;
        const std::array<XTBackgroundLayer, 4> bg56_layers =
        {
            XTBackgroundLayer{lastH, 0, 455, 0, 0, 0, bg56h, 455, 50, 0, 0, 0, 0, 0, 0, 0, layer_flags | XTBackgroundLayer::REPEAT_BOTTOM | XTBackgroundLayer::REPEAT_STRIPY | XTBackgroundLayer::REPEAT_FLIPY},
            XTBackgroundLayer{110, 0, 345, 0, 0, 0, bg56h, 345, 45, 0, 0, 0, 0, 0, 0, 0, layer_flags},
            XTBackgroundLayer{245, 0, 100, 0, 0, 0, bg56h, 100, 40, 0, 0, 0, 0, 0, 0, 0, layer_flags},
            XTBackgroundLayer{100, 0, 0, 0, 0, 0, bg56h, 0, 35, 0, 0, 0, 0, 0, 0, 0, layer_flags},
        };

        for(const XTBackgroundLayer& layer : bg56_layers)
            DrawXTBackgroundLayer(GFXBackground2[56], layer, S, Z);
    }
    else if(Background2[S] == 57)
    {
        // Warehouse
        // looks more like a temple to me.
        // in vanilla asset:
        // top: can loop top 672px
        // bottom: can loop bottom 64px
        DrawCenterAnchoredBackground(S, Z, 57, 800, 64, 672);
    }
    else if(Background2[S] == 58)
    {
        // Night (S4)
        // can loop top 600px and bottom 128px of vanilla asset
        DrawCenterAnchoredBackground(S, Z, 58, 3584, 128, 600, false, 2, true);
    }
}
