/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2021 Vitaly Novichkov <admin@wohlnet.ru>
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

#include <Logger/logger.h>
#include <Utils/maths.h>

#include "touchscreen.h"
#include "../globals.h"
#ifdef __ANDROID__
#include "../gfx.h"
#endif

#ifdef HAS_VIBRATOR
#include <SDL2/SDL_haptic.h>
#endif

#ifdef __ANDROID__
#   include <jni.h>
#   if 1
#       undef JNIEXPORT
#       undef JNICALL
#       define JNIEXPORT extern "C"
#       define JNICALL
#   endif

enum
{
    KEYBOARD_NOKEYS = 0x00000001,
    KEYBOARD_QWERTY = 0x00000002,
    KEYBOARD_12KEY = 0x00000003
};

enum
{
    TOUCHSCREEN_DISABLE = 0,
    TOUCHSCREEN_DISABLE_ON_KEYBOARD = 1,
    TOUCHSCREEN_ENABLE = 2,
};

enum
{
    TOUCHPAD_STYLE_ACTIONS = 0,
    TPUCHPAD_STYLE_ABXY = 1,
    TPUCHPAD_STYLE_XODA = 2
};

//! Is hardware keyboard presented?
static int s_keyboardPresence = KEYBOARD_NOKEYS;
static int s_touchscreenMode = TOUCHSCREEN_DISABLE_ON_KEYBOARD;
static bool s_showTouchscreenOnStart = false;
static int  s_touchPadStyle = 0;
static double s_screenSize = 0;
static double s_screenWidth = 0;
static double s_screenHeight = 0;

static bool     s_vibrationEnable = false;
static float    s_vibrationStrength = 1.0;
static int      s_vibrationLength = 12;

#endif

bool TouchScreenController::touchSupported()
{
    if(m_touchDevicesCount <= 0)
        return false;

#ifdef __ANDROID__
    switch(s_touchscreenMode)
    {
    case TOUCHSCREEN_DISABLE:
        return false;
    case TOUCHSCREEN_DISABLE_ON_KEYBOARD:
        if(s_keyboardPresence > KEYBOARD_NOKEYS)
            return false;
        break;
    case TOUCHSCREEN_ENABLE:
        return true;
    }
#endif

    return true;
}


#ifdef __ANDROID__

static void initTouchMap();

JNIEXPORT void JNICALL
Java_ru_wohlsoft_thextech_thextechActivity_setHardwareKeyboardPresence(
    JNIEnv *env,
    jclass type,
    jint keyboard
)
{
    (void)env;
    (void)type;
    s_keyboardPresence = keyboard;
}

JNIEXPORT void JNICALL
Java_ru_wohlsoft_thextech_thextechActivity_setTouchScreenMode(
    JNIEnv *env,
    jclass type,
    jint mode
)
{
    (void)env;
    (void)type;
    s_touchscreenMode = mode;
}

JNIEXPORT void JNICALL
Java_ru_wohlsoft_thextech_thextechActivity_setTouchScreenShowOnStart(
        JNIEnv *env,
        jclass type,
        jboolean showOnStart
)
{
    (void)env;
    (void)type;
    s_showTouchscreenOnStart = showOnStart;
}

JNIEXPORT void JNICALL
Java_ru_wohlsoft_thextech_thextechActivity_setScreenSize(
        JNIEnv *env,
        jclass type,
        jdouble screenSize,
        jdouble screenWidth,
        jdouble screenHeight
)
{
    (void)env;
    (void)type;
    s_screenSize = screenSize;
    s_screenWidth = screenWidth;
    s_screenHeight = screenHeight;
    initTouchMap();
}

JNIEXPORT void JNICALL
Java_ru_wohlsoft_thextech_thextechActivity_setTouchPadStyle(
        JNIEnv *env,
        jclass type,
        jint style
)
{
    (void)env;
    (void)type;
    s_touchPadStyle = style;
}

JNIEXPORT void JNICALL
Java_ru_wohlsoft_thextech_thextechActivity_setVibrationEnabled(
    JNIEnv *env,
    jclass type,
    jboolean enableVibration
)
{
    (void)env;
    (void)type;
    s_vibrationEnable = enableVibration;
}

JNIEXPORT void JNICALL
Java_ru_wohlsoft_thextech_thextechActivity_setVibrationStrength(
    JNIEnv *env,
    jclass type,
    jfloat strength
)
{
    (void)env;
    (void)type;
    s_vibrationStrength = strength;
}

JNIEXPORT void JNICALL
Java_ru_wohlsoft_thextech_thextechActivity_setVibrationLength(
    JNIEnv *env,
    jclass type,
    jint length
)
{
    (void)env;
    (void)type;
    s_vibrationLength = length;
}
#endif

TouchScreenController::FingerState::FingerState()
{
    for(int i = key_BEGIN; i < key_END; i++)
    {
        heldKey[i] = false;
        heldKeyPrev[i] = false;
    }
}

TouchScreenController::FingerState::FingerState(const FingerState &fs)
{
    alive = fs.alive;
    for(int i = key_BEGIN; i < key_END; i++)
    {
        heldKey[i] = fs.heldKey[i];
        heldKeyPrev[i] = fs.heldKeyPrev[i];
    }
}

TouchScreenController::FingerState &TouchScreenController::FingerState::operator=(const FingerState &fs)
{
    alive = fs.alive;
    for(int i = key_BEGIN; i < key_END; i++)
    {
        heldKey[i] = fs.heldKey[i];
        heldKeyPrev[i] = fs.heldKeyPrev[i];
    }
    return *this;
}

static int buttonLeft()
{
    if(LevelSelect && GamePaused)
        return GFX_t::BUTTON_LEFT_CHAR;
    else
        return GFX_t::BUTTON_LEFT;
}

static int buttonRight()
{
    if(LevelSelect && GamePaused)
        return GFX_t::BUTTON_RIGHT_CHAR;
    else
        return GFX_t::BUTTON_RIGHT;
}

static int buttonA()
{
    switch(s_touchPadStyle)
    {
    case TPUCHPAD_STYLE_ABXY:
        return GFX_t::BUTTON_A;
    case TPUCHPAD_STYLE_XODA:
        return GFX_t::BUTTON_A_PS;
    default:
    case TOUCHPAD_STYLE_ACTIONS:
        if(GamePaused || GameMenu)
            return GFX_t::BUTTON_A_DO;
        else if(GameOutro)
            return GFX_t::BUTTON_A_BLANK;
        else if(LevelSelect)
            return GFX_t::BUTTON_A_ENTER;
        else
            return GFX_t::BUTTON_A_JUMP;
    }
}

static int buttonX()
{
    switch(s_touchPadStyle)
    {
    case TPUCHPAD_STYLE_ABXY:
        return GFX_t::BUTTON_X;
    case TPUCHPAD_STYLE_XODA:
        return GFX_t::BUTTON_X_PS;
    default:
    case TOUCHPAD_STYLE_ACTIONS:
        if(GamePaused || GameMenu)
            return GFX_t::BUTTON_X_BACK;
        else if(LevelSelect || GameOutro)
            return GFX_t::BUTTON_X_BLANK;
        else
        {
            if(numPlayers >= 1)
            {
                auto &p = Player[1];
                if(p.Character == 5 || p.State == 4 || p.State == 5)
                    return GFX_t::BUTTON_X_SWORD;
                else if(p.State < 3)
                    return GFX_t::BUTTON_X_RUN;
                else if(p.State == 3 || p.State == 7)
                    return GFX_t::BUTTON_X_FIRE;
                else if(p.State == 6)
                {
                    switch(p.Character)
                    {
                    default:
                    case 1:
                    case 2:
                        return GFX_t::BUTTON_X_HAMMER;
                    case 3:
                        return GFX_t::BUTTON_X_BOMB;
                    case 4:
                        return GFX_t::BUTTON_X_BUMERANG;
                    case 5:
                        return GFX_t::BUTTON_X_SWORD;
                    }
                }
            }
            return GFX_t::BUTTON_X_BLANK;
        }
    }
}

static int buttonB()
{
    switch(s_touchPadStyle)
    {
    case TPUCHPAD_STYLE_ABXY:
        return GFX_t::BUTTON_B;
    case TPUCHPAD_STYLE_XODA:
        return GFX_t::BUTTON_B_PS;
    default:
    case TOUCHPAD_STYLE_ACTIONS:
        if(LevelSelect || GamePaused || GameMenu || GameOutro)
            return GFX_t::BUTTON_B_BLANK;
        else
        {
            if(numPlayers >= 1)
            {
                auto &p = Player[1];
                if(p.Character <= 2 || p.Character == 4)
                    return GFX_t::BUTTON_B_SPINJUMP;
                else
                    return GFX_t::BUTTON_B_JUMP;
            }
            return GFX_t::BUTTON_B_BLANK;
        }
    }
}

static int buttonY()
{
    switch(s_touchPadStyle)
    {
    case TPUCHPAD_STYLE_ABXY:
        return GFX_t::BUTTON_Y;
    case TPUCHPAD_STYLE_XODA:
        return GFX_t::BUTTON_Y_PS;
    default:
    case TOUCHPAD_STYLE_ACTIONS:
        if(LevelSelect || GamePaused || GameMenu || GameOutro)
            return GFX_t::BUTTON_Y_BLANK;
        else
        {
            if(numPlayers >= 1)
            {
                auto &p = Player[1];
                if(p.State == 5)
                    return GFX_t::BUTTON_Y_STATUE;
                if(p.Character == 5 || p.State == 4)
                    return GFX_t::BUTTON_Y_SWORD;
                else if(p.State < 3)
                    return GFX_t::BUTTON_Y_RUN;
                else if(p.State == 3 || p.State == 7)
                    return GFX_t::BUTTON_Y_FIRE;
                else if(p.State == 6)
                {
                    switch(p.Character)
                    {
                        default:
                        case 1:
                        case 2:
                            return GFX_t::BUTTON_Y_HAMMER;
                        case 3:
                            return GFX_t::BUTTON_Y_BOMB;
                        case 4:
                            return GFX_t::BUTTON_Y_BUMERANG;
                        case 5:
                            return GFX_t::BUTTON_Y_SWORD;
                    }
                }
            }
            return GFX_t::BUTTON_Y_BLANK;
        }
    }
}


static struct TouchKeyMap
{
    struct KeyPos
    {
        float x1;
        float y1;
        float x2;
        float y2;
        TouchScreenController::commands cmd;
    };

    //! Width of canvas area
    float touchCanvasWidth = 1024.0f;
    //! Height of canvas area
    float touchCanvasHeight = 600.0f;

    //! List of key hit boxes
    KeyPos touchKeysMap[TouchScreenController::key_END];

    TouchKeyMap()
    {
        SDL_assert(sizeof(touchKeysMap) == TouchScreenController::key_END * sizeof(KeyPos));
        for(int it = TouchScreenController::key_BEGIN; it < TouchScreenController::key_END; it++)
        {
            auto &p = touchKeysMap[it];
            p.cmd = static_cast<TouchScreenController::commands>(it);
            SDL_assert(p.cmd >= TouchScreenController::key_BEGIN && p.cmd < TouchScreenController::key_END);
        }
    }

    /**
     * \brief Change size of virtual canvas
     * @param width Width of touchable canvas
     * @param height Height of touchable canvas
     */
    void setCanvasSize(float width, float height)
    {
        touchCanvasWidth = width;
        touchCanvasHeight = height;
    }

    /**
     * \brief Change hitbox of key
     * @param cmd Command
     * @param left Left side position
     * @param top Top side position
     * @param right Right side position
     * @param bottom Bottom side position
     */
    void setKeyPos(TouchScreenController::commands cmd, float left, float top, float right, float bottom)
    {
        if(cmd < TouchScreenController::key_BEGIN || cmd >= TouchScreenController::key_END)
            return;

        SDL_assert_release(touchKeysMap[cmd].cmd == cmd);

        auto &key = touchKeysMap[cmd];
        key.x1 = left;
        key.y1 = top;
        key.x2 = right;
        key.y2 = bottom;
    }

    /**
     * \brief Find keys are touched by one finger
     * @param x X position of finger
     * @param y Y position of finger
     * @param keys Destination array to write captured keys
     * @return Count of keys are got touched
     */
    int findTouchKeys(float x, float y, TouchScreenController::FingerState &fs)
    {
//        const size_t touchKeyMapSize = sizeof(touchKeysMap) / sizeof(KeyPos);
        int count = 0;
        x *= touchCanvasWidth;
        y *= touchCanvasHeight;

        for(const auto &p : touchKeysMap)
        {
            SDL_assert(p.cmd >= TouchScreenController::key_BEGIN && p.cmd < TouchScreenController::key_END);
            fs.heldKey[p.cmd] = false;
            if(x >= p.x1 && x <= p.x2 && y >= p.y1 && y <= p.y2)
            {
                fs.heldKey[p.cmd] = true;
                count++;
            }
        }

        return count;
    }

} g_touchKeyMap;

/*-----------------------Screen size depending layouts ----------------------------------*/
/************************************************
 * Please use this tool to edit these layouts:  *
 * https://github.com/Wohlstand/TouchpadTuner   *
 ************************************************/

static const TouchKeyMap::KeyPos c_4_tinyPhoneMap[TouchScreenController::key_END] =
{
    /* Note that order of keys must match the TouchScreenController::commands enum!!! */
    {335.0f, 12.0f, 451.0f, 52.0f, TouchScreenController::key_start},
    {11.0f, 262.0f, 91.0f, 342.0f, TouchScreenController::key_left},
    {171.0f, 262.0f, 251.0f, 342.0f, TouchScreenController::key_right},
    {91.0f, 182.0f, 171.0f, 262.0f, TouchScreenController::key_up},
    {91.0f, 342.0f, 171.0f, 422.0f, TouchScreenController::key_down},
    {11.0f, 182.0f, 91.0f, 262.0f, TouchScreenController::key_upleft},
    {171.0f, 182.0f, 251.0f, 262.0f, TouchScreenController::key_upright},
    {11.0f, 342.0f, 91.0f, 422.0f, TouchScreenController::key_downleft},
    {171.0f, 342.0f, 251.0f, 422.0f, TouchScreenController::key_downright},
    {396.0f, 276.0f, 493.0f, 373.0f, TouchScreenController::key_run},
    {512.0f, 307.0f, 609.0f, 404.0f, TouchScreenController::key_jump},
    {416.0f, 163.0f, 513.0f, 260.0f, TouchScreenController::key_altrun},
    {531.0f, 191.0f, 628.0f, 288.0f, TouchScreenController::key_altjump},
    {196.0f, 12.0f, 312.0f, 52.0f, TouchScreenController::key_drop},
    {494.0f, 50.0f, 618.0f, 97.0f, TouchScreenController::key_holdRun},
    {10.0f, 10.0f, 64.0f, 64.0f, TouchScreenController::key_toggleKeysView},
};

static const TouchKeyMap::KeyPos c_averagePhoneMap[TouchScreenController::key_END] =
{
    /* Note that order of keys must match the TouchScreenController::commands enum!!! */
    {542.0f, 537.0f, 693.0f, 587.0f, TouchScreenController::key_start},
    {1.0f, 410.0f, 83.0f, 492.0f, TouchScreenController::key_left},
    {165.0f, 410.0f, 247.0f, 492.0f, TouchScreenController::key_right},
    {83.0f, 328.0f, 165.0f, 410.0f, TouchScreenController::key_up},
    {83.0f, 492.0f, 165.0f, 574.0f, TouchScreenController::key_down},
    {1.0f, 328.0f, 83.0f, 410.0f, TouchScreenController::key_upleft},
    {165.0f, 328.0f, 247.0f, 410.0f, TouchScreenController::key_upright},
    {1.0f, 492.0f, 83.0f, 574.0f, TouchScreenController::key_downleft},
    {165.0f, 492.0f, 247.0f, 574.0f, TouchScreenController::key_downright},
    {764.0f, 403.0f, 868.0f, 507.0f, TouchScreenController::key_run},
    {885.0f, 436.0f, 989.0f, 540.0f, TouchScreenController::key_jump},
    {786.0f, 287.0f, 890.0f, 391.0f, TouchScreenController::key_altrun},
    {904.0f, 317.0f, 1008.0f, 421.0f, TouchScreenController::key_altjump},
    {331.0f, 537.0f, 482.0f, 587.0f, TouchScreenController::key_drop},
    {827.0f, 129.0f, 943.0f, 169.0f, TouchScreenController::key_holdRun},
    {10.0f, 10.0f, 70.0f, 70.0f, TouchScreenController::key_toggleKeysView},
};

static const TouchKeyMap::KeyPos c_averagePhoneLongMap[TouchScreenController::key_END] =
{
    /* Note that order of keys must match the TouchScreenController::commands enum!!! */
    {727.0f, 632.0f, 911.0f, 691.0f, TouchScreenController::key_start},
    {5.0f, 444.0f, 106.0f, 545.0f, TouchScreenController::key_left},
    {207.0f, 444.0f, 308.0f, 545.0f, TouchScreenController::key_right},
    {106.0f, 343.0f, 207.0f, 444.0f, TouchScreenController::key_up},
    {106.0f, 545.0f, 207.0f, 646.0f, TouchScreenController::key_down},
    {5.0f, 343.0f, 106.0f, 444.0f, TouchScreenController::key_upleft},
    {207.0f, 343.0f, 308.0f, 444.0f, TouchScreenController::key_upright},
    {5.0f, 545.0f, 106.0f, 646.0f, TouchScreenController::key_downleft},
    {207.0f, 545.0f, 308.0f, 646.0f, TouchScreenController::key_downright},
    {1055.0f, 500.0f, 1183.0f, 628.0f, TouchScreenController::key_run},
    {1218.0f, 537.0f, 1346.0f, 665.0f, TouchScreenController::key_jump},
    {1085.0f, 351.0f, 1213.0f, 479.0f, TouchScreenController::key_altrun},
    {1242.0f, 389.0f, 1370.0f, 517.0f, TouchScreenController::key_altjump},
    {474.0f, 632.0f, 658.0f, 691.0f, TouchScreenController::key_drop},
    {1178.0f, 214.0f, 1315.0f, 275.0f, TouchScreenController::key_holdRun},
    {10.0f, 10.0f, 85.0f, 85.0f, TouchScreenController::key_toggleKeysView},
};

static const TouchKeyMap::KeyPos c_7_tablet[TouchScreenController::key_END] =
{
    /* Note that order of keys must match the TouchScreenController::commands enum!!! */
    {636.0f, 544.0f, 775.0f, 582.0f, TouchScreenController::key_start},
    {3.0f, 430.0f, 76.0f, 503.0f, TouchScreenController::key_left},
    {149.0f, 430.0f, 222.0f, 503.0f, TouchScreenController::key_right},
    {76.0f, 357.0f, 149.0f, 430.0f, TouchScreenController::key_up},
    {76.0f, 503.0f, 149.0f, 576.0f, TouchScreenController::key_down},
    {3.0f, 357.0f, 76.0f, 430.0f, TouchScreenController::key_upleft},
    {149.0f, 357.0f, 222.0f, 430.0f, TouchScreenController::key_upright},
    {3.0f, 503.0f, 76.0f, 576.0f, TouchScreenController::key_downleft},
    {149.0f, 503.0f, 222.0f, 576.0f, TouchScreenController::key_downright},
    {797.0f, 439.0f, 887.0f, 529.0f, TouchScreenController::key_run},
    {897.0f, 463.0f, 987.0f, 553.0f, TouchScreenController::key_jump},
    {819.0f, 341.0f, 909.0f, 431.0f, TouchScreenController::key_altrun},
    {919.0f, 363.0f, 1009.0f, 453.0f, TouchScreenController::key_altjump},
    {257.0f, 544.0f, 396.0f, 582.0f, TouchScreenController::key_drop},
    {873.0f, 226.0f, 968.0f, 258.0f, TouchScreenController::key_holdRun},
    {10.0f, 10.0f, 58.0f, 58.0f, TouchScreenController::key_toggleKeysView},
};

static const TouchKeyMap::KeyPos c_10_6_tablet[TouchScreenController::key_END] =
{
    /* Note that order of keys must match the TouchScreenController::commands enum!!! */
    {869.0f, 753.0f, 1002.0f, 785.0f, TouchScreenController::key_start},
    {19.0f, 591.0f, 89.0f, 661.0f, TouchScreenController::key_left},
    {159.0f, 591.0f, 229.0f, 661.0f, TouchScreenController::key_right},
    {89.0f, 521.0f, 159.0f, 591.0f, TouchScreenController::key_up},
    {89.0f, 661.0f, 159.0f, 731.0f, TouchScreenController::key_down},
    {19.0f, 521.0f, 89.0f, 591.0f, TouchScreenController::key_upleft},
    {159.0f, 521.0f, 229.0f, 591.0f, TouchScreenController::key_upright},
    {19.0f, 661.0f, 89.0f, 731.0f, TouchScreenController::key_downleft},
    {159.0f, 661.0f, 229.0f, 731.0f, TouchScreenController::key_downright},
    {1047.0f, 588.0f, 1137.0f, 678.0f, TouchScreenController::key_run},
    {1159.0f, 624.0f, 1249.0f, 714.0f, TouchScreenController::key_jump},
    {1067.0f, 477.0f, 1157.0f, 567.0f, TouchScreenController::key_altrun},
    {1179.0f, 505.0f, 1269.0f, 595.0f, TouchScreenController::key_altjump},
    {256.0f, 753.0f, 389.0f, 785.0f, TouchScreenController::key_drop},
    {1131.0f, 387.0f, 1225.0f, 426.0f, TouchScreenController::key_holdRun},
    {10.0f, 10.0f, 57.0f, 57.0f, TouchScreenController::key_toggleKeysView},
};
/*---------------------------------------------------------------------------------------*/

static void initTouchMap()
{
    if(s_screenSize >= 9.0) // Big tablets
    {
        g_touchKeyMap.touchCanvasWidth = 1300.0f;
        g_touchKeyMap.touchCanvasHeight = 812.0f;
        SDL_memcpy(g_touchKeyMap.touchKeysMap, c_10_6_tablet, sizeof(g_touchKeyMap.touchKeysMap));
    }
    else if(s_screenSize >= 7.0) // Middle tablets
    {
        g_touchKeyMap.touchCanvasWidth = 1024.0f;
        g_touchKeyMap.touchCanvasHeight = 600.0f;
        SDL_memcpy(g_touchKeyMap.touchKeysMap, c_7_tablet, sizeof(g_touchKeyMap.touchKeysMap));
    }
    else if(s_screenSize < 4.0) // Very small phones
    {
        g_touchKeyMap.touchCanvasWidth = 640.0f;
        g_touchKeyMap.touchCanvasHeight = 480.0f;
        SDL_memcpy(g_touchKeyMap.touchKeysMap, c_4_tinyPhoneMap, sizeof(g_touchKeyMap.touchKeysMap));
    }
    else // All other devices
    {
        // Longer screens (big ration between sides, more like a stick)
        if((s_screenWidth / s_screenHeight) > 1.6f)
        {
            g_touchKeyMap.touchCanvasWidth = 1396.0f;
            g_touchKeyMap.touchCanvasHeight = 720.0f;
            SDL_memcpy(g_touchKeyMap.touchKeysMap, c_averagePhoneLongMap, sizeof(g_touchKeyMap.touchKeysMap));
        }
        else // Shorter screens (smaller ratio between sides, more like a square)
        {
            g_touchKeyMap.touchCanvasWidth = 1024.0f;
            g_touchKeyMap.touchCanvasHeight = 600.0f;
            SDL_memcpy(g_touchKeyMap.touchKeysMap, c_averagePhoneMap, sizeof(g_touchKeyMap.touchKeysMap));
        }
    }
}


void TouchScreenController::doVibration()
{
#ifdef HAS_VIBRATOR
    if(!s_vibrationEnable || !m_vibrator)
        return;

    int ret = SDL_HapticRumblePlay(m_vibrator, s_vibrationStrength, s_vibrationLength);
    D_pLogDebug("TouchScreen: Vibration %g, %d ms, ret %d", s_vibrationStrength, s_vibrationLength, ret);
#endif
}

TouchScreenController::TouchScreenController() noexcept = default;
TouchScreenController::~TouchScreenController() = default;

void TouchScreenController::init()
{
    initTouchMap();

    for(int key = key_BEGIN; key < key_END; ++key)
        m_keysHeld[key] = false;
    m_touchHidden = !s_showTouchscreenOnStart;
    D_pLogDebugNA("Initialization of touch-screen controller...");
    m_touchDevicesCount = SDL_GetNumTouchDevices();
    updateScreenSize();
    D_pLogDebug("Found %d touch devices, screen size: %d x %d",
                m_touchDevicesCount,
                m_screenWidth, m_screenHeight);
    pLogDebug("The screen size: %g inches (%g x %g)", s_screenSize, s_screenWidth, s_screenHeight);

#ifdef HAS_VIBRATOR
    m_vibrator = nullptr;
    int numHaptics = SDL_NumHaptics();

    for(int i = 0; i < numHaptics; ++i)
    {
        if(SDL_strcmp(SDL_HapticName(i), "VIBRATOR_SERVICE") == 0)
        {
            m_vibrator = SDL_HapticOpen(i);
            if(m_vibrator)
            {
                pLogDebug("TouchScreen: Opened the vibrator service");
                SDL_HapticRumbleInit(m_vibrator);
            }
            else
                pLogWarning("TouchScreen: Can't open the vibrator service");
            break;
        }
    }
#endif
}

void TouchScreenController::quit()
{
#ifdef HAS_VIBRATOR
    if(m_vibrator)
        SDL_HapticClose(m_vibrator);
    m_vibrator = nullptr;
#endif
}

void TouchScreenController::updateScreenSize()
{
    SDL_GetWindowSize(frmMain.getWindow(), &m_screenWidth, &m_screenHeight);
}

static void updateKeyValue(bool &key, bool state)
{
    key = state;
    D_pLogDebug("= Touch key: State=%d", (int)key);
}

static void updateFingerKeyState(TouchScreenController::FingerState &st,
        Controls_t &keys, int keyCommand, bool setState, TouchScreenController::ExtraKeys &extraSt)
{
    st.alive = (setState != 0);
    if(keyCommand >= static_cast<int>(TouchScreenController::key_BEGIN) && keyCommand < static_cast<int>(TouchScreenController::key_END))
    {
        switch(keyCommand)
        {
            case TouchScreenController::key_left:
                updateKeyValue(keys.Left, setState);
                break;
            case TouchScreenController::key_right:
                updateKeyValue(keys.Right, setState);
                break;
            case TouchScreenController::key_up:
                updateKeyValue(keys.Up, setState);
                break;
            case TouchScreenController::key_down:
                updateKeyValue(keys.Down, setState);
                break;
            case TouchScreenController::key_upleft:
                updateKeyValue(keys.Up, setState);
                updateKeyValue(keys.Left, setState);
                break;
            case TouchScreenController::key_upright:
                updateKeyValue(keys.Up, setState);
                updateKeyValue(keys.Right, setState);
                break;
            case TouchScreenController::key_downleft:
                updateKeyValue(keys.Down, setState);
                updateKeyValue(keys.Left, setState);
                break;
            case TouchScreenController::key_downright:
                updateKeyValue(keys.Down, setState);
                updateKeyValue(keys.Right, setState);
                break;
            case TouchScreenController::key_jump:
                updateKeyValue(keys.Jump, setState);
                break;
            case TouchScreenController::key_altjump:
                updateKeyValue(keys.AltJump, setState);
                break;
            case TouchScreenController::key_run:
                extraSt.keyRunOnce = (setState & !keys.Run);
                updateKeyValue(keys.Run, setState);
                break;
            case TouchScreenController::key_altrun:
                extraSt.keyAltRunOnce = (setState & !keys.AltRun);
                updateKeyValue(keys.AltRun, setState);
                break;
            case TouchScreenController::key_drop:
                updateKeyValue(keys.Drop, setState);
                break;
            case TouchScreenController::key_start:
                updateKeyValue(keys.Start, setState);
                break;
            case TouchScreenController::key_toggleKeysView:
                extraSt.keyToggleViewOnce = (setState & !extraSt.keyToggleView);
                extraSt.keyToggleView = setState;
                break;
            case TouchScreenController::key_holdRun:
                extraSt.keyHoldRunOnce = (setState & !extraSt.keyHoldRun);
                extraSt.keyHoldRun = setState;
                break;
            default:
                break;
        }
    }
    else
    {
        st.alive = false;
    }
}

void TouchScreenController::processTouchDevice(int dev_i)
{
    const SDL_TouchID dev = SDL_GetTouchDevice(dev_i);

    int fingers = SDL_GetNumTouchFingers(dev);

    for(auto &m_finger : m_fingers)
    {
        // Mark as "dead"
        m_finger.second.alive = false;
    }

    for(int i = 0; i < fingers; i++)
    {
        SDL_Finger *f = SDL_GetTouchFinger(dev, i);
        if (!f || (f->id < 0)) //Skip a wrong finger
            continue;

        SDL_FingerID finger_id = f->id;
        float finger_x = f->x, finger_y = f->y, finger_pressure = f->pressure;

        auto found = m_fingers.find(finger_id);
        if(found != m_fingers.end())
        {
            FingerState &fs = found->second;
            int keysCount = g_touchKeyMap.findTouchKeys(finger_x, finger_y, fs);
            for(int key = key_BEGIN; key < key_END; ++key)
            {
                if(m_touchHidden && key != key_toggleKeysView)
                    key = key_toggleKeysView;
                if(fs.heldKeyPrev[key] && !fs.heldKey[key]) // set key off
                {
                    updateFingerKeyState(fs, m_current_keys, key, false, m_current_extra_keys);
                    D_pLogDebug("= Finger Key ID=%d released (move)", static_cast<int>(key));
                    m_keysHeld[key] = false;
                    fs.heldKeyPrev[key] = fs.heldKey[key];
                }
                else if(fs.heldKey[key]) // set key on and keep alive
                {
                    if(!fs.heldKeyPrev[key] && fs.heldKey[key])
                        doVibration(); // Vibrate when key gets on
                    updateFingerKeyState(fs, m_current_keys, key, true, m_current_extra_keys);
                    D_pLogDebug("= Finger Key ID=%d pressed (move)", static_cast<int>(key));
                    m_keysHeld[key] = true;
                    fs.heldKeyPrev[key] = fs.heldKey[key];
                }
            }
            fs.alive = (keysCount > 0);
        }
        else
        {
            // Detect which key is pressed, and press it
            FingerState st;
            int keysCount = g_touchKeyMap.findTouchKeys(finger_x, finger_y, st);
            for(int key = key_BEGIN; key < key_END; key++)
            {
                if(m_touchHidden && key != key_toggleKeysView)
                    key = key_toggleKeysView;
                if(st.heldKey[key]) // set key on
                {
                    updateFingerKeyState(st, m_current_keys, key, true, m_current_extra_keys);
                    D_pLogDebug("= Finger Key ID=%d pressed (put)", static_cast<int>(key));
                    doVibration();
                    m_keysHeld[key] = true;
                    st.heldKeyPrev[key] = st.heldKey[key];
                    // Also: when more than one touch devices found, choose one which is actual
                    // Otherwise, the spam of on/off events will happen
                    if(m_actualDevice < 0)
                        m_actualDevice = dev_i;
                }
            }

            st.alive = (keysCount > 0);
            if(st.alive)
            {
                D_pLogDebug("= Finger ID=%d came", static_cast<int>(finger_id));
                m_fingers.insert({finger_id, st});
            }
        }

        D_pLogDebug("= Finger press: ID=%d, X=%.04f, Y=%.04f, P=%.04f",
                    static_cast<int>(finger_id), finger_x, finger_y, finger_pressure);
    }

    for(auto it = m_fingers.begin(); it != m_fingers.end();)
    {
        if(!it->second.alive)
        {
            for(int key = key_BEGIN; key < key_END; key++)
            {
                if(it->second.heldKey[key]) // Key was previously held
                {
                    updateFingerKeyState(it->second, m_current_keys, key, false, m_current_extra_keys);
                    D_pLogDebug("= Finger Key ID=%d released (take)", static_cast<int>(key));
                    m_keysHeld[key] = false;
                }
            }
            D_pLogDebug("= Finger ID=%d has gone", static_cast<int>(it->first));

            it = m_fingers.erase(it);
            continue;
        }
        it++;
    }

    if(m_current_extra_keys.keyToggleViewOnce)
        m_touchHidden = !m_touchHidden;

    if(m_current_extra_keys.keyHoldRunOnce)
        m_runHeld = !m_runHeld;
}

void TouchScreenController::update()
{
    if(!touchSupported())
        return;

    // If actually used in the game touch was found, use it
    if(m_actualDevice >= 0)
    {
        processTouchDevice(m_actualDevice);
        return;
    }
    // Otherwise, find it
    for(int dev_i = 0; dev_i < m_touchDevicesCount; dev_i++)
    {
        processTouchDevice(dev_i);
        if(m_actualDevice >= 0)
            break;
    }
}

void TouchScreenController::render()
{
    if(!touchSupported())
        return;

    frmMain.offsetViewportIgnore(true);

    for(int key = key_BEGIN; key < key_END; key++)
    {
        if((m_touchHidden && key != TouchScreenController::key_toggleKeysView) || LoadingInProcess)
            key = TouchScreenController::key_toggleKeysView;
        const auto &k = g_touchKeyMap.touchKeysMap[key];
        int x1 = Maths::iRound((k.x1 / g_touchKeyMap.touchCanvasWidth) * float(m_screenWidth));
        int y1 = Maths::iRound((k.y1 / g_touchKeyMap.touchCanvasHeight) * float(m_screenHeight));
        int x2 = Maths::iRound((k.x2 / g_touchKeyMap.touchCanvasWidth) * float(m_screenWidth));
        int y2 = Maths::iRound((k.y2 / g_touchKeyMap.touchCanvasHeight) * float(m_screenHeight));
        int w = x2 - x1;
        int h = y2 - y1;
        float r = 1.0f;
        float g = 0.0f;

//        if(key == key_holdRun && m_runHeld)
//        {
//            r = 0.f;
//            g = 1.f;
//        }

#ifdef __ANDROID__
        float a = m_keysHeld[key] ? 0.9f : 0.3f;

        switch(key)
        {
        case TouchScreenController::key_toggleKeysView:
            frmMain.renderTextureScale(x1, y1, w, h,
                                       GFX.touch[m_touchHidden ? GFX_t::BUTTON_VIEW_TOGGLE_OFF : GFX_t::BUTTON_VIEW_TOGGLE_ON],
                                       1.f, 1.f, 1.f, a);
            break;
        case TouchScreenController::key_start:
            frmMain.renderTextureScale(x1, y1, w, h, GFX.touch[GFX_t::BUTTON_START], 1.f, 1.f, 1.f, a);
            break;
        case TouchScreenController::key_drop:
            frmMain.renderTextureScale(x1, y1, w, h, GFX.touch[GFX_t::BUTTON_DROP], 1.f, 1.f, 1.f, a);
            break;
        case TouchScreenController::key_up:
            frmMain.renderTextureScale(x1, y1, w, h, GFX.touch[GFX_t::BUTTON_UP], 1.f, 1.f, 1.f, a);
            break;
        case TouchScreenController::key_left:
            frmMain.renderTextureScale(x1, y1, w, h, GFX.touch[buttonLeft()], 1.f, 1.f, 1.f, a);
            break;
        case TouchScreenController::key_right:
            frmMain.renderTextureScale(x1, y1, w, h, GFX.touch[buttonRight()], 1.f, 1.f, 1.f, a);
            break;
        case TouchScreenController::key_down:
            frmMain.renderTextureScale(x1, y1, w, h, GFX.touch[GFX_t::BUTTON_DOWN], 1.f, 1.f, 1.f, a);
            break;

        case TouchScreenController::key_upleft:
            frmMain.renderTextureScale(x1, y1, w, h, GFX.touch[GFX_t::BUTTON_UPLEFT], 1.f, 1.f, 1.f, a);
            break;
        case TouchScreenController::key_upright:
            frmMain.renderTextureScale(x1, y1, w, h, GFX.touch[GFX_t::BUTTON_UPRIGHT], 1.f, 1.f, 1.f, a);
            break;
        case TouchScreenController::key_downleft:
            frmMain.renderTextureScale(x1, y1, w, h, GFX.touch[GFX_t::BUTTON_DOWNLEFT], 1.f, 1.f, 1.f, a);
            break;
        case TouchScreenController::key_downright:
            frmMain.renderTextureScale(x1, y1, w, h, GFX.touch[GFX_t::BUTTON_DOWNRIGHT], 1.f, 1.f, 1.f, a);
            break;

        case TouchScreenController::key_holdRun:
            frmMain.renderTextureScale(x1, y1, w, h,
                                       GFX.touch[m_runHeld ? GFX_t::BUTTON_HOLD_RUN_ON : GFX_t::BUTTON_HOLD_RUN_OFF],
                                       1.f, 1.f, 1.f, a);
            break;
        case TouchScreenController::key_jump:
            frmMain.renderTextureScale(x1, y1, w, h, GFX.touch[buttonA()], 1.f, 1.f, 1.f, a);
            break;
        case TouchScreenController::key_run:
            frmMain.renderTextureScale(x1, y1, w, h, GFX.touch[buttonX()], 1.f, 1.f, 1.f, a);
            break;
        case TouchScreenController::key_altjump:
            frmMain.renderTextureScale(x1, y1, w, h, GFX.touch[buttonB()], 1.f, 1.f, 1.f, a);
            break;
        case TouchScreenController::key_altrun:
            frmMain.renderTextureScale(x1, y1, w, h, GFX.touch[buttonY()], 1.f, 1.f, 1.f, a);
            break;
        default:
            frmMain.renderRect(x1, y1, w, h, r, g, 0.f, 0.3f);
            break;
        }
#else
        frmMain.renderRect(x1, y1, w, h, r, g, 0.f, 0.3f);
#endif
    }

    frmMain.offsetViewportIgnore(false);
}
