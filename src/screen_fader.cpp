/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2024 Vitaly Novichkov <admin@wohlnet.ru>
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
#include <algorithm>

#include "sdl_proxy/sdl_stdinc.h"

#include "global_constants.h"
#include "globals.h" // vScreen
#include "range_arr.hpp"

#include "core/render.h"

#include "screen_fader.h"


#ifdef THEXTECH_BUILD_GL_MODERN
#    include <vector>
#    include "core/opengl/gl_program_bank.h"

static std::vector<LoadedGLProgramRef_t> s_loaded_effects;
#endif // #ifdef THEXTECH_BUILD_GL_MODERN


void ScreenFader::clearTransitEffects()
{
#ifdef THEXTECH_BUILD_GL_MODERN
    s_loaded_effects.clear();
#endif
}

int ScreenFader::loadTransitEffect(const std::string& name)
{
#ifdef THEXTECH_BUILD_GL_MODERN
    LoadedGLProgramRef_t effect = ResolveGLProgram("transit-" + name);

    if((int)effect < 0)
        return S_FADE;

    auto it = std::find(s_loaded_effects.begin(), s_loaded_effects.end(), effect);

    if(it != s_loaded_effects.end())
        return static_cast<int>(it - s_loaded_effects.begin()) + S_CUSTOM;

    s_loaded_effects.push_back(effect);

    return static_cast<int>(s_loaded_effects.size() - 1) + S_CUSTOM;
#else
    UNUSED(name);
    return S_FADE;
#endif
}

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

void ScreenFader::setupFader(int step, int start, int goal, int shape, bool useFocus, int focusX, int focusY, int screen)
{
    m_shape = shape;
    m_fader.setRatio(start / 65.0);
    m_fader.setFade(15, goal / 65.0, step / 65.0);
    m_scale = (float)m_fader.fadeRatio();
    m_dirUp = start < goal;
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

#ifdef THEXTECH_BUILD_GL_MODERN
    if(m_shape >= S_CUSTOM && m_shape - S_CUSTOM < (int)s_loaded_effects.size() && s_loaded_effects[m_shape - S_CUSTOM]->get())
        m_focusUniform = XRender::registerUniform(*s_loaded_effects[m_shape - S_CUSTOM]->get(), "u_focus");
    else
        m_focusUniform = -1;
#endif
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

bool ScreenFader::isFadingIn()
{
    return m_active && m_dirUp;
}

bool ScreenFader::isFadingOut()
{
    return m_active && !m_dirUp;
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

void ScreenFader::draw(bool fullscreen)
{
    if(!m_active)
        return;

    XRender::offsetViewportIgnore(true);

    int drawW = XRender::TargetW;
    int drawH = XRender::TargetH;

    if(m_focusScreen > 0 && m_focusScreen <= c_vScreenCount && !fullscreen)
    {
        drawW = vScreen[m_focusScreen].Width;
        drawH = vScreen[m_focusScreen].Height;
    }

    int focusX = m_focusSet ? m_focusX : (drawW / 2);
    int focusY = m_focusSet ? m_focusY : (drawH / 2);

    if(m_focusSet && m_focusScreen > 0 && m_focusScreen <= c_vScreenCount)
    {
        focusX += vScreen[m_focusScreen].X;
        focusY += vScreen[m_focusScreen].Y;

        if(fullscreen)
        {
            focusX += vScreen[m_focusScreen].TargetX();
            focusY += vScreen[m_focusScreen].TargetY();
        }
    }

    switch(m_shape)
    {
    default:
#ifdef THEXTECH_BUILD_GL_MODERN
        if(XRender::userShadersSupported() && m_shape >= S_CUSTOM && m_shape - S_CUSTOM < (int)s_loaded_effects.size() && s_loaded_effects[m_shape - S_CUSTOM]->get())
        {
            StdPicture& effect = *s_loaded_effects[m_shape - S_CUSTOM]->get();

            if(m_focusUniform >= 0)
                XRender::assignUniform(effect, m_focusUniform, UniformValue_t((GLfloat)focusX, (GLfloat)focusY));

            XRender::renderTextureScale(0, 0, drawW, drawH, effect, XTAlphaF(m_scale));

            // if catastrophic failure, fallback to normal fader
            if(effect.inited)
                break;
        }
#endif // #ifdef THEXTECH_BUILD_GL_MODERN

    // fallthrough
    case S_FADE:
        XRender::renderRect(0, 0, drawW, drawH, color.with_alphaF(m_scale), true);
        break;

    case S_RECT:
        if(m_scale >= 1.0f)
            XRender::renderRect(0, 0, drawW, drawH, color.with_alphaF(m_scale), true);
        else
        {
            float rightW = (drawW - focusX),
                    bottomH = (drawH - focusY),
                    leftW = focusX * m_scale, // left side
                    topY = focusY * m_scale, // top side
                    rightX = drawW - SDL_ceil(rightW * m_scale) + 1, // right side
                    bottomY = drawH - SDL_ceil(bottomH * m_scale) + 1; // bottom side

            // Left side
            XRender::renderRect(0, 0, leftW, drawH, color, true);
            // right side
            XRender::renderRect(rightX, 0, rightW * m_scale, drawH, color, true);
            // Top side
            XRender::renderRect(0, 0, drawW, topY, color, true);
            // Bottom side
            XRender::renderRect(0, bottomY, drawW, bottomH * m_scale, color, true);
        }
        break;

    case S_CIRCLE:
        if(m_scale >= 1.0f)
            XRender::renderRect(0, 0, drawW, drawH, color.with_alphaF(m_scale), true);
        else
        {
            // int radius = drawH - (drawH * m_scale);
            int maxRadius = 0, maxRadiusPre;

            // top-left corner
            maxRadiusPre = std::sqrt(SDL_pow(focusX, 2) + SDL_pow(focusY, 2));
            if(maxRadius < maxRadiusPre)
                maxRadius = maxRadiusPre;

            // top-right corner
            maxRadiusPre = std::sqrt(SDL_pow(drawW - focusX,  2) + SDL_pow(focusY, 2));
            if(maxRadius < maxRadiusPre)
                maxRadius = maxRadiusPre;

            // bottom-left corner
            maxRadiusPre = std::sqrt(SDL_pow(focusX, 2) + SDL_pow(drawH - focusY, 2));
            if(maxRadius < maxRadiusPre)
                maxRadius = maxRadiusPre;

            // bottom-right corner
            maxRadiusPre = std::sqrt(SDL_pow(drawW - focusX, 2) + SDL_pow(drawH - focusY, 2));
            if(maxRadius < maxRadiusPre)
                maxRadius = maxRadiusPre;

            int radius = maxRadius - (maxRadius * m_scale);

            XRender::renderCircleHole(focusX, focusY, radius, color);
            // left side
            XRender::renderRect(0, 0, focusX - radius, drawH, color, true);
            // right side
            XRender::renderRect(focusX + radius, 0, drawW - (focusX + radius), drawH, color, true);
            // Top side
            XRender::renderRect(0, 0, drawW, focusY - radius + 1, color, true);
            // Bottom side
            XRender::renderRect(0, focusY + radius, drawW, drawH - (focusY + radius), color, true);
        }
        break;

    case S_FLIP_H:
        if(m_scale >= 1.0f)
            XRender::renderRect(0, 0, drawW, drawH, color.with_alphaF(m_scale), true);
        else
        {
            float center = (drawH / 2);
            float sideHeight = SDL_ceil(center * m_scale);
            XRender::renderRect(0, 0, drawW, sideHeight, color, true);
            XRender::renderRect(0, drawH - sideHeight, drawW, sideHeight, color, true);
        }
        break;

    case S_FLIP_V:
        if(m_scale >= 1.0f)
            XRender::renderRect(0, 0, drawW, drawH, color.with_alphaF(m_scale), true);
        else
        {
            float center = (drawW / 2);
            float sideWidth = SDL_ceil(center * m_scale);
            XRender::renderRect(0, 0, sideWidth, drawH, color, true);
            XRender::renderRect(drawW - sideWidth, 0, sideWidth, drawH, color, true);
        }
        break;
    }

    XRender::offsetViewportIgnore(false);
}
