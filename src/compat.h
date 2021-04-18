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

#ifndef COMPAT_H
#define COMPAT_H

struct Compatibility_t
{
    bool enable_last_warp_hub_resume;
    bool fix_pokey_collapse;
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
    bool fix_peach_escape_shell_surf;
    bool fix_keyhole_framerate;
    bool fix_link_clowncar_fairy;
    bool fix_dont_switch_player_by_clowncar;
};

extern Compatibility_t g_compatibility;

void LoadCustomCompat();
void ResetCompat();

#endif // COMPAT_H
