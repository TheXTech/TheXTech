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

#ifndef NO_SDL
#include <SDL2/SDL_audio.h>
#endif

#include "../globals.h"
#include "../game_main.h"
#include "../graphics.h"
#include "../sound.h"
#include "../video.h"
#include "../control/joystick.h"
#include "../config.h"

#include "speedrunner.h"
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

#ifndef NO_SDL
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
#endif

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
        config.endGroup();

#ifndef NO_SDL
        config.beginGroup("sound");
        config.read("disable-sound", g_audioSetup.disableSound, false);
        config.read("sample-rate", g_audioSetup.sampleRate, 44100);
        config.read("channels", g_audioSetup.channels, 2);
        config.readEnum("format", g_audioSetup.format, (uint16_t)AUDIO_F32, sampleFormats);
        config.read("buffer-size", g_audioSetup.bufferSize, 512);
        config.endGroup();
#endif // #ifndef NO_SDL

        config.beginGroup("video");
#ifndef FIXED_RES
        config.read("internal-width", g_config.InternalW, 800);
        config.read("internal-height", g_config.InternalH, 600);
#endif
        IniProcessing::StrEnumMap scaleModes =
        {
            {"linear", SCALE_DYNAMIC_LINEAR},
            {"integer", SCALE_DYNAMIC_INTEGER},
            {"nearest", SCALE_DYNAMIC_NEAREST},
            {"1x", SCALE_FIXED_1X},
            {"2x", SCALE_FIXED_2X},
        };
        config.readEnum("scale-mode", g_videoSettings.ScaleMode, (int)SCALE_DYNAMIC_NEAREST, scaleModes);
        config.endGroup();
    }
}

static void readJoyKey(IniProcessing &setup, const char *n, KM_Key &key)
{
    std::string joyKey(n);
    setup.read((joyKey + "-val").c_str(), key.val, key.val);
    setup.read((joyKey + "-id").c_str(), key.id, key.id);
    setup.read((joyKey + "-type").c_str(), key.type, key.type);
    setup.read((joyKey + "-ctrl-val").c_str(), key.ctrl_val, key.ctrl_val);
    setup.read((joyKey + "-ctrl-id").c_str(), key.ctrl_id, key.ctrl_id);
    setup.read((joyKey + "-ctrl-type").c_str(), key.ctrl_type, key.ctrl_type);
}

static void writeJoyKey(IniProcessing &setup, const char *n, KM_Key &key)
{
    std::string joyKey(n);
    setup.setValue((joyKey + "-val").c_str(), key.val);
    setup.setValue((joyKey + "-id").c_str(), key.id);
    setup.setValue((joyKey + "-type").c_str(), key.type);
    setup.setValue((joyKey + "-ctrl-val").c_str(), key.ctrl_val);
    setup.setValue((joyKey + "-ctrl-id").c_str(), key.ctrl_id);
    setup.setValue((joyKey + "-ctrl-type").c_str(), key.ctrl_type);
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
        config.read("record-gameplay", g_recordControlRecord, g_recordControlRecord);
        config.read("new-editor", g_config.UseNewEditor, false);
        config.endGroup();

        config.beginGroup("video");
        config.read("full-screen", resBool, resBool);
        config.endGroup();

        config.beginGroup("gameplay");
        config.read("ground-pound-by-alt-run", GameplayPoundByAltRun, false);
        config.read("world-map-fast-move", g_config.FastMove, false);
        config.endGroup();

        config.beginGroup("effects");
        config.read("enable-thwomp-screen-shake", GameplayShakeScreenThwomp, true);
        config.read("enable-yoshi-ground-pound-screen-shake", GameplayShakeScreenPound, true);
        config.read("enable-bowser-iiird-screen-shake", GameplayShakeScreenBowserIIIrd, true);
        config.endGroup();

        config.beginGroup("joystick");
        config.read("enable-rumble", JoystickEnableRumble, true);
        config.read("enable-battery-status", JoystickEnableBatteryStatus, true);
        config.endGroup();

        For(A, 1, 2)
        {
            auto keys = ctl->childGroups();
            auto keyNeed = fmt::format_ne("joystick-uuid-{0}-", A);

            for(auto &k : keys)
            {
                auto r = k.find(keyNeed);
                if(r != std::string::npos && r == 0)
                {
                    std::string u;
                    ctl->beginGroup(k);
                    ctl->read("device-uuid", u, "");
                    if(u.empty())
                    {
                        ctl->endGroup();
                        continue;
                    }
                    auto &j = joyGetByUuid(A, u);
                    readJoyKey(*ctl, "Up", j.Up);
                    readJoyKey(*ctl, "Down", j.Down);
                    readJoyKey(*ctl, "Left", j.Left);
                    readJoyKey(*ctl, "Right", j.Right);
                    readJoyKey(*ctl, "Run", j.Run);
                    readJoyKey(*ctl, "Jump", j.Jump);
                    readJoyKey(*ctl, "Drop", j.Drop);
                    readJoyKey(*ctl, "Start", j.Start);
                    readJoyKey(*ctl, "AltJump", j.AltJump);
                    readJoyKey(*ctl, "AltRun", j.AltRun);
                    ctl->endGroup();
                }
            }

            ctl->beginGroup(fmt::format_ne("player-{0}-keyboard", A));
            ctl->read("device", useJoystick[A], useJoystick[A]);
            ctl->read("wanted-keyboard", wantedKeyboard[A], wantedKeyboard[A]);
            ctl->read("Up", conKeyboard[A].Up, conKeyboard[A].Up);
            ctl->read("Down", conKeyboard[A].Down, conKeyboard[A].Down);
            ctl->read("Left", conKeyboard[A].Left, conKeyboard[A].Left);
            ctl->read("Right", conKeyboard[A].Right, conKeyboard[A].Right);
            ctl->read("Run", conKeyboard[A].Run, conKeyboard[A].Run);
            ctl->read("Jump", conKeyboard[A].Jump, conKeyboard[A].Jump);
            ctl->read("Drop", conKeyboard[A].Drop, conKeyboard[A].Drop);
            ctl->read("Start", conKeyboard[A].Start, conKeyboard[A].Start);
            ctl->read("AltJump", conKeyboard[A].AltJump, conKeyboard[A].AltJump);
            ctl->read("AltRun", conKeyboard[A].AltRun, conKeyboard[A].AltRun);
            ctl->endGroup();

            ctl->beginGroup(fmt::format_ne("player-{0}-joystick", A));
//            config.read("used-device", conJoystick[A].hwGUID, std::string());
            readJoyKey(*ctl, "Up", conJoystick[A].Up);
            readJoyKey(*ctl, "Down", conJoystick[A].Down);
            readJoyKey(*ctl, "Left", conJoystick[A].Left);
            readJoyKey(*ctl, "Right", conJoystick[A].Right);
            readJoyKey(*ctl, "Run", conJoystick[A].Run);
            readJoyKey(*ctl, "Jump", conJoystick[A].Jump);
            readJoyKey(*ctl, "Drop", conJoystick[A].Drop);
            readJoyKey(*ctl, "Start", conJoystick[A].Start);
            readJoyKey(*ctl, "AltJump", conJoystick[A].AltJump);
            readJoyKey(*ctl, "AltRun", conJoystick[A].AltRun);
            ctl->endGroup();
        }
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
    config.setValue("new-editor", g_config.UseNewEditor);
    // TODO: Make sure, saving of those settings will not been confused by line arguments
    // by separating config settings from global active settings
//    config.setValue("frame-skip", FrameSkip);
//    config.setValue("show-fps", ShowFPS);
//    config.setValue("record-gameplay", g_recordControlRecord);
    config.endGroup();

#if !defined(NO_SDL)
#   if !defined(__3DS__)
    config.beginGroup("video");
    {
#       if !defined(__EMSCRIPTEN__) && !defined(__ANDROID__) // Don't remember fullscreen state for Emscripten!
        config.setValue("full-screen", resChanged);
#       endif
#       ifndef FIXED_RES
        config.setValue("internal-width", g_config.InternalW);
        config.setValue("internal-height", g_config.InternalH);
#       endif
        config.setValue("scale-mode", ScaleMode_strings.at(g_videoSettings.ScaleMode));

        std::unordered_map<int, std::string> renderMode =
        {
            {RENDER_SOFTWARE, "sw"},
            {RENDER_ACCELERATED, "hw"},
            {RENDER_ACCELERATED_VSYNC, "vsync"},
        };

        config.setValue("render", renderMode[g_videoSettings.renderMode]);
        config.setValue("background-work", g_videoSettings.allowBgWork);
        config.setValue("background-controller-input", g_videoSettings.allowBgControllerInput);
        config.setValue("frame-skip", g_videoSettings.enableFrameSkip);
        config.setValue("show-fps", g_videoSettings.showFrameRate);
        config.setValue("scale-down-all-textures", g_videoSettings.scaleDownAllTextures);
        config.setValue("display-controllers", g_drawController);
    }
    config.endGroup();
#   endif // !defined(__3DS__)

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
#endif // !defined(NO_SDL)

    config.beginGroup("gameplay");
    config.setValue("ground-pound-by-alt-run", GameplayPoundByAltRun);
    config.setValue("world-map-fast-move", g_config.FastMove);
    config.endGroup();

    config.beginGroup("effects");
    config.setValue("enable-thwomp-screen-shake", GameplayShakeScreenThwomp);
    config.setValue("enable-yoshi-ground-pound-screen-shake", GameplayShakeScreenPound);
    config.setValue("enable-bowser-iiird-screen-shake", GameplayShakeScreenBowserIIIrd);
    config.endGroup();

    config.beginGroup("joystick");
    config.setValue("enable-rumble", JoystickEnableRumble);
    config.setValue("enable-battery-status", JoystickEnableBatteryStatus);
    config.endGroup();

    For(A, 1, 2)
    {
        std::vector<std::string> joystickUuid;
        joyGetAllUUIDs(A, joystickUuid);

        for(auto &u : joystickUuid)
        {
            auto &j = joyGetByUuid(A, u);
            controls.beginGroup(fmt::format_ne("joystick-uuid-{0}-{1}", A, u));
            controls.setValue("device-uuid", u);
            writeJoyKey(controls, "Up", j.Up);
            writeJoyKey(controls, "Down", j.Down);
            writeJoyKey(controls, "Left", j.Left);
            writeJoyKey(controls, "Right", j.Right);
            writeJoyKey(controls, "Run", j.Run);
            writeJoyKey(controls, "Jump", j.Jump);
            writeJoyKey(controls, "Drop", j.Drop);
            writeJoyKey(controls, "Start", j.Start);
            writeJoyKey(controls, "AltJump", j.AltJump);
            writeJoyKey(controls, "AltRun", j.AltRun);
            controls.endGroup();
        }

        controls.beginGroup(fmt::format_ne("player-{0}-keyboard", A));
        controls.setValue("device", useJoystick[A]);
        controls.setValue("wanted-keyboard", wantedKeyboard[A]);
        controls.setValue("Up", conKeyboard[A].Up);
        controls.setValue("Down", conKeyboard[A].Down);
        controls.setValue("Left", conKeyboard[A].Left);
        controls.setValue("Right", conKeyboard[A].Right);
        controls.setValue("Run", conKeyboard[A].Run);
        controls.setValue("Jump", conKeyboard[A].Jump);
        controls.setValue("Drop", conKeyboard[A].Drop);
        controls.setValue("Start", conKeyboard[A].Start);
        controls.setValue("AltJump", conKeyboard[A].AltJump);
        controls.setValue("AltRun", conKeyboard[A].AltRun);
        controls.endGroup();

        controls.beginGroup(fmt::format_ne("player-{0}-joystick", A));
//        config.setValue("used-device", conJoystick[A].hwGUID);
        writeJoyKey(controls, "Up", conJoystick[A].Up);
        writeJoyKey(controls, "Down", conJoystick[A].Down);
        writeJoyKey(controls, "Left", conJoystick[A].Left);
        writeJoyKey(controls, "Right", conJoystick[A].Right);
        writeJoyKey(controls, "Run", conJoystick[A].Run);
        writeJoyKey(controls, "Jump", conJoystick[A].Jump);
        writeJoyKey(controls, "Drop", conJoystick[A].Drop);
        writeJoyKey(controls, "Start", conJoystick[A].Start);
        writeJoyKey(controls, "AltJump", conJoystick[A].AltJump);
        writeJoyKey(controls, "AltRun", conJoystick[A].AltRun);
        controls.endGroup();
    }

    config.writeIniFile();
    controls.writeIniFile();
#ifdef __EMSCRIPTEN__
    AppPathManager::syncFs();
#endif

    pLogDebug("Saved config: %s, control mappings: %s", configPath.c_str(), controlsPath.c_str());
}
