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

#include "fader.h"


struct ScreenFader
{
    bool m_active = false;
    bool m_full = false;
    PGE_Fader m_fader;
    float m_scale = 0.0f;

    float color_r = 0.f;
    float color_g = 0.f;
    float color_b = 0.f;

    // Focus on the point (using circle or rectangular effect)
    int m_focusX = -1;
    int m_focusY = -1;
    int m_focusScreen = -1;
    bool m_focusSet = false;

    double *m_focusTrackX = nullptr;
    double *m_focusTrackY = nullptr;
    double m_focusOffsetX = 0.0;
    double m_focusOffsetY = 0.0;

    enum Shape
    {
        S_FADE = 0,
        S_RECT,
        S_CIRCLE,
        S_FLIP_H,
        S_FLIP_V
    } m_shape = S_FADE;

    void clearFader();

    void setupFader(int step, int start, int goal, Shape shape, bool useFocus = false, int focusX = -1, int focusY = -1, int screen = -1);
    void setTrackedFocus(double *x, double *y, double offX, double offY);

    void update();

    void draw();
};

#endif // SCREEN_FADER_H
