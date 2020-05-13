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

#include "../globals.h"
#include "../graphics.h"
#include "../collision.h"

#include <IniProcessor/ini_processing.h>

void DrawBackground(int S, int Z)
{
    int A = 0;
    int B = 0;
    Location_t tempLocation;
    Location_t tempLevel;
    tempLevel = level[S];
    if(LevelEditor == true)
        LevelREAL[S] = level[S];
    level[S] = LevelREAL[S];

    A = 1; // Blocks
    if(Background2[S] == 1)
    {
        int tempVar = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = -(BGOffset+1)*tempVar; B <= (BGOffset+1)*tempVar; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.5);
            tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }

    A = 2; // Clouds
    if(Background2[S] == 1 || Background2[S] == 2 || Background2[S] == 22 || Background2[S] == 60)
    {
        int tempVar2 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.75))) + 1;
        for(B = -(BGOffset+1)*tempVar2; B <= (BGOffset+1)*tempVar2; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.75);
            tempLocation.Y = level[S].Height - GFXBackground2Height[A] - ScreenH + 100;
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }

    if(Background2[S] == 13)
    {

        int tempVar3 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.75))) + 1;
        for(B = -(BGOffset+1)*tempVar3; B <= (BGOffset+1)*tempVar3; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.75);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
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
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }

    A = 3; // Hills
    if(Background2[S] == 2)
    {

        int tempVar4 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = -(BGOffset+1)*tempVar4; B <= (BGOffset+1)*tempVar4; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.5);
            tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }

    A = 4; // Castle
    if(Background2[S] == 3)
    {

        int tempVar5 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 2;
        for(B = -(BGOffset+1)*tempVar5; B <= (BGOffset+1)*tempVar5; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
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
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }

    A = 5; // Pipes
    if(Background2[S] == 4)
    {

        int tempVar6 = static_cast<int>(floor(static_cast<double>((level[S].Height - level[S].Y) / GFXBackground2Height[A] * 0.5))) + 1;
        for(B = -(BGOffset+1)*tempVar6; B <= (BGOffset+1)*tempVar6; B++)
        {
            tempLocation.Y = level[S].Y + ((B * GFXBackground2Height[A] - B) - (vScreenY[Z] + vScreen[Z].Top + level[S].Y + BGOffset * (level[S].Width - level[S].X)) * 0.5) - 32;
            if(level[S].Width - level[S].X > GFXBackground2Width[A])
            {
                tempLocation.X = (-vScreenX[Z] - level[S].X) / (level[S].Width - level[S].X - (800 - vScreen[Z].Left)) * (GFXBackground2Width[A] - (800 - vScreen[Z].Left));
                tempLocation.X = -vScreenX[Z] - tempLocation.X;
            }
            else
                tempLocation.X = level[S].Width - GFXBackground2Width[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }

    A = 6; // Trees
    if(Background2[S] == 5)
    {

        int tempVar7 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = -(BGOffset+1)*tempVar7; B <= (BGOffset+1)*tempVar7; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
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
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }

    A = 7; // Bonus
    if(Background2[S] == 6)
    {

        int tempVar8 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = -(BGOffset+1)*tempVar8; B <= (BGOffset+1)*tempVar8; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
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
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }

    A = 8; // SMB Underground
    if(Background2[S] == 7)
    {

        int tempVar9 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = -(BGOffset+1)*tempVar9; B <= (BGOffset+1)*tempVar9; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.5);
            tempLocation.Y = level[S].Y - 32;
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }

    A = 9; // Night
    if(Background2[S] == 8 || Background2[S] == 9)
    {

        int tempVar10 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.75))) + 1;
        for(B = -(BGOffset+1)*tempVar10; B <= (BGOffset+1)*tempVar10; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.75);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
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
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }

    A = 10; // Night 2
    if(Background2[S] == 9)
    {

        int tempVar11 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = -(BGOffset+1)*tempVar11; B <= (BGOffset+1)*tempVar11; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.5);
            tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }

    A = 11; // Overworld
    if(Background2[S] == 10)
    {

        int tempVar12 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = -(BGOffset+1)*tempVar12; B <= (BGOffset+1)*tempVar12; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
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
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }

    A = 12; // SMW Hills
    if(Background2[S] == 11)
    {

        int tempVar13 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = -(BGOffset+1)*tempVar13; B <= (BGOffset+1)*tempVar13; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
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
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }

    A = 13; // SMW Trees
    if(Background2[S] == 12)
    {

        int tempVar14 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = -(BGOffset+1)*tempVar14; B <= (BGOffset+1)*tempVar14; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
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
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }

    A = 14; // SMB3 Desert
    if(Background2[S] == 14)
    {

        int tempVar15 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = -(BGOffset+1)*tempVar15; B <= (BGOffset+1)*tempVar15; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
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
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }

    A = 15; // SMB3 Dungeon
    if(Background2[S] == 15)
    {

        int tempVar16 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = -(BGOffset+1)*tempVar16; B <= (BGOffset+1)*tempVar16; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
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
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }

    A = 16; // Crateria
    if(Background2[S] == 16)
    {

        int tempVar17 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = -(BGOffset+1)*tempVar17; B <= (BGOffset+1)*tempVar17; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
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
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }

    A = 17; // smb3 ship
    if(Background2[S] == 17)
    {

        int tempVar18 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = -(BGOffset+1)*tempVar18; B <= (BGOffset+1)*tempVar18; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.5);
            tempLocation.Y = level[S].Y - 32;
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }

    A = 18; // SMW ghost house
    if(Background2[S] == 18)
    {

        int tempVar19 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = -(BGOffset+1)*tempVar19; B <= (BGOffset+1)*tempVar19; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A] / 4.0)
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
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A] / 4.0, GFXBackground2[A], 0, (GFXBackground2Height[A] / 4.0) * SpecialFrame[3]);
            }
        }
    }

    A = 19; // smw forest
    if(Background2[S] == 19)
    {

        int tempVar20 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = -(BGOffset+1)*tempVar20; B <= (BGOffset+1)*tempVar20; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
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
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }

    A = 20; // smb3 forest
    if(Background2[S] == 20)
    {

        int tempVar21 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = -(BGOffset+1)*tempVar21; B <= (BGOffset+1)*tempVar21; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
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
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }

    A = 21; // smb3 battle game
    if(Background2[S] == 21)
    {

        int tempVar22 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = -(BGOffset+1)*tempVar22; B <= (BGOffset+1)*tempVar22; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
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
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }

    A = 22; // SMB3 Waterfall
    if(Background2[S] == 22)
    {

        int tempVar23 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = -(BGOffset+1)*tempVar23; B <= (BGOffset+1)*tempVar23; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.5);
            tempLocation.Y = level[S].Height - GFXBackground2Height[A] / 4.0;
            tempLocation.Height = GFXBackground2Height[A] / 4;
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A] / 4.0, GFXBackground2[A], 0, (GFXBackground2Height[A] / 4.0) * SpecialFrame[3]);
            }
        }
    }

    A = 23; // SMB3 Tank
    if(Background2[S] == 23)
    {

        int tempVar24 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = -(BGOffset+1)*tempVar24; B <= (BGOffset+1)*tempVar24; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
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
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }

    A = 24; // smb3 bowsers castle
    if(Background2[S] == 24)
    {

        int tempVar25 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = -(BGOffset+1)*tempVar25; B <= (BGOffset+1)*tempVar25; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.5);
            tempLocation.Y = level[S].Y - 32; // Height - GFXBackground2height(a)
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }

    A = 25; // SMB2 Underground
    if(Background2[S] == 25)
    {

        int tempVar26 = static_cast<int>(floor(static_cast<double>((level[S].Height - level[S].Y) / GFXBackground2Height[A] * 0.5))) + 1;
        for(B = -(BGOffset+1)*tempVar26; B <= (BGOffset+1)*tempVar26; B++)
        {
            tempLocation.Y = level[S].Y + ((B * GFXBackground2Height[A] - B) - (vScreenY[Z] + vScreen[Z].Top + level[S].Y) * 0.5) - 32;
            // .X = Level(S).X
            if(level[S].Width - level[S].X > GFXBackground2Width[A])
            {
                // .X = (-vScreenX(Z) - level(S).X) / (level(S).Width - level(S).X - 800) * (GFXBackground2Width(A) - 800)
                tempLocation.X = (-vScreenX[Z] - level[S].X) / (level[S].Width - level[S].X - (800 - vScreen[Z].Left)) * (GFXBackground2Width[A] - (800 - vScreen[Z].Left));
                tempLocation.X = -vScreenX[Z] - tempLocation.X;
            }
            else
                tempLocation.X = level[S].Width - GFXBackground2Width[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }

    A = 26; // Toad's House
    if(Background2[S] == 26)
    {
        int tempVar27 = int(std::ceil((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5)) + 1;
        for(B = -(BGOffset+1)*tempVar27; B <= (BGOffset+1)*tempVar27; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.5);
            tempLocation.Y = level[S].Y - 32; // Height - GFXBackground2height(a)
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X,
                                      vScreenY[Z] + tempLocation.Y,
                                      GFXBackground2Width[A],
                                      GFXBackground2Height[A],
                                      GFXBackground2[A],
                                      0, 0);
            }
        }
    }

    A = 27; // SMB3 Castle
    if(Background2[S] == 27)
    {

        int tempVar28 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = -(BGOffset+1)*tempVar28; B <= (BGOffset+1)*tempVar28; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
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
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }

    A = 28; // SMW Bonus
    if(Background2[S] == 28)
    {

        int tempVar29 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 3;
        for(B = -(BGOffset+1)*tempVar29; B <= (BGOffset+1)*tempVar29; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
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
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }

    A = 29; // SMW Night
    if(Background2[S] == 29)
    {

        int tempVar30 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = -(BGOffset+1)*tempVar30; B <= (BGOffset+1)*tempVar30; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A] / 4.0)
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
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A] / 4.0, GFXBackground2[A], 0, (GFXBackground2Height[A] / 4.0) * SpecialFrame[6]);
            }
        }
    }

    A = 30; // SMW Cave
    if(Background2[S] == 30)
    {

        int tempVar31 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = -(BGOffset+1)*tempVar31; B <= (BGOffset+1)*tempVar31; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A] / 4.0)
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

            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A] / 4.0, GFXBackground2[A], 0, (GFXBackground2Height[A] / 4.0) * SpecialFrame[3]);
            }


        }
    }

    A = 31; // SMW Hills 2
    if(Background2[S] == 31)
    {

        int tempVar32 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = -(BGOffset+1)*tempVar32; B <= (BGOffset+1)*tempVar32; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
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
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }

    A = 32; // SMW Clouds
    if(Background2[S] == 32)
    {

        int tempVar33 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = -(BGOffset+1)*tempVar33; B <= (BGOffset+1)*tempVar33; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
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
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }

    A = 33; // SMW Snow
    if(Background2[S] == 33)
    {

        int tempVar34 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = -(BGOffset+1)*tempVar34; B <= (BGOffset+1)*tempVar34; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
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
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }

    A = 34; // SMW Hills 3
    if(Background2[S] == 34)
    {

        int tempVar35 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = -(BGOffset+1)*tempVar35; B <= (BGOffset+1)*tempVar35; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
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
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }

    A = 35; // SMB 3 Snow Trees
    if(Background2[S] == 35)
    {

        int tempVar36 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = -(BGOffset+1)*tempVar36; B <= (BGOffset+1)*tempVar36; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.5);
            tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }

    A = 36; // Snow Clouds
    if(Background2[S] == 35 || Background2[S] == 37)
    {

        int tempVar37 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.75))) + 1;
        for(B = -(BGOffset+1)*tempVar37; B <= (BGOffset+1)*tempVar37; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.75);
            tempLocation.Y = level[S].Height - GFXBackground2Height[A] - ScreenH + 100;
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }

    if(Background2[S] == 36)
    {

        int tempVar38 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.75))) + 1;
        for(B = -(BGOffset+1)*tempVar38; B <= (BGOffset+1)*tempVar38; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.75);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
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
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }

    A = 37; // SMB 3 Snow Hills
    if(Background2[S] == 37)
    {

        int tempVar39 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = -(BGOffset+1)*tempVar39; B <= (BGOffset+1)*tempVar39; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.5);
            tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }

    A = 38; // SMB3 Cave with Sky
    if(Background2[S] == 38)
    {

        int tempVar40 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = -(BGOffset+1)*tempVar40; B <= (BGOffset+1)*tempVar40; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.5);
            tempLocation.Y = level[S].Y - 20; // Height - GFXBackground2height(a)
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }

    A = 39; // SMB3 Cave no Sky
    if(Background2[S] == 39)
    {

        int tempVar41 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = -(BGOffset+1)*tempVar41; B <= (BGOffset+1)*tempVar41; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
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
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }

    A = 40; // Mystic Cave Zone
    if(Background2[S] == 40)
    {
        if(level[S].Height - level[S].Y > GFXBackground2Height[A])
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

        int tempVar42 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = -(BGOffset+1)*tempVar42; B <= (BGOffset+1)*tempVar42; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.5);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 953, GFXBackground2Width[A], 47, GFXBackground2[A], 0, 953);
            }
        }

        int tempVar43 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.6))) + 1;
        for(B = -(BGOffset+1)*tempVar43; B <= (BGOffset+1)*tempVar43; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.6);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 916, GFXBackground2Width[A], 37, GFXBackground2[A], 0, 916);
            }
        }

        int tempVar44 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.7))) + 1;
        for(B = -(BGOffset+1)*tempVar44; B <= (BGOffset+1)*tempVar44; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.7);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 849, GFXBackground2Width[A], 67, GFXBackground2[A], 0, 849);
            }
        }

        int tempVar45 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.8))) + 1;
        for(B = -(BGOffset+1)*tempVar45; B <= (BGOffset+1)*tempVar45; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.8);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 815, GFXBackground2Width[A], 34, GFXBackground2[A], 0, 815);
            }
        }

        int tempVar46 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.9))) + 1;
        for(B = -(BGOffset+1)*tempVar46; B <= (BGOffset+1)*tempVar46; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.9);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 709, GFXBackground2Width[A], 106, GFXBackground2[A], 0, 709);
            }
        }

        int tempVar47 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.85))) + 1;
        for(B = -(BGOffset+1)*tempVar47; B <= (BGOffset+1)*tempVar47; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.85);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 664, GFXBackground2Width[A], 45, GFXBackground2[A], 0, 664);
            }
        }

        int tempVar48 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.8))) + 1;
        for(B = -(BGOffset+1)*tempVar48; B <= (BGOffset+1)*tempVar48; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.8);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 614, GFXBackground2Width[A], 50, GFXBackground2[A], 0, 614);
            }
        }

        int tempVar49 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.75))) + 1;
        for(B = -(BGOffset+1)*tempVar49; B <= (BGOffset+1)*tempVar49; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.75);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 540, GFXBackground2Width[A], 74, GFXBackground2[A], 0, 540);
            }
        }

        int tempVar50 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.7))) + 1;
        for(B = -(BGOffset+1)*tempVar50; B <= (BGOffset+1)*tempVar50; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.7);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 408, GFXBackground2Width[A], 132, GFXBackground2[A], 0, 408);
            }
        }

        int tempVar51 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.75))) + 1;
        for(B = -(BGOffset+1)*tempVar51; B <= (BGOffset+1)*tempVar51; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.75);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 333, GFXBackground2Width[A], 75, GFXBackground2[A], 0, 333);
            }
        }

        int tempVar52 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.8))) + 1;
        for(B = -(BGOffset+1)*tempVar52; B <= (BGOffset+1)*tempVar52; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.8);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 278, GFXBackground2Width[A], 55, GFXBackground2[A], 0, 278);
            }
        }

        int tempVar53 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.85))) + 1;
        for(B = -(BGOffset+1)*tempVar53; B <= (BGOffset+1)*tempVar53; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.85);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 235, GFXBackground2Width[A], 43, GFXBackground2[A], 0, 235);
            }
        }

        int tempVar54 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.9))) + 1;
        for(B = -(BGOffset+1)*tempVar54; B <= (BGOffset+1)*tempVar54; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.9);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 123, GFXBackground2Width[A], 112, GFXBackground2[A], 0, 123);
            }
        }

        int tempVar55 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.8))) + 1;
        for(B = -(BGOffset+1)*tempVar55; B <= (BGOffset+1)*tempVar55; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.8);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 85, GFXBackground2Width[A], 38, GFXBackground2[A], 0, 85);
            }
        }

        int tempVar56 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.7))) + 1;
        for(B = -(BGOffset+1)*tempVar56; B <= (BGOffset+1)*tempVar56; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.7);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 48, GFXBackground2Width[A], 37, GFXBackground2[A], 0, 48);
            }
        }

        int tempVar57 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.6))) + 1;
        for(B = -(BGOffset+1)*tempVar57; B <= (BGOffset+1)*tempVar57; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.6);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], 48, GFXBackground2[A], 0, 0);
            }
        }
    }

    A = 41; // SMB 1 Castle
    if(Background2[S] == 41)
    {

        int tempVar58 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 2;
        for(B = -(BGOffset+1)*tempVar58; B <= (BGOffset+1)*tempVar58; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
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
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }
    A = 42; // SMW Castle
    if(Background2[S] == 42)
    {

        int tempVar59 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = -(BGOffset+1)*tempVar59; B <= (BGOffset+1)*tempVar59; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A] / 4.0)
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
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A] / 4.0, GFXBackground2[A], 0, (GFXBackground2Height[A] / 4.0) * SpecialFrame[3]);
            }
        }
    }
    A = 43; // SMW Castle 2
    if(Background2[S] == 43)
    {

        int tempVar60 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = -(BGOffset+1)*tempVar60; B <= (BGOffset+1)*tempVar60; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
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
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }
    A = 44; // SMB2 Castle
    if(Background2[S] == 44)
    {

        int tempVar61 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = -(BGOffset+1)*tempVar61; B <= (BGOffset+1)*tempVar61; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
            {
                tempLocation.Y = (-vScreenY[Z] - level[S].Y) / (level[S].Height - level[S].Y - 600) * (GFXBackground2Height[A] - 600);
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }
    A = 45; // Brinstar
    if(Background2[S] == 45)
    {

        int tempVar62 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 2;
        for(B = -(BGOffset+1)*tempVar62; B <= (BGOffset+1)*tempVar62; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
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
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }
    A = 46; // Transport
    if(Background2[S] == 46)
    {

        int tempVar63 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 2;
        for(B = -(BGOffset+1)*tempVar63; B <= (BGOffset+1)*tempVar63; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
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
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }
    A = 47; // Transport
    if(Background2[S] == 47)
    {

        int tempVar64 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 2;
        for(B = -(BGOffset+1)*tempVar64; B <= (BGOffset+1)*tempVar64; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
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
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }
    A = 48; // SMB2 Blouds
    if(Background2[S] == 48)
    {

        int tempVar65 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 2;
        for(B = -(BGOffset+1)*tempVar65; B <= (BGOffset+1)*tempVar65; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
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
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }
    A = 49; // Desert Night
    if(Background2[S] == 49)
    {
        if(level[S].Height - level[S].Y > GFXBackground2Height[A])
        {
            // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
            tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] - ScreenH) + vScreen[Z].Top;
            tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
        }
        else
            tempLocation.Y = level[S].Height - GFXBackground2Height[A];
        tempLocation.Height = GFXBackground2Height[A];
        tempLocation.Width = GFXBackground2Width[A];

        int tempVar66 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = -(BGOffset+1)*tempVar66; B <= (BGOffset+1)*tempVar66; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.5);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 280, GFXBackground2Width[A], 450, GFXBackground2[A], 0, 280);
            }
        }

        int tempVar67 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.9))) + 1;
        for(B = -(BGOffset+1)*tempVar67; B <= (BGOffset+1)*tempVar67; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.9);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 268, GFXBackground2Width[A], 12, GFXBackground2[A], 0, 268);
            }
        }

        int tempVar68 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.89))) + 1;
        for(B = -(BGOffset+1)*tempVar68; B <= (BGOffset+1)*tempVar68; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.89);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 244, GFXBackground2Width[A], 24, GFXBackground2[A], 0, 244);
            }
        }

        int tempVar69 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.88))) + 1;
        for(B = -(BGOffset+1)*tempVar69; B <= (BGOffset+1)*tempVar69; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.88);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 228, GFXBackground2Width[A], 16, GFXBackground2[A], 0, 228);
            }
        }

        int tempVar70 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.87))) + 1;
        for(B = -(BGOffset+1)*tempVar70; B <= (BGOffset+1)*tempVar70; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.87);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 196, GFXBackground2Width[A], 32, GFXBackground2[A], 0, 196);
            }
        }

        int tempVar71 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.86))) + 1;
        for(B = -(BGOffset+1)*tempVar71; B <= (BGOffset+1)*tempVar71; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.86);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 164, GFXBackground2Width[A], 32, GFXBackground2[A], 0, 164);
            }
        }

        int tempVar72 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.85))) + 1;
        for(B = -(BGOffset+1)*tempVar72; B <= (BGOffset+1)*tempVar72; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.85);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 116, GFXBackground2Width[A], 48, GFXBackground2[A], 0, 116);
            }
        }

        int tempVar73 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.84))) + 1;
        for(B = -(BGOffset+1)*tempVar73; B <= (BGOffset+1)*tempVar73; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.84);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 58, GFXBackground2Width[A], 58, GFXBackground2[A], 0, 58);
            }
        }

        int tempVar74 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.83))) + 1;
        for(B = -(BGOffset+1)*tempVar74; B <= (BGOffset+1)*tempVar74; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.83);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], 58, GFXBackground2[A], 0, 0);
            }
        }
    }
    A = 50; // Shrooms
    if(Background2[S] == 50)
    {
        if(level[S].Height - level[S].Y > GFXBackground2Height[A])
        {
            // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
            tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] - ScreenH) + vScreen[Z].Top;
            tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
        }
        else
            tempLocation.Y = level[S].Height - GFXBackground2Height[A];
        tempLocation.Height = GFXBackground2Height[A];
        tempLocation.Width = GFXBackground2Width[A];

        int tempVar75 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = -(BGOffset+1)*tempVar75; B <= (BGOffset+1)*tempVar75; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.5);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 378, GFXBackground2Width[A], 378, GFXBackground2[A], 0, 378);
            }
        }

        int tempVar76 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.65))) + 1;
        for(B = -(BGOffset+1)*tempVar76; B <= (BGOffset+1)*tempVar76; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.65);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], 220, GFXBackground2[A], 0, 0);
            }
        }

        int tempVar77 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.6))) + 1;
        for(B = -(BGOffset+1)*tempVar77; B <= (BGOffset+1)*tempVar77; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.6);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 220, GFXBackground2Width[A], 159, GFXBackground2[A], 0, 220);
            }
        }
    }

    A = 51; // SMB1 Desert
    if(Background2[S] == 51)
    {
        if(level[S].Height - level[S].Y > GFXBackground2Height[A])
        {
            // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
            tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] - ScreenH) + vScreen[Z].Top;
            tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
        }
        else
            tempLocation.Y = level[S].Height - GFXBackground2Height[A];
        tempLocation.Height = GFXBackground2Height[A];
        tempLocation.Width = GFXBackground2Width[A];

        int tempVar78 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.75))) + 1;
        for(B = -(BGOffset+1)*tempVar78; B <= (BGOffset+1)*tempVar78; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.75);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], 350, GFXBackground2[A], 0, 0);
            }
        }

        int tempVar79 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = -(BGOffset+1)*tempVar79; B <= (BGOffset+1)*tempVar79; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.5);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 350, GFXBackground2Width[A], GFXBackground2Height[A] - 350, GFXBackground2[A], 0, 350);
            }
        }
    }
    A = 52; // SMB2 Desert Night
    if(Background2[S] == 52)
    {
        if(level[S].Height - level[S].Y > GFXBackground2Height[A])
        {
            // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
            tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] - ScreenH) + vScreen[Z].Top;
            tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
        }
        else
            tempLocation.Y = level[S].Height - GFXBackground2Height[A];
        tempLocation.Height = GFXBackground2Height[A];
        tempLocation.Width = GFXBackground2Width[A];

        int tempVar80 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = -(BGOffset+1)*tempVar80; B <= (BGOffset+1)*tempVar80; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.5);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 280, GFXBackground2Width[A], GFXBackground2Height[A] - 280, GFXBackground2[A], 0, 280);
            }
        }

        int tempVar81 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.9))) + 1;
        for(B = -(BGOffset+1)*tempVar81; B <= (BGOffset+1)*tempVar81; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.9);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 268, GFXBackground2Width[A], 12, GFXBackground2[A], 0, 268);
            }
        }

        int tempVar82 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.89))) + 1;
        for(B = -(BGOffset+1)*tempVar82; B <= (BGOffset+1)*tempVar82; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.89);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 244, GFXBackground2Width[A], 24, GFXBackground2[A], 0, 244);
            }
        }

        int tempVar83 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.88))) + 1;
        for(B = -(BGOffset+1)*tempVar83; B <= (BGOffset+1)*tempVar83; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.88);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 228, GFXBackground2Width[A], 16, GFXBackground2[A], 0, 228);
            }
        }

        int tempVar84 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.87))) + 1;
        for(B = -(BGOffset+1)*tempVar84; B <= (BGOffset+1)*tempVar84; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.87);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 196, GFXBackground2Width[A], 32, GFXBackground2[A], 0, 196);
            }
        }

        int tempVar85 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.86))) + 1;
        for(B = -(BGOffset+1)*tempVar85; B <= (BGOffset+1)*tempVar85; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.86);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 164, GFXBackground2Width[A], 32, GFXBackground2[A], 0, 164);
            }
        }

        int tempVar86 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.85))) + 1;
        for(B = -(BGOffset+1)*tempVar86; B <= (BGOffset+1)*tempVar86; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.85);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 116, GFXBackground2Width[A], 48, GFXBackground2[A], 0, 116);
            }
        }

        int tempVar87 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.84))) + 1;
        for(B = -(BGOffset+1)*tempVar87; B <= (BGOffset+1)*tempVar87; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.84);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 58, GFXBackground2Width[A], 58, GFXBackground2[A], 0, 58);
            }
        }

        int tempVar88 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.83))) + 1;
        for(B = -(BGOffset+1)*tempVar88; B <= (BGOffset+1)*tempVar88; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.83);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], 58, GFXBackground2[A], 0, 0);
            }
        }
    }
    A = 53; // Cliffs
    if(Background2[S] == 53)
    {

        int tempVar89 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 2;
        for(B = -(BGOffset+1)*tempVar89; B <= (BGOffset+1)*tempVar89; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
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
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }
    A = 54; // Warehouse
    if(Background2[S] == 54)
    {

        int tempVar90 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 2;
        for(B = -(BGOffset+1)*tempVar90; B <= (BGOffset+1)*tempVar90; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
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
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }

    A = 55; // SMW Water
    if(Background2[S] == 55)
    {

        int tempVar91 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = -(BGOffset+1)*tempVar91; B <= (BGOffset+1)*tempVar91; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A] / 4.0)
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) / 4 - 600)
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] / 4.0 - ScreenH) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A] / 4.0;
            tempLocation.Height = GFXBackground2Height[A] / 4;
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A] / 4.0, GFXBackground2[A], 0, (GFXBackground2Height[A] / 4.0) * SpecialFrame[3]);
            }
        }
    }

    A = 56; // SMB3 Water
    if(Background2[S] == 56)
    {
        if(level[S].Height - level[S].Y > GFXBackground2Height[A])
        {
            // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
            tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] - ScreenH) + vScreen[Z].Top;
            tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
        }
        else
            tempLocation.Y = level[S].Height - GFXBackground2Height[A];
        tempLocation.Height = GFXBackground2Height[A];
        tempLocation.Width = GFXBackground2Width[A];



        int tempVar92 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.65))) + 1;
        for(B = -(BGOffset+1)*tempVar92; B <= (BGOffset+1)*tempVar92; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.65);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], 100, GFXBackground2[A], 0, 0);
            }
        }


        int tempVar93 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.6))) + 1;
        for(B = -(BGOffset+1)*tempVar93; B <= (BGOffset+1)*tempVar93; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.6);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 100, GFXBackground2Width[A], 245, GFXBackground2[A], 0, 100);
            }
        }


        int tempVar94 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.55))) + 1;
        for(B = -(BGOffset+1)*tempVar94; B <= (BGOffset+1)*tempVar94; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.55);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 345, GFXBackground2Width[A], 110, GFXBackground2[A], 0, 345);
            }
        }


        int tempVar95 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = -(BGOffset+1)*tempVar95; B <= (BGOffset+1)*tempVar95; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.5);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 455, GFXBackground2Width[A], GFXBackground2Height[A] - 455, GFXBackground2[A], 0, 455);
            }
        }
    }

    A = 57; // Warehouse
    if(Background2[S] == 57)
    {
        int tempVar96 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 2;
        for(B = -(BGOffset+1)*tempVar96; B <= (BGOffset+1)*tempVar96; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
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
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X,
                                      vScreenY[Z] + tempLocation.Y,
                                      GFXBackground2Width[A],
                                      GFXBackground2Height[A],
                                      GFXBackground2[A],
                                      0, 0);
            }
        }
    }

    A = 58; // SMW Night
    if(Background2[S] == 58)
    {
        int tempVar97 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = -(BGOffset+1)*tempVar97; B <= (BGOffset+1)*tempVar97; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A] / 4.0)
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
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X,
                                      vScreenY[Z] + tempLocation.Y,
                                      GFXBackground2Width[A],
                                      GFXBackground2Height[A] / 4.0,
                                      GFXBackground2[A],
                                      0,
                                      (GFXBackground2Height[A] / 4.0) * SpecialFrame[6]
                );
            }
        }
    }

    A = 59; // SMB3 Night Hills
    if(Background2[S] == 59)
    {
        int tempVar98 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = -(BGOffset+1)*tempVar98; B <= (BGOffset+1)*tempVar98; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A] / 3.0)
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - (600 - vScreen(Z).Top)) * (GFXBackground2Height(A) / 4 - (600 - vScreen(Z).Top))
                // .Y = -vScreenY(Z) - .Y
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] / 3.0 - ScreenH) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A] / 3.0;
            tempLocation.Height = GFXBackground2Height[A] / 3;
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X,
                                      vScreenY[Z] + tempLocation.Y,
                                      GFXBackground2Width[A],
                                      GFXBackground2Height[A] / 3.0,
                                      GFXBackground2[A],
                                      0,
                                      (GFXBackground2Height[A] / 3.0) * SpecialFrame[10]
                );
            }
        }
    }

    A = 60; // SMB3 Giant Hills
    if(Background2[S] == 60)
    {
        int tempVar99 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = -(BGOffset+1)*tempVar99; B <= (BGOffset+1)*tempVar99; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.5);
            tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }

    A = 61; // SMW Castle 3
    if(Background2[S] == 61)
    {
        int tempVar100 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 21;
        for(B = -(BGOffset+1)*tempVar100; B <= (BGOffset+1)*tempVar100; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A] / 2.0)
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - (600 - vScreen(Z).Top)) * (GFXBackground2Height(A) / 4 - (600 - vScreen(Z).Top))
                // .Y = -vScreenY(Z) - .Y
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] / 2.0 - ScreenH) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A] / 2.0;
            tempLocation.Height = GFXBackground2Height[A] / 2;
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X,
                                      vScreenY[Z] + tempLocation.Y,
                                      GFXBackground2Width[A],
                                      GFXBackground2Height[A] / 2.0,
                                      GFXBackground2[A],
                                      0,
                                      (GFXBackground2Height[A] / 2.0) * SpecialFrame[11]
                );
            }
        }
    }

    A = 62; // SMW Hills 4
    if(Background2[S] == 62)
    {

        int tempVar101 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = -(BGOffset+1)*tempVar101; B <= (BGOffset+1)*tempVar101; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
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
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }

    A = 63; // smb3 pyramid
    if(Background2[S] == 63)
    {

        int tempVar102 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A]))) + 2;
        int tempVar103 = static_cast<int>(floor(static_cast<double>((level[S].Height - level[S].Y) / GFXBackground2Height[A]))) + 2;
        for(B = -(BGOffset+1)*tempVar102; B <= (BGOffset+1)*tempVar102; B++)
        {
            for(int C = -tempVar103; C <= tempVar103; C++)
            {
                tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.5);
                tempLocation.Y = level[S].Y + ((C * GFXBackground2Height[A]) - (vScreenY[Z] + vScreen[Z].Top + level[S].Y) * 0.5);
                tempLocation.Height = GFXBackground2Height[A];
                tempLocation.Width = GFXBackground2Width[A];
                if(vScreenCollision(Z, tempLocation))
                {
                    frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
                }
            }
        }
    }

    A = 64; // smb3 ice cave
    if(Background2[S] == 64)
    {

        int tempVar104 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A]))) + 2;
        int tempVar105 = static_cast<int>(floor(static_cast<double>((level[S].Height - level[S].Y) / GFXBackground2Height[A] / 6.0))) + 6.0;
        for(B = -(BGOffset+1)*tempVar104; B <= (BGOffset+1)*tempVar104; B++)
        {
            for(int C = -tempVar105; C <= tempVar105; C++)
            {
                tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.5);
                tempLocation.Y = level[S].Y + ((C * GFXBackground2Height[A] / 6.0) - (vScreenY[Z] + vScreen[Z].Top + level[S].Y) * 0.5);
                tempLocation.Height = GFXBackground2Height[A] / 6.0;
                tempLocation.Width = GFXBackground2Width[A];
                if(vScreenCollision(Z, tempLocation))
                {
                    frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A] / 6.0, GFXBackground2[A], 0, (GFXBackground2Height[A] / 6.0) * SpecialFrame[12]);
                }
            }
        }
    }

    A = 65; // sma4 metalic
    if(Background2[S] == 65)
    {

        int tempVar106 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A]))) + 2;
        int tempVar107 = static_cast<int>(floor(static_cast<double>((level[S].Height - level[S].Y) / GFXBackground2Height[A]))) + 2;
        for(B = -(BGOffset+1)*tempVar106; B <= (BGOffset+1)*tempVar106; B++)
        {
            for(int C = -tempVar107; C <= tempVar107; C++)
            {
                tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.5);
                tempLocation.Y = level[S].Y + ((C * GFXBackground2Height[A]) - (vScreenY[Z] + vScreen[Z].Top + level[S].Y) * 0.5);
                tempLocation.Height = GFXBackground2Height[A];
                tempLocation.Width = GFXBackground2Width[A];
                if(vScreenCollision(Z, tempLocation))
                {
                    frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
                }
            }
        }
    }


    A = 66; // smb2 snow hills
    if(Background2[S] == 66)
    {

        int tempVar108 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = -(BGOffset+1)*tempVar108; B <= (BGOffset+1)*tempVar108; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
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
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }

    A = 67; // smb2 snow mountains
    if(Background2[S] == 67)
    {

        int tempVar109 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = -(BGOffset+1)*tempVar109; B <= (BGOffset+1)*tempVar109; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
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
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }

    A = 68; // smb2 mountains
    if(Background2[S] == 68)
    {

        int tempVar110 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = -(BGOffset+1)*tempVar110; B <= (BGOffset+1)*tempVar110; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X + BGOffset * (level[S].Width - level[S].X)) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
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
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }

    level[S] = tempLevel;

}
