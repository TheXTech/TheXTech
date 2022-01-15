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

#include "renderop_effect.h"
#include "core/render.h"
#include "globals.h"

// DRAW
RenderEffectOp::RenderEffectOp() :
    effect_type(RNDEFF_ScreenGlow),
    blend_type(BLEND_Additive),
    color(0x00000000),
    intensity(0),
    flip_type(FLIP_TYPE_NONE) {}

void RenderEffectOp::Draw(Renderer *g)
{
    switch(effect_type)
    {
    case RNDEFF_ScreenGlow:
        ScreenGlow(g);
        break;
    case RNDEFF_Flip:
        Flip(g);
        break;
    default:
        break;
    }
}

// SCREEN GLOW
void RenderEffectOp::ScreenGlow(Renderer *g)
{
    UNUSED(g);
    // TODO: Re-Implement this differently

//    HDC hScreen = g->GetScreenDC();

//    HBITMAP hOld = (HBITMAP)SelectObject(ghMemDC, ghGeneralDIB);

//    if(ghGeneralDIB && ghMemDC && gpScreenBits)
//    {
//        BitBlt(ghMemDC, 0, 0, 800, 600, hScreen, 0, 0, SRCCOPY);

//#ifndef __MINGW32__
//        // MMX code and loop
//        _mm_empty();
//        int nLoops = (800 * 600) / 2;
//        __m64 color64 = _mm_set_pi32(color, color);
//        __m64 *pDest = (__m64 *)gpScreenBits;

//        if(blend_type == BLEND_Additive)
//        {
//            for(int i = 0; i < nLoops; i++)
//            {
//                //tmp =
//                pDest[i] = _mm_adds_pu8(color64, pDest[i]);

//                //pDest[i] = tmp;
//            }
//        }
//        else if(blend_type == BLEND_Subtractive)
//        {
//            for(int i = 0; i < nLoops; i++)
//            {
//                //tmp =
//                pDest[i] = _mm_subs_pu8(color64, pDest[i]);

//                //pDest[i] = tmp;
//            }
//        }

//        _mm_empty();
//#endif

//        BitBlt(hScreen, 0, 0, 800, 600, ghMemDC, 0, 0, SRCCOPY);

//        SelectObject(ghMemDC, hOld);
//    }
}


// Flip
void RenderEffectOp::Flip(Renderer *g)
{
    UNUSED(g);
//    HDC hScreen = g->GetScreenDC();

//    HBITMAP hOld = (HBITMAP)SelectObject(ghMemDC, ghGeneralDIB);
//    if(!(ghGeneralDIB && ghMemDC && gpScreenBits)) return;

//    BitBlt(ghMemDC, 0, 0, 800, 600, hScreen, 0, 0, SRCCOPY);

//    StretchBlt(hScreen,
//               (flip_type & FLIP_TYPE_X) ? 800 : 0,
//               (flip_type & FLIP_TYPE_Y) ? 600 : 0,
//               (flip_type & FLIP_TYPE_X) ? -800 : 800,
//               (flip_type & FLIP_TYPE_Y) ? -600 : 600,
//               ghMemDC, 0, 0, 800, 600, SRCCOPY);

//    SelectObject(ghMemDC, hOld);
}
