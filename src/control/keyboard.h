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

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "../controls.h"

namespace Controls
{

constexpr int null_key = -1;

class InputMethod_Keyboard : public InputMethod
{
public:
    using InputMethod::Type;
    using InputMethod::Profile;

    ~InputMethod_Keyboard();

    // Update functions that set player controls (and editor controls)
    // based on current device input. Return false if device lost.
    bool Update(int player, Controls_t &c, CursorControls_t &m, EditorControls_t &e, HotkeysPressed_t &h);

    void Rumble(int ms, float strength);

    StatusInfo GetStatus();
};

class InputMethodProfile_Keyboard : public InputMethodProfile
{
private:
    bool m_canPoll = false;

public:
    using InputMethodProfile::Name;
    using InputMethodProfile::Type;

    int m_keys[PlayerControls::n_buttons] = {null_key, null_key, null_key, null_key, null_key, null_key, null_key, null_key, null_key, null_key};
    int m_keys2[PlayerControls::n_buttons] = {null_key, null_key, null_key, null_key, null_key, null_key, null_key, null_key, null_key, null_key};

    int m_editor_keys[EditorControls::n_buttons] = {null_key, null_key, null_key, null_key, null_key, null_key, null_key, null_key, null_key, null_key};
    int m_editor_keys2[EditorControls::n_buttons] = {null_key, null_key, null_key, null_key, null_key, null_key, null_key, null_key, null_key, null_key};

    int m_cursor_keys2[CursorControls::n_buttons] = {null_key, null_key, null_key, null_key, null_key, null_key, null_key};

    int m_hotkeys[Hotkeys::n_buttons] = {null_key, null_key, null_key, null_key, null_key, null_key, null_key};
    int m_hotkeys2[Hotkeys::n_buttons] = {null_key, null_key, null_key, null_key, null_key, null_key, null_key};

    InputMethodProfile_Keyboard();

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

class InputMethodType_Keyboard : public InputMethodType
{
private:
    bool m_canPoll = false;

    int m_lastNumKeyboards = 0;
    bool m_touchscreenActive = false;
    Uint32 m_lastMousePress = 0;
    int m_scroll = 0;

    InputMethodProfile *AllocateProfile() noexcept;

public:
    int m_numKeyboards = 0;

    // options
    int m_maxKeyboards = 2;
    bool m_directText = true;

    using InputMethodType::m_profiles;

    const uint8_t *m_keyboardState;
    int m_keyboardStateSize = 0;

    InputMethodType_Keyboard();

    const std::string& LocalName() const override;

    bool TestProfileType(InputMethodProfile *profile);
    bool RumbleSupported();

    void UpdateControlsPre();
    void UpdateControlsPost();

    // null if no input method is ready
    // allocates the new InputMethod on the heap
    InputMethod *Poll(const std::vector<InputMethod *> &active_methods) noexcept;

    /*-----------------------*\
    || OPTIONAL METHODS      ||
    \*-----------------------*/
protected:
    // optional function allowing developer to associate device information with profile, etc
    // if developer wants to forbid assignment, return false
    bool SetProfile_Custom(InputMethod *method, int player_no, InputMethodProfile *profile, const std::vector<InputMethod *> &active_methods);

public:
    bool DefaultHotkey(const SDL_Event *ev);
    bool ConsumeEvent(const SDL_Event *ev);

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
    // called when left is pressed
    bool OptionRotateLeft(size_t i);
    // called when right is pressed
    bool OptionRotateRight(size_t i);

protected:
    void SaveConfig_Custom(IniProcessing *ctl);
    void LoadConfig_Custom(IniProcessing *ctl);
};

} // namespace Controls

#endif // #ifndef KEYBOARD_H
