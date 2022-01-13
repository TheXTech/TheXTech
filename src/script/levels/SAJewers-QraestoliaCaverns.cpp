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

/*************************************************
 *  Episode:    A2MBXT, Episode 1: Analog Funk   *
 *  Level:      Qraestolia Caverns               *
 *  Filename:   SAJewers-QraestoliaCaverns.lvl   *
 *  Author:     SAJewers                         *
 *************************************************/

#include "SAJewers-QraestoliaCaverns.h"
#include "globals.h"
#include "../lunaplayer.h"
#include "../lunainput.h"

void QraestoliaCavernsCode()
{
    static int lastDownPress = 0;
    Player_t *demo = PlayerF::Get(1);
    if(demo == 0)
        return;

    // Player pressed down, what do we do?
    if(PlayerF::PressingDown(demo))
    {
        // Else, see if pressed down in the last 7 frames
        if(gFrames < lastDownPress + 10 && gFrames > lastDownPress + 1)
        {
            if(demo->Character == 2 && demo->MountType != 0)
                PlayerF::CycleLeft(demo);
            else
                PlayerF::CycleRight(demo);

            lastDownPress = gFrames - 9;
            return;
        }

        // Else, set last press frame as this one
        lastDownPress = gFrames;
    }
}
