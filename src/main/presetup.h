/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2023 Vitaly Novichkov <admin@wohlnet.ru>
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
#ifndef PRESETUP_H
#define PRESETUP_H

// Game settings loaded on initial startup and used to manipulate options
// controllable via command line arguments

enum
{
    SPEEDRUN_EFFECT_BLINK_UNDEFINED = 0,
    SPEEDRUN_EFFECT_BLINK_OPAQUEONLY,
    SPEEDRUN_EFFECT_BLINK_ALWAYS,
    SPEEDRUN_EFFECT_BLINK_NEVER
};

extern struct PreSetup_t
{
    int     speedRunMode = 0;
    bool    speedRunSemiTransparentTimer = false;
    int     speedRunEffectBlink = SPEEDRUN_EFFECT_BLINK_UNDEFINED;

    int     compatibilityMode = 0;
} g_preSetup; // main_config.cpp


#endif // PRESETUP_H

