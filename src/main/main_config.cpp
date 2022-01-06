/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2021 Vitaly Novichkov <admin@wohlnet.ru>
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
#include "../video.h"
#include "speedrunner.h"
#include "../controls.h"

#include "../config.h"

#include "record.h"

#include <Utils/files.h>
#include <Utils/strings.h>
#include <IniProcessor/ini_processing.h>
#include <fmt_format_ne.h>
#include <AppPath/app_path.h>
#include <Logger/logger.h>

Config_t g_config;
VideoSettings_t g_videoSettings;

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

    const IniProcessing::StrEnumMap batteryStatus =
    {
        {"off", BATTERY_STATUS_OFF},
        {"fullscreen-low", BATTERY_STATUS_FULLSCREEN_WHEN_LOW},
        {"low", BATTERY_STATUS_ANY_WHEN_LOW},
        {"fullscreen", BATTERY_STATUS_FULLSCREEN_ON},
        {"on", BATTERY_STATUS_ALWAYS_ON}
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
        config.read("scale-down-all-textures", g_videoSettings.scaleDownAllTextures, false);
        config.read("display-controllers", g_drawController, false);
        config.readEnum("battery-status", g_videoSettings.batteryStatus, (int)BATTERY_STATUS_OFF, batteryStatus);
        config.read("osk-fill-screen", g_config.osk_fill_screen, false);
        config.endGroup();

        config.beginGroup("sound");
        config.read("disable-sound", g_audioSetup.disableSound, false);
        config.read("sample-rate", g_audioSetup.sampleRate, 44100);
        config.read("channels", g_audioSetup.channels, 2);
        config.readEnum("format", g_audioSetup.format, (uint16_t)AUDIO_F32, sampleFormats);
        config.read("buffer-size", g_audioSetup.bufferSize, 512);
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

        config.beginGroup("main");
        config.read("release", FileRelease, curRelease);
        config.read("full-screen", resBool, false);
        config.read("record-gameplay", g_config.RecordGameplayData, g_config.RecordGameplayData);
        config.endGroup();

        config.beginGroup("gameplay");
        config.read("ground-pound-by-alt-run", GameplayPoundByAltRun, false);
        config.read("strict-drop-add", g_config.StrictDropAdd, false);
        config.read("no-pause-reconnect", g_config.NoPauseReconnect, false);
        config.read("enter-cheats-menu-item", g_config.enter_cheats_menu_item, false);
        config.endGroup();

        config.beginGroup("effects");
        config.read("enable-thwomp-screen-shake", GameplayShakeScreenThwomp, true);
        config.read("enable-yoshi-ground-pound-screen-shake", GameplayShakeScreenPound, true);
        config.read("enable-bowser-iiird-screen-shake", GameplayShakeScreenBowserIIIrd, true);
        config.endGroup();

        Controls::LoadConfig(ctl);
    }
//    If resBool = True And resChanged = False And LevelEditor = False Then ChangeScreen
#ifndef __ANDROID__
    if(resBool && !resChanged)
        ChangeScreen();
#endif

    pLogDebug("Loaded config: %s", configPath.c_str());
}

void SaveConfig()
{
//    Dim A As Integer
    std::string configPath = AppPathManager::settingsFileSTD();
    std::string controlsPath = AppPathManager::settingsControlsFileSTD();

    IniProcessing config(configPath);
    IniProcessing controls(controlsPath);

    config.beginGroup("main");
    config.setValue("release", curRelease);
#if !defined(__EMSCRIPTEN__) && !defined(__ANDROID__) // Don't remember fullscreen state for Emscripten!
    config.setValue("full-screen", resChanged);
#endif
    // TODO: Make sure, saving of those settings will not been confused by line arguments
    // by separating config settings from global active settings
//    config.setValue("frame-skip", FrameSkip);
//    config.setValue("show-fps", ShowFPS);
    config.setValue("record-gameplay", g_config.RecordGameplayData);
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

        config.setValue("render", renderMode[g_videoSettings.renderMode]);
        config.setValue("background-work", g_videoSettings.allowBgWork);
        config.setValue("background-controller-input", g_videoSettings.allowBgControllerInput);
        config.setValue("frame-skip", g_videoSettings.enableFrameSkip);
        config.setValue("show-fps", g_videoSettings.showFrameRate);
        config.setValue("scale-down-all-textures", g_videoSettings.scaleDownAllTextures);
        config.setValue("display-controllers", g_drawController);
        config.setValue("battery-status", batteryStatus[g_videoSettings.batteryStatus]);
        config.setValue("osk-fill-screen", g_config.osk_fill_screen);
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
    config.setValue("ground-pound-by-alt-run", GameplayPoundByAltRun);
    config.setValue("strict-drop-add", g_config.StrictDropAdd);
    config.setValue("no-pause-reconnect", g_config.NoPauseReconnect);
    config.setValue("enter-cheats-menu-item", g_config.enter_cheats_menu_item);
    config.endGroup();

    config.beginGroup("effects");
    config.setValue("enable-thwomp-screen-shake", GameplayShakeScreenThwomp);
    config.setValue("enable-yoshi-ground-pound-screen-shake", GameplayShakeScreenPound);
    config.setValue("enable-bowser-iiird-screen-shake", GameplayShakeScreenBowserIIIrd);
    config.endGroup();

    Controls::SaveConfig(&controls);

    config.writeIniFile();
    controls.writeIniFile();
#ifdef __EMSCRIPTEN__
    AppPathManager::syncFs();
#endif

    pLogDebug("Saved config: %s, control mappings: %s", configPath.c_str(), controlsPath.c_str());
}
