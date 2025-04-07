/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2025 Vitaly Novichkov <admin@wohlnet.ru>
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

#include "sdl_proxy/sdl_stdinc.h"

#include "../globals.h"
#include "../graphics.h"
#include "../collision.h"
#include "../core/render.h"
#include "config.h"

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
    int camX = vScreen[Z].CameraAddX();
    int camY = vScreen[Z].CameraAddY();

    const auto& sect = LevelREAL[S];
    int levelX = sect.X;

    int camX_levelX = camX + levelX;
    int Left = vScreen[Z].Left;
    int offsetX = (camX_levelX * (4 - h_num) - Left * h_num) / 4;

    int offsetY = sect.Y - offset + camY;
    if(GameMenu && offsetY > 0)
        offsetY = 0;

    for(; offsetX < vScreen[Z].Width; offsetX += GFXBackground2[A].w)
    {
        if(offsetX + GFXBackground2[A].w <= 0)
            continue;

        int drawH = GFXBackground2[A].h;
        int offsetY_i = offsetY;

        while(offsetY_i < vScreen[Z].Height)
        {
            XRender::renderTextureBasic(offsetX, offsetY_i,
                GFXBackground2[A].w, drawH, GFXBackground2[A], 0, GFXBackground2[A].h - drawH);

            if(g_config.disable_background2_tiling)
                break;

            if(expected_height != 0 && GFXBackground2[A].h != expected_height)
                break;

            offsetY_i += drawH;

            if(tile_bottom != 0)
            {
                if(GFXBackground2[A].h != expected_height)
                    break;
                drawH = tile_bottom;
            }
        }
    }
}

void DrawCenterAnchoredBackground(int S, int Z, int A, int expected_height = 0, int tile_bottom = 0, int tile_top = 0, bool flip_tile = false, int h_num = 2, bool anim = false)
{
    const auto& sect = LevelREAL[S];
    const Screen_t& screen = Screens[vScreen[Z].screen_ref];
    int camX = vScreen[Z].CameraAddX();
    int camY = vScreen[Z].CameraAddY();

    int Eff_ScreenH = vScreen[Z].Height;
    int Eff_Top = 0;
    if(screen.Type == ScreenTypes::Dynamic)
    {
        Eff_Top = vScreen[Z].Top;
        Eff_ScreenH = 0;
        for(int i = screen.active_begin(); i < screen.active_end(); i++)
        {
            const auto& s = screen.vScreen(i + 1);
            if(s.Left == vScreen[Z].Left)
                Eff_ScreenH += s.Height;
        }
    }

    bool no_tiling = g_config.disable_background2_tiling;
    if((tile_bottom != 0 || tile_top != 0 || expected_height != 0) && GFXBackground2[A].h != expected_height)
    {
        // HACK: don't cancel the tiling if we have the slightly incorrect original asset
        if(!(A == 42 && GFXBackground2[A].h == expected_height - 1))
            no_tiling = true;
    }

    int frameH = GFXBackground2[A].h;
    // HACK: align non-rounded pictures (there was Redigit's original with the 3455 pixels height,
    // but it must be 3456. There are lot of custom resources that using the 3455 height by mistake)
    // in the original image, the fourth frame is missing its top line.
    if(A == 42 && GFXBackground2[A].h == expected_height - 1 && anim)
    {
        frameH = expected_height / 4;
    }
    else if(anim)
    {
        frameH = GFXBackground2[A].h / 4;
    }

    int CanvasH = frameH;
    int CanvasOffset = 0;

    // ensure that the canvas covers above and below the screen
    if(Eff_ScreenH > CanvasH)
    {
        CanvasOffset = Eff_ScreenH - CanvasH;
        CanvasH += CanvasOffset * 2;
    }

    TinyLocation_t tempLocation;

    int camX_levelX = camX + sect.X;
    int Left = vScreen[Z].Left;

    int offsetX = (camX_levelX * (4 - h_num) - Left * h_num) / 4;

    for(; offsetX < vScreen[Z].Width; offsetX += GFXBackground2[A].w)
    {
        if(offsetX + GFXBackground2[A].w <= 0)
            continue;

        tempLocation.Width = GFXBackground2[A].w;
        tempLocation.Height = frameH;

        tempLocation.X = offsetX;
        if(sect.Height - sect.Y > CanvasH)
        {
            // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - (600 - vScreen(Z).Top)) * (GFXBackground2Height(A) / 4 - (600 - vScreen(Z).Top))
            // .Y = -vScreenY(Z) - .Y
            tempLocation.Y = (-camY - Eff_Top - sect.Y) * (CanvasH - Eff_ScreenH) / (sect.Height - sect.Y - Eff_ScreenH) + Eff_Top;
            tempLocation.Y = -camY - tempLocation.Y;
            tempLocation.Y += CanvasOffset;
        }
        else if(CanvasH > frameH)
        {
            tempLocation.Y = sect.Y + (sect.Height - sect.Y - frameH) / 2;
        }
        else
            tempLocation.Y = sect.Height - frameH;

        int bottom_Y = tempLocation.Y + frameH;
        unsigned int flip = X_FLIP_NONE;
        while(tempLocation.Y + tempLocation.Height > -camY)
        {
            // HACK: place the fourth frame in the correct location if we are missing a single line
            if(A == 42 && GFXBackground2[A].h == expected_height - 1 && anim && SpecialFrame[3] == 3)
            {
                // duplicate the line
                XRender::renderTextureScaleEx(tempLocation.X, camY + tempLocation.Y,
                    GFXBackground2[A].w, 1,
                    GFXBackground2[A],
                    0, frameH * SpecialFrame[3],
                    GFXBackground2[A].w, 1,
                    0, nullptr, flip);
                // draw the frame
                XRender::renderTextureScaleEx(tempLocation.X, camY + tempLocation.Y + 1,
                    GFXBackground2[A].w, tempLocation.Height - 1,
                    GFXBackground2[A],
                    0, frameH * SpecialFrame[3],
                    GFXBackground2[A].w, tempLocation.Height - 1,
                    0, nullptr, flip);
            }
            else if(anim)
            {
                XRender::renderTextureScaleEx(tempLocation.X, camY + tempLocation.Y,
                    GFXBackground2[A].w, tempLocation.Height,
                    GFXBackground2[A],
                    0, frameH * SpecialFrame[3],
                    GFXBackground2[A].w, tempLocation.Height,
                    0, nullptr, flip);
            }
            else
            {
                XRender::renderTextureScaleEx(tempLocation.X, camY + tempLocation.Y,
                    GFXBackground2[A].w, tempLocation.Height,
                    GFXBackground2[A],
                    0, 0,
                    GFXBackground2[A].w, tempLocation.Height,
                    0, nullptr, flip);
            }

            if(no_tiling)
                break;

            if(tile_top != 0)
                tempLocation.Height = tile_top;

            tempLocation.Y -= tempLocation.Height;
            if(flip_tile)
                flip ^= X_FLIP_VERTICAL;
        }

        if(no_tiling)
            continue;

        tempLocation.Y = bottom_Y;
        if(tile_bottom != 0)
            tempLocation.Height = tile_bottom;
        else
            tempLocation.Height = frameH;

        if(flip_tile)
            flip = X_FLIP_VERTICAL;

        while(tempLocation.Y < -camY + vScreen[Z].Height)
        {
            // HACK: use the smaller frame size if we are missing a single line
            if(A == 42 && GFXBackground2[A].h == expected_height - 1 && anim && SpecialFrame[3] == 3)
            {
                XRender::renderTextureScaleEx(tempLocation.X, camY + tempLocation.Y,
                    GFXBackground2[A].w, tempLocation.Height,
                    GFXBackground2[A],
                    0, frameH * SpecialFrame[3] + (frameH - 1) - tempLocation.Height,
                    GFXBackground2[A].w, tempLocation.Height,
                    0, nullptr, flip);
            }
            else if(anim)
            {
                XRender::renderTextureScaleEx(tempLocation.X, camY + tempLocation.Y,
                    GFXBackground2[A].w, tempLocation.Height,
                    GFXBackground2[A],
                    0, frameH * SpecialFrame[3] + frameH - tempLocation.Height,
                    GFXBackground2[A].w, tempLocation.Height,
                    0, nullptr, flip);
            }
            else
            {
                XRender::renderTextureScaleEx(tempLocation.X, camY + tempLocation.Y,
                    GFXBackground2[A].w, tempLocation.Height,
                    GFXBackground2[A],
                    0, frameH - tempLocation.Height,
                    GFXBackground2[A].w, tempLocation.Height,
                    0, nullptr, flip);
            }

            tempLocation.Y += tempLocation.Height;
            if(flip_tile)
                flip ^= X_FLIP_VERTICAL;
        }
    }

    // use the remaining information from the tempLocation (Y and Height, never tiled)
    if(no_tiling)
    {
        int undrawn_above = tempLocation.Y + camY;
        int undrawn_below = tempLocation.Y + camY + tempLocation.Height;
        XRender::renderRect(0, 0, vScreen[Z].Width, undrawn_above,
            GFXBackground2[A].ColorUpper);
        XRender::renderRect(0, undrawn_below, vScreen[Z].Width, vScreen[Z].Height - undrawn_below + 1,
            GFXBackground2[A].ColorLower);
    }
}

void DrawBottomAnchoredBackground(int S, int Z, int A, int offset = 0, int expected_height = 0, int tile_top = 0, int h_parallax_num = 2, bool no_bg = false, bool anim = false)
{
    const auto& sect = LevelREAL[S];
    const Screen_t& screen = Screens[vScreen[Z].screen_ref];
    int camX = vScreen[Z].CameraAddX();
    int camY = vScreen[Z].CameraAddY();

    if(!no_bg)
        DrawBackgroundColor(A, Z, false);

    int frameH = GFXBackground2[A].h;
    if(anim)
        frameH = GFXBackground2[A].h / 4;

    TinyLocation_t tempLocation;

    int horiz_reps = ((sect.Width - sect.X) * h_parallax_num / 4 + screen.W) / GFXBackground2[A].w + 1;
    for(int B = 0; B <= horiz_reps; B++)
    {
        tempLocation.X = sect.X + ((B * GFXBackground2[A].w) - (camX + vScreen[Z].Left + sect.X) * h_parallax_num / 4);
        tempLocation.Y = sect.Height - frameH - offset;

        tempLocation.Height = frameH;
        tempLocation.Width = GFXBackground2[A].w;

        if(vScreenCollision(Z, tempLocation))
        {
            if(anim)
            {
                XRender::renderTextureBasic(camX + tempLocation.X, camY + tempLocation.Y,
                    GFXBackground2[A].w, tempLocation.Height,
                    GFXBackground2[A], 0, frameH * SpecialFrame[3]);
            }
            else
            {
                XRender::renderTextureBasic(camX + tempLocation.X, camY + tempLocation.Y,
                    GFXBackground2[A].w, tempLocation.Height,
                    GFXBackground2[A], 0, 0);
            }
        }

        if(g_config.disable_background2_tiling)
            continue;

        if(expected_height != 0 && GFXBackground2[A].h != expected_height)
            continue;

        if(tile_top != 0 && GFXBackground2[A].h != expected_height)
            continue;

        if(tile_top != 0)
            tempLocation.Height = tile_top;

        while(tempLocation.Y > sect.Y || tempLocation.Y > -camY)
        {
            tempLocation.Y -= tempLocation.Height;
            if(vScreenCollision(Z, tempLocation))
            {
                if(anim)
                {
                    XRender::renderTextureBasic(camX + tempLocation.X, camY + tempLocation.Y,
                        GFXBackground2[A].w, tempLocation.Height,
                        GFXBackground2[A], 0, frameH * SpecialFrame[3]);
                }
                else
                {
                    XRender::renderTextureBasic(camX + tempLocation.X, camY + tempLocation.Y,
                        GFXBackground2[A].w, tempLocation.Height,
                        GFXBackground2[A], 0, 0);
                }
            }
        }
    }
}

static void DrawYTiledBackground(int off_x, int off_y, int vscreen_w, int vscreen_h, StdPicture& tx)
{
    off_x %= tx.w;
    off_y %= tx.h;

    if(off_x > 0)
        off_x -= tx.w;
    if(off_y > 0)
        off_y -= tx.h;

    // Fixed an SMBX 1.3 peculiarity -- this was previously -1 rather than force-to-even
    int stride_y = tx.h & ~1;

    for(; off_y < vscreen_h; off_y += stride_y)
    {
        for(int off_x_i = off_x; off_x_i < vscreen_w; off_x_i += tx.w)
            XRender::renderTextureBasic(off_x_i, off_y, tx);
    }
}

void DrawBackground(int S, int Z)
{
    const Screen_t& screen = Screens[vScreen[Z].screen_ref];
    int camX = vScreen[Z].CameraAddX();
    int camY = vScreen[Z].CameraAddY();

    int Left = vScreen[Z].Left;

    int A = 0;

    const auto& sect = LevelREAL[S];

    int camX_levelX = camX + sect.X;

    int Eff_ScreenH = vScreen[Z].Height;
    int Eff_Top = 0;

    if(screen.Type == ScreenTypes::Dynamic)
    {
        Eff_Top = vScreen[Z].Top;
        Eff_ScreenH = 0;

        for(int i = screen.active_begin(); i < screen.active_end(); i++)
        {
            const auto& s = screen.vScreen(i + 1);
            if(s.Left == vScreen[Z].Left)
                Eff_ScreenH += s.Height;
        }
    }

    if(Background2[S] == 0)
        XRender::renderRect(0, 0, vScreen[Z].Width, vScreen[Z].Height, {0, 0, 0});

    A = 2; // Clouds
    if(Background2[S] == 1 || Background2[S] == 2 || Background2[S] == 22)
    {
        DrawBottomAnchoredBackground(S, Z, A, 500, 0, 0, 3);
    }

    if(Background2[S] == 13)
    {
        DrawCenterAnchoredBackground(S, Z, A, 0, 0, 0, false, 3);
    }

    A = 1; // Blocks
    if(Background2[S] == 1)
    {
        DrawBottomAnchoredBackground(S, Z, A, 0, -1, 0, 2, true);
    }

    A = 3; // Hills
    if(Background2[S] == 2)
    {
        DrawBottomAnchoredBackground(S, Z, A, 0, -1, 0, 2, true);
    }

    A = 4; // Castle
    if(Background2[S] == 3)
    {
        DrawCenterAnchoredBackground(S, Z, A);
    }

    A = 5; // Pipes
    if(Background2[S] == 4)
    {
        int Eff_Top = 0;
        if(screen.Type == ScreenTypes::Dynamic)
            Eff_Top = vScreen[Z].Top;

        int off_y = camY + sect.Y - (camY + Eff_Top + sect.Y) / 2 - 32;
        int off_x = sect.Width - GFXBackground2[A].w;

        if(sect.Width - sect.X > GFXBackground2[A].w)
        {
            off_x = (-camX - sect.X - vScreen[Z].Left) * (GFXBackground2[A].w - 800) / (sect.Width - sect.X - 800) + vScreen[Z].Left;
            off_x = -off_x;
        }

        DrawYTiledBackground(off_x, off_y, vScreen[Z].Width, vScreen[Z].Height, GFXBackground2[A]);
    }

    A = 6; // Trees
    if(Background2[S] == 5)
    {
        DrawCenterAnchoredBackground(S, Z, A, -1);
    }

    A = 7; // Bonus
    if(Background2[S] == 6)
    {
        DrawCenterAnchoredBackground(S, Z, A);
    }

    A = 8; // SMB Underground
    if(Background2[S] == 7)
    {
        DrawTopAnchoredBackground(S, Z, A);
    }

    A = 9; // Night
    if(Background2[S] == 8 || Background2[S] == 9)
    {
        DrawCenterAnchoredBackground(S, Z, A, -1, 0, 0, false, 3);
    }

    A = 10; // Night 2
    if(Background2[S] == 9)
    {
        DrawBottomAnchoredBackground(S, Z, A, 0, -1, 0, 2, true);
    }

    A = 11; // Overworld
    if(Background2[S] == 10)
    {
        DrawCenterAnchoredBackground(S, Z, A, -1);
    }

    A = 12; // SMW Hills
    if(Background2[S] == 11)
    {
        DrawCenterAnchoredBackground(S, Z, A, -1);
    }

    A = 13; // SMW Trees
    if(Background2[S] == 12)
    {
        DrawCenterAnchoredBackground(S, Z, A, 800, 0, 0, true);
    }

    A = 14; // SMB3 Desert
    if(Background2[S] == 14)
    {
        // could possibly tile
        DrawCenterAnchoredBackground(S, Z, A, -1);
    }

    A = 15; // SMB3 Dungeon
    if(Background2[S] == 15)
    {
        DrawCenterAnchoredBackground(S, Z, A);
    }

    A = 16; // Crateria
    if(Background2[S] == 16)
    {
        DrawCenterAnchoredBackground(S, Z, A);
    }

    A = 17; // smb3 ship
    if(Background2[S] == 17)
    {
        // top segment is 254px tall
        DrawTopAnchoredBackground(S, Z, A, 32, 858, 858-254);
    }

    A = 18; // SMW ghost house
    if(Background2[S] == 18)
    {
        DrawCenterAnchoredBackground(S, Z, A, 0, 0, 0, false, 2, true);
    }

    A = 19; // smw forest
    if(Background2[S] == 19)
    {
        DrawCenterAnchoredBackground(S, Z, A, 1200, 0, 0, true);
    }

    A = 20; // smb3 forest
    if(Background2[S] == 20)
    {
        DrawCenterAnchoredBackground(S, Z, A, -1);
    }

    A = 21; // smb3 battle game
    if(Background2[S] == 21)
    {
        DrawCenterAnchoredBackground(S, Z, A);
    }

    A = 22; // SMB3 Waterfall
    if(Background2[S] == 22)
    {
        DrawBottomAnchoredBackground(S, Z, A, 0, -1, 0, 2, true, true);
    }

    A = 23; // SMB3 Tank
    if(Background2[S] == 23)
    {
        DrawCenterAnchoredBackground(S, Z, A, -1);
    }

    A = 24; // smb3 bowsers castle
    if(Background2[S] == 24)
    {
        DrawTopAnchoredBackground(S, Z, A, 32, 1504, 384);
    }

    A = 25; // SMB2 Underground
    if(Background2[S] == 25)
    {
        int Eff_Top = 0;
        if(screen.Type == ScreenTypes::Dynamic)
            Eff_Top = vScreen[Z].Top;

        int off_y = camY + sect.Y - (camY + Eff_Top + sect.Y) / 2 - 32;
        int off_x = sect.Width - GFXBackground2[A].w;

        if(sect.Width - sect.X > GFXBackground2[A].w)
        {
            off_x = (-camX - sect.X - vScreen[Z].Left) * (GFXBackground2[A].w - 800) / (sect.Width - sect.X - 800) + vScreen[Z].Left;
            off_x = -off_x;
        }

        DrawYTiledBackground(off_x, off_y, vScreen[Z].Width, vScreen[Z].Height, GFXBackground2[A]);
    }

    A = 26; // Toad's House
    if(Background2[S] == 26)
    {
        // top segment is 244px tall
        DrawTopAnchoredBackground(S, Z, A, 32, 1396, 1396-244);
    }

    A = 27; // SMB3 Castle
    if(Background2[S] == 27)
    {
        // can be tiled; top section is 1000px
        DrawCenterAnchoredBackground(S, Z, A, -1);
    }

    A = 28; // SMW Bonus
    if(Background2[S] == 28)
    {
        DrawCenterAnchoredBackground(S, Z, A);
    }

    A = 29; // SMW Night
    if(Background2[S] == 29)
    {
        DrawCenterAnchoredBackground(S, Z, A, -1, 0, 0, false, 2, true);
    }

    A = 30; // SMW Cave
    if(Background2[S] == 30)
    {
        DrawCenterAnchoredBackground(S, Z, A, 3456, 0, 0, true, 2, true);
    }

    A = 31; // SMW Hills 2
    if(Background2[S] == 31)
    {
        DrawCenterAnchoredBackground(S, Z, A, -1);
    }

    A = 32; // SMW Clouds
    if(Background2[S] == 32)
    {
        DrawCenterAnchoredBackground(S, Z, A, -1);
    }

    A = 33; // SMW Snow
    if(Background2[S] == 33)
    {
        DrawCenterAnchoredBackground(S, Z, A, -1);
    }

    A = 34; // SMW Hills 3
    if(Background2[S] == 34)
    {
        DrawCenterAnchoredBackground(S, Z, A, -1);
    }

    A = 36; // Snow Clouds
    if(Background2[S] == 35 || Background2[S] == 37)
    {
        DrawBottomAnchoredBackground(S, Z, A, 500, 0, 0, 3);
    }

    if(Background2[S] == 36)
    {
        DrawCenterAnchoredBackground(S, Z, A, 0, 0, 0, false, 3);
    }

    A = 35; // SMB 3 Snow Trees
    if(Background2[S] == 35)
    {
        DrawBottomAnchoredBackground(S, Z, A, 0, -1, 0, 2, true);
    }

    A = 37; // SMB 3 Snow Hills
    if(Background2[S] == 37)
    {
        DrawBottomAnchoredBackground(S, Z, A, 0, -1, 0, 2, true);
    }

    A = 38; // SMB3 Cave with Sky
    if(Background2[S] == 38)
    {
        // each of the lower cave segments are 428px
        DrawTopAnchoredBackground(S, Z, A, 20, 1500, 428*2);
    }

    A = 39; // SMB3 Cave no Sky
    if(Background2[S] == 39)
    {
        DrawCenterAnchoredBackground(S, Z, A);
    }

    A = 40; // Mystic Cave Zone
    if(Background2[S] == 40)
    {
        if(g_config.disable_background2_tiling)
        {
            XRender::lazyPreLoad(GFXBackground2[A]);
            XRender::renderRect(0, 0, vScreen[Z].Width, vScreen[Z].Height,
                GFXBackground2[A].ColorUpper);
        }

        int offsetY = 0;

        if(Eff_ScreenH > GFXBackground2[A].h)
        {
            offsetY = Eff_ScreenH - GFXBackground2[A].h + (sect.Height + camY + Eff_Top - Eff_ScreenH) / 2 - Eff_Top;
        }
        else if(sect.Height - sect.Y > GFXBackground2[A].h)
        {
            // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - (screen.H - vScreen(Z).Top)) * (GFXBackground2Height(A) - (screen.H - vScreen(Z).Top))
            // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
            offsetY = (camY + Eff_Top + sect.Y) * (GFXBackground2[A].h - Eff_ScreenH) / (sect.Height - sect.Y - Eff_ScreenH) - Eff_Top;
        }
        else
        {
            offsetY = sect.Height + camY - GFXBackground2[A].h;
        }

        do
        {
            int offsetX = (camX_levelX * 1 - Left * 1) / 2;
            for(; offsetX < vScreen[Z].Width; offsetX += GFXBackground2[A].w)
            {
                if(offsetX + GFXBackground2[A].w <= 0)
                    continue;

                XRender::renderTextureBasic(offsetX, offsetY + 953, GFXBackground2[A].w, 47, GFXBackground2[A], 0, 953);
            }

            offsetX = (camX_levelX * 4 - Left * 6) / 10;
            for(; offsetX < vScreen[Z].Width; offsetX += GFXBackground2[A].w)
            {
                if(offsetX + GFXBackground2[A].w <= 0)
                    continue;

                XRender::renderTextureBasic(offsetX, offsetY + 916, GFXBackground2[A].w, 37, GFXBackground2[A], 0, 916);
            }

            offsetX = (camX_levelX * 3 - Left * 7) / 10;
            for(; offsetX < vScreen[Z].Width; offsetX += GFXBackground2[A].w)
            {
                if(offsetX + GFXBackground2[A].w <= 0)
                    continue;

                XRender::renderTextureBasic(offsetX, offsetY + 849, GFXBackground2[A].w, 67, GFXBackground2[A], 0, 849);
            }

            offsetX = (camX_levelX * 2 - Left * 8) / 10;
            for(; offsetX < vScreen[Z].Width; offsetX += GFXBackground2[A].w)
            {
                if(offsetX + GFXBackground2[A].w <= 0)
                    continue;

                XRender::renderTextureBasic(offsetX, offsetY + 815, GFXBackground2[A].w, 34, GFXBackground2[A], 0, 815);
            }

            offsetX = (camX_levelX * 1 - Left * 9) / 10;
            for(; offsetX < vScreen[Z].Width; offsetX += GFXBackground2[A].w)
            {
                if(offsetX + GFXBackground2[A].w <= 0)
                    continue;

                XRender::renderTextureBasic(offsetX, offsetY + 709, GFXBackground2[A].w, 106, GFXBackground2[A], 0, 709);
            }

            offsetX = (camX_levelX * 15 - Left * 85) / 100;
            for(; offsetX < vScreen[Z].Width; offsetX += GFXBackground2[A].w)
            {
                if(offsetX + GFXBackground2[A].w <= 0)
                    continue;

                XRender::renderTextureBasic(offsetX, offsetY + 664, GFXBackground2[A].w, 45, GFXBackground2[A], 0, 664);
            }

            offsetX = (camX_levelX * 2 - Left * 8) / 10;
            for(; offsetX < vScreen[Z].Width; offsetX += GFXBackground2[A].w)
            {
                if(offsetX + GFXBackground2[A].w <= 0)
                    continue;

                XRender::renderTextureBasic(offsetX, offsetY + 614, GFXBackground2[A].w, 50, GFXBackground2[A], 0, 614);
            }

            offsetX = (camX_levelX * 25 - Left * 75) / 100;
            for(; offsetX < vScreen[Z].Width; offsetX += GFXBackground2[A].w)
            {
                if(offsetX + GFXBackground2[A].w <= 0)
                    continue;

                XRender::renderTextureBasic(offsetX, offsetY + 540, GFXBackground2[A].w, 74, GFXBackground2[A], 0, 540);
            }

            offsetX = (camX_levelX * 3 - Left * 7) / 10;
            for(; offsetX < vScreen[Z].Width; offsetX += GFXBackground2[A].w)
            {
                if(offsetX + GFXBackground2[A].w <= 0)
                    continue;

                XRender::renderTextureBasic(offsetX, offsetY + 408, GFXBackground2[A].w, 132, GFXBackground2[A], 0, 408);
            }

            offsetX = (camX_levelX * 25 - Left * 75) / 100;
            for(; offsetX < vScreen[Z].Width; offsetX += GFXBackground2[A].w)
            {
                if(offsetX + GFXBackground2[A].w <= 0)
                    continue;

                XRender::renderTextureBasic(offsetX, offsetY + 333, GFXBackground2[A].w, 75, GFXBackground2[A], 0, 333);
            }

            offsetX = (camX_levelX * 2 - Left * 8) / 10;
            for(; offsetX < vScreen[Z].Width; offsetX += GFXBackground2[A].w)
            {
                if(offsetX + GFXBackground2[A].w <= 0)
                    continue;

                XRender::renderTextureBasic(offsetX, offsetY + 278, GFXBackground2[A].w, 55, GFXBackground2[A], 0, 278);
            }

            offsetX = (camX_levelX * 15 - Left * 85) / 100;
            for(; offsetX < vScreen[Z].Width; offsetX += GFXBackground2[A].w)
            {
                if(offsetX + GFXBackground2[A].w <= 0)
                    continue;

                XRender::renderTextureBasic(offsetX, offsetY + 235, GFXBackground2[A].w, 43, GFXBackground2[A], 0, 235);
            }

            offsetX = (camX_levelX * 1 - Left * 9) / 10;
            for(; offsetX < vScreen[Z].Width; offsetX += GFXBackground2[A].w)
            {
                if(offsetX + GFXBackground2[A].w <= 0)
                    continue;

                XRender::renderTextureBasic(offsetX, offsetY + 123, GFXBackground2[A].w, 112, GFXBackground2[A], 0, 123);
            }

            offsetX = (camX_levelX * 2 - Left * 8) / 10;
            for(; offsetX < vScreen[Z].Width; offsetX += GFXBackground2[A].w)
            {
                if(offsetX + GFXBackground2[A].w <= 0)
                    continue;

                XRender::renderTextureBasic(offsetX, offsetY + 85, GFXBackground2[A].w, 38, GFXBackground2[A], 0, 85);
            }

            offsetX = (camX_levelX * 3 - Left * 7) / 10;
            for(; offsetX < vScreen[Z].Width; offsetX += GFXBackground2[A].w)
            {
                if(offsetX + GFXBackground2[A].w <= 0)
                    continue;

                XRender::renderTextureBasic(offsetX, offsetY + 48, GFXBackground2[A].w, 37, GFXBackground2[A], 0, 48);
            }

            offsetX = (camX_levelX * 4 - Left * 6) / 10;
            for(; offsetX < vScreen[Z].Width; offsetX += GFXBackground2[A].w)
            {
                if(offsetX + GFXBackground2[A].w <= 0)
                    continue;

                XRender::renderTextureBasic(offsetX, offsetY + 0, GFXBackground2[A].w, 48, GFXBackground2[A], 0, 0);
            }

            offsetY -= GFXBackground2[A].h;

            if(g_config.disable_background2_tiling)
                break;

        } while(offsetY + GFXBackground2[A].h > 0);
    }

    A = 41; // SMB 1 Castle
    if(Background2[S] == 41)
    {
        DrawCenterAnchoredBackground(S, Z, A);
    }

    A = 42; // SMW Castle
    if(Background2[S] == 42)
    {
        // safe to tile top 546px and bottom 160px of vanilla asset
        DrawCenterAnchoredBackground(S, Z, A, 3456, 160, 546, false, 2, true);
    }

    A = 43; // SMW Castle 2
    if(Background2[S] == 43)
    {
        DrawCenterAnchoredBackground(S, Z, A);
    }

    A = 44; // SMB2 Castle
    if(Background2[S] == 44)
    {
        DrawCenterAnchoredBackground(S, Z, A);
    }

    A = 45; // Brinstar
    if(Background2[S] == 45)
    {
        DrawCenterAnchoredBackground(S, Z, A);
    }

    A = 46; // Transport
    if(Background2[S] == 46)
    {
        DrawCenterAnchoredBackground(S, Z, A);
    }

    A = 47; // Transport
    if(Background2[S] == 47)
    {
        DrawCenterAnchoredBackground(S, Z, A);
    }

    A = 48; // SMB2 Blouds
    if(Background2[S] == 48)
    {
        DrawCenterAnchoredBackground(S, Z, A, -1);
    }

    A = 49; // Desert Night
    if(Background2[S] == 49)
    {
        int CanvasH = GFXBackground2[A].h;
        int CanvasOffset = 0;

        // ensure that the canvas covers above and below the screen
        if(Eff_ScreenH > CanvasH)
        {
            CanvasOffset = Eff_ScreenH - CanvasH;
            CanvasH += CanvasOffset * 2;
        }

        int offsetY = 0;

        if(sect.Height - sect.Y > CanvasH)
        {
            // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
            offsetY = (-camY - Eff_Top - sect.Y) * (CanvasH - Eff_ScreenH) / (sect.Height - sect.Y - Eff_ScreenH) + Eff_Top;
            offsetY = -offsetY;
            offsetY += CanvasOffset;
        }
        else if(CanvasH > GFXBackground2[A].h)
        {
            offsetY = camY + sect.Y + (sect.Height - sect.Y - GFXBackground2[A].h) / 2;
        }
        else
            offsetY = camY + sect.Height - GFXBackground2[A].h;

        // use a simple color fill for the sky above and sand below the texture
        XRender::lazyPreLoad(GFXBackground2[A]);
        int undrawn_above = offsetY;
        int undrawn_below = offsetY + GFXBackground2[A].h;
        XRender::renderRect(0, 0, vScreen[Z].Width, std::ceil(undrawn_above),
            GFXBackground2[A].ColorUpper);
        XRender::renderRect(0, undrawn_below, vScreen[Z].Width, vScreen[Z].Height - undrawn_below + 1,
            GFXBackground2[A].ColorLower);

        int offsetX = (camX_levelX * 1 - Left * 1) / 2;
        for(; offsetX < vScreen[Z].Width; offsetX += GFXBackground2[A].w)
        {
            if(offsetX + GFXBackground2[A].w <= 0)
                continue;

            XRender::renderTextureBasic(offsetX, offsetY + 280, GFXBackground2[A].w, 450, GFXBackground2[A], 0, 280);
        }

        offsetX = (camX_levelX * 1 - Left * 9) / 10;
        for(; offsetX < vScreen[Z].Width; offsetX += GFXBackground2[A].w)
        {
            if(offsetX + GFXBackground2[A].w <= 0)
                continue;

            XRender::renderTextureBasic(offsetX, offsetY + 268, GFXBackground2[A].w, 12, GFXBackground2[A], 0, 268);
        }

        offsetX = (camX_levelX * 11 - Left * 89) / 100;
        for(; offsetX < vScreen[Z].Width; offsetX += GFXBackground2[A].w)
        {
            if(offsetX + GFXBackground2[A].w <= 0)
                continue;

            XRender::renderTextureBasic(offsetX, offsetY + 244, GFXBackground2[A].w, 24, GFXBackground2[A], 0, 244);
        }

        offsetX = (camX_levelX * 12 - Left * 88) / 100;
        for(; offsetX < vScreen[Z].Width; offsetX += GFXBackground2[A].w)
        {
            if(offsetX + GFXBackground2[A].w <= 0)
                continue;

            XRender::renderTextureBasic(offsetX, offsetY + 228, GFXBackground2[A].w, 16, GFXBackground2[A], 0, 228);
        }

        offsetX = (camX_levelX * 13 - Left * 87) / 100;
        for(; offsetX < vScreen[Z].Width; offsetX += GFXBackground2[A].w)
        {
            if(offsetX + GFXBackground2[A].w <= 0)
                continue;

            XRender::renderTextureBasic(offsetX, offsetY + 196, GFXBackground2[A].w, 32, GFXBackground2[A], 0, 196);
        }

        offsetX = (camX_levelX * 14 - Left * 86) / 100;
        for(; offsetX < vScreen[Z].Width; offsetX += GFXBackground2[A].w)
        {
            if(offsetX + GFXBackground2[A].w <= 0)
                continue;

            XRender::renderTextureBasic(offsetX, offsetY + 164, GFXBackground2[A].w, 32, GFXBackground2[A], 0, 164);
        }

        offsetX = (camX_levelX * 15 - Left * 85) / 100;
        for(; offsetX < vScreen[Z].Width; offsetX += GFXBackground2[A].w)
        {
            if(offsetX + GFXBackground2[A].w <= 0)
                continue;

            XRender::renderTextureBasic(offsetX, offsetY + 116, GFXBackground2[A].w, 48, GFXBackground2[A], 0, 116);
        }

        offsetX = (camX_levelX * 16 - Left * 84) / 100;
        for(; offsetX < vScreen[Z].Width; offsetX += GFXBackground2[A].w)
        {
            if(offsetX + GFXBackground2[A].w <= 0)
                continue;

            XRender::renderTextureBasic(offsetX, offsetY + 58, GFXBackground2[A].w, 58, GFXBackground2[A], 0, 58);
        }

        offsetX = (camX_levelX * 17 - Left * 83) / 100;
        for(; offsetX < vScreen[Z].Width; offsetX += GFXBackground2[A].w)
        {
            if(offsetX + GFXBackground2[A].w <= 0)
                continue;

            XRender::renderTextureBasic(offsetX, offsetY + 0, GFXBackground2[A].w, 58, GFXBackground2[A], 0, 0);
        }
    }

    A = 50; // Shrooms
    if(Background2[S] == 50)
    {
        XRender::lazyPreLoad(GFXBackground2[A]);
        XRender::renderRect(0, 0, vScreen[Z].Width, vScreen[Z].Height,
            GFXBackground2[A].ColorLower);

        int CanvasH = GFXBackground2[A].h;
        int CanvasOffset = 0;

        // ensure that the canvas covers above and below the screen
        if(Eff_ScreenH > CanvasH)
        {
            CanvasOffset = (Eff_ScreenH - CanvasH) * 3 / 2;
            CanvasH += CanvasOffset;
        }

        int offsetY = 0;

        if(sect.Height - sect.Y > CanvasH)
        {
            // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
            offsetY = (-camY - Eff_Top - sect.Y) * (CanvasH - Eff_ScreenH) / (sect.Height - sect.Y - Eff_ScreenH) + Eff_Top;
            offsetY = -offsetY;
            offsetY += CanvasOffset;
        }
        else
            offsetY = camY + sect.Height - GFXBackground2[A].h;

        int offsetX = (camX_levelX - Left) / 2;
        for(; offsetX < vScreen[Z].Width; offsetX += GFXBackground2[A].w)
        {
            if(offsetX + GFXBackground2[A].w <= 0)
                continue;

            XRender::renderTextureBasic(offsetX, offsetY + 378, GFXBackground2[A].w, 378, GFXBackground2[A], 0, 378);
        }

        while(offsetY > -378)
        {
            offsetX = (camX_levelX * 35 - Left * 65) / 100;
            for(; offsetX < vScreen[Z].Width; offsetX += GFXBackground2[A].w)
            {
                if(offsetX + GFXBackground2[A].w <= 0)
                    continue;

                XRender::renderTextureBasic(offsetX, offsetY, GFXBackground2[A].w, 220, GFXBackground2[A], 0, 0);
            }

            offsetX = (camX_levelX * 4 - Left * 6) / 10;
            for(; offsetX < vScreen[Z].Width; offsetX += GFXBackground2[A].w)
            {
                if(offsetX + GFXBackground2[A].w <= 0)
                    continue;

                XRender::renderTextureBasic(offsetX, offsetY + 220, GFXBackground2[A].w, 158, GFXBackground2[A], 0, 220);
            }

            offsetY -= 378;

            if(g_config.disable_background2_tiling)
                break;
        }
    }

    A = 51; // SMB1 Desert
    if(Background2[S] == 51)
    {
        int CanvasH = GFXBackground2[A].h;
        int CanvasOffset = 0;

        // ensure that the canvas covers above and below the screen
        if(Eff_ScreenH > CanvasH)
        {
            CanvasOffset = 0;
            CanvasH += (Eff_ScreenH - CanvasH);
        }

        int offsetY = 0;

        if(sect.Height - sect.Y > CanvasH)
        {
            // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
            offsetY = (-camY - Eff_Top - sect.Y) * (CanvasH - Eff_ScreenH) / (sect.Height - sect.Y - Eff_ScreenH) + Eff_Top;
            offsetY = -offsetY;
            offsetY += CanvasOffset;
        }
        else if(CanvasH > GFXBackground2[A].h)
        {
            offsetY = camY + sect.Y + (sect.Height - sect.Y - GFXBackground2[A].h) / 2;
        }
        else
            offsetY = camY + sect.Height - GFXBackground2[A].h;

        // use a simple color fill for the sky above and sand below the texture
        XRender::lazyPreLoad(GFXBackground2[A]);
        int undrawn_above = offsetY;
        int undrawn_below = offsetY + GFXBackground2[A].h;
        XRender::renderRect(0, 0, vScreen[Z].Width, undrawn_above,
            GFXBackground2[A].ColorUpper);
        XRender::renderRect(0, undrawn_below, vScreen[Z].Width, vScreen[Z].Height - undrawn_below + 1,
            GFXBackground2[A].ColorLower);

        int offsetX = (camX_levelX - Left * 3) / 4;
        for(; offsetX < vScreen[Z].Width; offsetX += GFXBackground2[A].w)
        {
            if(offsetX + GFXBackground2[A].w <= 0)
                continue;

            XRender::renderTextureBasic(offsetX, offsetY, GFXBackground2[A].w, 350, GFXBackground2[A], 0, 0);
        }

        offsetX = (camX_levelX - Left) / 2;
        for(; offsetX < vScreen[Z].Width; offsetX += GFXBackground2[A].w)
        {
            if(offsetX + GFXBackground2[A].w <= 0)
                continue;

            XRender::renderTextureBasic(offsetX, offsetY + 350, GFXBackground2[A].w, GFXBackground2[A].h - 350, GFXBackground2[A], 0, 350);
        }
    }

    A = 52; // SMB2 Desert Night
    if(Background2[S] == 52)
    {
        int CanvasH = GFXBackground2[A].h;
        int CanvasOffset = 0;

        // ensure that the canvas covers above and below the screen
        if(Eff_ScreenH > CanvasH)
        {
            CanvasOffset = (Eff_ScreenH - CanvasH) * 3 / 2;
            CanvasH += CanvasOffset;
        }

        int offsetY = 0;

        if(sect.Height - sect.Y > CanvasH)
        {
            // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
            offsetY = (-camY - Eff_Top - sect.Y) * (CanvasH - Eff_ScreenH) / (sect.Height - sect.Y - Eff_ScreenH) + Eff_Top;
            offsetY = -offsetY;
            offsetY += CanvasOffset;
        }
        else if(CanvasH > GFXBackground2[A].h)
        {
            offsetY = camY + sect.Y + (sect.Height - sect.Y - GFXBackground2[A].h) / 2;
        }
        else
            offsetY = camY + sect.Height - GFXBackground2[A].h;


        // use a simple color fill for the sky above and sand below the texture
        XRender::lazyPreLoad(GFXBackground2[A]);
        int undrawn_above = offsetY;
        int undrawn_below = offsetY + GFXBackground2[A].h;
        XRender::renderRect(0, 0, vScreen[Z].Width, undrawn_above,
            GFXBackground2[A].ColorUpper);
        XRender::renderRect(0, undrawn_below, vScreen[Z].Width, vScreen[Z].Height - undrawn_below + 1,
            GFXBackground2[A].ColorLower);

        int offsetX = (camX_levelX * 1 - Left * 1) / 2;
        for(; offsetX < vScreen[Z].Width; offsetX += GFXBackground2[A].w)
        {
            if(offsetX + GFXBackground2[A].w <= 0)
                continue;

            XRender::renderTextureBasic(offsetX, offsetY + 280, GFXBackground2[A].w, GFXBackground2[A].h - 280, GFXBackground2[A], 0, 280);
        }

        offsetX = (camX_levelX * 1 - Left * 9) / 10;
        for(; offsetX < vScreen[Z].Width; offsetX += GFXBackground2[A].w)
        {
            if(offsetX + GFXBackground2[A].w <= 0)
                continue;

            XRender::renderTextureBasic(offsetX, offsetY + 268, GFXBackground2[A].w, 12, GFXBackground2[A], 0, 268);
        }

        offsetX = (camX_levelX * 11 - Left * 89) / 100;
        for(; offsetX < vScreen[Z].Width; offsetX += GFXBackground2[A].w)
        {
            if(offsetX + GFXBackground2[A].w <= 0)
                continue;

            XRender::renderTextureBasic(offsetX, offsetY + 244, GFXBackground2[A].w, 24, GFXBackground2[A], 0, 244);
        }

        offsetX = (camX_levelX * 12 - Left * 88) / 100;
        for(; offsetX < vScreen[Z].Width; offsetX += GFXBackground2[A].w)
        {
            if(offsetX + GFXBackground2[A].w <= 0)
                continue;

            XRender::renderTextureBasic(offsetX, offsetY + 228, GFXBackground2[A].w, 16, GFXBackground2[A], 0, 228);
        }

        offsetX = (camX_levelX * 13 - Left * 87) / 100;
        for(; offsetX < vScreen[Z].Width; offsetX += GFXBackground2[A].w)
        {
            if(offsetX + GFXBackground2[A].w <= 0)
                continue;

            XRender::renderTextureBasic(offsetX, offsetY + 196, GFXBackground2[A].w, 32, GFXBackground2[A], 0, 196);
        }

        offsetX = (camX_levelX * 14 - Left * 86) / 100;
        for(; offsetX < vScreen[Z].Width; offsetX += GFXBackground2[A].w)
        {
            if(offsetX + GFXBackground2[A].w <= 0)
                continue;

            XRender::renderTextureBasic(offsetX, offsetY + 164, GFXBackground2[A].w, 32, GFXBackground2[A], 0, 164);
        }

        offsetX = (camX_levelX * 15 - Left * 85) / 100;
        for(; offsetX < vScreen[Z].Width; offsetX += GFXBackground2[A].w)
        {
            if(offsetX + GFXBackground2[A].w <= 0)
                continue;

            XRender::renderTextureBasic(offsetX, offsetY + 116, GFXBackground2[A].w, 48, GFXBackground2[A], 0, 116);
        }

        offsetX = (camX_levelX * 16 - Left * 84) / 100;
        for(; offsetX < vScreen[Z].Width; offsetX += GFXBackground2[A].w)
        {
            if(offsetX + GFXBackground2[A].w <= 0)
                continue;

            XRender::renderTextureBasic(offsetX, offsetY + 58, GFXBackground2[A].w, 58, GFXBackground2[A], 0, 58);
        }

        offsetX = (camX_levelX * 17 - Left * 83) / 100;
        for(; offsetX < vScreen[Z].Width; offsetX += GFXBackground2[A].w)
        {
            if(offsetX + GFXBackground2[A].w <= 0)
                continue;

            XRender::renderTextureBasic(offsetX, offsetY + 0, GFXBackground2[A].w, 58, GFXBackground2[A], 0, 0);
        }
    }

    A = 53; // Cliffs
    if(Background2[S] == 53)
    {
        DrawCenterAnchoredBackground(S, Z, A, -1);
    }

    A = 54; // Warehouse
    if(Background2[S] == 54)
    {
        DrawCenterAnchoredBackground(S, Z, A);
    }

    A = 55; // SMW Water
    if(Background2[S] == 55)
    {
        DrawCenterAnchoredBackground(S, Z, A, 0, 0, 0, false, 2, true);
    }

    A = 56; // SMB3 Water
    if(Background2[S] == 56)
    {
        int CanvasH = GFXBackground2[A].h;
        int CanvasOffset = 0;

        // ensure that the canvas covers above and below the screen
        if(Eff_ScreenH > CanvasH)
        {
            CanvasOffset = (Eff_ScreenH - CanvasH) * 3 / 2;
            CanvasH += CanvasOffset;
        }

        int offsetY = 0;

        if(sect.Height - sect.Y > CanvasH)
        {
            // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
            offsetY = (-camY - Eff_Top - sect.Y) * (CanvasH - Eff_ScreenH) / (sect.Height - sect.Y - Eff_ScreenH) + Eff_Top;
            offsetY = -offsetY;
            offsetY += CanvasOffset;
        }
        else if(CanvasH > GFXBackground2[A].h)
        {
            offsetY = camY + sect.Y + (sect.Height - sect.Y - GFXBackground2[A].h) / 2;
        }
        else
            offsetY = camY + sect.Height - GFXBackground2[A].h;

        // use a simple color fill for the water above and below the texture
        XRender::lazyPreLoad(GFXBackground2[A]);
        int undrawn_above = offsetY;
        int undrawn_below = offsetY + GFXBackground2[A].h;
        XRender::renderRect(0, 0, vScreen[Z].Width, undrawn_above,
            GFXBackground2[A].ColorUpper);
        XRender::renderRect(0, undrawn_below, vScreen[Z].Width, vScreen[Z].Height - undrawn_below + 1,
            GFXBackground2[A].ColorLower);

        int offsetX = (camX_levelX * 35 - Left * 65) / 100;
        for(; offsetX < vScreen[Z].Width; offsetX += GFXBackground2[A].w)
        {
            if(offsetX + GFXBackground2[A].w <= 0)
                continue;

            XRender::renderTextureBasic(offsetX, offsetY, GFXBackground2[A].w, 100, GFXBackground2[A], 0, 0);
        }

        offsetX = (camX_levelX * 4 - Left * 6) / 10;
        for(; offsetX < vScreen[Z].Width; offsetX += GFXBackground2[A].w)
        {
            if(offsetX + GFXBackground2[A].w <= 0)
                continue;

            XRender::renderTextureBasic(offsetX, offsetY + 100, GFXBackground2[A].w, 245, GFXBackground2[A], 0, 100);
        }

        offsetX = (camX_levelX * 45 - Left * 55) / 100;
        for(; offsetX < vScreen[Z].Width; offsetX += GFXBackground2[A].w)
        {
            if(offsetX + GFXBackground2[A].w <= 0)
                continue;

            XRender::renderTextureBasic(offsetX, offsetY + 345, GFXBackground2[A].w, 110, GFXBackground2[A], 0, 345);
        }

        offsetX = (camX_levelX * 5 - Left * 5) / 10;
        for(; offsetX < vScreen[Z].Width; offsetX += GFXBackground2[A].w)
        {
            if(offsetX + GFXBackground2[A].w <= 0)
                continue;

            XRender::renderTextureBasic(offsetX, offsetY + 455, GFXBackground2[A].w, GFXBackground2[A].h - 455, GFXBackground2[A], 0, 455);
        }
    }

    A = 57; // Warehouse
    if(Background2[S] == 57)
    {
        // looks more like a temple to me.
        // in vanilla asset:
        // top: can loop top 672px
        // bottom: can loop bottom 64px
        DrawCenterAnchoredBackground(S, Z, A, 800, 64, 672);
    }

    A = 58; // SMW Night
    if(Background2[S] == 58)
    {
        // can loop top 600px and bottom 128px of vanilla asset
        DrawCenterAnchoredBackground(S, Z, A, 3584, 128, 600, false, 2, true);
    }
}
