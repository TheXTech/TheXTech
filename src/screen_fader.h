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

#ifndef SCREEN_FADER_H
#define SCREEN_FADER_H

#include "frm_main.h"
#include "fader.h"
#include "global_constants.h"

extern FrmMain frmMain;


struct ScreenFader
{
    bool m_active = false;
    bool m_full = false;
    PGE_Fader m_fader;
    float m_scale = 0.0f;

    float color_r = 0.f;
    float color_g = 0.f;
    float color_b = 0.f;

    enum Shape
    {
        S_FADE = 0,
        S_RECT
    } m_shape = S_FADE;

    void clearFader()
    {
        m_active = false;
    }

    void setupFader(int step, int start, int goal, Shape shape)
    {
        m_shape = shape;
        m_fader.setRatio(start / 65.0);
        m_fader.setFade(15, goal / 65.0, step / 65.0);
        m_scale = (float)m_fader.fadeRatio();
        m_active = true;
        m_full = false;
    }

    void update()
    {
        if(!m_active)
            return;

        if(!m_fader.isFading())
        {
            if(m_scale <= 0.0f)
                m_active = false;
            else if(m_scale >= 1.0f)
                m_full = true;
        }

        m_fader.tickFader(1000.0 / 65.0);
        m_scale = (float)m_fader.fadeRatio();
    }

    void draw()
    {
        if(!m_active)
            return;

        frmMain.offsetViewportIgnore(true);

        switch(m_shape)
        {
        case S_FADE:
            frmMain.renderRect(0, 0, ScreenW, ScreenH, color_r, color_b, color_g, m_scale, true);
            break;

        case S_RECT:
        {
            if(m_scale >= 1.0f)
                frmMain.renderRect(0, 0, ScreenW, ScreenH, color_r, color_b, color_g, m_scale, true);
            else
            {
                float hw = (ScreenW / 2) * m_scale, hh = (ScreenH / 2) * m_scale;
                // Left side
                frmMain.renderRect(0, 0, hw, ScreenH, color_r, color_b, color_g, 1.f, true);
                // right side
                frmMain.renderRect(ScreenW - hw + 1, 0, hw, ScreenH, color_r, color_b, color_g, 1.f, true);
                // Top side
                frmMain.renderRect(0, 0, ScreenW, hh, color_r, color_b, color_g, 1.f, true);
                // Bottom side
                frmMain.renderRect(0, ScreenH - hh + 1, ScreenW, hh, color_r, color_b, color_g, 1.f, true);
            }
        }
            break;
        }

        frmMain.offsetViewportIgnore(false);
    }
};

#endif // SCREEN_FADER_H
