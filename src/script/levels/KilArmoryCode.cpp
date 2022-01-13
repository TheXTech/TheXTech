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

/**************************************************
 *  Episode:    MAGL X2 ?????                     *
 *  Level:      Thou starts a new video           *
 *  Filename:   LUNA12-thou_starts_a_new_video.lvl*
 *  Author:     ??????                            *
 **************************************************/

#include "KilArmoryCode.h"
#include "globals.h"
#include "../lunaplayer.h"
#include "../lunainput.h"
#include "../lunarender.h"
#include "../renderop_effect.h"
#include "../autocode_manager.h"


void KilArmoryInit()
{
    auto *demo = PlayerF::Get(1);
    if(demo)
    {
        PlayerF::FilterToBig(demo);
        PlayerF::FilterMount(demo);
        PlayerF::FilterReservePowerup(demo);
        demo->Character = 1;
    }
}

void KilArmoryCode()
{
    Player_t *demo = PlayerF::Get(1);

    if(demo)
    {
        // Section 20(19) glow effect code
        if(gFrames > 60 && demo->Section == 19)
        {
            int intensity = (int)(sin((float)(gFrames) / 22) * 35) + 60;
            intensity <<= 16;
            RenderEffectOp *op = new RenderEffectOp(RNDEFF_ScreenGlow, BLEND_Additive, intensity, 100);
            op->m_FramesLeft = 1;
            Renderer::Get().AddOp(op);
        }

        // Section 1(0) glow effect code
        if(gFrames > 60 && demo->Section == 0 && gAutoMan.GetVar("GOTSANGRE") == 0)
        {
            int intensity = (int)(sin((float)(gFrames) / 10) * 45) + 48;
            intensity <<= 16;
            RenderEffectOp *op = new RenderEffectOp(RNDEFF_ScreenGlow, BLEND_Additive, intensity, 100);
            op->m_FramesLeft = 1;
            Renderer::Get().AddOp(op);
        }
    }
}

