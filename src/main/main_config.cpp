/*
 * A2xTech - A platform game engine ported from old source code for VB6
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

#include "../globals.h"
#include "../game_main.h"
#include "../graphics.h"

#include <Utils/files.h>
#include <IniProcessor/ini_processing.h>
#include <fmt_format_ne.h>


void OpenConfig_preSetup()
{
    std::string configPath = AppPath + "config.ini";
    if(Files::fileExists(configPath))
    {
        IniProcessing config(configPath);
        config.beginGroup("main");
        config.read("render", RenderMode, 1);
        if(RenderMode > 2) // Allowed values: 0, 1 and 2
            RenderMode = 2;
        config.endGroup();
    }
}


void OpenConfig()
{
    int FileRelease = 0;
    bool resBool = false;
    std::string configPath = AppPath + "config.ini";

    if(Files::fileExists(configPath))
    {
        IniProcessing config(configPath);
        config.beginGroup("main");
        config.read("release", FileRelease, curRelease);
        config.read("full-screen", resBool, false);
        config.endGroup();

        For(A, 1, 2)
        {
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
            config.read("Run", conJoystick[A].Run, conJoystick[A].Run);
            config.read("Jump", conJoystick[A].Jump, conJoystick[A].Jump);
            config.read("Drop", conJoystick[A].Drop, conJoystick[A].Drop);
            config.read("Start", conJoystick[A].Start, conJoystick[A].Start);
            config.read("AltJump", conJoystick[A].AltJump, conJoystick[A].AltJump);
            config.read("AltRun", conJoystick[A].AltRun, conJoystick[A].AltRun);
            config.endGroup();
        }
    }
//    If resBool = True And resChanged = False And LevelEditor = False Then ChangeScreen
    if(resBool && !resChanged)
        ChangeScreen();
}

void SaveConfig()
{
//    Dim A As Integer
    std::string configPath = AppPath + "config.ini";
    IniProcessing config(configPath);
    config.beginGroup("main");
    config.setValue("release", curRelease);
    config.setValue("full-screen", resChanged);
    config.endGroup();

    For(A, 1, 2)
    {
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
        config.setValue("Run", conJoystick[A].Run);
        config.setValue("Jump", conJoystick[A].Jump);
        config.setValue("Drop", conJoystick[A].Drop);
        config.setValue("Start", conJoystick[A].Start);
        config.setValue("AltJump", conJoystick[A].AltJump);
        config.setValue("AltRun", conJoystick[A].AltRun);
        config.endGroup();
    }

    config.writeIniFile();
}
