#include "../controls.h"
#include "keyboard.h"

namespace Controls
{

// helper functions


/*====================================================*\
|| implementation for InputMethod_Keyboard            ||
\*====================================================*/

// Update functions that set player controls (and editor controls)
// based on current device input. Return false if device lost.
bool InputMethod_Keyboard::Update(Controls_t& c, CursorControls_t& m, EditorControls_t& e)
{
    InputMethodType_Keyboard* k = dynamic_cast<InputMethodType_Keyboard*>(this->Type);
    InputMethodProfile_Keyboard* p = dynamic_cast<InputMethodProfile_Keyboard*>(this->Profile);
    if(!k || !p)
        return false;
    bool altPressed = (k->m_keyboardState[SDL_SCANCODE_LALT] == KEY_PRESSED
                || k->m_keyboardState[SDL_SCANCODE_RALT] == KEY_PRESSED
                || k->m_keyboardState[SDL_SCANCODE_LCTRL] == KEY_PRESSED
                || k->m_keyboardState[SDL_SCANCODE_RCTRL] == KEY_PRESSED);
    for(size_t i = 0; i < PlayerControls::n_buttons; i++)
    {
        int key = p->m_keys[i];
        int key2 = p->m_keys2[i];
        bool& b = PlayerControls::GetButton(c, i);

        bool inhibit = altPressed && (key == SDL_SCANCODE_RETURN || key == SDL_SCANCODE_F);

        if(inhibit)
            b = false;
        else if(k->m_keyboardState[key] != 0)
            b = true;
        else if(key2 != null_key && k->m_keyboardState[key2] != 0)
            b = true;
        else
            b = false;
    }

    // TODO: for debugging purposes, REMOVE
    if(c.Up && c.Down)
        return false;
    return true;
}

void InputMethod_Keyboard::Rumble(int ms, float strength)
{
    (void)ms;
    (void)strength;
}

/*====================================================*\
|| implementation for InputMethodProfile_Keyboard     ||
\*====================================================*/

// the job of this function is to initialize the class in a consistent state
InputMethodProfile_Keyboard::InputMethodProfile_Keyboard()
{
    this->m_keys[PlayerControls::Buttons::Up] = SDL_SCANCODE_UP;
    this->m_keys[PlayerControls::Buttons::Down] = SDL_SCANCODE_DOWN;
    this->m_keys[PlayerControls::Buttons::Left] = SDL_SCANCODE_LEFT;
    this->m_keys[PlayerControls::Buttons::Right] = SDL_SCANCODE_RIGHT;
    this->m_keys[PlayerControls::Buttons::Jump] = SDL_SCANCODE_Z;
    this->m_keys[PlayerControls::Buttons::AltJump] = SDL_SCANCODE_A;
    this->m_keys[PlayerControls::Buttons::Run] = SDL_SCANCODE_X;
    this->m_keys[PlayerControls::Buttons::AltRun] = SDL_SCANCODE_S;
    this->m_keys[PlayerControls::Buttons::Drop] = SDL_SCANCODE_LSHIFT;
    this->m_keys[PlayerControls::Buttons::Start] = SDL_SCANCODE_ESCAPE;
    for(size_t i = 0; i < PlayerControls::n_buttons; i++)
    {
        this->m_keys2[i] = null_key;
    }
}

bool InputMethodProfile_Keyboard::PollPrimaryButton(ControlsClass c, size_t i)
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
    this->m_canPoll = false;

    // minor switching algorithm to ensure that every button always has at least one key
    // and no button ever has a non-unique key
    // if a button's secondary key (including the current one) is the new key, delete it.
    // if a button's primary key (excluding the current one) is the new key,
    //     and it has a secondary key, overwrite it with the secondary key.
    //     otherwise, replace it with the button the player is replacing.
    for(size_t j = 0; j < PlayerControls::n_buttons; j++)
    {
        if(this->m_keys2[j] == key)
        {
            this->m_keys2[j] = null_key;
        }
        else if(i != j && this->m_keys[j] == key)
        {
            if(this->m_keys2[j] != null_key)
            {
                this->m_keys[j] = this->m_keys2[j];
                this->m_keys2[j] = null_key;
            }
            else
            {
                this->m_keys[j] = this->m_keys[i];
            }
        }
    }
    this->m_keys[i] = key;
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

    // minor switching algorithm to ensure that every button always has at least one key
    // and no button ever has a non-unique key

    // if the current button's primary key is the new key,
    //     delete its secondary key instead of setting it.
    if(this->m_keys[i] == key)
    {
        this->m_keys2[i] = null_key;
        return true;
    }
    // if another button's secondary key is the new key, delete it.
    // if another button's primary key is the new key,
    //     and it has a secondary key, overwrite it with the secondary key.
    //     otherwise, if this button's secondary key is defined, overwrite the other's with this.
    //     if all else fails, overwrite the other button's with this button's PRIMARY key and assign
    //         this button's PRIMARY key instead

    bool can_do_secondary = true;
    for(size_t j = 0; j < PlayerControls::n_buttons; j++)
    {
        if(i != j && this->m_keys2[j] == key)
        {
            this->m_keys2[j] = null_key;
        }
        else if(i != j && this->m_keys[j] == key)
        {
            if(this->m_keys2[j] != null_key)
            {
                this->m_keys[j] = this->m_keys2[j];
                this->m_keys2[j] = null_key;
            }
            else if(this->m_keys2[i] != null_key)
            {
                this->m_keys[j] = this->m_keys2[i];
            }
            else
            {
                this->m_keys[j] = this->m_keys[i];
                can_do_secondary = false;
            }
        }
    }
    if(can_do_secondary)
        this->m_keys2[i] = key;
    else
        this->m_keys[i] = key;
    return true;
}

bool InputMethodProfile_Keyboard::DeleteSecondaryButton(ControlsClass c, size_t i)
{
    if(this->m_keys2[i] != null_key)
    {
        this->m_keys2[i] = null_key;
        return true;
    }
    return false;
}

const char* InputMethodProfile_Keyboard::NamePrimaryButton(ControlsClass c, size_t i)
{
    if(this->m_keys[i] == null_key)
        return "NONE";
    return SDL_GetScancodeName((SDL_Scancode)this->m_keys[i]);
}

const char* InputMethodProfile_Keyboard::NameSecondaryButton(ControlsClass c, size_t i)
{
    if(this->m_keys2[i] == null_key)
        return "NONE";
    return SDL_GetScancodeName((SDL_Scancode)this->m_keys2[i]);
}

void InputMethodProfile_Keyboard::SaveConfig(IniProcessing* ctl)
{
    char name2[20];
    for(size_t i = 0; i < PlayerControls::n_buttons; i++)
    {
        const char* name = PlayerControls::GetButtonName_INI(i);
        ctl->setValue(name, this->m_keys[i]);
        for(size_t c = 0; c < 20; c++)
        {
            if(c+2 == 20 || name[c] == '\0')
            {
                name2[c] = '2';
                name2[c+1] = '\0';
                break;
            }
            name2[c] = name[c];
        }
        ctl->setValue(name2, this->m_keys2[i]);
    }
}

void InputMethodProfile_Keyboard::LoadConfig(IniProcessing* ctl)
{
    char name2[20];
    for(size_t i = 0; i < PlayerControls::n_buttons; i++)
    {
        const char* name = PlayerControls::GetButtonName_INI(i);
        ctl->read(name, this->m_keys[i], this->m_keys[i]);
        for(size_t c = 0; c < 20; c++)
        {
            if(c+2 == 20 || name[c] == '\0')
            {
                name2[c] = '2';
                name2[c+1] = '\0';
                break;
            }
            name2[c] = name[c];
        }
        ctl->read(name2, this->m_keys2[i], this->m_keys2[i]);
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

void InputMethodType_Keyboard::UpdateControlsPre() {}
void InputMethodType_Keyboard::UpdateControlsPost()
{
    bool altPressed = this->m_keyboardState[SDL_SCANCODE_LALT] == KEY_PRESSED ||
                      this->m_keyboardState[SDL_SCANCODE_RALT] == KEY_PRESSED;
    bool escPressed = this->m_keyboardState[SDL_SCANCODE_ESCAPE] == KEY_PRESSED;
    bool returnPressed = this->m_keyboardState[SDL_SCANCODE_RETURN] == KEY_PRESSED;
    bool spacePressed = this->m_keyboardState[SDL_SCANCODE_SPACE] == KEY_PRESSED;
    bool upPressed = this->m_keyboardState[SDL_SCANCODE_UP] == KEY_PRESSED;
    bool downPressed = this->m_keyboardState[SDL_SCANCODE_DOWN] == KEY_PRESSED;
    bool leftPressed = this->m_keyboardState[SDL_SCANCODE_LEFT] == KEY_PRESSED;
    bool rightPressed = this->m_keyboardState[SDL_SCANCODE_RIGHT] == KEY_PRESSED;

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
                escPressed = false;
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
    bool backPressed = this->m_keyboardState[SDL_SCANCODE_AC_BACK] == KEY_PRESSED;
#else
    bool backPressed = false;
#endif

    SharedControls.QuitCredits |= (spacePressed || backPressed);
    SharedControls.QuitCredits |= (returnPressed && !altPressed);
    SharedControls.QuitCredits |= (escPressed && !altPressed);

    SharedControls.Pause |= backPressed;
    SharedControls.Pause |= (escPressed && !altPressed);

    SharedControls.MenuUp |= upPressed;
    SharedControls.MenuDown |= downPressed;
    SharedControls.MenuLeft |= leftPressed;
    SharedControls.MenuRight |= rightPressed;
    SharedControls.MenuDo |= (returnPressed && !altPressed) || spacePressed;
    SharedControls.MenuBack |= (escPressed && !altPressed);

    if(this->m_touchscreenActive)
    {
        // let the touchscreen input method handle that
    }
    // handle the mouse
    else if(SDL_GetMouseFocus())
    {
        int window_x, window_y;
        Uint32 buttons = SDL_GetMouseState(&window_x, &window_y);

        SDL_Point p = frmMain.MapToScr(window_x, window_y);
        if(p.x - SharedCursor.X <= 1 || p.x - SharedCursor.X >= 1
            || p.y - SharedCursor.Y <= 1 || p.y - SharedCursor.Y >= 1)
        {
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
    else if(SharedCursor.X >= 0 || SharedCursor.Y >= 0)
    {
        SharedCursor.GoOffscreen();
    }
}

// this is challenging for the keyboard because we don't want to allocate 20 copies of it
InputMethod* InputMethodType_Keyboard::Poll(const std::vector<InputMethod*>& active_methods) noexcept
{
    int n_keyboards = 0;
    for(InputMethod* method : active_methods)
    {
        if(!method)
            continue;
        InputMethod_Keyboard* m = dynamic_cast<InputMethod_Keyboard*>(method);
        if(m)
        {
            n_keyboards ++;
        }
    }

    if(n_keyboards != m_lastNumKeyboards)
    {
        // this ensures that keys that were held when a keyboard method was removed cannot be polled to add that method back
        if(n_keyboards < this->m_lastNumKeyboards)
        {
            this->m_canPoll = false;
        }
        this->m_lastNumKeyboards = n_keyboards;
    }

    if(n_keyboards >= m_maxKeyboards)
    {
        // reset in case things change
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
        if(key == SDL_SCANCODE_LALT || key == SDL_SCANCODE_RALT || key == SDL_SCANCODE_LCTRL || key == SDL_SCANCODE_RCTRL)
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
        }
        if(!allowed)
            continue;
        // must find new profile
        for(InputMethodProfile* profile : this->m_profiles)
        {
            InputMethodProfile_Keyboard* p = dynamic_cast<InputMethodProfile_Keyboard*>(profile);
            if(!p)
                continue;
            for(size_t i = 0; i < PlayerControls::n_buttons; i++)
            {
                if(p->m_keys[i] == key || p->m_keys2[i] == key)
                {
                    target_profile = profile;
                    break;
                }
            }
            if(target_profile)
                break;
        }
        if(target_profile)
            break;
    }

    // if didn't find any key, allow poll in future but return false
    if(key == this->m_keyboardStateSize || target_profile == nullptr)
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

    method->Name = "Keyboard";
    method->Type = this;
    method->Profile = target_profile;

    return (InputMethod*)method;
}

/*-----------------------*\
|| OPTIONAL METHODS      ||
\*-----------------------*/

bool InputMethodType_Keyboard::ConsumeEvent(const SDL_Event* ev)
{
    bool is_touch = false;
    switch(ev->type)
    {
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
            is_touch = (ev->button.which == SDL_TOUCH_MOUSEID);
            break;
        case SDL_MOUSEMOTION:
            is_touch = (ev->motion.which == SDL_TOUCH_MOUSEID);
            break;
        default:
            return false;
    }
    this->m_touchscreenActive = is_touch;
    return false;
}

// optional function allowing developer to associate device information with profile, etc
bool InputMethodType_Keyboard::SetProfile_Custom(InputMethod* method, int player_no, InputMethodProfile* profile,
    const std::vector<InputMethod*>& active_methods)
{
    if(!method || !profile || player_no < 0 || player_no >= maxLocalPlayers)
    {
        return false;
    }
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
size_t InputMethodType_Keyboard::GetSpecialOptionCount()
{
    return 1;
}

// Methods to manage per-profile options
// It is guaranteed that none of these will be called if
// GetOptionCount() returns 0.
// get a char* describing the option
const char* InputMethodType_Keyboard::GetOptionName(size_t i)
{
    if(i == 0)
    {
        return "MAX KBD PLAYERS";
    }
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
    return nullptr;
}
// called when A is pressed; allowed to interrupt main game loop
bool InputMethodType_Keyboard::OptionChange(size_t i)
{
    if(i == 0)
    {
        this->m_maxKeyboards ++;
        if(this->m_maxKeyboards > 6)
            this->m_maxKeyboards = 0;
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
            this->m_maxKeyboards --;
        return true;
    }
    return false;
}
// called when right is pressed
bool InputMethodType_Keyboard::OptionRotateRight(size_t i)
{
    if(i == 0)
    {
        if(this->m_maxKeyboards < 6)
            this->m_maxKeyboards ++;
        return true;
    }
    return false;
}

void InputMethodType_Keyboard::SaveConfig_Custom(IniProcessing* ctl)
{
    ctl->setValue("max-keyboards", this->m_maxKeyboards);
}

void InputMethodType_Keyboard::LoadConfig_Custom(IniProcessing* ctl)
{
    ctl->read("max-keyboards", this->m_maxKeyboards, 2);
}

} // namespace Controls
