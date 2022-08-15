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

#include "globals.h"
#include "game_main.h"

#include "core/render.h"

#include "controls.h"
#include "control/input_wii.h"

#include <Logger/logger.h>

#include <wiiuse/wpad.h>

#define WPAD_STICK_LL  0x2000
#define WPAD_STICK_LR  0x4000
#define WPAD_STICK_LU  0x6000
#define WPAD_STICK_LD  0x8000
#define WPAD_STICK_RL  0xA000
#define WPAD_STICK_RR  0xC000
#define WPAD_STICK_RU  0xE000
// not technically legal.
#define WPAD_STICK_RD  0xD000

#define WPAD_SHAKE     0x1FFF

static constexpr std::array<const uint32_t, 12> wiimote_buttons = {WPAD_BUTTON_2, WPAD_BUTTON_1, WPAD_BUTTON_B, WPAD_BUTTON_A, WPAD_BUTTON_MINUS, WPAD_BUTTON_HOME, WPAD_BUTTON_LEFT, WPAD_BUTTON_RIGHT, WPAD_BUTTON_DOWN, WPAD_BUTTON_UP, WPAD_BUTTON_PLUS, WPAD_SHAKE};
static constexpr std::array<const char*, 12> wiimote_button_names = {"2", "1", "B", "A", "-", "HOME", "D LEFT", "D RIGHT", "D DOWN", "D UP", "+", "SHAKE"};
static constexpr std::array<const char*, 12> wiimote_button_names_side = {"2", "1", "B", "A", "-", "HOME", "DOWN", "UP", "RIGHT", "LEFT", "+", "SHAKE"};

static constexpr std::array<const uint32_t, 6> nunchuck_buttons = {WPAD_STICK_LL, WPAD_STICK_LR, WPAD_STICK_LU, WPAD_STICK_LD, WPAD_NUNCHUK_BUTTON_Z, WPAD_NUNCHUK_BUTTON_C};
static constexpr std::array<const char*, 6> nunchuck_button_names = {"N LEFT", "N RIGHT", "N UP", "N DOWN", "Z", "C"};

static constexpr std::array<const uint32_t, 23> classic_buttons = {WPAD_CLASSIC_BUTTON_UP, WPAD_CLASSIC_BUTTON_LEFT, WPAD_CLASSIC_BUTTON_ZR, WPAD_CLASSIC_BUTTON_X, WPAD_CLASSIC_BUTTON_A, WPAD_CLASSIC_BUTTON_Y,
    WPAD_CLASSIC_BUTTON_B, WPAD_CLASSIC_BUTTON_ZL, WPAD_CLASSIC_BUTTON_FULL_R, WPAD_CLASSIC_BUTTON_PLUS, WPAD_CLASSIC_BUTTON_HOME, WPAD_CLASSIC_BUTTON_MINUS, WPAD_CLASSIC_BUTTON_FULL_L, WPAD_CLASSIC_BUTTON_DOWN,
    WPAD_CLASSIC_BUTTON_RIGHT, WPAD_STICK_LL, WPAD_STICK_LR, WPAD_STICK_LU, WPAD_STICK_LD, WPAD_STICK_RL, WPAD_STICK_RR, WPAD_STICK_RU, WPAD_STICK_RD};
static constexpr std::array<const char*, 23> classic_button_names = {"D UP", "D LEFT", "ZR", "X", "A", "Y", "B", "ZL", "RT", "+", "HOME", "-", "LT", "D DOWN", "D RIGHT",
    "L LEFT", "L RIGHT", "L UP", "L DOWN", "R LEFT", "R RIGHT", "R UP", "R DOWN"};

static const char* s_get_name(uint32_t button, uint8_t expansion)
{
    if(button == Controls::null_but)
        return "NONE";

    // main Wiimote buttons
    if(button < WPAD_STICK_LL)
    {
        auto i = std::find(wiimote_buttons.begin(), wiimote_buttons.end(), button);
        if(i == wiimote_buttons.end())
            return "INVALID";

        int index = i - wiimote_buttons.begin();
        if(expansion)
            return wiimote_button_names[index];
        else
            return wiimote_button_names_side[index];
    }

    // nunchuck buttons
    if(expansion == WPAD_EXP_NUNCHUK)
    {
        auto i = std::find(nunchuck_buttons.begin(), nunchuck_buttons.end(), button);
        if(i == nunchuck_buttons.end())
            return "INVALID";

        int index = i - nunchuck_buttons.begin();
        return nunchuck_button_names[index];
    }

    // classic buttons
    if(expansion == WPAD_EXP_CLASSIC)
    {
        auto i = std::find(classic_buttons.begin(), classic_buttons.end(), button);
        if(i == classic_buttons.end())
            return "INVALID";

        int index = i - classic_buttons.begin();
        return classic_button_names[index];
    }

    return "INVALID";
}

static bool s_get_button(WPADData* data, uint32_t button, uint8_t expansion)
{
    (void)expansion;

    if(button == Controls::null_but)
        return false;

    if(button < WPAD_SHAKE)
    {
        return button & data->btns_h;
    }

    return false;
}

double s_get_button_dbl(WPADData* data, uint32_t button, uint8_t expansion)
{
    if(s_get_button(data, button, expansion))
        return 0.5;
    else
        return 0.0;
}

namespace Controls
{

// helper functions


/*===============================================*\
|| implementation for InputMethod_Wii            ||
\*===============================================*/

InputMethod_Wii::InputMethod_Wii(int chn) : m_chn(chn) {}

InputMethod_Wii::~InputMethod_Wii()
{
    InputMethodType_Wii* t = dynamic_cast<InputMethodType_Wii*>(this->Type);

    if(!t)
        return;

    t->m_canPoll = false;
}

// Update functions that set player controls (and editor controls)
// based on current device input. Return false if device lost.
bool InputMethod_Wii::Update(int player, Controls_t& c, CursorControls_t& m, EditorControls_t& e, HotkeysPressed_t& h)
{
    InputMethodProfile_Wii* p = dynamic_cast<InputMethodProfile_Wii*>(this->Profile);

    if(!p)
        return false;

    WPADData* data = WPAD_Data(m_chn);
    if(!data)
        return false;

    if(data->err)
        return false;

    if(data->exp.type != p->m_expansion)
        return false;

    if(m_rumble_ticks)
    {
        m_rumble_ticks--;
        if(!m_rumble_ticks)
            WPAD_Rumble(m_chn, 0);
    }

    m_battery_status = data->battery_level;

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
            keys = nullptr;
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
                key = null_but;

            int key2 = keys2[i];

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

            if(s_get_button(data, key, p->m_expansion))
                *b = true;
            else if(s_get_button(data, key2, p->m_expansion))
                *b = true;

            if(a == 3 && *b)
                h[i] = player;
        }
    }

    double* const scroll[4] = {&e.ScrollUp, &e.ScrollDown, &e.ScrollLeft, &e.ScrollRight};

    for(int i = 0; i < 4; i++)
    {
        int key = p->m_editor_keys[i];
        int key2 = p->m_editor_keys2[i];

        *scroll[i] += s_get_button_dbl(data, key, p->m_expansion) * 32.0;
        *scroll[i] += s_get_button_dbl(data, key2, p->m_expansion) * 32.0;
    }

    double cursor[4];

    for(int i = 0; i < 4; i++)
    {
        int key = p->m_cursor_keys2[i];
        cursor[i] = s_get_button_dbl(data, key, p->m_expansion) * 32.0;
    }

    // Cursor control (UDLR)
    if(cursor[0] || cursor[1] || cursor[2] || cursor[3])
    {
        if(m.X < 0)
            m.X = ScreenW / 2;

        if(m.Y < 0)
            m.Y = ScreenH / 2;

        m.X += cursor[3];
        m.X -= cursor[2];
        m.Y += cursor[1];
        m.Y -= cursor[0];

        if(m.X < 0)
            m.X = 0;
        else if(m.X >= ScreenW)
            m.X = ScreenW - 1;

        if(m.Y < 0)
            m.Y = 0;
        else if(m.Y >= ScreenH)
            m.Y = ScreenH - 1;

        m.Move = true;
    }

    return true;
}

void InputMethod_Wii::Rumble(int ms, float strength)
{
    UNUSED(strength);

    WPAD_Rumble(m_chn, 1);

    m_rumble_ticks = ms / 15 + 1;
}

StatusInfo InputMethod_Wii::GetStatus()
{
    StatusInfo res;

    res.power_status = StatusInfo::POWER_DISCHARGING;

    res.power_level = m_battery_status / 255.f;

    return res;
}

/*===============================================*\
|| implementation for InputMethodProfile_Wii     ||
\*===============================================*/

// the job of this function is to initialize the class in a consistent state
InputMethodProfile_Wii::InputMethodProfile_Wii(uint8_t expansion) : m_expansion(expansion)
{
    for(size_t i = 0; i < PlayerControls::n_buttons; i++)
    {
        this->m_keys[i] = null_but;
        this->m_keys2[i] = null_but;
    }

    for(size_t i = 0; i < CursorControls::n_buttons; i++)
        this->m_cursor_keys2[i] = null_but;

    for(size_t i = 0; i < EditorControls::n_buttons; i++)
    {
        this->m_editor_keys[i] = null_but;
        this->m_editor_keys2[i] = null_but;
    }

    for(size_t i = 0; i < Hotkeys::n_buttons; i++)
    {
        this->m_hotkeys[i] = null_but;
        this->m_hotkeys2[i] = null_but;
    }

    if(m_expansion == WPAD_EXP_NUNCHUK)
    {
        this->m_keys[PlayerControls::Buttons::Up] = WPAD_STICK_LU;
        this->m_keys[PlayerControls::Buttons::Down] = WPAD_STICK_LD;
        this->m_keys[PlayerControls::Buttons::Left] = WPAD_STICK_LL;
        this->m_keys[PlayerControls::Buttons::Right] = WPAD_STICK_LR;
        this->m_keys[PlayerControls::Buttons::Jump] = WPAD_BUTTON_A;
        this->m_keys[PlayerControls::Buttons::AltJump] = WPAD_NUNCHUK_BUTTON_C;
        this->m_keys[PlayerControls::Buttons::Run] = WPAD_BUTTON_B;
        this->m_keys[PlayerControls::Buttons::AltRun] = WPAD_NUNCHUK_BUTTON_Z;
        this->m_keys[PlayerControls::Buttons::Drop] = WPAD_BUTTON_MINUS;
        this->m_keys[PlayerControls::Buttons::Start] = WPAD_BUTTON_PLUS;

        this->m_keys2[PlayerControls::Buttons::Drop] = WPAD_BUTTON_DOWN;

        this->m_editor_keys[EditorControls::Buttons::ScrollUp] = WPAD_STICK_LU;
        this->m_editor_keys[EditorControls::Buttons::ScrollDown] = WPAD_STICK_LD;
        this->m_editor_keys[EditorControls::Buttons::ScrollLeft] = WPAD_STICK_LL;
        this->m_editor_keys[EditorControls::Buttons::ScrollRight] = WPAD_STICK_LR;
        this->m_editor_keys[EditorControls::Buttons::FastScroll] = WPAD_NUNCHUK_BUTTON_Z;
        this->m_editor_keys[EditorControls::Buttons::ModeErase] = WPAD_NUNCHUK_BUTTON_C;
        this->m_editor_keys[EditorControls::Buttons::PrevSection] = WPAD_BUTTON_MINUS;
        this->m_editor_keys[EditorControls::Buttons::NextSection] = WPAD_BUTTON_PLUS;
        this->m_editor_keys[EditorControls::Buttons::SwitchScreens] = WPAD_BUTTON_1;
        this->m_editor_keys[EditorControls::Buttons::TestPlay] = WPAD_BUTTON_2;

        this->m_cursor_keys2[CursorControls::Buttons::Primary] = WPAD_BUTTON_A;
        this->m_cursor_keys2[CursorControls::Buttons::Secondary] = WPAD_BUTTON_B;
    }
    else if(m_expansion == WPAD_EXP_CLASSIC)
    {
        this->m_keys[PlayerControls::Buttons::Up] = WPAD_STICK_LU;
        this->m_keys[PlayerControls::Buttons::Down] = WPAD_STICK_LD;
        this->m_keys[PlayerControls::Buttons::Left] = WPAD_STICK_LL;
        this->m_keys[PlayerControls::Buttons::Right] = WPAD_STICK_LR;
        this->m_keys[PlayerControls::Buttons::Jump] = WPAD_CLASSIC_BUTTON_B;
        this->m_keys[PlayerControls::Buttons::AltJump] = WPAD_CLASSIC_BUTTON_A;
        this->m_keys[PlayerControls::Buttons::Run] = WPAD_CLASSIC_BUTTON_Y;
        this->m_keys[PlayerControls::Buttons::AltRun] = WPAD_CLASSIC_BUTTON_X;
        this->m_keys[PlayerControls::Buttons::Drop] = WPAD_CLASSIC_BUTTON_MINUS;
        this->m_keys[PlayerControls::Buttons::Start] = WPAD_CLASSIC_BUTTON_PLUS;

        this->m_keys2[PlayerControls::Buttons::Up] = WPAD_CLASSIC_BUTTON_UP;
        this->m_keys2[PlayerControls::Buttons::Down] = WPAD_CLASSIC_BUTTON_DOWN;
        this->m_keys2[PlayerControls::Buttons::Left] = WPAD_CLASSIC_BUTTON_LEFT;
        this->m_keys2[PlayerControls::Buttons::Right] = WPAD_CLASSIC_BUTTON_RIGHT;

        this->m_editor_keys[EditorControls::Buttons::ScrollUp] = WPAD_STICK_LU;
        this->m_editor_keys[EditorControls::Buttons::ScrollDown] = WPAD_STICK_LD;
        this->m_editor_keys[EditorControls::Buttons::ScrollLeft] = WPAD_STICK_LL;
        this->m_editor_keys[EditorControls::Buttons::ScrollRight] = WPAD_STICK_LR;
        this->m_editor_keys[EditorControls::Buttons::FastScroll] = WPAD_CLASSIC_BUTTON_ZL;
        this->m_editor_keys[EditorControls::Buttons::ModeErase] = WPAD_CLASSIC_BUTTON_ZR;
        this->m_editor_keys[EditorControls::Buttons::PrevSection] = WPAD_CLASSIC_BUTTON_B;
        this->m_editor_keys[EditorControls::Buttons::NextSection] = WPAD_CLASSIC_BUTTON_A;
        this->m_editor_keys[EditorControls::Buttons::SwitchScreens] = WPAD_CLASSIC_BUTTON_MINUS;
        this->m_editor_keys[EditorControls::Buttons::TestPlay] = WPAD_CLASSIC_BUTTON_PLUS;

        this->m_cursor_keys2[CursorControls::Buttons::CursorUp] = WPAD_STICK_RU;
        this->m_cursor_keys2[CursorControls::Buttons::CursorDown] = WPAD_STICK_RD;
        this->m_cursor_keys2[CursorControls::Buttons::CursorLeft] = WPAD_STICK_RL;
        this->m_cursor_keys2[CursorControls::Buttons::CursorRight] = WPAD_STICK_RR;
        this->m_cursor_keys2[CursorControls::Buttons::Primary] = WPAD_CLASSIC_BUTTON_FULL_R;
        this->m_cursor_keys2[CursorControls::Buttons::Secondary] = WPAD_CLASSIC_BUTTON_FULL_L;
    }
    else
    {
        this->m_keys[PlayerControls::Buttons::Up] = WPAD_BUTTON_RIGHT;
        this->m_keys[PlayerControls::Buttons::Down] = WPAD_BUTTON_LEFT;
        this->m_keys[PlayerControls::Buttons::Left] = WPAD_BUTTON_UP;
        this->m_keys[PlayerControls::Buttons::Right] = WPAD_BUTTON_DOWN;
        this->m_keys[PlayerControls::Buttons::Jump] = WPAD_BUTTON_2;
        this->m_keys[PlayerControls::Buttons::AltJump] = WPAD_SHAKE;
        this->m_keys[PlayerControls::Buttons::Run] = WPAD_BUTTON_1;
        this->m_keys[PlayerControls::Buttons::AltRun] = WPAD_BUTTON_B;
        this->m_keys[PlayerControls::Buttons::Drop] = WPAD_BUTTON_MINUS;
        this->m_keys[PlayerControls::Buttons::Start] = WPAD_BUTTON_PLUS;

        this->m_editor_keys[EditorControls::Buttons::ScrollUp] = WPAD_BUTTON_UP;
        this->m_editor_keys[EditorControls::Buttons::ScrollDown] = WPAD_BUTTON_DOWN;
        this->m_editor_keys[EditorControls::Buttons::ScrollLeft] = WPAD_BUTTON_LEFT;
        this->m_editor_keys[EditorControls::Buttons::ScrollRight] = WPAD_BUTTON_RIGHT;
        this->m_editor_keys[EditorControls::Buttons::FastScroll] = WPAD_BUTTON_1;
        this->m_editor_keys[EditorControls::Buttons::PrevSection] = WPAD_BUTTON_MINUS;
        this->m_editor_keys[EditorControls::Buttons::NextSection] = WPAD_BUTTON_PLUS;
        this->m_editor_keys[EditorControls::Buttons::SwitchScreens] = WPAD_BUTTON_HOME;
        this->m_editor_keys[EditorControls::Buttons::TestPlay] = WPAD_BUTTON_2;

        this->m_cursor_keys2[CursorControls::Buttons::Primary] = WPAD_BUTTON_A;
        this->m_cursor_keys2[CursorControls::Buttons::Secondary] = WPAD_BUTTON_B;
    }
}

bool InputMethodProfile_Wii::PollPrimaryButton(ControlsClass c, size_t i)
{
    if(c == ControlsClass::Cursor)
        return true;

    // note: m_canPoll is initialized to false
    uint32_t key = null_but;

    for(int chn = 0; chn < 4; chn++)
    {
        WPADData* data = WPAD_Data(chn);

        if(!data)
            continue;

        if(data->err)
            continue;

        if(data->exp.type != m_expansion)
            continue;

        for(uint32_t but : wiimote_buttons)
        {
            if(s_get_button(data, but, m_expansion))
            {
                key = but;
                break;
            }
        }

        if(key != null_but)
            break;
    }

    // if didn't find any key, allow poll in future but return false
    if(key == null_but)
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
        // BAD!
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
            keys2[j] = null_but;
        else if(i != j && keys[j] == key)
        {
            if(keys2[j] != null_but)
            {
                keys[j] = keys2[j];
                keys2[j] = null_but;
            }
            else
                keys[j] = keys[i];
        }
    }

    keys[i] = key;
    return true;
}

bool InputMethodProfile_Wii::PollSecondaryButton(ControlsClass c, size_t i)
{
    // note: m_canPoll is initialized to false
    uint32_t key = null_but;

    for(int chn = 0; chn < 4; chn++)
    {
        WPADData* data = WPAD_Data(chn);

        if(!data)
            continue;

        if(data->err)
            continue;

        if(data->exp.type != m_expansion)
            continue;

        for(uint32_t but : wiimote_buttons)
        {
            if(s_get_button(data, but, m_expansion))
            {
                key = but;
                break;
            }
        }

        if(key != null_but)
            break;
    }

    // if didn't find any key, allow poll in future but return false
    if(key == null_but)
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
        keys = nullptr;
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
        // BAD!
        return true;
    }

    // minor switching algorithm to ensure that every button always has at least one key
    // and no button ever has a non-unique key

    // if the current button's primary key is the new key,
    //     delete its secondary key instead of setting it.
    if(keys && keys[i] == key)
    {
        keys2[i] = null_but;
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
            keys2[j] = null_but;
        }
        else if(keys && i != j && keys[j] == key)
        {
            if(keys2[j] != null_but)
            {
                keys[j] = keys2[j];
                keys2[j] = null_but;
            }
            else if(keys2[i] != null_but)
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
    else if(keys)
        keys[i] = key;

    return true;
}

bool InputMethodProfile_Wii::DeletePrimaryButton(ControlsClass c, size_t i)
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
        return false;
    }
    else if(c == ControlsClass::Editor)
    {
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
        // BAD!
        return false;
    }

    if(keys2[i] != null_but)
    {
        keys[i] = keys2[i];
        keys2[i] = null_but;
        return true;
    }

    if(c == ControlsClass::Player)
        return false;

    if(keys[i] != null_but)
    {
        keys[i] = null_but;
        return true;
    }

    return false;
}

bool InputMethodProfile_Wii::DeleteSecondaryButton(ControlsClass c, size_t i)
{
    uint32_t* keys2;

    if(c == ControlsClass::Player)
        keys2 = this->m_keys2;
    else if(c == ControlsClass::Cursor)
        keys2 = this->m_cursor_keys2;
    else if(c == ControlsClass::Editor)
        keys2 = this->m_editor_keys2;
    else if(c == ControlsClass::Hotkey)
        keys2 = this->m_hotkeys2;
    else
        return false;

    if(keys2[i] != null_but)
    {
        keys2[i] = null_but;
        return true;
    }

    return false;
}

const char* InputMethodProfile_Wii::NamePrimaryButton(ControlsClass c, size_t i)
{
    uint32_t* keys;

    if(c == ControlsClass::Player)
        keys = this->m_keys;
    else if(c == ControlsClass::Cursor)
    {
        if(i < CursorControls::Buttons::Primary)
            return "(IR)";
        else
            return "(NONE)";
    }
    else if(c == ControlsClass::Editor)
        keys = this->m_editor_keys;
    else if(c == ControlsClass::Hotkey)
        keys = this->m_hotkeys;
    else
        return "";

    return s_get_name(keys[i], m_expansion);
}

const char* InputMethodProfile_Wii::NameSecondaryButton(ControlsClass c, size_t i)
{
    uint32_t* keys2;

    if(c == ControlsClass::Player)
        keys2 = this->m_keys2;
    else if(c == ControlsClass::Cursor)
        keys2 = this->m_cursor_keys2;
    else if(c == ControlsClass::Editor)
        keys2 = this->m_editor_keys2;
    else if(c == ControlsClass::Hotkey)
        keys2 = this->m_hotkeys2;
    else
        return "";

    return s_get_name(keys2[i], m_expansion);
}

void InputMethodProfile_Wii::SaveConfig(IniProcessing* ctl)
{
    ctl->setValue("expansion", this->m_expansion);

    char name2[20];

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
            keys = nullptr;
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
            const char* name;

            if(a == 0)
                name = PlayerControls::GetButtonName_INI(i);
            else if(a == 1)
                name = CursorControls::GetButtonName_INI(i);
            else if(a == 2)
                name = EditorControls::GetButtonName_INI(i);
            else
                name = Hotkeys::GetButtonName_INI(i);

            if(keys)
                ctl->setValue(name, keys[i]);

            for(size_t c = 0; c < 20; c++)
            {
                if(c + 2 == 20 || name[c] == '\0')
                {
                    name2[c] = '2';
                    name2[c + 1] = '\0';
                    break;
                }

                name2[c] = name[c];
            }

            ctl->setValue(name2, keys2[i]);
        }
    }
}

void InputMethodProfile_Wii::LoadConfig(IniProcessing* ctl)
{
    ctl->read("expansion", this->m_expansion, WPAD_EXP_NONE);

    char name2[20];

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
            keys = nullptr;
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
            const char* name;

            if(a == 0)
                name = PlayerControls::GetButtonName_INI(i);
            else if(a == 1)
                name = CursorControls::GetButtonName_INI(i);
            else if(a == 2)
                name = EditorControls::GetButtonName_INI(i);
            else
                name = Hotkeys::GetButtonName_INI(i);

            if(keys)
                ctl->read(name, keys[i], keys[i]);

            for(size_t c = 0; c < 20; c++)
            {
                if(c + 2 == 20 || name[c] == '\0')
                {
                    name2[c] = '2';
                    name2[c + 1] = '\0';
                    break;
                }

                name2[c] = name[c];
            }

            ctl->read(name2, keys2[i], keys2[i]);
        }
    }
}

/*===============================================*\
|| implementation for InputMethodType_Wii        ||
\*===============================================*/

InputMethodProfile* InputMethodType_Wii::AllocateProfile() noexcept
{
    return (InputMethodProfile*) new(std::nothrow) InputMethodProfile_Wii(WPAD_EXP_NONE);
}

InputMethodType_Wii::InputMethodType_Wii()
{
    this->Name = "WII";
    this->LegacyName = "wii";
    WPAD_Init();
}

InputMethodType_Wii::~InputMethodType_Wii()
{
    WPAD_Shutdown();
}

bool InputMethodType_Wii::TestProfileType(InputMethodProfile* profile)
{
    return (bool)dynamic_cast<InputMethodProfile_Wii*>(profile);
}

bool InputMethodType_Wii::RumbleSupported()
{
    return false;
}

void InputMethodType_Wii::UpdateControlsPre()
{
    WPAD_ScanPads();
}

void InputMethodType_Wii::UpdateControlsPost() {}

InputMethod* InputMethodType_Wii::Poll(const std::vector<InputMethod*>& active_methods) noexcept
{
    std::array<bool, 4> in_use = {false};

    for(InputMethod* method : active_methods)
    {
        InputMethod_Wii* m = dynamic_cast<InputMethod_Wii*>(method);
        if(!m)
            continue;

        if(m->m_chn >= 0 || m->m_chn < 4)
            in_use[m->m_chn] = true;
    }

    int chn = 0;
    uint8_t expansion = 0;

    for(chn = 0; chn < 4; chn++)
    {
        if(in_use[chn])
            continue;

        WPADData* data = WPAD_Data(chn);

        if(!data)
            continue;

        if(data->err)
            continue;

        uint32_t key = null_but;

        for(uint32_t but : wiimote_buttons)
        {
            if(s_get_button(data, but, data->exp.type))
            {
                key = but;
                break;
            }
        }

        if(key != null_but)
        {
            expansion = data->exp.type;
            break;
        }
    }

    // if didn't find any wiimote allow poll in future but return false
    if(chn == 4)
    {
        this->m_canPoll = true;
        return nullptr;
    }

    // if poll not allowed, return false
    if(!this->m_canPoll)
        return nullptr;

    // we're going to create a new wiimote!
    // reset canPoll for next time
    this->m_canPoll = false;

    printf("got button %d\n", chn);

    InputMethod_Wii* method = new(std::nothrow) InputMethod_Wii(chn);

    if(!method)
        return nullptr;

    printf("allocated\n");

    if(expansion == WPAD_EXP_NUNCHUK)
        method->Name = "Nunchuck";
    else if(expansion == WPAD_EXP_CLASSIC)
        method->Name = "Classic";
    else
        method->Name = "Wiimote";
    method->Type = this;

    printf("it's a %s!\n", method->Name.c_str());

    // now, cleverly find a profile!

    // 1. cleverly look for profile by GUID and player...
    uint16_t new_index = expansion * 256 + 1;

    for(const InputMethod* m : active_methods)
    {
        if(!m)
            break;

        new_index += 1;
    }

    std::unordered_map<uint16_t, InputMethodProfile*>::iterator found
        = this->m_lastProfileByPlayerAndExp.find(new_index);

    // ...then by just expansion index.
    if(found == this->m_lastProfileByPlayerAndExp.end())
        found = this->m_lastProfileByPlayerAndExp.find(expansion * 256);

    if(found != this->m_lastProfileByPlayerAndExp.end())
        method->Profile = found->second;

    // 2. find first profile appropriate to the expansion of the controller
    if(!method->Profile)
    {
        for(InputMethodProfile* p_ : this->m_profiles)
        {
            InputMethodProfile_Wii* p = dynamic_cast<InputMethodProfile_Wii*>(p_);

            if(!p)
                continue;

            if(p->m_expansion == expansion)
            {
                method->Profile = p_;
                break;
            }
        }

        // 3. make appropriate new profile (note that allocs could fail, that will be cleaned up later)
        if(!method->Profile)
        {
            method->Profile = new(std::nothrow) InputMethodProfile_Wii(expansion);
            if(method->Profile)
            {
                method->Profile->Name = method->Name + " 1";
                method->Profile->Type = this;
                this->m_profiles.push_back(method->Profile);
            }
        }
    }

    if(method->Profile)
    {
        printf("bound profile %s!\n", method->Profile->Name.c_str());
        this->m_lastProfileByPlayerAndExp[new_index] = method->Profile;
        this->m_lastProfileByPlayerAndExp[expansion * 256] = method->Profile;
    }

    return (InputMethod*)method;
}

/*-----------------------*\
|| OPTIONAL METHODS      ||
\*-----------------------*/
bool InputMethodType_Wii::ConsumeEvent(const SDL_Event* ev)
{
    UNUSED(ev);
    return false;
}

// optional function allowing developer to associate device information with profile, etc
bool InputMethodType_Wii::SetProfile_Custom(InputMethod* method, int player_no, InputMethodProfile* profile,
        const std::vector<InputMethod*>& active_methods)
{
    UNUSED(active_methods);

    if(!method || !profile || player_no < 0 || player_no >= maxLocalPlayers)
        return false;

    InputMethod_Wii* m = dynamic_cast<InputMethod_Wii*>(method);
    InputMethodProfile_Wii* p = dynamic_cast<InputMethodProfile_Wii*>(profile);

    if(!m || !p || m->m_chn < 0 || m->m_chn >= 4)
        return false;

    WPADData* data = WPAD_Data(m->m_chn);

    if(!data || data->err || data->exp.type != p->m_expansion)
        return false;

    this->m_lastProfileByPlayerAndExp[p->m_expansion * 256 + player_no + 1] = profile;
    this->m_lastProfileByPlayerAndExp[p->m_expansion * 256] = profile;

    m_canPoll = false;
    return true;
}

// How many per-type special options are there?
size_t InputMethodType_Wii::GetOptionCount()
{
    return 2;
}

// Methods to manage per-profile options
// It is guaranteed that none of these will be called if
// GetOptionCount() returns 0.
// get a char* describing the option
const char* InputMethodType_Wii::GetOptionName(size_t i)
{
    if(i == 0)
        return "NEW NUNCHUCK PROFILE";
    else if(i == 1)
        return "NEW CLASSIC PROFILE";

    return nullptr;
}

// get a char* describing the current option value
// must be allocated in static or instance memory
// WILL NOT be freed
const char* InputMethodType_Wii::GetOptionValue(size_t i)
{
    (void)i;
    return nullptr;
}

// called when A is pressed; allowed to interrupt main game loop
bool InputMethodType_Wii::OptionChange(size_t i)
{
    if(i == 0)
    {
        return true;
    }
    else if(i == 1)
    {
        return true;
    }

    return false;
}

void InputMethodType_Wii::SaveConfig_Custom(IniProcessing* ctl)
{
    std::string name = "last-profile-";
    int uuid_begin = name.size();

    // set all default controller profiles
    for(auto it = m_lastProfileByPlayerAndExp.begin(); it != m_lastProfileByPlayerAndExp.end(); ++it)
    {
        std::vector<InputMethodProfile*>::iterator loc = std::find(this->m_profiles.begin(), this->m_profiles.end(), it->second);
        size_t index = loc - this->m_profiles.begin();

        ctl->setValue(name.replace(uuid_begin, std::string::npos, std::to_string(it->first)).c_str(), index);
    }
}

void InputMethodType_Wii::LoadConfig_Custom(IniProcessing* ctl)
{
    // load all default controller profiles
    std::vector<std::string> keys = ctl->allKeys();
    std::string keyNeed = "last-profile-";

    for(std::string& k : keys)
    {
        std::string::size_type r = k.find(keyNeed);

        if(r != std::string::npos && r == 0)
        {
            std::string type_index_str = k.substr(13); // length of "last-profile-"
            uint32_t type_index = strtoul(type_index_str.c_str(), nullptr, 10);

            int prof_index;
            ctl->read(k.c_str(), prof_index, -1);

            if(prof_index >= 0 && prof_index < (int)this->m_profiles.size() && this->m_profiles[prof_index])
            {
                this->m_lastProfileByPlayerAndExp[type_index] = this->m_profiles[prof_index];
                pLogDebug("Set default profile for '%u' to '%s'.", type_index, this->m_profiles[prof_index]->Name.c_str());
            }
        }
    }
}

} // namespace Controls
