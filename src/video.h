/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2022 Vitaly Novichkov <admin@wohlnet.ru>
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
#ifndef VIDEO_H
#define VIDEO_H

#include <string>
#include <unordered_map>

enum RenderMode_t
{
    RENDER_AUTO = -1,
    RENDER_SOFTWARE = 0,
    RENDER_ACCELERATED,
    RENDER_ACCELERATED_VSYNC
};

enum BatteryStatus_t
{
    BATTERY_STATUS_OFF = 0,
    BATTERY_STATUS_FULLSCREEN_WHEN_LOW,
    BATTERY_STATUS_ANY_WHEN_LOW,
    BATTERY_STATUS_FULLSCREEN_ON,
    BATTERY_STATUS_ALWAYS_ON,
};

enum ScaleModes
{
    SCALE_DYNAMIC_INTEGER = -3,
    SCALE_DYNAMIC_NEAREST = -2,
    SCALE_DYNAMIC_LINEAR = -1,
    SCALE_FIXED_05X = 0,
    SCALE_FIXED_1X = 1,
    SCALE_FIXED_2X = 2,
};

static const std::unordered_map<int, std::string> ScaleMode_strings =
{
    {SCALE_DYNAMIC_INTEGER, "integer"},
    {SCALE_DYNAMIC_NEAREST, "nearest"},
    {SCALE_DYNAMIC_LINEAR, "linear"},
    {SCALE_FIXED_05X, "0.5x"},
    {SCALE_FIXED_1X, "1x"},
    {SCALE_FIXED_2X, "2x"},
};

extern struct VideoSettings_t
{
    enum ScaleDownTextures
    {
        SCALE_NONE = 0,
        SCALE_SAFE = 1,
        SCALE_ALL = 2,
    };

    //! Render mode
    int    renderMode = RENDER_ACCELERATED;
    //! The currently running render mode
    int    renderModeObtained = RENDER_AUTO;
    //! Render scaling mode
    int    scaleMode = SCALE_DYNAMIC_NEAREST;
    //! Device battery status indicator
    int    batteryStatus = BATTERY_STATUS_OFF;
    //! Allow game to work when window is not active
    bool   allowBgWork = false;
    //! Allow background input handling for game controllers
    bool   allowBgControllerInput = false;
    //! Enable frameskip
    bool   enableFrameSkip = true;
    //! Show FPS counter
    bool   showFrameRate = false;
    //! 2x scale down all textures to reduce the memory usage
    int    scaleDownTextures = SCALE_NONE;
} g_videoSettings; // main_config.cpp

#endif // VIDEO_H
