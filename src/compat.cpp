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

#include <Logger/logger.h>
#include <IniProcessor/ini_processing.h>
#include <Utils/files.h>
#include <Utils/dir_list_ci.h>
#include "Graphics/graphics_funcs.h"
#include "globals.h"
#include "global_dirs.h"
#include "compat.h"
#include "main/speedrunner.h"
#include "main/presetup.h"

#include "core/sdl.h"


static int s_compatLevel = COMPAT_MODERN;

Compatibility_t g_compatibility;

static void compatInit(Compatibility_t &c)
{
    if(g_speedRunnerMode != SPEEDRUN_MODE_OFF)
    {
        switch(g_speedRunnerMode)
        {
        default:
        case SPEEDRUN_MODE_1:
            CompatSetEnforcedLevel(COMPAT_MODERN);
            break;
        case SPEEDRUN_MODE_2:
            CompatSetEnforcedLevel(COMPAT_SMBX2);
            break;
        case SPEEDRUN_MODE_3:
            CompatSetEnforcedLevel(COMPAT_SMBX13);
            break;
        }
    }

    // 1.3.4
    c.enable_last_warp_hub_resume = true;
    c.fix_platforms_acceleration = true;
    c.fix_npc247_collapse = true;
    c.fix_player_filter_bounce = true;
    c.fix_player_downward_clip = true;
    c.fix_npc_downward_clip = true;
    c.fix_npc55_kick_ice_blocks = false;
    c.fix_climb_invisible_fences = true;
    c.fix_climb_bgo_speed_adding = true;
    c.enable_climb_bgo_layer_move = true;
    c.fix_player_clip_wall_at_npc = true;
    c.fix_skull_raft = true;
    c.fix_char3_escape_shell_surf = true;
    c.fix_keyhole_framerate = true;
    // 1.3.5
    c.fix_char5_vehicle_climb = true;
    c.fix_vehicle_char_switch = true;
    c.fix_vanilla_checkpoints = true;
    c.fix_autoscroll_speed = false;
    // 1.3.5.1
    c.fix_squid_stomp_effect = true;
    c.fix_special_coin_switch = true;
    // 1.3.5.2
    c.fix_bat_start_while_inactive = true;
    c.fix_FreezeNPCs_no_reset = false;
    c.world_map_stars_show_policy = Compatibility_t::STARS_UNSPECIFIED;
    // 1.3.5.3
    // c.require_ground_to_enter_warps = false; // REMOVED SINCE 1.3.6
    c.fix_npc_activation_event_loop_bug = true;
    c.sfx_player_grow_with_got_item = Compatibility_t::SPGWGI_UNSPECIFIED;
    // 1.3.6
    c.pause_on_disconnect = true;
    c.allow_drop_add = true;
    c.multiplayer_pause_controls = true;
    c.demos_counter_enable = false;
    SDL_strlcpy(c.demos_counter_title, "", sizeof(c.demos_counter_title));
    c.luna_allow_level_codes = false;
    c.luna_enable_engine = Compatibility_t::LUNA_ENGINE_UNSPECIFIED;
    c.fix_fairy_stuck_in_pipe = true;
    c.world_map_fast_move = false;
    c.fix_flamethrower_gravity = true;
    // 1.3.6-1
    c.fix_npc_ceiling_speed = true;
    c.emulate_classic_block_order = false;
    c.bitblit_background_colour[0] = 0;
    c.bitblit_background_colour[1] = 0;
    c.bitblit_background_colour[2] = 0;


    if(s_compatLevel >= COMPAT_SMBX2) // Make sure that bugs were same as on SMBX2 Beta 4 on this moment
    {
        c.enable_last_warp_hub_resume = false;
        c.fix_platforms_acceleration = false;
        c.fix_npc247_collapse = false;
        c.fix_npc_downward_clip = false;
        c.fix_npc55_kick_ice_blocks = false; //-V1048
        c.fix_climb_invisible_fences = false;
        c.fix_climb_bgo_speed_adding = false;
        c.enable_climb_bgo_layer_move = false;
        c.fix_skull_raft = false;
        c.fix_char3_escape_shell_surf = false;
        // 1.3.5
        c.fix_keyhole_framerate = false;
        c.fix_char5_vehicle_climb = false;
        c.fix_vehicle_char_switch = false;
        c.fix_vanilla_checkpoints = false;
        c.fix_autoscroll_speed = false; //-V1048
        // 1.3.5.1
        c.fix_squid_stomp_effect = false;
        c.fix_special_coin_switch = false;
        // 1.3.5.2
        c.fix_bat_start_while_inactive = false;
        c.fix_FreezeNPCs_no_reset = false; //-V1048
        // 1.3.5.3
        c.fix_npc_activation_event_loop_bug = false;
        // 1.3.6
        c.pause_on_disconnect = false;
        c.allow_drop_add = false;
        c.multiplayer_pause_controls = false;
        c.fix_fairy_stuck_in_pipe = false;
        c.fix_flamethrower_gravity = false;
        // 1.3.6-1
        c.fix_npc_ceiling_speed = false;
        c.emulate_classic_block_order = true;
    }

    if(s_compatLevel >= COMPAT_SMBX13) // Strict vanilla SMBX
    {
        // 1.3.4
        c.fix_player_filter_bounce = false;
        c.fix_player_downward_clip = false;
        c.fix_player_clip_wall_at_npc = false;
        // 1.3.6
    }

    c.speedrun_stop_timer_by = Compatibility_t::SPEEDRUN_STOP_NONE;
    SDL_memset(c.speedrun_stop_timer_at, 0, sizeof(c.speedrun_stop_timer_at));
    SDL_strlcpy(c.speedrun_stop_timer_at, "Boss Dead", sizeof(c.speedrun_stop_timer_at));
    c.speedrun_blink_effect = SPEEDRUN_EFFECT_BLINK_OPAQUEONLY;
}

static void deprecatedWarning(IniProcessing &s, const char* fieldName, const char *newName)
{
    if(s.hasKey(fieldName))
    {
        pLogWarning("File %s contains the deprecated setting \"%s\" at the section [%s]. Please rename it into \"%s\".",
                    s.fileName().c_str(),
                    fieldName,
                    s.group().c_str(),
                    newName);
    }
}

static void loadCompatIni(Compatibility_t &c, const std::string &fileName)
{
    pLogDebug("Loading %s...", fileName.c_str());

    IniProcessing compat(fileName);
    if(!compat.isOpened())
    {
        pLogWarning("Can't open the compat.ini file: %s", fileName.c_str());
        return;
    }

    compat.beginGroup("speedrun");
    {
        std::string buffer;
        const IniProcessing::StrEnumMap stopBy
        {
            {"none", Compatibility_t::SPEEDRUN_STOP_NONE},
            {"event", Compatibility_t::SPEEDRUN_STOP_EVENT},
            {"leave", Compatibility_t::SPEEDRUN_STOP_LEAVE_LEVEL},
            {"enter", Compatibility_t::SPEEDRUN_STOP_ENTER_LEVEL}
        };

        const IniProcessing::StrEnumMap speedRunBlinkMode =
        {
            {"opaque", SPEEDRUN_EFFECT_BLINK_OPAQUEONLY},
            {"always", SPEEDRUN_EFFECT_BLINK_ALWAYS},
            {"true", SPEEDRUN_EFFECT_BLINK_ALWAYS},
            {"never", SPEEDRUN_EFFECT_BLINK_NEVER},
            {"false", SPEEDRUN_EFFECT_BLINK_NEVER}
        };

        compat.readEnum("stop-timer-by", c.speedrun_stop_timer_by, c.speedrun_stop_timer_by, stopBy);
        compat.read("stop-timer-at", buffer, std::string(c.speedrun_stop_timer_at));
        SDL_strlcpy(c.speedrun_stop_timer_at, buffer.c_str(), sizeof(c.speedrun_stop_timer_at));
        compat.readEnum("blink-effect", c.speedrun_blink_effect, c.speedrun_blink_effect, speedRunBlinkMode);
    }
    compat.endGroup();

    compat.beginGroup("effects");
    {
        const IniProcessing::StrEnumMap spgwgi
        {
            {"unspecified", Compatibility_t::SPGWGI_UNSPECIFIED},
            {"enable", Compatibility_t::SPGWGI_ENABLE},
            {"true", Compatibility_t::SPGWGI_ENABLE},
            {"disable", Compatibility_t::SPGWGI_DISABLE},
            {"false", Compatibility_t::SPGWGI_DISABLE}
        };
        compat.readEnum("sfx-player-grow-with-got-item", c.sfx_player_grow_with_got_item, c.sfx_player_grow_with_got_item, spgwgi);
    }
    compat.endGroup();

    if(!compat.beginGroup("fails-counter"))
        compat.beginGroup("death-counter"); // Backup fallback
    {
        std::string buffer;
        compat.read("enabled", c.demos_counter_enable, c.demos_counter_enable);
        compat.read("title", buffer, std::string(c.demos_counter_title));
        SDL_strlcpy(c.demos_counter_title, buffer.c_str(), sizeof(c.demos_counter_title));
    }
    compat.endGroup();

    compat.beginGroup("luna-script");
    {
        const IniProcessing::StrEnumMap lunaEnable
        {
            {"unspecified", Compatibility_t::LUNA_ENGINE_UNSPECIFIED},
            {"enable", Compatibility_t::LUNA_ENGINE_ENABLE},
            {"true", Compatibility_t::LUNA_ENGINE_ENABLE},
            {"disable", Compatibility_t::LUNA_ENGINE_DISABLE},
            {"false", Compatibility_t::LUNA_ENGINE_DISABLE}
        };
        compat.readEnum("enable-engine", c.luna_enable_engine, c.luna_enable_engine, lunaEnable);
        compat.read("allow-level-codes", c.luna_allow_level_codes, c.luna_allow_level_codes);
    }
    compat.endGroup();

#ifdef DEBUG_BUILD // FIXME: Don't enable this at release builds until a specific moment
    compat.beginGroup("bitblit-bg-color");
    compat.read("red", c.bitblit_background_colour[0], c.bitblit_background_colour[0]);
    compat.read("green", c.bitblit_background_colour[1], c.bitblit_background_colour[1]);
    compat.read("blue", c.bitblit_background_colour[2], c.bitblit_background_colour[2]);
    compat.endGroup();
#endif

    if(s_compatLevel >= COMPAT_SMBX13)
    {
        if(g_speedRunnerMode >= SPEEDRUN_MODE_3)
            pLogDebug("Speed-Run Mode 3 detected, the [compatibility] section for the compat.ini completely skipped, all old bugs enforced.", fileName.c_str());
        return;
    }

    compat.beginGroup("compatibility");
    if(s_compatLevel < COMPAT_SMBX2) // Ignore options are still not been fixed at the SMBX2
    {
        // 1.3.4
        compat.read("enable-last-warp-hub-resume", c.enable_last_warp_hub_resume, c.enable_last_warp_hub_resume);
        compat.read("fix-platform-acceleration", c.fix_platforms_acceleration, c.fix_platforms_acceleration);
        compat.read("fix-pokey-collapse", c.fix_npc247_collapse, c.fix_npc247_collapse);// DEPRECATED since 1.3.6
        deprecatedWarning(compat, "fix-pokey-collapse", "fix-npc247-collapse");
        compat.read("fix-npc55-kick-ice-blocks", c.fix_npc55_kick_ice_blocks, c.fix_npc55_kick_ice_blocks);
        compat.read("fix-climb-invisible-fences", c.fix_climb_invisible_fences, c.fix_climb_invisible_fences);
        compat.read("fix-climb-bgo-speed-adding", c.fix_climb_bgo_speed_adding, c.fix_climb_bgo_speed_adding);
        compat.read("enable-climb-bgo-layer-move", c.enable_climb_bgo_layer_move, c.enable_climb_bgo_layer_move);
        compat.read("fix-player-clip-wall-at-npc", c.fix_player_clip_wall_at_npc, c.fix_player_clip_wall_at_npc);
        compat.read("fix-skull-raft", c.fix_skull_raft, c.fix_skull_raft);
        compat.read("fix-peach-escape-shell-surf", c.fix_char3_escape_shell_surf, c.fix_char3_escape_shell_surf);// DEPRECATED since 1.3.6
        deprecatedWarning(compat, "fix-peach-escape-shell-surf", "fix-char3-escape-shell-surf");
        compat.read("fix-keyhole-framerate", c.fix_keyhole_framerate, c.fix_keyhole_framerate);
        // 1.3.5
        compat.read("fix-link-clowncar-fairy", c.fix_char5_vehicle_climb, c.fix_char5_vehicle_climb);// DEPRECATED since 1.3.6
        deprecatedWarning(compat, "fix-link-clowncar-fairy", "fix-char5-vehicle-climb");
        compat.read("fix-dont-switch-player-by-clowncar", c.fix_vehicle_char_switch, c.fix_vehicle_char_switch);// DEPRECATED since 1.3.6
        deprecatedWarning(compat, "fix-dont-switch-player-by-clowncar", "fix-vehicle-char-switch");
        compat.read("enable-multipoints", c.fix_vanilla_checkpoints, c.fix_vanilla_checkpoints);// DEPRECATED since 1.3.6
        deprecatedWarning(compat, "enable-multipoints", "fix-vanilla-checkpoints");
        compat.read("fix-autoscroll-speed", c.fix_autoscroll_speed, c.fix_autoscroll_speed);
        // 1.3.5.1
        compat.read("fix-blooper-stomp-effect", c.fix_squid_stomp_effect, c.fix_squid_stomp_effect);// DEPRECATED since 1.3.6
        deprecatedWarning(compat, "fix-blooper-stomp-effect", "fix-squid-stomp-effect");
        compat.read("fix-pswitch-dragon-coin", c.fix_special_coin_switch, c.fix_special_coin_switch);// DEPRECATED since 1.3.6
        deprecatedWarning(compat, "fix-pswitch-dragon-coin", "fix-special-coin-switch");
        // 1.3.5.2
        compat.read("fix-swooper-start-while-inactive", c.fix_bat_start_while_inactive, c.fix_bat_start_while_inactive);// DEPRECATED since 1.3.6
        deprecatedWarning(compat, "fix-swooper-start-while-inactive", "fix-bat-start-while-inactive");
        compat.read("fix-FreezeNPCs-no-reset", c.fix_FreezeNPCs_no_reset, c.fix_FreezeNPCs_no_reset);
        IniProcessing::StrEnumMap starsShowPolicy
        {
            {"unspecified", Compatibility_t::STARS_UNSPECIFIED},
            {"hide", Compatibility_t::STARS_DONT_SHOW},
            {"show-collected", Compatibility_t::STARS_SHOW_COLLECTED_ONLY},
            {"show", Compatibility_t::STARS_SHOW_COLLECTED_AND_AVAILABLE}
        };
        compat.readEnum("world-map-stars-show-policy", c.world_map_stars_show_policy, c.world_map_stars_show_policy, starsShowPolicy);
        // 1.3.6
        compat.read("pause-on-disconnect", c.pause_on_disconnect, c.pause_on_disconnect);
        compat.read("allow-drop-add", c.allow_drop_add, c.allow_drop_add);
        compat.read("multiplayer-pause-controls", c.multiplayer_pause_controls, c.multiplayer_pause_controls);
        compat.read("fix-fairy-stuck-in-pipe", c.fix_fairy_stuck_in_pipe, c.fix_fairy_stuck_in_pipe);
        // New names for old fields
        compat.read("fix-vanilla-checkpoints", c.fix_vanilla_checkpoints, c.fix_vanilla_checkpoints);
        compat.read("fix-squid-stomp-effect", c.fix_squid_stomp_effect, c.fix_squid_stomp_effect);
        compat.read("fix-char3-escape-shell-surf", c.fix_char3_escape_shell_surf, c.fix_char3_escape_shell_surf);
        compat.read("fix-char5-vehicle-climb", c.fix_char5_vehicle_climb, c.fix_char5_vehicle_climb);
        compat.read("fix-vehicle-char-switch", c.fix_vehicle_char_switch, c.fix_vehicle_char_switch);
        compat.read("fix-npc247-collapse", c.fix_npc247_collapse, c.fix_npc247_collapse);
        compat.read("fix-special-coin-switch", c.fix_special_coin_switch, c.fix_special_coin_switch);
        compat.read("fix-bat-start-while-inactive", c.fix_bat_start_while_inactive, c.fix_bat_start_while_inactive);
        // 1.3.6-1
        compat.read("fix-npc-ceiling-speed", c.fix_npc_ceiling_speed, c.fix_npc_ceiling_speed);
        compat.read("emulate-classic-block-order", c.emulate_classic_block_order, c.emulate_classic_block_order);
    }
    // 1.3.4
    compat.read("fix-player-filter-bounce", c.fix_player_filter_bounce, c.fix_player_filter_bounce);
    compat.read("fix-player-downward-clip", c.fix_player_downward_clip, c.fix_player_downward_clip);
    compat.read("fix-npc-downward-clip", c.fix_npc_downward_clip, c.fix_npc_downward_clip);
    // 1.3.5.3
    // compat.read("require-ground-to-enter-warps", c.require_ground_to_enter_warps, c.require_ground_to_enter_warps); // REMOVED SINCE 1.3.6
    compat.read("fix-npc-activation-event-loop-bug", c.fix_npc_activation_event_loop_bug, c.fix_npc_activation_event_loop_bug);
    // 1.3.6
    compat.read("world-map-fast-move", c.world_map_fast_move, c.world_map_fast_move);
    compat.read("fix-framethrower-gravity", c.fix_flamethrower_gravity, c.fix_flamethrower_gravity);
    compat.endGroup();
}

void LoadCustomCompat()
{
    std::string episodeCompat, customCompat;

    g_dirEpisode.setCurDir(FileNamePath);
    g_dirCustom.setCurDir(FileNamePath + FileName);

    // Episode-wide custom player setup
    episodeCompat = g_dirEpisode.resolveFileCaseExistsAbs("compat.ini");
    // Level-wide custom player setup
    customCompat = g_dirCustom.resolveFileCaseExistsAbs("compat.ini");

    compatInit(g_compatibility);

    if(!episodeCompat.empty())
        loadCompatIni(g_compatibility, episodeCompat);
    if(!customCompat.empty())
        loadCompatIni(g_compatibility, customCompat);

    GraphicsHelps::setBitBlitBG((uint8_t)g_compatibility.bitblit_background_colour[0],
                                (uint8_t)g_compatibility.bitblit_background_colour[1],
                                (uint8_t)g_compatibility.bitblit_background_colour[2]);
}

void ResetCompat()
{
    compatInit(g_compatibility);
    GraphicsHelps::resetBitBlitBG();
}

void CompatSetEnforcedLevel(int cLevel)
{
    if(s_compatLevel == cLevel)
        return;

    s_compatLevel = cLevel;

    switch(s_compatLevel)
    {
    default:
    case COMPAT_MODERN:
        pLogDebug("The compatibility level was changed: Modern");
        break;
    case COMPAT_SMBX2:
        pLogDebug("The compatibility level was changed: Enforced SMBX2");
        break;
    case COMPAT_SMBX13:
        pLogDebug("The compatibility level was changed: Enforced SMBX 1.3");
        break;
    }
}

int CompatGetLevel()
{
    return s_compatLevel;
}
