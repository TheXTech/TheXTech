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
#ifndef CONFIG_H
#define CONFIG_H

#include <string>

#include "compat.h"
#include "video.h"

extern struct Config_t : public Compatibility_t, public VideoSettings_t
{
    /* ---- Main ----*/

    //! Record gameplay data
    bool    record_gameplay_data = false;
    //! Use the native onscreen keyboard instead of the TheXTech one
    bool    use_native_osk = false;
    //! Enable the in-game editor
    static constexpr bool enable_editor = true;
    //! Preferred file format for editor (0 is Moondust engine lvlx format)
    // int     editor_preferred_file_format = 0;
    //! Choose asset pack on game startup
    bool    pick_assets_on_start = false;
    //! User's preferred language / dialect
    std::string language = "auto";

    /* ---- Video ----*/

    //! Show the episode title onscreen
    enum
    {
        EPISODE_TITLE_OFF = 0,
        EPISODE_TITLE_BOTTOM,
        EPISODE_TITLE_TOP
    };
    int show_episode_title = EPISODE_TITLE_OFF;

    /* ---- Gameplay ----*/

    enum
    {
        //* don't show medals in HUD
        MEDALS_SHOW_OFF = 0,
        //* show counts of gotten medals
        MEDALS_SHOW_GOT,
        //* show counts of gotten and available medals
        MEDALS_SHOW_COUNTS,
        //* show position of gotten and available medals
        MEDALS_SHOW_FULL,
    };

    //! Do ground-point by alt-run key instead of down
    // bool    GameplayPoundByAltRun = false;
    //! Policy of stars showing at world map
    int     WorldMapStarShowPolicyGlobal = 0;
    //! Policy of medals showing
    int     medals_show_policy = MEDALS_SHOW_OFF;
    //! Show fails counter
    bool    show_fails_counter = true;
    //! Use the quick (non-pausing) reconnect screen
    bool    NoPauseReconnect = false;
    //! Allow player quickly move through paths on the world map (works for compatibility Mode 1 only)
    bool    worldMapFastMove = false;
    //! Internal gameplay resolution (0 means that Renderer may set on window resize)
    int     InternalW = 800;
    int     InternalH = 600;
#ifdef ENABLE_XTECH_DISCORD_RPC
    //! Enable Discord Rich Presense support on supported platforms
    bool    discord_rpc = false;
#endif

    /* ---- Effects ----*/

    //! Shake screen on thwomp falling, Pet ground pount, and Set III Villain ground pound
    bool    extra_screen_shake = true;
    //! Play "player grow" together with the "get item" sound to represent original games behaviour
    // bool    SoundPlayerGrowWithGetItem = false; // removed in TheXTech v1.3.7
    //! Enables the inter-level fade effect
    bool    EnableInterLevelFade = true;
    //! Hide an event trigger NPC that is inactive but onscreen
    enum
    {
        INACTIVE_NPC_HIDE = 0,
        INACTIVE_NPC_SHADE,
        INACTIVE_NPC_SHOW,
    };
    static constexpr int render_inactive_NPC = INACTIVE_NPC_SHADE;

    //! Translate the coordinates of autocode screen-space objects based on the HUD location
    static constexpr bool autocode_translate_coords = true;
    //! Camera optimizations that improve the experience at low resolutions
    static constexpr bool small_screen_camera_features = true;

    // temporary fields replacing global flags at the main branch, in preparation to merge the config branch
    bool fullscreen = false;
    bool show_fps = false;
    bool enable_frameskip = false;
    bool unlimited_framerate = false;
    bool audio_enable = true;

    // temporary fields for new SFX settings, in preparation to merge the config branch
    bool sfx_pet_beat = true;
    bool sfx_modern = true;
    bool sfx_audio_fx = true;
    bool sfx_spatial_audio = true;

    // temporary fields for new features, in preparation to merge the config branch
    bool modern_item_drop = false;
    bool show_screen_shake = true;
    bool td_compat_mode = false;

    // temporary field for playtime counter
    enum
    {
        PLAYTIME_COUNTER_OFF = 0,
        PLAYTIME_COUNTER_SUBTLE = 1,
        PLAYTIME_COUNTER_OPAQUE,
        PLAYTIME_COUNTER_ANIMATED,
    };
    int show_playtime_counter = PLAYTIME_COUNTER_OPAQUE;

} g_config; // main_config.cpp

#endif // CONFIG_H
