#include <SDL2/SDL_version.h>
#include <SDL2/SDL_joystick.h>
#include <SDL2/SDL_gamecontroller.h>
#include <SDL2/SDL_haptic.h>

#include <Logger/logger.h>

#include "../controls.h"
#include "joystick.h"

namespace Controls
{

/*====================================================*\
|| implementation for InputMethod_Joystick            ||
\*====================================================*/

static void s_updateJoystickKey(SDL_Joystick *j, bool &key, const KM_Key &mkey)
{
    Sint32 val = 0, dx = 0, dy = 0;
    Sint16 val_initial = 0;
    bool key_new = false;

    switch(mkey.type)
    {
    case KM_Key::JoyAxis:
        //Note: SDL_JoystickGetAxisInitialState is a new API function added into dev version
        //      and doesn't available in already released assemblies
        if(SDL_JoystickGetAxisInitialState(j, mkey.id, &val_initial) == SDL_FALSE)
        {
            key_new = false;
            break;
        }
        val = SDL_JoystickGetAxis(j, mkey.id);

        if(SDL_abs(val) <= 15000)
            key_new = false;
        else if(mkey.val > val_initial)
            key_new = (val > val_initial);
        else if(mkey.val < val_initial)
            key_new = (val < val_initial);
        else
            key_new = false;

        break;

    case KM_Key::JoyBallX:
        SDL_JoystickGetBall(j, mkey.id, &dx, &dy);

        if(mkey.id > 0)
            key_new = (dx > 0);
        else if(mkey.id < 0)
            key_new = (dx < 0);
        else key_new = false;

        break;

    case KM_Key::JoyBallY:
        SDL_JoystickGetBall(j, mkey.id, &dx, &dy);

        if(mkey.id > 0)
            key_new = (dy > 0);
        else if(mkey.id < 0)
            key_new = (dy < 0);
        else key_new = false;

        break;

    case KM_Key::JoyHat:
        val = (Sint32)SDL_JoystickGetHat(j, mkey.id);
        key_new = ((val & mkey.val)) != 0;
        break;

    case KM_Key::JoyButton:
        key_new = (0 != (Sint32)SDL_JoystickGetButton(j, mkey.id));
        break;

    default:
        key_new = false;
        break;
    }

    key |= key_new;
}

static void s_updateCtrlKey(SDL_GameController *c, bool &key, const KM_Key &mkey)
{
    bool key_new = false;

    switch(mkey.type)
    {
    // using brackets to scope val
    case KM_Key::CtrlAxis:
    {
        Sint32 val = SDL_GameControllerGetAxis(c, (SDL_GameControllerAxis)mkey.id);

        if(SDL_abs(val) <= 15000)
            key_new = false;
        else if(mkey.val > 0)
            key_new = (val > 0);
        else if(mkey.val < 0)
            key_new = (val < 0);
        else
            key_new = false;
        break;
    }
    case KM_Key::CtrlButton:
        key_new = (SDL_GameControllerGetButton(c, (SDL_GameControllerButton)mkey.id) != 0);
        break;

    default:
        key_new = false;
        break;
    }

    key |= key_new;
}

static void s_updateJoystickAnalogue(SDL_Joystick *j, double &amount, const KM_Key &mkey)
{
    Sint32 val = 0, dx = 0, dy = 0;
    Sint16 val_initial = 0;
    double amount_new = 0.;

    switch(mkey.type)
    {
    case KM_Key::JoyAxis:
        //Note: SDL_JoystickGetAxisInitialState is a new API function added into dev version
        //      and doesn't available in already released assemblies
        if(SDL_JoystickGetAxisInitialState(j, mkey.id, &val_initial) == SDL_FALSE)
        {
            amount_new = 0.;
            break;
        }
        val = SDL_JoystickGetAxis(j, mkey.id);

        if(SDL_abs(val) <= 15000)
            amount_new = 0.;
        else if(mkey.val > val_initial)
        {
            amount_new = (val - val_initial)/(32768. - val_initial);
            double other_amount_new = (val - 15000)/(32768. - 15000);
            if(other_amount_new > 0 && other_amount_new < amount_new)
                amount_new = other_amount_new;
        }
        else if(mkey.val < val_initial)
        {
            amount_new = (val_initial - val)/(val_initial + 32768.);
            double other_amount_new = (-val - 15000)/(32768. - 15000);
            if(other_amount_new > 0 && other_amount_new < amount_new)
                amount_new = other_amount_new;
        }
        else
            amount_new = 0.;

        break;

    case KM_Key::JoyBallX:
        SDL_JoystickGetBall(j, mkey.id, &dx, &dy);

        if(mkey.id > 0)
            amount_new = (dx / 32.);
        else if(mkey.id < 0)
            amount_new = (-dx / 32.);
        else amount_new = 0.;

        break;

    case KM_Key::JoyBallY:
        SDL_JoystickGetBall(j, mkey.id, &dx, &dy);

        if(mkey.id > 0)
            amount_new = (dy / 32.);
        else if(mkey.id < 0)
            amount_new = (dy / 32.);
        else amount_new = 0.;

        break;

    case KM_Key::JoyHat:
        val = (Sint32)SDL_JoystickGetHat(j, mkey.id);
        if((val & mkey.val))
            amount_new = 1.;
        break;

    case KM_Key::JoyButton:
        if((Sint32)SDL_JoystickGetButton(j, mkey.id))
            amount_new = 1.;
        break;

    default:
        amount_new = 0.;
        break;
    }

    if(amount_new > 0)
        amount = amount_new;
}

static void s_updateCtrlAnalogue(SDL_GameController *c, double &amount, const KM_Key &mkey)
{
    double amount_new = 0.;

    switch(mkey.type)
    {
    // using brackets to scope val
    case KM_Key::CtrlAxis:
    {
        Sint32 val = SDL_GameControllerGetAxis(c, (SDL_GameControllerAxis)mkey.id);

        if(SDL_abs(val) <= 15000)
            amount_new = 0.;
        else if(mkey.val > 0)
            amount_new = (val - 15000)/(32768. - 15000);
        else if(mkey.val < 0)
            amount_new = (-val - 15000)/(32768. - 15000);
        else
            amount_new = 0.;

        break;
    }
    case KM_Key::CtrlButton:
        if(SDL_GameControllerGetButton(c, (SDL_GameControllerButton)mkey.id) != 0)
            amount_new = 1.;
        break;

    default:
        amount_new = 0.;
        break;
    }

    if(amount_new > 0.)
        amount = amount_new;
}

static bool s_bindJoystickKey(SDL_Joystick *joy, KM_Key &k)
{
    Sint32 val = 0;
    Sint16 val_initial = 0;
    int dx = 0, dy = 0;
    int balls = SDL_JoystickNumBalls(joy);
    int hats = SDL_JoystickNumHats(joy);
    int buttons = SDL_JoystickNumButtons(joy);
    int axes = SDL_JoystickNumAxes(joy);

    for(int i = 0; i < buttons; i++)
    {
        val = SDL_JoystickGetButton(joy, i);

        if(val != 0)
        {
            k.val = val;
            k.id = i;
            k.type = (int)KM_Key::JoyButton;
            return true;
        }
    }

    for(int i = 0; i < hats; i++)
    {
        val = SDL_JoystickGetHat(joy, i);

        if(val != 0)
        {
            k.val = val;
            k.id = i;
            k.type = (int)KM_Key::JoyHat;
            return true;
        }
    }

    for(int i = 0; i < axes; i++)
    {
        //Note: The SDL 2.0.6 and higher is requires to support this function
        if(SDL_JoystickGetAxisInitialState(joy, i, &val_initial) == SDL_FALSE)
            break;

        val = SDL_JoystickGetAxis(joy, i);

        if(SDL_abs(val) > 15000 && val != (Sint32)val_initial)
        {
            k.val = val;
            k.id = i;
            k.type = (int)KM_Key::JoyAxis;
            return true;
        }
    }

    for(int i = 0; i < balls; i++)
    {
        dx = 0;
        dy = 0;
        SDL_JoystickGetBall(joy, i, &dx, &dy);

        if(dx != 0)
        {
            k.val = dx;
            k.id = i;
            k.type = (int)KM_Key::JoyBallX;
            return true;
        }
        else if(dy != 0)
        {
            k.val = dy;
            k.id = i;
            k.type = (int)KM_Key::JoyBallY;
            return true;
        }
    }

    k.val = 0;
    k.id = 0;
    k.type = (int)KM_Key::NoControl;
    return false;
}

static bool s_bindControllerKey(SDL_GameController *ctrl, KM_Key &k)
{
    Sint32 val = 0;

    for(int i = 0; i < SDL_CONTROLLER_BUTTON_MAX; i++)
    {
        val = SDL_GameControllerGetButton(ctrl, (SDL_GameControllerButton)i);

        if(val != 0)
        {
            k.val = val;
            k.id = i;
            k.type = (int)KM_Key::CtrlButton;
            return true;
        }
    }

    for(int i = 0; i < SDL_CONTROLLER_AXIS_MAX; i++)
    {
        val = SDL_GameControllerGetAxis(ctrl, (SDL_GameControllerAxis)i);

        if(SDL_abs(val) > 15000)
        {
            k.val = val;
            k.id = i;
            k.type = (int)KM_Key::CtrlAxis;
            return true;
        }
    }

    k.val = 0;
    k.id = 0;
    k.type = (int)KM_Key::NoControl;
    return false;
}

/*====================================================*\
|| implementation for InputMethod_Joystick            ||
\*====================================================*/

// Update functions that set player controls (and editor controls)
// based on current device input. Return false if device lost.
bool InputMethod_Joystick::Update(Controls_t& c, CursorControls_t& m, EditorControls_t& e)
{
    InputMethodProfile_Joystick* p = dynamic_cast<InputMethodProfile_Joystick*>(this->Profile);
    if(!p || !this->m_devices)
        return false;
    if(!SDL_JoystickGetAttached(this->m_devices->joy))
    {
        // note: the joystick is owned by the InputMethodType, not the InputMethod.
        this->m_devices = nullptr;
        return false;
    }
    bool hotkey_okay = true;
    for(int a = 0; a < 4; a++)
    {
        KM_Key* keys;
        KM_Key* keys2;
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
            const KM_Key& key = keys[i];
            const KM_Key& key2 = keys2[i];
            bool* b;
            if(a == 0)
            {
                b = &PlayerControls::GetButton(c, i);
                *b = false;
            }
            else if(a == 1)
                b = &CursorControls::GetButton(m, i);
            else if(a == 2)
                b = &EditorControls::GetButton(e, i);
            else
            {
                b = &activate;
                *b = false;
            }

            if(p->m_controllerProfile && this->m_devices->ctrl)
            {
                s_updateCtrlKey(this->m_devices->ctrl, *b, key);
                s_updateCtrlKey(this->m_devices->ctrl, *b, key2);
            }
            else
            {
                s_updateJoystickKey(this->m_devices->joy, *b, key);
                s_updateJoystickKey(this->m_devices->joy, *b, key2);
            }

            if(a == 3 && *b)
            {
                if(this->m_hotkey_okay)
                    Hotkeys::Activate(i);
                hotkey_okay = false;
            }
        }
    }
    this->m_hotkey_okay = hotkey_okay;

    double cursor[4];
    double* const scroll[4] = {&e.ScrollUp, &e.ScrollDown, &e.ScrollLeft, &e.ScrollRight};

    if(p->m_controllerProfile && this->m_devices->ctrl)
    {
        for(int i = 0; i < 4; i++)
        {
            cursor[i] = 0.;
            s_updateCtrlAnalogue(this->m_devices->ctrl, cursor[i], p->m_cursor_keys[i]);
            s_updateCtrlAnalogue(this->m_devices->ctrl, cursor[i], p->m_cursor_keys2[i]);
            s_updateCtrlAnalogue(this->m_devices->ctrl, *scroll[i], p->m_editor_keys[i]);
            s_updateCtrlAnalogue(this->m_devices->ctrl, *scroll[i], p->m_editor_keys2[i]);
        }
    }
    else
    {
        for(int i = 0; i < 4; i++)
        {
            cursor[i] = 0.;
            s_updateJoystickAnalogue(this->m_devices->joy, cursor[i], p->m_cursor_keys[i]);
            s_updateJoystickAnalogue(this->m_devices->joy, cursor[i], p->m_cursor_keys2[i]);
            s_updateJoystickAnalogue(this->m_devices->joy, *scroll[i], p->m_editor_keys[i]);
            s_updateJoystickAnalogue(this->m_devices->joy, *scroll[i], p->m_editor_keys2[i]);
        }
    }

    // Cursor control (UDLR)
    if(cursor[0] || cursor[1] || cursor[2] || cursor[3])
    {
        if(m.X < 0)
            m.X = ScreenW/2;
        if(m.Y < 0)
            m.Y = ScreenH/2;
        m.X += (cursor[3] - cursor[2])*16.;
        m.Y += (cursor[1] - cursor[0])*16.;
        if(m.X < 0)
            m.X = 0;
        else if(m.X >= ScreenW)
            m.X = ScreenW-1;
        if(m.Y < 0)
            m.Y = 0;
        else if(m.Y >= ScreenH)
            m.Y = ScreenH-1;
        m.Move = true;
    }

    return true;
}

void InputMethod_Joystick::Rumble(int ms, float strength)
{
    if(!this->m_devices)
        return;

    if(this->m_devices->haptic)
    {
        pLogDebug("Trying to use SDL haptic rumble: %dms %f", ms, strength);
        if(SDL_HapticRumblePlay(this->m_devices->haptic, strength, ms) == 0)
            return;
    }

    int intStrength = (int)(0xFFFF * strength + 0.5f);

    if(intStrength > 0xFFFF)
        intStrength = 0xFFFF;

    if(intStrength < 0)
        intStrength = 0;

#if SDL_VERSION_ATLEAST(2, 0, 12)
    if(this->m_devices->joy)
    {
        pLogDebug("Trying to use SDL joystick rumble: %dms %f", ms, strength);
        SDL_JoystickRumble(this->m_devices->joy, intStrength, intStrength, ms);
    }
#endif
}

/*====================================================*\
|| implementation for InputMethodProfile_Joystick     ||
\*====================================================*/

// the job of this function is to initialize the class in a consistent state
InputMethodProfile_Joystick::InputMethodProfile_Joystick()
{
    this->InitAsController();
}

void InputMethodProfile_Joystick::InitAsJoystick()
{
    this->m_controllerProfile = false;
    this->m_keys[PlayerControls::Buttons::Up].assign(KM_Key::JoyHat, 0, SDL_HAT_UP);
    this->m_keys[PlayerControls::Buttons::Down].assign(KM_Key::JoyHat, 0, SDL_HAT_DOWN);
    this->m_keys[PlayerControls::Buttons::Left].assign(KM_Key::JoyHat, 0, SDL_HAT_LEFT);
    this->m_keys[PlayerControls::Buttons::Right].assign(KM_Key::JoyHat, 0, SDL_HAT_RIGHT);
    this->m_keys[PlayerControls::Buttons::Jump].assign(KM_Key::JoyButton, 0, 1);
    this->m_keys[PlayerControls::Buttons::AltJump].assign(KM_Key::JoyButton, 1, 1);
    this->m_keys[PlayerControls::Buttons::Run].assign(KM_Key::JoyButton, 2, 1);
    this->m_keys[PlayerControls::Buttons::AltRun].assign(KM_Key::JoyButton, 3, 1);
    this->m_keys[PlayerControls::Buttons::Drop].assign(KM_Key::JoyButton, 6, 1);
    this->m_keys[PlayerControls::Buttons::Start].assign(KM_Key::JoyButton, 7, 1);

    // clear all of them, then fill in some of them
    for(size_t i = 0; i < PlayerControls::n_buttons; i++)
    {
        this->m_keys2[i].assign(KM_Key::NoControl, -1, -1);
    }
    this->m_keys2[PlayerControls::Buttons::Up].assign(KM_Key::JoyAxis, 1, -1);
    this->m_keys2[PlayerControls::Buttons::Down].assign(KM_Key::JoyAxis, 1, 1);
    this->m_keys2[PlayerControls::Buttons::Left].assign(KM_Key::JoyAxis, 0, -1);
    this->m_keys2[PlayerControls::Buttons::Right].assign(KM_Key::JoyAxis, 0, 1);

    // clear all of the non-standard controls, then fill in some of them
    for(size_t i = 0; i < CursorControls::n_buttons; i++)
    {
        this->m_cursor_keys[i].assign(KM_Key::NoControl, -1, -1);
        this->m_cursor_keys2[i].assign(KM_Key::NoControl, -1, -1);
    }
    for(size_t i = 0; i < EditorControls::n_buttons; i++)
    {
        this->m_editor_keys[i].assign(KM_Key::NoControl, -1, -1);
        this->m_editor_keys2[i].assign(KM_Key::NoControl, -1, -1);
    }
    for(size_t i = 0; i < Hotkeys::n_buttons; i++)
    {
        this->m_hotkeys[i].assign(KM_Key::NoControl, -1, -1);
        this->m_hotkeys2[i].assign(KM_Key::NoControl, -1, -1);
    }

    this->m_cursor_keys[CursorControls::Buttons::CursorUp].assign(KM_Key::JoyAxis, 3, -1);
    this->m_cursor_keys[CursorControls::Buttons::CursorDown].assign(KM_Key::JoyAxis, 3, 1);
    this->m_cursor_keys[CursorControls::Buttons::CursorLeft].assign(KM_Key::JoyAxis, 2, -1);
    this->m_cursor_keys[CursorControls::Buttons::CursorRight].assign(KM_Key::JoyAxis, 2, 1);
    this->m_cursor_keys[CursorControls::Buttons::Primary].assign(KM_Key::JoyAxis, 4, 1);
    this->m_cursor_keys[CursorControls::Buttons::Secondary].assign(KM_Key::JoyAxis, 5, 1);
}

void InputMethodProfile_Joystick::InitAsController()
{
    this->m_controllerProfile = true;

    this->m_keys[PlayerControls::Buttons::Up].assign(KM_Key::CtrlButton, SDL_CONTROLLER_BUTTON_DPAD_UP, 1);
    this->m_keys[PlayerControls::Buttons::Down].assign(KM_Key::CtrlButton, SDL_CONTROLLER_BUTTON_DPAD_DOWN, 1);
    this->m_keys[PlayerControls::Buttons::Left].assign(KM_Key::CtrlButton, SDL_CONTROLLER_BUTTON_DPAD_LEFT, 1);
    this->m_keys[PlayerControls::Buttons::Right].assign(KM_Key::CtrlButton, SDL_CONTROLLER_BUTTON_DPAD_RIGHT, 1);
    this->m_keys[PlayerControls::Buttons::Jump].assign(KM_Key::CtrlButton, SDL_CONTROLLER_BUTTON_A, 1);
    this->m_keys[PlayerControls::Buttons::AltJump].assign(KM_Key::CtrlButton, SDL_CONTROLLER_BUTTON_B, 1);
    this->m_keys[PlayerControls::Buttons::Run].assign(KM_Key::CtrlButton, SDL_CONTROLLER_BUTTON_X, 1);
    this->m_keys[PlayerControls::Buttons::AltRun].assign(KM_Key::CtrlButton, SDL_CONTROLLER_BUTTON_Y, 1);
    this->m_keys[PlayerControls::Buttons::Drop].assign(KM_Key::CtrlButton, SDL_CONTROLLER_BUTTON_BACK, 1);
    this->m_keys[PlayerControls::Buttons::Start].assign(KM_Key::CtrlButton, SDL_CONTROLLER_BUTTON_START, 1);

    // clear all of them, then fill in some of them
    for(size_t i = 0; i < PlayerControls::n_buttons; i++)
    {
        this->m_keys2[i].assign(KM_Key::NoControl, -1, -1);
    }
    this->m_keys2[PlayerControls::Buttons::Up].assign(KM_Key::CtrlAxis, SDL_CONTROLLER_AXIS_LEFTY, -1);
    this->m_keys2[PlayerControls::Buttons::Down].assign(KM_Key::CtrlAxis, SDL_CONTROLLER_AXIS_LEFTY, 1);
    this->m_keys2[PlayerControls::Buttons::Left].assign(KM_Key::CtrlAxis, SDL_CONTROLLER_AXIS_LEFTX, -1);
    this->m_keys2[PlayerControls::Buttons::Right].assign(KM_Key::CtrlAxis, SDL_CONTROLLER_AXIS_LEFTX, 1);

    // clear all of the non-standard controls, then fill in some of them
    for(size_t i = 0; i < CursorControls::n_buttons; i++)
    {
        this->m_cursor_keys[i].assign(KM_Key::NoControl, -1, -1);
        this->m_cursor_keys2[i].assign(KM_Key::NoControl, -1, -1);
    }
    for(size_t i = 0; i < EditorControls::n_buttons; i++)
    {
        this->m_editor_keys[i].assign(KM_Key::NoControl, -1, -1);
        this->m_editor_keys2[i].assign(KM_Key::NoControl, -1, -1);
    }
    for(size_t i = 0; i < Hotkeys::n_buttons; i++)
    {
        this->m_hotkeys[i].assign(KM_Key::NoControl, -1, -1);
        this->m_hotkeys2[i].assign(KM_Key::NoControl, -1, -1);
    }

    this->m_cursor_keys[CursorControls::Buttons::CursorUp].assign(KM_Key::CtrlAxis, SDL_CONTROLLER_AXIS_RIGHTY, -1);
    this->m_cursor_keys[CursorControls::Buttons::CursorDown].assign(KM_Key::CtrlAxis, SDL_CONTROLLER_AXIS_RIGHTY, 1);
    this->m_cursor_keys[CursorControls::Buttons::CursorLeft].assign(KM_Key::CtrlAxis, SDL_CONTROLLER_AXIS_RIGHTX, -1);
    this->m_cursor_keys[CursorControls::Buttons::CursorRight].assign(KM_Key::CtrlAxis, SDL_CONTROLLER_AXIS_RIGHTX, 1);
    this->m_cursor_keys[CursorControls::Buttons::Primary].assign(KM_Key::CtrlAxis, SDL_CONTROLLER_AXIS_TRIGGERRIGHT, 1);
    this->m_cursor_keys[CursorControls::Buttons::Secondary].assign(KM_Key::CtrlAxis, SDL_CONTROLLER_AXIS_TRIGGERLEFT, 1);
}

void InputMethodProfile_Joystick::ExpandAsJoystick()
{
    this->m_legacyProfile = false;
    this->m_controllerProfile = false;

    // controller keys are stored in m_keys, joystick in m_keys2
    // copy joystick keys from m_keys2 to m_keys
    this->m_keys[PlayerControls::Buttons::Up] = this->m_keys2[PlayerControls::Buttons::Up];
    this->m_keys[PlayerControls::Buttons::Down] = this->m_keys2[PlayerControls::Buttons::Down];
    this->m_keys[PlayerControls::Buttons::Left] = this->m_keys2[PlayerControls::Buttons::Left];
    this->m_keys[PlayerControls::Buttons::Right] = this->m_keys2[PlayerControls::Buttons::Right];
    this->m_keys[PlayerControls::Buttons::Jump] = this->m_keys2[PlayerControls::Buttons::Jump];
    this->m_keys[PlayerControls::Buttons::AltJump] = this->m_keys2[PlayerControls::Buttons::AltJump];
    this->m_keys[PlayerControls::Buttons::Run] = this->m_keys2[PlayerControls::Buttons::Run];
    this->m_keys[PlayerControls::Buttons::AltRun] = this->m_keys2[PlayerControls::Buttons::AltRun];
    this->m_keys[PlayerControls::Buttons::Drop] = this->m_keys2[PlayerControls::Buttons::Drop];
    this->m_keys[PlayerControls::Buttons::Start] = this->m_keys2[PlayerControls::Buttons::Start];

    // clear all of the old joystick keys that have been copied over
    for(size_t i = 0; i < PlayerControls::n_buttons; i++)
    {
        this->m_keys2[i].assign(KM_Key::NoControl, -1, -1);
    }

    // this is needed because using the LStick was not configurable before
    this->m_keys2[PlayerControls::Buttons::Up].assign(KM_Key::JoyAxis, 1, -1);
    this->m_keys2[PlayerControls::Buttons::Down].assign(KM_Key::JoyAxis, 1, 1);
    this->m_keys2[PlayerControls::Buttons::Left].assign(KM_Key::JoyAxis, 0, -1);
    this->m_keys2[PlayerControls::Buttons::Right].assign(KM_Key::JoyAxis, 0, 1);

    // clear all of the non-standard controllers, then fill in some of them
    for(size_t i = 0; i < CursorControls::n_buttons; i++)
    {
        this->m_cursor_keys[i].assign(KM_Key::NoControl, -1, -1);
        this->m_cursor_keys2[i].assign(KM_Key::NoControl, -1, -1);
    }
    for(size_t i = 0; i < EditorControls::n_buttons; i++)
    {
        this->m_editor_keys[i].assign(KM_Key::NoControl, -1, -1);
        this->m_editor_keys2[i].assign(KM_Key::NoControl, -1, -1);
    }
    for(size_t i = 0; i < Hotkeys::n_buttons; i++)
    {
        this->m_hotkeys[i].assign(KM_Key::NoControl, -1, -1);
        this->m_hotkeys2[i].assign(KM_Key::NoControl, -1, -1);
    }

    this->m_cursor_keys[CursorControls::Buttons::CursorUp].assign(KM_Key::JoyAxis, 3, -1);
    this->m_cursor_keys[CursorControls::Buttons::CursorDown].assign(KM_Key::JoyAxis, 3, 1);
    this->m_cursor_keys[CursorControls::Buttons::CursorLeft].assign(KM_Key::JoyAxis, 2, -1);
    this->m_cursor_keys[CursorControls::Buttons::CursorRight].assign(KM_Key::JoyAxis, 2, 1);
}

void InputMethodProfile_Joystick::ExpandAsController()
{
    this->m_legacyProfile = false;
    this->m_controllerProfile = true;

    // controller keys are stored in m_keys, joystick in m_keys2
    // no action needed for m_keys

    // clear all of the old joystick keys, then fill in some of them
    for(size_t i = 0; i < PlayerControls::n_buttons; i++)
    {
        this->m_keys2[i].assign(KM_Key::NoControl, -1, -1);
    }

    // this is needed because using the LStick was not configurable before
    this->m_keys2[PlayerControls::Buttons::Up].assign(KM_Key::CtrlAxis, SDL_CONTROLLER_AXIS_LEFTY, -1);
    this->m_keys2[PlayerControls::Buttons::Down].assign(KM_Key::CtrlAxis, SDL_CONTROLLER_AXIS_LEFTY, 1);
    this->m_keys2[PlayerControls::Buttons::Left].assign(KM_Key::CtrlAxis, SDL_CONTROLLER_AXIS_LEFTX, -1);
    this->m_keys2[PlayerControls::Buttons::Right].assign(KM_Key::CtrlAxis, SDL_CONTROLLER_AXIS_LEFTX, 1);

    // clear all of the non-standard controllers, then fill in some of them
    for(size_t i = 0; i < CursorControls::n_buttons; i++)
    {
        this->m_cursor_keys[i].assign(KM_Key::NoControl, -1, -1);
        this->m_cursor_keys2[i].assign(KM_Key::NoControl, -1, -1);
    }
    for(size_t i = 0; i < EditorControls::n_buttons; i++)
    {
        this->m_editor_keys[i].assign(KM_Key::NoControl, -1, -1);
        this->m_editor_keys2[i].assign(KM_Key::NoControl, -1, -1);
    }
    for(size_t i = 0; i < Hotkeys::n_buttons; i++)
    {
        this->m_hotkeys[i].assign(KM_Key::NoControl, -1, -1);
        this->m_hotkeys2[i].assign(KM_Key::NoControl, -1, -1);
    }

    this->m_cursor_keys[CursorControls::Buttons::CursorUp].assign(KM_Key::CtrlAxis, SDL_CONTROLLER_AXIS_RIGHTY, -1);
    this->m_cursor_keys[CursorControls::Buttons::CursorDown].assign(KM_Key::CtrlAxis, SDL_CONTROLLER_AXIS_RIGHTY, 1);
    this->m_cursor_keys[CursorControls::Buttons::CursorLeft].assign(KM_Key::CtrlAxis, SDL_CONTROLLER_AXIS_RIGHTX, -1);
    this->m_cursor_keys[CursorControls::Buttons::CursorRight].assign(KM_Key::CtrlAxis, SDL_CONTROLLER_AXIS_RIGHTX, 1);
}

bool InputMethodProfile_Joystick::PollPrimaryButton(ControlsClass c, size_t i)
{
    // note: m_canPoll is initialized to false
    InputMethodType_Joystick* t = dynamic_cast<InputMethodType_Joystick*>(this->Type);
    if(!t)
        return false;

    KM_Key key;
    if(this->m_controllerProfile)
        key = t->PollControllerKeyAll();
    else
        key = t->PollJoystickKeyAll();

    // if didn't find any key, allow poll in future but return false
    if(key.type == KM_Key::NoControl)
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
    KM_Key* keys;
    KM_Key* keys2;
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
        {
            keys2[j] = KM_Key();
        }
        else if(i != j && keys[j] == key)
        {
            if(keys2[j].type != KM_Key::NoControl)
            {
                keys[j] = keys2[j];
                keys2[j] = KM_Key();
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

bool InputMethodProfile_Joystick::PollSecondaryButton(ControlsClass c, size_t i)
{
    // note: m_canPoll is initialized to false
    InputMethodType_Joystick* t = dynamic_cast<InputMethodType_Joystick*>(this->Type);
    if(!t)
        return false;

    KM_Key key;
    if(this->m_controllerProfile)
        key = t->PollControllerKeyAll();
    else
        key = t->PollJoystickKeyAll();

    // if didn't find any key, allow poll in future but return false
    if(key.type == KM_Key::NoControl)
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
    KM_Key* keys;
    KM_Key* keys2;
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
        // BAD!
        return true;
    }

    // minor switching algorithm to ensure that every button always has at least one key
    // and no button ever has a non-unique key

    // if the current button's primary key is the new key,
    //     delete its secondary key instead of setting it.
    if(keys[i] == key)
    {
        keys2[i] = KM_Key();
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
            keys2[j] = KM_Key();
        }
        else if(i != j && keys[j] == key)
        {
            if(keys2[j].type != KM_Key::NoControl)
            {
                keys[j] = keys2[j];
                keys2[j] = KM_Key();
            }
            else if(keys2[i].type != KM_Key::NoControl)
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

bool InputMethodProfile_Joystick::DeletePrimaryButton(ControlsClass c, size_t i)
{
    // resolve the particular primary and secondary key arrays
    KM_Key* keys;
    KM_Key* keys2;
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

    if(keys2[i].type != KM_Key::NoControl)
    {
        keys[i] = keys2[i];
        keys2[i] = KM_Key();
        return true;
    }
    if(c == ControlsClass::Player)
        return false;
    if(keys[i].type != KM_Key::NoControl)
    {
        keys[i] = KM_Key();
        return true;
    }
    return false;
}

bool InputMethodProfile_Joystick::DeleteSecondaryButton(ControlsClass c, size_t i)
{
    // resolve the particular primary and secondary key arrays
    KM_Key* keys2;
    if(c == ControlsClass::Player)
        keys2 = this->m_keys2;
    else if(c == ControlsClass::Cursor)
        keys2 = this->m_cursor_keys2;
    else if(c == ControlsClass::Editor)
        keys2 = this->m_editor_keys2;
    else if(c == ControlsClass::Hotkey)
        keys2 = this->m_hotkeys2;
    else
    {
        // BAD!
        return false;
    }

    if(keys2[i].type != KM_Key::NoControl)
    {
        keys2[i] = KM_Key();
        return true;
    }
    return false;
}

static char s_buttonNameBuffer[16] = "XXXXXXXXXXXXXXX";

static const char* s_nameButton(const KM_Key& k)
{
    switch(k.type)
    {
    case KM_Key::NoControl:
        return "NONE";
        break;
    case KM_Key::CtrlButton:
        return SDL_GameControllerGetStringForButton((SDL_GameControllerButton)k.id);
        break;
    case KM_Key::CtrlAxis:
        if(k.val < 0)
            snprintf(s_buttonNameBuffer, 15, "%s-", SDL_GameControllerGetStringForAxis((SDL_GameControllerAxis)k.id));
        else
            snprintf(s_buttonNameBuffer, 15, "%s+", SDL_GameControllerGetStringForAxis((SDL_GameControllerAxis)k.id));
        break;
    case KM_Key::JoyButton:
        snprintf(s_buttonNameBuffer, 15, "BUT %d", k.id);
        break;
    case KM_Key::JoyAxis:
        if(k.val < 0)
            snprintf(s_buttonNameBuffer, 15, "AX %d-", k.id);
        else
            snprintf(s_buttonNameBuffer, 15, "AX %d+", k.id);
        break;
    case KM_Key::JoyBallX:
        if(k.val < 0)
            snprintf(s_buttonNameBuffer, 15, "BALL %dX-", k.id);
        else
            snprintf(s_buttonNameBuffer, 15, "BALL %dX+", k.id);
        break;
    case KM_Key::JoyBallY:
        if(k.val < 0)
            snprintf(s_buttonNameBuffer, 15, "BALL %dY-", k.id);
        else
            snprintf(s_buttonNameBuffer, 15, "BALL %dY+", k.id);
        break;
    case KM_Key::JoyHat:
        if(k.val == SDL_HAT_UP)
            snprintf(s_buttonNameBuffer, 15, "HAT %dU", k.id);
        else if(k.val == SDL_HAT_DOWN)
            snprintf(s_buttonNameBuffer, 15, "HAT %dD", k.id);
        else if(k.val == SDL_HAT_LEFT)
            snprintf(s_buttonNameBuffer, 15, "HAT %dL", k.id);
        else if(k.val == SDL_HAT_RIGHT)
            snprintf(s_buttonNameBuffer, 15, "HAT %dR", k.id);
        else
            snprintf(s_buttonNameBuffer, 15, "HAT %d %d", k.id, k.val);
        break;
    default:
        return "INVALID";
        break;
    }

    return s_buttonNameBuffer;
}

const char* InputMethodProfile_Joystick::NamePrimaryButton(ControlsClass c, size_t i)
{
    KM_Key* keys;
    if(c == ControlsClass::Player)
        keys = this->m_keys;
    else if(c == ControlsClass::Cursor)
        keys = this->m_cursor_keys;
    else if(c == ControlsClass::Editor)
        keys = this->m_editor_keys;
    else if(c == ControlsClass::Hotkey)
        keys = this->m_hotkeys;
    else
        return nullptr;

    return s_nameButton(keys[i]);
}

const char* InputMethodProfile_Joystick::NameSecondaryButton(ControlsClass c, size_t i)
{
    KM_Key* keys2;
    if(c == ControlsClass::Player)
        keys2 = this->m_keys2;
    else if(c == ControlsClass::Cursor)
        keys2 = this->m_cursor_keys2;
    else if(c == ControlsClass::Editor)
        keys2 = this->m_editor_keys2;
    else if(c == ControlsClass::Hotkey)
        keys2 = this->m_hotkeys2;
    else
        return nullptr;

    return s_nameButton(keys2[i]);
}

void InputMethodProfile_Joystick::SaveConfig(IniProcessing* ctl)
{
    std::string name;
    for(int a = 0; a < 4; a++)
    {
        KM_Key* keys;
        KM_Key* keys2;
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
            size_t orig_l = name.size();

            ctl->setValue(name.replace(orig_l, std::string::npos, "-type").c_str(),
                keys[i].type);
            ctl->setValue(name.replace(orig_l, std::string::npos, "-id").c_str(),
                keys[i].id);
            ctl->setValue(name.replace(orig_l, std::string::npos, "-val").c_str(),
                keys[i].val);

            ctl->setValue(name.replace(orig_l, std::string::npos, "2-type").c_str(),
                keys2[i].type);
            ctl->setValue(name.replace(orig_l, std::string::npos, "2-id").c_str(),
                keys2[i].id);
            ctl->setValue(name.replace(orig_l, std::string::npos, "2-val").c_str(),
                keys2[i].val);
        }
    }

    ctl->setValue("enable-rumble", this->m_rumbleEnabled);
    ctl->setValue("old-joystick", !this->m_controllerProfile);
}

void InputMethodProfile_Joystick::LoadConfig(IniProcessing* ctl)
{
    std::string name;
    for(int a = 0; a < 4; a++)
    {
        KM_Key* keys;
        KM_Key* keys2;
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
            size_t orig_l = name.size();

            ctl->read(name.replace(orig_l, std::string::npos, "-type").c_str(),
                keys[i].type, keys[i].type);
            ctl->read(name.replace(orig_l, std::string::npos, "-id").c_str(),
                keys[i].id, keys[i].id);
            ctl->read(name.replace(orig_l, std::string::npos, "-val").c_str(),
                keys[i].val, keys[i].val);

            ctl->read(name.replace(orig_l, std::string::npos, "2-type").c_str(),
                keys2[i].type, keys2[i].type);
            ctl->read(name.replace(orig_l, std::string::npos, "2-id").c_str(),
                keys2[i].id, keys2[i].id);
            ctl->read(name.replace(orig_l, std::string::npos, "2-val").c_str(),
                keys2[i].val, keys2[i].val);
        }
    }


    ctl->read("enable-rumble", this->m_rumbleEnabled, true);

    bool legacyProfile;
    ctl->read("old-joystick", legacyProfile, false);
    this->m_controllerProfile = !legacyProfile;
}

void InputMethodProfile_Joystick::SaveConfig_Legacy(IniProcessing* ctl)
{
    std::string name;
    for(size_t i = 0; i < PlayerControls::n_buttons; i++)
    {
        name = PlayerControls::GetButtonName_INI(i);
        size_t orig_l = name.size();

        ctl->setValue(name.replace(orig_l, std::string::npos, "-ctrl-type").c_str(),
            this->m_keys[i].type);
        ctl->setValue(name.replace(orig_l, std::string::npos, "-ctrl-id").c_str(),
            this->m_keys[i].id);
        ctl->setValue(name.replace(orig_l, std::string::npos, "-ctrl-val").c_str(),
            this->m_keys[i].val);

        ctl->setValue(name.replace(orig_l, std::string::npos, "-type").c_str(),
            this->m_keys2[i].type);
        ctl->setValue(name.replace(orig_l, std::string::npos, "-id").c_str(),
            this->m_keys2[i].id);
        ctl->setValue(name.replace(orig_l, std::string::npos, "-val").c_str(),
            this->m_keys2[i].val);
    }
}

void InputMethodProfile_Joystick::LoadConfig_Legacy(IniProcessing* ctl)
{
    std::string name;
    for(size_t i = 0; i < PlayerControls::n_buttons; i++)
    {
        name = PlayerControls::GetButtonName_INI(i);
        size_t orig_l = name.size();

        ctl->read(name.replace(orig_l, std::string::npos, "-ctrl-type").c_str(),
            this->m_keys[i].type, this->m_keys[i].type);
        ctl->read(name.replace(orig_l, std::string::npos, "-ctrl-id").c_str(),
            this->m_keys[i].id, this->m_keys[i].id);
        ctl->read(name.replace(orig_l, std::string::npos, "-ctrl-val").c_str(),
            this->m_keys[i].val, this->m_keys[i].val);

        ctl->read(name.replace(orig_l, std::string::npos, "-type").c_str(),
            this->m_keys2[i].type, this->m_keys2[i].type);
        ctl->read(name.replace(orig_l, std::string::npos, "-id").c_str(),
            this->m_keys2[i].id, this->m_keys2[i].id);
        ctl->read(name.replace(orig_l, std::string::npos, "-val").c_str(),
            this->m_keys2[i].val, this->m_keys2[i].val);
    }

    this->m_legacyProfile = true;
}

// How many per-type special options are there?
size_t InputMethodProfile_Joystick::GetSpecialOptionCount()
{
    return 1;
}
// Methods to manage per-profile options
// It is guaranteed that none of these will be called if
// GetOptionCount() returns 0.
// get a char* describing the option
const char* InputMethodProfile_Joystick::GetOptionName(size_t i)
{
    if(i == 0)
        return "RUMBLE";
    else
        return nullptr;
}
// get a char* describing the current option value
// must be allocated in static or instance memory
// WILL NOT be freed
const char* InputMethodProfile_Joystick::GetOptionValue(size_t i)
{
    if(i == 0)
    {
        if(this->m_rumbleEnabled)
            return "ENABLED";
        else
            return "DISABLED";
    }
    else
        return nullptr;
}
// called when A is pressed; allowed to interrupt main game loop
bool InputMethodProfile_Joystick::OptionChange(size_t i)
{
    if(i == 0) // rumble
    {
        this->m_rumbleEnabled = !this->m_rumbleEnabled;
        for(InputMethod* m : g_InputMethods)
        {
            if(!m || m->Profile != this)
                continue;
            if(this->m_rumbleEnabled)
                m->Rumble(200, .5);
        }
        return true;
    }
    else
    {
        return false;
    }
}
// called when left is pressed
bool InputMethodProfile_Joystick::OptionRotateLeft(size_t i)
{
    return this->OptionChange(i);
}
// called when right is pressed
bool InputMethodProfile_Joystick::OptionRotateRight(size_t i)
{
    return this->OptionChange(i);
}

/*====================================================*\
|| implementation for InputMethodType_Joystick        ||
\*====================================================*/

InputMethodProfile* InputMethodType_Joystick::AllocateProfile() noexcept
{
    return (InputMethodProfile*) new(std::nothrow) InputMethodProfile_Joystick;
}

InputMethodType_Joystick::InputMethodType_Joystick()
{
    this->Name = "Joystick";

    SDL_JoystickEventState(SDL_ENABLE);
    int num = SDL_NumJoysticks();

    for(int i = 0; i < num; ++i)
    {
        this->OpenJoystick(i);
    }
}

void InputMethodType_Joystick::UpdateControlsPre() {}
void InputMethodType_Joystick::UpdateControlsPost() {}

InputMethod* InputMethodType_Joystick::Poll(const std::vector<InputMethod*>& active_methods) noexcept
{
    JoystickDevices* active_joystick = nullptr;

    for(std::pair<const int, JoystickDevices*>& p : this->m_availableJoysticks)
    {
        SDL_Joystick* joy = p.second->joy;
        bool duplicate = false;
        for(InputMethod* method : active_methods)
        {
            InputMethod_Joystick* m = dynamic_cast<InputMethod_Joystick*>(method);
            if(!m)
                continue;
            if(m->m_devices == p.second)
            {
                duplicate = true;
                break;
            }
        }

        // don't want any duplicates!
        if(duplicate)
            continue;

        KM_Key k;
        s_bindJoystickKey(joy, k);
        if(k.type != KM_Key::NoControl)
        {
            active_joystick = p.second;
            break;
        }
    }

    // if didn't find any joystick, allow poll in future but return false right now
    if(!active_joystick)
    {
        this->m_canPoll = true;
        return nullptr;
    }

    // if poll not allowed, return nullptr
    if(!this->m_canPoll)
        return nullptr;

    // we're going to create a new joystick!
    // reset canPoll for next time
    this->m_canPoll = false;

    InputMethod_Joystick* method = new(std::nothrow) InputMethod_Joystick;

    // alloc failed :(
    if(!method)
        return nullptr;

    method->Type = this;
    method->Name = SDL_JoystickName(active_joystick->joy);
    method->m_devices = active_joystick;

    // I've found that truncation to 10 characters is
    // needed for this to display well with multiplayer
    if(method->Name.size() > 10)
        method->Name.resize(10);

    // find controller profile...!

    int my_index = 0;
    for(const InputMethod* method : active_methods)
    {
        if(!method)
        {
            break;
        }
        my_index ++;
    }

    // 1. cleverly look for profile by GUID and player...
    std::unordered_map<std::string, InputMethodProfile*>::iterator found
        = this->m_lastProfileByGUID.find(std::to_string(my_index+1)+"-"+active_joystick->guid);

    // ...then by just GUID.
    if(found == this->m_lastProfileByGUID.end())
    {
        found = this->m_lastProfileByGUID.find(active_joystick->guid);
    }

    if(found != this->m_lastProfileByGUID.end())
    {
        method->Profile = found->second;
        InputMethodProfile_Joystick* profile = dynamic_cast<InputMethodProfile_Joystick*>(found->second);

        if(profile->m_legacyProfile)
        {
            // expanding a legacy profile based on information from controller
            profile->Name = method->Name + " P" + std::to_string(my_index+1);
            if(active_joystick->ctrl)
            {
                profile->ExpandAsController();
            }
            else
            {
                profile->ExpandAsJoystick();
            }
            this->m_profiles.push_back(method->Profile);
        }
    }

    // 2. find first profile appropriate to the controller-ness of the controller
    if(!method->Profile)
    {
        for(InputMethodProfile* p_ : this->m_profiles)
        {
            InputMethodProfile_Joystick* p = dynamic_cast<InputMethodProfile_Joystick*>(p_);
            if(!p)
                continue;
            if((active_joystick->ctrl && p->m_controllerProfile)
                || (!active_joystick->ctrl && !p->m_controllerProfile))
            {
                method->Profile = p_;
                break;
            }
        }

        // 3. make appropriate new profile (note that allocs could fail, that will be cleaned up later)
        if(!method->Profile && active_joystick->ctrl)
        {
            method->Profile = this->AddProfile();
        }
        else if(!method->Profile)
        {
            method->Profile = this->AddOldJoystickProfile();
        }
    }

    return (InputMethod*)method;
}

/*-----------------------*\
|| CUSTOM METHODS        ||
\*-----------------------*/


KM_Key InputMethodType_Joystick::PollJoystickKeyAll()
{
    KM_Key k;

    for(std::pair<const int, JoystickDevices*>& p : this->m_availableJoysticks)
    {
        if(p.second->joy)
            s_bindJoystickKey(p.second->joy, k);
        if(k.type != KM_Key::NoControl)
            break;
    }

    return k;
}

KM_Key InputMethodType_Joystick::PollControllerKeyAll()
{
    KM_Key k;

    for(std::pair<const int, JoystickDevices*>& p : this->m_availableJoysticks)
    {
        if(p.second->ctrl)
            s_bindControllerKey(p.second->ctrl, k);
        if(k.type != KM_Key::NoControl)
            break;
    }

    return k;
}

/*-----------------------*\
|| CUSTOM METHODS        ||
\*-----------------------*/

bool InputMethodType_Joystick::OpenJoystick(int joystick_index)
{
    pLogDebug("Opening joystick id %d...", joystick_index);
    SDL_Joystick* joy = SDL_JoystickOpen(joystick_index);
    if(!joy)
    {
        pLogDebug("  could not open.");
        return false;
    }
    pLogDebug("  successfully opened!");
    pLogDebug("  Name: %s", SDL_JoystickName(joy));
    std::string guid;
    // explicitly scoping this buffer
    {
        SDL_JoystickGUID guid_bytes = SDL_JoystickGetGUID(joy);
        char guid_chars[35];
        SDL_JoystickGetGUIDString(guid_bytes, guid_chars, 35);
        pLogDebug("  GUID: %s", guid_chars);
        guid = guid_chars;
    }
    pLogDebug("  Number of Axes: %d", SDL_JoystickNumAxes(joy));
    pLogDebug("  Number of Buttons: %d", SDL_JoystickNumButtons(joy));
    pLogDebug("  Number of Balls: %d", SDL_JoystickNumBalls(joy));
    JoystickDevices* devices = new(std::nothrow) JoystickDevices;
    if(!devices)
    {
        SDL_JoystickClose(joy);
        pLogDebug("  could not allocate devices struct (OOM).");
        return false;
    }

    devices->joy = joy;
    devices->guid = guid;
    if(SDL_IsGameController(joystick_index))
    {
        pLogDebug("  claims to be a game controller...");
        devices->ctrl = SDL_GameControllerOpen(joystick_index);
        if(devices->ctrl)
        {
            pLogDebug("    successfully opened game controller!");
        }
        else
        {
            pLogDebug("    could not open.");
        }
    }
    else
    {
        pLogDebug("  is not a game controller.");
    }
    if(SDL_JoystickIsHaptic(joy))
    {
        pLogDebug("  claims to support haptic...");
        devices->haptic = SDL_HapticOpenFromJoystick(joy);
        if(!devices->haptic)
        {
            pLogDebug("    could not open haptic!");
        }
        else if(!SDL_HapticRumbleSupported(devices->haptic) || SDL_HapticRumbleInit(devices->haptic) != 0)
        {
            pLogDebug("    could not init haptic rumble!");
            SDL_HapticClose(devices->haptic);
            devices->haptic = nullptr;
        }
        else
        {
            pLogDebug("    successfully initialized haptic rumble!");
        }
    }
    else
    {
        pLogDebug("  does not support haptic.");
    }
    int id = SDL_JoystickInstanceID(joy);
    pLogDebug("  received ID %d. adding to poll list!", id);

    this->m_availableJoysticks.emplace(id, devices);

    return true;
}

bool InputMethodType_Joystick::CloseJoystick(int instance_id)
{
    pLogDebug("joystick with ID %d removed.", instance_id);
    std::unordered_map<int, JoystickDevices*>::iterator found
        = this->m_availableJoysticks.find(instance_id);
    if(found == this->m_availableJoysticks.end())
    {
        pLogDebug("  could not find!");
        return false;
    }

    JoystickDevices* devices = found->second;

    for(InputMethod* method : g_InputMethods)
    {
        InputMethod_Joystick* m = dynamic_cast<InputMethod_Joystick*>(method);
        if(!m)
            continue;
        if(m->m_devices == devices)
        {
            pLogDebug("  unset m_devices of InputMethod '%s' using Profile '%s'.", m->Name.c_str(), m->Profile->Name.c_str());
            m->m_devices = nullptr;
        }
    }

    if(devices->haptic)
        SDL_HapticClose(devices->haptic);
    if(devices->ctrl)
        SDL_GameControllerClose(devices->ctrl);
    SDL_JoystickClose(devices->joy);
    delete devices;

    this->m_availableJoysticks.erase(found);

    return true;
}

InputMethodProfile* InputMethodType_Joystick::AddOldJoystickProfile()
{
    InputMethodProfile* p_ = this->AddProfile();
    InputMethodProfile_Joystick* p = dynamic_cast<InputMethodProfile_Joystick*>(p_);
    if(!p)
        return nullptr;
    p->InitAsJoystick();
    p->Name = "Old Joy " + std::to_string(this->m_profiles.size());

    return p_;
}

/*-----------------------*\
|| OPTIONAL METHODS      ||
\*-----------------------*/

// optional function allowing developer to associate device information with profile, etc
// if developer wants to forbid assignment, return false
bool InputMethodType_Joystick::SetProfile_Custom(InputMethod* method, int player_no, InputMethodProfile* profile, const std::vector<InputMethod*>& active_methods)
{
    (void)active_methods;

    InputMethod_Joystick* m = dynamic_cast<InputMethod_Joystick*>(method);
    InputMethodProfile_Joystick* p = dynamic_cast<InputMethodProfile_Joystick*>(profile);

    if(!m || !p || !m->m_devices)
        return false;

    // C++ has no proper logical XOR operator, so two lines
    if(p->m_controllerProfile && !m->m_devices->ctrl)
        return false;
    if(!p->m_controllerProfile && m->m_devices->ctrl)
        return false;

    // set in map!
    this->m_lastProfileByGUID[std::to_string(player_no+1)+"-"+m->m_devices->guid] = profile;
    this->m_lastProfileByGUID[m->m_devices->guid] = profile;

    pLogDebug("Setting default controller for GUID '%s' and player %d to %s.",
        m->m_devices->guid.c_str(), player_no, profile->Name.c_str());

    return true;
}

// unregisters any references to the profile before final deallocation
// returns false to prevent deletion if this is impossible
bool InputMethodType_Joystick::DeleteProfile_Custom(InputMethodProfile* profile, const std::vector<InputMethod*>& active_methods)
{
    (void)active_methods;

    for(auto it = m_lastProfileByGUID.begin(); it != m_lastProfileByGUID.end(); )
    {
        if(it->second == profile)
        {
            pLogDebug("Unsetting default controller for GUID '%s'.", it->first.c_str());
            it = m_lastProfileByGUID.erase(it);
        }
        else
        {
            ++it;
        }
    }

    return true;
}

// Optional function allowing developer to consume an SDL event (on SDL clients)
//     usually used for hotkeys or for connect/disconnect events.
// Called (1) in order of InputMethodTypes, then (2) in order of InputMethods
// Returns true if event is consumed, false if other InputMethodTypes and InputMethods
//     should receive it.
bool InputMethodType_Joystick::ConsumeEvent(const SDL_Event* ev)
{
    if(ev->type == SDL_JOYDEVICEADDED)
    {
        this->OpenJoystick(ev->jdevice.which);
        return true;
    }
    else if(ev->type == SDL_JOYDEVICEREMOVED)
    {
        this->CloseJoystick(ev->jdevice.which);
        return true;
    }
    return false;
}

// How many per-type special options are there?
size_t InputMethodType_Joystick::GetSpecialOptionCount()
{
    return 1;
}
// Methods to manage per-profile options
// It is guaranteed that none of these will be called if
// GetOptionCount() returns 0.
// get a char* describing the option
const char* InputMethodType_Joystick::GetOptionName(size_t i)
{
    if(i == 0)
        return "NEW PROFILE FOR OLD JOYSTICK";
    else
        return nullptr;
}
// get a char* describing the current option value
// must be allocated in static or instance memory
// WILL NOT be freed
const char* InputMethodType_Joystick::GetOptionValue(size_t i)
{
    (void)i;
    return nullptr;
}
// called when A is pressed; allowed to interrupt main game loop
bool InputMethodType_Joystick::OptionChange(size_t i)
{
    if(i == 0 && this->AddOldJoystickProfile())
        return true;

    return false;
}

void InputMethodType_Joystick::SaveConfig_Custom(IniProcessing* ctl)
{
    std::string name = "last-profile-";
    int uuid_begin = name.size();

    // set all default controller profiles
    for(auto it = m_lastProfileByGUID.begin(); it != m_lastProfileByGUID.end(); ++it)
    {
        std::vector<InputMethodProfile*>::iterator loc
            = std::find(this->m_profiles.begin(), this->m_profiles.end(), it->second);
        size_t index = loc - this->m_profiles.begin();
        if(index == this->m_profiles.size())
        {
            // this probably a legacy profile, let's check.
            InputMethodProfile_Joystick* p = dynamic_cast<InputMethodProfile_Joystick*>(it->second);
            if(p && p->m_legacyProfile)
            {
                ctl->endGroup();
                ctl->beginGroup("joystick-uuid-" + it->first);
                p->SaveConfig_Legacy(ctl);
                ctl->endGroup();
                ctl->beginGroup(this->Name);
            }
        }
        else
        {
            ctl->setValue(name.replace(uuid_begin, std::string::npos, it->first).c_str(), index);
        }
    }
}

void InputMethodType_Joystick::LoadConfig_Custom(IniProcessing* ctl)
{
    // load all default controller profiles
    std::vector<std::string> keys = ctl->allKeys();
    std::string keyNeed = "last-profile-";

    for(std::string& k : keys)
    {
        std::string::size_type r = k.find(keyNeed);
        if(r != std::string::npos && r == 0)
        {
            std::string guid = k.substr(13); // length of "last-profile-"
            int index;
            ctl->read(k.c_str(), index, -1);
            if(index >= 0 && index < (int)this->m_profiles.size() && this->m_profiles[index])
            {
                this->m_lastProfileByGUID[guid] = this->m_profiles[index];
                pLogDebug("Set default profile for '%s' to '%s'.", guid.c_str(), this->m_profiles[index]->Name.c_str());
            }
        }
    }

    ctl->endGroup();

    // load legacy controller profiles
    keys = ctl->childGroups();
    keyNeed = "joystick-uuid-";

    for(std::string& k : keys)
    {
        std::string::size_type r = k.find(keyNeed);
        if(r != std::string::npos && r == 0)
        {
            std::string guid = k.substr(14); // length of "joystick-uuid-"

            // once legacy controller has been converted, forget about it
            if(this->m_lastProfileByGUID.find(guid) != this->m_lastProfileByGUID.end())
                continue;

            ctl->beginGroup(k);
            InputMethodProfile_Joystick* profile = new(std::nothrow) InputMethodProfile_Joystick;
            if(profile)
            {
                profile->LoadConfig_Legacy(ctl);
                this->m_lastProfileByGUID[guid] = profile;
                pLogDebug("Loaded legacy profile as '%s'.", guid.c_str());
            }
            else
            {
                pLogWarning("Could not allocate legacy profile (OOM).");
            }
            ctl->endGroup();
        }
    }

    ctl->beginGroup(this->Name);
}

} // namespace Controls
