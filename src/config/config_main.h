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

#pragma once
#ifndef CONFIG_MAIN_H
#define CONFIG_MAIN_H

#include <PGE_File_Formats/file_formats.h>

#include "sdl_proxy/sdl_audio.h"
#include "sound.h"
#include "screen.h"

#include "Logger/logger_level.h"

#include "config/config_base.hpp"
#include "config/config_hooks.h"

struct saveUserData;

template<bool writable>
class _Config_t
{
public:
    struct Scope
    {
        enum
        {
            None = 0,
            UserGlobal = 1 << 0,
            UserEpisode = 1 << 1,
            UserFile = 1 << 2,

            UserLocal = UserEpisode | UserFile,
            User = UserGlobal | UserLocal,

            CreatorGlobal = 1 << 3,
            CreatorEpisode = 1 << 4,
            CreatorFile = 1 << 5,

            CreatorLocal = CreatorEpisode | CreatorFile,
            Creator = CreatorGlobal | CreatorLocal,

            Episode = UserEpisode | CreatorEpisode,
            File = UserFile | CreatorFile,
            Local = Episode | File,
            Global = UserGlobal | CreatorGlobal,
            All = 0xff,
        };
    };

protected:
    template<class value_t> using opt = ConfigOption_t<writable, value_t>;
    template<class value_t> using opt_enum = ConfigEnumOption_t<writable, value_t>;
    using section = ConfigSection_t<writable>;
    using subsection = ConfigSubSection_t<writable>;
    using language_t = ConfigLanguage_t<writable>;
    using rendermode_t = ConfigRenderMode_t<writable>;

    template<class value_t>
    static constexpr value_t defaults(value_t value)
    {
        return value;
    }

public:
    const _Config_t<false>* m_base = nullptr;
    std::vector<BaseConfigOption_t<writable>*> m_options;
    uint8_t m_scope = Scope::All;

    _Config_t() {}
    _Config_t(const _Config_t<false>& base, uint8_t scope) : m_base(&base), m_scope(scope) {}

    void Clear();

    void LoadLegacySettings(IniProcessing* ini, ConfigSetLevel level = ConfigSetLevel::set);
    void LoadLegacyCompat(IniProcessing* ini, ConfigSetLevel level = ConfigSetLevel::set);

    void SetFromDefaults(ConfigSetLevel level = ConfigSetLevel::set);

    void DisableBugfixes(ConfigSetLevel level = ConfigSetLevel::set);

    void UpdateFromIni(IniProcessing* ini, ConfigSetLevel level = ConfigSetLevel::set);
    void UpdateFrom(const _Config_t<true>& other, ConfigSetLevel level = ConfigSetLevel::set);
    void UpdateFromCompat(CompatMode compat_mode, ConfigSetLevel level = ConfigSetLevel::set);

    void LoadEpisodeConfig(const saveUserData& userdata);

    void SaveToIni(IniProcessing* ini);
    void SaveEpisodeConfig(saveUserData& userdata);

    // for translations (note: you'll get a linker error if you invoke these on a writable Config_t.)
    void reset_options();
    void make_translation(XTechTranslate& translate);


    /* ---- Main ----*/
    section main{this, Scope::All, "main", "Main", nullptr};

    language_t language{this, defaults(std::string("auto")), {}, Scope::UserGlobal,
        "language", "Language", nullptr,
        config_language_set};

#ifdef ENABLE_XTECH_DISCORD_RPC
    opt<bool> discord_rpc{this, defaults(false), {}, Scope::UserGlobal,
        "discord-rpc", "Discord Integration", "Share your play data on Discord!",
        config_integrations_set};
#endif

#ifndef NO_WINDOW_FOCUS_TRACKING
    opt<bool> background_work{this, defaults(false), {}, Scope::UserGlobal,
        "background-work", "Run in background", "Play with joystick while game is unfocused"};
#else
    static constexpr bool background_work = true;
#endif

    static constexpr bool enable_editor = true;

    /* ---- Main - Frame Timing ----*/
    subsection main_frame_timing{this, "timing", "Frame Timing"};

    opt<bool> enable_frameskip{this, defaults(false), {}, Scope::UserGlobal,
        "frame-skip", "Frameskip", "Skip frames to maintain game speed"};

    opt<bool> unlimited_framerate{this, defaults(false), {}, Scope::UserGlobal,
        "unlimited-framerate", "Unlimited framerate", "Allow framerate above 66 FPS"};

    opt<bool> render_vsync{this, defaults(false), {}, Scope::UserGlobal,
        "vsync", "V-Sync", "Sync frames to screen refresh, reduces tearing",
        config_res_set};

    /* ---- Main - Multiplayer ----*/
    subsection main_multiplayer{this, "multiplayer", "Multiplayer"};

    opt_enum<int> two_screen_mode{this,
        {
            {MultiplayerPrefs::Dynamic, "smbx", "SMBX"},
            {MultiplayerPrefs::Shared, "shared", "Shared"},
            {MultiplayerPrefs::Split, "split", "Left/Right"},
            {MultiplayerPrefs::TopBottom, "topbottom", "Top/Bottom"},
        },
        defaults(MultiplayerPrefs::Dynamic), {CompatClass::critical_update, MultiplayerPrefs::Dynamic}, Scope::UserGlobal,
        "two-screen-mode", "2P screen mode", nullptr,
        config_screenmode_set
    };

    opt_enum<int> four_screen_mode{this,
        {
            {MultiplayerPrefs::Shared, "shared", "Shared"},
            {MultiplayerPrefs::Split, "split", "Split"},
        },
        defaults(MultiplayerPrefs::Shared), {CompatClass::critical_update, MultiplayerPrefs::Shared}, Scope::UserGlobal,
        "four-screen-mode", "4P screen mode", nullptr,
        config_screenmode_set
    };


    /* ---- Video ----*/
    section video{this, Scope::UserGlobal, "video", "Video", nullptr};

    /* ---- Video - System ----*/
    subsection video_system{this, "video-system", "System"};

#ifndef RENDER_FULLSCREEN_ALWAYS
    opt<bool> fullscreen{this, defaults(false), {}, Scope::UserGlobal,
        "fullscreen", "Fullscreen", nullptr,
        config_fullscreen_set};
#else
    static constexpr bool fullscreen = true;
#endif

    opt_enum<std::pair<int, int>> internal_res{this,
        {
            {{480, 320}, "gba", "480x320 (GBA)"},
            {{512, 384}, "nds", "512x384 (NDS)"},
            {{512, 448}, "snes", "512x448 (SNES)"},
            {{640, 480}, "vga", "640x480 (VGA)"},
            {{768, 432}, "hello", "768x432 (HELLO)"},
            {{800, 480}, "3ds", "800x480 (3DS)"},
            {{800, 600}, "smbx", "800x600 (SMBX)"},
#ifndef __16M__
            {{0, 600}, "smbx-wide", "SMBX (wide)"},
#endif
            {{1280, 720}, "hd", "1280x720 (HD)"},
            {{0, 0}, "dynamic", "Dynamic"},
        },
        defaults(std::pair<int, int>({0, 0})), {}, Scope::UserGlobal,
        "internal-res", "Screen size", "Resolution of gameplay field",
        config_res_set
    };

    static constexpr bool dynamic_width = false;

    enum ScaleModes
    {
        SCALE_DYNAMIC_INTEGER = -3,
        SCALE_DYNAMIC_NEAREST = -2,
        SCALE_DYNAMIC_LINEAR = -1,
        SCALE_FIXED_05X = 0,
        SCALE_FIXED_1X = 1,
        SCALE_FIXED_2X = 2,
    };

#if defined(__WII__) || !defined(PGE_MIN_PORT)
    opt_enum<int> scale_mode{this,
        {
#   ifdef __WII__
            {SCALE_DYNAMIC_NEAREST, "full", "Full"},
            {SCALE_FIXED_1X, "center", "Center"},
#   else
            {SCALE_DYNAMIC_INTEGER, "integer", "Integer"},
            {SCALE_DYNAMIC_NEAREST, "nearest", "Nearest"},
            {SCALE_DYNAMIC_LINEAR, "linear", "Smooth"},
            {SCALE_FIXED_05X, "0.5x", "0.5x"},
            {SCALE_FIXED_1X, "1x", "1x"},
            {SCALE_FIXED_2X, "2x", "2x"},
#   endif
        },
        defaults(SCALE_DYNAMIC_NEAREST), {}, Scope::UserGlobal,
        "scale-mode", "Scale mode", nullptr,
        config_res_set
    };
#else
    static constexpr int scale_mode = SCALE_DYNAMIC_NEAREST;
#endif

    /* ---- Video - Meta Info ----*/
    subsection info_meta{this, "info-meta", "Onscreen meta info"};

    static constexpr bool show_backdrop = true;

    opt<bool> show_controllers{this, defaults(false), {}, Scope::UserGlobal,
        "display-controllers", "Controls activity", "Show which buttons players press"};

    opt<bool> show_fps{this, defaults(false), {}, Scope::UserGlobal,
        "show-fps", "Framerate", nullptr};

    enum
    {
        BATTERY_STATUS_OFF = 0,
        BATTERY_STATUS_LOW,
        BATTERY_STATUS_FULLSCREEN,
        BATTERY_STATUS_ALWAYS_ON,
    };

#if !defined(__WII__) && !defined(__WIIU__)
    opt_enum<int> show_battery_status{this,
        {
            {BATTERY_STATUS_OFF, "off", "Off", nullptr},
            {BATTERY_STATUS_LOW, "low", "Low", "Show when the battery is low"},
#ifndef RENDER_FULLSCREEN_ALWAYS
            {BATTERY_STATUS_FULLSCREEN, "fullscreen", "Fullscreen", "Show when the game is fullscreen"},
#endif
            {BATTERY_STATUS_ALWAYS_ON, "on", "Always", nullptr},
            {BATTERY_STATUS_OFF, "never"},
            {BATTERY_STATUS_ALWAYS_ON, "always"},
        },
        defaults<int>(BATTERY_STATUS_OFF), {}, Scope::UserGlobal,
        "battery-status", "Device battery status", nullptr};
#else
    static constexpr bool show_battery_status = BATTERY_STATUS_OFF;
#endif

    /* ---- Video - Game Info ----*/
    subsection info_game{this, "info-game", "Onscreen game info"};

    opt<bool> show_fails_counter{this, defaults(false), {}, Scope::UserGlobal,
        "show-fails-counter", "Fails counter", "Show fails on the episode and current level"};

    enum
    {
        MEDALS_SHOW_OFF = 0,
        MEDALS_SHOW_GOT,
        MEDALS_SHOW_COUNTS,
        MEDALS_SHOW_FULL,
    };
#if 0
    opt_enum<int> medals_show_policy{this,
        {
            {MEDALS_SHOW_OFF, "hide", "Off", "Don't show medals"},
            {MEDALS_SHOW_GOT, "collected-only", "Collected", "Show counts of gotten medals"},
            {MEDALS_SHOW_COUNTS, "counts-only", "Counts", "Show counts of gotten and available medals"},
            {MEDALS_SHOW_FULL, "show-full", "Full", "Show order of gotten and available medals"},
        },
        defaults<int>(MEDALS_SHOW_FULL), {}, Scope::UserGlobal,
        "show-medals-policy", "Medals policy"};
#else
    static constexpr int medals_show_policy = MEDALS_SHOW_FULL;
#endif

    opt<bool> show_medals_counter{this,
        defaults<bool>(true), {CompatClass::pure_preference, false}, Scope::UserGlobal,
        "show-medals-counter", "Medals counter", "Show medals in HUD and at level entrance"};

    // used elsewhere to track creator-specified options
    enum
    {
        MAP_STARS_UNSPECIFIED = 0,
        MAP_STARS_HIDE,
        MAP_STARS_COLLECTED,
        MAP_STARS_SHOW,
    };

    opt<bool> world_map_stars_show{this, defaults(true), {CompatClass::pure_preference, false}, Scope::UserGlobal,
        "world-map-stars-show", "World map stars", "Show collected stars above levels"};

    /* ---- Video - Run Info ----*/
    subsection info_run{this, "info-run", "Onscreen run info"};

    enum
    {
        EPISODE_TITLE_OFF = 0,
        EPISODE_TITLE_TOP,
        EPISODE_TITLE_BOTTOM,
    };
    opt_enum<int> show_episode_title{this,
        {
            {EPISODE_TITLE_OFF, "off", "Off", "Never show"},
            {EPISODE_TITLE_TOP, "top", "Top", "Show above HUD at high resolutions"},
            {EPISODE_TITLE_BOTTOM, "bottom", "Bottom", "Show above speedrun timer"},
            {EPISODE_TITLE_BOTTOM, "transparent"},
            {EPISODE_TITLE_BOTTOM, "on"},
        },
        defaults(EPISODE_TITLE_OFF), {}, Scope::UserGlobal,
        "show-episode-title", "Episode name", nullptr};

    enum
    {
        PLAYTIME_COUNTER_OFF = 0,
        PLAYTIME_COUNTER_SUBTLE = 1,
        PLAYTIME_COUNTER_OPAQUE,
        PLAYTIME_COUNTER_ANIMATED,
    };
    opt_enum<int> show_playtime_counter{this,
        {
            {PLAYTIME_COUNTER_OFF, "off", "Off", "Show only during speedruns"},
            {PLAYTIME_COUNTER_SUBTLE, "transparent", "Transparent", nullptr},
            {PLAYTIME_COUNTER_SUBTLE, "subtle"},
            {PLAYTIME_COUNTER_OPAQUE, "opaque", "Opaque", nullptr},
            {PLAYTIME_COUNTER_ANIMATED, "animated", "Animated", "Adds a rainbow effect on level end"},
        },
        defaults(PLAYTIME_COUNTER_OFF), {}, Scope::UserGlobal,
        "show-playtime-counter", "Playtime counter", "Show time spent on the episode and current attempt"};

    /* ---- Video - Effects ----*/
    subsection effects{this, "effects", "Screen Effects"};

    opt<bool> show_screen_shake{this,
        defaults(true), {CompatClass::pure_preference, true}, Scope::UserGlobal,
        "show-screen-shake", "Screen shake", nullptr};

    opt<bool> EnableInterLevelFade{this, defaults(true), {CompatClass::pure_preference, false}, Scope::UserGlobal,
        "enable-inter-level-fade-effect", "Fade transitions", nullptr};


    /* ---- Audio ----*/
    section audio{this, Scope::UserGlobal, "audio", "Audio", nullptr};

    opt<bool> audio_enable{this, defaults(true), {}, Scope::UserGlobal,
        "audio-enable", "Enable", nullptr,
        config_audio_set};

    opt_enum<int> audio_channels{this,
        {
            {1, "mono", "Mono", nullptr},
            {2, "stereo", "Stereo", nullptr},
        },
        defaults(g_audioDefaults.channels), {}, Scope::UserGlobal,
        "audio-channels", "Channels", nullptr,
        config_audio_set};

    /* ---- Audio - Preferences ----*/
    subsection audio_preferences{this, "audio-prefs", "Preferences"};

    // opt<bool> sfx_player_grow_with_got_item{this, defaults(false), {VER_THEXTECH136, CompatClass::critical_update, false}, Scope::UserGlobal | Scope::CreatorEpisode,
    //     "sfx-player-grow-with-got-item", "Player grow on get item", "Play \"player grow\" SFX together with \"get item\" SFX like early SNES games"};
    static constexpr bool sfx_player_grow_with_got_item = false;

    opt<bool> sfx_modern{this, defaults(true), {CompatClass::pure_preference, false}, Scope::UserGlobal,
        "sfx-modern", "Modern SFX", "Use sounds added in TheXTech", nullptr};

#if 0
    enum
    {
        MOUNTDRUMS_NEVER = 0,
        MOUNTDRUMS_NORMAL,
        MOUNTDRUMS_ALWAYS
    };
    opt_enum<int> sfx_mount_drums{this,
        {
            {MOUNTDRUMS_NEVER, "never", "Never", "Never plays the special drums"},
            {MOUNTDRUMS_NORMAL, "normal", "Normal", "Plays the special drums when you are riding a pet mount"},
            {MOUNTDRUMS_ALWAYS, "always", "Always", "Always plays the special drums"},
        },
        defaults<int>(MOUNTDRUMS_NORMAL), {CompatClass::pure_preference, MOUNTDRUMS_NEVER}, Scope::UserGlobal,
        "sfx-mount-drums", "Mount drums", "Should special drums play while the player is riding a pet?",
        config_mountdrums_set};
#endif

    opt<bool> sfx_pet_beat{this, defaults(true), {CompatClass::pure_preference, false}, Scope::UserGlobal,
        "sfx-pet-beat", "Pet grooves", "Play special groove when riding a pet",
        config_mountdrums_set};

#ifdef THEXTECH_ENABLE_AUDIO_FX

#   ifdef __3DS__
    static constexpr bool sfx_audio_fx_default = false;
#   else
    static constexpr bool sfx_audio_fx_default = true;
#   endif

    opt<bool> sfx_audio_fx{this, defaults(sfx_audio_fx_default), {CompatClass::pure_preference, false}, Scope::UserGlobal,
        "sfx-audio-fx", "Echo effects", "Add echo in some sections",
        config_audiofx_set};
#endif

    opt<bool> sfx_spatial_audio{this, defaults(true), {CompatClass::pure_preference, false}, Scope::UserGlobal,
        "sfx-spatial-audio", "Spatial audio", "Reduce volume of offscreen sounds"};


    /* ---- Controls ----*/
    section controls{this, Scope::UserGlobal, "controls", "Controls"};


    /* ---- Advanced ----*/
    section advanced{this, Scope::UserGlobal, "advanced", "Advanced", "Technical options for internal operations"};

    // static constexpr bool record_gameplay_data = false;

    opt<bool> modern_item_drop{this, defaults(false), {CompatClass::standard_update, false}, Scope::UserGlobal,
        "modern-item-drop", "Modern item drop", "Experimental implementation of modern item drop"};

    opt<bool> record_gameplay_data{this, defaults(false), {}, Scope::UserGlobal,
        "record-gameplay-data", "Record gameplay", "Save play data for replay or debugging"};

    opt_enum<int> log_level{this,
        {
            {PGE_LogLevel::NoLog, "none", "None", nullptr},
            {PGE_LogLevel::Fatal, "fatal", "Fatal", nullptr},
            {PGE_LogLevel::Critical, "critical", "Critical", nullptr},
            {PGE_LogLevel::Warning, "warning", "Warning", nullptr},
            {PGE_LogLevel::Info, "info", "Info", nullptr},
            {PGE_LogLevel::Debug, "debug", "Debug", nullptr},
            {PGE_LogLevel::NoLog, "0", nullptr, nullptr},
            {PGE_LogLevel::Fatal, "1", nullptr, nullptr},
            {PGE_LogLevel::Critical, "2", nullptr, nullptr},
            {PGE_LogLevel::Warning, "3", nullptr, nullptr},
            {PGE_LogLevel::Info, "4", nullptr, nullptr},
            {PGE_LogLevel::Debug, "5", nullptr, nullptr},
        },
#if defined(DEBUG_BUILD)
        defaults(PGE_LogLevel::Debug),
#else
        defaults(PGE_LogLevel::Info),
#endif
        {}, Scope::UserGlobal,
        "log-level", "Log level", "Log events at or above this level",
        config_log_level_set};

#if defined(__ANDROID__) || defined(__3DS__)
    opt<bool> use_native_osk{this, defaults(false), {}, Scope::UserGlobal,
        "use-native-osk", "Native OSK", "Use system's native keyboard"};
#endif

    /* ---- Advanced - Video ----*/
    subsection advanced_video{this, "advanced-video", "Video"};

#ifndef RENDER_CUSTOM
    enum
    {
        RENDER_SOFTWARE = 0,
        RENDER_ACCELERATED_AUTO,
        RENDER_ACCELERATED_SDL,
        RENDER_ACCELERATED_OPENGL,
        RENDER_ACCELERATED_OPENGL_ES,
        RENDER_ACCELERATED_OPENGL_LEGACY,
        RENDER_ACCELERATED_OPENGL_ES_LEGACY,
        RENDER_END
    };
    rendermode_t render_mode{this,
        {
            {RENDER_SOFTWARE, "sw", "Software", nullptr},
            {RENDER_ACCELERATED_AUTO, "hw", "Auto", ""},
            {RENDER_ACCELERATED_SDL, "sdl", "SDL2", "Basic cross-platform render API"},
#ifdef THEXTECH_BUILD_GL_DESKTOP_MODERN
            {RENDER_ACCELERATED_OPENGL, "opengl", "OpenGL 3+", "Desktop API with support for all new visual effects and full accuracy to SMBX64"},
#endif
#ifdef THEXTECH_BUILD_GL_ES_MODERN
            {RENDER_ACCELERATED_OPENGL_ES, "opengles", "OpenGL ES 2+", "Mobile API with support for all new visual effects and high accuracy to SMBX64"},
#endif
#ifdef THEXTECH_BUILD_GL_DESKTOP_LEGACY
            {RENDER_ACCELERATED_OPENGL_LEGACY, "opengl11", "OpenGL 1.1", "Legacy desktop API with full accuracy to SMBX64"},
#endif
#ifdef THEXTECH_BUILD_GL_ES_LEGACY
            {RENDER_ACCELERATED_OPENGL_ES_LEGACY, "opengles11", "OpenGL ES 1.1", "Legacy mobile API with full accuracy to SMBX64"},
#endif
            {RENDER_SOFTWARE, "0"},
            {RENDER_ACCELERATED_SDL, "1"},
        },
        defaults(RENDER_ACCELERATED_AUTO), {}, Scope::UserGlobal,
        "render", "Render mode", nullptr,
        config_rendermode_set};
#endif

#ifndef PGE_MIN_PORT
    enum ScaleDownTextures
    {
        SCALE_DOWN_NONE = 0,
        SCALE_DOWN_SAFE = 1,
        SCALE_DOWN_ALL = 2,
    };

    opt_enum<int> scale_down_textures{this,
        {
            {SCALE_DOWN_NONE, "none", "None", "Least loading stutter"},
            {SCALE_DOWN_SAFE, "safe", "Safe", "Checks if images are in 2x format"},
            {SCALE_DOWN_ALL, "all", "All", "Less loading stutter than 'Safe'"},
        },
        defaults(SCALE_DOWN_SAFE), {}, Scope::UserGlobal,
        "scale-down-textures", "Scale down images", "Store images as 1x to save memory"};
#endif

#ifndef THEXTECH_NO_SDL_BUILD
    /* ---- Advanced - Audio ----*/
    subsection advanced_audio{this, "advanced-audio", "Audio"};

    opt_enum<int> audio_sample_rate{this,
        {
            {11025, "11025", "11025 Hz", nullptr},
            {16000, "16000", "16000 Hz", nullptr},
            {22050, "22050", "22050 Hz", nullptr},
            {32000, "32000", "32000 Hz", nullptr},
            {44100, "44100", "44100 Hz", nullptr},
            {48000, "48000", "48000 Hz", nullptr},
        },
        defaults(g_audioDefaults.sampleRate), {}, Scope::UserGlobal,
        "audio-sample-rate", "Sample rate", nullptr,
        config_audio_set};

    opt_enum<int> audio_format{this,
        {
            {AUDIO_S8, "s8", "s8"},
            {AUDIO_S8, "pcm_s8"},

            {AUDIO_U8, "u8", "u8"},
            {AUDIO_U8, "pcm_u8"},

            {AUDIO_S16SYS, "s16", "s16"},
            {AUDIO_S16SYS, "pcm_s16"},
#       if AUDIO_S16LSB != AUDIO_S16SYS
            {AUDIO_S16LSB, "s16le", "s16le"},
            {AUDIO_S16LSB, "pcm_s16le"},
#       else
            {AUDIO_S16MSB, "s16be", "s16be"},
            {AUDIO_S16MSB, "pcm_s16be"},
#       endif

            {AUDIO_U16SYS, "u16", "u16"},
            {AUDIO_U16SYS, "pcm_u16"},
#       if AUDIO_U16LSB != AUDIO_U16SYS
            {AUDIO_U16LSB, "u16le", "u16le"},
            {AUDIO_U16LSB, "pcm_u16le"},
#       else
            {AUDIO_U16MSB, "u16be", "u16be"},
            {AUDIO_U16MSB, "pcm_u16be"},
#       endif

            {AUDIO_S32SYS, "s32", "s32"},
            {AUDIO_S32SYS, "pcm_s32"},
#       if AUDIO_S32LSB != AUDIO_S32SYS
            {AUDIO_S32LSB, "s32le", "s32le"},
            {AUDIO_S32LSB, "pcm_s32le"},
#       else
            {AUDIO_S32MSB, "s32be", "s32be"},
            {AUDIO_S32MSB, "pcm_s32be"},
#       endif

            {AUDIO_F32SYS, "float32", "float32"},
            {AUDIO_F32SYS, "pcm_f32"},
#       if AUDIO_F32LSB != AUDIO_F32SYS
            {AUDIO_F32LSB, "float32le", "float32le"},
            {AUDIO_F32LSB, "pcm_f32le"},
#       else
            {AUDIO_F32MSB, "float32be", "float32be"},
            {AUDIO_F32MSB, "pcm_f32be"},
#       endif
        },
        defaults(g_audioDefaults.format), {}, Scope::UserGlobal,
        "audio-format", "Audio format", "(Advanced) format for sound driver",
        config_audio_set};

    opt_enum<int> audio_buffer_size{this,
        {
            {512, "512", "512"},
            {768, "768", "768"},
            {1024, "1024", "1024"},
            {1536, "1536", "1536"},
            {2048, "2048", "2048"},
            {4096, "4096", "4096"},
        },
        defaults(g_audioDefaults.bufferSize), {}, Scope::UserGlobal,
        "audio-buffer-size", "Buffer size", "(Advanced) increase for fewer pops but more lag",
        config_audio_set};
#endif


    /* ---- Episode options ----*/
    section episode_options{this, Scope::UserLocal, "episode-options", "Episode Options"};

    enum
    {
        MODE_MODERN = 0,
        MODE_CLASSIC,
        MODE_VANILLA,
    };
    opt_enum<int> playstyle{this,
        {
            {MODE_MODERN, "modern", "Modern", "Enables all gameplay updates"},
            {MODE_CLASSIC, "classic", "Classic", "Critical updates only"},
            {MODE_VANILLA, "vanilla", "Vanilla", "Preserves all known bugs"},
        },
        defaults(MODE_MODERN), {CompatClass::pure_preference, MODE_VANILLA}, Scope::UserEpisode,
        "playstyle", "Playstyle", nullptr,
        config_compat_changed};

    enum
    {
        CREATORCOMPAT_ENABLE = 0,
        CREATORCOMPAT_FILEONLY,
        CREATORCOMPAT_DISABLE,
    };
    opt_enum<int> creator_compat{this,
        {
            {CREATORCOMPAT_ENABLE, "enable", "Enable", nullptr},
            {CREATORCOMPAT_FILEONLY, "file-only", "File only", nullptr},
            {CREATORCOMPAT_DISABLE, "disable", "Disable", nullptr},
        },
        defaults(CREATORCOMPAT_ENABLE), {CompatClass::critical_update, CREATORCOMPAT_DISABLE}, Scope::UserEpisode,
        "creator-compat", "Creator compat", "Logic tweaks by content creator"};

    // /* ---- Prefs - Effects ----*/
    // subsection effects{this, "effects", "Visual Effects"};

    // opt<bool> small_screen_camera_features{this, defaults(true), {VER_THEXTECH137}, Scope::UserGlobal,
    //     "small-screen-camera-features", "Small screen camera features", "Camera optimizations that improve the experience at low resolutions"};
    static constexpr bool small_screen_camera_features = true;



    /* ---- Compatibility ----*/
    section compat{this, Scope::File, "compatibility", "Session Tweaks", "Options for compat or debugging"};

    subsection reset_all{this, "reset-all", "Reset All"};

    // TODO: make sure that Modern / Classic do not override compat.ini settings
    enum
    {
        COMPAT_OFF = 0,
        COMPAT_CLASSIC,
        COMPAT_SMBX13,
        COMPAT_MODERN,
        COMPAT_CONTENT,
    };
    opt_enum<int> compatibility_mode{this,
        {
            {COMPAT_OFF, "off", "Off", nullptr},
            {COMPAT_OFF, "native"},
            {COMPAT_MODERN, "modern", "Modern", nullptr},
            {COMPAT_CLASSIC, "classic", "Classic", nullptr},
            {COMPAT_SMBX13, "smbx13", "Vanilla", nullptr},
            // {COMPAT_CONTENT, "content", "Content", "Preserves the gameplay of the version for which the content was released"},
        },
        defaults(COMPAT_OFF), {}, Scope::None,
        "compatibility-mode", "Compat mode", "Overrides settings (even custom ones in a world or level) to guarantee behavior compatible with an older fan game"};

    /* ---- Compatibility - Features ----*/
    subsection compat_features{this, "features", "Features"};

    opt<bool> prefer_modern_pause{this, defaults(true), {CompatClass::critical_update, false}, Scope::None,
        "prefer-modern-pause", "Modern pause screen", "Display the expanded pause menu by default (if off, expand pause menu using Alt Pause)"};
    opt<bool> allow_drop_add{this, defaults(true), {CompatClass::critical_update, false}, Scope::None,
        "allow-drop-add", "Allow drop/add", "Allow players to be dropped/added from the pause menu item during gameplay"};
    opt<bool> multiplayer_pause_controls{this, defaults(true), {CompatClass::critical_update, false}, Scope::None,
        "multiplayer-pause-controls", "Multiplayer pause controls", "Allows players other than P1 to pause the game and control the pause menu"};

    opt<bool> world_map_lvlname_marquee{this, defaults(false), {CompatClass::pure_preference, false}, Scope::Episode,
    "world-map-lvlname-marquee", "Level name marquee", "In the world map, use a marquee effect for the level name instead of splitting it over two lines"};

    opt<bool> fix_vanilla_checkpoints{this, defaults(true), {CompatClass::standard_update, false}, Scope::File,
        "fix-vanilla-checkpoints", "Fix vanilla checkpoints", "Resume from the most recently touched checkpoint after death"};
    opt<bool> enable_last_warp_hub_resume{this, defaults(true), {CompatClass::standard_update, false}, Scope::Episode,
        "enable-last-warp-hub-resume", "Save warp in hub", "In a hub world, resume play at the last warp you entered the hub level from"};
    opt<bool> disable_background2_tiling{this, defaults(false), {CompatClass::standard_update, true}, Scope::Local,
        "disable-background2-tiling", "Disable BG tiling", "Disable tiling of the level background texture if it causes visual disturbances"};
    opt<bool> modern_section_change{this, defaults(true), {CompatClass::critical_update, false}, Scope::File,
        "modern-section-change", "Modern section change", "Animate sections resizing when they shrink or during 2-player mode"};

    opt<bool> extra_screen_shake{this, defaults(true), {CompatClass::standard_update, false}, Scope::Local,
        "extra-screen-shake", "Extra screen shake", "Shake screen for large enemy and player ground pounds"};

    opt<bool> enable_playtime_tracking{this, defaults(true), {CompatClass::pure_preference, false}, Scope::File,
        "enable-playtime-tracking", "Playtime tracking", "Record the amount of time spent playing an episode"};
    opt<bool> enable_fails_tracking{this, defaults(true), {CompatClass::pure_preference, false}, Scope::File,
        "enable-fails-tracking", "Fails tracking", "Record the number of fails on the current level and world"};

    opt<bool> world_map_fast_move{this, defaults(true), {CompatClass::standard_update, false}, Scope::Local,
        "world-map-fast-move", "Automatic movement", "Moves automatically between forks on the path"};

    /* ---- Compatibility - Autocode ----*/
    subsection compat_autocode{this, "autocode", "Autocode"};

    // Luna
    opt<bool> luna_enable_engine{this, defaults(true), {CompatClass::critical_update, false}, Scope::File,
        "luna-script-enable-engine", "Enable engine", nullptr};
    opt<bool> luna_allow_level_codes{this, defaults(true), {CompatClass::critical_update, false}, Scope::File,
        "luna-script-allow-level-codes", "Allow level codes", nullptr};
    static constexpr bool autocode_translate_coords = true;
    // opt<bool> autocode_translate_coords{this, defaults(true), {}, Scope::File,
    //     "autocode-translate-coords", "Translate coords", "Translate the coordinates of autocode screen-space objects based on the HUD location"};

    /* ---- Compatibility - Bugfixes ----*/
    subsection bugfixes{this, "bugfixes", "Bugfixes"};

    // <1.3.4
    opt<bool> fix_restored_block_move{this, defaults(true), {CompatClass::standard_update, false}, Scope::File,
        "fix-restored-block-move", "Fix restored block move", "Don't move powerup blocks to the right when they are hit after restoring"};
    opt<bool> fix_player_slope_speed{this, defaults(true), {CompatClass::standard_update, false}, Scope::File,
        "fix-player-slope-speed", "Player slope speed", nullptr};
    // 1.3.4
    opt<bool> fix_player_filter_bounce{this, defaults(true), {CompatClass::critical_update, false}, Scope::File,
        "fix-player-filter-bounce", "Player filter bounce", "Fix a glitch where player could clip downwards"};
    opt<bool> fix_player_clip_wall_at_npc{this, defaults(true), {CompatClass::critical_update, false}, Scope::File,
        "fix-player-clip-wall-at-npc", "Player clip wall at NPC", nullptr};
    opt<bool> fix_player_downward_clip{this, defaults(true), {CompatClass::critical_update, false}, Scope::File,
        "fix-player-downward-clip", "Player downward clip", nullptr};
    opt<bool> fix_npc_downward_clip{this, defaults(true), {CompatClass::critical_update, false}, Scope::File,
        "fix-npc-downward-clip", "NPC downward clip", nullptr};
    opt<bool> fix_npc247_collapse{this, defaults(true), {CompatClass::standard_update, false}, Scope::File,
        "fix-npc247-collapse", "NPC 247 collapse", "Prevents the stacked spiky plants from collapsing incorrectly"};
    opt<bool> fix_platforms_acceleration{this, defaults(true), {CompatClass::critical_update, false}, Scope::File,
        "fix-platform-acceleration", "Platform acceleration", nullptr};
    opt<bool> fix_npc55_kick_ice_blocks{this, defaults(true), {CompatClass::standard_update, false}, Scope::File,
        "fix-npc55-kick-ice-blocks", "NPC55 kick ice blocks", "The beached turtle should not sadly kick ice blocks"};
    opt<bool> fix_climb_invisible_fences{this, defaults(true), {CompatClass::standard_update, false}, Scope::File,
        "fix-climb-invisible-fences", "Don't climb hidden fences", nullptr};
    opt<bool> fix_climb_bgo_speed_adding{this, defaults(true), {CompatClass::standard_update, false}, Scope::File,
        "fix-climb-bgo-speed-adding", "Climb BGO speed adding", nullptr};
    opt<bool> fix_climb_bgo_layer_move{this, defaults(true), {CompatClass::standard_update, false}, Scope::File,
        "enable-climb-bgo-layer-move", "Climb moving BGOs", nullptr};
    opt<bool> fix_skull_raft{this, defaults(true), {CompatClass::standard_update, false}, Scope::File,
        "fix-skull-raft", "Skull raft", nullptr};
    opt<bool> fix_char3_escape_shell_surf{this, defaults(true), {CompatClass::critical_update, false}, Scope::File,
        "fix-char3-escape-shell-surf", "Char 3 escape shell surf", nullptr};
    opt<bool> fix_plant_wobble{this, defaults(true), {CompatClass::standard_update, false}, Scope::File,
        "fix-plant-wobble", "Fix plant wobble", "Plants should correctly resize instead of appearing to change size / position"};
    opt<bool> fix_powerup_lava_bug{this, defaults(true), {CompatClass::standard_update, false}, Scope::File,
        "fix-powerup-lava-bug", "Fix powerup lava bug", "Powerups should be destroyed when hitting lava instead of surviving depending on the current NPC array"};
    // opt<bool> fix_keyhole_framerate{this, defaults(true), {VER_THEXTECH134, CompatClass::critical_update, false}, Scope::File,
    //     "fix-keyhole-framerate", "Fix keyhole framerate", nullptr};
    // 1.3.5
    opt<bool> fix_char5_vehicle_climb{this, defaults(true), {CompatClass::critical_update, false}, Scope::File,
        "fix-char5-vehicle-climb", "Fix char 5 car fairy", nullptr};
    opt<bool> fix_vehicle_char_switch{this, defaults(true), {CompatClass::critical_update, false}, Scope::File,
        "fix-vehicle-char-switch", "Don't switch character by car", nullptr};
    opt<bool> fix_autoscroll_speed{this, defaults(true), {CompatClass::standard_update, false}, Scope::File,
        "fix-autoscroll-speed", "Autoscroll speed", nullptr};
    opt<bool> fix_submerged_splash_effect{this, defaults(true), {CompatClass::standard_update, false}, Scope::File,
        "fix-submerged-splash-effect", "Fix submerged splash effect", "Don't make a splash effect for items already underwater"};
    // 1.3.5.1
    opt<bool> fix_squid_stomp_effect{this, defaults(true), {CompatClass::standard_update, false}, Scope::File,
        "fix-squid-stomp-effect", "Fix calamari stomp effect", nullptr};
    // deprecated, fixed non-vanilla bug
    // opt<bool> keep_bullet_bill_dir{this, defaults(true), {VER_THEXTECH1351, CompatClass::critical_update, false}, Scope::File,
    //     "keep-bullet-bill-direction", "Keep bullet dir", nullptr};
    opt<bool> fix_special_coin_switch{this, defaults(true), {CompatClass::standard_update, false}, Scope::File,
        "fix-special-coin-switch", "Preserve special coins", "Don't turn special coins into blocks when pressing switch"};
    // 1.3.5.2
    opt<bool> fix_bat_start_while_inactive{this, defaults(true), {CompatClass::standard_update, false}, Scope::File,
        "fix-bat-start-while-inactive", "Bat start", "Don't activate the bats until they are onscreen"};
    // opt<bool> fix_FreezeNPCs_no_reset{this, defaults(false), {VER_THEXTECH137, CompatClass::standard_update, false}, Scope::None,
    //     "fix-FreezeNPCs-no-reset", "'Fix' FreezeNPCs", "There is a bug that results in NPCs vanishing after getting a freeze clock, but this 'fix' introduces other bugs"};
    static constexpr bool fix_FreezeNPCs_no_reset = false;
    // 1.3.5.3
    opt<bool> fix_npc_activation_event_loop_bug{this, defaults(true), {CompatClass::critical_update, false}, Scope::File,
        "fix-npc-activation-event-loop-bug", "NPC activation loop", "Fixes softlock bug that results when NPCs activate each other in a loop"};
    // 1.3.6
    opt<bool> fix_fairy_stuck_in_pipe{this, defaults(true), {CompatClass::critical_update, false}, Scope::File,
        "fix-fairy-stuck-in-pipe", "Fairy stuck in pipe", nullptr};
    opt<bool> fix_flamethrower_gravity{this, defaults(true), {CompatClass::standard_update, false}, Scope::File,
        "fix-flamethrower-gravity", "Flamethrower gravity", "Should this have even been made?"};
    // 1.3.6.1
    opt<bool> fix_npc_ceiling_speed{this, defaults(true), {CompatClass::standard_update, false}, Scope::File,
        "fix-npc-ceiling-speed", "Fix NPC ceiling speed", "Don't stick a thrown NPC to ceiling when a layer is moving"};
    opt<bool> emulate_classic_block_order{this, defaults(false), {CompatClass::critical_update, true}, Scope::File,
        "emulate-classic-block-order", "Emulate classic block order", "Use the classic game's block sorting methods internally for frame-perfect compatibility"};
    opt<bool> custom_powerup_collect_score{this, defaults(true), {CompatClass::standard_update, false}, Scope::File,
        "custom-powerup-collect-score", "Custom powerup collect score", "Collected powerups give score from npc-X.txt"};
    opt<bool> fix_player_crush_death{this, defaults(true), {CompatClass::critical_update, false}, Scope::File,
        "fix-player-crush-death", "Fix player crush death", "Player should not be crushed by corners of slopes or by hitting a horizontally moving ceiling"};
    opt<bool> fix_pound_skip_warp{this, defaults(true), {CompatClass::critical_update, false}, Scope::File,
        "fix-pound-skip-warp", "Fix pound skip warp", "Pound move should not skip instant / portal warps"};
    opt<bool> fix_held_item_cancel{this, defaults(true), {CompatClass::standard_update, false}, Scope::File,
        "fix-held-item-cancel", "Fix held item cancel", "Do not cancel held item hitting hostile NPC that intersects with an immune NPC"};
    opt<bool> fix_frame_perfect_despawn{this, defaults(true), {CompatClass::standard_update, false}, Scope::File,
        "fix-frame-perfect-despawn", "Fix frame perfect despawn", "If NPC comes onscreen later in the same frame it timed out, should still be able to spawn"};

    // TODO: implement proper read/write operations
    // opt<std::array<uint8_t, 3>> bitblit_background_colour{this, defaults<std::array<uint8_t, 3>>({0, 0, 0}), {VER_THEXTECH1361}, Scope::File,
    //     "bitblit-background-colour", "Bitblit background colour", "Background colour for legacy transparency effects", config_bitblit_background_set};

    // 1.3.6.3
    opt<bool> pound_by_alt_run{this, defaults(true), {CompatClass::critical_update, false}, Scope::File,
        "pound-by-alt-run", "Pound by alt run", "Use Alt Run for Pound action when player is in a purple pet mount"};

    // 1.3.6.5
    opt<bool> fix_visual_bugs{this, defaults(true), {CompatClass::standard_update, false}, Scope::File,
        "fix-visual-bugs", "Fix visual bugs", "Fix misc visual bugs from SMBX 1.3"};
    opt<bool> fix_npc_emerge_size{this, defaults(true), {CompatClass::standard_update, false}, Scope::File,
        "fix-npc-emerge-size", "Fix NPC emerge size", "Fix size of NPC emerging from a block"};
    opt<bool> fix_switched_block_clipping{this, defaults(true), {CompatClass::standard_update, false}, Scope::File,
        "fix-switched-block-clipping", "Fix switched block clipping", "Don't let blocks become intangible to NPCs after coin switch"};


    // 1.3.7
    opt<bool> fix_npc_camera_logic{this, defaults(true), {CompatClass::critical_update, false}, Scope::File,
        "fix-npc-camera-logic", "Fix NPC camera logic", "NPCs should support more than two cameras"};
    opt<bool> modern_npc_targeting{this, defaults(true), {CompatClass::standard_update, false}, Scope::File,
        "modern-npc-targeting", "Modern NPC targeting", "Updated NPC targeting logic in multiplayer"};
    opt<bool> allow_multires{this, defaults(true), {CompatClass::critical_update, false}, Scope::Local,
        "allow-multires", "Allow multires", "Allows the gameplay field to be <800x600", config_res_set};
    opt<bool> dynamic_camera_logic{this, defaults(true), {CompatClass::standard_update, false}, Scope::File,
        "dynamic-camera-logic", "Dynamic camera logic", "Allow a non-800x600 camera for most logic, and only use the event logic camera in rare cases",
        config_res_set};
    opt<bool> modern_lives_system{this, defaults(true), {CompatClass::critical_update, false}, Scope::None,
        "modern-lives-system", "Modern lives system", "Allow negative lives instead of returning to the main menu"};

    /* ---- Compatibility - Speedrun ----*/

    section speedrun{this, Scope::CreatorEpisode, "speedrun", "Speedrun"};

    opt_enum<int> speedrun_mode{this,
        {
            {0, "0", "0", nullptr},
            {1, "1", "1", nullptr},
            {2, "2", "2", nullptr},
            {3, "3", "3", nullptr},
        },
        defaults<int>(0), {}, Scope::UserEpisode,
        "mode", "Speedrun mode", nullptr,
        config_compat_changed};

    enum
    {
        SPEEDRUN_STOP_NONE = 0,
        SPEEDRUN_STOP_EVENT,
        SPEEDRUN_STOP_LEAVE_LEVEL,
        SPEEDRUN_STOP_ENTER_LEVEL
    };
    opt_enum<int> speedrun_stop_timer_by{this,
        {
            {SPEEDRUN_STOP_NONE, "none", "Off", nullptr},
            {SPEEDRUN_STOP_EVENT, "event", "Event (specify)", nullptr},
            {SPEEDRUN_STOP_LEAVE_LEVEL, "leave", "Leave level (specify)", nullptr},
            {SPEEDRUN_STOP_ENTER_LEVEL, "enter", "Enter level (specify)", nullptr},
        },
        defaults<int>(SPEEDRUN_STOP_NONE), {}, Scope::Episode,
        "stop-timer-by", "Stop speedrun timer on", "This triggers the speedrun timer to stop"};
    opt<std::string> speedrun_stop_timer_at{this, defaults<std::string>("Boss Dead"), {}, Scope::Episode,
        "stop-timer-at", "Stop speedrun timer event/level", "If above is 'Event' or 'Level', this specifies which"};

    opt<std::string> fails_counter_title{this, defaults<std::string>("FAILS"), {}, Scope::CreatorGlobal,
        "fails-counter-title", "Fails counter title", "Label used for the fails counter"};

    /* ---- View Credits ----*/
    section view_credits{this, Scope::Global, "view-credits", "View Credits"};
};

void OpenConfig();
void LoadCustomConfig();
void ResetCustomConfig();
void UpdateConfig();
void SaveConfig();

using Options_t = _Config_t<false>;
using Config_t = _Config_t<true>;

extern const std::vector<const Config_t*> g_config_levels;

extern _Config_t<false> g_options;

// <defaults>                                1
// extern GameInfo g_gameInfo;            // 2, decl game_info.h, defn config_main.cpp
extern Config_t g_config_game_user;       // 3
extern Config_t g_config_episode_creator; // 4
// extern Config_t g_config_episode_user;    // 5
extern Config_t g_config_file_creator;    // 6
// extern Config_t g_config_cmdline;         // 7 // temporary
// <cmdline>                                10
// <usertest>                               11
// <cheats>                                 12
// <compat>                                 13

extern Config_t g_config;

#endif // CONFIG_MAIN_H
