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

#pragma once
#ifndef SCREEN_FADER_H
#define SCREEN_FADER_H

#include <string>

#include "fader.h"

struct ScreenFader
{
    enum Shape
    {
        S_FADE = 0,
        S_RECT,
        S_CIRCLE,
        S_FLIP_H,
        S_FLIP_V,
        // must be higher than all LevelDoor::TRANSIT_*
        S_CUSTOM = 16,
    };

    bool m_active = false;
    bool m_full = false;
    bool m_complete = false;
    bool m_dirUp = false;
    PGE_Fader m_fader;
    float m_scale = 0.0f;

    XTColor color = {0, 0, 0};

    // Focus on the point (using circle or rectangular effect)
    int m_focusUniform = -1;
    int m_focusX = -1;
    int m_focusY = -1;
    int m_focusScreen = -1;
    bool m_focusSet = false;

    double *m_focusTrackX = nullptr;
    double *m_focusTrackY = nullptr;
    double m_focusOffsetX = 0.0;
    double m_focusOffsetY = 0.0;

    int m_shape = S_FADE;

    static void clearTransitEffects();
    static int loadTransitEffect(const std::string& name);

    void clearFader();

    void setupFader(int step, int start, int goal, int shape, bool useFocus = false, int focusX = -1, int focusY = -1, int screen = -1);
    void setTrackedFocus(double *x, double *y, double offX, double offY);

    bool isComplete();
    bool isVisible();
    bool isFadingIn();
    bool isFadingOut();

    void update();

    // fullscreen should be false if the fader is being drawn inside a vScreen, otherwise true
    void draw(bool fullscreen = true);
};

#endif // SCREEN_FADER_H
