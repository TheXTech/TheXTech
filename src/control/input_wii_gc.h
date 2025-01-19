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

#ifndef INPUT_WII_GC_H
#define INPUT_WII_GC_H

#include "../controls.h"
#include "control/input_wii.h"

#include <string>

namespace Controls
{

class InputMethod_GameCube : public InputMethod
{
public:
    using InputMethod::Type;
    using InputMethod::Profile;

    int m_chn = 0;
    int m_rumble_ticks = 0;

    InputMethod_GameCube(int chn) : m_chn(chn) {}
    ~InputMethod_GameCube();

    // Update functions that set player controls (and editor controls)
    // based on current device input. Return false if device lost.
    bool Update(int player, Controls_t &c, CursorControls_t &m, EditorControls_t &e, HotkeysPressed_t &h);

    void Rumble(int ms, float strength);

    StatusInfo GetStatus();
};

class InputMethodProfile_GameCube : public InputMethodProfile
{
private:
    bool m_canPoll = false;

public:
    using InputMethodProfile::Name;
    using InputMethodProfile::Type;

    // current settings
    bool m_simple_editor = true;

    // primary keys (also controller keys in legacy mode)
    uint32_t m_keys[PlayerControls::n_buttons];
    // secondary keys (also joystick keys in legacy mode)
    uint32_t m_keys2[PlayerControls::n_buttons];

    // cursor keys
    uint32_t m_cursor_keys[CursorControls::n_buttons];
    uint32_t m_cursor_keys2[CursorControls::n_buttons];

    // editor keys
    uint32_t m_editor_keys[EditorControls::n_buttons];
    uint32_t m_editor_keys2[EditorControls::n_buttons];

    // hotkeys
    uint32_t m_hotkeys[Hotkeys::n_buttons];
    uint32_t m_hotkeys2[Hotkeys::n_buttons];

    InputMethodProfile_GameCube();

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

public:
    // How many per-type special options are there?
    size_t GetOptionCount_Custom();
    // Methods to manage per-profile options
    // It is guaranteed that none of these will be called if
    // GetOptionCount_Custom() returns 0.
    // get a char* describing the option
    const char *GetOptionName_Custom(size_t i);
    // get a char* describing the current option value
    // must be allocated in static or instance memory
    // WILL NOT be freed
    const char *GetOptionValue_Custom(size_t i);
    // called when A is pressed; allowed to interrupt main game loop
    bool OptionChange_Custom(size_t i);
    // called when left is pressed
    bool OptionRotateLeft_Custom(size_t i);
    // called when right is pressed
    bool OptionRotateRight_Custom(size_t i);
};

class InputMethodType_GameCube : public InputMethodType
{
private:
    InputMethodProfile *AllocateProfile() noexcept override;

public:
    using InputMethodType::Name;
    using InputMethodType::m_profiles;

    bool m_canPoll = false;

    PADStatus m_status_raw[4];
    PADStatus m_status[4];

    // how many frames has the controller been connected?
    std::array<uint8_t, 4> m_chnConnected{0};

    InputMethodType_GameCube();
    ~InputMethodType_GameCube();

    const std::string& LocalName() const override;

    bool TestProfileType(InputMethodProfile *profile) override;
    bool RumbleSupported() override;
    bool PowerStatusSupported() override;

    void UpdateControlsPre() override;
    void UpdateControlsPost() override;

    // null if no input method is ready
    // allocates the new InputMethod on the heap
    InputMethod *Poll(const std::vector<InputMethod *> &active_methods) noexcept override;

    uint32_t PollKeyAll() const;
};

} // namespace Controls

#endif // #ifndef INPUT_WII_GC_H
