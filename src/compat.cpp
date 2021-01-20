/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2020 Vitaly Novichkov <admin@wohlnet.ru>
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


Compatibility_t g_compatibility;

static void compatInit(Compatibility_t &c)
{
    c.fix_platforms_acceleration = true;
    c.fix_pokey_collapse = true;
    c.fix_player_filter_bounce = true;
    c.fix_player_downward_clip = true;
    c.fix_npc_downward_clip = true;
    c.fix_npc55_kick_ice_blocks = false;
    c.fix_climb_invisible_fences = true;
    c.fix_climb_bgo_speed_adding = true;
    c.enable_climb_bgo_layer_move = true;
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

    compat.beginGroup("compatibility");
    compat.read("fix-platform-acceleration", c.fix_platforms_acceleration, c.fix_platforms_acceleration);
    compat.read("fix-pokey-collapse", c.fix_pokey_collapse, c.fix_pokey_collapse);
    compat.read("fix-player-filter-bounce", c.fix_player_filter_bounce, c.fix_player_filter_bounce);
    compat.read("fix-player-downward-clip", c.fix_player_downward_clip, c.fix_player_downward_clip);
    compat.read("fix-npc-downward-clip", c.fix_npc_downward_clip, c.fix_npc_downward_clip);
    compat.read("fix-npc55-kick-ice-blocks", c.fix_npc55_kick_ice_blocks, c.fix_npc55_kick_ice_blocks);
    compat.read("fix-climb-invisible-fences", c.fix_climb_invisible_fences, c.fix_climb_invisible_fences);
    compat.read("fix-climb-bgo-speed-adding", c.fix_climb_bgo_speed_adding, c.fix_climb_bgo_speed_adding);
    compat.read("enable-climb-bgo-layer-move", c.enable_climb_bgo_layer_move, c.enable_climb_bgo_layer_move);
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
