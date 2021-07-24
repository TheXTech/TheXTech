/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2021 Vitaly Novichkov <admin@wohlnet.ru>
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
#include "../player.h"


void GameThing()
{
    int A = 0;
//    int B = 0;
    Player_t tempPlayer[3];

    frmMain.setTargetTexture();

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
#ifdef __3DS__
    frmMain.initDraw(0);
#endif

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
