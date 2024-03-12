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
#ifndef COMPAT_H
#define COMPAT_H

struct Compatibility_t
{
    bool fix_restored_block_move; // don't move powerup blocks to the right when they are hit after restoring
    bool fix_player_slope_speed;
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
    bool fix_plant_wobble; // improves visual appearance of plants
    bool fix_powerup_lava_bug; // powerups always die when hitting lava, instead of checking the types of arbitrary NPCs
    // 1.3.5
    bool fix_char5_vehicle_climb;
    bool fix_vehicle_char_switch;
    bool fix_vanilla_checkpoints;
    bool fix_autoscroll_speed;
    bool fix_submerged_splash_effect;
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
    // 1.3.6-1
    bool fix_npc_ceiling_speed; // when an NPC hits a ceiling block, it takes its SpeedY from that block, not from an arbitrary one
    bool emulate_classic_block_order; // the quadtree should return blocks in the order they had at the beginning of the level, not their current order
    bool custom_powerup_collect_score; // collected powerups give score from npc-X.txt
    bool fix_player_crush_death; // player should not be crushed by corners of slopes or by hitting a horizontally moving ceiling
    bool fix_pound_skip_warp; // ground pound state should not skip instant / portal warps
    bool fix_held_item_cancel; // do not cancel held item hitting hostile NPC that intersects with an immune NPC
    bool modern_section_change; // fix glitches and improve animations for section resize
    bool fix_frame_perfect_despawn; // if NPC comes onscreen later in the same frame it timed out, should still be able to spawn
    unsigned int bitblit_background_colour[3];
    // 1.3.6.3
    bool pound_by_alt_run; // use alt run for pound action when player is in a purple pet mount
    // 1.3.6.5
    bool fix_visual_bugs; // fix misc visual bugs from SMBX 1.3
    bool fix_npc_emerge_size; // fix size of NPC emerging from a block
    bool fix_switched_block_clipping; // don't let blocks become intangible to NPCs after coin switch
    // 1.3.7
    bool modern_npc_camera_logic; // NPCs should support more than two cameras, and consider the event logic camera when activating
    bool allow_multires;
    bool disable_background2_tiling;
    bool world_map_lvlname_marquee;
    bool modern_lives_system;

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
