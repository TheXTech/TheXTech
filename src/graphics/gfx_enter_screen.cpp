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
#include "../player.h"


void GameThing()
{
    int A = 0;
//    int B = 0;
    Player_t tempPlayer[3];
    tempPlayer[1] = Player[1];
    tempPlayer[2] = Player[2];

    SetupPlayers();
    Player[1].Location.SpeedY = 0;
    Player[2].Location.SpeedY = 0;

    if(numPlayers == 1)
    {
        Player[1].Location.X = ScreenW / 2.0 - Player[1].Location.Width / 2.0;
        Player[1].Location.Y = ScreenH / 2.0 - Player[1].Location.Height + 24;
        Player[1].Direction = 1;
    }
    else
    {
        Player[1].Location.X = ScreenW / 2.0 - Player[1].Location.Width / 2.0 - 30;
        Player[1].Location.Y = ScreenH / 2.0 - Player[1].Location.Height + 24;
        Player[1].Direction = -1;
        Player[2].Location.X = ScreenW / 2.0 - Player[2].Location.Width / 2.0 + 32;
        Player[2].Location.Y = ScreenH / 2.0 - Player[2].Location.Height + 24;
        Player[2].Direction = 1;
    }

    PlayerFrame(1);
    PlayerFrame(2);

    frmMain.clearBuffer();

    for(A = 1; A <= numPlayers; A++)
    {
        DrawPlayer(A, 0);
    }

    if(TestLevel)
    {
        std::string loading = "LOADING...";
        SuperPrint(loading, 3, (ScreenW / 2.0f) - float(loading.size() / 2) * 18, ScreenH / 2.0f + 32);
    }
    else
    {
        frmMain.renderTexture(ScreenW / 2.0 - 46, ScreenH / 2.0 + 31, GFX.Interface[3].w, GFX.Interface[3].h, GFX.Interface[3], 0, 0);
        frmMain.renderTexture(ScreenW / 2.0 - GFX.Interface[1].w / 2, ScreenH / 2.0 + 32, GFX.Interface[1].w, GFX.Interface[1].h, GFX.Interface[1], 0, 0);
        SuperPrint(std::to_string(int(Lives)), 1, ScreenW / 2.0 + 12, ScreenH / 2.0 + 32);
    }

    frmMain.repaint();

    Player[1] = tempPlayer[1];
    Player[2] = tempPlayer[2];
    DoEvents();
}
