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

#include <Logger/logger.h>
#include <Utils/maths.h>

#include "duplicate.h"
#include "../globals.h"

namespace Controls
{

/*====================================================*\
|| implementation for InputMethod_Duplicate         ||
\*====================================================*/

// Update functions that set player controls (and editor controls)
// based on current device input. Return false if device lost.
bool InputMethod_Duplicate::Update(Controls_t& c, CursorControls_t& m, EditorControls_t& e)
{
    if(this->player_no < 1 || this->player_no > maxLocalPlayers)
        return false;
    if(this->player_no - 1 >= (int)g_InputMethods.size())
        return false;
    if(!g_InputMethods[this->player_no - 1])
        return false;
    c = Player[this->player_no].Controls;

    return true;
}

void InputMethod_Duplicate::Rumble(int ms, float strength)
{
    Controls::Rumble(this->player_no, ms, strength);
}

/*====================================================*\
|| implementation for InputMethodProfile_Duplicate  ||
\*====================================================*/

// the job of this function is to initialize the class in a consistent state
InputMethodProfile_Duplicate::InputMethodProfile_Duplicate()
{
}

bool InputMethodProfile_Duplicate::PollPrimaryButton(ControlsClass c, size_t i)
{
    (void)c;
    (void)i;
    return true;
}

bool InputMethodProfile_Duplicate::PollSecondaryButton(ControlsClass c, size_t i)
{
    (void)c;
    (void)i;
    return true;
}

bool InputMethodProfile_Duplicate::DeletePrimaryButton(ControlsClass c, size_t i)
{
    (void)c;
    (void)i;
    return true;
}

bool InputMethodProfile_Duplicate::DeleteSecondaryButton(ControlsClass c, size_t i)
{
    (void)c;
    (void)i;
    return true;
}

const char* InputMethodProfile_Duplicate::NamePrimaryButton(ControlsClass c, size_t i)
{
    (void)c;
    (void)i;
    return "(DUP)";
}

const char* InputMethodProfile_Duplicate::NameSecondaryButton(ControlsClass c, size_t i)
{
    (void)c;
    (void)i;
    return "";
}

void InputMethodProfile_Duplicate::SaveConfig(IniProcessing* ctl)
{
    (void)ctl;
}

void InputMethodProfile_Duplicate::LoadConfig(IniProcessing* ctl)
{
    (void)ctl;
}

/*====================================================*\
|| implementation for InputMethodType_Duplicate     ||
\*====================================================*/

InputMethodProfile* InputMethodType_Duplicate::AllocateProfile() noexcept
{
    return (InputMethodProfile*) new(std::nothrow) InputMethodProfile_Duplicate;
}

InputMethodType_Duplicate::InputMethodType_Duplicate()
{
    this->Name = "Duplicate";
}

bool InputMethodType_Duplicate::TestProfileType(InputMethodProfile* profile)
{
    return (bool)dynamic_cast<InputMethodProfile_Duplicate*>(profile);
}

void InputMethodType_Duplicate::UpdateControlsPre()
{
}

void InputMethodType_Duplicate::UpdateControlsPost()
{
}

InputMethod* InputMethodType_Duplicate::Poll(const std::vector<InputMethod*>& active_methods) noexcept
{
    int found = 0;

    for(int p = 1; p <= maxLocalPlayers; p++)
    {
        if(p > numPlayers)
            continue;
        if(!Player[p].Controls.Drop || !Player[p].Controls.Up)
            continue;
        if(p - 1 >= (int)active_methods.size())
            continue;
        if(!active_methods[p - 1])
            continue;

        bool allowed = true;
        for(InputMethod* method : active_methods)
        {
            if(!method)
                continue;
            InputMethod_Duplicate* m = dynamic_cast<InputMethod_Duplicate*>(method);
            if(m && m->player_no == p)
            {
                allowed = false;
                break;
            }
        }

        if(!allowed)
            continue;

        found = p;
        break;
    }

    InputMethod_Duplicate* method = new(std::nothrow) InputMethod_Duplicate;

    if(!method)
        return nullptr;

    method->player_no = found;

    method->Name = "Duplicate P" + std::to_string(found);
    method->Type = this;

    return (InputMethod*)method;
}

/*-----------------------*\
|| OPTIONAL METHODS      ||
\*-----------------------*/

// How many per-type special options are there?
size_t InputMethodType_Duplicate::GetSpecialOptionCount()
{
    return 1;
}

// Methods to manage per-profile options
// It is guaranteed that none of these will be called if
// GetOptionCount() returns 0.
// get a char* describing the option
const char* InputMethodType_Duplicate::GetOptionName(size_t i)
{
    if(i == 0)
    {
        return "PRESS DROP & UP";
    }
    return nullptr;
}
// get a char* describing the current option value
// must be allocated in static or instance memory
// WILL NOT be freed
const char* InputMethodType_Duplicate::GetOptionValue(size_t i)
{
    (void)i;
    return nullptr;
}
// called when A is pressed; allowed to interrupt main game loop
bool InputMethodType_Duplicate::OptionChange(size_t i)
{
    (void)i;
    return false;
}
// called when left is pressed
bool InputMethodType_Duplicate::OptionRotateLeft(size_t i)
{
    (void)i;
    return false;
}
// called when right is pressed
bool InputMethodType_Duplicate::OptionRotateRight(size_t i)
{
    (void)i;
    return false;
}

void InputMethodType_Duplicate::SaveConfig_Custom(IniProcessing* ctl)
{
    (void)ctl;
}

void InputMethodType_Duplicate::LoadConfig_Custom(IniProcessing* ctl)
{
    (void)ctl;
}

} // namespace Controls