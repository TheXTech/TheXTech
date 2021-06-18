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

    // Update functions that set player controls (and editor controls)
    // based on current device input. Return false if device lost.
    bool Update(Controls_t& c);
    // bool Update(EditorControls_t& c);

    void Rumble(int ms, float strength);
};

class InputMethodProfile_Keyboard : public InputMethodProfile
{
private:
    bool m_canPoll = false;

public:
    using InputMethodProfile::Name;
    using InputMethodProfile::Type;

    int m_keys[PlayerControls::n_buttons] = {null_key};
    int m_keys2[PlayerControls::n_buttons] = {null_key};

    InputMethodProfile_Keyboard();

    // Polls a new (secondary) device button for the i'th player button
    // Returns true on success and false if no button pressed
    // Never allows two player buttons to bind to the same device button
    bool PollPrimaryButton(size_t i);
    bool PollSecondaryButton(size_t i);

    // Deletes a secondary device button for the i'th player button
    bool DeleteSecondaryButton(size_t i);

    // Gets strings for the device buttons currently used for the i'th player button
    const char* NamePrimaryButton(size_t i);
    const char* NameSecondaryButton(size_t i);

    // one can assume that the IniProcessing* is already in the correct group
    void SaveConfig(IniProcessing* ctl);
    void LoadConfig(IniProcessing* ctl);
};

class InputMethodType_Keyboard : public InputMethodType
{
private:
    bool m_canPoll = false;
    int m_maxKeyboards = 1;

    InputMethodProfile* AllocateProfile() noexcept;

public:
    using InputMethodType::Name;
    using InputMethodType::m_profiles;

    const uint8_t* m_keyboardState;
    int m_keyboardStateSize;

    InputMethodType_Keyboard();

    void UpdateControlsPre();
    void UpdateControlsPost();

    // null if no input method is ready
    // allocates the new InputMethod on the heap
    InputMethod* Poll(const std::vector<InputMethod*>& active_methods) noexcept;

    // How many per-type special options are there?
    size_t GetSpecialOptionCount();

    /*-----------------------*\
    || OPTIONAL METHODS      ||
    \*-----------------------*/
    // Methods to manage per-profile options
    // It is guaranteed that none of these will be called if
    // GetOptionCount() returns 0.
    // get a char* describing the option
    const char* GetOptionName(size_t i);
    // get a char* describing the current option value
    // must be allocated in static or instance memory
    // WILL NOT be freed
    const char* GetOptionValue(size_t i);
    // called when A is pressed; allowed to interrupt main game loop
    bool OptionChange(size_t i);
    // called when left is pressed
    bool OptionRotateLeft(size_t i);
    // called when right is pressed
    bool OptionRotateRight(size_t i);
};

} // namespace Controls

#endif // #ifndef KEYBOARD_H
