/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2020 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <Logger/logger.h>

#include "touchscreen.h"
#include "../globals.h"

//#ifdef __ANDROID__
//#   include <jni.h>
//#   if 1
//#       undef JNIEXPORT
//#       undef JNICALL
//#       define JNIEXPORT extern "C"
//#       define JNICALL
//#   endif
//#endif

TouchScreenController::FingerState::FingerState()
{
    SDL_memset(&heldKey, 0, sizeof(heldKey));
    SDL_memset(&heldKeyPrev, 0, sizeof(heldKeyPrev));
}

TouchScreenController::FingerState::FingerState(const FingerState &fs)
{
    alive = fs.alive;
    SDL_memcpy(&heldKey, &fs.heldKey, sizeof(heldKey));
    SDL_memcpy(&heldKeyPrev, &fs.heldKeyPrev, sizeof(heldKeyPrev));
}

TouchScreenController::FingerState &TouchScreenController::FingerState::operator=(const FingerState &fs)
{
    alive = fs.alive;
    SDL_memcpy(&heldKey, &fs.heldKey, sizeof(heldKey));
    SDL_memcpy(&heldKeyPrev, &fs.heldKeyPrev, sizeof(heldKeyPrev));
    return *this;
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
    KeyPos touchKeysMap[TouchScreenController::key_END] =
    {
        /* Note that order of keys must match the Controller::commands enum!!! */
        {331.0f, 537.0f, 482.0f,  587.0f, TouchScreenController::key_start},

        {1.0f,   328.0f, 91.0f,   498.0f, TouchScreenController::key_left},
        {171.0f, 328.0f, 261.0f,  498.0f, TouchScreenController::key_right},
        {1.0f,  328.0f, 261.0f,  418.0f, TouchScreenController::key_up},
        {1.0f,  498.0f, 261.0f,  588.0f, TouchScreenController::key_down},

        {807.0f, 431.0f, 914.0f,  522.0f, TouchScreenController::key_run},
        {898.0f, 396.0f, 1005.0f, 487.0f, TouchScreenController::key_jump},
        {807.0f, 325.0f, 914.0f,  416.0f, TouchScreenController::key_altrun},
        {898.0f, 290.0f, 1005.0f, 381.0f, TouchScreenController::key_altjump},

        {542.0f, 537.0f, 693.0f,  587.0f, TouchScreenController::key_drop},

        {10.0f, 10.0f, 70.0f,  70.0f, TouchScreenController::key_toggleKeysView},
    };

    TouchKeyMap()
    {
        for(int it = TouchScreenController::key_BEGIN; it < TouchScreenController::key_END; it++)
            touchKeysMap[it].cmd = static_cast<TouchScreenController::commands>(it);
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


//#ifdef __ANDROID__

//JNIEXPORT void JNICALL
//Java_ru_wohlsoft_moondust_moondustActivity_setKeyPos(JNIEnv *env, jclass type,
//        jint cmd, jfloat left, jfloat top, jfloat right, jfloat bottom)
//{
//    (void)env;
//    (void)type;
//    g_touchKeyMap.setKeyPos(static_cast<Controller::commands>(cmd), left, top, right, bottom);
//}

//JNIEXPORT void JNICALL
//Java_ru_wohlsoft_moondust_moondustActivity_setCanvasSize(JNIEnv *env, jclass type,
//        jfloat width, jfloat height)
//{

//    (void)env;
//    (void)type;
//    g_touchKeyMap.setCanvasSize(width, height);
//}

//#endif


TouchScreenController::TouchScreenController()
{}

TouchScreenController::~TouchScreenController()
{}

void TouchScreenController::init()
{
    D_pLogDebugNA("Initialization of touch-screen controller...");
    m_touchDevicesCount = SDL_GetNumTouchDevices();
    updateScreenSize();
    D_pLogDebug("Found %d touch devices, screen size: %d x %d",
                m_touchDevicesCount,
                m_screenWidth, m_screenHeight);
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
        Controls_t &keys, int keyCommand, bool setState, TouchScreenController &kkk)
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
            case TouchScreenController::key_jump:
                updateKeyValue(keys.Jump, setState);
                break;
            case TouchScreenController::key_altjump:
                updateKeyValue(keys.AltJump, setState);
                break;
            case TouchScreenController::key_run:
                updateKeyValue(keys.Run, setState);
                break;
            case TouchScreenController::key_altrun:
                updateKeyValue(keys.AltRun, setState);
                break;
            case TouchScreenController::key_drop:
                updateKeyValue(keys.Drop, setState);
                break;
            case TouchScreenController::key_start:
                updateKeyValue(keys.Start, setState);
                break;
            case TouchScreenController::key_toggleKeysView:
                kkk.m_key_toggleViewOnce = (setState & !kkk.m_key_toggleView);
                kkk.m_key_toggleView = setState;
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
            for(int key = key_BEGIN; key < key_END; key++)
            {
                if(m_touchHidden && key != key_toggleKeysView)
                    key = key_toggleKeysView;
                if(fs.heldKeyPrev[key] && !fs.heldKey[key]) // set key off
                {
                    updateFingerKeyState(found->second, m_current_keys, key, false, *this);
                    fs.heldKeyPrev[key] = fs.heldKey[key];
                }
                else if(fs.heldKey[key]) // set key on and keep alive
                {
                    updateFingerKeyState(found->second, m_current_keys, key, true, *this);
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
                    updateFingerKeyState(found->second, m_current_keys, key, true, *this);
                    st.heldKeyPrev[key] = st.heldKey[key];
                    // Also: when more than one touch devices found, choose one which is actual
                    // Otherwise, the spam of on/off events will happen
                    if(m_actualDevice < 0)
                        m_actualDevice = dev_i;
                }
            }
            st.alive = (keysCount > 0);
            if(st.alive)
                m_fingers.insert({finger_id, st});
        }

        D_pLogDebug("= Finger press: ID=%d, X=%.04f, Y=%.04f, P=%.04f",
                    static_cast<int>(finger_id), finger_x, finger_y, finger_pressure);
    }

    for(auto it = m_fingers.begin(); it != m_fingers.end();)
    {
        if(!it->second.alive)
        {
            for(int key = key_BEGIN; key < key_END; key++)
                updateFingerKeyState(it->second, m_current_keys, key, false, *this);
            it = m_fingers.erase(it);
            continue;
        }
        it++;
    }

    if(m_key_toggleViewOnce)
        m_touchHidden = !m_touchHidden;
}

void TouchScreenController::update()
{
    if(m_touchDevicesCount == 0)
        return; // Nothing to do

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
    for(int key = key_BEGIN; key < key_END; key++)
    {
        if(m_touchHidden && key != TouchScreenController::key_toggleKeysView)
            key = TouchScreenController::key_toggleKeysView;
        const auto &k = g_touchKeyMap.touchKeysMap[key];
        int x1 = std::round((k.x1 / g_touchKeyMap.touchCanvasWidth) * m_screenWidth);
        int y1 = std::round((k.y1 / g_touchKeyMap.touchCanvasHeight) * m_screenHeight);
        int x2 = std::round((k.x2 / g_touchKeyMap.touchCanvasWidth) * m_screenWidth);
        int y2 = std::round((k.y2 / g_touchKeyMap.touchCanvasHeight) * m_screenHeight);
        int w = x2 - x1;
        int h = y2 - y1;
        frmMain.renderRect(x1, y1, w, h, 1.0f, 0.f, 0.f, 0.3f);
    }
}
