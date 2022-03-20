/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2022 Vitaly Novichkov <admin@wohlnet.ru>
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
#include "../compat.h"


void DrawBackground(int S, int Z)
{
    int A = 0;
    int B = 0;
    Location_t tempLocation;
    Location_t tempLevel;
    tempLevel = level[S];
    if(LevelEditor)
        LevelREAL[S] = level[S];
    level[S] = LevelREAL[S];

    if(Background2[S] == 0)
    {
        XRender::renderRect(0, 0, vScreen[Z].Width, vScreen[Z].Height, 0.f, 0.f, 0.f, 1.f, true);
    }
    A = 1; // Blocks
    if(Background2[S] == 1)
    {
        int tempVar = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }

    A = 2; // Clouds
    if(Background2[S] == 1 || Background2[S] == 2 || Background2[S] == 22)
    {
        int tempVar2 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.75 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar2; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.75);
            // the lower graphics is 500px tall, so we really need exactly this offset
            tempLocation.Y = level[S].Height - GFXBackground2Height[A] - 600 + 100;
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];

            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }

            while(tempLocation.Y > level[S].Y || tempLocation.Y > -vScreenY[Z])
            {
                tempLocation.Y -= tempLocation.Height;
                if(vScreenCollision(Z, tempLocation))
                    XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }

    if(Background2[S] == 13) // clouds
    {

        int tempVar3 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.75 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar3; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.75);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - 600) * (GFXBackground2Height[A] - 600) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];

            // this motif, repeated a few times in this file, tiles a texture in two directions (up and down here)
            double bottom_Y = tempLocation.Y + GFXBackground2Height[A];
            while(tempLocation.X + tempLocation.Width > -vScreenX[Z]
                && tempLocation.X < -vScreenX[Z] + vScreen[Z].Width
                && tempLocation.Y + tempLocation.Height > -vScreenY[Z])
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
                tempLocation.Y -= GFXBackground2Height[A];
            }
            tempLocation.Y = bottom_Y;
            while(tempLocation.X + tempLocation.Width > -vScreenX[Z]
                && tempLocation.X < -vScreenX[Z] + vScreen[Z].Width
                && tempLocation.Y < -vScreenY[Z] + vScreen[Z].Height)
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
                tempLocation.Y += GFXBackground2Height[A];
            }

        }
    }

    A = 3; // Hills
    if(Background2[S] == 2)
    {

        int tempVar4 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar4; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }

    A = 4; // Castle
    if(Background2[S] == 3)
    {

        int tempVar5 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar5; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - 600) * (GFXBackground2Height[A] - 600) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];

            double bottom_Y = tempLocation.Y + GFXBackground2Height[A];
            while(tempLocation.X + tempLocation.Width > -vScreenX[Z]
                && tempLocation.X < -vScreenX[Z] + vScreen[Z].Width
                && tempLocation.Y + tempLocation.Height > -vScreenY[Z])
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
                tempLocation.Y -= GFXBackground2Height[A];
            }
            tempLocation.Y = bottom_Y;
            while(tempLocation.X + tempLocation.Width > -vScreenX[Z]
                && tempLocation.X < -vScreenX[Z] + vScreen[Z].Width
                && tempLocation.Y < -vScreenY[Z] + vScreen[Z].Height)
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
                tempLocation.Y += GFXBackground2Height[A];
            }
        }
    }

    A = 5; // Pipes
    if(Background2[S] == 4)
    {

        int tempVar6 = static_cast<int>(floor(static_cast<double>((level[S].Height - level[S].Y) / GFXBackground2Height[A] * 0.5 + (double)ScreenH * 2 / GFXBackground2Height[A]))) + 1;
        for(B = 0; B <= tempVar6; B++)
        {
            // why is the background 381px...
            tempLocation.Y = level[S].Y + ((B * GFXBackground2Height[A] - B) - (vScreenY[Z] + vScreen[Z].Top + level[S].Y) * 0.5) - 32;
            if(GameMenu && level[S].Y > -vScreenY[Z])
            {
                tempLocation.Y = -vScreenY[Z] + B * (GFXBackground2Height[A] - 1);
            }

            if(level[S].Width - level[S].X > GFXBackground2Width[A])
            {
                tempLocation.X = (-vScreenX[Z] - level[S].X) / (level[S].Width - level[S].X - (800 - vScreen[Z].Left)) * (GFXBackground2Width[A] - (800 - vScreen[Z].Left));
                tempLocation.X = -vScreenX[Z] - tempLocation.X;
            }
            else
                tempLocation.X = level[S].Width - GFXBackground2Width[A];

            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];

            double X_right = tempLocation.X + GFXBackground2Width[A];
            while(tempLocation.X + tempLocation.Width > -vScreenX[Z]
                && tempLocation.Y < -vScreenY[Z] + vScreen[Z].Height
                && tempLocation.Y + tempLocation.Height > -vScreenY[Z])
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
                tempLocation.X -= GFXBackground2Width[A];
            }
            tempLocation.X = X_right;
            while(tempLocation.X < -vScreenX[Z] + vScreen[Z].Width
                && tempLocation.Y < -vScreenY[Z] + vScreen[Z].Height
                && tempLocation.Y + tempLocation.Height > -vScreenY[Z])
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
                tempLocation.X += GFXBackground2Width[A];
            }
        }
    }

    A = 6; // Trees
    if(Background2[S] == 5)
    {
        // use a simple color fill for the sky beyond the texture
        // (bonus: allows smaller texture in asset packs!)
        if(tempLocation.Y > -vScreenY[Z])
        {
            XRender::lazyPreLoad(GFXBackground2[A]);
            XRender::renderRect(0, 0, vScreen[Z].Width, vScreen[Z].Height,
                GFXBackground2[A].ColorUpper.r, GFXBackground2[A].ColorUpper.g, GFXBackground2[A].ColorUpper.b);
        }

        int tempVar7 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = 0; B <= tempVar7; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(g_compatibility.free_level_res && (vScreen[Z].Height > GFXBackground2Height[A] || (vScreen[Z].Height == ScreenH / 2 && ScreenH > GFXBackground2Height[A])))
            {
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            }
            else if(level[S].Height - level[S].Y > GFXBackground2Height[A])
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] - ScreenH) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }

    A = 7; // Bonus
    if(Background2[S] == 6)
    {

        int tempVar8 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar8; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - 600) * (GFXBackground2Height[A] - 600) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];

            double bottom_Y = tempLocation.Y + GFXBackground2Height[A];
            while(tempLocation.X + tempLocation.Width > -vScreenX[Z]
                && tempLocation.X < -vScreenX[Z] + vScreen[Z].Width
                && tempLocation.Y + tempLocation.Height > -vScreenY[Z])
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
                tempLocation.Y -= GFXBackground2Height[A];
            }
            tempLocation.Y = bottom_Y;
            while(tempLocation.X + tempLocation.Width > -vScreenX[Z]
                && tempLocation.X < -vScreenX[Z] + vScreen[Z].Width
                && tempLocation.Y < -vScreenY[Z] + vScreen[Z].Height)
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
                tempLocation.Y += GFXBackground2Height[A];
            }
        }
    }

    A = 8; // SMB Underground
    if(Background2[S] == 7)
    {

        int tempVar9 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar9; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            tempLocation.Y = level[S].Y - 32;
            if(GameMenu && tempLocation.Y > -vScreenY[Z])
                tempLocation.Y = -vScreenY[Z];

            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];

            while(tempLocation.X + tempLocation.Width > -vScreenX[Z]
                && tempLocation.X < -vScreenX[Z] + vScreen[Z].Width
                && tempLocation.Y < -vScreenY[Z] + vScreen[Z].Height)
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
                tempLocation.Y += GFXBackground2Height[A];
            }
        }
    }

    A = 9; // Night
    if(Background2[S] == 8 || Background2[S] == 9)
    {
        XRender::lazyPreLoad(GFXBackground2[A]);
        XRender::renderRect(0, 0, vScreen[Z].Width, vScreen[Z].Height,
            GFXBackground2[A].ColorUpper.r, GFXBackground2[A].ColorUpper.g, GFXBackground2[A].ColorUpper.b);

        int tempVar10 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.75 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar10; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.75);
            if(g_compatibility.free_level_res && (vScreen[Z].Height > GFXBackground2Height[A] || (vScreen[Z].Height == ScreenH / 2 && ScreenH > GFXBackground2Height[A])))
            {
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            }
            else if(level[S].Height - level[S].Y > GFXBackground2Height[A])
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] - ScreenH) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];

            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }

    A = 10; // Night 2
    if(Background2[S] == 9)
    {

        int tempVar11 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar11; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }

    A = 11; // Overworld
    if(Background2[S] == 10)
    {

        int tempVar12 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar12; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(g_compatibility.free_level_res && (vScreen[Z].Height > GFXBackground2Height[A] || (vScreen[Z].Height == ScreenH / 2 && ScreenH > GFXBackground2Height[A])))
            {
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            }
            else if(level[S].Height - level[S].Y > GFXBackground2Height[A])
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] - ScreenH) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];

            if(tempLocation.X + tempLocation.Width > -vScreenX[Z]
                && tempLocation.X < -vScreenX[Z] + vScreen[Z].Width)
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);

                // clouds occupy top 640px of vanilla asset
                if(tempLocation.Y > -vScreenY[Z])
                {
                    if(GFXBackground2Height[A] < 640 || g_compatibility.disable_background2_tiling)
                    {
                        // color fill fallback
                        double color_height = tempLocation.Y + vScreenY[Z];
                        XRender::renderRect(vScreenX[Z] + tempLocation.X, 0, GFXBackground2Width[A], color_height,
                            GFXBackground2[A].ColorUpper.r, GFXBackground2[A].ColorUpper.g, GFXBackground2[A].ColorUpper.b);
                    }
                    else
                    {
                        tempLocation.Y -= 640;
                        tempLocation.Height = 640;
                        while(tempLocation.X + tempLocation.Width > -vScreenX[Z]
                            && tempLocation.X < -vScreenX[Z] + vScreen[Z].Width
                            && tempLocation.Y + tempLocation.Height > -vScreenY[Z])
                        {
                            XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], tempLocation.Height, GFXBackground2[A], 0, 0);
                            tempLocation.Y -= 640;
                        }
                    }
                }
            }
        }
    }

    A = 12; // SMW Hills
    if(Background2[S] == 11)
    {
        XRender::lazyPreLoad(GFXBackground2[A]);
        XRender::renderRect(0, 0, vScreen[Z].Width, vScreen[Z].Height,
            GFXBackground2[A].ColorUpper.r, GFXBackground2[A].ColorUpper.g, GFXBackground2[A].ColorUpper.b);

        int tempVar13 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar13; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(g_compatibility.free_level_res && (vScreen[Z].Height > GFXBackground2Height[A] || (vScreen[Z].Height == ScreenH / 2 && ScreenH > GFXBackground2Height[A])))
            {
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            }
            else if(level[S].Height - level[S].Y > GFXBackground2Height[A])
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] - ScreenH) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];

            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }

    A = 13; // SMW Trees
    if(Background2[S] == 12)
    {

        int tempVar14 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar14; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - 600) * (GFXBackground2Height[A] - 600) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];

            // repeat it with vertical flips
            unsigned int flip = X_FLIP_NONE;
            double bottom_Y = tempLocation.Y + GFXBackground2Height[A];
            while(tempLocation.X + tempLocation.Width > -vScreenX[Z]
                && tempLocation.X < -vScreenX[Z] + vScreen[Z].Width
                && tempLocation.Y + tempLocation.Height > -vScreenY[Z])
            {
                XRender::renderTextureScaleEx(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0, GFXBackground2Width[A], GFXBackground2Height[A], 0., nullptr, flip);
                flip ^= X_FLIP_VERTICAL;
                tempLocation.Y -= GFXBackground2Height[A];
            }

            flip = X_FLIP_VERTICAL;
            tempLocation.Y = bottom_Y;
            while(tempLocation.X + tempLocation.Width > -vScreenX[Z]
                && tempLocation.X < -vScreenX[Z] + vScreen[Z].Width
                && tempLocation.Y < -vScreenY[Z] + vScreen[Z].Height)
            {
                XRender::renderTextureScaleEx(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0, GFXBackground2Width[A], GFXBackground2Height[A], 0., nullptr, flip);
                flip ^= X_FLIP_VERTICAL;
                tempLocation.Y += GFXBackground2Height[A];
            }
        }
    }

    A = 14; // SMB3 Desert
    if(Background2[S] == 14)
    {
        XRender::lazyPreLoad(GFXBackground2[A]);
        XRender::renderRect(0, 0, vScreen[Z].Width, vScreen[Z].Height,
            GFXBackground2[A].ColorUpper.r, GFXBackground2[A].ColorUpper.g, GFXBackground2[A].ColorUpper.b);

        int tempVar15 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar15; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(g_compatibility.free_level_res && (vScreen[Z].Height > GFXBackground2Height[A] || (vScreen[Z].Height == ScreenH / 2 && ScreenH > GFXBackground2Height[A])))
            {
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            }
            else if(level[S].Height - level[S].Y > GFXBackground2Height[A])
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] - ScreenH) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }

    A = 15; // SMB3 Dungeon
    if(Background2[S] == 15)
    {

        int tempVar16 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar16; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - 600) * (GFXBackground2Height[A] - 600) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];

            double bottom_Y = tempLocation.Y + GFXBackground2Height[A];
            while(tempLocation.X + tempLocation.Width > -vScreenX[Z]
                && tempLocation.X < -vScreenX[Z] + vScreen[Z].Width
                && tempLocation.Y + tempLocation.Height > -vScreenY[Z])
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
                tempLocation.Y -= GFXBackground2Height[A];
            }
            tempLocation.Y = bottom_Y;
            while(tempLocation.X + tempLocation.Width > -vScreenX[Z]
                && tempLocation.X < -vScreenX[Z] + vScreen[Z].Width
                && tempLocation.Y < -vScreenY[Z] + vScreen[Z].Height)
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
                tempLocation.Y += GFXBackground2Height[A];
            }
        }
    }

    A = 16; // Crateria
    if(Background2[S] == 16)
    {

        int tempVar17 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar17; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - 600) * (GFXBackground2Height[A] - 600) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];

            double bottom_Y = tempLocation.Y + GFXBackground2Height[A];
            while(tempLocation.X + tempLocation.Width > -vScreenX[Z]
                && tempLocation.X < -vScreenX[Z] + vScreen[Z].Width
                && tempLocation.Y + tempLocation.Height > -vScreenY[Z])
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
                tempLocation.Y -= GFXBackground2Height[A];
            }
            tempLocation.Y = bottom_Y;
            while(tempLocation.X + tempLocation.Width > -vScreenX[Z]
                && tempLocation.X < -vScreenX[Z] + vScreen[Z].Width
                && tempLocation.Y < -vScreenY[Z] + vScreen[Z].Height)
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
                tempLocation.Y += GFXBackground2Height[A];
            }
        }
    }

    A = 17; // smb3 ship
    if(Background2[S] == 17)
    {
        int tempVar18 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5 + (double)ScreenW / GFXBackground2Width[A]))) + 1;

        if(g_compatibility.disable_background2_tiling)
        {
            XRender::lazyPreLoad(GFXBackground2[A]);
            XRender::renderRect(0, 0, vScreen[Z].Width, vScreen[Z].Height,
                GFXBackground2[A].ColorLower.r, GFXBackground2[A].ColorLower.g, GFXBackground2[A].ColorLower.b);
        }

        // in tall levels, must repeat the texture.
        // the upper part of the texture is 254px in the original assets
        for(B = 0; B <= tempVar18; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            tempLocation.Y = level[S].Y - 32;
            if(GameMenu && vScreen[Z].Height > level[S].Height - level[S].Y)
                tempLocation.Y = -vScreenY[Z];

            tempLocation.Height = 254;
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], 254, GFXBackground2[A], 0, 0);
            }

            tempLocation.Y += 254;
            tempLocation.Height = GFXBackground2Height[A] - 254;

            int verticalReps = static_cast<int>(floor(static_cast<double>((vScreen[Z].Height - (tempLocation.Y + vScreenY[Z])) / tempLocation.Height))) + 1;

            for(int C = 0; C <= verticalReps; C++)
            {
                if(vScreenCollision(Z, tempLocation))
                {
                    XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A] - 254, GFXBackground2[A], 0, 254);
                }
                tempLocation.Y += GFXBackground2Height[A] - 254;
                if(g_compatibility.disable_background2_tiling)
                    break;
            }
        }
    }

    A = 18; // SMW ghost house
    if(Background2[S] == 18)
    {

        int tempVar19 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar19; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A] / 4.0)
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - (600 - vScreen(Z).Top)) * (GFXBackground2Height(A) / 4 - (600 - vScreen(Z).Top))
                // .Y = -vScreenY(Z) - .Y
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - 600) * (GFXBackground2Height[A] / 4.0 - 600) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A] / 4.0;
            tempLocation.Height = GFXBackground2Height[A] / 4;
            tempLocation.Width = GFXBackground2Width[A];

            double bottom_Y = tempLocation.Y + GFXBackground2Height[A] / 4.0;
            while(tempLocation.X + tempLocation.Width > -vScreenX[Z]
                && tempLocation.X < -vScreenX[Z] + vScreen[Z].Width
                && tempLocation.Y + tempLocation.Height > -vScreenY[Z])
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A] / 4.0, GFXBackground2[A], 0, (GFXBackground2Height[A] / 4.0) * SpecialFrame[3]);
                tempLocation.Y -= GFXBackground2Height[A] / 4.0;
            }
            tempLocation.Y = bottom_Y;
            while(tempLocation.X + tempLocation.Width > -vScreenX[Z]
                && tempLocation.X < -vScreenX[Z] + vScreen[Z].Width
                && tempLocation.Y < -vScreenY[Z] + vScreen[Z].Height)
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A] / 4.0, GFXBackground2[A], 0, (GFXBackground2Height[A] / 4.0) * SpecialFrame[3]);
                tempLocation.Y += GFXBackground2Height[A] / 4.0;
            }
        }
    }

    A = 19; // smw forest
    if(Background2[S] == 19)
    {

        int tempVar20 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar20; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - 600) * (GFXBackground2Height[A] - 600) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];

            // repeat it with vertical flips
            unsigned int flip = X_FLIP_NONE;
            double bottom_Y = tempLocation.Y + GFXBackground2Height[A];
            while(tempLocation.X + tempLocation.Width > -vScreenX[Z]
                && tempLocation.X < -vScreenX[Z] + vScreen[Z].Width
                && tempLocation.Y + tempLocation.Height > -vScreenY[Z])
            {
                XRender::renderTextureScaleEx(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0, GFXBackground2Width[A], GFXBackground2Height[A], 0., nullptr, flip);
                flip ^= X_FLIP_VERTICAL;
                tempLocation.Y -= GFXBackground2Height[A];
            }

            flip = X_FLIP_VERTICAL;
            tempLocation.Y = bottom_Y;
            while(tempLocation.X + tempLocation.Width > -vScreenX[Z]
                && tempLocation.X < -vScreenX[Z] + vScreen[Z].Width
                && tempLocation.Y < -vScreenY[Z] + vScreen[Z].Height)
            {
                XRender::renderTextureScaleEx(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0, GFXBackground2Width[A], GFXBackground2Height[A], 0., nullptr, flip);
                flip ^= X_FLIP_VERTICAL;
                tempLocation.Y += GFXBackground2Height[A];
            }
        }
    }

    A = 20; // smb3 forest
    if(Background2[S] == 20)
    {
        int tempVar21 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar21; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(g_compatibility.free_level_res && (vScreen[Z].Height > GFXBackground2Height[A] || (vScreen[Z].Height == ScreenH / 2 && ScreenH > GFXBackground2Height[A])))
            {
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            }
            else if(level[S].Height - level[S].Y > GFXBackground2Height[A])
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] - ScreenH) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            
            if(tempLocation.X + tempLocation.Width > -vScreenX[Z]
                && tempLocation.X < -vScreenX[Z] + vScreen[Z].Width)
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            
                // top 500px of vanilla asset loop
                if(tempLocation.Y > -vScreenY[Z])
                {
                    if(GFXBackground2Height[A] < 500 || g_compatibility.disable_background2_tiling)
                    {
                        // color fill fallback
                        double color_height = tempLocation.Y + vScreenY[Z];
                        XRender::renderRect(vScreenX[Z] + tempLocation.X, 0, GFXBackground2Width[A], color_height,
                            GFXBackground2[A].ColorUpper.r, GFXBackground2[A].ColorUpper.g, GFXBackground2[A].ColorUpper.b);
                    }
                    else
                    {
                        tempLocation.Y -= 500;
                        tempLocation.Height = 500;
                        while(tempLocation.X + tempLocation.Width > -vScreenX[Z]
                            && tempLocation.X < -vScreenX[Z] + vScreen[Z].Width
                            && tempLocation.Y + tempLocation.Height > -vScreenY[Z])
                        {
                            XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], tempLocation.Height, GFXBackground2[A], 0, 0);
                            tempLocation.Y -= 500;
                        }
                    }
                }
            }
        }
    }

    A = 21; // smb3 battle game
    if(Background2[S] == 21)
    {

        int tempVar22 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar22; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - 600) * (GFXBackground2Height[A] - 600) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];

            double bottom_Y = tempLocation.Y + GFXBackground2Height[A];
            while(tempLocation.X + tempLocation.Width > -vScreenX[Z]
                && tempLocation.X < -vScreenX[Z] + vScreen[Z].Width
                && tempLocation.Y + tempLocation.Height > -vScreenY[Z])
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
                tempLocation.Y -= GFXBackground2Height[A];
            }
            tempLocation.Y = bottom_Y;
            while(tempLocation.X + tempLocation.Width > -vScreenX[Z]
                && tempLocation.X < -vScreenX[Z] + vScreen[Z].Width
                && tempLocation.Y < -vScreenY[Z] + vScreen[Z].Height)
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
                tempLocation.Y += GFXBackground2Height[A];
            }
        }
    }

    A = 22; // SMB3 Waterfall
    if(Background2[S] == 22)
    {

        int tempVar23 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5) + vScreen[Z].Width / GFXBackground2Width[A])) + 1;
        for(B = 0; B <= tempVar23; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            tempLocation.Y = level[S].Height - GFXBackground2Height[A] / 4.0;
            tempLocation.Height = GFXBackground2Height[A] / 4;
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A] / 4.0, GFXBackground2[A], 0, (GFXBackground2Height[A] / 4.0) * SpecialFrame[3]);
            }
        }
    }

    A = 23; // SMB3 Tank
    if(Background2[S] == 23)
    {

        XRender::lazyPreLoad(GFXBackground2[A]);
        XRender::renderRect(0, 0, vScreen[Z].Width, vScreen[Z].Height,
            GFXBackground2[A].ColorUpper.r, GFXBackground2[A].ColorUpper.g, GFXBackground2[A].ColorUpper.b);

        int tempVar24 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar24; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(g_compatibility.free_level_res && (vScreen[Z].Height > GFXBackground2Height[A] || (vScreen[Z].Height == ScreenH / 2 && ScreenH > GFXBackground2Height[A])))
            {
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            }
            else if(level[S].Height - level[S].Y > GFXBackground2Height[A])
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - 600) * (GFXBackground2Height[A] - 600) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];

            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }

    A = 24; // smb3 bowsers castle
    if(Background2[S] == 24)
    {
        // the bottom segment is 384px tall
        const int loop = 384;

        if(g_compatibility.disable_background2_tiling)
        {
            XRender::lazyPreLoad(GFXBackground2[A]);
            XRender::renderRect(0, 0, vScreen[Z].Width, vScreen[Z].Height,
                GFXBackground2[A].ColorLower.r, GFXBackground2[A].ColorLower.g, GFXBackground2[A].ColorLower.b);
        }

        int horizontalReps = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= horizontalReps; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            tempLocation.Y = level[S].Y - 32; // Height - GFXBackground2height(a)
            if(GameMenu && vScreen[Z].Height > level[S].Height - level[S].Y)
                tempLocation.Y = -vScreenY[Z];

            tempLocation.Height = GFXBackground2Height[A] - loop;
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], tempLocation.Height, GFXBackground2[A], 0, 0);
            }

            tempLocation.Y += tempLocation.Height;
            tempLocation.Height = loop;
            while(tempLocation.X + tempLocation.Width > -vScreenX[Z]
                && tempLocation.X < -vScreenX[Z] + vScreen[Z].Width
                && tempLocation.Y < -vScreenY[Z] + vScreen[Z].Height)
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], loop, GFXBackground2[A], 0, GFXBackground2Height[A] - loop);
                tempLocation.Y += loop;

                if(g_compatibility.disable_background2_tiling)
                    break;
            }
        }
    }

    A = 25; // SMB2 Underground
    if(Background2[S] == 25)
    {

        int tempVar26 = static_cast<int>(floor(static_cast<double>((level[S].Height - level[S].Y) / GFXBackground2Height[A] * 0.5 + (double)ScreenH * 2 / GFXBackground2Height[A]))) + 1;
        for(B = 0; B <= tempVar26; B++)
        {
            tempLocation.Y = level[S].Y + ((B * GFXBackground2Height[A] - B) - (vScreenY[Z] + vScreen[Z].Top + level[S].Y) * 0.5) - 32;
            if(GameMenu && level[S].Y > -vScreenY[Z])
            {
                tempLocation.Y -= level[S].Y + vScreenY[Z];
            }
            // .X = Level(S).X
            if(level[S].Width - level[S].X > GFXBackground2Width[A])
            {
                // .X = (-vScreenX(Z) - level(S).X) / (level(S).Width - level(S).X - 800) * (GFXBackground2Width(A) - 800)
                // note: fixed the left alignment to match the vanilla game
                tempLocation.X = (-vScreenX[Z] - level[S].X - vScreen[Z].Left) / (level[S].Width - level[S].X - 800) * (GFXBackground2Width[A] - 800) + vScreen[Z].Left;
                tempLocation.X = -vScreenX[Z] - tempLocation.X;
            }
            else
                tempLocation.X = level[S].Width - GFXBackground2Width[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];

            while(tempLocation.Y + tempLocation.Height > -vScreenY[Z]
                && tempLocation.Y < -vScreenY[Z] + vScreen[Z].Height
                && tempLocation.X < -vScreenX[Z] + vScreen[Z].Width)
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
                tempLocation.X += GFXBackground2Width[A];
            }
        }
    }

    A = 26; // Toad's House
    if(Background2[S] == 26)
    {
        // the top segment is 244px tall
        const int loop = GFXBackground2Height[A] - 244;

        if(g_compatibility.disable_background2_tiling)
        {
            XRender::lazyPreLoad(GFXBackground2[A]);
            XRender::renderRect(0, 0, vScreen[Z].Width, vScreen[Z].Height,
                GFXBackground2[A].ColorLower.r, GFXBackground2[A].ColorLower.g, GFXBackground2[A].ColorLower.b);
        }

        int horizontalReps = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        int verticalReps = static_cast<int>(floor(static_cast<double>((level[S].Height - level[S].Y - (GFXBackground2Height[A] - loop) + 32) / loop))) + 1;
        for(B = 0; B <= horizontalReps; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            tempLocation.Y = level[S].Y - 32; // Height - GFXBackground2height(a)
            if(GameMenu && vScreen[Z].Height > level[S].Height - level[S].Y)
                tempLocation.Y = -vScreenY[Z];

            tempLocation.Height = GFXBackground2Height[A] - loop;
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], tempLocation.Height, GFXBackground2[A], 0, 0);
            }
            tempLocation.Y += tempLocation.Height;
            tempLocation.Height = loop;
            for(int C = 0; C <= verticalReps; C++)
            {
                if(vScreenCollision(Z, tempLocation))
                    XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], loop, GFXBackground2[A], 0, GFXBackground2Height[A] - loop);
                tempLocation.Y += loop;
                if(g_compatibility.disable_background2_tiling)
                    break;
            }
        }
    }

    A = 27; // SMB3 Castle
    if(Background2[S] == 27)
    {
        // the top 1000px of the vanilla asset loops

        if(g_compatibility.disable_background2_tiling)
        {
            XRender::lazyPreLoad(GFXBackground2[A]);
            XRender::renderRect(0, 0, vScreen[Z].Width, vScreen[Z].Height,
                GFXBackground2[A].ColorUpper.r, GFXBackground2[A].ColorUpper.g, GFXBackground2[A].ColorUpper.b);
        }

        int tempVar28 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar28; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - 600) * (GFXBackground2Height[A] - 600) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];

            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];

            double bottom_Y = tempLocation.Y + 1000;
            while(tempLocation.X + tempLocation.Width > -vScreenX[Z]
                && tempLocation.X < -vScreenX[Z] + vScreen[Z].Width)
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], tempLocation.Height, GFXBackground2[A], 0, 0);
                if(tempLocation.Height < 1000 || g_compatibility.disable_background2_tiling)
                {
                    if(tempLocation.Y > -vScreenY[Z])
                    {
                        // color fill fallback
                        double color_height = tempLocation.Y + vScreenY[Z];
                        XRender::renderRect(vScreenX[Z] + tempLocation.X, 0, GFXBackground2Width[A], color_height,
                            GFXBackground2[A].ColorUpper.r, GFXBackground2[A].ColorUpper.g, GFXBackground2[A].ColorUpper.b);
                    }
                    if(tempLocation.Y + tempLocation.Height < -vScreenY[Z] + vScreen[Z].Height)
                    {
                        double shortfall = -vScreenY[Z] + vScreen[Z].Height - tempLocation.Y - tempLocation.Height;
                        // color fill
                        XRender::renderRect(vScreenX[Z] + tempLocation.X, vScreen[Z].Height - shortfall, GFXBackground2Width[A], shortfall,
                            GFXBackground2[A].ColorLower.r, GFXBackground2[A].ColorLower.g, GFXBackground2[A].ColorLower.b);
                    }
                    break;
                }
                tempLocation.Height = 1000;
                tempLocation.Y -= tempLocation.Height;
                if(tempLocation.Y < -vScreenY[Z])
                    break;
            }
            if(tempLocation.Height >= 1000 && !g_compatibility.disable_background2_tiling)
            {
                tempLocation.Y = bottom_Y;
                while(tempLocation.X + tempLocation.Width > -vScreenX[Z]
                    && tempLocation.X < -vScreenX[Z] + vScreen[Z].Width
                    && tempLocation.Y < -vScreenY[Z] + vScreen[Z].Height)
                {
                    XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], tempLocation.Height, GFXBackground2[A], 0, 0);
                    tempLocation.Y += tempLocation.Height;
                }
            }
        }
    }

    A = 28; // SMW Bonus
    if(Background2[S] == 28)
    {

        int tempVar29 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar29; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - 600) * (GFXBackground2Height[A] - 600) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];

            double bottom_Y = tempLocation.Y + GFXBackground2Height[A];
            while(tempLocation.X + tempLocation.Width > -vScreenX[Z]
                && tempLocation.X < -vScreenX[Z] + vScreen[Z].Width
                && tempLocation.Y + tempLocation.Height >= -vScreenY[Z])
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
                tempLocation.Y -= GFXBackground2Height[A];
            }
            tempLocation.Y = bottom_Y;
            while(tempLocation.X + tempLocation.Width > -vScreenX[Z]
                && tempLocation.X < -vScreenX[Z] + vScreen[Z].Width
                && tempLocation.Y < -vScreenY[Z] + vScreen[Z].Height)
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
                tempLocation.Y += GFXBackground2Height[A];
            }
        }
    }

    A = 29; // SMW Night
    if(Background2[S] == 29)
    {
        // safe to loop the top 600px of the vanilla asset

        int tempVar30 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar30; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(g_compatibility.free_level_res && (vScreen[Z].Height > GFXBackground2Height[A] / 4.0 || (vScreen[Z].Height == ScreenH / 2 && ScreenH > GFXBackground2Height[A] / 4.0)))
            {
                tempLocation.Y = level[S].Height - GFXBackground2Height[A] / 4.0;
            }
            else if(level[S].Height - level[S].Y > GFXBackground2Height[A] / 4.0)
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - (600 - vScreen(Z).Top)) * (GFXBackground2Height(A) / 4 - (600 - vScreen(Z).Top))
                // .Y = -vScreenY(Z) - .Y
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] / 4.0 - ScreenH) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A] / 4.0;
            tempLocation.Height = GFXBackground2Height[A] / 4;
            tempLocation.Width = GFXBackground2Width[A];

            while(tempLocation.X + tempLocation.Width > -vScreenX[Z]
                && tempLocation.X < -vScreenX[Z] + vScreen[Z].Width
                && tempLocation.Y + tempLocation.Height >= -vScreenY[Z])
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], tempLocation.Height, GFXBackground2[A], 0, 0);
                if(tempLocation.Height < 600 || g_compatibility.disable_background2_tiling)
                {
                    if(tempLocation.Y > -vScreenY[Z])
                    {
                        // color fill fallback
                        double color_height = tempLocation.Y + vScreenY[Z];
                        XRender::renderRect(vScreenX[Z] + tempLocation.X, 0, GFXBackground2Width[A], color_height,
                            GFXBackground2[A].ColorUpper.r, GFXBackground2[A].ColorUpper.g, GFXBackground2[A].ColorUpper.b);
                    }
                    break;
                }
                tempLocation.Height = 600;
                tempLocation.Y -= tempLocation.Height;
            }
        }
    }

    A = 30; // SMW Cave
    if(Background2[S] == 30)
    {

        int tempVar31 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar31; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A] / 4.0)
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - (600 - vScreen(Z).Top)) * (GFXBackground2Height(A) / 4 - (600 - vScreen(Z).Top))
                // .Y = -vScreenY(Z) - .Y
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - 600) * (GFXBackground2Height[A] / 4.0 - 600) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A] / 4.0;
            tempLocation.Height = GFXBackground2Height[A] / 4;
            tempLocation.Width = GFXBackground2Width[A];

            // repeat it with vertical flips
            unsigned int flip = X_FLIP_NONE;
            double bottom_Y = tempLocation.Y + tempLocation.Height;
            while(tempLocation.X + tempLocation.Width > -vScreenX[Z]
                && tempLocation.X < -vScreenX[Z] + vScreen[Z].Width
                && tempLocation.Y + tempLocation.Height >= -vScreenY[Z])
            {
                XRender::renderTextureScaleEx(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A] / 4.0, GFXBackground2[A], 0, (GFXBackground2Height[A] / 4.0) * SpecialFrame[3], tempLocation.Width, tempLocation.Height, 0., nullptr, flip);
                flip ^= X_FLIP_VERTICAL;
                tempLocation.Y -= tempLocation.Height;
            }

            tempLocation.Y = bottom_Y;
            flip = X_FLIP_VERTICAL;
            while(tempLocation.X + tempLocation.Width > -vScreenX[Z]
                && tempLocation.X < -vScreenX[Z] + vScreen[Z].Width
                && tempLocation.Y < -vScreenY[Z] + vScreen[Z].Height)
            {
                XRender::renderTextureScaleEx(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A] / 4.0, GFXBackground2[A], 0, (GFXBackground2Height[A] / 4.0) * SpecialFrame[3], tempLocation.Width, tempLocation.Height, 0., nullptr, flip);
                flip ^= X_FLIP_VERTICAL;
                tempLocation.Y += tempLocation.Height;
            }
        }
    }

    A = 31; // SMW Hills 2
    if(Background2[S] == 31)
    {
        // use a simple color fill
        XRender::lazyPreLoad(GFXBackground2[A]);
        XRender::renderRect(0, 0, vScreen[Z].Width, vScreen[Z].Height,
            GFXBackground2[A].ColorUpper.r, GFXBackground2[A].ColorUpper.g, GFXBackground2[A].ColorUpper.b);

        int tempVar32 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar32; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(g_compatibility.free_level_res && (vScreen[Z].Height > GFXBackground2Height[A] || (vScreen[Z].Height == ScreenH / 2 && ScreenH > GFXBackground2Height[A])))
            {
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            }
            else if(level[S].Height - level[S].Y > GFXBackground2Height[A])
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] - ScreenH) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];

            if(tempLocation.X + tempLocation.Width > -vScreenX[Z]
                && tempLocation.X < -vScreenX[Z] + vScreen[Z].Width)
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }

    A = 32; // SMW Clouds
    if(Background2[S] == 32)
    {
        // safe to loop the top 400px of the vanilla asset

        int tempVar33 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar33; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(g_compatibility.free_level_res && (vScreen[Z].Height > GFXBackground2Height[A] || (vScreen[Z].Height == ScreenH / 2 && ScreenH > GFXBackground2Height[A])))
            {
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            }
            else if(level[S].Height - level[S].Y > GFXBackground2Height[A])
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] - ScreenH) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];

            while(tempLocation.X + tempLocation.Width > -vScreenX[Z]
                && tempLocation.X < -vScreenX[Z] + vScreen[Z].Width
                && tempLocation.Y + tempLocation.Height > -vScreenY[Z])
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], tempLocation.Height, GFXBackground2[A], 0, 0);
                if(tempLocation.Height < 400 || g_compatibility.disable_background2_tiling)
                {
                    if(tempLocation.Y > -vScreenY[Z])
                    {
                        // color fill fallback
                        double color_height = tempLocation.Y + vScreenY[Z];
                        XRender::renderRect(vScreenX[Z] + tempLocation.X, 0, GFXBackground2Width[A], color_height,
                            GFXBackground2[A].ColorUpper.r, GFXBackground2[A].ColorUpper.g, GFXBackground2[A].ColorUpper.b);
                    }
                    break;
                }
                tempLocation.Height = 400;
                tempLocation.Y -= tempLocation.Height;
            }
        }
    }

    A = 33; // SMW Snow
    if(Background2[S] == 33)
    {
        int tempVar34 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar34; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(g_compatibility.free_level_res && (vScreen[Z].Height > GFXBackground2Height[A] || (vScreen[Z].Height == ScreenH / 2 && ScreenH > GFXBackground2Height[A])))
            {
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            }
            else if(level[S].Height - level[S].Y > GFXBackground2Height[A])
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] - ScreenH) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];

            if(tempLocation.X + tempLocation.Width > -vScreenX[Z]
                && tempLocation.X < -vScreenX[Z] + vScreen[Z].Width)
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);

                // use a simple color fill
                if(tempLocation.Y > -vScreenY[Z])
                {
                    double color_height = tempLocation.Y + vScreenY[Z];
                    XRender::renderRect(vScreenX[Z] + tempLocation.X, 0, GFXBackground2Width[A], color_height,
                        GFXBackground2[A].ColorUpper.r, GFXBackground2[A].ColorUpper.g, GFXBackground2[A].ColorUpper.b);
                }
            }
        }
    }

    A = 34; // SMW Hills 3
    if(Background2[S] == 34)
    {
        int tempVar35 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar35; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(g_compatibility.free_level_res && (vScreen[Z].Height > GFXBackground2Height[A] || (vScreen[Z].Height == ScreenH / 2 && ScreenH > GFXBackground2Height[A])))
            {
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            }
            else if(level[S].Height - level[S].Y > GFXBackground2Height[A])
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] - ScreenH) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];

            while(tempLocation.X + tempLocation.Width > -vScreenX[Z]
                && tempLocation.X < -vScreenX[Z] + vScreen[Z].Width
                && tempLocation.Y + tempLocation.Height > -vScreenY[Z])
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], tempLocation.Height, GFXBackground2[A], 0, 0);
                if(tempLocation.Height < 412 || g_compatibility.disable_background2_tiling)
                {
                    if(tempLocation.Y > -vScreenY[Z])
                    {
                        // color fill fallback
                        double color_height = tempLocation.Y + vScreenY[Z];
                        XRender::renderRect(vScreenX[Z] + tempLocation.X, 0, GFXBackground2Width[A], color_height,
                            GFXBackground2[A].ColorUpper.r, GFXBackground2[A].ColorUpper.g, GFXBackground2[A].ColorUpper.b);
                    }
                    break;
                }
                tempLocation.Height = 412;
                tempLocation.Y -= tempLocation.Height;
            }
        }
    }

    A = 35; // SMB 3 Snow Trees
    if(Background2[S] == 35)
    {

        int tempVar36 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar36; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }

    A = 36; // Snow Clouds
    if(Background2[S] == 35 || Background2[S] == 37)
    {

        int tempVar37 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.75 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar37; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.75);
            // the other background is 500px, so this really needs to be 600...
            tempLocation.Y = level[S].Height - GFXBackground2Height[A] - 600 + 100;
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];

            while(tempLocation.X + tempLocation.Width > -vScreenX[Z]
                && tempLocation.X < -vScreenX[Z] + vScreen[Z].Width
                && tempLocation.Y + tempLocation.Height > -vScreenY[Z])
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
                tempLocation.Y -= tempLocation.Height;
            }
        }
    }

    if(Background2[S] == 36)
    {

        int tempVar38 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.75 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar38; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.75);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - 600) * (GFXBackground2Height[A] - 600) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];

            double bottom_Y = tempLocation.Y + GFXBackground2Height[A];
            while(tempLocation.X + tempLocation.Width > -vScreenX[Z]
                && tempLocation.X < -vScreenX[Z] + vScreen[Z].Width
                && tempLocation.Y + tempLocation.Height > -vScreenY[Z])
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
                tempLocation.Y -= GFXBackground2Height[A];
            }
            tempLocation.Y = bottom_Y;
            while(tempLocation.X + tempLocation.Width > -vScreenX[Z]
                && tempLocation.X < -vScreenX[Z] + vScreen[Z].Width
                && tempLocation.Y < -vScreenY[Z] + vScreen[Z].Height)
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
                tempLocation.Y += GFXBackground2Height[A];
            }
        }
    }

    A = 37; // SMB 3 Snow Hills
    if(Background2[S] == 37)
    {

        int tempVar39 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar39; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }

    A = 38; // SMB3 Cave with Sky
    if(Background2[S] == 38)
    {
        XRender::lazyPreLoad(GFXBackground2[A]);
        XRender::renderRect(0, 0, vScreen[Z].Width, vScreen[Z].Height,
            GFXBackground2[A].ColorLower.r, GFXBackground2[A].ColorLower.g, GFXBackground2[A].ColorLower.b);

        // the two cave segments are each 428px tall

        int tempVar40 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar40; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            tempLocation.Y = level[S].Y - 20; // Height - GFXBackground2height(a)
            if(GameMenu && tempLocation.Y > -vScreenY[Z])
                tempLocation.Y = -vScreenY[Z];

            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            while(tempLocation.X + tempLocation.Width > -vScreenX[Z]
                && tempLocation.X < -vScreenX[Z] + vScreen[Z].Width
                && tempLocation.Y < -vScreenY[Z] + vScreen[Z].Height)
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], tempLocation.Height, GFXBackground2[A], 0, GFXBackground2Height[A] - tempLocation.Height);
                tempLocation.Y += tempLocation.Height;
                if(tempLocation.Height > 428*2)
                {
                    tempLocation.Height = 428*2;
                }
                if(g_compatibility.disable_background2_tiling)
                    break;
            }
        }
    }

    A = 39; // SMB3 Cave no Sky
    if(Background2[S] == 39)
    {

        int tempVar41 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar41; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - 600) * (GFXBackground2Height[A] - 600) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];

            double bottom_Y = tempLocation.Y + GFXBackground2Height[A];
            while(tempLocation.X + tempLocation.Width > -vScreenX[Z]
                && tempLocation.X < -vScreenX[Z] + vScreen[Z].Width
                && tempLocation.Y + tempLocation.Height > -vScreenY[Z])
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
                tempLocation.Y -= GFXBackground2Height[A];
            }
            tempLocation.Y = bottom_Y;
            while(tempLocation.X + tempLocation.Width > -vScreenX[Z]
                && tempLocation.X < -vScreenX[Z] + vScreen[Z].Width
                && tempLocation.Y < -vScreenY[Z] + vScreen[Z].Height)
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
                tempLocation.Y += GFXBackground2Height[A];
            }
        }
    }

    A = 40; // Mystic Cave Zone
    if(Background2[S] == 40)
    {
        if(g_compatibility.free_level_res && (vScreen[Z].Height > GFXBackground2Height[A] || (vScreen[Z].Height == ScreenH / 2 && ScreenH > GFXBackground2Height[A])))
        {
            tempLocation.Y = level[S].Height - GFXBackground2Height[A];
        }
        else if(level[S].Height - level[S].Y > GFXBackground2Height[A])
        {



            // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - (ScreenH - vScreen(Z).Top)) * (GFXBackground2Height(A) - (ScreenH - vScreen(Z).Top))
            // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)



            tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] - ScreenH) + vScreen[Z].Top;
            tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
        }
        else
            tempLocation.Y = level[S].Height - GFXBackground2Height[A];
        tempLocation.Height = GFXBackground2Height[A];
        tempLocation.Width = GFXBackground2Width[A];

        // ADDED A LOOP HERE, not indenting to avoid a huge diff hunk
        do {

        int tempVar42 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar42; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 953, GFXBackground2Width[A], 47, GFXBackground2[A], 0, 953);
            }
        }

        int tempVar43 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.6 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar43; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.6);
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 916, GFXBackground2Width[A], 37, GFXBackground2[A], 0, 916);
            }
        }

        int tempVar44 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.7 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar44; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.7);
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 849, GFXBackground2Width[A], 67, GFXBackground2[A], 0, 849);
            }
        }

        int tempVar45 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.8 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar45; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.8);
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 815, GFXBackground2Width[A], 34, GFXBackground2[A], 0, 815);
            }
        }

        int tempVar46 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.9 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar46; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.9);
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 709, GFXBackground2Width[A], 106, GFXBackground2[A], 0, 709);
            }
        }

        int tempVar47 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.85 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar47; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.85);
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 664, GFXBackground2Width[A], 45, GFXBackground2[A], 0, 664);
            }
        }

        int tempVar48 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.8 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar48; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.8);
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 614, GFXBackground2Width[A], 50, GFXBackground2[A], 0, 614);
            }
        }

        int tempVar49 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.75 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar49; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.75);
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 540, GFXBackground2Width[A], 74, GFXBackground2[A], 0, 540);
            }
        }

        int tempVar50 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.7 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar50; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.7);
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 408, GFXBackground2Width[A], 132, GFXBackground2[A], 0, 408);
            }
        }

        int tempVar51 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.75 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar51; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.75);
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 333, GFXBackground2Width[A], 75, GFXBackground2[A], 0, 333);
            }
        }

        int tempVar52 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.8 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar52; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.8);
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 278, GFXBackground2Width[A], 55, GFXBackground2[A], 0, 278);
            }
        }

        int tempVar53 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.85 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar53; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.85);
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 235, GFXBackground2Width[A], 43, GFXBackground2[A], 0, 235);
            }
        }

        int tempVar54 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.9 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar54; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.9);
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 123, GFXBackground2Width[A], 112, GFXBackground2[A], 0, 123);
            }
        }

        int tempVar55 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.8 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar55; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.8);
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 85, GFXBackground2Width[A], 38, GFXBackground2[A], 0, 85);
            }
        }

        int tempVar56 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.7 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar56; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.7);
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 48, GFXBackground2Width[A], 37, GFXBackground2[A], 0, 48);
            }
        }

        int tempVar57 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.6 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar57; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.6);
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], 48, GFXBackground2[A], 0, 0);
            }
        }

        tempLocation.Y -= GFXBackground2Height[A];

        } while(tempLocation.Y + GFXBackground2Height[A] > -vScreenY[Z]);
    }

    A = 41; // SMB 1 Castle
    if(Background2[S] == 41)
    {

        int tempVar58 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar58; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - 600) * (GFXBackground2Height[A] - 600) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];

            double bottom_Y = tempLocation.Y + GFXBackground2Height[A];
            while(tempLocation.X + tempLocation.Width > -vScreenX[Z]
                && tempLocation.X < -vScreenX[Z] + vScreen[Z].Width
                && tempLocation.Y + tempLocation.Height > -vScreenY[Z])
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
                tempLocation.Y -= GFXBackground2Height[A];
            }
            tempLocation.Y = bottom_Y;
            while(tempLocation.X + tempLocation.Width > -vScreenX[Z]
                && tempLocation.X < -vScreenX[Z] + vScreen[Z].Width
                && tempLocation.Y < -vScreenY[Z] + vScreen[Z].Height)
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
                tempLocation.Y += GFXBackground2Height[A];
            }
        }
    }
    A = 42; // SMW Castle
    if(Background2[S] == 42)
    {
        // safe to loop top 546px of vanilla asset

        int tempVar59 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar59; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            double frameH = vb6Round(GFXBackground2Height[A] / 4.0);

            if(g_compatibility.free_level_res && (vScreen[Z].Height > frameH || (vScreen[Z].Height == ScreenH / 2 && ScreenH > frameH)))
            {
                tempLocation.Y = level[S].Height - frameH;
            }
            else if(level[S].Height - level[S].Y > frameH)
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - (600 - vScreen(Z).Top)) * (GFXBackground2Height(A) / 4 - (600 - vScreen(Z).Top))
                // .Y = -vScreenY(Z) - .Y
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - 600) * (frameH - 600) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - frameH;

            // HACK: align non-rounded pictures (there was Redigit's original with the 3455 pixels height,
            // but it must be 3466. There are lot of custom resources that using the 3455 height by mistake)
            if(SpecialFrame[3] == 3 && (GFXBackground2Height[A] % 4 != 0))
                tempLocation.Y += 4 - (GFXBackground2Height[A] % 4); // So, align them by the same way

            tempLocation.Height = GFXBackground2Height[A] / 4;
            tempLocation.Width = GFXBackground2Width[A];

            while(tempLocation.X + tempLocation.Width > -vScreenX[Z]
                && tempLocation.X < -vScreenX[Z] + vScreen[Z].Width
                && tempLocation.Y + tempLocation.Height > -vScreenY[Z])
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], tempLocation.Height, GFXBackground2[A], 0, (frameH) * SpecialFrame[3]);
                if(tempLocation.Height < 546 || g_compatibility.disable_background2_tiling)
                {
                    if(tempLocation.Y > -vScreenY[Z])
                    {
                        // color fill fallback
                        double color_height = tempLocation.Y + vScreenY[Z];
                        XRender::renderRect(vScreenX[Z] + tempLocation.X, 0, GFXBackground2Width[A], color_height,
                            GFXBackground2[A].ColorUpper.r, GFXBackground2[A].ColorUpper.g, GFXBackground2[A].ColorUpper.b);
                    }
                    break;
                }
                tempLocation.Height = 546;
                tempLocation.Y -= tempLocation.Height;
            }
        }
    }
    A = 43; // SMW Castle 2
    if(Background2[S] == 43)
    {

        int tempVar60 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar60; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - 600) * (GFXBackground2Height[A] - 600) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];

            double bottom_Y = tempLocation.Y + GFXBackground2Height[A];
            while(tempLocation.X + tempLocation.Width > -vScreenX[Z]
                && tempLocation.X < -vScreenX[Z] + vScreen[Z].Width
                && tempLocation.Y + tempLocation.Height > -vScreenY[Z])
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
                tempLocation.Y -= GFXBackground2Height[A];
            }
            tempLocation.Y = bottom_Y;
            while(tempLocation.X + tempLocation.Width > -vScreenX[Z]
                && tempLocation.X < -vScreenX[Z] + vScreen[Z].Width
                && tempLocation.Y < -vScreenY[Z] + vScreen[Z].Height)
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
                tempLocation.Y += GFXBackground2Height[A];
            }
        }
    }
    A = 44; // SMB2 Castle
    if(Background2[S] == 44)
    {

        int tempVar61 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar61; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
            {
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - 600) * (GFXBackground2Height[A] - 600) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];

            double bottom_Y = tempLocation.Y + GFXBackground2Height[A];
            while(tempLocation.X + tempLocation.Width > -vScreenX[Z]
                && tempLocation.X < -vScreenX[Z] + vScreen[Z].Width
                && tempLocation.Y + tempLocation.Height > -vScreenY[Z])
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
                tempLocation.Y -= GFXBackground2Height[A];
            }
            tempLocation.Y = bottom_Y;
            while(tempLocation.X + tempLocation.Width > -vScreenX[Z]
                && tempLocation.X < -vScreenX[Z] + vScreen[Z].Width
                && tempLocation.Y < -vScreenY[Z] + vScreen[Z].Height)
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
                tempLocation.Y += GFXBackground2Height[A];
            }
        }
    }
    A = 45; // Brinstar
    if(Background2[S] == 45)
    {

        int tempVar62 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar62; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - 600) * (GFXBackground2Height[A] - 600) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];

            double bottom_Y = tempLocation.Y + GFXBackground2Height[A];
            while(tempLocation.X + tempLocation.Width > -vScreenX[Z]
                && tempLocation.X < -vScreenX[Z] + vScreen[Z].Width
                && tempLocation.Y + tempLocation.Height > -vScreenY[Z])
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
                tempLocation.Y -= GFXBackground2Height[A];
            }
            tempLocation.Y = bottom_Y;
            while(tempLocation.X + tempLocation.Width > -vScreenX[Z]
                && tempLocation.X < -vScreenX[Z] + vScreen[Z].Width
                && tempLocation.Y < -vScreenY[Z] + vScreen[Z].Height)
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
                tempLocation.Y += GFXBackground2Height[A];
            }
        }
    }
    A = 46; // Transport
    if(Background2[S] == 46)
    {

        int tempVar63 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar63; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - 600) * (GFXBackground2Height[A] - 600) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];

            double bottom_Y = tempLocation.Y + GFXBackground2Height[A];
            while(tempLocation.X + tempLocation.Width > -vScreenX[Z]
                && tempLocation.X < -vScreenX[Z] + vScreen[Z].Width
                && tempLocation.Y + tempLocation.Height > -vScreenY[Z])
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
                tempLocation.Y -= GFXBackground2Height[A];
            }
            tempLocation.Y = bottom_Y;
            while(tempLocation.X + tempLocation.Width > -vScreenX[Z]
                && tempLocation.X < -vScreenX[Z] + vScreen[Z].Width
                && tempLocation.Y < -vScreenY[Z] + vScreen[Z].Height)
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
                tempLocation.Y += GFXBackground2Height[A];
            }
        }
    }
    A = 47; // Transport
    if(Background2[S] == 47)
    {

        int tempVar64 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar64; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - 600) * (GFXBackground2Height[A] - 600) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];

            double bottom_Y = tempLocation.Y + GFXBackground2Height[A];
            while(tempLocation.X + tempLocation.Width > -vScreenX[Z]
                && tempLocation.X < -vScreenX[Z] + vScreen[Z].Width
                && tempLocation.Y + tempLocation.Height > -vScreenY[Z])
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
                tempLocation.Y -= GFXBackground2Height[A];
            }
            tempLocation.Y = bottom_Y;
            while(tempLocation.X + tempLocation.Width > -vScreenX[Z]
                && tempLocation.X < -vScreenX[Z] + vScreen[Z].Width
                && tempLocation.Y < -vScreenY[Z] + vScreen[Z].Height)
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
                tempLocation.Y += GFXBackground2Height[A];
            }
        }
    }
    A = 48; // SMB2 Blouds
    if(Background2[S] == 48)
    {

        int tempVar65 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar65; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(g_compatibility.free_level_res && (vScreen[Z].Height > GFXBackground2Height[A] || (vScreen[Z].Height == ScreenH / 2 && ScreenH > GFXBackground2Height[A])))
            {
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            }
            else if(level[S].Height - level[S].Y > GFXBackground2Height[A])
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] - ScreenH) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];

            if(tempLocation.X + tempLocation.Width > -vScreenX[Z]
                && tempLocation.X < -vScreenX[Z] + vScreen[Z].Width
                && tempLocation.Y + tempLocation.Height > -vScreenY[Z])
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);

                // use a simple color fill for the sky beyond the texture
                if(tempLocation.Y > -vScreenY[Z])
                {
                    double color_height = tempLocation.Y + vScreenY[Z];
                    XRender::renderRect(vScreenX[Z] + tempLocation.X, 0, GFXBackground2Width[A], color_height,
                        GFXBackground2[A].ColorUpper.r, GFXBackground2[A].ColorUpper.g, GFXBackground2[A].ColorUpper.b);
                }
            }
        }
    }
    A = 49; // Desert Night
    if(Background2[S] == 49)
    {
        // use a simple color fill for the sky beyond the texture
        XRender::lazyPreLoad(GFXBackground2[A]);
        XRender::renderRect(0, 0, vScreen[Z].Width, vScreen[Z].Height,
            GFXBackground2[A].ColorUpper.r, GFXBackground2[A].ColorUpper.g, GFXBackground2[A].ColorUpper.b);

        if(g_compatibility.free_level_res && (vScreen[Z].Height > GFXBackground2Height[A] || (vScreen[Z].Height == ScreenH / 2 && ScreenH > GFXBackground2Height[A])))
        {
            tempLocation.Y = level[S].Height - GFXBackground2Height[A];
        }
        else if(level[S].Height - level[S].Y > GFXBackground2Height[A])
        {
            // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
            tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] - ScreenH) + vScreen[Z].Top;
            tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
        }
        else
            tempLocation.Y = level[S].Height - GFXBackground2Height[A];
        tempLocation.Height = GFXBackground2Height[A];
        tempLocation.Width = GFXBackground2Width[A];

        int tempVar66 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar66; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 280, GFXBackground2Width[A], 450, GFXBackground2[A], 0, 280);
            }
        }

        int tempVar67 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.9 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar67; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.9);
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 268, GFXBackground2Width[A], 12, GFXBackground2[A], 0, 268);
            }
        }

        int tempVar68 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.89 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar68; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.89);
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 244, GFXBackground2Width[A], 24, GFXBackground2[A], 0, 244);
            }
        }

        int tempVar69 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.88 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar69; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.88);
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 228, GFXBackground2Width[A], 16, GFXBackground2[A], 0, 228);
            }
        }

        int tempVar70 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.87 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar70; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.87);
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 196, GFXBackground2Width[A], 32, GFXBackground2[A], 0, 196);
            }
        }

        int tempVar71 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.86 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar71; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.86);
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 164, GFXBackground2Width[A], 32, GFXBackground2[A], 0, 164);
            }
        }

        int tempVar72 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.85 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar72; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.85);
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 116, GFXBackground2Width[A], 48, GFXBackground2[A], 0, 116);
            }
        }

        int tempVar73 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.84 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar73; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.84);
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 58, GFXBackground2Width[A], 58, GFXBackground2[A], 0, 58);
            }
        }

        int tempVar74 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.83 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar74; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.83);
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], 58, GFXBackground2[A], 0, 0);
            }
        }
    }
    A = 50; // Shrooms
    if(Background2[S] == 50)
    {
        if(g_compatibility.free_level_res && (vScreen[Z].Height > GFXBackground2Height[A] || (vScreen[Z].Height == ScreenH / 2 && ScreenH > GFXBackground2Height[A])))
        {
            tempLocation.Y = level[S].Height - GFXBackground2Height[A];
        }
        else if(level[S].Height - level[S].Y > GFXBackground2Height[A])
        {
            // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
            tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] - ScreenH) + vScreen[Z].Top;
            tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
        }
        else
            tempLocation.Y = level[S].Height - GFXBackground2Height[A];
        tempLocation.Height = GFXBackground2Height[A];
        tempLocation.Width = GFXBackground2Width[A];

        int tempVar75 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar75; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 378, GFXBackground2Width[A], 378, GFXBackground2[A], 0, 378);
            }
        }

        while(tempLocation.Y + 378 > -vScreenY[Z])
        {
            int tempVar76 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.65 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
            for(B = 0; B <= tempVar76; B++)
            {
                tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.65);
                if(vScreenCollision(Z, tempLocation))
                {
                    XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], 220, GFXBackground2[A], 0, 0);
                }
            }

            int tempVar77 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.6 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
            for(B = 0; B <= tempVar77; B++)
            {
                tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.6);
                if(vScreenCollision(Z, tempLocation))
                {
                    XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 220, GFXBackground2Width[A], 158, GFXBackground2[A], 0, 220);
                }
            }

            tempLocation.Y -= 378;
        }
    }

    A = 51; // SMB1 Desert
    if(Background2[S] == 51)
    {
        if(g_compatibility.free_level_res && (vScreen[Z].Height > GFXBackground2Height[A] || (vScreen[Z].Height == ScreenH / 2 && ScreenH > GFXBackground2Height[A])))
        {
            tempLocation.Y = level[S].Height - GFXBackground2Height[A];
        }
        else if(level[S].Height - level[S].Y > GFXBackground2Height[A])
        {
            // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
            if(GFXBackground2Height[A] - ScreenH > 0)
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] - ScreenH) + vScreen[Z].Top;
            else
                tempLocation.Y = (GFXBackground2Height[A] - vScreen[Z].Height) + vScreen[Z].Top;
            tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
        }
        else
            tempLocation.Y = level[S].Height - GFXBackground2Height[A];
        tempLocation.Height = GFXBackground2Height[A];
        tempLocation.Width = GFXBackground2Width[A];

        // use a simple color fill for the sky beyond the texture
        XRender::lazyPreLoad(GFXBackground2[A]);
        XRender::renderRect(0, 0, vScreen[Z].Width, vScreen[Z].Height, GFXBackground2[A].ColorUpper.r, GFXBackground2[A].ColorUpper.g, GFXBackground2[A].ColorUpper.b, 1.f, true);

        int tempVar78 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.75 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar78; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.75);
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], 350, GFXBackground2[A], 0, 0);
            }
        }

        int tempVar79 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar79; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 350, GFXBackground2Width[A], GFXBackground2Height[A] - 350, GFXBackground2[A], 0, 350);
            }
        }
    }
    A = 52; // SMB2 Desert Night
    if(Background2[S] == 52)
    {
        if(g_compatibility.free_level_res && (vScreen[Z].Height > GFXBackground2Height[A] || (vScreen[Z].Height == ScreenH / 2 && ScreenH > GFXBackground2Height[A])))
        {
            tempLocation.Y = level[S].Height - GFXBackground2Height[A];
        }
        else if(level[S].Height - level[S].Y > GFXBackground2Height[A])
        {
            // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
            if (GFXBackground2Height[A] - ScreenH > 0)
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] - ScreenH) + vScreen[Z].Top;
            else
                tempLocation.Y = (GFXBackground2Height[A] - vScreen[Z].Height) + vScreen[Z].Top;
            tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
        }
        else
            tempLocation.Y = level[S].Height - GFXBackground2Height[A];
        tempLocation.Height = GFXBackground2Height[A];
        tempLocation.Width = GFXBackground2Width[A];

        // use a simple color fill for the sky beyond the texture
        XRender::lazyPreLoad(GFXBackground2[A]);
        XRender::renderRect(0, 0, vScreen[Z].Width, vScreen[Z].Height, GFXBackground2[A].ColorUpper.r, GFXBackground2[A].ColorUpper.g, GFXBackground2[A].ColorUpper.b, 1.f, true);

        int tempVar80 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar80; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 280, GFXBackground2Width[A], GFXBackground2Height[A] - 280, GFXBackground2[A], 0, 280);
            }
        }

        int tempVar81 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.9 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar81; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.9);
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 268, GFXBackground2Width[A], 12, GFXBackground2[A], 0, 268);
            }
        }

        int tempVar82 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.89 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar82; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.89);
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 244, GFXBackground2Width[A], 24, GFXBackground2[A], 0, 244);
            }
        }

        int tempVar83 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.88 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar83; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.88);
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 228, GFXBackground2Width[A], 16, GFXBackground2[A], 0, 228);
            }
        }

        int tempVar84 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.87 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar84; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.87);
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 196, GFXBackground2Width[A], 32, GFXBackground2[A], 0, 196);
            }
        }

        int tempVar85 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.86 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar85; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.86);
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 164, GFXBackground2Width[A], 32, GFXBackground2[A], 0, 164);
            }
        }

        int tempVar86 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.85 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar86; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.85);
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 116, GFXBackground2Width[A], 48, GFXBackground2[A], 0, 116);
            }
        }

        int tempVar87 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.84 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar87; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.84);
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 58, GFXBackground2Width[A], 58, GFXBackground2[A], 0, 58);
            }
        }

        int tempVar88 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.83 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar88; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.83);
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], 58, GFXBackground2[A], 0, 0);
            }
        }
    }
    A = 53; // Cliffs
    if(Background2[S] == 53)
    {

        // use a simple color fill for the sky beyond the texture
        XRender::lazyPreLoad(GFXBackground2[A]);
        XRender::renderRect(0, 0, vScreen[Z].Width, vScreen[Z].Height, GFXBackground2[A].ColorUpper.r, GFXBackground2[A].ColorUpper.g, GFXBackground2[A].ColorUpper.b, 1.f, true);

        int tempVar89 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar89; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(g_compatibility.free_level_res && (vScreen[Z].Height > GFXBackground2Height[A] || (vScreen[Z].Height == ScreenH / 2 && ScreenH > GFXBackground2Height[A])))
            {
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            }
            else if(level[S].Height - level[S].Y > GFXBackground2Height[A])
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] - ScreenH) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }
    A = 54; // Warehouse
    if(Background2[S] == 54)
    {

        int tempVar90 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar90; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - 600) * (890 - 600) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];

            double bottom_Y = tempLocation.Y + GFXBackground2Height[A];
            while(tempLocation.X + tempLocation.Width > -vScreenX[Z]
                && tempLocation.X < -vScreenX[Z] + vScreen[Z].Width
                && tempLocation.Y + tempLocation.Height > -vScreenY[Z])
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
                tempLocation.Y -= GFXBackground2Height[A];
            }
            tempLocation.Y = bottom_Y;
            while(tempLocation.X + tempLocation.Width > -vScreenX[Z]
                && tempLocation.X < -vScreenX[Z] + vScreen[Z].Width
                && tempLocation.Y < -vScreenY[Z] + vScreen[Z].Height)
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
                tempLocation.Y += GFXBackground2Height[A];
            }
        }
    }

    A = 55; // SMW Water
    if(Background2[S] == 55)
    {

        int tempVar91 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar91; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A] / 4.0)
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) / 4 - 600)
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - 600) * (GFXBackground2Height[A] / 4.0 - 600) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A] / 4.0;
            tempLocation.Height = GFXBackground2Height[A] / 4;
            tempLocation.Width = GFXBackground2Width[A];

            double bottom_Y = tempLocation.Y + GFXBackground2Height[A] / 4.;
            while(tempLocation.X + tempLocation.Width > -vScreenX[Z]
                && tempLocation.X < -vScreenX[Z] + vScreen[Z].Width
                && tempLocation.Y + tempLocation.Height > -vScreenY[Z])
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A] / 4.0, GFXBackground2[A], 0, (GFXBackground2Height[A] / 4.0) * SpecialFrame[3]);
                tempLocation.Y -= GFXBackground2Height[A] / 4.;
            }
            tempLocation.Y = bottom_Y;
            while(tempLocation.X + tempLocation.Width > -vScreenX[Z]
                && tempLocation.X < -vScreenX[Z] + vScreen[Z].Width
                && tempLocation.Y < -vScreenY[Z] + vScreen[Z].Height)
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A] / 4.0, GFXBackground2[A], 0, (GFXBackground2Height[A] / 4.0) * SpecialFrame[3]);
                tempLocation.Y += GFXBackground2Height[A] / 4.;
            }
        }
    }

    A = 56; // SMB3 Water
    if(Background2[S] == 56)
    {
        if(g_compatibility.free_level_res && (vScreen[Z].Height > GFXBackground2Height[A] || (vScreen[Z].Height == ScreenH / 2 && ScreenH > GFXBackground2Height[A])))
        {
            tempLocation.Y = level[S].Height - GFXBackground2Height[A];
        }
        else if(level[S].Height - level[S].Y > GFXBackground2Height[A])
        {
            // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
            tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] - ScreenH) + vScreen[Z].Top;
            tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
        }
        else
            tempLocation.Y = level[S].Height - GFXBackground2Height[A];
        tempLocation.Height = GFXBackground2Height[A];
        tempLocation.Width = GFXBackground2Width[A];

        // use a simple color fill for the water beyond the texture
        XRender::lazyPreLoad(GFXBackground2[A]);
        XRender::renderRect(0, 0, vScreen[Z].Width, vScreen[Z].Height, GFXBackground2[A].ColorUpper.r, GFXBackground2[A].ColorUpper.g, GFXBackground2[A].ColorUpper.b, 1.f, true);

        int tempVar92 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.65 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar92; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.65);
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], 100, GFXBackground2[A], 0, 0);
            }
        }


        int tempVar93 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.6 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar93; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.6);
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 100, GFXBackground2Width[A], 245, GFXBackground2[A], 0, 100);
            }
        }


        int tempVar94 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.55 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar94; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.55);
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 345, GFXBackground2Width[A], 110, GFXBackground2[A], 0, 345);
            }
        }


        int tempVar95 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar95; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(vScreenCollision(Z, tempLocation))
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 455, GFXBackground2Width[A], GFXBackground2Height[A] - 455, GFXBackground2[A], 0, 455);
            }
        }
    }

    A = 57; // Warehouse (doesn't look like one, more like a temple)
    if(Background2[S] == 57)
    {
        // can loop everything but bottom 128px (top 672px by default)
        // TODO: add compat flag

        int tempVar96 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar96; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(g_compatibility.free_level_res && (vScreen[Z].Height > GFXBackground2Height[A] || (vScreen[Z].Height == ScreenH / 2 && ScreenH > GFXBackground2Height[A])))
            {
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            }
            else if(level[S].Height - level[S].Y > GFXBackground2Height[A])
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] - ScreenH) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];

            while(tempLocation.X + tempLocation.Width > -vScreenX[Z]
                && tempLocation.X < -vScreenX[Z] + vScreen[Z].Width
                && tempLocation.Y + tempLocation.Height > -vScreenY[Z])
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X,
                                      vScreenY[Z] + tempLocation.Y,
                                      GFXBackground2Width[A],
                                      tempLocation.Height,
                                      GFXBackground2[A],
                                      0, 0);

                if(GFXBackground2Height[A] < 129)
                {
                    if(tempLocation.Y > -vScreenY[Z])
                    {
                        // color fill fallback
                        double color_height = tempLocation.Y + vScreenY[Z];
                        XRender::renderRect(vScreenX[Z] + tempLocation.X, 0, GFXBackground2Width[A], color_height,
                            GFXBackground2[A].ColorUpper.r, GFXBackground2[A].ColorUpper.g, GFXBackground2[A].ColorUpper.b);
                    }
                    break;
                }
                tempLocation.Height = GFXBackground2Height[A] - 128;
                tempLocation.Y -= tempLocation.Height;
            }
        }
    }

    A = 58; // SMW Night
    if(Background2[S] == 58)
    {
        // can loop top 600px
        // TODO: add compat flag

        int tempVar97 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5 + (double)ScreenW / GFXBackground2Width[A]))) + 1;
        for(B = 0; B <= tempVar97; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(g_compatibility.free_level_res && (vScreen[Z].Height > GFXBackground2Height[A] / 4. || (vScreen[Z].Height == ScreenH / 2 && ScreenH > GFXBackground2Height[A] / 4.)))
            {
                tempLocation.Y = level[S].Height - GFXBackground2Height[A] / 4.;
            }
            else if(level[S].Height - level[S].Y > GFXBackground2Height[A] / 4.0)
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - (600 - vScreen(Z).Top)) * (GFXBackground2Height(A) / 4 - (600 - vScreen(Z).Top))
                // .Y = -vScreenY(Z) - .Y
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] / 4.0 - ScreenH) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A] / 4.0;
            tempLocation.Height = GFXBackground2Height[A] / 4;
            tempLocation.Width = GFXBackground2Width[A];

            while(tempLocation.X + tempLocation.Width > -vScreenX[Z]
                && tempLocation.X < -vScreenX[Z] + vScreen[Z].Width
                && tempLocation.Y + tempLocation.Height > -vScreenY[Z])
            {
                XRender::renderTexture(vScreenX[Z] + tempLocation.X,
                                      vScreenY[Z] + tempLocation.Y,
                                      GFXBackground2Width[A],
                                      tempLocation.Height,
                                      GFXBackground2[A],
                                      0,
                                      (GFXBackground2Height[A] / 4.0) * SpecialFrame[6]
                );

                if(tempLocation.Height < 600)
                {
                    if(tempLocation.Y > -vScreenY[Z])
                    {
                        // color fill fallback
                        double color_height = tempLocation.Y + vScreenY[Z];
                        XRender::renderRect(vScreenX[Z] + tempLocation.X, 0, GFXBackground2Width[A], color_height,
                            GFXBackground2[A].ColorUpper.r, GFXBackground2[A].ColorUpper.g, GFXBackground2[A].ColorUpper.b);
                    }
                    break;
                }
                tempLocation.Height = 600;
                tempLocation.Y -= tempLocation.Height;
            }
        }
    }

    level[S] = tempLevel;

}
