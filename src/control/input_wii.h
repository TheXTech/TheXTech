#ifndef INPUT_WII_H
#define INPUT_WII_H

#include <unordered_map>
#include <utility>

#include "../controls.h"

namespace Controls
{

constexpr uint32_t null_but = -1;

class InputMethod_Wii : public InputMethod
{
public:
    int m_chn = 0;
    int m_rumble_ticks = 0;
    uint8_t m_battery_status = 0;
    double m_shake_accum = 0.0;

    using InputMethod::Type;
    using InputMethod::Profile;

    InputMethod_Wii(int chn);
    ~InputMethod_Wii();

    // Update functions that set player controls (and editor controls)
    // based on current device input. Return false if device lost.
    bool Update(int player, Controls_t &c, CursorControls_t &m, EditorControls_t &e, HotkeysPressed_t &h);

    void Rumble(int ms, float strength);

    StatusInfo GetStatus();
};

class InputMethodProfile_Wii : public InputMethodProfile
{
private:
    std::unordered_map<uint16_t, InputMethodProfile *> m_lastProfileByPlayerAndExp;
    bool m_canPoll = false;

public:
    using InputMethodProfile::Name;
    using InputMethodProfile::Type;

    uint8_t m_expansion = 0;

    uint32_t m_keys[PlayerControls::n_buttons] = {null_but};
    uint32_t m_keys2[PlayerControls::n_buttons] = {null_but};

    uint32_t m_editor_keys[EditorControls::n_buttons] = {null_but};
    uint32_t m_editor_keys2[EditorControls::n_buttons] = {null_but};

    uint32_t m_cursor_keys2[CursorControls::n_buttons] = {null_but};

    uint32_t m_hotkeys[Hotkeys::n_buttons] = {null_but};
    uint32_t m_hotkeys2[Hotkeys::n_buttons] = {null_but};

    InputMethodProfile_Wii(uint8_t expansion);

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

class InputMethodType_Wii : public InputMethodType
{
private:
    std::unordered_map<uint16_t, InputMethodProfile *> m_lastProfileByPlayerAndExp;

    InputMethodProfile *AllocateProfile() noexcept;

public:
    bool m_canPoll = false;
    int m_irActiveChn = -1;
    int m_irLastX = -10;
    int m_irLastY = -10;

    using InputMethodType::Name;
    using InputMethodType::m_profiles;

    InputMethodType_Wii();
    ~InputMethodType_Wii();

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

protected:
    void SaveConfig_Custom(IniProcessing *ctl);
    void LoadConfig_Custom(IniProcessing *ctl);
};

} // namespace Controls

#endif // #ifndef INPUT_WII_H
