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

#include "sdl_proxy/sdl_stdinc.h"
#include "sdl_proxy/sdl_timer.h"

#include <json/json_rwops_input.hpp>
#include <json/json.hpp>
#include <algorithm>

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
#include "config.h"
#include "../graphics.h"
#include "../editor.h"
#include "../npc_id.h"
#include "level_file.h"
#include "main/level_save_info.h"
#include "main/level_medals.h"
#include "main/screen_progress.h"
#include "main/game_strings.h"
#include "trees.h"
#include "npc_traits.h"
#include "npc_special_data.h"
#include "graphics/gfx_camera.h"
#include "graphics/gfx_update.h"
#include "npc/npc_activation.h"
#include "npc/npc_queues.h"
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

#include "npc/section_overlap.h"


#ifdef THEXTECH_BUILD_GL_MODERN
#    include "core/opengl/gl_program_bank.h"
#endif


// warning for improper rects
static const char* s_improper_rect_warning = "Attempted to set %s %d %s to %f, setting to 0";

// used to signal a total failure in a callback
#ifdef PGEFL_CALLBACK_API
using callback_error = PGE_FileFormats_misc::callback_error;
#else
using callback_error = std::runtime_error;
#endif

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

    PGE_FileFormats_misc::RWopsTextInput in(Files::open_file(FilePath, "r"), FilePath);

    if(in.eof())
    {
        pLogWarning("File [%s] missing!", FilePath.c_str());
        return false;
    }

    if(!OpenLevelData(in, FilePath))
        return false;

    OpenLevelDataPost();

    return true;
}

struct LevelLoad
{
    std::vector<std::string> current_layers;
    std::vector<std::string> current_events;

    std::vector<layerindex_t> final_layer_index;
    std::vector<eventindex_t> final_event_index;

    SaveInfoInit si;

    int numPlayerStart = 0;

    layerindex_t layers_finalized = 0;
    eventindex_t events_finalized = 0;

    void AddCurrentLayer(const std::string& layer_name)
    {
        current_layers.push_back(layer_name);
        final_layer_index.push_back(LAYER_NONE);
    }

    layerindex_t FindLayer(const std::string& layer_name)
    {
        if(layer_name.empty())
            return LAYER_NONE;

        const char* s = layer_name.c_str();
        if(s[0] == 'D' && s[1] == 'e' && s[2] == 'f' && s[3] == 'a' && s[4] == 'u' && s[5] == 'l' && s[6] == 't' && s[7] == '\0')
            return LAYER_DEFAULT;

        for(layerindex_t A = 0; A < current_layers.size(); A++)
        {
            if(SDL_strcasecmp(current_layers[A].c_str(), layer_name.c_str()) == 0)
                return A;
        }

        if(current_layers.size() == LAYER_NONE)
            return LAYER_NONE;

        layerindex_t old_size = (layerindex_t)current_layers.size();
        AddCurrentLayer(layer_name);

        return old_size;
    }

    layerindex_t FinalizeLayer(layerindex_t current_index)
    {
        if(current_index == LAYER_NONE)
            return LAYER_NONE;

        if(final_layer_index[current_index] == LAYER_NONE)
        {
            if(layers_finalized == maxLayers)
                return LAYER_NONE;

            final_layer_index[current_index] = layers_finalized;
            layers_finalized++;
        }

        return final_layer_index[current_index];
    }

    void AddCurrentEvent(const std::string& event_name)
    {
        current_events.push_back(event_name);
        final_event_index.push_back(EVENT_NONE);
    }

    eventindex_t FindEvent(const std::string& event_name)
    {
        if(event_name.empty())
            return EVENT_NONE;

        for(eventindex_t A = 0; A < current_events.size(); A++)
        {
            if(SDL_strcasecmp(current_events[A].c_str(), event_name.c_str()) == 0)
                return A;
        }

        if(current_events.size() == EVENT_NONE)
            return EVENT_NONE;

        eventindex_t old_size = (eventindex_t)current_events.size();
        AddCurrentEvent(event_name);

        return old_size;
    }

    eventindex_t FinalizeEvent(eventindex_t current_index)
    {
        if(current_index == EVENT_NONE)
            return EVENT_NONE;

        if(final_event_index[current_index] == EVENT_NONE)
        {
            if(events_finalized == maxEvents)
                return EVENT_NONE;

            final_event_index[current_index] = events_finalized;
            events_finalized++;
        }

        return final_event_index[current_index];
    }
};

#ifdef PGEFL_CALLBACK_API
LevelLoadCallbacks OpenLevel_SetupCallbacks(LevelLoad& load);
#else
bool OpenLevel_Unpack(LevelLoad& load, LevelData& lvl);
#endif

void OpenLevel_FixLayersEvents(const LevelLoad& load);

bool OpenLevelData(PGE_FileFormats_misc::TextInput& input, const std::string FilePath)
{
    if(FilePath == ".lvl" || FilePath == ".lvlx")
        return false;

    LevelLoad load;

    qScreen = false;
    qScreen_canonical = false;
    ClearLevel();
    BlockSound();
    FreezeNPCs = false;
    CoinMode = false;

    numBlock = 0;
    numBackground = 0;
    numLocked = 0;
    numNPCs = 0;
    numWarps = 0;
    numSections = 0;

    // initialize basic layers and events left by ClearLevel()
    // they'll get updated, but this is necessary to handle PGE-X files without them included
    load.AddCurrentLayer("Default");

    for(int l = 0; l < 3; l++)
        load.FinalizeLayer(load.FindLayer(Layer[l].Name));

    for(int e = 0; e < 3; e++)
        load.FinalizeEvent(load.FindEvent(Events[e].Name));


    // set the file path and load custom configuration
    const std::string& path = (FilePath.empty()) ? input.getFilePath() : FilePath;

    FileNamePath = Files::dirname(path) + "/";
    g_dirEpisode.setCurDir(FileNamePath);

    FileName = g_dirEpisode.resolveDirCase(Files::basenameNoSuffix(path));
    g_dirCustom.setCurDir(FileNamePath + FileName);

    FileNameFull = Files::basename(path);
    FullFileName = path;


// Load Custom Stuff
    LoadCustomConfig();
    FindCustomPlayers();
    FindCustomNPCs();
    LoadCustomGFX();
    LoadCustomSound();
    FontManager::loadCustomFonts();


    load.si.begin(g_curLevelMedals.should_initialize());

#ifdef PGEFL_CALLBACK_API
    LevelLoadCallbacks callbacks = OpenLevel_SetupCallbacks(load);
    if(!FileFormats::OpenLevelFileT(input, callbacks))
    {
        pLogDebug("Failed to load [%s]", FilePath.c_str());
        load.si.on_error();
        return false;
    }
#else
    LevelData lvl;
    if(!FileFormats::OpenLevelFileT(input, lvl))
    {
        pLogWarning("Error of level \"%s\" file loading: %s (line %d).",
                    FilePath.c_str(),
                    lvl.meta.ERROR_info.c_str(),
                    lvl.meta.ERROR_linenum);
        load.si.on_error();
        return false;
    }

    if(!OpenLevel_Unpack(load, lvl))
        return false;
#endif

    OpenLevel_FixLayersEvents(load);

    return true;
}

#ifdef PGEFL_CALLBACK_API
void OpenLevel_Error(void*, FileFormatsError& e)
{
    pLogWarning("Error of level file loading: %s (line %d).",
                e.ERROR_info.c_str(),
                e.ERROR_linenum);
}
#endif

#ifdef PGEFL_CALLBACK_API
bool OpenLevel_Head(void* userdata, LevelHead& head)
#else
bool OpenLevel_Head(void* userdata, LevelData& head)
#endif
{
    LevelLoad& load = *static_cast<LevelLoad*>(userdata);

    // Level-wide settings
    maxStars = head.stars;
    LevelName = head.LevelName;

#ifdef PGEFL_CALLBACK_API
    FileFormat = head.RecentFormat;
#else
    FileFormat = head.meta.RecentFormat;
#endif

    // Level-wide extra settings
    if(!head.custom_params.empty())
    {
        // none supported yet
    }

    load.si.check_head(head);

    return true;
}

bool OpenLevel_Section(void*, LevelSection& s)
{
    // preserving indent
    {
        // load failure, invalid section ID
        if(s.id > maxSections || s.id < 0)
            throw callback_error("Invalid section id");

        if(s.id + 1 > numSections)
            numSections = s.id + 1;

        int B = s.id;

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

        // NOTE: maybe this will get stored and used as a Lua table (instead of as a string) on platforms supporting Lua
        if(LevelEditor && !s.custom_params.empty())
            SetS(SectionJSONInfo[B], s.custom_params);

#if defined(THEXTECH_BUILD_GL_MODERN)
        if(!s.custom_params.empty())
        {
            try
            {
                const nlohmann::json section_data = nlohmann::json::parse(s.custom_params);

                if(section_data.contains("effects"))
                {
                    std::string screen_effect = section_data["effects"].value("screenEffect", "");
                    if(screen_effect.empty())
                    {
                        const char* x2_screen_effects[] = {"", "wavy", "lava", "caustics", "underwater", "mist", "sepia", "grayscale", "inverted", "gameboy", "gameboy-dither"};

                        int x2_effect_idx = section_data["effects"].value("screenEffects", 0);
                        if(x2_effect_idx > 0 && x2_effect_idx < int(sizeof(x2_screen_effects) / sizeof(const char*)))
                        {
                            screen_effect += "x2-";
                            screen_effect += x2_screen_effects[x2_effect_idx];
                        }
                    }

                    if(!screen_effect.empty())
                        SectionEffect[B] = ResolveGLProgram("screen-" + screen_effect);

                    std::string particles_fg = section_data["effects"].value("fgParticles", "");
                    if(particles_fg.empty())
                    {
                        const char* x2_weather_effects[] = {"", "rain", "snow", "fog", "sandstorm", "cinders", "wisps"};

                        int x2_weather_idx = section_data["effects"].value("weather", 0);
                        if(x2_weather_idx > 0 && x2_weather_idx < int(sizeof(x2_weather_effects) / sizeof(const char*)))
                        {
                            particles_fg += "x2-";
                            particles_fg += x2_weather_effects[x2_weather_idx];
                        }
                    }

                    if(!particles_fg.empty())
                        SectionParticlesFG[B] = ResolveGLParticleSystem("particles-" + particles_fg);

                    std::string particles_bg = section_data["effects"].value("bgParticles", "");
                    if(!particles_bg.empty())
                        SectionParticlesBG[B] = ResolveGLParticleSystem("particles-" + particles_bg);
                }

                SectionLighting[B] = GLLightSystem();
                if(section_data.contains("darkness") && section_data["darkness"].value("enableDarkness", false))
                {
                    double shadow_strength = section_data["darkness"].value("shadowStrength", 0.5);
                    SectionLighting[B].shadow_strength = (float)shadow_strength;

                    int shadow_type = section_data["darkness"].value("shadowType", -1);
                    if(shadow_type == -1)
                    {
                        int x2_shadows = section_data["darkness"].value("shadows", 0);

                        if(x2_shadows == 1)
                            SectionLighting[B].system_type = GLLightSystemType::shadow_rays;
                        else if(x2_shadows == 2)
                        {
                            SectionLighting[B].system_type = GLLightSystemType::shadow_rays;
                            SectionLighting[B].shadow_strength = 1.0f;
                        }
                        else
                            SectionLighting[B].system_type = GLLightSystemType::shadow_none;
                    }

                    if(shadow_type == 1)
                        SectionLighting[B].system_type = GLLightSystemType::shadow_rays;
                    else if(shadow_type == 2)
                        SectionLighting[B].system_type = GLLightSystemType::shadow_drop;
                    else
                        SectionLighting[B].system_type = GLLightSystemType::shadow_none;

                    std::string ambient = section_data["darkness"].value("ambient", "#181828ff");

                    SectionLighting[B].ambient = XTColorString(ambient);
                }
            }
            catch(const std::exception &e)
            {
                pLogWarning("Failed to load Section %d JSON data: %s", B, e.what());

                SectionEffect[B] = LoadedGLProgramRef_t();
                SectionParticlesBG[B] = LoadedGLProgramRef_t();
                SectionParticlesFG[B] = LoadedGLProgramRef_t();
                SectionLighting[B] = GLLightSystem();
            }
        }
#endif
    }
    return true;
}


bool OpenLevel_PlayerStart(void* userdata, PlayerPoint& p)
{
    LevelLoad& load = *static_cast<LevelLoad*>(userdata);

    if(load.numPlayerStart == 2)
        return false;

    {
        // TODO: should try to use the startpoint's ID field if possible
        load.numPlayerStart++;
        int A = load.numPlayerStart;

        PlayerStart[A].X = double(p.x);
        PlayerStart[A].Y = double(p.y);
        PlayerStart[A].Width = double(p.w);
        PlayerStart[A].Height = double(p.h);

        // don't allow improper rects
        if(PlayerStart[A].Width < 0)
        {
            pLogWarning(s_improper_rect_warning, "PlayerStart", A, "Width", PlayerStart[A].Width);
            PlayerStart[A].Width = 0;
        }

        if(PlayerStart[A].Height < 0)
        {
            pLogWarning(s_improper_rect_warning, "PlayerStart", A, "Height", PlayerStart[A].Height);
            PlayerStart[A].Height = 0;
        }

        // width and height are zero in LVLX
        // (note: PGE-FL sometimes defaults these to SMBX values in PGE-X, but that logic is removed in MDX so the PGEX check here will no longer be necessary)
        if(PlayerStart[A].Width == 0 || FileFormat == FileFormats::LVL_PGEX)
            PlayerStart[A].Width = Physics.PlayerWidth[A][2];
        if(PlayerStart[A].Height == 0 || FileFormat == FileFormats::LVL_PGEX)
            PlayerStart[A].Height = Physics.PlayerHeight[A][2];

        PlayerStart[A].Direction = p.direction;

        if(load.numPlayerStart == 2)
            return false;
    }

    return true;
}

bool OpenLevel_Layer(void* userdata, LevelLayer& l)
{
    LevelLoad& load = *static_cast<LevelLoad*>(userdata);

    {
        int A = load.FinalizeLayer(load.FindLayer(l.name));

        // too many layers
        if(A == LAYER_NONE)
            throw callback_error("Too many layers");

        // update layer count
        if(A + 1 > numLayers)
            numLayers = A + 1;

        auto &layer = Layer[A];

        layer = Layer_t();

        layer.Name = l.name;
        layer.Hidden = l.hidden;
        // hide layers after everything is done
    }

    return true;
}

bool OpenLevel_Event(void* userdata, LevelSMBX64Event& e)
{
    LevelLoad& load = *static_cast<LevelLoad*>(userdata);

    {
        int A = load.FinalizeEvent(load.FindEvent(e.name));

        // too many events
        if(A == EVENT_NONE)
            throw callback_error("Too many events");

        // update events count
        if(A + 1 > numEvents)
            numEvents = A + 1;

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
            layerindex_t found = load.FindLayer(l);
            if(found != LAYER_NONE)
                event.HideLayer.push_back(found);
        }
        event.ShowLayer.clear();
        for(std::string& l : e.layers_show)
        {
            layerindex_t found = load.FindLayer(l);
            if(found != LAYER_NONE)
                event.ShowLayer.push_back(found);
        }
        event.ToggleLayer.clear();
        for(std::string& l : e.layers_toggle)
        {
            layerindex_t found = load.FindLayer(l);
            if(found != LAYER_NONE)
                event.ToggleLayer.push_back(found);
        }

        // this was done in ClearLevel, but may be necessary here to refresh matters
        for(int B = 0; B <= maxSections; B++)
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

        // unpack section settings
        for(auto &s : e.sets)
        {
            // this has a different meaning (padding) when we are not actually using callbacks
            if(s.id == -1)
                continue;
            // invalid section ID
            else if(s.id > maxSections || s.id < 0)
                throw callback_error("Invalid section id");

            // relies on the fact that this is an ARRAY at TheXTech's side
            auto &ss = event.section[s.id];

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

        event.TriggerEvent = load.FindEvent(e.trigger);

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
        event.MoveLayer = load.FindLayer(e.movelayer);
        event.SpeedX = float(e.layer_speed_x);
        event.SpeedY = float(e.layer_speed_y);

        event.AutoX = float(e.move_camera_x);
        event.AutoY = float(e.move_camera_y);
        event.AutoSection = int(e.scroll_section);
    }

    return true;
}

bool OpenLevel_Block(void* userdata, LevelBlock& b)
{
    LevelLoad& load = *static_cast<LevelLoad*>(userdata);

    {
        numBlock++;
        if(numBlock > maxBlocks)
        {
            numBlock = maxBlocks;
            return false;
        }

        auto &block = Block[numBlock];

        block = Block_t();

        block.Location.X = double(b.x);
        block.Location.Y = double(b.y);
        block.Location.Height = double(b.h);
        block.Location.Width = double(b.w);

        // don't allow improper rects
        if(block.Location.Width < 0)
        {
            pLogWarning(s_improper_rect_warning, "Block", numBlock, "Width", block.Location.Width);
            block.Location.Width = 0;
        }

        if(block.Location.Height < 0)
        {
            pLogWarning(s_improper_rect_warning, "Block", numBlock, "Height", block.Location.Height);
            block.Location.Height = 0;
        }

        block.Type = int(b.id);
        block.DefaultType = block.Type;

        block.Special = int(b.npc_id > 0 ? b.npc_id + 1000 : -1 * b.npc_id);

        switch(block.Special) // Replace some legacy NPC codes with new
        {
        case 100: block.Special = 1000 + NPCID_POWER_S3; break;
        case 102: block.Special = 1000 + NPCID_FIRE_POWER_S3; break;
        case 103: block.Special = 1000 + NPCID_LEAF_POWER; break;
        case 105: block.Special = 1000 + NPCID_PET_GREEN; break;
        default: break;
        }

        block.DefaultSpecial = block.Special;

        block.forceSmashable = false;
        if(b.id == 90)
        {
            // if(lvl.meta.RecentFormat == LevelData::SMBX64 && lvl.meta.RecentFormatVersion < 20)
            //     block.forceSmashable = true; // Restore bricks algorithm for turn blocks for SMBX19 and lower
            // else
            block.forceSmashable = (bool)b.special_data; // load it if set in the modern format
        }

        block.Invis = b.invisible;
        block.Slippy = b.slippery;
        block.Layer = load.FindLayer(b.layer);
        block.TriggerDeath = load.FindEvent(b.event_destroy);
        block.TriggerHit = load.FindEvent(b.event_hit);
        block.TriggerLast = load.FindEvent(b.event_emptylayer);

        if(IF_OUTRANGE(block.Type, 0, maxBlockType)) // Drop ID to 1 for blocks of out of range IDs
        {
            pLogWarning("Block-%d ID is out of range (max types %d), reset to Block-1", block.Type, maxBlockType);
            block.Type = 1;
        }
    }

    return true;
}

bool OpenLevel_Background(void* userdata, LevelBGO& b)
{
    LevelLoad& load = *static_cast<LevelLoad*>(userdata);

    {
        numBackground++;
        if(numBackground > maxBackgrounds)
        {
            numBackground = maxBackgrounds;
            return false;
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

        bgo.Layer = load.FindLayer(b.layer);
        bgo.Location.Width = GFXBackgroundWidth[bgo.Type];
        bgo.Location.Height = BackgroundHeight[bgo.Type];

        bgo.SetSortPriority(b.z_mode, std::round(b.z_offset));
    }

    return true;
}

bool OpenLevel_NPC(void* userdata, LevelNPC& n)
{
    LevelLoad& load = *static_cast<LevelLoad*>(userdata);

    {
        bool variantHandled = false;

        numNPCs++;
        if(numNPCs > maxNPCs)
        {
            numNPCs = maxNPCs;
            return false;
        }

        auto &npc = NPC[numNPCs];

        npc = NPC_t();

        npc.Location.X = n.x;
        npc.Location.Y = n.y;
        if(!LevelEditor)
            npc.Location.Y -= 0.01;
        npc.Direction = n.direct;

        if(n.id > maxNPCType) // Drop ID to 1 for NPCs of out of range IDs
        {
            pLogWarning("NPC-%d ID is out of range (max types %d), reset to NPC-1", (int)n.id, maxNPCType);
            npc.Type = NPCID(1);
        }
        else
            npc.Type = NPCID(n.id);

        if(npc.Type == NPCID_ITEM_BURIED || npc.Type == NPCID_ITEM_POD ||
           npc.Type == NPCID_ITEM_BUBBLE || npc.Type == NPCID_ITEM_THROWER)
        {
            npc.Special = (vbint_t)n.contents;
            npc.DefaultSpecial = npc.Special;
            npc.Variant = n.special_data;
            variantHandled = true;
        }

        if(npc.Type == NPCID_DOOR_MAKER || npc.Type == NPCID_MAGIC_DOOR ||
          (npc.Type == NPCID_ITEM_BURIED && n.contents == NPCID_DOOR_MAKER))
        {
            npc.Special2 = (vbint_t)n.special_data;
            npc.DefaultSpecial2 = npc.Special2;
        }

        if(NPCIsAParaTroopa(npc))
        {
            npc.Special = (vbint_t)n.special_data;
            npc.DefaultSpecial = npc.Special;
        }

        if(npc->IsFish)
        {
            npc.Special = (vbint_t)n.special_data;
            npc.DefaultSpecial = npc.Special;
        }

        if(npc.Type == NPCID_FIRE_CHAIN)
        {
            npc.Special = (vbint_t)n.special_data;
            npc.DefaultSpecial = npc.Special;
        }

        if(npc.Type == NPCID_STAR_EXIT || npc.Type == NPCID_STAR_COLLECT || npc.Type == NPCID_MEDAL)
        {
            npc.Variant = n.special_data;
            variantHandled = true;
        }

        // don't load anything for SMBX64 files
        if(FileFormat == FileFormats::LVL_SMBX64)
        {
            npc.Variant = 0;
        }
        // only load Variant for NPCs that support it
        else if(find_Variant_Data(npc.Type))
        {
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

        npc.Layer = load.FindLayer(n.layer);
        npc.TriggerActivate = load.FindEvent(n.event_activate);
        npc.TriggerDeath = load.FindEvent(n.event_die);
        npc.TriggerTalk = load.FindEvent(n.event_talk);
        npc.TriggerLast = load.FindEvent(n.event_emptylayer);
        npc.AttLayer = load.FindLayer(n.attach_layer);

        npc.DefaultType = npc.Type;
        npc.Location.Width = npc->TWidth;
        npc.Location.Height = npc->THeight;
        npc.DefaultLocationX = npc.Location.X;
        npc.DefaultLocationY = npc.Location.Y;
        npc.DefaultDirection = npc.Direction;

        // allow every NPC to be active for one frame to initialize its internal state
        npc.TimeLeft = 1;
        npc.Active = true;
        npc.JustActivated = 1;
    }

    // update the level's save info based on the NPC
    load.si.check_npc(n);

    return true;
}

bool OpenLevel_Warp(void* userdata, LevelDoor& w)
{
    LevelLoad& load = *static_cast<LevelLoad*>(userdata);

    w.isSetIn = (!w.lvl_i);
    w.isSetOut = (!w.lvl_o || (w.lvl_i));

    if(!w.isSetIn && w.isSetOut)
    {
        w.ix = w.ox;
        w.iy = w.oy;
    }

    if(!w.isSetOut && w.isSetIn)
    {
        w.ox = w.ix;
        w.oy = w.iy;
    }

    {
        numWarps++;
        if(numWarps > maxWarps)
        {
            numWarps = maxWarps;
            return false;
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
            std::string level_name;
            validateLevelName(level_name, w.lname);
            SetS(warp.level, level_name);
        }

        warp.LevelWarp = int(w.warpto);
        warp.LevelEnt = w.lvl_i;

        warp.MapWarp = w.lvl_o;
        warp.MapX = int(w.world_x);
        warp.MapY = int(w.world_y);

        warp.Stars = w.stars;
        warp.Layer = load.FindLayer(w.layer);
        warp.Hidden = w.unknown;

        warp.NoYoshi = w.novehicles;
        warp.WarpNPC = w.allownpc;
        warp.Locked = w.locked;

        // custom fields:
        warp.twoWay = w.two_way;

        warp.cannonExit = w.cannon_exit;
        warp.cannonExitSpeed = w.cannon_exit_speed;
        warp.eventEnter = load.FindEvent(w.event_enter);
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
    }

    return true;
}

bool OpenLevel_Water(void* userdata, LevelPhysEnv& w)
{
    LevelLoad& load = *static_cast<LevelLoad*>(userdata);

    {
        numWater++;
        if(numWater > maxWater)
        {
            numWater = maxWater;
            return false;
        }

        auto &water = Water[numWater];

        water = Water_t();

        water.Location.X = w.x;
        water.Location.Y = w.y;
        water.Location.Width = w.w;
        water.Location.Height = w.h;

        // don't allow improper rects
        if(water.Location.Width < 0)
        {
            pLogWarning(s_improper_rect_warning, "Water", numWater, "Width", water.Location.Width);
            water.Location.Width = 0;
        }

        if(water.Location.Height < 0)
        {
            pLogWarning(s_improper_rect_warning, "Water", numWater, "Height", water.Location.Height);
            water.Location.Height = 0;
        }

        water.Buoy = w.buoy;
        water.Quicksand = w.env_type;
        water.Layer = load.FindLayer(w.layer);
    }

    return true;
}

void OpenLevel_FixLayersEvents(const LevelLoad& load)
{
    // Everything is loaded.
    // Now we fix the layers and events that got temporary indexes.
    // We also do some NPC logic that depends on the section boundaries.
    for(int A = 1; A <= numBlock; A++)
    {
        if(Block[A].Layer != LAYER_NONE)
            Block[A].Layer = load.final_layer_index[Block[A].Layer];

        if(Block[A].TriggerDeath != EVENT_NONE)
            Block[A].TriggerDeath = load.final_event_index[Block[A].TriggerDeath];

        if(Block[A].TriggerHit != EVENT_NONE)
            Block[A].TriggerHit = load.final_event_index[Block[A].TriggerHit];

        if(Block[A].TriggerLast != EVENT_NONE)
            Block[A].TriggerLast = load.final_event_index[Block[A].TriggerLast];
    }

    for(int A = 1; A <= numBackground; A++)
    {
        if(Background[A].Layer != LAYER_NONE)
            Background[A].Layer = load.final_layer_index[Background[A].Layer];
    }

    // Prepare for NPC logic
    CalculateSectionOverlaps();
    int checkPointId = 0;

    for(int A = 1; A <= numNPCs; A++)
    {
        if(NPC[A].Layer != LAYER_NONE)
            NPC[A].Layer = load.final_layer_index[NPC[A].Layer];

        if(NPC[A].AttLayer != LAYER_NONE)
            NPC[A].AttLayer = load.final_layer_index[NPC[A].AttLayer];

        if(NPC[A].TriggerActivate != EVENT_NONE)
            NPC[A].TriggerActivate = load.final_event_index[NPC[A].TriggerActivate];

        if(NPC[A].TriggerDeath != EVENT_NONE)
            NPC[A].TriggerDeath = load.final_event_index[NPC[A].TriggerDeath];

        if(NPC[A].TriggerTalk != EVENT_NONE)
            NPC[A].TriggerTalk = load.final_event_index[NPC[A].TriggerTalk];

        if(NPC[A].TriggerLast != EVENT_NONE)
            NPC[A].TriggerLast = load.final_event_index[NPC[A].TriggerLast];

        CheckSectionNPC(A);


        // Extra NPC load logic
        NPC_t& npc = NPC[A];

        if(npc.Type == NPCID_CHECKPOINT) // Is a checkpoint
        {
            checkPointId++;
            if(g_config.fix_vanilla_checkpoints)
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
                (NPCID(npc.Special) == NPCID_STAR_EXIT || NPCID(npc.Special) == NPCID_STAR_COLLECT)) // Is a container that has a star inside
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
                if(NPCID(npc.Special) == NPCID_STAR_COLLECT)
                    npc.Killed = 9;
            }
        }

        syncLayers_NPC(A);
    }

    for(int A = 1; A <= numWarps; A++)
    {
        if(Warp[A].Layer != LAYER_NONE)
            Warp[A].Layer = load.final_layer_index[Warp[A].Layer];

        if(Warp[A].eventEnter != EVENT_NONE)
            Warp[A].eventEnter = load.final_event_index[Warp[A].eventEnter];

        syncLayers_Warp(A);
    }

    for(int A = 1; A <= numWater; A++)
    {
        if(Water[A].Layer != LAYER_NONE)
            Water[A].Layer = load.final_layer_index[Water[A].Layer];

        syncLayers_Water(A);
    }

    for(int A = 0; A < numEvents; A++)
    {
        if(Events[A].MoveLayer != LAYER_NONE)
            Events[A].MoveLayer = load.final_layer_index[Events[A].MoveLayer];

        if(Events[A].TriggerEvent != EVENT_NONE)
            Events[A].TriggerEvent = load.final_event_index[Events[A].TriggerEvent];

        // fix all layer indices
        for(int i = 0; i < 3; i++)
        {
            auto& arr = (i == 0) ? Events[A].ShowLayer
                : (i == 1) ? Events[A].HideLayer
                : Events[A].ToggleLayer;

            auto out_it = arr.begin();
            for(auto in_it = out_it; in_it != arr.end(); ++in_it)
            {
                layerindex_t l = *in_it;

                if(l != LAYER_NONE)
                    l = load.final_layer_index[l];

                if(l != LAYER_NONE)
                    *(out_it++) = l;
            }

            arr.resize(out_it - arr.begin());
        }
    }

    LAYER_USED_P_SWITCH = FindLayer(LAYER_USED_P_SWITCH_TITLE);
}

#ifdef PGEFL_CALLBACK_API

LevelLoadCallbacks OpenLevel_SetupCallbacks(LevelLoad& load)
{
    LevelLoadCallbacks callbacks;

    callbacks.on_error = OpenLevel_Error;
    callbacks.load_head = OpenLevel_Head;
    callbacks.load_section = OpenLevel_Section;
    callbacks.load_startpoint = OpenLevel_PlayerStart;
    callbacks.load_block = OpenLevel_Block;
    callbacks.load_bgo = OpenLevel_Background;
    callbacks.load_npc = OpenLevel_NPC;
    callbacks.load_warp = OpenLevel_Warp;
    callbacks.load_phys = OpenLevel_Water;
    callbacks.load_layer = OpenLevel_Layer;
    callbacks.load_event = OpenLevel_Event;

    callbacks.userdata = &load;

    return callbacks;
}
#else // #ifdef PGEFL_CALLBACK_API

bool OpenLevel_Unpack(LevelLoad& load, LevelData& lvl)
{
    try
    {
        OpenLevel_Head(&load, lvl);

        for(auto &s : lvl.sections)
        {
            if(!OpenLevel_Section(&load, s))
                break;
        }
        for(auto &p : lvl.players)
        {
            if(!OpenLevel_PlayerStart(&load, p))
                break;
        }
        for(auto &b : lvl.blocks)
        {
            if(!OpenLevel_Block(&load, b))
                break;
        }
        for(auto &b : lvl.bgo)
        {
            if(!OpenLevel_Background(&load, b))
                break;
        }
        for(auto &n : lvl.npc)
        {
            if(!OpenLevel_NPC(&load, n))
                break;
        }
        for(auto &w : lvl.doors)
        {
            if(!OpenLevel_Warp(&load, w))
                break;
        }
        for(auto &w : lvl.physez)
        {
            if(!OpenLevel_Water(&load, w))
                break;
        }
        for(auto &l : lvl.layers)
        {
            if(!OpenLevel_Layer(&load, l))
                break;
        }
        for(auto &e : lvl.events)
        {
            if(!OpenLevel_Event(&load, e))
                break;
        }
    }
    catch(const callback_error& e)
    {
        pLogWarning("Error of level \"%s\" file loading: %s.",
                    lvl.meta.filename.c_str(),
                    e.what());

        return false;
    }

    return true;
}
#endif // #else // #ifdef PGEFL_CALLBACK_API

void OpenLevelDataPost()
{
    TranslateEpisode tr;

    if(!GameMenu && !LevelEditor)
        tr.loadLevelTranslation(FileNameFull);


    IsEpisodeIntro = (StartLevel == FileNameFull);

    if(IsEpisodeIntro)
    {
        IsHubLevel = NoMap;
        FileRecentSubHubLevel.clear();
    }

    if(!IsHubLevel)
    {
        IsHubLevel = std::find(SubHubLevels.begin(), SubHubLevels.end(), FileNameFull) != SubHubLevels.end();

        if(IsHubLevel)
            FileRecentSubHubLevel = FileNameFull;
    }

    // TODO: disable this if the file indicates that it is already sorted
    if(g_config.emulate_classic_block_order && FileFormat == FileFormats::LVL_PGEX)
    {
        qSortBlocksX(1, numBlock);

        int col_start = 1;
        int col_end = 2;
        for(; col_end <= numBlock; col_end++)
        {
            if(Block[col_end].Location.X > Block[col_start].Location.X)
            {
                qSortBlocksY(col_start, col_end - 1);
                col_start = col_end;
            }
        }

        // col_end = numBlock + 1
        qSortBlocksY(col_start, col_end - 1);
        qSortBackgrounds(1, numBackground);
    }
    else
        qSortBackgrounds(1, numBackground, false);

    // FindBlocks();
    UpdateBackgrounds();
    // FindSBlocks();
    syncLayersTrees_AllBlocks();
    syncLayers_AllBGOs();

    NPC_ConstructCanonicalSet();

    // moved the old event/layer loading code to the top
    // since it is needed before loading objects now

    for(int A = 0; A < numLayers; A++)
    {
        if(Layer[A].Hidden)
            HideLayer(A, true);
    }


    if(LevelEditor)
    {
        if(numSections < 21)
            numSections = 21;

        ResetSectionScrolls();
        SetSection(curSection);
    }
    else
    {
        FindStars();
        LevelMacro = LEVELMACRO_OFF;
        for(int A = 0; A < numSections; A++) // Automatically correct 608 section height to 600
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
                bgo.Location = w.Entrance;
                bgo.Type = 98;
                bgo.Location.Width = 16;
                syncLayers_BGO(B);

                if(w.twoWay)
                {
                    B++;
                    numLocked++;
                    auto &bgo2 = Background[B];
                    bgo2 = bgo;
                    bgo2.Location = w.Exit;
                    bgo2.Location.Width = 16;
                    syncLayers_BGO(B);
                }
            }
        }
    }

    if(!LevelEditor)
    {
        g_curLevelMedals.prepare_lvl();
        OrderMedals();
    }

    // If too much locks
    SDL_assert_release(numBackground + numLocked <= (maxBackgrounds + maxWarps));

    SoundPause[SFX_Camera] = 100;
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
    const SpeedlessLocation_t BlankSpeedless;
    const Effect_t blankEffect = Effect_t();
    NPCTraits[NPCID_MEDAL].Score = 6;
    RestoreWorldStrings();
    LevelName.clear();
    IsHubLevel = false;
    // default file format if level header is missing
    FileFormat = FileFormats::LVL_PGEX;
    // removed because the same logic is called inside of LoadCustomConfig()
    // ResetCustomConfig();
    SetupPhysics();
    LoadNPCDefaults();
    LoadPlayerDefaults();
    noUpdate = true;
    BlocksSorted = true;
    qScreen = false;
    qScreen_canonical = false;

#ifdef __16M__
    XRender::clearAllTextures();
#endif

    SectionJSONInfo.fill(STRINGINDEX_NONE);

#ifdef THEXTECH_BUILD_GL_MODERN
    SectionEffect.fill(LoadedGLProgramRef_t());
    SectionParticlesBG.fill(LoadedGLProgramRef_t());
    SectionParticlesFG.fill(LoadedGLProgramRef_t());
    SectionLighting.fill(GLLightSystem());
#endif

    UnloadCustomGFX();
    doShakeScreenClear();
    ResetCameraPanning();
    treeLevelCleanAll();
    FontManager::clearLevelFonts();

    invalidateDrawBlocks();
    invalidateDrawBGOs();
    NPCQueues::clear();

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
    g_curLevelMedals.reset_lvl();

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
        level[A] = BlankSpeedless;
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

    noUpdate = false;
}

void FindStars()
{
//    int A = 0;
//    int B = 0;
//    std::string newInput;
    LevelData tempData;

    uint32_t start_time = SDL_GetTicks();

    for(int A = 1; A <= numWarps; A++)
    {
        IndicateProgress(start_time, (double)A / numWarps, g_gameStrings.messageScanningLevels);

        auto &warp = Warp[A];

        if(warp.level != STRINGINDEX_NONE)
        {
            std::string lFile = GetS(warp.level);

            warp.curStars = 0;

            for(const auto& star : Star)
            {
                if(SDL_strcasecmp(star.level.c_str(), Files::basename(lFile).c_str()) == 0)
                    warp.curStars++;
            }

            if(warp.save_info().inited())
                continue;

            // set the warp's save info index

            // check world levels
            for(uint16_t idx = 1; idx != 0x7FFF && idx <= numWorldLevels; ++idx)
            {
                const auto& l = WorldLevel[idx];

                if(l.FileName == lFile)
                {
                    warp.save_info_idx = 0x8000 + idx;
                    break;
                }
            }

            if(warp.save_info().inited())
                continue;

            // check existing level warp save entries
            for(uint16_t idx = 0; idx != 0x7FFF && idx < LevelWarpSaveEntries.size(); ++idx)
            {
                const auto& e = LevelWarpSaveEntries[idx];

                if(e.levelPath == lFile)
                {
                    warp.save_info_idx = idx;
                    break;
                }
            }

            if(warp.save_info().inited())
                continue;

            // don't overflow the LevelWarpSaveEntries array
            if(LevelWarpSaveEntries.size() >= 0x7FFF)
                continue;

            // add a new save entry if the file exists
            std::string fullPath = g_dirEpisode.resolveFileCaseExistsAbs(lFile);

            if(!fullPath.empty())
            {
                LevelSaveInfo_t info = InitLevelSaveInfo(fullPath, tempData);

                if(info.inited())
                {
                    warp.save_info_idx = LevelWarpSaveEntries.size();
                    LevelWarpSaveEntries.push_back({lFile, info});
                }
            }
        }
    }
}

#if 0
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

    for(int i = 1; i <= numBackground; i++)
    {
        if(Background[i].GetCustomLayer() || Background[i].GetCustomOffset())
        {
            can_convert = false;
            if(reasons)
            {
                *reasons += g_editorStrings.fileConvertFeatureBgoOrder;
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

    for(int i = 1; i <= numBackground; i++)
        Background[i].SetSortPriority(0, 0);

    for(int i = 1; i <= numNPCs; i++)
        NPC[i].Variant = 0;


    for(int i = 1; i <= numBlock; i++)
        Block[i].forceSmashable = 0;
}
#endif
