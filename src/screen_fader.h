/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2025 Vitaly Novichkov <admin@wohlnet.ru>
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

    // each with a denominator of 65
    int m_current_fade = 0;
    int m_target_fade = 0;
    int m_step = 0;

    XTColor color = {0, 0, 0};

    // Focus on the point (using circle or rectangular effect)
    int m_focusUniform = -1;
    int m_focusX = -1;
    int m_focusY = -1;
    int m_focusScreen = -1;
    bool m_focusSet = false;

    num_t *m_focusTrackX = nullptr;
    num_t *m_focusTrackY = nullptr;
    num_t m_focusOffsetX = 0.0_n;
    num_t m_focusOffsetY = 0.0_n;

    int m_shape = S_FADE;

    static void clearTransitEffects();
    static int loadTransitEffect(const std::string& name);

    void clearFader();

    void setupFader(int step, int start, int goal, int shape, bool useFocus = false, int focusX = -1, int focusY = -1, int screen = -1);
    void setTrackedFocus(num_t *x, num_t *y, num_t offX, num_t offY);

    bool isComplete();
    bool isVisible();
    bool isFadingIn();
    bool isFadingOut();

    void update();

    // fullscreen should be false if the fader is being drawn inside a vScreen, otherwise true
    void draw(bool fullscreen = true);
};

#endif // SCREEN_FADER_H
