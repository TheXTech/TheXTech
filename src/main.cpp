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

#include "game_main.h"
#include <SDL2/SDL.h>
#include <AppPath/app_path.h>
#include <tclap/CmdLine.h>

extern "C"
int main(int argc, char**argv)
{
    CmdLineSetup_t setup;

    AppPathManager::initAppPath();
    AppPath = AppPathManager::userAppDirSTD();

    OpenConfig_preSetup();

    setup.renderType = CmdLineSetup_t::RenderType(RenderMode);

    try
    {
        // Define the command line object.
        TCLAP::CmdLine  cmd("A2xTech\n"
                            "Copyright (c) 2020-2020 Vitaly Novichkov <admin@wohlnet.ru>\n"
                            "This program is distributed under the MIT license\n", ' ', "1.3");

        TCLAP::SwitchArg switchFrameSkip("f", "frameskip", "Enable frame skipping mode", false);
        TCLAP::SwitchArg switchNoSound("s", "no-sound", "Disable sound", false);
        TCLAP::SwitchArg switchNoPause("p", "never-pause", "Never pause game when window losts a focus", false);
        TCLAP::ValueArg<std::string> renderType("r", "render", "Render mode: sw (software), hw (hardware), vsync (hardware with vsync)",
                                                false, "",
                                                "render type",
                                                cmd);

        cmd.add(&switchFrameSkip);
        cmd.add(&switchNoSound);
        cmd.add(&switchNoPause);

        cmd.parse(argc, argv);

        setup.frameSkip = switchFrameSkip.getValue();
        setup.noSound   = switchNoSound.getValue();
        setup.neverPause = switchNoPause.getValue();
        std::string rt = renderType.getValue();
        if(rt == "sw")
            setup.renderType = CmdLineSetup_t::RENDER_SW;
        else if(rt == "vsync")
            setup.renderType = CmdLineSetup_t::RENDER_VSYNC;
        else if(rt == "hw")
            setup.renderType = CmdLineSetup_t::RENDER_HW;
    }
    catch(TCLAP::ArgException &e)   // catch any exceptions
    {
        std::cerr << "Error: " << e.error() << " for arg " << e.argId() << std::endl;
        return 2;
    }

    // set this flag before SDL initialization to allow game be quit when closing a window before a loading process will be completed
    GameIsActive = true;

    if(frmMain.initSDL(setup))
    {
        frmMain.freeSDL();
        return 1;
    }

    int ret = GameMain(setup);
    frmMain.freeSDL();

    return ret;
}
