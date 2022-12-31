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

#include <3ds.h>
#include <citro3d.h>

#include <set>

#include <Logger/logger.h>

#include "globals.h"
#include "sound.h"
#include "core/msgbox.h"

namespace XRender
{
extern bool g_in_frame;
}

namespace XMsgBox
{

bool init()
{
    return true;
}

void quit() {}

int simpleMsgBox(uint32_t flags, const std::string &title, const std::string &message)
{
    errorConf* conf = new errorConf;
    if(!conf)
        return -2;

    if(XRender::g_in_frame)
        C3D_FrameEnd(0);
    SoundPauseAll();

    std::string total = title;
    total += " - ";
    total += message;

    errorInit(conf, ERROR_TEXT_WORD_WRAP, CFG_LANGUAGE_EN);
    errorText(conf, total.c_str());
    errorDisp(conf);
    int ret = (conf->returnCode == ERROR_SUCCESS) ? 0 : -1;
    delete conf;

    if(XRender::g_in_frame)
        C3D_FrameBegin(0);
    SoundResumeAll();

    return ret;
}

void errorMsgBox(const std::string &title, const std::string &message)
{
    simpleMsgBox(MESSAGEBOX_ERROR, title, message);
}


} // namespace XMsgBox
