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

#include <cmath>

#include "frm_main.h"
#include "global_constants.h"
#include "range_arr.hpp"

#include "screen_fader.h"

extern FrmMain frmMain;
//Public vScreenX(0 To maxPlayers) As Double  'vScreen offset
extern RangeArr<double, 0, maxPlayers> vScreenX;
//Public vScreenY(0 To maxPlayers) As Double 'vScreen offset
extern RangeArr<double, 0, maxPlayers> vScreenY;


void ScreenFader::clearFader()
{
    m_active = false;
    m_scale = 0.0f;
    m_full = false;
    m_focusX = -1;
    m_focusY = -1;
    m_focusScreen = -1;
    m_focusSet = false;
    m_focusTrackX = nullptr;
    m_focusTrackY = nullptr;
    m_focusOffsetX = 0.0;
    m_focusOffsetY = 0.0;
}

void ScreenFader::setupFader(int step, int start, int goal, Shape shape, bool useFocus, int focusX, int focusY, int screen)
{
    m_shape = shape;
    m_fader.setRatio(start / 65.0);
    m_fader.setFade(15, goal / 65.0, step / 65.0);
    m_scale = (float)m_fader.fadeRatio();
    m_active = true;
    m_full = false;
    m_complete = false;
    m_focusSet = useFocus;
    m_focusX = focusX;
    m_focusY = focusY;
    m_focusScreen = screen;
    m_focusTrackX = nullptr;
    m_focusTrackY = nullptr;
    m_focusOffsetX = 0.0;
    m_focusOffsetY = 0.0;
}

void ScreenFader::setTrackedFocus(double *x, double *y, double offX, double offY)
{
    m_focusTrackX = x;
    m_focusTrackY = y;
    m_focusOffsetX = offX;
    m_focusOffsetY = offY;
}

bool ScreenFader::isComplete()
{
    return m_complete;
}

bool ScreenFader::isVisible()
{
    return m_active || m_full;
}

void ScreenFader::update()
{
    if(!m_active)
        return;

    if(!m_fader.isFading())
    {
        if(m_scale <= 0.0f)
        {
            m_active = false;
            m_complete = true;
        }
        else if(m_scale >= 1.0f)
        {
            m_full = true;
            m_complete = true;
        }
    }

    if(m_focusSet)
    {
        if(m_focusTrackX)
            m_focusX = *m_focusTrackX + m_focusOffsetX;
        if(m_focusTrackY)
            m_focusY = *m_focusTrackY + m_focusOffsetY;
    }

    m_fader.tickFader(1000.0 / 65.0);
    m_scale = (float)m_fader.fadeRatio();
}

void ScreenFader::draw()
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
        if(m_scale >= 1.0f)
            frmMain.renderRect(0, 0, ScreenW, ScreenH, color_r, color_b, color_g, m_scale, true);
        else
        {
            int focusX = m_focusSet ? m_focusX : (ScreenW / 2);
            int focusY = m_focusSet ? m_focusY : (ScreenH / 2);

            if(m_focusScreen >= 0)
            {
                focusX += vScreenX[m_focusScreen];
                focusY += vScreenY[m_focusScreen];
            }

            float rightW = (ScreenW -focusX),
                    bottomH = (ScreenH - focusY),
                    leftW = focusX * m_scale, // left side
                    topY = focusY * m_scale, // top side
                    rightX = ScreenW - SDL_ceil(rightW * m_scale) + 1, // right side
                    bottomY = ScreenH - SDL_ceil(bottomH * m_scale) + 1; // bottom side

            // Left side
            frmMain.renderRect(0, 0, leftW, ScreenH, color_r, color_b, color_g, 1.f, true);
            // right side
            frmMain.renderRect(rightX, 0, rightW * m_scale, ScreenH, color_r, color_b, color_g, 1.f, true);
            // Top side
            frmMain.renderRect(0, 0, ScreenW, topY, color_r, color_b, color_g, 1.f, true);
            // Bottom side
            frmMain.renderRect(0, bottomY, ScreenW, bottomH * m_scale, color_r, color_b, color_g, 1.f, true);
        }
        break;

    case S_CIRCLE:
        if(m_scale >= 1.0f)
            frmMain.renderRect(0, 0, ScreenW, ScreenH, color_r, color_b, color_g, m_scale, true);
        else
        {
            int focusX = m_focusSet ? m_focusX : (ScreenW / 2);
            int focusY = m_focusSet ? m_focusY : (ScreenH / 2);

            if(m_focusScreen)
            {
                focusX += vScreenX[m_focusScreen];
                focusY += vScreenY[m_focusScreen];
            }

            // int radius = ScreenH - (ScreenH * m_scale);
            int maxRadius = 0, maxRadiusPre;

            // top-left corner
            maxRadiusPre = std::sqrt(SDL_pow(focusX, 2) + SDL_pow(focusY, 2));
            if(maxRadius < maxRadiusPre)
                maxRadius = maxRadiusPre;

            // top-right corner
            maxRadiusPre = std::sqrt(SDL_pow(ScreenW - focusX,  2) + SDL_pow(focusY, 2));
            if(maxRadius < maxRadiusPre)
                maxRadius = maxRadiusPre;

            // bottom-left corner
            maxRadiusPre = std::sqrt(SDL_pow(focusX, 2) + SDL_pow(ScreenH - focusY, 2));
            if(maxRadius < maxRadiusPre)
                maxRadius = maxRadiusPre;

            // bottom-right corner
            maxRadiusPre = std::sqrt(SDL_pow(ScreenW - focusX, 2) + SDL_pow(ScreenH - focusY, 2));
            if(maxRadius < maxRadiusPre)
                maxRadius = maxRadiusPre;

            int radius = maxRadius - (maxRadius * m_scale);

            frmMain.renderCircleHole(focusX, focusY, radius, color_r, color_b, color_g, 1.f);
            // left side
            frmMain.renderRect(0, 0, focusX - radius, ScreenH, color_r, color_b, color_g, 1.f, true);
            // right side
            frmMain.renderRect(focusX + radius, 0, ScreenW - (focusX + radius), ScreenH, color_r, color_b, color_g, 1.f, true);
            // Top side
            frmMain.renderRect(0, 0, ScreenW, focusY - radius + 1, color_r, color_b, color_g, 1.f, true);
            // Bottom side
            frmMain.renderRect(0, focusY + radius, ScreenW, ScreenH - (focusY + radius), color_r, color_b, color_g, 1.f, true);
        }
        break;

    case S_FLIP_H:
        if(m_scale >= 1.0f)
            frmMain.renderRect(0, 0, ScreenW, ScreenH, color_r, color_b, color_g, m_scale, true);
        else
        {
            float center = (ScreenH / 2);
            float sideHeight = SDL_ceil(center * m_scale);
            frmMain.renderRect(0, 0, ScreenW, sideHeight, color_r, color_b, color_g, 1.f, true);
            frmMain.renderRect(0, ScreenH - sideHeight, ScreenW, sideHeight, color_r, color_b, color_g, 1.f, true);
        }
        break;

    case S_FLIP_V:
        if(m_scale >= 1.0f)
            frmMain.renderRect(0, 0, ScreenW, ScreenH, color_r, color_b, color_g, m_scale, true);
        else
        {
            float center = (ScreenW / 2);
            float sideWidth = SDL_ceil(center * m_scale);
            frmMain.renderRect(0, 0, sideWidth, ScreenH, color_r, color_b, color_g, 1.f, true);
            frmMain.renderRect(ScreenW - sideWidth, 0, sideWidth, ScreenH, color_r, color_b, color_g, 1.f, true);
        }
        break;
    }

    frmMain.offsetViewportIgnore(false);
}
