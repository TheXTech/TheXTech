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

#include <string>

#include <nn/erreula.h>
#include <coreinit/filesystem.h>
#include <coreinit/memdefaultheap.h>

#include <whb/proc.h>
#include <whb/gfx.h>

#include "core/msgbox.h"
#include "msgbox_wiiu.h"

#include <Logger/logger.h>
#include <PGE_File_Formats/charsetconvert.h>

#include <core/render.h>

void MsgBoxWiiU::run()
{
    while(true)
    {
        VPADStatus vpadStatus;
        VPADRead(VPAD_CHAN_0, &vpadStatus, 1, nullptr);
        VPADGetTPCalibratedPoint(VPAD_CHAN_0, &vpadStatus.tpNormal, &vpadStatus.tpNormal);

        nn::erreula::ControllerInfo controllerInfo;
        controllerInfo.vpad = &vpadStatus;
        controllerInfo.kpad[0] = nullptr;
        controllerInfo.kpad[1] = nullptr;
        controllerInfo.kpad[2] = nullptr;
        controllerInfo.kpad[3] = nullptr;
        nn::erreula::Calc(controllerInfo);

        if(nn::erreula::IsDecideSelectButtonError())
        {
            nn::erreula::DisappearErrorViewer();
            XRender::repaint();
            break;
        }

        XRender::repaint();
    }
}

MsgBoxWiiU::MsgBoxWiiU() :
    AbstractMsgBox_t()
{}

MsgBoxWiiU::~MsgBoxWiiU()
{}

void MsgBoxWiiU::close()
{}

int MsgBoxWiiU::simpleMsgBox(uint32_t flags, const std::string& title, const std::string& message)
{
    int ret = 0;
    FSClient *fsClient = (FSClient *)MEMAllocFromDefaultHeap(sizeof(FSClient));
    FSAddClient(fsClient, FS_ERROR_FLAG_NONE);
    std::u16string title16;
    std::u16string title16full;
    std::u16string message16;

    SI_ConvertW<char16_t> utf8(true);
    size_t new_len;

    title16.resize(title.size() * 2);
    message16.resize(message.size() * 2);

    new_len = title.size() * 2;
    utf8.ConvertFromStore(title.c_str(), title.length(), &title16[0], new_len);

    new_len = message.size() * 2;
    utf8.ConvertFromStore(message.c_str(), message.length(), &message16[0], new_len);

    nn::erreula::CreateArg createArg;
    createArg.region = nn::erreula::RegionType::Europe;
    createArg.language = nn::erreula::LangType::English;
    createArg.workMemory =  MEMAllocFromDefaultHeap(nn::erreula::GetWorkMemorySize());
    createArg.fsClient = fsClient;

    if(!nn::erreula::Create(createArg))
    {
        pLogCritical("WiiU: Failed to create the message box via nn::erreula::Create.");
        return -1;
    }

    nn::erreula::AppearArg appearArg;
    appearArg.errorArg.errorType = nn::erreula::ErrorType::Message1Button;
    appearArg.errorArg.renderTarget = nn::erreula::RenderTarget::Both;
    appearArg.errorArg.controllerType = nn::erreula::ControllerType::DrcGamepad;

    if(flags & XMsgBox::MESSAGEBOX_ERROR)
        title16full = u"Error: " + title16;
    else if(flags & XMsgBox::MESSAGEBOX_WARNING)
        title16full = u"Warning: " + title16;
    else if(flags & XMsgBox::MESSAGEBOX_INFORMATION)
        title16full = u"Info: " + title16;
    else
        title16full = title16;

    appearArg.errorArg.errorTitle = title16full.c_str();
    appearArg.errorArg.errorMessage = message16.c_str();
    appearArg.errorArg.button1Label = u"Ok";
    nn::erreula::AppearErrorViewer(appearArg);

    run();

    ret = nn::erreula::GetResultCode();

    nn::erreula::Destroy();
    MEMFreeToDefaultHeap(createArg.workMemory);

    FSDelClient(fsClient, FS_ERROR_FLAG_NONE);
    MEMFreeToDefaultHeap(fsClient);

    return ret;
}

void MsgBoxWiiU::errorMsgBox(const std::string& title, const std::string& message)
{
    simpleMsgBox(XMsgBox::MESSAGEBOX_ERROR, title, message);
}
