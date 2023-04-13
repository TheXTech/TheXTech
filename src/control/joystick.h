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

#ifndef JOYSTICK_H
#define JOYSTICK_H

#include "../controls.h"

#include <string>
#include <set>
#include <unordered_map>

typedef struct _SDL_Joystick SDL_Joystick;
typedef struct _SDL_GameController SDL_GameController;
typedef struct _SDL_Haptic SDL_Haptic;

namespace Controls
{

struct JoystickDevices
{
    SDL_Joystick *joy = nullptr;
    SDL_GameController *ctrl = nullptr;
    SDL_Haptic *haptic = nullptr;

    bool can_poll = false;
    std::string guid;
};

struct KM_Key
{
    enum CtrlTypes
    {
        NoControl = -1,
        JoyAxis = 0,
        JoyBallX,
        JoyBallY,
        JoyHat,
        JoyButton,
        CtrlButton,
        CtrlAxis
    };

    // SDL_Joystick control or SDL_GameController control, depending on context
    int val = -1;
    int id = -1;
    int type = -1;

    inline bool operator==(const KM_Key &o)
    {
        return o.id == id && o.val == val && o.type == type;
    }

    inline void assign(int type, int id, int val)
    {
        this->type = type;
        this->id = id;
        this->val = val;
    }
};

class InputMethod_Joystick : public InputMethod
{
public:
    JoystickDevices *m_devices;

    using InputMethod::Type;
    using InputMethod::Profile;

    ~InputMethod_Joystick();

    // Update functions that set player controls (and editor controls)
    // based on current device input. Return false if device lost.
    bool Update(int player, Controls_t &c, CursorControls_t &m, EditorControls_t &e, HotkeysPressed_t &h);

    void Rumble(int ms, float strength);

    StatusInfo GetStatus();
};

class InputMethodProfile_Joystick : public InputMethodProfile
{
private:
    bool m_canPoll = false;

public:
    using InputMethodProfile::Name;
    using InputMethodProfile::Type;

    bool m_controllerProfile = false;
    bool m_legacyProfile = false;

    // primary keys (also controller keys in legacy mode)
    KM_Key m_keys[PlayerControls::n_buttons];
    // secondary keys (also joystick keys in legacy mode)
    KM_Key m_keys2[PlayerControls::n_buttons];

    // cursor keys
    KM_Key m_cursor_keys[CursorControls::n_buttons];
    KM_Key m_cursor_keys2[CursorControls::n_buttons];

    // editor keys
    KM_Key m_editor_keys[EditorControls::n_buttons];
    KM_Key m_editor_keys2[EditorControls::n_buttons];

    // hotkeys
    KM_Key m_hotkeys[Hotkeys::n_buttons];
    KM_Key m_hotkeys2[Hotkeys::n_buttons];

    InputMethodProfile_Joystick();

    void InitAsJoystick();
    void InitAsController();
    void ExpandAsJoystick();
    void ExpandAsController();
    void SaveConfig_Legacy(IniProcessing *ctl);
    void LoadConfig_Legacy(IniProcessing *ctl);

    // Polls a new (secondary) device button for the i'th player button
    // Returns true on success and false if no button pressed
    // Never allows two player buttons to bind to the same device button
    bool PollPrimaryButton(ControlsClass c, size_t i);
    bool PollSecondaryButton(ControlsClass c, size_t i);

    // Deletes a primary button for the i'th button of class c (only called for non-Player buttons)
    bool DeletePrimaryButton(ControlsClass c, size_t i);

    // Deletes a secondary device button for the i'th button of class c
    bool DeleteSecondaryButton(ControlsClass c, size_t i);

    // Gets strings for the device buttons currently used for the i'th button of class c
    const char *NamePrimaryButton(ControlsClass c, size_t i);
    const char *NameSecondaryButton(ControlsClass c, size_t i);

    // one can assume that the IniProcessing* is already in the correct group
    void SaveConfig(IniProcessing *ctl);
    void LoadConfig(IniProcessing *ctl);
};

class InputMethodType_Joystick : public InputMethodType
{
private:
    std::unordered_map<int, JoystickDevices *> m_availableJoysticks;
    std::set<InputMethodProfile_Joystick *> m_hiddenProfiles; // owns unconverted legacy profiles
    std::unordered_map<std::string, InputMethodProfile *> m_lastProfileByGUID;

    InputMethodProfile *AllocateProfile() noexcept;

    /*-----------------------*\
    || CUSTOM METHODS        ||
    \*-----------------------*/
    bool OpenJoystick(int joystick_index);
    bool CloseJoystick(int instance_id);

public:
    using InputMethodType::Name;
    using InputMethodType::m_profiles;

    InputMethodType_Joystick();
    ~InputMethodType_Joystick();

    const std::string& LocalName() const override;

    bool TestProfileType(InputMethodProfile *profile);
    bool RumbleSupported();

    void UpdateControlsPre();
    void UpdateControlsPost();

    // null if no input method is ready
    // allocates the new InputMethod on the heap
    InputMethod *Poll(const std::vector<InputMethod *> &active_methods) noexcept;

    /*-----------------------*\
    || CUSTOM METHODS        ||
    \*-----------------------*/
    KM_Key PollJoystickKeyAll();
    KM_Key PollControllerKeyAll();
    InputMethodProfile *AddOldJoystickProfile();

    /*-----------------------*\
    || OPTIONAL METHODS      ||
    \*-----------------------*/
protected:
    // optional function allowing developer to associate device information with profile, etc
    // if developer wants to forbid assignment, return false
    bool SetProfile_Custom(InputMethod *method, int player_no, InputMethodProfile *profile, const std::vector<InputMethod *> &active_methods);
    // unregisters any references to the profile before final deallocation
    // returns false to prevent deletion if this is impossible
    bool DeleteProfile_Custom(InputMethodProfile *profile, const std::vector<InputMethod *> &active_methods);
public:
    bool ConsumeEvent(const SDL_Event *ev);

public:
    // How many per-type special options are there?
    size_t GetOptionCount();
    // Methods to manage per-profile options
    // It is guaranteed that none of these will be called if
    // GetOptionCount() returns 0.
    // get a char* describing the option
    const char *GetOptionName(size_t i);
    // get a char* describing the current option value
    // must be allocated in static or instance memory
    // WILL NOT be freed
    const char *GetOptionValue(size_t i);
    // called when A is pressed; allowed to interrupt main game loop
    bool OptionChange(size_t i);

protected:
    void SaveConfig_Custom(IniProcessing *ctl);
    void LoadConfig_Custom(IniProcessing *ctl);

};

} // namespace Controls

#endif // #ifndef JOYSTICK_H
