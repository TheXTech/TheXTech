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
    //! Use the native onscreen keyboard instead of the TheXTech one
    bool    use_native_osk = false;
    //! Enable the in-game editor
    bool    enable_editor = true;
    //! Scroll when cursor is at edge of screen in editor
    bool    editor_edge_scroll = false;
    //! Preferred file format for editor (0 is Moondust engine lvlx format)
    int     editor_preferred_file_format = 0;

    /* ---- Video ----*/

    //! The onscreen keyboard should fill the screen
    bool    osk_fill_screen = false;

    /* ---- Gameplay ----*/

    //! Do ground-point by alt-run key instead of down
    bool    GameplayPoundByAltRun = false;
    //! Policy of stars showing at world map
    int     WorldMapStarShowPolicyGlobal = 0;
    //! Use strict Drop/Add (lose life on Add after Drop, SwapCharacter)
    bool    StrictDropAdd = false;
    //! Use the quick (non-pausing) reconnect screen
    bool    NoPauseReconnect = false;
    //! Access the Cheat Screen from the pause menu
    bool    enter_cheats_menu_item = false;
    //! Allow player quickly move through paths on the world map (works for compatibility Mode 1 only)
    bool    worldMapFastMove = false;
    //! Show number of dragon coins collected so far in level
    bool    ShowDragonCoins = false;
    //! Enter the pause menu after dying while testing a level
    bool    editor_pause_on_death = true;
    //! Internal gameplay resolution (0 means that Renderer may set on window resize)
    int     InternalW = 800;
    int     InternalH = 600;

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
