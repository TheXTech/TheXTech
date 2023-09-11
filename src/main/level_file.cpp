/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2023 Vitaly Novichkov <admin@wohlnet.ru>
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

#include "sdl_proxy/sdl_stdinc.h"

#ifdef __16M__
// used to clear loaded textures on level/world load
#include "core/render.h"
#endif

#include "../globals.h"
#include "../game_main.h" // SetupPhysics()
#include "../frame_timer.h"
#include "../npc.h"
#include "../load_gfx.h"
#include "../custom.h"
#include "../sound.h"
#include "../sorting.h"
#include "../layers.h"
#include "../compat.h"
#include "../graphics.h"
#include "../editor.h"
#include "../npc_id.h"
#include "level_file.h"
#include "trees.h"
#include "npc_special_data.h"
#include "graphics/gfx_update.h"
#include "translate_episode.h"
#include "fontman/font_manager.h"

#include <DirManager/dirman.h>
#include <Utils/files.h>
#include <Utils/strings.h>
#include <Utils/dir_list_ci.h>
#include <Logger/logger.h>
#include <PGE_File_Formats/file_formats.h>

#include "global_dirs.h"
#include "screen_fader.h"

#include "editor/editor_custom.h"
#include "editor/editor_strings.h"


#ifdef THEXTECH_BUILD_GL_MODERN
#    include "core/opengl/gl_program_bank.h"
#endif


void bgoApplyZMode(Background_t *bgo, int smbx64sp)
{
    if(bgo->zMode == LevelBGO::ZDefault)
        bgo->SortPriority = smbx64sp;
    else
    {
        switch(bgo->zMode)
        {
        case LevelBGO::Background2:
            bgo->SortPriority = 10;
            break;
        case LevelBGO::Background1:
            bgo->SortPriority = 30;
            break;
        case LevelBGO::Foreground1:
            bgo->SortPriority = 125;
            break;
        case LevelBGO::Foreground2:
            bgo->SortPriority = 200;
            break;
        default:
            break;
        }
    }
}

void addMissingLvlSuffix(std::string &fileName)
{
    if(!fileName.empty() && !Files::hasSuffix(fileName, ".lvl") && !Files::hasSuffix(fileName, ".lvlx") && !Files::hasSuffix(fileName, "tst"))
    {
        bool isAbsolute = Files::isAbsolute(fileName);
        bool lvlxExists;

        if(isAbsolute)
            lvlxExists = Files::fileExists(fileName + ".lvlx");
        else
            lvlxExists = !g_dirEpisode.resolveFileCaseExists(fileName + ".lvlx").empty();

        if(lvlxExists)
            fileName += ".lvlx";
        else
            fileName += ".lvl";
    }
}

void validateLevelName(std::string &out, const std::string &raw)
{
    if(raw.empty())
    {
        out.clear();
        return;
    }

    if(!Strings::endsWith(raw, ".lvl") && !Strings::endsWith(raw, ".lvlx"))
    {
        std::string lx = g_dirEpisode.resolveFileCaseExists(raw + ".lvlx"),
                    lo = g_dirEpisode.resolveFileCaseExists(raw + ".lvl");

        if(!lx.empty())
            out = lx;
        else if(!lo.empty())
            out = lo;
        else
            out = g_dirEpisode.resolveFileCase(raw);
    }
    else
        out = g_dirEpisode.resolveFileCase(raw);
}

bool OpenLevel(std::string FilePath)
{
    addMissingLvlSuffix(FilePath);
//    if(!Files::hasSuffix(FilePath, ".lvl") && !Files::hasSuffix(FilePath, ".lvlx"))
//    {
//        if(Files::fileExists(FilePath + ".lvlx"))
//            FilePath += ".lvlx";
//        else
//            FilePath += ".lvl";
//    }

    {
        LevelData lvl;
        if(!FileFormats::OpenLevelFile(FilePath, lvl))
        {
        pLogWarning("Error of level \"%s\" file loading: %s (line %d).",
                    FilePath.c_str(),
                    lvl.meta.ERROR_info.c_str(),
                    lvl.meta.ERROR_linenum);
            return false;
        }

        if(!OpenLevelData(lvl, FilePath))
            return false;
    }

    OpenLevelDataPost();

    return true;
}

bool OpenLevelData(LevelData &lvl, const std::string FilePath)
{
//    std::string newInput;
//    int FileRelease = 0;
    int A = 0;
    int B = 0;
    int checkPointId = 0;
//    int C = 0;
//    bool tempBool = false;
//    int mSections = 0;
//    Location_t tempLocation;

    qScreen = false;
    ClearLevel();
    BlockSound();
    FreezeNPCs = false;
    CoinMode = false;

    g_dirEpisode.setCurDir(lvl.meta.path);
    FileFormat = lvl.meta.RecentFormat;
    FileName = g_dirEpisode.resolveDirCase(lvl.meta.filename);
    FileNamePath = lvl.meta.path + "/";
    g_dirCustom.setCurDir(FileNamePath + FileName);

    bool compatModern = (CompatGetLevel() == COMPAT_MODERN);
    bool isSmbx64 = (lvl.meta.RecentFormat == LevelData::SMBX64);
    int  fVersion = lvl.meta.RecentFormatVersion;

    if(!FilePath.empty())
    {
        FileNameFull = Files::basename(FilePath);
        FullFileName = FilePath;
    }
    else if(FileFormat == FileFormats::LVL_SMBX64 || FileFormat == FileFormats::LVL_SMBX38A)
    {
        FileNameFull = FileName + ".lvl";
        FullFileName = FileNamePath + FileName + ".lvl";
    }
    else
    {
        FileNameFull = FileName + ".lvlx";
        FullFileName = FileNamePath + FileName + ".lvlx";
    }

    IsEpisodeIntro = (StartLevel == FileNameFull);

    FileFormats::smbx64LevelPrepare(lvl);
    FileFormats::smbx64LevelSortBlocks(lvl);
    FileFormats::smbx64LevelSortBGOs(lvl);

    numBlock = 0;
    numBackground = 0;
    numLocked = 0;
    numNPCs = 0;
    numWarps = 0;

    numLayers = 0;
    numEvents = 0;

// Load Custom Stuff
//    if(DirMan::exists(FileNamePath + FileName))
//        FindCustomNPCs(FileNamePath + FileName);
//    else
    LoadCustomCompat();
    FindCustomPlayers();
    FindCustomNPCs();
    LoadCustomGFX();
    LoadCustomSound();
    FontManager::loadCustomFonts();

//    if(DirMan::exists(FileNamePath + FileName)) // Useless now
//        LoadCustomGFX2(FileNamePath + FileName);
// Blah

    if(FilePath == ".lvl" || FilePath == ".lvlx")
        return false;

    maxStars = lvl.stars;
    LevelName = lvl.LevelName;

    numSections = 0;
    B = 0;
    for(auto & s : lvl.sections)
    {
        level[B].X = double(s.size_left);
        level[B].Y = double(s.size_top);
        level[B].Height = double(s.size_bottom);
        level[B].Width = double(s.size_right);
        LevelREAL[B] = level[B];
        bgMusic[B] = int(s.music_id);
        bgMusicREAL[B] = bgMusic[B];
        // bgColor[B] = s.bgcolor;    // unused since SMBX64, removed
        LevelWrap[B] = s.wrap_h;
        LevelVWrap[B] = s.wrap_v; // EXTRA
        OffScreenExit[B] = s.OffScreenEn;
        Background2[B] = int(s.background);
        Background2REAL[B] = Background2[B];
        NoTurnBack[B] = s.lock_left_scroll;
        UnderWater[B] = s.underwater;
        if(s.music_file.empty())
            CustomMusic[B].clear();
        else
            CustomMusic[B] = g_dirEpisode.resolveFileCase(s.music_file);

#if defined(THEXTECH_BUILD_GL_MODERN) && defined(THEXTECH_WIP_FEATURES)
        // FIXME: allow sections to specify shaders by name
        SectionEffect[B] = ResolveGLProgram("section-effect");
        SectionParticlesBG[B] = ResolveGLParticleSystem("particles-bg");
        SectionParticlesFG[B] = ResolveGLParticleSystem("particles-fg");
#endif

        B++;
        if(B > maxSections)
            break;
        numSections++;
    }

    for(A = 1; A <= 2; A++) // Fill with zeroes
    {
        PlayerStart[A].X = 0;
        PlayerStart[A].Y = 0;
        PlayerStart[A].Width = 0;
        PlayerStart[A].Height = 0;
    }

    A = 1;
    for(auto &p : lvl.players)
    {
        PlayerStart[A].X = double(p.x);
        PlayerStart[A].Y = double(p.y);
        PlayerStart[A].Width = double(p.w);
        PlayerStart[A].Height = double(p.h);
        PlayerStart[A].Direction = p.direction;
        A++;
        if(A > 2)
            break;
    }

    A = 0;
    for(auto &l : lvl.layers)
    {
        auto &layer = Layer[A];

        layer = Layer_t();

        layer.Name = l.name;
        layer.Hidden = l.hidden;
        // hide layers after everything is done
        A++;
        numLayers++;
        if(numLayers > maxLayers)
        {
            numLayers = maxLayers;
            break;
        }
    }

    LAYER_USED_P_SWITCH = FindLayer(LAYER_USED_P_SWITCH_TITLE);

    // items in layers will be hidden after they are loaded

    A = 0;
    for(auto &e : lvl.events)
    {
        auto &event = Events[A];

        event = Events_t();

        event.Name = e.name;
        if(!e.msg.empty())
            SetS(event.Text, e.msg);
        event.Sound = int(e.sound_id);
        event.EndGame = int(e.end_game);

        event.HideLayer.clear();
        for(std::string& l : e.layers_hide)
        {
            layerindex_t found = FindLayer(l);
            if(found != LAYER_NONE)
                event.HideLayer.push_back(found);
        }
        event.ShowLayer.clear();
        for(std::string& l : e.layers_show)
        {
            layerindex_t found = FindLayer(l);
            if(found != LAYER_NONE)
                event.ShowLayer.push_back(found);
        }
        event.ToggleLayer.clear();
        for(std::string& l : e.layers_toggle)
        {
            layerindex_t found = FindLayer(l);
            if(found != LAYER_NONE)
                event.ToggleLayer.push_back(found);
        }

        int maxSets = int(e.sets.size());
        if(maxSets > numSections)
            maxSets = numSections;

        for(B = 0; B <= maxSections; B++)
        {
            auto &s = event.section[B];
            s.music_id = LevelEvent_Sets::LESet_Nothing;
            s.background_id = LevelEvent_Sets::LESet_Nothing;
            s.music_file = STRINGINDEX_NONE;
            s.position.X = LevelEvent_Sets::LESet_Nothing;
            s.position.Y = 0;
            s.position.Height = 0;
            s.position.Width = 0;
        }

        for(B = 0; B < maxSets; B++)
        {
            auto &ss = event.section[B];
            auto &s = e.sets[size_t(B)];
            ss.music_id = int(s.music_id);
            ss.background_id = int(s.background_id);
            if(!s.music_file.empty())
            {
                SetS(ss.music_file, s.music_file);
            }

            auto &l = ss.position;
            l.X = s.position_left;
            l.Y = s.position_top;
            l.Height = s.position_bottom;
            l.Width = s.position_right;

            ss.autoscroll = s.autoscrol;
            // Simple style is only supported yet
            if(s.autoscroll_style == LevelEvent_Sets::AUTOSCROLL_SIMPLE)
            {
                ss.autoscroll_x = s.autoscrol_x;
                ss.autoscroll_y = s.autoscrol_y;
            }
        }

        event.TriggerDelay = e.trigger_timer;

        event.LayerSmoke = e.nosmoke;

        event.Controls.AltJump = e.ctrl_altjump;
        event.Controls.AltRun = e.ctrl_altrun;
        event.Controls.Down = e.ctrl_down;
        event.Controls.Drop = e.ctrl_drop;
        event.Controls.Jump = e.ctrl_jump;
        event.Controls.Left = e.ctrl_left;
        event.Controls.Right = e.ctrl_right;
        event.Controls.Run = e.ctrl_run;
        event.Controls.Start = e.ctrl_start;
        event.Controls.Up = e.ctrl_up;

        event.AutoStart = e.autostart;
        event.MoveLayer = FindLayer(e.movelayer);
        event.SpeedX = float(e.layer_speed_x);
        event.SpeedY = float(e.layer_speed_y);

        event.AutoX = float(e.move_camera_x);
        event.AutoY = float(e.move_camera_y);
        event.AutoSection = int(e.scroll_section);

        A++;
        numEvents++;
        if(numEvents > maxEvents)
        {
            numEvents = maxEvents;
            break;
        }
    }

    // second pass needed for events that trigger other events
    A = 0;
    for(auto &e : lvl.events)
    {
        Events[A].TriggerEvent = FindEvent(e.trigger);

        A++;
        if(A > maxEvents)
            break;
    }

    for(auto &b : lvl.blocks)
    {
        numBlock++;
        if(numBlock > maxBlocks)
        {
            numBlock = maxBlocks;
            break;
        }

        auto &block = Block[numBlock];

        block = Block_t();

        block.Location.X = double(b.x);
        block.Location.Y = double(b.y);
        block.Location.Height = double(b.h);
        block.Location.Width = double(b.w);
        block.Type = int(b.id);
        block.DefaultType = block.Type;

        block.Special = int(b.npc_id > 0 ? b.npc_id + 1000 : -1 * b.npc_id);
        if(block.Special == 100)
            block.Special = 1009;
        if(block.Special == 102)
            block.Special = 1014;
        if(block.Special == 103)
            block.Special = 1034;
        if(block.Special == 105)
            block.Special = 1095;
        block.DefaultSpecial = block.Special;

        block.forceSmashable = false;
        if(b.id == 90)
        {
            if(lvl.meta.RecentFormat == LevelData::SMBX64 && lvl.meta.RecentFormatVersion < 20)
                block.forceSmashable = true; // Restore bricks algorithm for turn blocks for SMBX19 and lower
            else
                block.forceSmashable = (bool)b.special_data; // load it if set in the modern format
        }

        block.Invis = b.invisible;
        block.Slippy = b.slippery;
        block.Layer = FindLayer(b.layer);
        block.TriggerDeath = FindEvent(b.event_destroy);
        block.TriggerHit = FindEvent(b.event_hit);
        block.TriggerLast = FindEvent(b.event_emptylayer);

        if(IF_OUTRANGE(block.Type, 0, maxBlockType)) // Drop ID to 1 for blocks of out of range IDs
        {
            pLogWarning("Block-%d ID is out of range (max types %d), reset to Block-1", block.Type, maxBlockType);
            block.Type = 1;
        }
    }

    for(auto &b : lvl.bgo)
    {
        numBackground++;
        if(numBackground > maxBackgrounds)
        {
            numBackground = maxBackgrounds;
            break;
        }

        auto &bgo = Background[numBackground];

        bgo = Background_t();

        bgo.Location.X = double(b.x);
        bgo.Location.Y = double(b.y);
        bgo.Type = int(b.id);

        if(IF_OUTRANGE(bgo.Type, 1, maxBackgroundType)) // Drop ID to 1 for BGOs of out of range IDs
        {
            pLogWarning("BGO-%d ID is out of range (max types %d), reset to BGO-1", bgo.Type, maxBackgroundType);
            bgo.Type = 1;
        }

        bgo.Layer = FindLayer(b.layer);
        bgo.Location.Width = GFXBackgroundWidth[bgo.Type];
        bgo.Location.Height = BackgroundHeight[bgo.Type];

        bgo.uid = int(b.meta.array_id);

        bgo.zMode = b.z_mode;
        bgo.zOffset = b.z_offset;

        bgoApplyZMode(&bgo, int(b.smbx64_sp));
    }


    for(auto &n : lvl.npc)
    {
        bool variantHandled = false;

        numNPCs++;
        if(numNPCs > maxNPCs)
        {
            numNPCs = maxNPCs;
            break;
        }

        auto &npc = NPC[numNPCs];

        npc = NPC_t();

        npc.Location.X = n.x;
        npc.Location.Y = n.y;
        if(!LevelEditor)
            npc.Location.Y -= 0.01;
        npc.Direction = n.direct;
        npc.Type = int(n.id);

        if(IF_OUTRANGE(npc.Type, 0, maxNPCType)) // Drop ID to 1 for NPCs of out of range IDs
        {
            pLogWarning("NPC-%d ID is out of range (max types %d), reset to NPC-1", npc.Type, maxNPCType);
            npc.Type = 1;
        }

        if(npc.Type == NPCID_ITEM_BURIED || npc.Type == NPCID_ITEM_POD ||
           npc.Type == NPCID_ITEM_BUBBLE || npc.Type == NPCID_ITEM_THROWER)
        {
            npc.Special = n.contents;
            npc.DefaultSpecial = int(npc.Special);
            npc.Variant = n.special_data;
            variantHandled = true;
        }

        if(npc.Type == NPCID_DOOR_MAKER || npc.Type == NPCID_MAGIC_DOOR ||
          (npc.Type == NPCID_ITEM_BURIED && n.contents == NPCID_DOOR_MAKER))
        {
            npc.Special2 = n.special_data;
            npc.DefaultSpecial2 = int(npc.Special2);
        }

        if(NPCIsAParaTroopa[npc.Type])
        {
            npc.Special = n.special_data;
            npc.DefaultSpecial = int(npc.Special);
        }

        if(NPCIsCheep[npc.Type])
        {
            npc.Special = n.special_data;
            npc.DefaultSpecial = int(npc.Special);
        }

        if(npc.Type == NPCID_FIRE_CHAIN)
        {
            npc.Special = n.special_data;
            npc.DefaultSpecial = int(npc.Special);
        }

        if(npc.Type == NPCID_STAR_EXIT || npc.Type == NPCID_STAR_COLLECT)
        {
            npc.Variant = n.special_data;
            variantHandled = true;
        }

        if(compatModern && isSmbx64)
        {
            // legacy Smbx64 NPC behavior tracking moved to npc_special_data.h
            npc.Variant = find_legacy_Variant(npc.Type, fVersion);
        }
        else if(isSmbx64)
        {
            // don't load anything for SMBX64 files
            npc.Variant = 0;
        }
        else if(find_Variant_Data(npc.Type) /* && compatModern */)
        {
            // only load Special7 for NPCs that support it
            if((n.special_data < 0) || (n.special_data >= 256))
                pLogWarning("Attempted to load npc Type %d with out-of-range variant index %f", npc.Type, n.special_data);
            else
                npc.Variant = (uint8_t)n.special_data;
        }
        else
        {
            if(!variantHandled)
                npc.Variant = 0;
        }

        // All of the following duplicate the new Special7 code.
        // That code should be updated instead, because doing so is required to update the in-game editor.

#if 0
        if(npc.Type == NPCID_CANNONITEM) // billy gun
        {
            if(compatModern && isSmbx64 && fVersion < 28)
                npc.Special7 = 2.0; // SMBX 1.1.x and 1.0.x behavior
            else if(compatModern && isSmbx64 && fVersion < 51)
                npc.Special7 = 1.0; // SMBX 1.2 behavior
            else if(compatModern)
                npc.Special7 = n.special_data; // SMBX 1.2.1 and newer behavior, customizable behavior
        }

        if(npc.Type == NPCID_STONE_S3)
        {
            if(compatModern && isSmbx64 && fVersion < 9)
                npc.Special7 = 1.0; // Make twomps to fall always
            else if(compatModern)
                npc.Special7 = n.special_data;
        }

        if(npc.Type == NPCID_VILLAIN_S3)
        {
            if(compatModern && isSmbx64 && fVersion < 30)
                npc.Special7 = 1.0; // Keep original behavior of Bowser as in SMBX 1.0
            else if(compatModern)
                npc.Special7 = n.special_data;
        }

        switch(npc.Type)
        {
        case NPCID_YEL_PLATFORM:
        case NPCID_BLU_PLATFORM:
        case NPCID_GRN_PLATFORM:
        case NPCID_RED_PLATFORM:
        case NPCID_PLATFORM_S3:
        case NPCID_SAW:
            if(compatModern && isSmbx64 && fVersion < 30)
                npc.Special7 = 1.0; // Workaround for yellow platform at The Invasion 1 on the "Clown Car Parking" level
            else if(compatModern)
                npc.Special7 = n.special_data;
            break;

        default:
            break;
        }
#endif

        npc.Generator = n.generator;
        if(npc.Generator)
        {
            npc.GeneratorDirection = n.generator_direct;
            npc.GeneratorEffect = n.generator_type;
            npc.GeneratorTimeMax = n.generator_period;
        }

        if(!n.msg.empty())
            SetS(npc.Text, n.msg);

        npc.Inert = n.friendly;
        if(npc.Type == NPCID_SIGN)
            npc.Inert = true;
        npc.Stuck = n.nomove;
        npc.DefaultStuck = npc.Stuck;

        npc.Legacy = n.is_boss;

        npc.Layer = FindLayer(n.layer);
        npc.TriggerActivate = FindEvent(n.event_activate);
        npc.TriggerDeath = FindEvent(n.event_die);
        npc.TriggerTalk = FindEvent(n.event_talk);
        npc.TriggerLast = FindEvent(n.event_emptylayer);
        npc.AttLayer = FindLayer(n.attach_layer);

        npc.DefaultType = npc.Type;
        npc.Location.Width = NPCWidth[npc.Type];
        npc.Location.Height = NPCHeight[npc.Type];
        npc.DefaultLocation = npc.Location;
        npc.DefaultDirection = npc.Direction;

        // allow every NPC to be active for one frame to initialize its internal state
        npc.TimeLeft = 1;
        npc.Active = true;
        npc.JustActivated = 1;

        CheckSectionNPC(numNPCs);

        if(npc.Type == NPCID_CHECKPOINT) // Is a checkpoint
        {
            checkPointId++;
            if(compatModern)
            {
                npc.Special = checkPointId;
                npc.DefaultSpecial = int(npc.Special);
            }
        }
        else if(npc.Type == NPCID_STAR_EXIT || npc.Type == NPCID_STAR_COLLECT) // Is a star
        {
            bool starFound = false;
            for(const auto& star : Star)
            {
                bool bySection = npc.Variant == 0 && (star.Section == npc.Section || star.Section == -1);
                bool byId = npc.Variant > 0 && -(star.Section + 100) == (int)npc.Variant;
                if(star.level == FileNameFull && (bySection || byId))
                    starFound = true;
            }

            if(starFound)
            {
                npc.Special = 1;
                npc.DefaultSpecial = 1;
                if(npc.Type == NPCID_STAR_COLLECT)
                    npc.Killed = 9;
            }
        }
        else if((npc.Type == NPCID_ITEM_BURIED || npc.Type == NPCID_ITEM_POD ||
                  npc.Type == NPCID_ITEM_BUBBLE || npc.Type == NPCID_ITEM_THROWER) &&
                (n.contents == NPCID_STAR_EXIT || n.contents == NPCID_STAR_COLLECT)) // Is a container that has a star inside
        {
            bool starFound = false;
            for(const auto& star : Star)
            {
                bool byId = npc.Variant > 0 && -(star.Section + 100) == (int)npc.Variant;
                if(star.level == FileNameFull && byId)
                    starFound = true;
            }

            if(starFound)
            {
                if(n.contents == NPCID_STAR_COLLECT)
                    npc.Killed = 9;
            }
        }

        syncLayers_NPC(numNPCs);
    }

    std::string level_name;

    for(auto &w : lvl.doors)
    {
        numWarps++;
        if(numWarps > maxWarps)
        {
            numWarps = maxWarps;
            break;
        }

        auto &warp = Warp[numWarps];

        warp = Warp_t();

        warp.PlacedEnt = true;
        warp.PlacedExit = true;
        warp.Entrance.X = w.ix;
        warp.Entrance.Y = w.iy;
        warp.Exit.X = w.ox;
        warp.Exit.Y = w.oy;
        warp.Direction = w.idirect;
        warp.Direction2 = w.odirect;
        warp.Effect = w.type;

        // Work around filenames with no extension suffix and case missmatch
        if(!w.lname.empty())
        {
            validateLevelName(level_name, w.lname);
            SetS(warp.level, level_name);
        }

        warp.LevelWarp = int(w.warpto);
        warp.LevelEnt = w.lvl_i;

        warp.MapWarp = w.lvl_o;
        warp.MapX = int(w.world_x);
        warp.MapY = int(w.world_y);

        warp.Stars = w.stars;
        warp.Layer = FindLayer(w.layer);
        warp.Hidden = w.unknown;

        warp.NoYoshi = w.novehicles;
        warp.WarpNPC = w.allownpc;
        warp.Locked = w.locked;

        // custom fields:
        warp.twoWay = w.two_way;

        warp.cannonExit = w.cannon_exit;
        warp.cannonExitSpeed = w.cannon_exit_speed;
        warp.eventEnter = FindEvent(w.event_enter);
        if(!w.stars_msg.empty())
            SetS(warp.StarsMsg, w.stars_msg);
        warp.noPrintStars = w.star_num_hide;
        warp.noEntranceScene = w.hide_entering_scene;

        warp.stoodRequired = w.stood_state_required;
        warp.transitEffect = w.transition_effect;

        warp.Entrance.Height = 32;
        warp.Entrance.Width = 32;
        warp.Exit.Height = 32;
        warp.Exit.Width = 32;

        // FIXME: allow warp object to specify a transit effect name as a string
        if(warp.transitEffect >= ScreenFader::S_CUSTOM)
            warp.transitEffect = ScreenFader::loadTransitEffect(std::to_string(warp.transitEffect));

        syncLayers_Warp(numWarps);
    }


    for(auto &w : lvl.physez)
    {
        numWater++;
        if(numWater > maxWater)
        {
            numWater = maxWater;
            break;
        }

        auto &water = Water[numWater];

        water = Water_t();

        water.Location.X = w.x;
        water.Location.Y = w.y;
        water.Location.Width = w.w;
        water.Location.Height = w.h;
        water.Buoy = w.buoy;
        water.Quicksand = w.env_type;
        water.Layer = FindLayer(w.layer);
        syncLayers_Water(numWater);
    }

    return true;
}

void OpenLevelDataPost()
{
    TranslateEpisode tr;

    if(!GameMenu && !LevelEditor)
        tr.loadLevelTranslation(FileNameFull);

    // FindBlocks();
    qSortBackgrounds(1, numBackground);
    UpdateBackgrounds();
    // FindSBlocks();
    syncLayersTrees_AllBlocks();
    syncLayers_AllBGOs();

    // moved the old event/layer loading code to the top
    // since it is needed before loading objects now

    for(int A = 0; A < numLayers; A++)
    {
        if(Layer[A].Hidden)
            HideLayer(A, true);
    }


    if(LevelEditor)
    {
        ResetSectionScrolls();
        SetSection(curSection);
    }
    else
    {
        FindStars();
        LevelMacro = LEVELMACRO_OFF;
        for(int A = 0; A <= numSections; A++) // Automatically correct 608 section height to 600
        {
//            if(int(level[A].Height - level[A].Y) == 608)
//                level[A].Y += 8;
            int height = int(level[A].Height - level[A].Y);
            if(height > 600 && height < 610)
                level[A].Y = level[A].Height - 600; // Better and cleaner logic
        }

        int B = numBackground;
        for(int A = 1; A <= numWarps; A++)
        {
            auto &w = Warp[A];
            if(w.Effect == 2 && w.Stars > numStars)
            {
                B++;
                numLocked++;
                auto &bgo = Background[B];
                bgo = Background_t();
                bgo.Layer = w.Layer;
                bgo.Hidden = w.Hidden;
                bgo.Location.Width = 24;
                bgo.Location.Height = 24;
                bgo.Location.Y = w.Entrance.Y - bgo.Location.Height;
                bgo.Location.X = w.Entrance.X + w.Entrance.Width / 2.0 - bgo.Location.Width / 2.0;
                bgo.Type = 160;
                syncLayers_BGO(B);

                if(w.twoWay)
                {
                    B++;
                    numLocked++;
                    auto &bgo2 = Background[B];
                    bgo2 = bgo;
                    bgo2.Location = bgo.Location;
                    bgo2.Location.Y = w.Exit.Y - bgo2.Location.Height;
                    bgo2.Location.X = w.Exit.X + w.Exit.Width / 2.0 - bgo2.Location.Width / 2.0;
                    syncLayers_BGO(B);
                }
            }
            else if(w.Effect == 2 && w.Locked) // For locks
            {
                B++;
                numLocked++;
                auto &bgo = Background[B];
                bgo = Background_t();
                bgo.Layer = w.Layer;
                bgo.Hidden = w.Hidden;
                bgo.Location = static_cast<Location_t>(w.Entrance);
                bgo.Type = 98;
                bgo.Location.Width = 16;
                syncLayers_BGO(B);

                if(w.twoWay)
                {
                    B++;
                    numLocked++;
                    auto &bgo2 = Background[B];
                    bgo2 = bgo;
                    bgo2.Location = static_cast<Location_t>(w.Exit);
                    bgo2.Location.Width = 16;
                    syncLayers_BGO(B);
                }
            }
        }
    }

    // If too much locks
    SDL_assert_release(numBackground + numLocked <= (maxBackgrounds + maxWarps));

    SoundPause[13] = 100;
    resetFrameTimer();
}

void ClearLevel()
{
    int A = 0;
    const NPC_t blankNPC = NPC_t();
    const Water_t blankwater = Water_t();
    const Warp_t blankWarp = Warp_t();
    const Block_t blankBlock = Block_t();
    const Background_t BlankBackground = Background_t();
    const Location_t BlankLocation = Location_t();
    const Effect_t blankEffect = Effect_t();
    NPCScore[NPCID_MEDAL] = 6;
    RestoreWorldStrings();
    LevelName.clear();
    ResetCompat();
    SetupPhysics();
    LoadNPCDefaults();
    LoadPlayerDefaults();
    noUpdate = true;
    BlocksSorted = true;
    qScreen = false;

#ifdef __16M__
    XRender::clearAllTextures();
#endif

#ifdef THEXTECH_BUILD_GL_MODERN
    SectionEffect.fill(LoadedGLProgramRef_t());
    SectionParticlesBG.fill(LoadedGLProgramRef_t());
    SectionParticlesFG.fill(LoadedGLProgramRef_t());
#endif

    UnloadCustomGFX();
    doShakeScreenClear();
    treeLevelCleanAll();
    FontManager::clearLevelFonts();

    invalidateDrawBlocks();
    invalidateDrawBGOs();

    AutoUseModern = false;

    numSections = 0;

    for(A = 1; A <= newEventNum; A++)
    {
        NewEvent[A] = EVENT_NONE;
        newEventDelay[A] = 0;
    }

    for(A = 0; A <= maxSections; A++)
    {
        AutoX[A] = 0;
        AutoY[A] = 0;
    }

    numEvents = 0;
    newEventNum = 0;
    for(A = 0; A <= maxEvents; A++)
    {
        InitializeEvent(Events[A]);
    }

    for(A = 0; A <= maxWater; A++)
        Water[A] = blankwater;

    numWater = 0;
    Events[0].Name = "Level - Start";
    Events[1].Name = "P Switch - Start";
    Events[2].Name = "P Switch - End";
    curMusic = 0;
    curStars = 0;
    maxStars = 0;
    PSwitchTime = 0;
    PSwitchStop = 0;
    BeltDirection = 1;
    StopMusic();
    Layer[0] = Layer_t();
    Layer[0].Name = "Default";
    Layer[0].Hidden = false;
    Layer[1] = Layer_t();
    Layer[1].Name = "Destroyed Blocks";
    Layer[1].Hidden = true;
    Layer[2] = Layer_t();
    Layer[2].Name = "Spawned NPCs";
    Layer[2].Hidden = false;

    LAYER_USED_P_SWITCH = LAYER_NONE;

    numLayers = 0;
    for(A = 3; A <= maxLayers; A++)
    {
        Layer[A] = Layer_t();
        // all of these are cleared by reinitializing the layer
        // if(A > 2)
        // {
        //     Layer[A].Name = "";
        //     Layer[A].Hidden = false;
        // }
        // Layer[A].SpeedX = 0;
        // Layer[A].SpeedY = 0;
    }

//    If LevelEditor = True Or MagicHand = True Then
    if(LevelEditor || MagicHand)
    {
//        MessageText = ""
        MessageText.clear();
        MessageTextMap.clear();
//        frmNPCs.chkMessage.Value = 0
//        frmBlocks.chkFill.Value = 0
//        frmEvents.txtEvent.Text = ""
//        noUpdate = True
        noUpdate = true;
//        frmEvents.RefreshEvents
//        frmLayers.lstLayer.Clear
//        frmLayers.lstLayer.AddItem "Default"
//        frmLayers.lstLayer.AddItem "Destroyed Blocks"
//        frmLayers.lstLayer.AddItem "Spawned NPCs"
//        frmLayers.lstLayer.Selected(1) = False
//        frmLayers.lstLayer.Selected(2) = True
//        frmLayers.lstLayer.Selected(0) = True
//        frmLayers.cmdDelete.Enabled = False
//        frmLayers.txtLayer.Enabled = False
//        frmLevelEditor.optCursor(13).Value = True
//    End If
    }

    for(A = -128; A <= maxNPCs; A++)
        NPC[A] = blankNPC;
    numNPCs = 0;

    for(A = 1; A <= maxBlocks; A++)
        Block[A] = blankBlock;
    numBlock = 0;

    for(A = 1; A <= maxBackgrounds; A++)
        Background[A] = BlankBackground;

    for(A = 0; A <= maxSections; A++)
    {
        Background2[A] = 0;
        // bgColor[A] = 0xF89868;    // unused since SMBX64, removed
        bgMusic[A] = 0;
        level[A] = BlankLocation;
        LevelWrap[A] = false;
        LevelVWrap[A] = false;
        LevelChop[A] = 0;
        NoTurnBack[A] = false;
        UnderWater[A] = false;
        OffScreenExit[A] = false;
        CustomMusic[A] = "";
    }

    for(A = 1; A <= numWarps; A++)
        Warp[A] = blankWarp;
    numWarps = 0;

    for(A = 1; A <= numEffects; A++)
        Effect[A] = blankEffect;
    numEffects = 0;
    numBackground = 0;
    numLocked = 0;
    MidBackground = 1;
    LastBackground = 0;
    PlayerStart[1] = BlankLocation;
    PlayerStart[2] = BlankLocation;

//    If LevelEditor = True Then
//        Unload frmLevelAdv
//        noUpdate = True
//        numPlayers = 0
//        frmLevelSettings.txtMusic.Enabled = False
//        frmLevelSettings.txtMusic.Text = ""
//        frmLevelSettings.txtMusic.Enabled = True
//        frmLevelSettings.optBackgroundColor(1).Value = 1
//        frmLevelSettings.cmdWrap.Caption = "Off"
//        frmLevelSettings.optMusic(bgMusic(0)).Value = True
//        vScreenX(1) = 0
//        vScreenY(1) = 0
//        curSection = 0
//        level(curSection).Height = 20000 * (curSection - maxSections / 2)
//        level(curSection).Height = Int(level(curSection).Height / 32) * 32
//        level(curSection).Y = level(curSection).Height - 600
//        level(curSection).X = 20000 * (curSection - maxSections / 2)
//        level(curSection).X = Int(level(curSection).X / 32) * 32
//        level(curSection).Width = level(curSection).X + 800
//        vScreenY(1) = -(level(curSection).Height - 600)
//        vScreenX(1) = -level(curSection).X
//        numWarps = 1
//        frmLevelSettings.optSection(0).Value = True
//        frmLevelSettings.optBackground(0).Value = True
//        frmLevelSettings.optLevel(0).Value = True
//        For A = 1 To frmLevelSettings.optBackground.Count - 1
//            frmLevelSettings.optBackground(A).Value = False
//        Next A
//    End If

    noUpdate = false;
}

void FindStars()
{
//    int A = 0;
//    int B = 0;
//    std::string newInput;
    LevelData head;

    for(int A = 1; A <= numWarps; A++)
    {
        auto &warp = Warp[A];

        if(warp.level != STRINGINDEX_NONE)
        {
            std::string lFile = GetS(warp.level);
            addMissingLvlSuffix(lFile);

            std::string fullPath = g_dirEpisode.resolveFileCaseExistsAbs(lFile);

            if(!fullPath.empty())
            {
                if(FileFormats::OpenLevelFileHeader(fullPath, head))
                {
                    warp.maxStars = head.stars;
                    warp.curStars = 0;

                    for(const auto& star : Star)
                    {
                        if(SDL_strcasecmp(star.level.c_str(), Files::basename(GetS(warp.level)).c_str()) == 0)
                            warp.curStars++;
                    }
                }
            }

        }
    }
}

// Is there any unsupported content for this format in the level?
bool CanConvertLevel(int format, std::string* reasons)
{
    if(format == FileFormats::LVL_PGEX)
        return true;

    if(format == FileFormats::LVL_SMBX38A)
    {
        if(reasons)
        {
            *reasons = g_editorStrings.fileConvert38aUnsupported;
            *reasons += '\n';
        }
        return false;
    }

    if(format != FileFormats::LVL_SMBX64)
    {
        if(reasons)
        {
            *reasons = g_editorStrings.fileConvertFormatUnknown;
            *reasons += '\n';
        }
        return false;
    }

    bool can_convert = true;
    if(reasons)
        reasons->clear();

    bool seen_transit = false;
    bool seen_stood = false;
    bool seen_cannon = false;
    bool seen_warp_event = false;
    bool seen_stars_msg = false;
    bool seen_no_print_stars = false;
    bool seen_no_entrance_scene = false;
    bool seen_portal_warp = false;
    for(int i = 1; i <= numWarps; i++)
    {
        Warp_t& w = Warp[i];

        if(!seen_transit && w.transitEffect != LevelDoor::TRANSIT_NONE)
        {
            can_convert = false;
            seen_transit = true;
            if(reasons)
            {
                *reasons += g_editorStrings.fileConvertFeatureWarpTransit;
                *reasons += '\n';
            }
        }

        if(!seen_stood && w.stoodRequired)
        {
            can_convert = false;
            seen_stood = true;
            if(reasons)
            {
                *reasons += g_editorStrings.fileConvertFeatureWarpNeedsStand;
                *reasons += '\n';
            }
        }

        if(!seen_cannon && w.cannonExit)
        {
            can_convert = false;
            seen_cannon = true;
            if(reasons)
            {
                *reasons += g_editorStrings.fileConvertFeatureWarpCannonExit;
                *reasons += '\n';
            }
        }

        if(!seen_warp_event && w.eventEnter != EVENT_NONE)
        {
            can_convert = false;
            seen_warp_event = true;
            if(reasons)
            {
                *reasons += g_editorStrings.fileConvertFeatureWarpEnterEvent;
                *reasons += '\n';
            }
        }

        if(!seen_stars_msg && !GetS(w.StarsMsg).empty())
        {
            can_convert = false;
            seen_stars_msg = true;
            if(reasons)
            {
                *reasons += g_editorStrings.fileConvertFeatureWarpCustomStarsMsg;
                *reasons += '\n';
            }
        }

        if(!seen_no_print_stars && w.noPrintStars)
        {
            can_convert = false;
            seen_no_print_stars = true;
            if(reasons)
            {
                *reasons += g_editorStrings.fileConvertFeatureWarpNoPrintStars;
                *reasons += '\n';
            }
        }

        if(!seen_no_entrance_scene && w.noEntranceScene)
        {
            can_convert = false;
            seen_no_entrance_scene = true;
            if(reasons)
            {
                *reasons += g_editorStrings.fileConvertFeatureWarpNoStartScene;
                *reasons += '\n';
            }
        }

        if(!seen_portal_warp && w.Effect == 3)
        {
            can_convert = false;
            seen_portal_warp = true;
            if(reasons)
            {
                *reasons += g_editorStrings.fileConvertFeatureWarpPortal;
                *reasons += '\n';
            }
         }
    }

    bool seen_event_custom_music = false;
    bool seen_modern_autoscroll = false;
    for(int i = 0; i < numEvents; ++i)
    {
        Events_t& e = Events[i];

        for(int j = 0; j < numSections; ++j)
        {
            auto &ss = e.section[j];

            if(!seen_event_custom_music && !GetS(ss.music_file).empty())
            {
                can_convert = false;
                seen_event_custom_music = true;
                if(reasons)
                {
                    *reasons += g_editorStrings.fileConvertFeatureEventCustomMusic;
                    *reasons += '\n';
                }
            }

            if(!seen_modern_autoscroll && ss.autoscroll)
            {
                can_convert = false;
                seen_modern_autoscroll = true;
                if(reasons)
                {
                    *reasons += g_editorStrings.fileConvertFeatureEventAutoscroll;
                    *reasons += '\n';
                }
            }
        }
    }

    for(int i = 1; i <= numNPCs; i++)
    {
        if(NPC[i].Variant != 0)
        {
            can_convert = false;
            if(reasons)
            {
                *reasons += g_editorStrings.fileConvertFeatureNPCVariant;
                *reasons += '\n';
            }
            break;
        }
    }

    for(int i = 1; i <= numBlock; i++)
    {
        if(Block[i].Type == 90 && Block[i].forceSmashable)
        {
            can_convert = false;
            if(reasons)
            {
                *reasons += g_editorStrings.fileConvertFeatureBlockForceSmashable;
                *reasons += '\n';
            }
            break;
        }
    }

    return can_convert;
}

// Strips all unsupported content from the level.
void ConvertLevel(int format)
{
    FileFormat = format;

    if(format == FileFormats::LVL_SMBX64 || format == FileFormats::LVL_SMBX38A)
    {
        if(!FileNameFull.empty() && FileNameFull.back() == 'x')
            FileNameFull.resize(FileNameFull.size() - 1);

        if(!FullFileName.empty() && FullFileName.back() == 'x')
            FullFileName.resize(FullFileName.size() - 1);
    }
    else
    {
        if(!FileNameFull.empty() && FileNameFull.back() != 'x')
            FileNameFull += "x";

        if(!FullFileName.empty() && FullFileName.back() != 'x')
            FullFileName += "x";
    }

    if(format != FileFormats::LVL_SMBX64)
        return;

    for(int i = 1; i <= numWarps; i++)
    {
        Warp_t& w = Warp[i];

        w.transitEffect = LevelDoor::TRANSIT_NONE;
        w.stoodRequired = false;
        w.cannonExit = false;
        w.eventEnter = EVENT_NONE;
        SetS(w.StarsMsg, "");
        w.noPrintStars = false;
        w.noEntranceScene = false;

        if(w.Effect == 3)
            w.Effect = 0;
    }

    for(int i = 0; i < numEvents; ++i)
    {
        Events_t& e = Events[i];

        for(int j = 0; j < numSections; ++j)
        {
            auto &ss = e.section[j];
            SetS(ss.music_file, "");
            ss.autoscroll = false;
        }
    }

    for(int i = 1; i <= numNPCs; i++)
        NPC[i].Variant = 0;


    for(int i = 1; i <= numBlock; i++)
        Block[i].forceSmashable = 0;
}
