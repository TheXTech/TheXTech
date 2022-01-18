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
#ifndef CONFIG_H
#define CONFIG_H

extern struct Config_t
{
    /* ---- Main ----*/

    //! Record gameplay data
    bool    RecordGameplayData = false;


    /* ---- Gameplay ----*/

    //! Do ground-point by alt-run key instead of down
    bool    GameplayPoundByAltRun = false;
    //! Policy of stars showing at world map
    int     WorldMapStarShowPolicyGlobal = 0;


    /* ---- Effects ----*/

    //! Shake screen on thwomp falling
    bool    GameplayShakeScreenThwomp = true;
    //! Shake screen on Yoshi ground pount
    bool    GameplayShakeScreenPound = true;
    //! Shake screen on Bowser III'rd ground pound
    bool    GameplayShakeScreenBowserIIIrd = true;
    //! Play "player grow" together with the "get item" sound to represent original games behaviour
    bool    SoundPlayerGrowWithGetItem = false;
    //! Enables the inter-level fade effect
    bool    EnableInterLevelFade = true;


    /* ---- Joystick ----*/

    //! Enable usage of the rumble control
    bool    JoystickEnableRumble = true;
    //! Show the battery status for wireless gamepads (if possible)
    bool    JoystickEnableBatteryStatus = true;


} g_config; // main_config.cpp

#endif // CONFIG_H
