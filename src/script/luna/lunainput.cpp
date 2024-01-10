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

#include "luna.h"
#include "lunainput.h"
#include "lunaplayer.h"
#include "lunacounter.h"
#include "main/cheat_code.h"
#include "sound.h"
#include "autocode_manager.h"

int gFrames;

int gLastDownPress;
int gDownTapped;
int gLastUpPress;
int gUpTapped;
int gLastLeftPress;
int gLeftTapped;
int gLastRightPress;
int gRightTapped;

int gLastJumpPress;
int gJumpTapped;
int gLastRunPress;
int gRunTapped;


bool Input::PressingUp()
{
    return gLastUpPress == gFrames;
}

bool Input::PressingDown()
{
    return gLastDownPress == gFrames;
}

bool Input::PressingLeft()
{
    return gLastLeftPress == gFrames;
}

bool Input::PressingRight()
{
    return gLastRightPress == gFrames;
}

bool Input::PressingRun()
{
    return gLastRunPress == gFrames;
}

bool Input::PressingJump()
{
    return gLastJumpPress == gFrames;
}

bool Input::UpThisFrame()
{
    return gUpTapped != 0;
}

bool Input::DownThisFrame()
{
    return gDownTapped != 0;
}

bool Input::LeftThisFrame()
{
    return gLeftTapped != 0;
}

bool Input::RightThisFrame()
{
    return gRightTapped != 0;
}

bool Input::RunThisFrame()
{
    return gRunTapped != 0;
}

bool Input::JumpThisFrame()
{
    return gJumpTapped != 0;
}

void Input::UpdateKeyRecords(Player_t *pPlayer)
{
    if(!pPlayer)
        return;

    //wchar_t* dbg = L"Update keys debug";

    // Up
    if(PlayerF::PressingUp(pPlayer))
    {
        // If still holding from last frame
        if(gFrames - 1 == gLastUpPress)
            gLastUpPress = gFrames;
        else   //else set tapped this frame
        {
            gLastUpPress = gFrames;
            gUpTapped = gFrames;
        }
    }

    // Down
    if(PlayerF::PressingDown(pPlayer))
    {
        // If still holding from last frame
        if(gFrames - 1 == gLastDownPress)
            gLastDownPress = gFrames;
        else   //else set tapped this frame
        {
            gLastDownPress = gFrames;
            gDownTapped = gFrames;
        }
    }

    // Left
    if(PlayerF::PressingLeft(pPlayer))
    {
        // If still holding from last frame
        if(gFrames - 1 == gLastLeftPress)
            gLastLeftPress = gFrames;
        else   //else set tapped this frame
        {
            gLastLeftPress = gFrames;
            gLeftTapped = gFrames;
        }
    }

    // Right
    if(PlayerF::PressingRight(pPlayer))
    {
        // If still holding from last frame
        if(gFrames - 1 == gLastRightPress)
            gLastRightPress = gFrames;
        else   //else set tapped this frame
        {
            gLastRightPress = gFrames;
            gRightTapped = gFrames;
        }
    }

    // Jump
    if(PlayerF::PressingJump(pPlayer))
    {
        // If still holding from last frame
        if(gFrames - 1 == gLastJumpPress)
            gLastJumpPress = gFrames;
        else   //else set tapped this frame
        {
            gLastJumpPress = gFrames;
            gJumpTapped = gFrames;
        }
    }

    // Run
    if(PlayerF::PressingRun(pPlayer))
    {
        // If still holding from last frame
        if(gFrames - 1 == gLastRunPress)
            gLastRunPress = gFrames;
        else   //else set tapped this frame
        {
            gLastRunPress = gFrames;
            gRunTapped = gFrames;
        }
    }
}

static const std::string FULL_LUNA_TOGGLE_CHT    = "thouartdamned";
static const std::string TOGGLE_DEMO_COUNTER_CHT = "toggledemocounter";
static const std::string DELETE_ALL_RECORDS_CHT  = "formatcdrive";
static const std::string LUNA_DEBUG_CHT          = "lunadebug";
static const std::string LUNA_LONG_DEBUG_CHT     = "lunalongdebug";

void Input::CheckSpecialCheats()
{
    if(cheats_contains(FULL_LUNA_TOGGLE_CHT))
    {
        gLunaEnabled = !gLunaEnabled;
        PlaySound(SFX_Smash);
        cheats_clearBuffer();
        return;
    }

    else if(cheats_contains(LUNA_DEBUG_CHT) || cheats_contains(LUNA_LONG_DEBUG_CHT))
    {
        int length = cheats_contains(LUNA_LONG_DEBUG_CHT) ? 99999 : 600;
        // FIXME: Replace this with the boolean toggle than adding this command infinitely times
        gAutoMan.m_CustomCodes.emplace_back(AT_DebugPrint, 0, 0, 0, 0, STRINGINDEX_NONE, length, 0, STRINGINDEX_NONE);
        PlaySound(SFX_Stomp);
        cheats_clearBuffer();
        return;
    }

    if(gEnableDemoCounter)
    {
        if(cheats_contains(TOGGLE_DEMO_COUNTER_CHT))
        {
            gShowDemoCounter = !gShowDemoCounter;
            PlaySound(SFX_Smash);
            cheats_clearBuffer();
            return;
        }
        else if(cheats_contains(DELETE_ALL_RECORDS_CHT))
        {
            gDeathCounter.ClearRecords();
            gDeathCounter.TrySave();
            gDeathCounter.Recount();
            PlaySound(SFX_Smash);
            cheats_clearBuffer();
            return;
        }
    }
}

void Input::UpdateInputTasks()
{
    //wchar_t* dbg = L"Input tasks debug";
    ResetTaps();
    UpdateKeyRecords(PlayerF::Get(1));
}

void Input::ResetTaps()
{
    gDownTapped = 0;
    gUpTapped = 0;
    gLeftTapped = 0;
    gRightTapped = 0;
    gRunTapped = 0;
    gJumpTapped = 0;
}

void Input::ResetAll()
{
    gLastDownPress = 0;
    gDownTapped = 0;
    gLastUpPress = 0;
    gUpTapped = 0;
    gLastLeftPress = 0;
    gLeftTapped = 0;
    gLastRightPress = 0;
    gRightTapped = 0;
    gLastJumpPress = 0;
    gJumpTapped = 0;
    gLastRunPress = 0;
    gRunTapped = 0;
}
