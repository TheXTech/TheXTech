/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2023 Vitaly Novichkov <admin@wohlnet.ru>
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

#include <SDL2/SDL_scancode.h>
#include <SDL2/SDL_power.h>
#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_events.h>
#include "sdl_proxy/sdl_timer.h"

#include "../controls.h"
#include "../globals.h"
#include "keyboard.h"
#include "../main/screen_textentry.h"
#include "../game_main.h"

#include "core/render.h"
#include "core/window.h"
#include "core/power.h"

#include "main/cheat_code.h"
#include "main/menu_main.h"

#include "control/controls_strings.h"

#include <Logger/logger.h>

namespace Controls
{

// helper functions


/*====================================================*\
|| implementation for InputMethod_Keyboard            ||
\*====================================================*/

InputMethod_Keyboard::~InputMethod_Keyboard()
{
    InputMethodType_Keyboard* t = dynamic_cast<InputMethodType_Keyboard*>(this->Type);

    if(!t)
        return;

    t->m_numKeyboards --;
}

// Update functions that set player controls (and editor controls)
// based on current device input. Return false if device lost.
bool InputMethod_Keyboard::Update(int player, Controls_t& c, CursorControls_t& m, EditorControls_t& e, HotkeysPressed_t& h)
{
    InputMethodType_Keyboard* k = dynamic_cast<InputMethodType_Keyboard*>(this->Type);
    InputMethodProfile_Keyboard* p = dynamic_cast<InputMethodProfile_Keyboard*>(this->Profile);

    if(!k || !p)
        return false;

    if(k->m_directText && GamePaused == PauseCode::TextEntry)
        return true;

    bool altPressed = (k->m_keyboardState[SDL_SCANCODE_LALT] ||
                       k->m_keyboardState[SDL_SCANCODE_RALT] ||
                       k->m_keyboardState[SDL_SCANCODE_LCTRL] ||
                       k->m_keyboardState[SDL_SCANCODE_RCTRL]);

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

            if(altPressed && (key == SDL_SCANCODE_F || key == SDL_SCANCODE_RETURN))
                key = null_key;

            if(altPressed && (key2 == SDL_SCANCODE_F || key2 == SDL_SCANCODE_RETURN))
                key2 = null_key;

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

            if(key != null_key && k->m_keyboardState[key] != 0)
                *b = true;
            else if(key2 != null_key && k->m_keyboardState[key2] != 0)
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

        if(key != null_key && k->m_keyboardState[key] != 0)
            *scroll[i] += 10.0;
        else if(key2 != null_key && k->m_keyboardState[key2] != 0)
            *scroll[i] += 10.0;
    }

    bool cursor[4];

    for(int i = 0; i < 4; i++)
    {
        int key = p->m_cursor_keys2[i];
        cursor[i] = (key != null_key && k->m_keyboardState[key]);
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

void InputMethod_Keyboard::Rumble(int ms, float strength)
{
    UNUSED(ms);
    UNUSED(strength);
}

StatusInfo InputMethod_Keyboard::GetStatus()
{
    return XPower::devicePowerStatus();
}

/*====================================================*\
|| implementation for InputMethodProfile_Keyboard     ||
\*====================================================*/

// the job of this function is to initialize the class in a consistent state
InputMethodProfile_Keyboard::InputMethodProfile_Keyboard()
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

    this->m_keys[PlayerControls::Buttons::Up] = SDL_SCANCODE_UP;
    this->m_keys[PlayerControls::Buttons::Down] = SDL_SCANCODE_DOWN;
    this->m_keys[PlayerControls::Buttons::Left] = SDL_SCANCODE_LEFT;
    this->m_keys[PlayerControls::Buttons::Right] = SDL_SCANCODE_RIGHT;
    this->m_keys[PlayerControls::Buttons::Jump] = SDL_SCANCODE_Z;
    this->m_keys[PlayerControls::Buttons::AltJump] = SDL_SCANCODE_A;
    this->m_keys[PlayerControls::Buttons::Run] = SDL_SCANCODE_X;
    this->m_keys[PlayerControls::Buttons::AltRun] = SDL_SCANCODE_S;
    this->m_keys[PlayerControls::Buttons::Drop] = SDL_SCANCODE_LSHIFT;
    this->m_keys2[PlayerControls::Buttons::Drop] = SDL_SCANCODE_RSHIFT;
    this->m_keys[PlayerControls::Buttons::Start] = SDL_SCANCODE_RETURN;

    this->m_editor_keys[EditorControls::Buttons::ScrollUp] = SDL_SCANCODE_UP;
    this->m_editor_keys[EditorControls::Buttons::ScrollDown] = SDL_SCANCODE_DOWN;
    this->m_editor_keys[EditorControls::Buttons::ScrollLeft] = SDL_SCANCODE_LEFT;
    this->m_editor_keys[EditorControls::Buttons::ScrollRight] = SDL_SCANCODE_RIGHT;
    this->m_editor_keys[EditorControls::Buttons::FastScroll] = SDL_SCANCODE_LSHIFT;
    this->m_editor_keys[EditorControls::Buttons::ModeSelect] = SDL_SCANCODE_Z;
    this->m_editor_keys[EditorControls::Buttons::ModeErase] = SDL_SCANCODE_X;
    this->m_editor_keys[EditorControls::Buttons::PrevSection] = SDL_SCANCODE_A;
    this->m_editor_keys[EditorControls::Buttons::NextSection] = SDL_SCANCODE_S;
    this->m_editor_keys[EditorControls::Buttons::SwitchScreens] = SDL_SCANCODE_RSHIFT;
    this->m_editor_keys[EditorControls::Buttons::TestPlay] = SDL_SCANCODE_RETURN;

    // ALSO UPDATE InputMethodType_Keyboard::DefaultHotkey
    this->m_hotkeys[Hotkeys::Buttons::ToggleHUD] = SDL_SCANCODE_F1;
    this->m_hotkeys[Hotkeys::Buttons::DebugInfo] = SDL_SCANCODE_F3;
    this->m_hotkeys[Hotkeys::Buttons::Fullscreen] = SDL_SCANCODE_F7;
#ifdef __APPLE__ // on macOS the F11 key is reserved by the "Show Desktop" global action
    this->m_hotkeys[Hotkeys::Buttons::RecordGif] = SDL_SCANCODE_F10;
#else
    this->m_hotkeys[Hotkeys::Buttons::RecordGif] = SDL_SCANCODE_F11;
#endif
    this->m_hotkeys[Hotkeys::Buttons::Screenshot] = SDL_SCANCODE_F12;
    this->m_hotkeys2[Hotkeys::Buttons::Screenshot] = SDL_SCANCODE_F2;
#ifdef DEBUG_BUILD
    this->m_hotkeys[Hotkeys::Buttons::ReloadLanguage] = SDL_SCANCODE_F5;
#endif
}

bool InputMethodProfile_Keyboard::PollPrimaryButton(ControlsClass c, size_t i)
{
    if(c == ControlsClass::Cursor)
        return true;

    // note: m_canPoll is initialized to false
    InputMethodType_Keyboard* k = dynamic_cast<InputMethodType_Keyboard*>(this->Type);

    if(!k)
        return false;

    int key;

    for(key = 0; key < k->m_keyboardStateSize; key++)
    {
        if(k->m_keyboardState[key] != 0)
            break;
    }

    // if didn't find any key, allow poll in future but return false
    if(key == k->m_keyboardStateSize)
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
        D_pLogWarning("Polling Keyboard primary button with disallowed controls class %d\n", (int)c);
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

bool InputMethodProfile_Keyboard::PollSecondaryButton(ControlsClass c, size_t i)
{
    // note: m_canPoll is initialized to false
    InputMethodType_Keyboard* k = dynamic_cast<InputMethodType_Keyboard*>(this->Type);

    if(!k)
        return false;

    int key;

    for(key = 0; key < k->m_keyboardStateSize; key++)
    {
        if(k->m_keyboardState[key] != 0)
            break;
    }

    // if didn't find any key, allow poll in future but return false
    if(key == k->m_keyboardStateSize)
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
        D_pLogWarning("Polling Keyboard secondary button with disallowed controls class %d\n", (int)c);
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

bool InputMethodProfile_Keyboard::DeletePrimaryButton(ControlsClass c, size_t i)
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
        D_pLogWarning("Attempted to delete Keyboard primary button with disallowed controls class %d\n", (int)c);
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

bool InputMethodProfile_Keyboard::DeleteSecondaryButton(ControlsClass c, size_t i)
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

const char* InputMethodProfile_Keyboard::NamePrimaryButton(ControlsClass c, size_t i)
{
    int* keys;

    if(c == ControlsClass::Player)
        keys = this->m_keys;
    else if(c == ControlsClass::Cursor)
        return g_controlsStrings.caseMouse.c_str();
    else if(c == ControlsClass::Editor)
        keys = this->m_editor_keys;
    else if(c == ControlsClass::Hotkey)
        keys = this->m_hotkeys;
    else
        return "";

    if(keys[i] == null_key)
        return g_mainMenu.caseNone.c_str();

    return SDL_GetScancodeName((SDL_Scancode)keys[i]);
}

const char* InputMethodProfile_Keyboard::NameSecondaryButton(ControlsClass c, size_t i)
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

    if(keys2[i] == null_key)
        return "";

    return SDL_GetScancodeName((SDL_Scancode)keys2[i]);
}

void InputMethodProfile_Keyboard::SaveConfig(IniProcessing* ctl)
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

void InputMethodProfile_Keyboard::LoadConfig(IniProcessing* ctl)
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

            // only add RSHIFT secondary key during conversion if current primary key is LSHIFT
            int def = keys2[i];

            if(a == 0 && i == PlayerControls::Buttons::Drop && keys[i] != SDL_SCANCODE_LSHIFT)
                def = null_key;

            ctl->read(name2, keys2[i], def);
        }
    }
}

/*====================================================*\
|| implementation for InputMethodType_Keyboard        ||
\*====================================================*/

InputMethodProfile* InputMethodType_Keyboard::AllocateProfile() noexcept
{
    return (InputMethodProfile*) new(std::nothrow) InputMethodProfile_Keyboard;
}

InputMethodType_Keyboard::InputMethodType_Keyboard()
{
    this->m_keyboardState = SDL_GetKeyboardState(&this->m_keyboardStateSize);
    this->Name = "Keyboard";
    this->LegacyName = "keyboard";
}

const std::string& InputMethodType_Keyboard::LocalName() const
{
    return g_controlsStrings.nameKeyboard;
}

bool InputMethodType_Keyboard::TestProfileType(InputMethodProfile* profile)
{
    return (bool)dynamic_cast<InputMethodProfile_Keyboard*>(profile);
}

bool InputMethodType_Keyboard::RumbleSupported()
{
    return false;
}

void InputMethodType_Keyboard::UpdateControlsPre() {}
void InputMethodType_Keyboard::UpdateControlsPost()
{
    if(this->m_touchscreenActive && g_renderTouchscreen)
    {
        // let the touchscreen input method handle the mouse
    }
    // handle the mouse
    else if(SDL_GetMouseFocus())
    {
        int window_x, window_y;
        Uint32 buttons = SDL_GetMouseState(&window_x, &window_y);

        SDL_Point p;
        XRender::mapToScreen(window_x, window_y, &p.x, &p.y);
        static SDL_Point last_p;

        if(p.x - last_p.x <= -1 || p.x - last_p.x >= 1 ||
           p.y - last_p.y <= -1 || p.y - last_p.y >= 1)
        {
            last_p = p;
            SharedCursor.Move = true;
            SharedCursor.X = p.x;
            SharedCursor.Y = p.y;
        }

        if(buttons & SDL_BUTTON_LMASK)
            SharedCursor.Primary = true;

        if(buttons & SDL_BUTTON_RMASK)
            SharedCursor.Secondary = true;

        if(buttons & SDL_BUTTON_MMASK)
            SharedCursor.Tertiary = true;
    }
    else if(XWindow::hasWindowInputFocus() && !SharedCursor.Move && (SharedCursor.X >= 0 || SharedCursor.Y >= 0))
    {
        SharedCursor.GoOffscreen();
    }

    if(this->m_scroll >= 1)
    {
        this->m_scroll -= 1;
        SharedCursor.ScrollDown |= true;
    }
    else if(this->m_scroll <= -1)
    {
        this->m_scroll += 1;
        SharedCursor.ScrollUp |= true;
    }

    if(this->m_directText && GamePaused == PauseCode::TextEntry)
        return;

    bool altPressed = this->m_keyboardState[SDL_SCANCODE_LALT] ||
                      this->m_keyboardState[SDL_SCANCODE_RALT];
    bool escBackPressed = this->m_keyboardState[SDL_SCANCODE_ESCAPE];
    bool escPausePressed = this->m_keyboardState[SDL_SCANCODE_ESCAPE];
    bool returnPressed = this->m_keyboardState[SDL_SCANCODE_RETURN];
    bool spacePressed = this->m_keyboardState[SDL_SCANCODE_SPACE];
    bool upPressed = this->m_keyboardState[SDL_SCANCODE_UP];
    bool downPressed = this->m_keyboardState[SDL_SCANCODE_DOWN];
    bool leftPressed = this->m_keyboardState[SDL_SCANCODE_LEFT];
    bool rightPressed = this->m_keyboardState[SDL_SCANCODE_RIGHT];

    // disable the shared keys if they are currently in use
    for(InputMethod* method : g_InputMethods)
    {
        if(!method)
            continue;

        InputMethodProfile* p = method->Profile;
        InputMethodProfile_Keyboard* profile = dynamic_cast<InputMethodProfile_Keyboard*>(p);

        if(!profile)
            continue;

        for(size_t i = 0; i < PlayerControls::n_buttons; i++)
        {
            if(profile->m_keys[i] == SDL_SCANCODE_ESCAPE || profile->m_keys2[i] == SDL_SCANCODE_ESCAPE)
            {
                // allow escape to count even when it is a player's start key, important for correct menu behavior
                if(i != PlayerControls::Buttons::Start)
                    escBackPressed = false;

                escPausePressed = false;
            }

            if(profile->m_keys[i] == SDL_SCANCODE_RETURN || profile->m_keys2[i] == SDL_SCANCODE_RETURN)
                returnPressed = false;

            if(profile->m_keys[i] == SDL_SCANCODE_SPACE || profile->m_keys2[i] == SDL_SCANCODE_SPACE)
                spacePressed = false;

            if(profile->m_keys[i] == SDL_SCANCODE_UP || profile->m_keys2[i] == SDL_SCANCODE_UP)
                upPressed = false;

            if(profile->m_keys[i] == SDL_SCANCODE_DOWN || profile->m_keys2[i] == SDL_SCANCODE_DOWN)
                downPressed = false;

            if(profile->m_keys[i] == SDL_SCANCODE_LEFT || profile->m_keys2[i] == SDL_SCANCODE_LEFT)
                leftPressed = false;

            if(profile->m_keys[i] == SDL_SCANCODE_RIGHT || profile->m_keys2[i] == SDL_SCANCODE_RIGHT)
                rightPressed = false;
        }
    }

#ifdef __ANDROID__ // Quit credits on BACK key press
    bool backPressed = this->m_keyboardState[SDL_SCANCODE_AC_BACK];
#else
    bool backPressed = false;
#endif

    SharedControls.QuitCredits |= (spacePressed || backPressed);
    SharedControls.QuitCredits |= (returnPressed && !altPressed);
    SharedControls.QuitCredits |= (escBackPressed && !altPressed);

    SharedControls.Pause |= backPressed;
    SharedControls.Pause |= (escPausePressed && !altPressed);

    SharedControls.MenuUp |= upPressed;
    SharedControls.MenuDown |= downPressed;
    SharedControls.MenuLeft |= leftPressed;
    SharedControls.MenuRight |= rightPressed;
    SharedControls.MenuDo |= (returnPressed && !altPressed) || spacePressed;
    SharedControls.MenuBack |= backPressed;
    SharedControls.MenuBack |= (escBackPressed && !altPressed);
}

// this is challenging for the keyboard because we don't want to allocate 20 copies of it
InputMethod* InputMethodType_Keyboard::Poll(const std::vector<InputMethod*>& active_methods) noexcept
{
    if(this->m_numKeyboards != this->m_lastNumKeyboards)
    {
        // this ensures that keys that were held when a keyboard method was removed cannot be polled to add that method back
        if(this->m_numKeyboards < this->m_lastNumKeyboards)
            this->m_canPoll = false;

        this->m_lastNumKeyboards = this->m_numKeyboards;
    }

    if(this->m_numKeyboards >= m_maxKeyboards)
    {
        // ban polling in case things change
        this->m_canPoll = false;
        return nullptr;
    }


    // ban attachment from active profile, must find new profile
    int key;
    InputMethodProfile* target_profile = nullptr;

    for(key = 0; key < this->m_keyboardStateSize; key++)
    {
        if(!this->m_keyboardState[key])
            continue;

        bool allowed = true;

        if(key == SDL_SCANCODE_LALT || key == SDL_SCANCODE_RALT || key == SDL_SCANCODE_LCTRL || key == SDL_SCANCODE_RCTRL || key == SDL_SCANCODE_ESCAPE)
            allowed = false;

        // ban attachment from active profile
        for(InputMethod* method : active_methods)
        {
            if(!allowed)
                break;

            if(!method)
                continue;

            InputMethodProfile_Keyboard* p = dynamic_cast<InputMethodProfile_Keyboard*>(method->Profile);

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

            InputMethodProfile_Keyboard* p = dynamic_cast<InputMethodProfile_Keyboard*>(profile);

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
    if(key == this->m_keyboardStateSize)
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

    InputMethod_Keyboard* method = new(std::nothrow) InputMethod_Keyboard;

    if(!method)
        return nullptr;

    method->Name = this->LocalName();
    method->Type = this;
    method->Profile = target_profile;

    this->m_numKeyboards ++;

    return (InputMethod*)method;
}

/*-----------------------*\
|| OPTIONAL METHODS      ||
\*-----------------------*/
bool InputMethodType_Keyboard::DefaultHotkey(const SDL_Event* ev)
{
    const SDL_KeyboardEvent& evt = ev->key;

    int KeyCode = evt.keysym.scancode;

    bool ctrlF = ((evt.keysym.mod & KMOD_CTRL) != 0 && evt.keysym.scancode == SDL_SCANCODE_F);
    bool altEnter = ((evt.keysym.mod & KMOD_ALT) != 0 && (evt.keysym.scancode == SDL_SCANCODE_RETURN || evt.keysym.scancode == SDL_SCANCODE_KP_ENTER));

    if(ctrlF || altEnter)
        g_hotkeysPressed[Hotkeys::Buttons::Fullscreen] = 0;

    bool use_defaults = false;

    if(this->m_numKeyboards == 0)
        use_defaults = true;

    if(this->m_directText && GamePaused == PauseCode::TextEntry)
        use_defaults = true;

    if(use_defaults && evt.repeat == 0)
    {
        // ALSO UPDATE InputMethodProfile_Keyboard::InputMethodProfile_Keyboard
        if(KeyCode == SDL_SCANCODE_F12 || KeyCode == SDL_SCANCODE_F2)
            g_hotkeysPressed[Hotkeys::Buttons::Screenshot] = 0;
        else if(KeyCode == SDL_SCANCODE_F3)
            g_hotkeysPressed[Hotkeys::Buttons::DebugInfo] = 0;
        else if(KeyCode == SDL_SCANCODE_F1)
            g_hotkeysPressed[Hotkeys::Buttons::ToggleHUD] = 0;

#ifdef __APPLE__
        else if(KeyCode == SDL_SCANCODE_F10) // Reserved by macOS as "show desktop"
#else
        else if(KeyCode == SDL_SCANCODE_F11)
#endif
            g_hotkeysPressed[Hotkeys::Buttons::RecordGif] = 0;
    }

    if(this->m_directText && GamePaused == PauseCode::TextEntry)
        return false;

    SDL_Scancode KeyASCII = evt.keysym.scancode;

    // classic cheat codes
    switch(KeyASCII)
    {
    case SDL_SCANCODE_A:
        CheatCode('a');
        break;

    case SDL_SCANCODE_B:
        CheatCode('b');
        break;

    case SDL_SCANCODE_C:
        CheatCode('c');
        break;

    case SDL_SCANCODE_D:
        CheatCode('d');
        break;

    case SDL_SCANCODE_E:
        CheatCode('e');
        break;

    case SDL_SCANCODE_F:
        CheatCode('f');
        break;

    case SDL_SCANCODE_G:
        CheatCode('g');
        break;

    case SDL_SCANCODE_H:
        CheatCode('h');
        break;

    case SDL_SCANCODE_I:
        CheatCode('i');
        break;

    case SDL_SCANCODE_J:
        CheatCode('j');
        break;

    case SDL_SCANCODE_K:
        CheatCode('k');
        break;

    case SDL_SCANCODE_L:
        CheatCode('l');
        break;

    case SDL_SCANCODE_M:
        CheatCode('m');
        break;

    case SDL_SCANCODE_N:
        CheatCode('n');
        break;

    case SDL_SCANCODE_O:
        CheatCode('o');
        break;

    case SDL_SCANCODE_P:
        CheatCode('p');
        break;

    case SDL_SCANCODE_Q:
        CheatCode('q');
        break;

    case SDL_SCANCODE_R:
        CheatCode('r');
        break;

    case SDL_SCANCODE_S:
        CheatCode('s');
        break;

    case SDL_SCANCODE_T:
        CheatCode('t');
        break;

    case SDL_SCANCODE_U:
        CheatCode('u');
        break;

    case SDL_SCANCODE_V:
        CheatCode('v');
        break;

    case SDL_SCANCODE_W:
        CheatCode('w');
        break;

    case SDL_SCANCODE_X:
        CheatCode('x');
        break;

    case SDL_SCANCODE_Y:
        CheatCode('y');
        break;

    case SDL_SCANCODE_Z:
        CheatCode('z');
        break;

    case SDL_SCANCODE_1:
        CheatCode('1');
        break;

    case SDL_SCANCODE_2:
        CheatCode('2');
        break;

    case SDL_SCANCODE_3:
        CheatCode('3');
        break;

    case SDL_SCANCODE_4:
        CheatCode('4');
        break;

    case SDL_SCANCODE_5:
        CheatCode('5');
        break;

    case SDL_SCANCODE_6:
        CheatCode('6');
        break;

    case SDL_SCANCODE_7:
        CheatCode('7');
        break;

    case SDL_SCANCODE_8:
        CheatCode('8');
        break;

    case SDL_SCANCODE_9:
        CheatCode('9');
        break;

    case SDL_SCANCODE_0:
        CheatCode('0');
        break;

    case SDL_SCANCODE_SEMICOLON:
        CheatCode(';');
        break; // for AZERTY support

    default:
        CheatCode(' ');
        break;
    }

    return true;
}

bool InputMethodType_Keyboard::ConsumeEvent(const SDL_Event* ev)
{
    switch(ev->type)
    {
    case SDL_MOUSEWHEEL:
        if(ev->wheel.which != SDL_TOUCH_MOUSEID)
        {
            // scrolling up results in traversing items backwards
            this->m_scroll -= ev->wheel.y;
            return true;
        }
        else
            return false;

    case SDL_MOUSEBUTTONUP:
        if(ev->button.button == SDL_BUTTON_LEFT && ev->button.which != SDL_TOUCH_MOUSEID)
        {
            bool doubleClick = (this->m_lastMousePress + 300) >= SDL_GetTicks();
            this->m_lastMousePress = SDL_GetTicks();

            if(doubleClick && !MagicHand && !LevelEditor)
            {
                this->m_lastMousePress = 0;
                g_hotkeysPressed[Hotkeys::Buttons::Fullscreen] = 0;
                return true;
            }
        }

    // intentional fallthrough
    case SDL_MOUSEBUTTONDOWN:
        if(ev->button.which == SDL_TOUCH_MOUSEID)
            this->m_touchscreenActive = true;
        else
            this->m_touchscreenActive = false;

        break;

    case SDL_MOUSEMOTION:
        if(ev->motion.which == SDL_TOUCH_MOUSEID)
            this->m_touchscreenActive = true;
        else
            this->m_touchscreenActive = false;

        break;

    case SDL_TEXTINPUT:
        if(this->m_directText && GamePaused == PauseCode::TextEntry)
        {
            TextEntryScreen::Insert(ev->text.text);
            return true;
        }

        return false;

    case SDL_KEYDOWN:
        if(this->DefaultHotkey(ev))
            return true;

        if(this->m_directText && GamePaused == PauseCode::TextEntry)
        {
            if(ev->key.keysym.scancode == SDL_SCANCODE_RETURN || ev->key.keysym.scancode == SDL_SCANCODE_KP_ENTER)
                TextEntryScreen::Commit();
            else if(ev->key.keysym.scancode == SDL_SCANCODE_LEFT)
                TextEntryScreen::CursorLeft();
            else if(ev->key.keysym.scancode == SDL_SCANCODE_RIGHT)
                TextEntryScreen::CursorRight();
            else if(ev->key.keysym.scancode == SDL_SCANCODE_BACKSPACE)
                TextEntryScreen::Backspace();

            return true;
        }

        return false;

    default:
        return false;
    }

    return false;
}

// optional function allowing developer to associate device information with profile, etc
bool InputMethodType_Keyboard::SetProfile_Custom(InputMethod* method, int player_no, InputMethodProfile* profile,
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
size_t InputMethodType_Keyboard::GetOptionCount()
{
    return 2;
}

// Methods to manage per-profile options
// It is guaranteed that none of these will be called if
// GetOptionCount() returns 0.
// get a char* describing the option
const char* InputMethodType_Keyboard::GetOptionName(size_t i)
{
    if(i == 0)
        return g_controlsStrings.sharedOptionMaxPlayers.c_str();

    if(i == 1)
        return g_controlsStrings.keyboardOptionTextEntryStyle.c_str();

    return nullptr;
}

// get a char* describing the current option value
// must be allocated in static or instance memory
// WILL NOT be freed
const char* InputMethodType_Keyboard::GetOptionValue(size_t i)
{
    if(i == 0)
    {
        static char buf[3];
        snprintf(buf, 3, "%d", this->m_maxKeyboards);
        return buf;
    }

    if(i == 1)
    {
        if(this->m_directText)
            return g_controlsStrings.nameKeyboard.c_str();
        else
            return g_controlsStrings.nameGamepad.c_str();
    }

    return nullptr;
}

// called when A is pressed; allowed to interrupt main game loop
bool InputMethodType_Keyboard::OptionChange(size_t i)
{
    if(i == 0)
    {
        this->m_maxKeyboards ++;

        if(this->m_maxKeyboards > 2)
            this->m_maxKeyboards = 0;

        return true;
    }

    if(i == 1)
    {
        this->m_directText = !this->m_directText;
        return true;
    }

    return false;
}

// called when left is pressed
bool InputMethodType_Keyboard::OptionRotateLeft(size_t i)
{
    if(i == 0)
    {
        if(this->m_maxKeyboards > 0)
        {
            this->m_maxKeyboards --;
            return true;
        }
    }

    if(i == 1)
    {
        this->m_directText = !this->m_directText;
        return true;
    }

    return false;
}

// called when right is pressed
bool InputMethodType_Keyboard::OptionRotateRight(size_t i)
{
    if(i == 0)
    {
        if(this->m_maxKeyboards < 2)
        {
            this->m_maxKeyboards ++;
            return true;
        }
    }

    if(i == 1)
    {
        this->m_directText = !this->m_directText;
        return true;
    }

    return false;
}

void InputMethodType_Keyboard::SaveConfig_Custom(IniProcessing* ctl)
{
    ctl->setValue("max-keyboards", this->m_maxKeyboards);
    ctl->setValue("direct-text-entry", this->m_directText);
}

void InputMethodType_Keyboard::LoadConfig_Custom(IniProcessing* ctl)
{
    ctl->read("max-keyboards", this->m_maxKeyboards, 2);
    ctl->read("direct-text-entry", this->m_directText, true);
}

} // namespace Controls
