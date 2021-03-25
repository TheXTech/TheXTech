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

#include <SDL2/SDL_audio.h>

#include "../globals.h"
#include "../game_main.h"
#include "../graphics.h"
#include "../sound.h"
#include "../control/joystick.h"

#include <Utils/files.h>
#include <Utils/strings.h>
#include <IniProcessor/ini_processing.h>
#include <fmt_format_ne.h>
#include <AppPath/app_path.h>
#include <Logger/logger.h>


void OpenConfig_preSetup()
{
    std::string configPath = AppPathManager::settingsFileSTD();
    if(Files::fileExists(configPath))
    {
        IniProcessing config(configPath);
        config.beginGroup("main");
        config.read("render", RenderMode, 1);
        if(RenderMode > 2) // Allowed values: 0, 1 and 2
            RenderMode = 2;
        config.endGroup();

        config.beginGroup("sound");
        config.read("sample-rate", g_audioSetup.sampleRate, 44100);
        config.read("channels", g_audioSetup.channels, 2);
        IniProcessing::StrEnumMap sampleFormats =
        {
            {"s8", AUDIO_S8},
            {"pcm_s8", AUDIO_S8},
            {"u8", AUDIO_U8},
            {"pcm_u8", AUDIO_U8},
            {"s16", AUDIO_S16},
            {"pcm_s16", AUDIO_S16},
            {"u16", AUDIO_U16},
            {"pcm_u16", AUDIO_U16},
            {"s32", AUDIO_S32},
            {"pcm_s32", AUDIO_S32},
            {"float32", AUDIO_F32},
            {"pcm_f32", AUDIO_F32}
        };
        config.readEnum("format", g_audioSetup.format, (uint16_t)AUDIO_F32, sampleFormats);
        config.read("buffer-size", g_audioSetup.bufferSize, 512);
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

    if(Files::fileExists(configPath))
    {
        IniProcessing config(configPath);
        config.beginGroup("main");
        config.read("release", FileRelease, curRelease);
        config.read("full-screen", resBool, false);
        config.read("frame-skip", FrameSkip, FrameSkip);
        config.read("show-fps", ShowFPS, ShowFPS);
        config.endGroup();

        config.beginGroup("gameplay");
        config.read("ground-pound-by-alt-run", GameplayPoundByAltRun, false);
        config.endGroup();

        config.beginGroup("joystick");
        config.read("enable-rumble", JoystickEnableRumble, true);
        config.read("enable-battery-status", JoystickEnableBatteryStatus, true);
        config.endGroup();

        For(A, 1, 2)
        {
            auto keys = config.childGroups();
            auto keyNeed = fmt::format_ne("joystick-uuid-{0}-", A);

            for(auto &k : keys)
            {
                auto r = k.find(keyNeed);
                if(r != std::string::npos && r == 0)
                {
                    std::string u;
                    config.beginGroup(k);
                    config.read("device-uuid", u, "");
                    if(u.empty())
                    {
                        config.endGroup();
                        continue;
                    }
                    auto &j = joyGetByUuid(A, u);
                    readJoyKey(config, "Up", j.Up);
                    readJoyKey(config, "Down", j.Down);
                    readJoyKey(config, "Left", j.Left);
                    readJoyKey(config, "Right", j.Right);
                    readJoyKey(config, "Run", j.Run);
                    readJoyKey(config, "Jump", j.Jump);
                    readJoyKey(config, "Drop", j.Drop);
                    readJoyKey(config, "Start", j.Start);
                    readJoyKey(config, "AltJump", j.AltJump);
                    readJoyKey(config, "AltRun", j.AltRun);
                    config.endGroup();
                }
            }

            config.beginGroup(fmt::format_ne("player-{0}-keyboard", A));
            config.read("device", useJoystick[A], useJoystick[A]);
            config.read("Up", conKeyboard[A].Up, conKeyboard[A].Up);
            config.read("Down", conKeyboard[A].Down, conKeyboard[A].Down);
            config.read("Left", conKeyboard[A].Left, conKeyboard[A].Left);
            config.read("Right", conKeyboard[A].Right, conKeyboard[A].Right);
            config.read("Run", conKeyboard[A].Run, conKeyboard[A].Run);
            config.read("Jump", conKeyboard[A].Jump, conKeyboard[A].Jump);
            config.read("Drop", conKeyboard[A].Drop, conKeyboard[A].Drop);
            config.read("Start", conKeyboard[A].Start, conKeyboard[A].Start);
            config.read("AltJump", conKeyboard[A].AltJump, conKeyboard[A].AltJump);
            config.read("AltRun", conKeyboard[A].AltRun, conKeyboard[A].AltRun);
            config.endGroup();

            config.beginGroup(fmt::format_ne("player-{0}-joystick", A));
//            config.read("used-device", conJoystick[A].hwGUID, std::string());
            readJoyKey(config, "Up", conJoystick[A].Up);
            readJoyKey(config, "Down", conJoystick[A].Down);
            readJoyKey(config, "Left", conJoystick[A].Left);
            readJoyKey(config, "Right", conJoystick[A].Right);
            readJoyKey(config, "Run", conJoystick[A].Run);
            readJoyKey(config, "Jump", conJoystick[A].Jump);
            readJoyKey(config, "Drop", conJoystick[A].Drop);
            readJoyKey(config, "Start", conJoystick[A].Start);
            readJoyKey(config, "AltJump", conJoystick[A].AltJump);
            readJoyKey(config, "AltRun", conJoystick[A].AltRun);
            config.endGroup();
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
    IniProcessing config(configPath);

    config.beginGroup("main");
    config.setValue("release", curRelease);
#if !defined(__EMSCRIPTEN__) && !defined(__ANDROID__) // Don't remember fullscreen state for Emscripten!
    config.setValue("full-screen", resChanged);
#endif
    // TODO: Make sure, saving of those settings will not been confused by line arguments
//    config.setValue("frame-skip", FrameSkip);
//    config.setValue("show-fps", ShowFPS);
    config.endGroup();

    config.beginGroup("gameplay");
    config.setValue("ground-pound-by-alt-run", GameplayPoundByAltRun);
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
            config.beginGroup(fmt::format_ne("joystick-uuid-{0}-{1}", A, u));
            config.setValue("device-uuid", u);
            writeJoyKey(config, "Up", j.Up);
            writeJoyKey(config, "Down", j.Down);
            writeJoyKey(config, "Left", j.Left);
            writeJoyKey(config, "Right", j.Right);
            writeJoyKey(config, "Run", j.Run);
            writeJoyKey(config, "Jump", j.Jump);
            writeJoyKey(config, "Drop", j.Drop);
            writeJoyKey(config, "Start", j.Start);
            writeJoyKey(config, "AltJump", j.AltJump);
            writeJoyKey(config, "AltRun", j.AltRun);
            config.endGroup();
        }

        config.beginGroup(fmt::format_ne("player-{0}-keyboard", A));
        config.setValue("device", useJoystick[A]);
        config.setValue("Up", conKeyboard[A].Up);
        config.setValue("Down", conKeyboard[A].Down);
        config.setValue("Left", conKeyboard[A].Left);
        config.setValue("Right", conKeyboard[A].Right);
        config.setValue("Run", conKeyboard[A].Run);
        config.setValue("Jump", conKeyboard[A].Jump);
        config.setValue("Drop", conKeyboard[A].Drop);
        config.setValue("Start", conKeyboard[A].Start);
        config.setValue("AltJump", conKeyboard[A].AltJump);
        config.setValue("AltRun", conKeyboard[A].AltRun);
        config.endGroup();

        config.beginGroup(fmt::format_ne("player-{0}-joystick", A));
//        config.setValue("used-device", conJoystick[A].hwGUID);
        writeJoyKey(config, "Up", conJoystick[A].Up);
        writeJoyKey(config, "Down", conJoystick[A].Down);
        writeJoyKey(config, "Left", conJoystick[A].Left);
        writeJoyKey(config, "Right", conJoystick[A].Right);
        writeJoyKey(config, "Run", conJoystick[A].Run);
        writeJoyKey(config, "Jump", conJoystick[A].Jump);
        writeJoyKey(config, "Drop", conJoystick[A].Drop);
        writeJoyKey(config, "Start", conJoystick[A].Start);
        writeJoyKey(config, "AltJump", conJoystick[A].AltJump);
        writeJoyKey(config, "AltRun", conJoystick[A].AltRun);
        config.endGroup();
    }

    config.writeIniFile();
#ifdef __EMSCRIPTEN__
    AppPathManager::syncFs();
#endif

    pLogDebug("Saved config: %s", configPath.c_str());
}
