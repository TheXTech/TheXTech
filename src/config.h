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
    //! Show debug string during the loading process
    bool    loading_show_debug = false;

    /* ---- Video ----*/

    //! The onscreen keyboard should fill the screen
    bool    osk_fill_screen = false;
    //! Show the episode title onscreen
    enum
    {
        EPISODE_TITLE_OFF = 0,
        EPISODE_TITLE_ON,
        EPISODE_TITLE_TRANSPARENT,
        EPISODE_TITLE_AUTO,
        EPISODE_TITLE_ON_ALWAYS,
        EPISODE_TITLE_TRANSPARENT_ALWAYS,
    };
    int show_episode_title = EPISODE_TITLE_OFF;
    //! Show a backdrop texture when level is smaller than screen
    bool    show_backdrop = true;

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
    //! HUD follows player onscreen (useful for levels much larger than the intended screen size)
    bool    hud_follows_player = false;

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
    //! Hide an event trigger NPC that is inactive but onscreen
    enum
    {
        INACTIVE_NPC_HIDE = 0,
        INACTIVE_NPC_SHADE,
        INACTIVE_NPC_SHOW,
    };
    int render_inactive_NPC = INACTIVE_NPC_SHADE;
    //! Allow the view to grow beyond 800x600 in the world map (nearly duplicates Compatibility_t::free_world_res),
    //  but does not enforce at least 800x600 render resolution when off.
    bool world_map_expand_view = false;
    //! Show fog on canonically hidden portions of the world map
    enum
    {
        WORLD_MAP_FOG_OFF = 0,
        WORLD_MAP_FOG_NORMAL,
        WORLD_MAP_FOG_SPOILERS,
    };
    int world_map_fog = WORLD_MAP_FOG_OFF;
    //! Intelligently pan the world map to focus on the path
    bool world_map_smart_pan = false;
    //! Translate the coordinates of autocode screen-space objects based on the HUD location
    bool autocode_translate_coords = true;
    //! Camera optimizations that improve the experience at low resolutions
    bool small_screen_camera_features = false;


    /* ---- Joystick ----*/

    //! Enable usage of the rumble control. Only affects new controller profiles.
    bool    JoystickEnableRumble = true;
    //! Show the battery status for wireless gamepads (if possible). Only affects new controller profiles.
    bool    JoystickEnableBatteryStatus = true;


} g_config; // main_config.cpp

#endif // CONFIG_H
