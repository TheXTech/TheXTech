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
void DrawTopAnchoredBackground(int S, int Z, int A, int offset = 32, int expected_height = 0, int tile_bottom = 0, double h_parallax = 0.5)
{
    DrawBackgroundColor(A, Z, true);
    double camX = vScreen[Z].CameraAddX();
    double camY = vScreen[Z].CameraAddY();

    const Screen_t& screen = Screens[vScreen[Z].screen_ref];

    int horiz_reps = (level[S].Width - level[S].X + screen.W / h_parallax) / GFXBackground2Width[A] * h_parallax + 1;

    Location_t tempLocation;

    for(int B = 0; B <= horiz_reps; B++)
    {
        tempLocation.Width = GFXBackground2Width[A];
        tempLocation.Height = GFXBackground2Height[A];

        tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (camX + vScreen[Z].Left + level[S].X) * h_parallax);

        tempLocation.Y = level[S].Y - offset;
        if(GameMenu && tempLocation.Y > -camY)
            tempLocation.Y = -camY;

        while(tempLocation.X + tempLocation.Width > -camX
                && tempLocation.X < -camX + vScreen[Z].Width
                && tempLocation.Y < -camY + vScreen[Z].Height)
        {
            XRender::renderTexture(camX + tempLocation.X, camY + tempLocation.Y,
                GFXBackground2Width[A], tempLocation.Height, GFXBackground2[A], 0, GFXBackground2Height[A] - tempLocation.Height);

            if(g_config.disable_background2_tiling)
                break;

            if(expected_height != 0 && GFXBackground2Height[A] != expected_height)
                break;

            tempLocation.Y += tempLocation.Height;

            if(tile_bottom != 0)
            {
                if(GFXBackground2Height[A] != expected_height)
                    break;
                tempLocation.Height = tile_bottom;
            }
        }
    }
}

void DrawCenterAnchoredBackground(int S, int Z, int A, int expected_height = 0, int tile_bottom = 0, int tile_top = 0, bool flip_tile = false, int h_num = 1, int h_den = 2, bool anim = false)
{
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
    if((tile_bottom != 0 || tile_top != 0 || expected_height != 0) && GFXBackground2Height[A] != expected_height)
    {
        // HACK: don't cancel the tiling if we have the slightly incorrect original asset
        if(!(A == 42 && GFXBackground2Height[A] == expected_height - 1))
            no_tiling = true;
    }

    int frameH = GFXBackground2Height[A];
    // HACK: align non-rounded pictures (there was Redigit's original with the 3455 pixels height,
    // but it must be 3456. There are lot of custom resources that using the 3455 height by mistake)
    // in the original image, the fourth frame is missing its top line.
    if(A == 42 && GFXBackground2Height[A] == expected_height - 1 && anim)
    {
        frameH = expected_height / 4;
    }
    else if(anim)
    {
        frameH = GFXBackground2Height[A] / 4;
    }

    int CanvasH = frameH;
    int CanvasOffset = 0;

    // ensure that the canvas covers above and below the screen
    if(Eff_ScreenH > CanvasH)
    {
        CanvasOffset = Eff_ScreenH - CanvasH;
        CanvasH += CanvasOffset * 2;
    }

    Location_t tempLocation;

    int levelX = SDL_round(level[S].X);

    int camX_levelX = camX + levelX;
    int Left = vScreen[Z].Left;

    int offsetX = (camX_levelX * (h_den - h_num) - Left * h_num) / h_den;

    for(; offsetX < vScreen[Z].Width; offsetX += GFXBackground2Width[A])
    {
        if(offsetX + GFXBackground2Width[A] <= 0)
            continue;

        tempLocation.Width = GFXBackground2Width[A];
        tempLocation.Height = frameH;

        tempLocation.X = offsetX;
        if(level[S].Height - level[S].Y > CanvasH)
        {
            // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - (600 - vScreen(Z).Top)) * (GFXBackground2Height(A) / 4 - (600 - vScreen(Z).Top))
            // .Y = -vScreenY(Z) - .Y
            tempLocation.Y = (-camY - Eff_Top - level[S].Y) / (level[S].Height - level[S].Y - Eff_ScreenH) * (CanvasH - Eff_ScreenH) + Eff_Top;
            tempLocation.Y = -camY - tempLocation.Y;
            tempLocation.Y += CanvasOffset;
        }
        else if(CanvasH > frameH)
        {
            tempLocation.Y = level[S].Y + (level[S].Height - level[S].Y) / 2 - frameH / 2;
        }
        else
            tempLocation.Y = level[S].Height - frameH;

        double bottom_Y = tempLocation.Y + frameH;
        unsigned int flip = X_FLIP_NONE;
        while(tempLocation.Y + tempLocation.Height > -camY)
        {
            // HACK: place the fourth frame in the correct location if we are missing a single line
            if(A == 42 && GFXBackground2Height[A] == expected_height - 1 && anim && SpecialFrame[3] == 3)
            {
                // duplicate the line
                XRender::renderTextureScaleEx(tempLocation.X, camY + tempLocation.Y,
                    GFXBackground2Width[A], 1,
                    GFXBackground2[A],
                    0, frameH * SpecialFrame[3],
                    GFXBackground2Width[A], 1,
                    0., nullptr, flip);
                // draw the frame
                XRender::renderTextureScaleEx(tempLocation.X, camY + tempLocation.Y + 1,
                    GFXBackground2Width[A], tempLocation.Height - 1,
                    GFXBackground2[A],
                    0, frameH * SpecialFrame[3],
                    GFXBackground2Width[A], tempLocation.Height - 1,
                    0., nullptr, flip);
            }
            else if(anim)
            {
                XRender::renderTextureScaleEx(tempLocation.X, camY + tempLocation.Y,
                    GFXBackground2Width[A], tempLocation.Height,
                    GFXBackground2[A],
                    0, frameH * SpecialFrame[3],
                    GFXBackground2Width[A], tempLocation.Height,
                    0., nullptr, flip);
            }
            else
            {
                XRender::renderTextureScaleEx(tempLocation.X, camY + tempLocation.Y,
                    GFXBackground2Width[A], tempLocation.Height,
                    GFXBackground2[A],
                    0, 0,
                    GFXBackground2Width[A], tempLocation.Height,
                    0., nullptr, flip);
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
            if(A == 42 && GFXBackground2Height[A] == expected_height - 1 && anim && SpecialFrame[3] == 3)
            {
                XRender::renderTextureScaleEx(tempLocation.X, camY + tempLocation.Y,
                    GFXBackground2Width[A], tempLocation.Height,
                    GFXBackground2[A],
                    0, frameH * SpecialFrame[3] + (frameH - 1) - tempLocation.Height,
                    GFXBackground2Width[A], tempLocation.Height,
                    0., nullptr, flip);
            }
            else if(anim)
            {
                XRender::renderTextureScaleEx(tempLocation.X, camY + tempLocation.Y,
                    GFXBackground2Width[A], tempLocation.Height,
                    GFXBackground2[A],
                    0, frameH * SpecialFrame[3] + frameH - tempLocation.Height,
                    GFXBackground2Width[A], tempLocation.Height,
                    0., nullptr, flip);
            }
            else
            {
                XRender::renderTextureScaleEx(tempLocation.X, camY + tempLocation.Y,
                    GFXBackground2Width[A], tempLocation.Height,
                    GFXBackground2[A],
                    0, frameH - tempLocation.Height,
                    GFXBackground2Width[A], tempLocation.Height,
                    0., nullptr, flip);
            }

            tempLocation.Y += tempLocation.Height;
            if(flip_tile)
                flip ^= X_FLIP_VERTICAL;
        }
    }

    // use the remaining information from the tempLocation (Y and Height, never tiled)
    if(no_tiling)
    {
        double undrawn_above = tempLocation.Y + camY;
        double undrawn_below = tempLocation.Y + camY + tempLocation.Height;
        XRender::renderRect(0, 0, vScreen[Z].Width, std::ceil(undrawn_above),
            GFXBackground2[A].ColorUpper);
        XRender::renderRect(0, undrawn_below, vScreen[Z].Width, vScreen[Z].Height - undrawn_below + 1,
            GFXBackground2[A].ColorLower);
    }
}

void DrawBottomAnchoredBackground(int S, int Z, int A, int offset = 0, int expected_height = 0, int tile_top = 0, double h_parallax = 0.5, bool no_bg = false, bool anim = false)
{
    const Screen_t& screen = Screens[vScreen[Z].screen_ref];
    double camX = vScreen[Z].CameraAddX();
    double camY = vScreen[Z].CameraAddY();

    if(!no_bg)
        DrawBackgroundColor(A, Z, false);

    double frameH = GFXBackground2Height[A];
    if(anim)
        frameH = GFXBackground2Height[A] / 4.0;

    Location_t tempLocation;

    int horiz_reps = (level[S].Width - level[S].X + screen.W / h_parallax) / GFXBackground2Width[A] * h_parallax + 1;
    for(int B = 0; B <= horiz_reps; B++)
    {
        tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (camX + vScreen[Z].Left + level[S].X) * h_parallax);
        tempLocation.Y = level[S].Height - frameH - offset;

        tempLocation.Height = frameH;
        tempLocation.Width = GFXBackground2Width[A];

        if(vScreenCollision(Z, tempLocation))
        {
            if(anim)
            {
                XRender::renderTexture(camX + tempLocation.X, camY + tempLocation.Y,
                    GFXBackground2Width[A], tempLocation.Height,
                    GFXBackground2[A], 0, frameH * SpecialFrame[3]);
            }
            else
            {
                XRender::renderTexture(camX + tempLocation.X, camY + tempLocation.Y,
                    GFXBackground2Width[A], tempLocation.Height,
                    GFXBackground2[A], 0, 0);
            }
        }

        if(g_config.disable_background2_tiling)
            continue;

        if(expected_height != 0 && GFXBackground2Height[A] != expected_height)
            continue;

        if(tile_top != 0 && GFXBackground2Height[A] != expected_height)
            continue;

        if(tile_top != 0)
            tempLocation.Height = tile_top;

        while(tempLocation.Y > level[S].Y || tempLocation.Y > -camY)
        {
            tempLocation.Y -= tempLocation.Height;
            if(vScreenCollision(Z, tempLocation))
            {
                if(anim)
                {
                    XRender::renderTexture(camX + tempLocation.X, camY + tempLocation.Y,
                        GFXBackground2Width[A], tempLocation.Height,
                        GFXBackground2[A], 0, frameH * SpecialFrame[3]);
                }
                else
                {
                    XRender::renderTexture(camX + tempLocation.X, camY + tempLocation.Y,
                        GFXBackground2Width[A], tempLocation.Height,
                        GFXBackground2[A], 0, 0);
                }
            }
        }
    }

}

void DrawBackground(int S, int Z)
{
    const Screen_t& screen = Screens[vScreen[Z].screen_ref];
    int camX = vScreen[Z].CameraAddX();
    int camY = vScreen[Z].CameraAddY();

    int A = 0;
    int B = 0;
    Location_t tempLocation;
    SpeedlessLocation_t tempLevel;
    tempLevel = level[S];
    if(LevelEditor)
        LevelREAL[S] = level[S];
    level[S] = LevelREAL[S];

    if(Background2[S] == 0)
        XRender::renderRect(0, 0, vScreen[Z].Width, vScreen[Z].Height, {0, 0, 0});

    A = 2; // Clouds
    if(Background2[S] == 1 || Background2[S] == 2 || Background2[S] == 22)
    {
        DrawBottomAnchoredBackground(S, Z, A, 500, 0, 0, 0.75);
    }

    if(Background2[S] == 13)
    {
        DrawCenterAnchoredBackground(S, Z, A, 0, 0, 0, false, 3, 4);
    }

    A = 1; // Blocks
    if(Background2[S] == 1)
    {
        DrawBottomAnchoredBackground(S, Z, A, 0, -1, 0, 0.5, true);
    }

    A = 3; // Hills
    if(Background2[S] == 2)
    {
        DrawBottomAnchoredBackground(S, Z, A, 0, -1, 0, 0.5, true);
    }

    A = 4; // Castle
    if(Background2[S] == 3)
    {
        DrawCenterAnchoredBackground(S, Z, A);
    }

    A = 5; // Pipes
    if(Background2[S] == 4)
    {
        double Eff_Top = 0;
        if(screen.Type == ScreenTypes::Dynamic)
            Eff_Top = vScreen[Z].Top;

        int tempVar6 = static_cast<int>(floor(static_cast<double>((level[S].Height - level[S].Y) / GFXBackground2Height[A] * 0.5 + (double)screen.H * 2 / GFXBackground2Height[A]))) + 1;
        for(B = 0; B <= tempVar6; B++)
        {
            // why is the background 381px...
            tempLocation.Y = level[S].Y + ((B * GFXBackground2Height[A] - B) - (camY + Eff_Top + level[S].Y) * 0.5) - 32;
            if(GameMenu && level[S].Y > -camY)
            {
                tempLocation.Y = -camY + B * (GFXBackground2Height[A] - 1);
            }

            if(level[S].Width - level[S].X > GFXBackground2Width[A])
            {
                tempLocation.X = (-camX - level[S].X - vScreen[Z].Left) / (level[S].Width - level[S].X - 800) * (GFXBackground2Width[A] - 800) + vScreen[Z].Left;
                tempLocation.X = -camX - tempLocation.X;
            }
            else
                tempLocation.X = level[S].Width - GFXBackground2Width[A];

            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];

            double X_right = tempLocation.X + GFXBackground2Width[A];
            while(tempLocation.X + tempLocation.Width > -camX
                && tempLocation.Y < -camY + vScreen[Z].Height
                && tempLocation.Y + tempLocation.Height > -camY)
            {
                XRender::renderTexture(camX + tempLocation.X, camY + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
                tempLocation.X -= GFXBackground2Width[A];
            }
            tempLocation.X = X_right;
            while(tempLocation.X < -camX + vScreen[Z].Width
                && tempLocation.Y < -camY + vScreen[Z].Height
                && tempLocation.Y + tempLocation.Height > -camY)
            {
                XRender::renderTexture(camX + tempLocation.X, camY + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
                tempLocation.X += GFXBackground2Width[A];
            }
        }
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
        DrawCenterAnchoredBackground(S, Z, A, -1, 0, 0, false, 3, 4);
    }

    A = 10; // Night 2
    if(Background2[S] == 9)
    {
        DrawBottomAnchoredBackground(S, Z, A, 0, -1, 0, 0.5, true);
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
        DrawCenterAnchoredBackground(S, Z, A, 0, 0, 0, false, 1, 2, true);
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
        DrawBottomAnchoredBackground(S, Z, A, 0, -1, 0, 0.5, true, true);
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
        // another y-tiled one
        double Eff_Top = 0;
        if(screen.Type == ScreenTypes::Dynamic)
            Eff_Top = vScreen[Z].Top;

        int tempVar26 = static_cast<int>(floor(static_cast<double>((level[S].Height - level[S].Y) / GFXBackground2Height[A] * 0.5 + (double)screen.H * 2 / GFXBackground2Height[A]))) + 1;
        for(B = 0; B <= tempVar26; B++)
        {
            tempLocation.Y = level[S].Y + ((B * GFXBackground2Height[A] - B) - (camY + Eff_Top + level[S].Y) * 0.5) - 32;
            if(GameMenu && level[S].Y > -camY)
            {
                tempLocation.Y -= level[S].Y + camY;
            }
            // .X = Level(S).X
            if(level[S].Width - level[S].X > GFXBackground2Width[A])
            {
                // .X = (-vScreenX(Z) - level(S).X) / (level(S).Width - level(S).X - 800) * (GFXBackground2Width(A) - 800)
                // note: fixed the left alignment to match the vanilla game
                tempLocation.X = (-camX - level[S].X - vScreen[Z].Left) / (level[S].Width - level[S].X - 800) * (GFXBackground2Width[A] - 800) + vScreen[Z].Left;
                tempLocation.X = -camX - tempLocation.X;
            }
            else
                tempLocation.X = level[S].Width - GFXBackground2Width[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];

            while(tempLocation.Y + tempLocation.Height > -camY
                && tempLocation.Y < -camY + vScreen[Z].Height
                && tempLocation.X < -camX + vScreen[Z].Width)
            {
                XRender::renderTexture(camX + tempLocation.X, camY + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
                tempLocation.X += GFXBackground2Width[A];
            }
        }
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
        DrawCenterAnchoredBackground(S, Z, A, -1, 0, 0, false, 1, 2, true);
    }

    A = 30; // SMW Cave
    if(Background2[S] == 30)
    {
        DrawCenterAnchoredBackground(S, Z, A, 3456, 0, 0, true, 1, 2, true);
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
        DrawBottomAnchoredBackground(S, Z, A, 500, 0, 0, 0.75);
    }

    if(Background2[S] == 36)
    {
        DrawCenterAnchoredBackground(S, Z, A, 0, 0, 0, false, 3, 4);
    }

    A = 35; // SMB 3 Snow Trees
    if(Background2[S] == 35)
    {
        DrawBottomAnchoredBackground(S, Z, A, 0, -1, 0, 0.5, true);
    }

    A = 37; // SMB 3 Snow Hills
    if(Background2[S] == 37)
    {
        DrawBottomAnchoredBackground(S, Z, A, 0, -1, 0, 0.5, true);
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
        double Eff_Top = 0;
        if(screen.Type == ScreenTypes::Dynamic)
            Eff_Top = vScreen[Z].Top;

        if(g_config.disable_background2_tiling)
        {
            XRender::lazyPreLoad(GFXBackground2[A]);
            XRender::renderRect(0, 0, vScreen[Z].Width, vScreen[Z].Height,
                GFXBackground2[A].ColorUpper);
        }

        if(g_config.allow_multires && (vScreen[Z].Height > GFXBackground2Height[A] || (vScreen[Z].Height == screen.H / 2 && screen.H > GFXBackground2Height[A])))
        {
            tempLocation.Y = level[S].Height - GFXBackground2Height[A];
        }
        else if(level[S].Height - level[S].Y > GFXBackground2Height[A])
        {



            // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - (screen.H - vScreen(Z).Top)) * (GFXBackground2Height(A) - (screen.H - vScreen(Z).Top))
            // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)



            tempLocation.Y = (-camY - Eff_Top - level[S].Y) / (level[S].Height - level[S].Y - screen.H) * (GFXBackground2Height[A] - screen.H) + Eff_Top;
            tempLocation.Y = -camY - tempLocation.Y;
        }
        else
            tempLocation.Y = level[S].Height - GFXBackground2Height[A];

        tempLocation.Height = GFXBackground2Height[A];
        tempLocation.Width = GFXBackground2Width[A];

        int levelX = SDL_round(level[S].X);
        int tempLocY = SDL_round(tempLocation.Y);

        int camX_levelX = camX + levelX;
        int Left = vScreen[Z].Left;

        // ADDED A LOOP HERE, not indenting to avoid a huge diff hunk
        do {

        int offsetX = (camX_levelX * 1 - Left * 1) / 2;
        for(; offsetX < vScreen[Z].Width; offsetX += GFXBackground2[A].w)
        {
            if(offsetX + GFXBackground2[A].w <= 0)
                continue;

            XRender::renderTextureBasic(offsetX, camY + tempLocY + 953, GFXBackground2[A].w, 47, GFXBackground2[A], 0, 953);
        }

        offsetX = (camX_levelX * 4 - Left * 6) / 10;
        for(; offsetX < vScreen[Z].Width; offsetX += GFXBackground2[A].w)
        {
            if(offsetX + GFXBackground2[A].w <= 0)
                continue;

            XRender::renderTextureBasic(offsetX, camY + tempLocY + 916, GFXBackground2[A].w, 37, GFXBackground2[A], 0, 916);
        }

        offsetX = (camX_levelX * 3 - Left * 7) / 10;
        for(; offsetX < vScreen[Z].Width; offsetX += GFXBackground2[A].w)
        {
            if(offsetX + GFXBackground2[A].w <= 0)
                continue;

            XRender::renderTextureBasic(offsetX, camY + tempLocY + 849, GFXBackground2[A].w, 67, GFXBackground2[A], 0, 849);
        }

        offsetX = (camX_levelX * 2 - Left * 8) / 10;
        for(; offsetX < vScreen[Z].Width; offsetX += GFXBackground2[A].w)
        {
            if(offsetX + GFXBackground2[A].w <= 0)
                continue;

            XRender::renderTextureBasic(offsetX, camY + tempLocY + 815, GFXBackground2[A].w, 34, GFXBackground2[A], 0, 815);
        }

        offsetX = (camX_levelX * 1 - Left * 9) / 10;
        for(; offsetX < vScreen[Z].Width; offsetX += GFXBackground2[A].w)
        {
            if(offsetX + GFXBackground2[A].w <= 0)
                continue;

            XRender::renderTextureBasic(offsetX, camY + tempLocY + 709, GFXBackground2[A].w, 106, GFXBackground2[A], 0, 709);
        }

        offsetX = (camX_levelX * 15 - Left * 85) / 100;
        for(; offsetX < vScreen[Z].Width; offsetX += GFXBackground2[A].w)
        {
            if(offsetX + GFXBackground2[A].w <= 0)
                continue;

            XRender::renderTextureBasic(offsetX, camY + tempLocY + 664, GFXBackground2[A].w, 45, GFXBackground2[A], 0, 664);
        }

        offsetX = (camX_levelX * 2 - Left * 8) / 10;
        for(; offsetX < vScreen[Z].Width; offsetX += GFXBackground2[A].w)
        {
            if(offsetX + GFXBackground2[A].w <= 0)
                continue;

            XRender::renderTextureBasic(offsetX, camY + tempLocY + 614, GFXBackground2[A].w, 50, GFXBackground2[A], 0, 614);
        }

        offsetX = (camX_levelX * 25 - Left * 75) / 100;
        for(; offsetX < vScreen[Z].Width; offsetX += GFXBackground2[A].w)
        {
            if(offsetX + GFXBackground2[A].w <= 0)
                continue;

            XRender::renderTextureBasic(offsetX, camY + tempLocY + 540, GFXBackground2[A].w, 74, GFXBackground2[A], 0, 540);
        }

        offsetX = (camX_levelX * 3 - Left * 7) / 10;
        for(; offsetX < vScreen[Z].Width; offsetX += GFXBackground2[A].w)
        {
            if(offsetX + GFXBackground2[A].w <= 0)
                continue;

            XRender::renderTextureBasic(offsetX, camY + tempLocY + 408, GFXBackground2[A].w, 132, GFXBackground2[A], 0, 408);
        }

        offsetX = (camX_levelX * 25 - Left * 75) / 100;
        for(; offsetX < vScreen[Z].Width; offsetX += GFXBackground2[A].w)
        {
            if(offsetX + GFXBackground2[A].w <= 0)
                continue;

            XRender::renderTextureBasic(offsetX, camY + tempLocY + 333, GFXBackground2[A].w, 75, GFXBackground2[A], 0, 333);
        }

        offsetX = (camX_levelX * 2 - Left * 8) / 10;
        for(; offsetX < vScreen[Z].Width; offsetX += GFXBackground2[A].w)
        {
            if(offsetX + GFXBackground2[A].w <= 0)
                continue;

            XRender::renderTextureBasic(offsetX, camY + tempLocY + 278, GFXBackground2[A].w, 55, GFXBackground2[A], 0, 278);
        }

        offsetX = (camX_levelX * 15 - Left * 85) / 100;
        for(; offsetX < vScreen[Z].Width; offsetX += GFXBackground2[A].w)
        {
            if(offsetX + GFXBackground2[A].w <= 0)
                continue;

            XRender::renderTextureBasic(offsetX, camY + tempLocY + 235, GFXBackground2[A].w, 43, GFXBackground2[A], 0, 235);
        }

        offsetX = (camX_levelX * 1 - Left * 9) / 10;
        for(; offsetX < vScreen[Z].Width; offsetX += GFXBackground2[A].w)
        {
            if(offsetX + GFXBackground2[A].w <= 0)
                continue;

            XRender::renderTextureBasic(offsetX, camY + tempLocY + 123, GFXBackground2[A].w, 112, GFXBackground2[A], 0, 123);
        }

        offsetX = (camX_levelX * 2 - Left * 8) / 10;
        for(; offsetX < vScreen[Z].Width; offsetX += GFXBackground2[A].w)
        {
            if(offsetX + GFXBackground2[A].w <= 0)
                continue;

            XRender::renderTextureBasic(offsetX, camY + tempLocY + 85, GFXBackground2[A].w, 38, GFXBackground2[A], 0, 85);
        }

        offsetX = (camX_levelX * 3 - Left * 7) / 10;
        for(; offsetX < vScreen[Z].Width; offsetX += GFXBackground2[A].w)
        {
            if(offsetX + GFXBackground2[A].w <= 0)
                continue;

            XRender::renderTextureBasic(offsetX, camY + tempLocY + 48, GFXBackground2[A].w, 37, GFXBackground2[A], 0, 48);
        }

        offsetX = (camX_levelX * 4 - Left * 6) / 10;
        for(; offsetX < vScreen[Z].Width; offsetX += GFXBackground2[A].w)
        {
            if(offsetX + GFXBackground2[A].w <= 0)
                continue;

            XRender::renderTextureBasic(offsetX, camY + tempLocY + 0, GFXBackground2[A].w, 48, GFXBackground2[A], 0, 0);
        }

        tempLocY -= GFXBackground2Height[A];

        if(g_config.disable_background2_tiling)
            break;

        } while(tempLocY + GFXBackground2Height[A] > -camY);
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
        DrawCenterAnchoredBackground(S, Z, A, 3456, 160, 546, false, 1, 2, true);
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
        double Eff_ScreenH = vScreen[Z].Height;
        double Eff_Top = 0;
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

        double CanvasH = GFXBackground2Height[A];
        double CanvasOffset = 0;

        // ensure that the canvas covers above and below the screen
        if(Eff_ScreenH > CanvasH)
        {
            CanvasOffset = Eff_ScreenH - CanvasH;
            CanvasH += CanvasOffset * 2;
        }

        if(level[S].Height - level[S].Y > CanvasH)
        {
            // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
            tempLocation.Y = (-camY - Eff_Top - level[S].Y) / (level[S].Height - level[S].Y - Eff_ScreenH) * (CanvasH - Eff_ScreenH) + Eff_Top;
            tempLocation.Y = -camY - tempLocation.Y;
            tempLocation.Y += CanvasOffset;
        }
        else if(CanvasH > GFXBackground2Height[A])
        {
            tempLocation.Y = level[S].Y + (level[S].Height - level[S].Y) / 2 - GFXBackground2Height[A] / 2;
        }
        else
            tempLocation.Y = level[S].Height - GFXBackground2Height[A];
        tempLocation.Height = GFXBackground2Height[A];
        tempLocation.Width = GFXBackground2Width[A];

        // use a simple color fill for the sky above and sand below the texture
        XRender::lazyPreLoad(GFXBackground2[A]);
        double undrawn_above = tempLocation.Y + camY;
        double undrawn_below = tempLocation.Y + camY + tempLocation.Height;
        XRender::renderRect(0, 0, vScreen[Z].Width, std::ceil(undrawn_above),
            GFXBackground2[A].ColorUpper);
        XRender::renderRect(0, undrawn_below, vScreen[Z].Width, vScreen[Z].Height - undrawn_below + 1,
            GFXBackground2[A].ColorLower);

        int tempVar66 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5 + (double)screen.W / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar66; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (camX + vScreen[Z].Left + level[S].X) * 0.5);
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(camX + tempLocation.X, camY + tempLocation.Y + 280, GFXBackground2Width[A], 450, GFXBackground2[A], 0, 280);
            }
        }

        int tempVar67 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.9 + (double)screen.W / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar67; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (camX + vScreen[Z].Left + level[S].X) * 0.9);
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(camX + tempLocation.X, camY + tempLocation.Y + 268, GFXBackground2Width[A], 12, GFXBackground2[A], 0, 268);
            }
        }

        int tempVar68 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.89 + (double)screen.W / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar68; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (camX + vScreen[Z].Left + level[S].X) * 0.89);
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(camX + tempLocation.X, camY + tempLocation.Y + 244, GFXBackground2Width[A], 24, GFXBackground2[A], 0, 244);
            }
        }

        int tempVar69 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.88 + (double)screen.W / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar69; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (camX + vScreen[Z].Left + level[S].X) * 0.88);
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(camX + tempLocation.X, camY + tempLocation.Y + 228, GFXBackground2Width[A], 16, GFXBackground2[A], 0, 228);
            }
        }

        int tempVar70 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.87 + (double)screen.W / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar70; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (camX + vScreen[Z].Left + level[S].X) * 0.87);
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(camX + tempLocation.X, camY + tempLocation.Y + 196, GFXBackground2Width[A], 32, GFXBackground2[A], 0, 196);
            }
        }

        int tempVar71 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.86 + (double)screen.W / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar71; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (camX + vScreen[Z].Left + level[S].X) * 0.86);
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(camX + tempLocation.X, camY + tempLocation.Y + 164, GFXBackground2Width[A], 32, GFXBackground2[A], 0, 164);
            }
        }

        int tempVar72 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.85 + (double)screen.W / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar72; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (camX + vScreen[Z].Left + level[S].X) * 0.85);
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(camX + tempLocation.X, camY + tempLocation.Y + 116, GFXBackground2Width[A], 48, GFXBackground2[A], 0, 116);
            }
        }

        int tempVar73 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.84 + (double)screen.W / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar73; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (camX + vScreen[Z].Left + level[S].X) * 0.84);
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(camX + tempLocation.X, camY + tempLocation.Y + 58, GFXBackground2Width[A], 58, GFXBackground2[A], 0, 58);
            }
        }

        int tempVar74 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.83 + (double)screen.W / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar74; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (camX + vScreen[Z].Left + level[S].X) * 0.83);
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(camX + tempLocation.X, camY + tempLocation.Y, GFXBackground2Width[A], 58, GFXBackground2[A], 0, 0);
            }
        }
    }
    A = 50; // Shrooms
    if(Background2[S] == 50)
    {
        XRender::lazyPreLoad(GFXBackground2[A]);
        XRender::renderRect(0, 0, vScreen[Z].Width, vScreen[Z].Height,
            GFXBackground2[A].ColorLower);

        double Eff_ScreenH = vScreen[Z].Height;
        double Eff_Top = 0;
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

        double CanvasH = GFXBackground2Height[A];
        double CanvasOffset = 0;

        // ensure that the canvas covers above and below the screen
        if(Eff_ScreenH > CanvasH)
        {
            CanvasOffset = Eff_ScreenH - CanvasH;
            CanvasH += CanvasOffset * 2;
        }

        if(level[S].Height - level[S].Y > CanvasH)
        {
            // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
            tempLocation.Y = (-camY - Eff_Top - level[S].Y) / (level[S].Height - level[S].Y - Eff_ScreenH) * (CanvasH - Eff_ScreenH) + Eff_Top;
            tempLocation.Y = -camY - tempLocation.Y;
            tempLocation.Y += CanvasOffset;
        }
        else if(CanvasH > GFXBackground2Height[A])
        {
            tempLocation.Y = level[S].Y + (level[S].Height - level[S].Y) / 2 - GFXBackground2Height[A] / 2;
        }
        else
            tempLocation.Y = level[S].Height - GFXBackground2Height[A];
        tempLocation.Height = GFXBackground2Height[A];
        tempLocation.Width = GFXBackground2Width[A];

        int tempVar75 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5 + (double)screen.W / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar75; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (camX + vScreen[Z].Left + level[S].X) * 0.5);
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(camX + tempLocation.X, camY + tempLocation.Y + 378, GFXBackground2Width[A], 378, GFXBackground2[A], 0, 378);
            }
        }

        while(tempLocation.Y + 378 > -camY)
        {
            int tempVar76 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.65 + (double)screen.W / GFXBackground2Width[A]))) + 1;
            for(B = 0; B <= tempVar76; B++)
            {
                tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (camX + vScreen[Z].Left + level[S].X) * 0.65);
                if(vScreenCollision(Z, tempLocation))
                {
                    XRender::renderTexture(camX + tempLocation.X, camY + tempLocation.Y, GFXBackground2Width[A], 220, GFXBackground2[A], 0, 0);
                }
            }

            int tempVar77 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.6 + (double)screen.W / GFXBackground2Width[A]))) + 1;
            for(B = 0; B <= tempVar77; B++)
            {
                tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (camX + vScreen[Z].Left + level[S].X) * 0.6);
                if(vScreenCollision(Z, tempLocation))
                {
                    XRender::renderTexture(camX + tempLocation.X, camY + tempLocation.Y + 220, GFXBackground2Width[A], 158, GFXBackground2[A], 0, 220);
                }
            }

            tempLocation.Y -= 378;

            if(g_config.disable_background2_tiling)
                break;
        }
    }

    A = 51; // SMB1 Desert
    if(Background2[S] == 51)
    {
        double Eff_ScreenH = vScreen[Z].Height;
        double Eff_Top = 0;
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

        double CanvasH = GFXBackground2Height[A];
        double CanvasOffset = 0;

        // ensure that the canvas covers above and below the screen
        if(Eff_ScreenH > CanvasH)
        {
            CanvasOffset = 0;
            CanvasH += (Eff_ScreenH - CanvasH);
        }

        if(level[S].Height - level[S].Y > CanvasH)
        {
            // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
            tempLocation.Y = (-camY - Eff_Top - level[S].Y) / (level[S].Height - level[S].Y - Eff_ScreenH) * (CanvasH - Eff_ScreenH) + Eff_Top;
            tempLocation.Y = -camY - tempLocation.Y;
            tempLocation.Y += CanvasOffset;
        }
        else if(CanvasH > GFXBackground2Height[A])
        {
            tempLocation.Y = level[S].Y + (level[S].Height - level[S].Y) / 2 - GFXBackground2Height[A] / 2;
        }
        else
            tempLocation.Y = level[S].Height - GFXBackground2Height[A];
        tempLocation.Height = GFXBackground2Height[A];
        tempLocation.Width = GFXBackground2Width[A];

        // use a simple color fill for the sky above and sand below the texture
        XRender::lazyPreLoad(GFXBackground2[A]);
        double undrawn_above = tempLocation.Y + camY;
        double undrawn_below = tempLocation.Y + camY + tempLocation.Height;
        XRender::renderRect(0, 0, vScreen[Z].Width, std::ceil(undrawn_above),
            GFXBackground2[A].ColorUpper);
        XRender::renderRect(0, undrawn_below, vScreen[Z].Width, vScreen[Z].Height - undrawn_below + 1,
            GFXBackground2[A].ColorLower);

        int tempVar78 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.75 + (double)screen.W / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar78; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (camX + vScreen[Z].Left + level[S].X) * 0.75);
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(camX + tempLocation.X, camY + tempLocation.Y, GFXBackground2Width[A], 350, GFXBackground2[A], 0, 0);
            }
        }

        int tempVar79 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5 + (double)screen.W / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar79; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (camX + vScreen[Z].Left + level[S].X) * 0.5);
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(camX + tempLocation.X, camY + tempLocation.Y + 350, GFXBackground2Width[A], GFXBackground2Height[A] - 350, GFXBackground2[A], 0, 350);
            }
        }
    }
    A = 52; // SMB2 Desert Night
    if(Background2[S] == 52)
    {
        double Eff_ScreenH = vScreen[Z].Height;
        double Eff_Top = 0;
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

        double CanvasH = GFXBackground2Height[A];
        double CanvasOffset = 0;

        // ensure that the canvas covers above and below the screen
        if(Eff_ScreenH > CanvasH)
        {
            CanvasOffset = (Eff_ScreenH - CanvasH) * 1.5;
            CanvasH += CanvasOffset;
        }

        if(level[S].Height - level[S].Y > CanvasH)
        {
            // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
            tempLocation.Y = (-camY - Eff_Top - level[S].Y) / (level[S].Height - level[S].Y - Eff_ScreenH) * (CanvasH - Eff_ScreenH) + Eff_Top;
            tempLocation.Y = -camY - tempLocation.Y;
            tempLocation.Y += CanvasOffset;
        }
        else if(CanvasH > GFXBackground2Height[A])
        {
            tempLocation.Y = level[S].Y + (level[S].Height - level[S].Y) / 2 - GFXBackground2Height[A] / 2;
        }
        else
            tempLocation.Y = level[S].Height - GFXBackground2Height[A];
        tempLocation.Height = GFXBackground2Height[A];
        tempLocation.Width = GFXBackground2Width[A];

        // use a simple color fill for the sky above and sand below the texture
        XRender::lazyPreLoad(GFXBackground2[A]);
        double undrawn_above = tempLocation.Y + camY;
        double undrawn_below = tempLocation.Y + camY + tempLocation.Height;
        XRender::renderRect(0, 0, vScreen[Z].Width, std::ceil(undrawn_above),
            GFXBackground2[A].ColorUpper);
        XRender::renderRect(0, undrawn_below, vScreen[Z].Width, vScreen[Z].Height - undrawn_below + 1,
            GFXBackground2[A].ColorLower);

        int tempVar80 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5 + (double)screen.W / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar80; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (camX + vScreen[Z].Left + level[S].X) * 0.5);
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(camX + tempLocation.X, camY + tempLocation.Y + 280, GFXBackground2Width[A], GFXBackground2Height[A] - 280, GFXBackground2[A], 0, 280);
            }
        }

        int tempVar81 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.9 + (double)screen.W / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar81; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (camX + vScreen[Z].Left + level[S].X) * 0.9);
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(camX + tempLocation.X, camY + tempLocation.Y + 268, GFXBackground2Width[A], 12, GFXBackground2[A], 0, 268);
            }
        }

        int tempVar82 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.89 + (double)screen.W / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar82; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (camX + vScreen[Z].Left + level[S].X) * 0.89);
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(camX + tempLocation.X, camY + tempLocation.Y + 244, GFXBackground2Width[A], 24, GFXBackground2[A], 0, 244);
            }
        }

        int tempVar83 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.88 + (double)screen.W / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar83; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (camX + vScreen[Z].Left + level[S].X) * 0.88);
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(camX + tempLocation.X, camY + tempLocation.Y + 228, GFXBackground2Width[A], 16, GFXBackground2[A], 0, 228);
            }
        }

        int tempVar84 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.87 + (double)screen.W / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar84; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (camX + vScreen[Z].Left + level[S].X) * 0.87);
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(camX + tempLocation.X, camY + tempLocation.Y + 196, GFXBackground2Width[A], 32, GFXBackground2[A], 0, 196);
            }
        }

        int tempVar85 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.86 + (double)screen.W / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar85; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (camX + vScreen[Z].Left + level[S].X) * 0.86);
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(camX + tempLocation.X, camY + tempLocation.Y + 164, GFXBackground2Width[A], 32, GFXBackground2[A], 0, 164);
            }
        }

        int tempVar86 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.85 + (double)screen.W / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar86; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (camX + vScreen[Z].Left + level[S].X) * 0.85);
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(camX + tempLocation.X, camY + tempLocation.Y + 116, GFXBackground2Width[A], 48, GFXBackground2[A], 0, 116);
            }
        }

        int tempVar87 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.84 + (double)screen.W / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar87; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (camX + vScreen[Z].Left + level[S].X) * 0.84);
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(camX + tempLocation.X, camY + tempLocation.Y + 58, GFXBackground2Width[A], 58, GFXBackground2[A], 0, 58);
            }
        }

        int tempVar88 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.83 + (double)screen.W / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar88; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (camX + vScreen[Z].Left + level[S].X) * 0.83);
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(camX + tempLocation.X, camY + tempLocation.Y, GFXBackground2Width[A], 58, GFXBackground2[A], 0, 0);
            }
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
        DrawCenterAnchoredBackground(S, Z, A, 0, 0, 0, false, 1, 2, true);
    }

    A = 56; // SMB3 Water
    if(Background2[S] == 56)
    {
        double Eff_ScreenH = vScreen[Z].Height;
        double Eff_Top = 0;
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

        double CanvasH = GFXBackground2Height[A];
        double CanvasOffset = 0;

        // ensure that the canvas covers above and below the screen
        if(Eff_ScreenH > CanvasH)
        {
            CanvasOffset = (Eff_ScreenH - CanvasH) * 1.5;
            CanvasH += CanvasOffset;
        }

        if(level[S].Height - level[S].Y > CanvasH)
        {
            // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
            tempLocation.Y = (-camY - Eff_Top - level[S].Y) / (level[S].Height - level[S].Y - Eff_ScreenH) * (CanvasH - Eff_ScreenH) + Eff_Top;
            tempLocation.Y = -camY - tempLocation.Y;
            tempLocation.Y += CanvasOffset;
        }
        else if(CanvasH > GFXBackground2Height[A])
        {
            tempLocation.Y = level[S].Y + (level[S].Height - level[S].Y) / 2 - GFXBackground2Height[A] / 2;
        }
        else
            tempLocation.Y = level[S].Height - GFXBackground2Height[A];
        tempLocation.Height = GFXBackground2Height[A];
        tempLocation.Width = GFXBackground2Width[A];

        // use a simple color fill for the water above and below the texture
        XRender::lazyPreLoad(GFXBackground2[A]);
        double undrawn_above = tempLocation.Y + camY;
        double undrawn_below = tempLocation.Y + camY + tempLocation.Height;
        XRender::renderRect(0, 0, vScreen[Z].Width, std::ceil(undrawn_above),
            GFXBackground2[A].ColorUpper);
        XRender::renderRect(0, undrawn_below, vScreen[Z].Width, vScreen[Z].Height - undrawn_below + 1,
            GFXBackground2[A].ColorLower);

        int tempVar92 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.65 + (double)screen.W / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar92; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (camX + vScreen[Z].Left + level[S].X) * 0.65);
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(camX + tempLocation.X, camY + tempLocation.Y, GFXBackground2Width[A], 100, GFXBackground2[A], 0, 0);
            }
        }


        int tempVar93 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.6 + (double)screen.W / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar93; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (camX + vScreen[Z].Left + level[S].X) * 0.6);
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(camX + tempLocation.X, camY + tempLocation.Y + 100, GFXBackground2Width[A], 245, GFXBackground2[A], 0, 100);
            }
        }


        int tempVar94 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.55 + (double)screen.W / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar94; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (camX + vScreen[Z].Left + level[S].X) * 0.55);
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(camX + tempLocation.X, camY + tempLocation.Y + 345, GFXBackground2Width[A], 110, GFXBackground2[A], 0, 345);
            }
        }


        int tempVar95 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5 + (double)screen.W / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar95; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (camX + vScreen[Z].Left + level[S].X) * 0.5);
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(camX + tempLocation.X, camY + tempLocation.Y + 455, GFXBackground2Width[A], GFXBackground2Height[A] - 455, GFXBackground2[A], 0, 455);
            }
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
        DrawCenterAnchoredBackground(S, Z, A, 3584, 128, 600, false, 1, 2, true);
    }

    level[S] = tempLevel;

}
