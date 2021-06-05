/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2021 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <Logger/logger.h>
#include <IniProcessor/ini_processing.h>
#include <Utils/files.h>
#include <Utils/dir_list_ci.h>
#include "globals.h"
#include "compat.h"
#include "main/speedrunner.h"


Compatibility_t g_compatibility;

static void compatInit(Compatibility_t &c)
{
    c.enable_last_warp_hub_resume = true;
    c.fix_platforms_acceleration = true;
    c.fix_pokey_collapse = true;
    c.fix_player_filter_bounce = true;
    c.fix_player_downward_clip = true;
    c.fix_npc_downward_clip = true;
    c.fix_npc55_kick_ice_blocks = false;
    c.fix_climb_invisible_fences = true;
    c.fix_climb_bgo_speed_adding = true;
    c.enable_climb_bgo_layer_move = true;
    c.fix_player_clip_wall_at_npc = true;
    c.fix_skull_raft = true;
    c.fix_peach_escape_shell_surf = true;
    c.fix_keyhole_framerate = true;
    c.fix_link_clowncar_fairy = true;
    c.fix_dont_switch_player_by_clowncar = true;
    c.enable_multipoints = true;
    c.fix_autoscroll_speed = false;
    c.fix_blooper_stomp_effect = true;
    c.keep_bullet_bill_dir = true;
    c.fix_pswitch_dragon_coin = true;
    c.free_level_res = true;
    c.free_world_res = true;
    c.NPC_activate_mode = NPC_activate_modes::smart;

    if(g_speedRunnerMode >= SPEEDRUN_MODE_2) // Make sure that bugs were same as on SMBX2 Beta 4 on this moment
    {
        c.enable_last_warp_hub_resume = false;
        c.fix_platforms_acceleration = false;
        c.fix_pokey_collapse = false;
        c.fix_npc_downward_clip = false;
        c.fix_npc55_kick_ice_blocks = false;
        c.fix_climb_invisible_fences = false;
        c.fix_climb_bgo_speed_adding = false;
        c.enable_climb_bgo_layer_move = false;
        c.fix_skull_raft = false;
        c.fix_peach_escape_shell_surf = false;
        c.fix_keyhole_framerate = false;
        c.fix_link_clowncar_fairy = false;
        c.fix_dont_switch_player_by_clowncar = false;
        c.enable_multipoints = false;
        c.fix_autoscroll_speed = false;
        c.fix_blooper_stomp_effect = false;
        c.keep_bullet_bill_dir = false;
        c.fix_pswitch_dragon_coin = false;
        c.free_level_res = false;
        c.free_world_res = false;
        c.NPC_activate_mode = NPC_activate_modes::onscreen;
    }

    if(g_speedRunnerMode >= SPEEDRUN_MODE_3) // Strict vanilla SMBX
    {
        c.fix_player_filter_bounce = false;
        c.fix_player_downward_clip = false;
        c.fix_player_clip_wall_at_npc = false;
    }

    c.speedrun_stop_timer_by_event = false;
    SDL_memset(c.speedrun_stop_timer_event_name, 0, sizeof(c.speedrun_stop_timer_event_name));
    SDL_strlcpy(c.speedrun_stop_timer_event_name, "Boss Dead", sizeof(c.speedrun_stop_timer_event_name));
}

static void loadCompatIni(Compatibility_t &c, const std::string &fileName)
{
    std::string buffer;
    pLogDebug("Loading %s...", fileName.c_str());

    IniProcessing compat(fileName);
    if(!compat.isOpened())
    {
        pLogWarning("Can't open the compat.ini file: %s", fileName.c_str());
        return;
    }

    compat.beginGroup("speedrun");
    compat.read("stop-timer-by-event", c.speedrun_stop_timer_by_event, c.speedrun_stop_timer_by_event);
    compat.read("stop-timer-event-name", buffer, std::string(c.speedrun_stop_timer_event_name));
    SDL_strlcpy(c.speedrun_stop_timer_event_name, buffer.c_str(), sizeof(c.speedrun_stop_timer_event_name));
    compat.endGroup();

    if(g_speedRunnerMode >= SPEEDRUN_MODE_3)
    {
        pLogDebug("Speed-Run Mode 3 detected, the [compatibility] section for the compat.ini completely skipped, all old bugs enforced.", fileName.c_str());
        return;
    }

    compat.beginGroup("compatibility");
    if(g_speedRunnerMode < SPEEDRUN_MODE_2)
    {
        compat.read("enable-last-warp-hub-resume", c.enable_last_warp_hub_resume, c.enable_last_warp_hub_resume);
        compat.read("fix-platform-acceleration", c.fix_platforms_acceleration, c.fix_platforms_acceleration);
        compat.read("fix-pokey-collapse", c.fix_pokey_collapse, c.fix_pokey_collapse);
        compat.read("fix-npc55-kick-ice-blocks", c.fix_npc55_kick_ice_blocks, c.fix_npc55_kick_ice_blocks);
        compat.read("fix-climb-invisible-fences", c.fix_climb_invisible_fences, c.fix_climb_invisible_fences);
        compat.read("fix-climb-bgo-speed-adding", c.fix_climb_bgo_speed_adding, c.fix_climb_bgo_speed_adding);
        compat.read("enable-climb-bgo-layer-move", c.enable_climb_bgo_layer_move, c.enable_climb_bgo_layer_move);
        compat.read("fix-player-clip-wall-at-npc", c.fix_player_clip_wall_at_npc, c.fix_player_clip_wall_at_npc);
        compat.read("fix-skull-raft", c.fix_skull_raft, c.fix_skull_raft);
        compat.read("fix-peach-escape-shell-surf", c.fix_peach_escape_shell_surf, c.fix_peach_escape_shell_surf);
        compat.read("fix-keyhole-framerate", c.fix_keyhole_framerate, c.fix_keyhole_framerate);
        compat.read("fix-link-clowncar-fairy", c.fix_link_clowncar_fairy, c.fix_link_clowncar_fairy);
        compat.read("fix-dont-switch-player-by-clowncar", c.fix_dont_switch_player_by_clowncar, c.fix_dont_switch_player_by_clowncar);
        compat.read("enable-multipoints", c.enable_multipoints, c.enable_multipoints);
        compat.read("fix-autoscroll-speed", c.fix_autoscroll_speed, c.fix_autoscroll_speed);
        compat.read("fix-blooper-stomp-effect", c.fix_blooper_stomp_effect, c.fix_blooper_stomp_effect);
        compat.read("keep-bullet-bill-direction", c.keep_bullet_bill_dir, c.keep_bullet_bill_dir);
        compat.read("fix-pswitch-dragon-coin", c.fix_pswitch_dragon_coin, c.fix_pswitch_dragon_coin);
        compat.read("free-level-res", c.free_level_res, c.free_level_res);
        compat.read("free-world-res", c.free_world_res, c.free_world_res);
        IniProcessing::StrEnumMap activModes =
        {
            {"onscreen", (int)NPC_activate_modes::onscreen},
            {"smart", (int)NPC_activate_modes::smart},
            {"orig", (int)NPC_activate_modes::orig},
        };
        compat.readEnum("npc-activate-mode", c.NPC_activate_mode, c.NPC_activate_mode, activModes);
    }
    compat.read("fix-player-filter-bounce", c.fix_player_filter_bounce, c.fix_player_filter_bounce);
    compat.read("fix-player-downward-clip", c.fix_player_downward_clip, c.fix_player_downward_clip);
    compat.read("fix-npc-downward-clip", c.fix_npc_downward_clip, c.fix_npc_downward_clip);
    compat.endGroup();
}

void LoadCustomCompat()
{
    DirListCI s_dirEpisode;
    DirListCI s_dirCustom;
    std::string episodeCompat, customCompat;

    s_dirEpisode.setCurDir(FileNamePath);
    s_dirCustom.setCurDir(FileNamePath + FileName);

    // Episode-wide custom player setup
    episodeCompat = FileNamePath + s_dirEpisode.resolveFileCase("compat.ini");
    // Level-wide custom player setup
    customCompat = FileNamePath + FileName + "/" + s_dirCustom.resolveFileCase("compat.ini");

    compatInit(g_compatibility);

    if(Files::fileExists(episodeCompat))
        loadCompatIni(g_compatibility, episodeCompat);
    if(Files::fileExists(customCompat))
        loadCompatIni(g_compatibility, customCompat);
}

void ResetCompat()
{
    compatInit(g_compatibility);
}
