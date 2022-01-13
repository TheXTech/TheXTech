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
 *  Level:      Abstract Assault                 *
 *  Filename:   Docopoper-AbstractAssault.lvl    *
 *  Author:     Docopoper                        *
 *************************************************/

#include <SDL2/SDL_assert.h>

#include "globals.h"
#include "layers.h"
#include "../lunaplayer.h"
#include "../lunalayer.h"
#include "Docopoper-AbstractAssault.h"


#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define CLAMP(x, l, h) (((x) > (h)) ? (h) : (((x) < (l)) ? (l) : (x)))


void AbstractAssaultCode()
{
    Player_t *demo = PlayerF::Get(1);
    Layer_t *layerDefault = LayerF::Get(0);
    Layer_t *layerStartingPlatform = LayerF::Get(3);

    SDL_assert_release(demo);
    SDL_assert_release(layerDefault);
    SDL_assert_release(layerStartingPlatform);

    static float hspeed = 0, vspeed = 0;
    static char gameStarted = 0;
    static unsigned short noControlTimer = 0;

    bool press_up = demo->Controls.Up;
    bool press_left = demo->Controls.Left;
    bool press_down = demo->Controls.Down;
    bool press_right = demo->Controls.Right;

    auto &powerup = demo->State;

    if(layerStartingPlatform -> SpeedY == 0)
    {
        demo -> Character = 1; //Demo
        //player_id_set = 1;
        gameStarted = 0;
        powerup = 1;
    }
    else if(!gameStarted)
    {
        powerup = 6;
        demo -> SpinJump = false;
        demo -> Hearts = 3;
        vspeed = -16;
        hspeed = -4;
        gameStarted = 1;
        noControlTimer = 30;
        demo->Character = 5; //Sheath
    }

    if(gameStarted)
    {
        if(layerStartingPlatform -> SpeedY == 0)
            gameStarted = 0;

        demo -> SpinJump = false;

        layerDefault -> SpeedX = (float) MAX(layerDefault -> SpeedX - 0.015, -2.5);

        if(demo -> Hearts > 1)
            powerup = 6;

        demo -> Direction = 1;
        demo -> Multiplier %= 9;

        if(noControlTimer == 0)
        {
            vspeed = (float) CLAMP(vspeed + (press_down - press_up) * 0.5, -10, 10);
            hspeed = (float) CLAMP(hspeed + (press_right - press_left) * 0.5, -10, 10);
        }
        else
            noControlTimer--;

        demo->Location.SpeedY = -0.4 + vspeed;
        demo->Location.SpeedX = hspeed;

        hspeed *= (float) 0.9;
        vspeed *= (float) 0.9;
    }
}
