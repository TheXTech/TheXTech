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

#include <nds.h>

#include "globals.h"
#include "game_main.h"

#include "core/render.h"
#include "editor/new_editor.h"

#include "controls.h"
#include "control/input_16m.h"

const char* KEYNAMES[32] = {
 "A", "B", "SELECT", "START",
 "RIGHT", "LEFT", "UP", "DOWN",
 "R", "L", "X", "Y",
 "12", "13", "14", "15",
 "16", "17", "18", "19",
 "TOUCH", "21", "22", "23",
 "24", "25", "26", "27",
 "28", "29", "30", "31"
};

enum KEYID {
 KEYID_A=0,
 KEYID_B=1,
 KEYID_SELECT=2,
 KEYID_START=3,
 KEYID_RIGHT=4,
 KEYID_LEFT=5,
 KEYID_UP=6,
 KEYID_DOWN=7,
 KEYID_R=8,
 KEYID_L=9,
 KEYID_X=10,
 KEYID_Y=11,

 KEYID_TOUCH=20,
};

namespace Controls
{

// helper functions


/*===============================================*\
|| implementation for InputMethod_16M            ||
\*===============================================*/

InputMethod_16M::~InputMethod_16M()
{
    InputMethodType_16M* t = dynamic_cast<InputMethodType_16M*>(this->Type);

    if(!t)
        return;

    t->m_numPlayers --;
}

// Update functions that set player controls (and editor controls)
// based on current device input. Return false if device lost.
bool InputMethod_16M::Update(int player, Controls_t& c, CursorControls_t& m, EditorControls_t& e, HotkeysPressed_t& h)
{
    InputMethodType_16M* k = dynamic_cast<InputMethodType_16M*>(this->Type);
    InputMethodProfile_16M* p = dynamic_cast<InputMethodProfile_16M*>(this->Profile);

    if(!k || !p)
        return false;

    uint32_t keys_held = keysCurrent();

    for(int a = 0; a < 4; a++)
    {
        int* keys;
        int* keys2;
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
            int key;

            if(keys)
                key = keys[i];
            else
                key = null_key;

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

            if(key != null_key && BIT(key) & keys_held)
                *b = true;
            else if(key2 != null_key && BIT(key2) & keys_held)
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

        if(key != null_key && BIT(key) & keys_held)
            *scroll[i] += 10.0;
        else if(key2 != null_key && BIT(key2) & keys_held)
            *scroll[i] += 10.0;
    }

    bool cursor[4];

    for(int i = 0; i < 4; i++)
    {
        int key = p->m_cursor_keys2[i];
        cursor[i] = (key != null_key && BIT(key) & keys_held);
    }

    // Cursor control (UDLR)
    if(cursor[0] || cursor[1] || cursor[2] || cursor[3])
    {
        if(m.X < 0)
            m.X = ScreenW / 2;

        if(m.Y < 0)
            m.Y = ScreenH / 2;

        if(cursor[3])
            m.X += 16.0;

        if(cursor[2])
            m.X -= 16.0;

        if(cursor[1])
            m.Y += 16.0;

        if(cursor[0])
            m.Y -= 16.0;

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

    // one may uncomment this to quickly simulate controller disconnection for debugging purposes
    // if(c.Up && c.Down)
    //     return false;

    return true;
}

void InputMethod_16M::Rumble(int ms, float strength)
{
    UNUSED(ms);
    UNUSED(strength);
}

StatusInfo InputMethod_16M::GetStatus()
{
    StatusInfo res;

    return res;
}

/*===============================================*\
|| implementation for InputMethodProfile_16M     ||
\*===============================================*/

// the job of this function is to initialize the class in a consistent state
InputMethodProfile_16M::InputMethodProfile_16M()
{
    for(size_t i = 0; i < PlayerControls::n_buttons; i++)
    {
        this->m_keys[i] = null_key;
        this->m_keys2[i] = null_key;
    }

    for(size_t i = 0; i < CursorControls::n_buttons; i++)
        this->m_cursor_keys2[i] = null_key;

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

    this->m_keys[PlayerControls::Buttons::Up] = KEYID_UP;
    this->m_keys[PlayerControls::Buttons::Down] = KEYID_DOWN;
    this->m_keys[PlayerControls::Buttons::Left] = KEYID_LEFT;
    this->m_keys[PlayerControls::Buttons::Right] = KEYID_RIGHT;
    this->m_keys[PlayerControls::Buttons::Jump] = KEYID_B;
    this->m_keys[PlayerControls::Buttons::AltJump] = KEYID_A;
    this->m_keys[PlayerControls::Buttons::Run] = KEYID_Y;
    this->m_keys[PlayerControls::Buttons::AltRun] = KEYID_X;
    this->m_keys[PlayerControls::Buttons::Drop] = KEYID_SELECT;
    this->m_keys[PlayerControls::Buttons::Start] = KEYID_START;

    this->m_editor_keys[EditorControls::Buttons::ScrollUp] = KEYID_UP;
    this->m_editor_keys[EditorControls::Buttons::ScrollDown] = KEYID_DOWN;
    this->m_editor_keys[EditorControls::Buttons::ScrollLeft] = KEYID_LEFT;
    this->m_editor_keys[EditorControls::Buttons::ScrollRight] = KEYID_RIGHT;
    this->m_editor_keys[EditorControls::Buttons::FastScroll] = KEYID_A;
    this->m_editor_keys[EditorControls::Buttons::ModeSelect] = KEYID_B;
    this->m_editor_keys[EditorControls::Buttons::ModeErase] = KEYID_Y;
    this->m_editor_keys[EditorControls::Buttons::PrevSection] = KEYID_L;
    this->m_editor_keys[EditorControls::Buttons::NextSection] = KEYID_R;
    this->m_editor_keys[EditorControls::Buttons::SwitchScreens] = KEYID_SELECT;
    this->m_editor_keys[EditorControls::Buttons::TestPlay] = KEYID_START;
}

bool InputMethodProfile_16M::PollPrimaryButton(ControlsClass c, size_t i)
{
    if(c == ControlsClass::Cursor)
        return true;

    // note: m_canPoll is initialized to false
    InputMethodType_16M* k = dynamic_cast<InputMethodType_16M*>(this->Type);

    if(!k)
        return false;

    uint32_t keys_held = keysCurrent();

    int key;

    for(key = 0; key < 32; key++)
    {
        if(BIT(key) & keys_held)
            break;
    }

    // if didn't find any key, allow poll in future but return false
    if(key == 32)
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
    int* keys;
    int* keys2;
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
            keys2[j] = null_key;
        else if(i != j && keys[j] == key)
        {
            if(keys2[j] != null_key)
            {
                keys[j] = keys2[j];
                keys2[j] = null_key;
            }
            else
                keys[j] = keys[i];
        }
    }

    keys[i] = key;
    return true;
}

bool InputMethodProfile_16M::PollSecondaryButton(ControlsClass c, size_t i)
{
    // note: m_canPoll is initialized to false
    InputMethodType_16M* k = dynamic_cast<InputMethodType_16M*>(this->Type);

    if(!k)
        return false;

    uint32_t keys_held = keysCurrent();

    int key;

    for(key = 0; key < 32; key++)
    {
        if(BIT(key) & keys_held)
            break;
    }

    // if didn't find any key, allow poll in future but return false
    if(key == 32)
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
    int* keys;
    int* keys2;
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
        else if(keys && i != j && keys[j] == key)
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
    else if(keys)
        keys[i] = key;

    return true;
}

bool InputMethodProfile_16M::DeletePrimaryButton(ControlsClass c, size_t i)
{
    // resolve the particular primary and secondary key arrays
    int* keys;
    int* keys2;

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

bool InputMethodProfile_16M::DeleteSecondaryButton(ControlsClass c, size_t i)
{
    int* keys2;

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

    if(keys2[i] != null_key)
    {
        keys2[i] = null_key;
        return true;
    }

    return false;
}

const char* InputMethodProfile_16M::NamePrimaryButton(ControlsClass c, size_t i)
{
    int* keys;

    if(c == ControlsClass::Player)
        keys = this->m_keys;
    else if(c == ControlsClass::Cursor)
        return "(PEN)";
    else if(c == ControlsClass::Editor)
        keys = this->m_editor_keys;
    else if(c == ControlsClass::Hotkey)
        keys = this->m_hotkeys;
    else
        return "";

    if(keys[i] == null_key)
        return "NONE";

    if(keys[i] < 0 || keys[i] >= 32)
        return "(INVALID)";

    return KEYNAMES[keys[i]];
}

const char* InputMethodProfile_16M::NameSecondaryButton(ControlsClass c, size_t i)
{
    int* keys2;

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

    if(keys2[i] < 0 || keys2[i] >= 32)
        return "(INVALID)";

    return KEYNAMES[keys2[i]];
}

void InputMethodProfile_16M::SaveConfig(IniProcessing* ctl)
{
    char name2[20];

    for(int a = 0; a < 4; a++)
    {
        int* keys;
        int* keys2;
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

void InputMethodProfile_16M::LoadConfig(IniProcessing* ctl)
{
    char name2[20];

    for(int a = 0; a < 4; a++)
    {
        int* keys;
        int* keys2;
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
|| implementation for InputMethodType_16M        ||
\*===============================================*/

InputMethodProfile* InputMethodType_16M::AllocateProfile() noexcept
{
    return (InputMethodProfile*) new(std::nothrow) InputMethodProfile_16M;
}

InputMethodType_16M::InputMethodType_16M()
{
    this->Name = "NDS";
}

InputMethodType_16M::~InputMethodType_16M()
{
}

bool InputMethodType_16M::TestProfileType(InputMethodProfile* profile)
{
    return (bool)dynamic_cast<InputMethodProfile_16M*>(profile);
}

bool InputMethodType_16M::RumbleSupported()
{
    return false;
}

void InputMethodType_16M::UpdateControlsPre()
{
    scanKeys();
}

void InputMethodType_16M::UpdateControlsPost()
{
    uint32_t keys_pressed = keysDown();
    uint32_t keys_held = keysCurrent();

    // handle the mouse
    int tx, ty;
    touchPosition pos;
    touchRead(&pos);
    XRender::mapToScreen(pos.px, pos.py, &tx, &ty);

    if(keys_pressed & KEY_TOUCH)
    {
        if((LevelEditor || MagicHand) && ty > 40 && !editorScreen.active)
        {
            int distance2 = (m_lastTouchX - tx)*(m_lastTouchX - tx)
                + (m_lastTouchY - ty)*(m_lastTouchY - ty);

            if(distance2 < 64)
                m_click_accepted = true;
            else
                m_click_accepted = false;
        }
        else
            m_click_accepted = true;
    }

    if(keys_held & KEY_TOUCH)
    {
        if(tx - m_lastTouchX < -1 || tx - m_lastTouchX > 1
            || ty - m_lastTouchY < -1 || ty - m_lastTouchY > 1)
        {
            SharedCursor.Move = true;
            SharedCursor.X = tx;
            SharedCursor.Y = ty;

            m_lastTouchX = tx;
            m_lastTouchY = ty;
        }

        if(m_click_accepted)
        {
            SharedCursor.Primary = true;
        }
    }
}

InputMethod* InputMethodType_16M::Poll(const std::vector<InputMethod*>& active_methods) noexcept
{
    if(this->m_numPlayers != this->m_lastNumPlayers)
    {
        // this ensures that keys that were held when a keyboard method was removed cannot be polled to add that method back
        if(this->m_numPlayers < this->m_lastNumPlayers)
            this->m_canPoll = false;

        this->m_lastNumPlayers = this->m_numPlayers;
    }

    if(this->m_numPlayers >= m_maxPlayers)
    {
        // ban polling in case things change
        this->m_canPoll = false;
        return nullptr;
    }

    uint32_t keys_held = keysCurrent();

    // ban attachment from active profile, must find new profile
    int key;
    InputMethodProfile* target_profile = nullptr;

    for(key = 0; key < 32; key++)
    {
        if((BIT(key) & keys_held) == 0)
            continue;

        bool allowed = true;

        // ban attachment from active profile
        for(InputMethod* method : active_methods)
        {
            if(!allowed)
                break;

            if(!method)
                continue;

            InputMethodProfile_16M* p = dynamic_cast<InputMethodProfile_16M*>(method->Profile);

            if(!p)
                continue;

            for(size_t i = 0; i < PlayerControls::n_buttons; i++)
            {
                if(p->m_keys[i] == key || p->m_keys2[i] == key)
                {
                    allowed = false;
                    break;
                }
            }

            if(LevelEditor)
            {
                for(size_t i = 0; i < EditorControls::n_buttons; i++)
                {
                    if(p->m_editor_keys[i] == key || p->m_editor_keys2[i] == key)
                    {
                        allowed = false;
                        break;
                    }
                }
            }

            for(size_t i = 0; i < CursorControls::n_buttons; i++)
            {
                if(p->m_cursor_keys2[i] == key)
                {
                    allowed = false;
                    break;
                }
            }

            for(size_t i = 0; i < Hotkeys::n_buttons; i++)
            {
                if(p->m_hotkeys[i] == key || p->m_hotkeys2[i] == key)
                {
                    allowed = false;
                    break;
                }
            }
        }

        if(!allowed)
            continue;

        // which player index is connecting?
        int my_index = 0;

        for(const InputMethod* method : active_methods)
        {
            if(!method)
                break;

            my_index ++;
        }

        // try to find profile matching the keypress
        for(int i = -1; i < (int)this->m_profiles.size(); i++)
        {
            // start with the most recent profile for this player index
            InputMethodProfile* profile;

            if(i == -1)
                profile = this->GetDefaultProfile(my_index);
            else
                profile = this->m_profiles[i];

            if(!profile)
                continue;

            InputMethodProfile_16M* p = dynamic_cast<InputMethodProfile_16M*>(profile);

            if(!p)
                continue;

            for(size_t j = 0; j < PlayerControls::n_buttons; j++)
            {
                if(p->m_keys[j] == key || p->m_keys2[j] == key)
                {
                    target_profile = profile;
                    break;
                }
            }

            if(target_profile)
                break;
        }

        if(target_profile || this->m_profiles.empty())
            break;
    }

    // if didn't find any key allow poll in future but return false
    if(key == 32)
    {
        this->m_canPoll = true;
        return nullptr;
    }

    // if poll not allowed, return false
    if(!this->m_canPoll)
        return nullptr;

    // we're going to create a new keyboard!
    // reset canPoll for next time
    this->m_canPoll = false;

    InputMethod_16M* method = new(std::nothrow) InputMethod_16M;

    if(!method)
        return nullptr;

    method->Name = "NDS";
    method->Type = this;
    method->Profile = target_profile;

    this->m_numPlayers ++;

    return (InputMethod*)method;
}

/*-----------------------*\
|| OPTIONAL METHODS      ||
\*-----------------------*/
bool InputMethodType_16M::ConsumeEvent(const SDL_Event* ev)
{
    UNUSED(ev);
    return false;
}

// optional function allowing developer to associate device information with profile, etc
bool InputMethodType_16M::SetProfile_Custom(InputMethod* method, int player_no, InputMethodProfile* profile,
        const std::vector<InputMethod*>& active_methods)
{
    if(!method || !profile || player_no < 0 || player_no >= maxLocalPlayers)
        return false;

    // prevent duplicates of a profile from ever being set
    for(InputMethod* o_method : active_methods)
    {
        if(!o_method)
            continue;

        if(o_method != method && o_method->Profile == profile)
            return false;
    }

    m_canPoll = false;
    return true;
}

// How many per-type special options are there?
size_t InputMethodType_16M::GetOptionCount()
{
    return 1;
}

// Methods to manage per-profile options
// It is guaranteed that none of these will be called if
// GetOptionCount() returns 0.
// get a char* describing the option
const char* InputMethodType_16M::GetOptionName(size_t i)
{
    if(i == 0)
        return "MAX PLAYERS";

    return nullptr;
}

// get a char* describing the current option value
// must be allocated in static or instance memory
// WILL NOT be freed
const char* InputMethodType_16M::GetOptionValue(size_t i)
{
    if(i == 0)
    {
        static char buf[3];
        snprintf(buf, 3, "%d", this->m_maxPlayers);
        return buf;
    }

    return nullptr;
}

// called when A is pressed; allowed to interrupt main game loop
bool InputMethodType_16M::OptionChange(size_t i)
{
    if(i == 0)
    {
        this->m_maxPlayers ++;

        if(this->m_maxPlayers > 2)
            this->m_maxPlayers = 0;

        return true;
    }

    return false;
}

// called when left is pressed
bool InputMethodType_16M::OptionRotateLeft(size_t i)
{
    if(i == 0)
    {
        if(this->m_maxPlayers > 0)
        {
            this->m_maxPlayers --;
            return true;
        }
    }

    return false;
}

// called when right is pressed
bool InputMethodType_16M::OptionRotateRight(size_t i)
{
    if(i == 0)
    {
        if(this->m_maxPlayers < 2)
        {
            this->m_maxPlayers ++;
            return true;
        }
    }

    return false;
}

void InputMethodType_16M::SaveConfig_Custom(IniProcessing* ctl)
{
    ctl->setValue("max-players", this->m_maxPlayers);
}

void InputMethodType_16M::LoadConfig_Custom(IniProcessing* ctl)
{
    ctl->read("max-players", this->m_maxPlayers, 2);
}

} // namespace Controls
