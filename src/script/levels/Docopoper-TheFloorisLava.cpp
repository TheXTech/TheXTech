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
 *  Level:      The Floor is Lava                *
 *  Filename:   Docopoper-TheFloorisLava.lvl     *
 *  Author:     Docopoper                        *
 *************************************************/

#include "globals.h"
#include "../lunaplayer.h"
#include "../lunalayer.h"
#include "layers.h"
#include "Docopoper-TheFloorisLava.h"


void TheFloorisLavaCode()
{
    Player_t *demo = PlayerF::Get(1);
    Layer_t *layerSecretExit = LayerF::Get(3);
    Layer_t *layerSinUpDown = LayerF::Get(4);
    Layer_t *layerSinRightLeft = LayerF::Get(5);
    Layer_t *layerSinUpDownAlternate = LayerF::Get(6);
    Layer_t *layerSinUpDownWeak = LayerF::Get(7);
    Layer_t *layerSinUpDownWeakAlt = LayerF::Get(8);
    Layer_t *layerSinRightLeftAlternate = LayerF::Get(9);

    static double layerSinUpDown_displacement = 0;
    static double layerSinRightLeft_displacement = 0;
    static char secretExitFailedStage = 0;

    if(demo == 0)
        return;

    // The player has touched the ground, trigger the fail sequence
    if(demo->Slippy && secretExitFailedStage == 0)
    {
        secretExitFailedStage = 1;
        layerSecretExit -> SpeedY = (float) -0.4;
    }

    //different movements to create the dropping off the world effect
    switch(secretExitFailedStage)
    {
    case 0:
        layerSecretExit -> SpeedY = (float)0;
        break;

    case 1:
        layerSecretExit -> SpeedY -= (float)0.2;

        if(layerSecretExit -> SpeedY < -4)
            secretExitFailedStage = 2;
        break;

    case 2:
        layerSecretExit -> SpeedY += (float)0.4;

        if(layerSecretExit -> SpeedY > -2.5)
            secretExitFailedStage = 3;
        break;

    case 3:
        layerSecretExit -> SpeedY += 1.5;

        if(layerSecretExit -> SpeedY > 50)
            secretExitFailedStage = 4;
        break;

    case 4:
        layerSecretExit -> SpeedY = 0;
        secretExitFailedStage = 5;
        break;
    }

    //Up Down sine wave motion
    if(layerSinUpDown -> SpeedY == 0)
    {
        secretExitFailedStage = 0;

        layerSinRightLeft_displacement = 0;
        layerSinRightLeft -> SpeedX = 3;

        layerSinUpDown_displacement = 0;
        layerSinUpDown -> SpeedY = 3;
    }

    layerSinUpDown -> SpeedY -= (float)layerSinUpDown_displacement / 1000;
    layerSinUpDown_displacement += layerSinUpDown -> SpeedY;

    layerSinUpDownAlternate -> SpeedY = -(layerSinUpDown -> SpeedY);
    layerSinUpDownWeak -> SpeedY = (layerSinUpDown -> SpeedY) / 3;
    layerSinUpDownWeakAlt -> SpeedY = -(layerSinUpDown -> SpeedY) / 3;

    //stop the initial trigger going off again
    if(layerSinUpDown -> SpeedY == (float)0)
        layerSinUpDown -> SpeedY = (float)0.001;

    //Right Left sine wave motion
    layerSinRightLeft -> SpeedX -= (float)layerSinRightLeft_displacement / 2000;
    layerSinRightLeft_displacement += layerSinRightLeft -> SpeedX;
    layerSinRightLeftAlternate -> SpeedX = -(layerSinRightLeft -> SpeedX);
}
