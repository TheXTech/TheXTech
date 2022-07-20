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

#include <SDL2/SDL_audio.h>

#include "../globals.h"
#include "../game_main.h"
#include "../graphics.h"
#include "../sound.h"
#include "../config.h"
#include "../video.h"
#include "../controls.h"

#include "speedrunner.h"
#include "presetup.h"
#include "record.h"

#include <Utils/files.h>
#include <Utils/strings.h>
#include <IniProcessor/ini_processing.h>
#include <fmt_format_ne.h>
#include <AppPath/app_path.h>
#include <Logger/logger.h>

Config_t g_config;
VideoSettings_t g_videoSettings;

PreSetup_t g_preSetup;

void OpenConfig_preSetup()
{
    const IniProcessing::StrEnumMap renderMode =
    {
        {"sw", RENDER_SOFTWARE},
        {"hw", RENDER_ACCELERATED},
        {"vsync", RENDER_ACCELERATED_VSYNC},
        {"0", RENDER_SOFTWARE},
        {"1", RENDER_ACCELERATED},
        {"2", RENDER_ACCELERATED_VSYNC}
    };

    const IniProcessing::StrEnumMap sampleFormats =
    {
        {"s8", AUDIO_S8},
        {"pcm_s8", AUDIO_S8},
        {"u8", AUDIO_U8},
        {"pcm_u8", AUDIO_U8},
        {"s16", AUDIO_S16SYS},
        {"pcm_s16", AUDIO_S16SYS},
        {"s16le", AUDIO_S16LSB},
        {"pcm_s16le", AUDIO_S16LSB},
        {"s16be", AUDIO_S16MSB},
        {"pcm_s16be", AUDIO_S16MSB},
        {"u16", AUDIO_U16SYS},
        {"pcm_u16", AUDIO_U16SYS},
        {"u16le", AUDIO_U16LSB},
        {"pcm_u16le", AUDIO_U16LSB},
        {"u16be", AUDIO_U16MSB},
        {"pcm_u16be", AUDIO_U16MSB},
        {"s32", AUDIO_S32SYS},
        {"pcm_s32", AUDIO_S32SYS},
        {"s32le", AUDIO_S32LSB},
        {"pcm_s32le", AUDIO_S32LSB},
        {"s32be", AUDIO_S32MSB},
        {"pcm_s32be", AUDIO_S32MSB},
        {"float32", AUDIO_F32SYS},
        {"pcm_f32", AUDIO_F32SYS},
        {"float32le", AUDIO_F32LSB},
        {"pcm_f32le", AUDIO_F32LSB},
        {"float32be", AUDIO_F32MSB},
        {"pcm_f32be", AUDIO_F32MSB}
    };

    const IniProcessing::StrEnumMap compatMode =
    {
        {"native", 0},
        {"smbx2", 1},
        {"smbx13", 2}
    };

    const IniProcessing::StrEnumMap speedRunBlinkMode =
    {
        {"undefined", SPEEDRUN_EFFECT_BLINK_UNDEFINED},
        {"opaque", SPEEDRUN_EFFECT_BLINK_OPAQUEONLY},
        {"always", SPEEDRUN_EFFECT_BLINK_ALWAYS},
        {"true", SPEEDRUN_EFFECT_BLINK_ALWAYS},
        {"never", SPEEDRUN_EFFECT_BLINK_NEVER},
        {"false", SPEEDRUN_EFFECT_BLINK_NEVER}
    };

    const IniProcessing::StrEnumMap scaleDownTextures =
    {
        {"none", VideoSettings_t::SCALE_NONE},
        {"safe", VideoSettings_t::SCALE_SAFE},
        {"all", VideoSettings_t::SCALE_ALL}
    };

    std::string configPath = AppPathManager::settingsFileSTD();

    if(Files::fileExists(configPath))
    {
        IniProcessing config(configPath);

        config.beginGroup("video");
        config.readEnum("render", g_videoSettings.renderMode, (int)RENDER_ACCELERATED, renderMode);
        config.read("background-work", g_videoSettings.allowBgWork, false);
        config.read("background-controller-input", g_videoSettings.allowBgControllerInput, false);
        config.read("frame-skip", g_videoSettings.enableFrameSkip, true);
        config.read("show-fps", g_videoSettings.showFrameRate, false);
        bool scale_down_all;
        config.read("scale-down-all-textures", scale_down_all, false);
        config.readEnum("scale-down-textures", g_videoSettings.scaleDownTextures, scale_down_all ? (int)VideoSettings_t::SCALE_ALL : (int)VideoSettings_t::SCALE_SAFE, scaleDownTextures);
#ifndef FIXED_RES
        config.read("internal-width", g_config.InternalW, 800);
        config.read("internal-height", g_config.InternalH, 600);

        ScreenW = g_config.InternalW;
        ScreenH = g_config.InternalH;
        if(ScreenH == 0)
        {
            ScreenW = 1280;
            ScreenH = 720;
        }
        else if(ScreenW == 0)
        {
            ScreenW = 800;
        }
#endif
        IniProcessing::StrEnumMap scaleModes =
        {
            {"linear", SCALE_DYNAMIC_LINEAR},
            {"integer", SCALE_DYNAMIC_INTEGER},
            {"nearest", SCALE_DYNAMIC_NEAREST},
            {"0.5x", SCALE_FIXED_05X},
            {"1x", SCALE_FIXED_1X},
            {"2x", SCALE_FIXED_2X},
        };
        config.readEnum("scale-mode", g_videoSettings.scaleMode, (int)SCALE_DYNAMIC_NEAREST, scaleModes);
        config.endGroup();

        config.beginGroup("sound");
        config.read("disable-sound", g_audioSetup.disableSound, false);
        config.read("sample-rate", g_audioSetup.sampleRate, 44100);
        config.read("channels", g_audioSetup.channels, 2);
        config.readEnum("format", g_audioSetup.format, (uint16_t)AUDIO_F32, sampleFormats);
        config.read("buffer-size", g_audioSetup.bufferSize, 512);
        config.endGroup();

        config.beginGroup("gameplay");
        config.readEnum("compatibility-mode", g_preSetup.compatibilityMode, 0, compatMode);
        config.endGroup();

        config.beginGroup("speedrun");
        config.read("mode", g_preSetup.speedRunMode, 0);
        config.read("semi-transparent-timer", g_preSetup.speedRunSemiTransparentTimer, false);
        config.readEnum("blink-effect", g_preSetup.speedRunEffectBlink, (int)SPEEDRUN_EFFECT_BLINK_UNDEFINED, speedRunBlinkMode);
        config.endGroup();
    }
}


void OpenConfig()
{
    int FileRelease = 0;
    bool resBool = false;
    std::string configPath = AppPathManager::settingsFileSTD();
    std::string controlsPath = AppPathManager::settingsControlsFileSTD();

    if(Files::fileExists(configPath))
    {
        IniProcessing config(configPath);
        IniProcessing controls(controlsPath);
         // Keep backward compatibility and restore old mappings from the "thextech.ini"
        IniProcessing *ctl = Files::fileExists(controlsPath) ? &controls : &config;

        const IniProcessing::StrEnumMap batteryStatus =
        {
            {"off", BATTERY_STATUS_OFF},
            {"fullscreen-low", BATTERY_STATUS_FULLSCREEN_WHEN_LOW},
            {"low", BATTERY_STATUS_ANY_WHEN_LOW},
            {"fullscreen", BATTERY_STATUS_FULLSCREEN_ON},
            {"on", BATTERY_STATUS_ALWAYS_ON}
        };

        const IniProcessing::StrEnumMap showEpisodeTitle
        {
            {"off", Config_t::EPISODE_TITLE_OFF},
            {"on", Config_t::EPISODE_TITLE_ON},
            {"transparent", Config_t::EPISODE_TITLE_TRANSPARENT},
            {"auto", Config_t::EPISODE_TITLE_AUTO},
            {"on-always", Config_t::EPISODE_TITLE_ON_ALWAYS},
            {"transparent-always", Config_t::EPISODE_TITLE_TRANSPARENT_ALWAYS},
            {"0", Config_t::EPISODE_TITLE_OFF},
            {"1", Config_t::EPISODE_TITLE_ON},
            {"2", Config_t::EPISODE_TITLE_TRANSPARENT}
        };

        const IniProcessing::StrEnumMap starsShowPolicy =
        {
            {"hide", 0},
            {"dont-show", 0},
            {"collected-only", 1},
            {"show", 2},
            {"show-all", 2}
        };

        const IniProcessing::StrEnumMap worldMapFog =
        {
            {"off", Config_t::WORLD_MAP_FOG_OFF},
            {"normal", Config_t::WORLD_MAP_FOG_NORMAL},
            {"spoilers", Config_t::WORLD_MAP_FOG_SPOILERS},
        };

        config.beginGroup("main");
        config.read("release", FileRelease, curRelease);
        config.read("full-screen", resBool, false);
        config.read("record-gameplay", g_config.RecordGameplayData, false);
        config.read("use-native-osk", g_config.use_native_osk, false);
        config.read("new-editor", g_config.enable_editor, false);
        config.read("enable-editor", g_config.enable_editor, g_config.enable_editor);
        config.read("editor-edge-scroll", g_config.editor_edge_scroll, g_config.editor_edge_scroll);
        config.endGroup();

        config.beginGroup("video");
        config.read("display-controllers", g_drawController, false);
        config.readEnum("battery-status", g_videoSettings.batteryStatus, (int)BATTERY_STATUS_OFF, batteryStatus);
        config.read("osk-fill-screen", g_config.osk_fill_screen, false);
        config.readEnum("show-episode-title", g_config.show_episode_title, (int)Config_t::EPISODE_TITLE_OFF, showEpisodeTitle);
        config.read("show-backdrop", g_config.show_backdrop, true);
        config.endGroup();

        config.beginGroup("recent");
        config.read("episode-1p", g_recentWorld1p, std::string());
        config.read("episode-2p", g_recentWorld2p, std::string());
        config.read("episode-editor", g_recentWorldEditor, std::string());
        config.endGroup();

        config.beginGroup("gameplay");
        config.read("ground-pound-by-alt-run", g_config.GameplayPoundByAltRun, false);
        config.readEnum("world-map-stars-show-policy", g_config.WorldMapStarShowPolicyGlobal, 0, starsShowPolicy);
        config.read("strict-drop-add", g_config.StrictDropAdd, false);
        config.read("no-pause-reconnect", g_config.NoPauseReconnect, false);
        config.read("enter-cheats-menu-item", g_config.enter_cheats_menu_item, false);
        config.read("world-map-fast-move", g_config.worldMapFastMove, false);
        config.read("editor-pause-on-death", g_config.editor_pause_on_death, true);
        config.read("hud-follows-player", g_config.hud_follows_player, false);
        config.readEnum("world-map-fog", g_config.world_map_fog, (int)Config_t::WORLD_MAP_FOG_NORMAL, worldMapFog);
        config.endGroup();

        config.beginGroup("effects");
        config.read("enable-thwomp-screen-shake", g_config.GameplayShakeScreenThwomp, true);
        config.read("enable-yoshi-ground-pound-screen-shake", g_config.GameplayShakeScreenPound, true);
        config.read("enable-bowser-iiird-screen-shake", g_config.GameplayShakeScreenBowserIIIrd, true);
        config.read("sfx-player-grow-with-got-item", g_config.SoundPlayerGrowWithGetItem, false);
        config.read("enable-inter-level-fade-effect", g_config.EnableInterLevelFade, true);
        config.endGroup();

        Controls::LoadConfig(ctl);

        pLogDebug("Loaded config: %s", configPath.c_str());
    }
    else
    {
        pLogDebug("Writing new config on first run.");
        SaveConfig(); // Create the config file on first run
    }
//    If resBool = True And resChanged = False And LevelEditor = False Then ChangeScreen
#ifndef RENDER_FULLSCREEN_ALWAYS
    if(resBool && !resChanged)
        ChangeScreen();
#endif

}

void SaveConfig()
{
    std::string configPath = AppPathManager::settingsFileSTD();
    std::string controlsPath = AppPathManager::settingsControlsFileSTD();

    IniProcessing config(configPath);
    IniProcessing controls(controlsPath);

    config.beginGroup("main");
    config.setValue("release", curRelease);
#if !defined(__EMSCRIPTEN__) && !defined(__ANDROID__) // Don't remember fullscreen state for Emscripten!
    config.setValue("full-screen", resChanged);
#endif
    config.setValue("record-gameplay", g_config.RecordGameplayData);
    config.setValue("use-native-osk", g_config.use_native_osk);
    config.setValue("enable-editor", g_config.enable_editor);
    config.setValue("editor-edge-scroll", g_config.editor_edge_scroll);
    config.endGroup();

    config.beginGroup("recent");
    config.setValue("episode-1p", g_recentWorld1p);
    config.setValue("episode-2p", g_recentWorld2p);
    config.setValue("episode-editor", g_recentWorldEditor);
    config.endGroup();

    config.beginGroup("video");
    {
        std::unordered_map<int, std::string> renderMode =
        {
            {RENDER_SOFTWARE, "sw"},
            {RENDER_ACCELERATED, "hw"},
            {RENDER_ACCELERATED_VSYNC, "vsync"},
        };

        std::unordered_map<int, std::string> batteryStatus =
        {
            {BATTERY_STATUS_OFF, "off"},
            {BATTERY_STATUS_FULLSCREEN_WHEN_LOW, "fullscreen-low"},
            {BATTERY_STATUS_ANY_WHEN_LOW, "low"},
            {BATTERY_STATUS_FULLSCREEN_ON, "fullscreen"},
            {BATTERY_STATUS_ALWAYS_ON, "on"}
        };

        std::unordered_map<int, std::string> showEpisodeTitle =
        {
            {Config_t::EPISODE_TITLE_OFF, "off"},
            {Config_t::EPISODE_TITLE_ON, "on"},
            {Config_t::EPISODE_TITLE_TRANSPARENT, "transparent"},
            {Config_t::EPISODE_TITLE_AUTO, "auto"},
            {Config_t::EPISODE_TITLE_ON_ALWAYS, "on-always"},
            {Config_t::EPISODE_TITLE_TRANSPARENT_ALWAYS, "transparent-always"}
        };

        std::unordered_map<int, std::string> scaleDownTextures =
        {
            {VideoSettings_t::SCALE_NONE, "none"},
            {VideoSettings_t::SCALE_SAFE, "safe"},
            {VideoSettings_t::SCALE_ALL, "all"},
        };

        config.setValue("render", renderMode[g_videoSettings.renderMode]);
        config.setValue("background-work", g_videoSettings.allowBgWork);
        config.setValue("background-controller-input", g_videoSettings.allowBgControllerInput);
        config.setValue("frame-skip", g_videoSettings.enableFrameSkip);
        config.setValue("show-fps", g_videoSettings.showFrameRate);
        config.setValue("scale-down-textures", scaleDownTextures[g_videoSettings.scaleDownTextures]);
        config.setValue("display-controllers", g_drawController);
        config.setValue("battery-status", batteryStatus[g_videoSettings.batteryStatus]);
        config.setValue("osk-fill-screen", g_config.osk_fill_screen);
        config.setValue("show-episode-title", showEpisodeTitle[g_config.show_episode_title]);
        config.setValue("show-backdrop", g_config.show_backdrop);
#       ifndef FIXED_RES
        config.setValue("internal-width", g_config.InternalW);
        config.setValue("internal-height", g_config.InternalH);
#       endif
        config.setValue("scale-mode", ScaleMode_strings.at(g_videoSettings.scaleMode));
    }
    config.endGroup();

    config.beginGroup("sound");
    config.setValue("disable-sound", g_audioSetup.disableSound);
    config.setValue("sample-rate", g_audioSetup.sampleRate);
    config.setValue("channels", g_audioSetup.channels);
    config.setValue("buffer-size", g_audioSetup.bufferSize);
    static const std::unordered_map<int, std::string> formats_back = {
        {AUDIO_S8 , "s8"},
        {AUDIO_U8 , "u8"},
        {AUDIO_S16LSB, "s16le"},
        {AUDIO_U16LSB, "u16le"},
        {AUDIO_S32LSB, "s32le"},
        {AUDIO_F32LSB, "float32le"},
        {AUDIO_S16MSB, "s16be"},
        {AUDIO_U16MSB, "u16be"},
        {AUDIO_S32MSB, "s32be"},
        {AUDIO_F32MSB, "float32be"}
    };
    config.setValue("format", formats_back.at(g_audioSetup.format));
    config.endGroup();

    config.beginGroup("gameplay");
    {
        std::unordered_map<int, std::string> starsShowPolicy =
        {
            {0, "hide"},
            {1, "collected-only"},
            {2, "show-all"}
        };

        std::unordered_map<int, std::string> compatMode =
        {
            {0, "native"},
            {1, "smbx2"},
            {2, "smbx13"}
        };

        config.setValue("ground-pound-by-alt-run", g_config.GameplayPoundByAltRun);
        config.setValue("world-map-stars-show-policy", starsShowPolicy[g_config.WorldMapStarShowPolicyGlobal]);
        config.setValue("compatibility-mode", compatMode[g_preSetup.compatibilityMode]);
    }
    config.setValue("strict-drop-add", g_config.StrictDropAdd);
    config.setValue("no-pause-reconnect", g_config.NoPauseReconnect);
    config.setValue("enter-cheats-menu-item", g_config.enter_cheats_menu_item);
    config.setValue("world-map-fast-move", g_config.worldMapFastMove);
    config.setValue("editor-pause-on-death", g_config.editor_pause_on_death);
    config.setValue("hud-follows-player", g_config.hud_follows_player);
    {
        std::unordered_map<int, std::string> worldMapFog =
        {
            {Config_t::WORLD_MAP_FOG_OFF, "off"},
            {Config_t::WORLD_MAP_FOG_NORMAL, "normal"},
            {Config_t::WORLD_MAP_FOG_SPOILERS, "spoilers"},
        };

        config.setValue("world-map-fog", worldMapFog[g_config.world_map_fog]);
    }
    config.endGroup();

    config.beginGroup("speedrun");
    config.setValue("mode", g_preSetup.speedRunMode);
    config.setValue("semi-transparent-timer", g_preSetup.speedRunSemiTransparentTimer);
    {
        std::unordered_map<int, std::string>  speedRunBlinkMode =
        {
            {SPEEDRUN_EFFECT_BLINK_UNDEFINED, "undefined"},
            {SPEEDRUN_EFFECT_BLINK_OPAQUEONLY, "opaque"},
            {SPEEDRUN_EFFECT_BLINK_ALWAYS, "always"},
            {SPEEDRUN_EFFECT_BLINK_NEVER, "never"}
        };
        config.setValue("blink-effect", speedRunBlinkMode[g_preSetup.speedRunEffectBlink]);
    }
    config.endGroup();

    config.beginGroup("effects");
    {
        config.setValue("enable-thwomp-screen-shake", g_config.GameplayShakeScreenThwomp);
        config.setValue("enable-yoshi-ground-pound-screen-shake", g_config.GameplayShakeScreenPound);
        config.setValue("enable-bowser-iiird-screen-shake", g_config.GameplayShakeScreenBowserIIIrd);
        config.setValue("sfx-player-grow-with-got-item", g_config.SoundPlayerGrowWithGetItem);
        config.setValue("enable-inter-level-fade-effect", g_config.EnableInterLevelFade);
    }
    config.endGroup();

    Controls::SaveConfig(&controls);

    config.writeIniFile();
    controls.writeIniFile();
#ifdef __EMSCRIPTEN__
    AppPathManager::syncFs();
#endif

    pLogDebug("Saved config: %s, control mappings: %s", configPath.c_str(), controlsPath.c_str());
}
