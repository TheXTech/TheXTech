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
#ifndef COMPAT_H
#define COMPAT_H

struct Compatibility_t
{
    // 1.3.4
    bool enable_last_warp_hub_resume;
    bool fix_npc247_collapse;
    bool fix_platforms_acceleration;
    bool fix_player_filter_bounce;
    bool fix_player_downward_clip;
    bool fix_npc_downward_clip;
    bool fix_npc55_kick_ice_blocks;
    bool fix_climb_invisible_fences;
    bool fix_climb_bgo_speed_adding;
    bool enable_climb_bgo_layer_move;
    bool fix_player_clip_wall_at_npc;
    bool fix_skull_raft;
    bool fix_char3_escape_shell_surf;
    bool fix_keyhole_framerate;
    // 1.3.5
    bool fix_char5_vehicle_climb;
    bool fix_vehicle_char_switch;
    bool fix_vanilla_checkpoints;
    bool fix_autoscroll_speed;
    // 1.3.5.1
    bool fix_squid_stomp_effect;
    bool fix_special_coin_switch;
    // 1.3.5.2
    bool fix_bat_start_while_inactive;
    bool fix_FreezeNPCs_no_reset;
    enum StarsShowPolicy_t
    {
        //! Prefer global settings
        STARS_UNSPECIFIED = -1,
        //! Never show stars counter
        STARS_DONT_SHOW = 0,
        //! Show collected only
        STARS_SHOW_COLLECTED_ONLY = 1,
        //! Show collected and available
        STARS_SHOW_COLLECTED_AND_AVAILABLE = 2
    };
    int world_map_stars_show_policy;
    // 1.3.5.3
    // bool require_ground_to_enter_warps; // REMOVED SINCE 1.3.6
    bool fix_npc_activation_event_loop_bug;
    enum SfxPlayerGrowWithGetItem_t
    {
        SPGWGI_UNSPECIFIED = 0,
        SPGWGI_ENABLE,
        SPGWGI_DISABLE
    };
    int sfx_player_grow_with_got_item;
    // 1.3.6
    bool pause_on_disconnect;
    bool allow_drop_add;
    bool multiplayer_pause_controls;
    bool fix_fairy_stuck_in_pipe;
    bool world_map_fast_move;
    bool fix_flamethrower_gravity;

    // SpeedRun section
    enum
    {
        SPEEDRUN_STOP_NONE = 0,
        SPEEDRUN_STOP_EVENT,
        SPEEDRUN_STOP_LEAVE_LEVEL,
        SPEEDRUN_STOP_ENTER_LEVEL
    };
    int speedrun_stop_timer_by;
    char speedrun_stop_timer_at[250];
    int speedrun_blink_effect;

    // Deaths counter
    bool demos_counter_enable;
    char demos_counter_title[250];

    // Luna
    enum LunaEnableEngine_t
    {
        LUNA_ENGINE_UNSPECIFIED = 0,
        LUNA_ENGINE_ENABLE,
        LUNA_ENGINE_DISABLE
    };
    LunaEnableEngine_t luna_enable_engine;
    bool luna_allow_level_codes;
};

extern Compatibility_t g_compatibility;

void LoadCustomCompat();
void ResetCompat();

/**
 * @brief The level of the enforced compatibility
 */
enum CompatibilityLevel
{
    //! No specific compatibiltiy will be enforced, all modern features will be enabled and tuned individually by compat.ini or settings menu
    COMPAT_MODERN = 0,
    //! Enables all bugs except these was been fixed at the SMBX2 project
    COMPAT_SMBX2,
    //! Enforces the full compatibility to the SMBX 1.3 engine and disables almost all new features and bugfixes that leads the gameplay difference
    COMPAT_SMBX13
};

void CompatSetEnforcedLevel(int level);
int  CompatGetLevel();

#endif // COMPAT_H
