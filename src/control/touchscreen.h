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

#pragma once
#ifndef TOUCHSCREEN_H
#define TOUCHSCREEN_H

#include <map>
#include <string>
#include <vector>

#include <SDL2/SDL_touch.h>

#include "../controls.h"
#include "../std_picture.h"

typedef struct _SDL_Haptic SDL_Haptic;

namespace Controls
{


// forward declaration for the TouchScreenController
class InputMethod_TouchScreen;

/*!
 * \brief A basic class including graphics for touchscreen controller
 */

class TouchScreenGFX_t
{
    std::vector<StdPicture *> m_loadedImages;
    void loadImage(StdPicture &img, const std::string &fileName);
    std::string m_gfxPath;
    int m_loadErrors = 0;
public:
    bool m_success = false;

    TouchScreenGFX_t();
    ~TouchScreenGFX_t();

    enum
    {
        BUTTON_START = 0,
        BUTTON_LEFT,
        BUTTON_LEFT_CHAR,
        BUTTON_RIGHT,
        BUTTON_RIGHT_CHAR,
        BUTTON_UP,
        BUTTON_DOWN,
        BUTTON_UPLEFT,
        BUTTON_UPRIGHT,
        BUTTON_DOWNLEFT,
        BUTTON_DOWNRIGHT,
        BUTTON_A,
        BUTTON_A_PS,
        BUTTON_A_BLANK,
        BUTTON_A_DO,
        BUTTON_A_ENTER,
        BUTTON_A_JUMP,
        BUTTON_B,
        BUTTON_B_PS,
        BUTTON_B_BLANK,
        BUTTON_B_JUMP,
        BUTTON_B_SPINJUMP,
        BUTTON_X,
        BUTTON_X_PS,
        BUTTON_X_BACK,
        BUTTON_X_BLANK,
        BUTTON_X_BOMB,
        BUTTON_X_BUMERANG,
        BUTTON_X_FIRE,
        BUTTON_X_HAMMER,
        BUTTON_X_RUN,
        BUTTON_X_SWORD,
        BUTTON_Y,
        BUTTON_Y_PS,
        BUTTON_Y_BLANK,
        BUTTON_Y_BOMB,
        BUTTON_Y_BUMERANG,
        BUTTON_Y_FIRE,
        BUTTON_Y_HAMMER,
        BUTTON_Y_RUN,
        BUTTON_Y_STATUE,
        BUTTON_Y_SWORD,
        BUTTON_DROP,
        BUTTON_HOLD_RUN_OFF,
        BUTTON_HOLD_RUN_ON,
        BUTTON_VIEW_TOGGLE_OFF,
        BUTTON_VIEW_TOGGLE_ON,
        BUTTON_ANALOG_BORDER,
        BUTTON_ANALOG_STICK,
        BUTTON_ENTER_CHEATS,
        BUTTONS_END
    };
    StdPicture touch[BUTTONS_END];
};

/*!
 * \brief A mobile touch-screen controller which reads state of the keyboard device
 */
class TouchScreenController
{
    //! Count of touch devices
    int m_touchDevicesCount = 0;
    //! Physical screen width (touchscreen coords)
    int m_screenWidth = 0;
    //! Physical screen height (touchscreen coords)
    int m_screenHeight = 0;
    //! Physical screen width (render coords)
    int m_renderWidth = 0;
    //! Physical screen height (render coords)
    int m_renderHeight = 0;
    //! Graphics for controller
    TouchScreenGFX_t m_GFX;

public:
    //! Vibrator used with the touch device as rumble and/or feedback
    SDL_Haptic *m_vibrator = nullptr;

    /*!
     * \brief Is touch-screen supported?
     */
    bool touchSupported();

    /*!
     * \brief Number of detected touchscreen devices
     * \return Count of devices detected
     */
    int numDevices() const;

    /*!
     * \brief Is touch-screen being touched?
     */
    bool touchOn();

    enum commands
    {
        key_BEGIN = 0,
        key_start = 0,
        key_left,
        key_right,
        key_up,
        key_down,
        key_upleft,
        key_upright,
        key_downleft,
        key_downright,
        key_run,
        key_jump,
        key_altrun,
        key_altjump,
        key_drop,
        key_holdRun,
        key_toggleKeysView,
        key_enterCheats,
        key_END
    };

    enum styles
    {
        style_actions = 0,
        style_abxy,
        style_xoda,
        style_END
    };

    enum layouts
    {
        layout_standard = 0,
        layout_tight,
        layout_old_tiny,
        layout_old_average,
        layout_old_long,
        layout_old_phablet,
        layout_old_tablet,
        layout_END
    };

    // touchscreen settings (duplicated from InputMethodProfile_TouchScreen)
    //! Current layout type
    int m_layout = TouchScreenController::layout_standard;
    //! General scale factor
    int m_scale_factor = 100;
    //! D-Pad exclusive scale factor
    int m_scale_factor_dpad = 100;
    //! Buttons scale factor
    int m_scale_factor_buttons = 100;
    //! Spacing between Select and Start
    int m_scale_factor_ss_spacing = 100;
    //! Current touchpad style
    int m_touchpad_style = TouchScreenController::style_actions;
    //! Feedback strength
    float m_feedback_strength = 0.f;
    //! Feedback length in milliseconds
    int m_feedback_length = 12;
    //! Hold run enabled
    bool m_hold_run = false;
    //! Show the enter cheats button
    bool m_enable_enter_cheats = false;

    // active InputMethod (nullable, used for configuration)
    InputMethod *m_active_method = nullptr;

    //! In-game controls pressed
    Controls_t m_current_keys;
    bool m_keysHeld[key_END] = {false};

    bool m_cursorActive = false;
    SDL_FingerID m_cursorFinger = -1;
    double m_cursorX = 0.0, m_cursorY = 0.0;
    double m_lastCursorX = -32.;
    double m_lastCursorY = -32.;

    bool m_scrollActive = false;
    bool m_wasScrolling = false;
    int m_lastMeanX = 0, m_lastMeanY = 0;
    double m_scrollX = 0., m_scrollY = 0.;
    // needed because of an SDL bug where the frame a finger it is lifted its finger still exists at its old location
    double m_lastScrollX = 0., m_lastScrollY = 0.;
    double m_scrollMomentumX = 0., m_scrollMomentumY = 0.;

    struct ExtraKeys_t
    {
        bool keyToggleView = false;
        bool keyToggleViewOnce = false;

        bool keyHoldRun = false;
        bool keyHoldRunOnce = false;

        bool keyRunOnce = false;
        bool keyAltRunOnce = false;

        bool keyCheats = false;

        ExtraKeys_t &operator|=(const ExtraKeys_t& o);
    } m_current_extra_keys;

    //! Touch can be hidden by left-top corner to use virtual mouse
    bool       m_touchHidden = false;
    bool       m_runHeld = false;

    struct FingerState
    {
        bool alive = false;
        bool ignore = false;
        bool heldKey[key_END] = {};
        bool heldKeyPrev[key_END] = {};

        FingerState();
        FingerState(const FingerState &fs);
        FingerState &operator=(const FingerState &fs);
    };

private:
    typedef std::map<SDL_FingerID, FingerState> FingersMap;

    struct TouchDevice_t
    {
        //! Touch device ID
        SDL_TouchID id = -1;
        //! Registered finger states
        FingersMap  fingers;
        //! States for control keys
        Controls_t current_keys;
        //! States for extra keys
        ExtraKeys_t extra_keys;
        //! Held finger states
        bool keysHeld[key_END] = {false};
    };

    std::vector<TouchDevice_t> m_devices;

    void doVibration();

    /*!
     * \brief Read current state of touch controller
     */
    void processTouchDevice(TouchDevice_t &dev);

public:
    /*!
     * \brief Constructor
     */
    TouchScreenController() noexcept;

    /*!
     * \brief Destructor
     */
    ~TouchScreenController();

    void scanTouchDevices();

    void updateScreenSize();

    void update();

    void render(int player_no);

    void resetState();
};

class InputMethod_TouchScreen : public InputMethod
{
public:
    using InputMethod::Type;
    using InputMethod::Profile;

    ~InputMethod_TouchScreen();

    // Update functions that set player controls (and editor controls)
    // based on current device input. Return false if device lost.
    bool Update(int player, Controls_t &c, CursorControls_t &m, EditorControls_t &e, HotkeysPressed_t &h);

    void Rumble(int ms, float strength);

    StatusInfo GetStatus();
};

class InputMethodProfile_TouchScreen : public InputMethodProfile
{
public:
    using InputMethodProfile::Name;
    using InputMethodProfile::Type;

    // touchscreen settings
    /* Screen-size depend defaults (computed on load) */
    int m_default_layout = TouchScreenController::layout_standard;
    int m_default_scale_factor = 100;
    int m_default_scale_factor_dpad = 100;
    int m_default_scale_factor_buttons = 100;
    int m_default_scale_factor_ss_spacing = 100;

    /* Current settings */
    //! Current layout type
    int m_layout = TouchScreenController::layout_standard;
    //! General scale factor
    int m_scale_factor = 100;
    //! D-Pad exclusive scale factor
    int m_scale_factor_dpad = 100;
    //! Buttons scale factor
    int m_scale_factor_buttons = 100;
    //! Spacing between Select and Start
    int m_scale_factor_ss_spacing = 100;
    //! Current touchpad style
    int m_touchpad_style = TouchScreenController::style_actions;
    float m_feedback_strength = 0.f;
    int m_feedback_length = 12;
    bool m_hold_run = false;
    bool m_enable_enter_cheats = false;
    //! Count of touch devices, gets filled on initialization
    int m_device_count = 0;

    InputMethodProfile_TouchScreen();

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

    /*-----------------------*\
    || OPTIONAL METHODS      ||
    \*-----------------------*/
    struct Options
    {
        enum o
        {
            layout,
            scale_factor,
            scale_factor_dpad,
            scale_factor_buttons,
            scale_factor_ss_spacing,
            reset_layout,
            style,
            fb_strength,
            fb_length,
            hold_run,
            enable_enter_cheats,
            COUNT
        };
    };

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

class InputMethodType_TouchScreen : public InputMethodType
{
private:
    bool m_canPoll = false;

    InputMethodProfile *AllocateProfile() noexcept;

public:
    using InputMethodType::Name;
    using InputMethodType::m_profiles;

    TouchScreenController m_controller;

    InputMethodType_TouchScreen();

    bool TestProfileType(InputMethodProfile *profile);
    bool RumbleSupported();
    bool ConsumeEvent(const SDL_Event *ev);

    void UpdateControlsPre();
    void UpdateControlsPost();

    // null if no input method is ready
    // allocates the new InputMethod on the heap
    InputMethod *Poll(const std::vector<InputMethod *> &active_methods) noexcept;
};

} // namespace Controls

#endif // TOUCHSCREEN_H
