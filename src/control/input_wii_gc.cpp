/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2025 Vitaly Novichkov <admin@wohlnet.ru>
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

#include <algorithm>

#include "globals.h"
#include "game_main.h"

#include "core/render.h"

#include "controls.h"
#include "control/input_wii_gc.h"

#include "control/controls_strings.h"
#include "main/menu_main.h"

#include <Logger/logger.h>

#include <ogc/pad.h>

#define PAD_STICK_LL  0x2000
#define PAD_STICK_LR  0x4000
#define PAD_STICK_LU  0x6000
// not technically legal.
#define PAD_STICK_LD  0x7000
#define PAD_STICK_RL  0xA000
#define PAD_STICK_RR  0xC000
#define PAD_STICK_RU  0xE000
// not technically legal.
#define PAD_STICK_RD  0xF000

#define buttonRight Controls::PlayerControls::g_button_name_UI[Controls::PlayerControls::Right]
#define buttonLeft  Controls::PlayerControls::g_button_name_UI[Controls::PlayerControls::Left]
#define buttonUp    Controls::PlayerControls::g_button_name_UI[Controls::PlayerControls::Up]
#define buttonDown  Controls::PlayerControls::g_button_name_UI[Controls::PlayerControls::Down]

static constexpr uint32_t null_key = -1;

static constexpr std::array<const uint32_t, 21> gamecube_buttons = {
    PAD_BUTTON_LEFT,
    PAD_BUTTON_RIGHT,
    PAD_BUTTON_DOWN,
    PAD_BUTTON_UP,
    PAD_TRIGGER_Z,
    PAD_TRIGGER_R,
    PAD_TRIGGER_L,
    PAD_BUTTON_A,
    PAD_BUTTON_B,
    PAD_BUTTON_X,
    PAD_BUTTON_Y,
    PAD_BUTTON_MENU,
    PAD_BUTTON_START,
    PAD_STICK_LL,
    PAD_STICK_LR,
    PAD_STICK_LU,
    PAD_STICK_LD,
    PAD_STICK_RL,
    PAD_STICK_RR,
    PAD_STICK_RU,
    PAD_STICK_RD,
};

static const std::array<const std::string*, 21> gamecube_button_names = {
    &buttonLeft,
    &buttonRight,
    &buttonDown,
    &buttonUp,
    &g_controlsStrings.wiiButtonZ,
    &g_controlsStrings.wiiButtonRT,
    &g_controlsStrings.wiiButtonLT,
    &g_controlsStrings.wiiButtonA,
    &g_controlsStrings.wiiButtonB,
    &g_controlsStrings.wiiButtonX,
    &g_controlsStrings.wiiButtonY,
    &g_controlsStrings.wiiButtonHome,
    &g_controlsStrings.wiiButtonPlus,
    &buttonLeft,
    &buttonRight,
    &buttonUp,
    &buttonDown,
    &buttonLeft,
    &buttonRight,
    &buttonUp,
    &buttonDown,
};

static std::string s_buffer;

static const char* s_get_name(uint32_t key)
{
    if(key == null_key)
        return g_mainMenu.caseNone.c_str();

    auto it = std::find(gamecube_buttons.begin(), gamecube_buttons.end(), key);
    if(it == gamecube_buttons.end())
        return g_controlsStrings.sharedCaseInvalid.c_str();

    int i = it - gamecube_buttons.begin();

    if(key <= PAD_BUTTON_UP)
        s_buffer = g_controlsStrings.wiiDpad;
    else if(key >= PAD_STICK_LL && key < PAD_STICK_RL)
        s_buffer = g_controlsStrings.wiiLStick;
    else if(key >= PAD_STICK_RL)
        s_buffer = g_controlsStrings.wiiRStick;
    else
        s_buffer.clear();

    if(!s_buffer.empty())
        s_buffer += ' ';

    s_buffer += *gamecube_button_names[i];
    return s_buffer.c_str();
}

namespace Controls
{

/*====================================================*\
|| implementation for InputMethod_GameCube            ||
\*====================================================*/

static inline double s_s8_to_dbl(int8_t in)
{
    if(in <= 32)
        return 0;

    return SDL_min((in - 32) / 64.0, 1.0);
}

static inline double s_get_thumb_dbl(const PADStatus* data, uint32_t button)
{
    switch(button)
    {
    case(PAD_STICK_LL):
        return s_s8_to_dbl(-data->stickX);
    case(PAD_STICK_LR):
        return s_s8_to_dbl( data->stickX);
    case(PAD_STICK_LU):
        return s_s8_to_dbl( data->stickY);
    case(PAD_STICK_LD):
        return s_s8_to_dbl(-data->stickY);
    case(PAD_STICK_RL):
        return s_s8_to_dbl(-data->substickX);
    case(PAD_STICK_RR):
        return s_s8_to_dbl( data->substickX);
    case(PAD_STICK_RU):
        return s_s8_to_dbl( data->substickY);
    case(PAD_STICK_RD):
        return s_s8_to_dbl(-data->substickY);
    default:
        return 0;
    }
}

static bool s_get_button(const PADStatus* data, uint32_t button)
{
    if(button == null_key)
        return false;

    if(button < PAD_STICK_LL)
        return (button & data->button);

    // thumbstick button
    if(button <= PAD_STICK_RD)
        return s_get_thumb_dbl(data, button) > 0.25;

    return false;
}

static double s_get_button_dbl(const PADStatus* data, uint32_t button)
{
    if(button >= PAD_STICK_LL && button <= PAD_STICK_RD)
        return s_get_thumb_dbl(data, button);

    if(s_get_button(data, button))
        return 0.5;
    else
        return 0.0;
}

/*====================================================*\
|| implementation for InputMethod_GameCube            ||
\*====================================================*/

InputMethod_GameCube::~InputMethod_GameCube()
{
    InputMethodType_GameCube* t = dynamic_cast<InputMethodType_GameCube*>(this->Type);

    if(!t)
        return;

    t->m_canPoll = false;
}

// Update functions that set player controls (and editor controls)
// based on current device input. Return false if device lost.
bool InputMethod_GameCube::Update(int player, Controls_t& c, CursorControls_t& m, EditorControls_t& e, HotkeysPressed_t& h)
{
    InputMethodProfile_GameCube* p = dynamic_cast<InputMethodProfile_GameCube*>(this->Profile);
    InputMethodType_GameCube* t = dynamic_cast<InputMethodType_GameCube*>(this->Type);

    if(!p || !t || m_chn < 0 || m_chn >= 4)
        return false;

    const PADStatus* data = &t->m_status[m_chn];
    if(!data)
        return false;

    if(data->err != PAD_ERR_NONE)
        return false;

    if(m_rumble_ticks)
    {
        m_rumble_ticks--;
        if(!m_rumble_ticks)
            PAD_ControlMotor(m_chn, PAD_MOTOR_STOP);
    }

    for(int a = 0; a < 4; a++)
    {
        uint32_t* keys;
        uint32_t* keys2;
        size_t key_start;
        size_t key_max;
        bool activate = false;

        if(a == 0)
        {
            keys = p->m_keys;
            keys2 = p->m_keys2;
            key_start = 0;
            key_max = PlayerControls::n_buttons;
        }
        else if(a == 1)
        {
            keys = p->m_cursor_keys;
            keys2 = p->m_cursor_keys2;
            key_start = 4;
            key_max = CursorControls::n_buttons;
        }
        else if(a == 2)
        {
            keys = p->m_editor_keys;
            keys2 = p->m_editor_keys2;
            key_start = 4;
            key_max = EditorControls::n_buttons;

            if(p->m_simple_editor && LevelEditor)
                continue;
        }
        else
        {
            keys = p->m_hotkeys;
            keys2 = p->m_hotkeys2;
            key_start = 0;
            key_max = Hotkeys::n_buttons;
        }

        for(size_t i = key_start; i < key_max; i++)
        {
            uint32_t key;

            if(keys)
                key = keys[i];
            else
                key = null_key;

            uint32_t key2 = keys2[i];

            bool* b;

            if(a == 0)
            {
                b = &PlayerControls::GetButton(c, i);
                *b = false;
            }
            else if(a == 1)
            {
                b = &CursorControls::GetButton(m, i);
            }
            else if(a == 2)
            {
                b = &EditorControls::GetButton(e, i);
            }
            else
            {
                b = &activate;
                *b = false;
            }

            if(s_get_button(data, key))
                *b = true;
            else if(s_get_button(data, key2))
                *b = true;

            if(a == 3 && *b)
                h[i] = player;
        }
    }

    // simple editor controls
    if(p->m_simple_editor && LevelEditor)
    {
        CursorControls::GetButton(m, CursorControls::Buttons::Primary) = PlayerControls::GetButton(c, PlayerControls::Buttons::Jump);
        CursorControls::GetButton(m, CursorControls::Buttons::Secondary) = false;
        CursorControls::GetButton(m, CursorControls::Buttons::Tertiary) = false;

        EditorControls::GetButton(e, EditorControls::FastScroll) = false;
        EditorControls::GetButton(e, EditorControls::PrevSection) = false;
        EditorControls::GetButton(e, EditorControls::NextSection) = false;

        EditorControls::GetButton(e, EditorControls::ModeSelect) |= PlayerControls::GetButton(c, PlayerControls::Buttons::Run);
        EditorControls::GetButton(e, EditorControls::ModeErase) |= PlayerControls::GetButton(c, PlayerControls::Buttons::AltJump);
        EditorControls::GetButton(e, EditorControls::SwitchScreens) |= PlayerControls::GetButton(c, PlayerControls::Buttons::Drop);
        EditorControls::GetButton(e, EditorControls::TestPlay) |= PlayerControls::GetButton(c, PlayerControls::Buttons::Start);
    }

    // analogue controls
    double cursor[4] = {0, 0, 0, 0};
    double scroll[4] = {0, 0, 0, 0};

    for(int i = 0; i < 4; i++)
    {
        if(p->m_simple_editor && LevelEditor)
        {
            cursor[i] += s_get_button_dbl(data, p->m_keys[i]);
            cursor[i] += s_get_button_dbl(data, p->m_keys2[i]);
        }
        else
        {
            scroll[i] += s_get_button_dbl(data, p->m_editor_keys[i]);
            scroll[i] += s_get_button_dbl(data, p->m_editor_keys2[i]);
            cursor[i] += s_get_button_dbl(data, p->m_cursor_keys[i]);
            cursor[i] += s_get_button_dbl(data, p->m_cursor_keys2[i]);
        }
    }

    // Scroll control (UDLR)
    double* const scroll_dest[4] = {&e.ScrollUp, &e.ScrollDown, &e.ScrollLeft, &e.ScrollRight};
    for(int i = 0; i < 4; i++)
        *scroll_dest[i] += scroll[i] * 10.;

    // Cursor control (UDLR)
    if(cursor[0] || cursor[1] || cursor[2] || cursor[3])
    {
        bool edge_scroll = LevelEditor && !MagicHand;

        if(m.X < 0)
            m.X = XRender::TargetW / 2;

        if(m.Y < 0)
            m.Y = XRender::TargetH / 2;

        m.X += (cursor[3] - cursor[2]) * 16.;
        m.Y += (cursor[1] - cursor[0]) * 16.;

        if(m.X < 0)
        {
            if(edge_scroll)
                e.ScrollLeft += -m.X;

            m.X = 0;
        }
        else if(m.X >= XRender::TargetW)
        {
            if(edge_scroll)
                e.ScrollRight += m.X - (XRender::TargetW - 1);

            m.X = XRender::TargetW - 1;
        }

        if(m.Y < 0)
        {
            if(edge_scroll)
                e.ScrollUp += -m.Y;

            m.Y = 0;
        }
        else if(m.Y >= XRender::TargetH)
        {
            if(edge_scroll)
                e.ScrollDown += m.Y - (XRender::TargetH - 1);

            m.Y = XRender::TargetH - 1;
        }

        m.Move = true;
    }

    return true;
}

void InputMethod_GameCube::Rumble(int ms, float strength)
{
    UNUSED(strength);

    PAD_ControlMotor(m_chn, PAD_MOTOR_RUMBLE);

    m_rumble_ticks = ms / 15 + 1;
}

StatusInfo InputMethod_GameCube::GetStatus()
{
    StatusInfo res;
    res.power_status = StatusInfo::POWER_WIRED;

    return res;
}

/*====================================================*\
|| implementation for InputMethodProfile_GameCube     ||
\*====================================================*/

// the job of this function is to initialize the class in a consistent state
InputMethodProfile_GameCube::InputMethodProfile_GameCube()
{
    this->m_simple_editor = true;

    this->m_keys[PlayerControls::Buttons::Up] = PAD_BUTTON_UP;
    this->m_keys[PlayerControls::Buttons::Down] = PAD_BUTTON_DOWN;
    this->m_keys[PlayerControls::Buttons::Left] = PAD_BUTTON_LEFT;
    this->m_keys[PlayerControls::Buttons::Right] = PAD_BUTTON_RIGHT;

    this->m_keys[PlayerControls::Buttons::Jump] = PAD_BUTTON_A;
    this->m_keys[PlayerControls::Buttons::AltJump] = PAD_BUTTON_X;
    this->m_keys[PlayerControls::Buttons::Run] = PAD_BUTTON_B;
    this->m_keys[PlayerControls::Buttons::AltRun] = PAD_BUTTON_Y;

    this->m_keys[PlayerControls::Buttons::Drop] = PAD_TRIGGER_Z;
    this->m_keys[PlayerControls::Buttons::Start] = PAD_BUTTON_START;

    // clear all of them, then fill in some of them
    for(size_t i = 0; i < PlayerControls::n_buttons; i++)
        this->m_keys2[i] = null_key;

    this->m_keys2[PlayerControls::Buttons::Up] = PAD_STICK_LU;
    this->m_keys2[PlayerControls::Buttons::Down] = PAD_STICK_LD;
    this->m_keys2[PlayerControls::Buttons::Left] = PAD_STICK_LL;
    this->m_keys2[PlayerControls::Buttons::Right] = PAD_STICK_LR;

    // clear all of the non-standard controls, then fill in some of them
    for(size_t i = 0; i < CursorControls::n_buttons; i++)
    {
        this->m_cursor_keys[i] = null_key;
        this->m_cursor_keys2[i] = null_key;
    }

    for(size_t i = 0; i < EditorControls::n_buttons; i++)
    {
        this->m_editor_keys[i] = null_key;
        this->m_editor_keys2[i] = null_key;
    }

    for(size_t i = 0; i < Hotkeys::n_buttons; i++)
    {
        this->m_hotkeys[i] = null_key;
        this->m_hotkeys2[i] = null_key;
    }

    this->m_cursor_keys[CursorControls::Buttons::CursorUp] = PAD_STICK_RU;
    this->m_cursor_keys[CursorControls::Buttons::CursorDown] = PAD_STICK_RD;
    this->m_cursor_keys[CursorControls::Buttons::CursorLeft] = PAD_STICK_RL;
    this->m_cursor_keys[CursorControls::Buttons::CursorRight] = PAD_STICK_RR;
}

bool InputMethodProfile_GameCube::PollPrimaryButton(ControlsClass c, size_t i)
{
    // note: m_canPoll is initialized to false
    InputMethodType_GameCube* t = dynamic_cast<InputMethodType_GameCube*>(this->Type);

    if(!t)
        return true;

    if(this->m_simple_editor && c == ControlsClass::Editor)
        return true;

    uint32_t key = t->PollKeyAll();

    // if didn't find any key, allow poll in future but return false
    if(key == null_key)
    {
        this->m_canPoll = true;
        return false;
    }

    // if poll not allowed, return false
    if(!this->m_canPoll)
        return false;

    // we will assign the key!
    // reset canPoll for next time
    this->m_canPoll = false;

    // resolve the particular primary and secondary key arrays
    uint32_t* keys;
    uint32_t* keys2;
    size_t key_max;

    if(c == ControlsClass::Player)
    {
        keys = this->m_keys;
        keys2 = this->m_keys2;
        key_max = PlayerControls::n_buttons;
    }
    else if(c == ControlsClass::Cursor)
    {
        keys = this->m_cursor_keys;
        keys2 = this->m_cursor_keys2;
        key_max = CursorControls::n_buttons;
    }
    else if(c == ControlsClass::Editor)
    {
        keys = this->m_editor_keys;
        keys2 = this->m_editor_keys2;
        key_max = EditorControls::n_buttons;
    }
    else if(c == ControlsClass::Hotkey)
    {
        keys = this->m_hotkeys;
        keys2 = this->m_hotkeys2;
        key_max = Hotkeys::n_buttons;
    }
    else
    {
        D_pLogWarning("Polling Joystick primary button with disallowed controls class %d\n", (int)c);
        return true;
    }

    // minor switching algorithm to ensure that every button always has at least one key
    // and no button ever has a non-unique key
    // if a button's secondary key (including the current one) is the new key, delete it.
    // if a button's primary key (excluding the current one) is the new key,
    //     and it has a secondary key, overwrite it with the secondary key.
    //     otherwise, replace it with the button the player is replacing.
    for(size_t j = 0; j < key_max; j++)
    {
        if(keys2[j] == key)
        {
            keys2[j] = null_key;
        }
        else if(i != j && keys[j] == key)
        {
            if(keys2[j] == null_key)
            {
                keys[j] = keys2[j];
                keys2[j] = null_key;
            }
            else
            {
                keys[j] = keys[i];
            }
        }
    }

    keys[i] = key;
    return true;
}

bool InputMethodProfile_GameCube::PollSecondaryButton(ControlsClass c, size_t i)
{
    // note: m_canPoll is initialized to false
    InputMethodType_GameCube* t = dynamic_cast<InputMethodType_GameCube*>(this->Type);

    if(!t)
        return true;

    if(this->m_simple_editor && c == ControlsClass::Editor)
        return true;

    uint32_t key = t->PollKeyAll();

    // if didn't find any key, allow poll in future but return false
    if(key == null_key)
    {
        this->m_canPoll = true;
        return false;
    }

    // if poll not allowed, return false
    if(!this->m_canPoll)
        return false;

    // we will assign the key!
    // reset canPoll for next time
    m_canPoll = false;

    // resolve the particular primary and secondary key arrays
    uint32_t* keys;
    uint32_t* keys2;
    size_t key_max;

    if(c == ControlsClass::Player)
    {
        keys = this->m_keys;
        keys2 = this->m_keys2;
        key_max = PlayerControls::n_buttons;
    }
    else if(c == ControlsClass::Cursor)
    {
        keys = this->m_cursor_keys;
        keys2 = this->m_cursor_keys2;
        key_max = CursorControls::n_buttons;
    }
    else if(c == ControlsClass::Editor)
    {
        keys = this->m_editor_keys;
        keys2 = this->m_editor_keys2;
        key_max = EditorControls::n_buttons;
    }
    else if(c == ControlsClass::Hotkey)
    {
        keys = this->m_hotkeys;
        keys2 = this->m_hotkeys2;
        key_max = Hotkeys::n_buttons;
    }
    else
    {
        D_pLogWarning("Polling Joystick secondary button with disallowed controls class %d\n", (int)c);
        return true;
    }

    // minor switching algorithm to ensure that every button always has at least one key
    // and no button ever has a non-unique key

    // if the current button's primary key is the new key,
    //     delete its secondary key instead of setting it.
    if(keys[i] == key)
    {
        keys2[i] = null_key;
        return true;
    }

    // if another button's secondary key is the new key, delete it.
    // if another button's primary key is the new key,
    //     and it has a secondary key, overwrite it with the secondary key.
    //     otherwise, if this button's secondary key is defined, overwrite the other's with this.
    //     if all else fails, overwrite the other button's with this button's PRIMARY key and assign
    //         this button's PRIMARY key instead

    bool can_do_secondary = true;

    for(size_t j = 0; j < key_max; j++)
    {
        if(i != j && keys2[j] == key)
        {
            keys2[j] = null_key;
        }
        else if(i != j && keys[j] == key)
        {
            if(keys2[j] != null_key)
            {
                keys[j] = keys2[j];
                keys2[j] = null_key;
            }
            else if(keys2[i] != null_key)
            {
                keys[j] = keys2[i];
            }
            else
            {
                keys[j] = keys[i];
                can_do_secondary = false;
            }
        }
    }

    if(can_do_secondary)
        keys2[i] = key;
    else
        keys[i] = key;

    return true;
}

bool InputMethodProfile_GameCube::DeletePrimaryButton(ControlsClass c, size_t i)
{
    // resolve the particular primary and secondary key arrays
    uint32_t* keys;
    uint32_t* keys2;

    if(c == ControlsClass::Player)
    {
        keys = this->m_keys;
        keys2 = this->m_keys2;
    }
    else if(c == ControlsClass::Cursor)
    {
        keys = this->m_cursor_keys;
        keys2 = this->m_cursor_keys2;
    }
    else if(c == ControlsClass::Editor)
    {
        if(this->m_simple_editor)
            return false;

        keys = this->m_editor_keys;
        keys2 = this->m_editor_keys2;
    }
    else if(c == ControlsClass::Hotkey)
    {
        keys = this->m_hotkeys;
        keys2 = this->m_hotkeys2;
    }
    else
    {
        D_pLogWarning("Attempted to delete Joystick primary button with disallowed controls class %d\n", (int)c);
        return false;
    }

    if(keys2[i] != null_key)
    {
        keys[i] = keys2[i];
        keys2[i] = null_key;
        return true;
    }

    if(c == ControlsClass::Player)
        return false;

    if(keys[i] != null_key)
    {
        keys[i] = null_key;
        return true;
    }

    return false;
}

bool InputMethodProfile_GameCube::DeleteSecondaryButton(ControlsClass c, size_t i)
{
    // resolve the particular primary and secondary key arrays
    uint32_t* keys2;

    if(c == ControlsClass::Player)
        keys2 = this->m_keys2;
    else if(c == ControlsClass::Cursor)
        keys2 = this->m_cursor_keys2;
    else if(c == ControlsClass::Editor)
    {
        if(this->m_simple_editor)
            return false;

        keys2 = this->m_editor_keys2;
    }
    else if(c == ControlsClass::Hotkey)
        keys2 = this->m_hotkeys2;
    else
        return false; // BAD!

    if(keys2[i] != null_key)
    {
        keys2[i] = null_key;
        return true;
    }

    return false;
}

const char* InputMethodProfile_GameCube::NamePrimaryButton(ControlsClass c, size_t i)
{
    uint32_t* keys;

    if(c == ControlsClass::Player)
        keys = this->m_keys;
    else if(c == ControlsClass::Cursor)
        keys = this->m_cursor_keys;
    else if(c == ControlsClass::Editor && this->m_simple_editor)
    {
        keys = this->m_keys;

        if(i == EditorControls::Buttons::ModeSelect)
            i = PlayerControls::Buttons::Run;
        else if(i == EditorControls::Buttons::ModeErase)
            i = PlayerControls::Buttons::AltJump;
        else if(i == EditorControls::Buttons::SwitchScreens)
            i = PlayerControls::Buttons::Drop;
        else if(i == EditorControls::Buttons::TestPlay)
            i = PlayerControls::Buttons::Start;
        else
            return g_mainMenu.caseNone.c_str();
    }
    else if(c == ControlsClass::Editor)
        keys = this->m_editor_keys;
    else if(c == ControlsClass::Hotkey)
        keys = this->m_hotkeys;
    else
        return "";

    return s_get_name(keys[i]);
}

const char* InputMethodProfile_GameCube::NameSecondaryButton(ControlsClass c, size_t i)
{
    uint32_t* keys2;

    if(c == ControlsClass::Player)
        keys2 = this->m_keys2;
    else if(c == ControlsClass::Cursor)
        keys2 = this->m_cursor_keys2;
    else if(c == ControlsClass::Editor && this->m_simple_editor)
    {
        keys2 = this->m_keys2;

        if(i == EditorControls::Buttons::ModeSelect)
            i = PlayerControls::Buttons::Run;
        else if(i == EditorControls::Buttons::ModeErase)
            i = PlayerControls::Buttons::AltJump;
        else if(i == EditorControls::Buttons::SwitchScreens)
            i = PlayerControls::Buttons::Drop;
        else if(i == EditorControls::Buttons::TestPlay)
            i = PlayerControls::Buttons::Start;
        else
            return "";
    }
    else if(c == ControlsClass::Editor)
        keys2 = this->m_editor_keys2;
    else if(c == ControlsClass::Hotkey)
        keys2 = this->m_hotkeys2;
    else
        return "";

    return s_get_name(keys2[i]);
}

void InputMethodProfile_GameCube::SaveConfig(IniProcessing* ctl)
{
    std::string name;

    for(int a = 0; a < 4; a++)
    {
        uint32_t* keys;
        uint32_t* keys2;
        size_t key_max;

        if(a == 0)
        {
            keys = this->m_keys;
            keys2 = this->m_keys2;
            key_max = PlayerControls::n_buttons;
        }
        else if(a == 1)
        {
            keys = this->m_cursor_keys;
            keys2 = this->m_cursor_keys2;
            key_max = CursorControls::n_buttons;
        }
        else if(a == 2)
        {
            keys = this->m_editor_keys;
            keys2 = this->m_editor_keys2;
            key_max = EditorControls::n_buttons;
        }
        else
        {
            keys = this->m_hotkeys;
            keys2 = this->m_hotkeys2;
            key_max = Hotkeys::n_buttons;
        }

        for(size_t i = 0; i < key_max; i++)
        {
            if(a == 0)
                name = PlayerControls::GetButtonName_INI(i);
            else if(a == 1)
                name = CursorControls::GetButtonName_INI(i);
            else if(a == 2)
                name = EditorControls::GetButtonName_INI(i);
            else
                name = Hotkeys::GetButtonName_INI(i);

            ctl->setValue(name.c_str(), keys[i]);

            name += '2';

            ctl->setValue(name.c_str(), keys2[i]);
        }
    }

    ctl->setValue("simple-editor", this->m_simple_editor);
}

void InputMethodProfile_GameCube::LoadConfig(IniProcessing* ctl)
{
    std::string name;

    for(int a = 0; a < 4; a++)
    {
        uint32_t* keys;
        uint32_t* keys2;
        size_t key_max;

        if(a == 0)
        {
            keys = this->m_keys;
            keys2 = this->m_keys2;
            key_max = PlayerControls::n_buttons;
        }
        else if(a == 1)
        {
            keys = this->m_cursor_keys;
            keys2 = this->m_cursor_keys2;
            key_max = CursorControls::n_buttons;
        }
        else if(a == 2)
        {
            keys = this->m_editor_keys;
            keys2 = this->m_editor_keys2;
            key_max = EditorControls::n_buttons;
        }
        else
        {
            keys = this->m_hotkeys;
            keys2 = this->m_hotkeys2;
            key_max = Hotkeys::n_buttons;
        }

        for(size_t i = 0; i < key_max; i++)
        {
            if(a == 0)
                name = PlayerControls::GetButtonName_INI(i);
            else if(a == 1)
                name = CursorControls::GetButtonName_INI(i);
            else if(a == 2)
                name = EditorControls::GetButtonName_INI(i);
            else
                name = Hotkeys::GetButtonName_INI(i);

            ctl->read(name.c_str(), keys[i], keys[i]);

            name += '2';

            ctl->read(name.c_str(), keys2[i], keys2[i]);
        }
    }

    ctl->read("simple-editor", this->m_simple_editor, true);
}

size_t InputMethodProfile_GameCube::GetOptionCount_Custom()
{
    return 1;
}

const char *InputMethodProfile_GameCube::GetOptionName_Custom(size_t i)
{
    UNUSED(i);

    return g_controlsStrings.joystickSimpleEditor.c_str();
}

const char *InputMethodProfile_GameCube::GetOptionValue_Custom(size_t i)
{
    UNUSED(i);

    if(this->m_simple_editor)
        return g_mainMenu.wordOn.c_str();
    else
        return g_mainMenu.wordOff.c_str();
}

bool InputMethodProfile_GameCube::OptionChange_Custom(size_t i)
{
    return this->OptionRotateRight_Custom(i);
}

bool InputMethodProfile_GameCube::OptionRotateLeft_Custom(size_t i)
{
    return this->OptionRotateRight_Custom(i);
}

bool InputMethodProfile_GameCube::OptionRotateRight_Custom(size_t i)
{
    UNUSED(i);

    this->m_simple_editor = !this->m_simple_editor;

    return true;
}

/*====================================================*\
|| implementation for InputMethodType_GameCube        ||
\*====================================================*/

InputMethodProfile* InputMethodType_GameCube::AllocateProfile() noexcept
{
    return (InputMethodProfile*) new(std::nothrow) InputMethodProfile_GameCube;
}

InputMethodType_GameCube::InputMethodType_GameCube()
{
    this->Name = "GameCube";
    PAD_Init();
}

InputMethodType_GameCube::~InputMethodType_GameCube()
{
}

const std::string& InputMethodType_GameCube::LocalName() const
{
    return g_controlsStrings.wiiTypeGamecube;
}

bool InputMethodType_GameCube::TestProfileType(InputMethodProfile* profile)
{
    return (bool)dynamic_cast<InputMethodProfile_GameCube*>(profile);
}

bool InputMethodType_GameCube::RumbleSupported()
{
    return true;
}

bool InputMethodType_GameCube::PowerStatusSupported()
{
    return false;
}

void InputMethodType_GameCube::UpdateControlsPre()
{
    PAD_Read(m_status_raw);

    for(int chn = 0; chn < 4; chn++)
    {
        if(m_status_raw[chn].err == PAD_ERR_NONE || m_status_raw[chn].err == PAD_ERR_NO_CONTROLLER)
            m_status[chn] = m_status_raw[chn];
    }

    for(int chn = 0; chn < 4; chn++)
    {
        if(m_status[chn].err != PAD_ERR_NONE)
            m_chnConnected[chn] = 0;
        else if(m_chnConnected[chn] < 8)
            m_chnConnected[chn]++;
    }
}

void InputMethodType_GameCube::UpdateControlsPost() {}

InputMethod* InputMethodType_GameCube::Poll(const std::vector<InputMethod*>& active_methods) noexcept
{
    std::array<bool, 4> in_use = {false};

    for(InputMethod* method : active_methods)
    {
        InputMethod_GameCube* m = dynamic_cast<InputMethod_GameCube*>(method);
        if(!m)
            continue;

        if(m->m_chn >= 0 || m->m_chn < 4)
            in_use[m->m_chn] = true;
    }

    int chn = 0;

    for(chn = 0; chn < 4; chn++)
    {
        if(in_use[chn])
            continue;

        const PADStatus* data = &m_status[chn];

        if(data->err != PAD_ERR_NONE)
            continue;

        // newly connected controller
        bool newly_connected = (m_chnConnected[chn] < 8);

        uint32_t key = null_key;

        for(uint32_t but : gamecube_buttons)
        {
            if(s_get_button(data, but))
            {
                key = but;
                break;
            }
        }

        if(key != null_key || newly_connected)
            break;
    }

    // if didn't find any controller allow poll in future but return false
    if(chn == 4)
    {
        this->m_canPoll = true;
        return nullptr;
    }

    // if poll not allowed, return false
    if(!this->m_canPoll)
        return nullptr;

    // we're going to create a new gamecube controller!
    // reset canPoll for next time
    this->m_canPoll = false;

    InputMethod_GameCube* method = new(std::nothrow) InputMethod_GameCube(chn);

    if(!method)
        return nullptr;

    method->Name = g_controlsStrings.wiiTypeGamecube;

    method->Name += " ";
    method->Name += std::to_string(chn + 1);
    method->Type = this;

    return (InputMethod*)method;
}

uint32_t InputMethodType_GameCube::PollKeyAll() const
{
    for(int chn = 0; chn < 4; chn++)
    {
        const PADStatus* data = &m_status[chn];

        if(data->err != PAD_ERR_NONE)
            continue;

        for(uint32_t but : gamecube_buttons)
        {
            if(s_get_button(data, but))
                return but;
        }
    }

    return null_key;
}

} // namespace Controls
