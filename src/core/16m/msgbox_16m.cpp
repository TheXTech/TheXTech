/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2024 Vitaly Novichkov <admin@wohlnet.ru>
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

#include <nds.h>

#include <set>

#include <Logger/logger.h>

#include "globals.h"
#include "sound.h"
#include "core/msgbox.h"

namespace XMsgBox
{

bool init()
{
    return true;
}

void quit() {}

int simpleMsgBox(uint32_t flags, const std::string &title, const std::string &message)
{
    UNUSED(flags);

    SoundPauseAll();

    printf("\n===============\n%s\n===============\n\n%s\n\nPress any button...\n", title.c_str(), message.c_str());

    while(1) {
        swiWaitForVBlank();
        scanKeys();
        if(keysDown()) break;
    }


    SoundResumeAll();

    return 0;
}

void errorMsgBox(const std::string &title, const std::string &message)
{
    simpleMsgBox(MESSAGEBOX_ERROR, title, message);
}


}; // namespace XMsgBox
