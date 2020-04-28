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

#include "globals.h"
#include "joystick.h"

#include "pseudo_vb.h"

// this module handles the players controls, both keyboard and joystick

static std::vector<SDL_Joystick*> g_joysticks;

static void updateJoyKey(SDL_Joystick *j, bool &key, const KM_Key &mkey)
{
    Sint32 val = 0, dx = 0, dy = 0;
    Sint16 val_initial = 0;
    bool key_new = false;

    switch(mkey.type)
    {
    case ConJoystick_t::JoyAxis:
        //Note: SDL_JoystickGetAxisInitialState is a new API function added into dev version
        //      and doesn't available in already released assemblies
        if(SDL_JoystickGetAxisInitialState(j, mkey.id, &val_initial) == SDL_FALSE)
        {
            key_new = false;
            break;
        }
        val = SDL_JoystickGetAxis(j, mkey.id);

        if(mkey.val > val_initial)
            key_new = (val > val_initial);
        else if(mkey.val < val_initial)
            key_new = (val < val_initial);
        else key_new = false;

        break;

    case ConJoystick_t::JoyBallX:
        SDL_JoystickGetBall(j, mkey.id, &dx, &dy);

        if(mkey.id > 0)
            key_new = (dx > 0);
        else if(mkey.id < 0)
            key_new = (dx < 0);
        else key_new = false;

        break;

    case ConJoystick_t::JoyBallY:
        SDL_JoystickGetBall(j, mkey.id, &dx, &dy);

        if(mkey.id > 0)
            key_new = (dy > 0);
        else if(mkey.id < 0)
            key_new = (dy < 0);
        else key_new = false;

        break;

    case ConJoystick_t::JoyHat:
        val = (Sint32)SDL_JoystickGetHat(j, mkey.id);
        key_new = ((val & mkey.val)) != 0;
        break;

    case ConJoystick_t::JoyButton:
        key_new = (0 != (Sint32)SDL_JoystickGetButton(j, mkey.id));
        break;

    default:
        key_new = false;
        break;
    }

//    key_pressed = (key_new && !key);
    key = key_new;
}

static bool bindJoystickKey(SDL_Joystick *joy, KM_Key &k)
{
    Sint32 val = 0;
    Sint16 val_initial = 0;
    int dx = 0, dy = 0;
    //SDL_PumpEvents();
    SDL_JoystickUpdate();
    int balls = SDL_JoystickNumBalls(joy);
    int hats = SDL_JoystickNumHats(joy);
    int buttons = SDL_JoystickNumButtons(joy);
    int axes = SDL_JoystickNumAxes(joy);

    for(int i = 0; i < balls; i++)
    {
        dx = 0;
        dy = 0;
        SDL_JoystickGetBall(joy, i, &dx, &dy);

        if(dx != 0)
        {
            k.val = dx;
            k.id = i;
            k.type = (int)ConJoystick_t::JoyBallX;
            return true;
        }
        else if(dy != 0)
        {
            k.val = dy;
            k.id = i;
            k.type = (int)ConJoystick_t::JoyBallY;
            return true;
        }
    }

    for(int i = 0; i < hats; i++)
    {
        val = 0;
        val = SDL_JoystickGetHat(joy, i);

        if(val != 0)
        {
            k.val = val;
            k.id = i;
            k.type = (int)ConJoystick_t::JoyHat;
            return true;
        }
    }

    for(int i = 0; i < buttons; i++)
    {
        val = 0;
        val = SDL_JoystickGetButton(joy, i);

        if(val == 1)
        {
            k.val = val;
            k.id = i;
            k.type = (int)ConJoystick_t::JoyButton;
            return true;
        }
    }

    for(int i = 0; i < axes; i++)
    {
        val = 0;
        //Note: The SDL 2.0.6 and higher is requires to support this function
        if(SDL_JoystickGetAxisInitialState(joy, i, &val_initial) == SDL_FALSE)
            break;

        val = SDL_JoystickGetAxis(joy, i);

        if(val != (Sint32)val_initial)
        {
            k.val = val;
            k.id = i;
            k.type = (int)ConJoystick_t::JoyAxis;
            return true;
        }
    }

    k.val = 0;
    k.id = 0;
    k.type = (int)ConJoystick_t::NoControl;
    return false;
}

void UpdateControls()
{
    int A = 0;
//    int B = 0;
//    int C = 0;

//    If TestLevel = True And Not (nPlay.Online = True And nPlay.Mode = 0) Then
//    if(TestLevel)
//    {
//        if(getKeyState(vbKeyEscape) == KEY_PRESSED)
//        {
//            EndLevel = true;
//        }
//    }

    if(numJoysticks > 0 && (useJoystick[1] || useJoystick[2]))
        SDL_JoystickUpdate();

    For(B, 1, numPlayers)
    {
        if(B == 2 && numPlayers == 2) {
            A = 2;
        } else {
            A = 1;
        }

        // With Player(A).Controls
        {
            Controls_t &c = Player[A].Controls;
            c.Down = false;
            c.Drop = false;
            c.Jump = false;
            c.Left = false;
            c.Right = false;
            c.Run = false;
            c.Start = false;
            c.Up = false;
            c.AltJump = false;
            c.AltRun = false;

            if(useJoystick[A] > 0) // There is a joystick
            {
                int jNum = useJoystick[A] - 1;

                if(jNum < 0 || jNum >= int(g_joysticks.size()))
                    continue;

                auto *j = g_joysticks[size_t(jNum)];

                updateJoyKey(j, c.Up, conJoystick[A].Up);
                updateJoyKey(j, c.Down, conJoystick[A].Down);
                updateJoyKey(j, c.Left, conJoystick[A].Left);
                updateJoyKey(j, c.Right, conJoystick[A].Right);
                updateJoyKey(j, c.Run, conJoystick[A].Run);
                updateJoyKey(j, c.AltRun, conJoystick[A].AltRun);
                updateJoyKey(j, c.Jump, conJoystick[A].Jump);
                updateJoyKey(j, c.AltJump, conJoystick[A].AltJump);
                updateJoyKey(j, c.Drop, conJoystick[A].Drop);
                updateJoyKey(j, c.Start, conJoystick[A].Start);
            }

            if(useJoystick[A] == 0) // Keyboard controls
            {
                if(getKeyStateI(conKeyboard[A].Up)) {
                    c.Up = true;
                }

                if(getKeyStateI(conKeyboard[A].Down)) {
                    c.Down = true;
                }

                if(getKeyStateI(conKeyboard[A].Left)) {
                    c.Left = true;
                }

                if(getKeyStateI(conKeyboard[A].Right)) {
                    c.Right = true;
                }

                if(getKeyStateI(conKeyboard[A].Jump)) {
                    c.Jump = true;
                }

                if(getKeyStateI(conKeyboard[A].Run)) {
                    c.Run = true;
                }

                if(getKeyStateI(conKeyboard[A].Drop)) {
                    c.Drop = true;
                }

                if(getKeyStateI(conKeyboard[A].Start)) {
                    c.Start = true;
                }

                if(getKeyStateI(conKeyboard[A].AltJump)) {
                    c.AltJump = true;
                }

                if(getKeyStateI(conKeyboard[A].AltRun)) {
                    c.AltRun = true;
                }
            }

            /* // DEAD CODE
//            If .Left = True And .Right = True Then
            if(c.Left And c.Right) {
//                .Left = False
                c.Left = False;
//                .Right = False
                c.Right = False;
//            End If
            }*/

            if(!Player[A].Controls.Start && !Player[A].Controls.Jump) {
                Player[A].UnStart = true;
            }

            if(c.Up && c.Down)
            {
                c.Up = false;
                c.Down = false;
            }

            if(c.Left && c.Right)
            {
                c.Left = false;
                c.Right = false;
            }

            if(!(Player[A].State == 5 && Player[A].Mount == 0) && c.AltRun)
                c.Run = true;
            if(ForcedControls && !GamePaused )
            {
                Player[A].Controls = ForcedControl;
            }
        } // End With
    }

    if(SingleCoop > 0)
    {
        if(numPlayers == 1 || numPlayers > 2)
            SingleCoop = 0;

        Controls_t tempControls;
        if(SingleCoop == 1) {
            Player[2].Controls = tempControls;
        } else {
            Player[2].Controls = Player[1].Controls;
            Player[1].Controls = tempControls;
        }
    }

    // !UNNEEDED DEAD CODE
//    If nPlay.Online = True Then
//        Player(nPlay.MySlot + 1).Controls = Player(1).Controls
//        If Not (nPlay.MyControls.AltJump = Player(1).Controls.AltJump And nPlay.MyControls.AltRun = Player(1).Controls.AltRun And nPlay.MyControls.Down = Player(1).Controls.Down And nPlay.MyControls.Drop = Player(1).Controls.Drop And nPlay.MyControls.Jump = Player(1).Controls.Jump And nPlay.MyControls.Left = Player(1).Controls.Left And nPlay.MyControls.Right = Player(1).Controls.Right And nPlay.MyControls.Run = Player(1).Controls.Run And nPlay.MyControls.Start = Player(1).Controls.Start And nPlay.MyControls.Up = Player(1).Controls.Up) Then
//            nPlay.MyControls = Player(1).Controls
//            nPlay.Player(nPlay.MySlot).Controls = Player(1).Controls
//            If Player(nPlay.MySlot + 1).Dead = False And Player(nPlay.MySlot + 1).TimeToLive = 0 Then Netplay.sendData Netplay.PutPlayerControls(nPlay.MySlot)
//        Else
//            nPlay.MyControls = Player(1).Controls
//            nPlay.Player(nPlay.MySlot).Controls = Player(1).Controls
//        End If
//        For A = 0 To numPlayers - 1
//            If nPlay.Player(A).Active = True And A <> nPlay.MySlot Then
//                Player(A + 1).Controls = nPlay.Player(A).Controls
//            End If
//        Next A
//    End If

    For(A, 1, numPlayers)
    {
        {
            Player_t &p = Player[A];
            if(p.SpinJump)
            {
                if(p.SpinFrame < 4 || p.SpinFrame > 9) {
                    p.Direction = -1;
                } else {
                    p.Direction = 1;
                }
            }
        }
    }
}


int InitJoysticks()
{
    SDL_JoystickEventState(SDL_ENABLE);
    int num = SDL_NumJoysticks();

    for(int i = 0; i < num; ++i)
    {
        SDL_Joystick *joy = SDL_JoystickOpen(i);
        if(joy)
        {
            pLogDebug("==========================");
            pLogDebug("Josytick %s", SDL_JoystickName(joy));
            pLogDebug("--------------------------");
            pLogDebug("Axes:    %d", SDL_JoystickNumAxes(joy));
            pLogDebug("Balls:   %d", SDL_JoystickNumBalls(joy));
            pLogDebug("Hats:    %d", SDL_JoystickNumHats(joy));
            pLogDebug("Buttons: %d", SDL_JoystickNumButtons(joy));
            if(SDL_IsGameController(i))
                pLogDebug("Supported by the game controller interface!");
            pLogDebug("==========================");

            g_joysticks.push_back(joy);
        }
        else
        {
            pLogWarning("==========================");
            pLogWarning("Can't open joystick #%d", i);
            pLogWarning("==========================");
            return false;
        }
    }

    return int(g_joysticks.size());
}


bool StartJoystick(int JoystickNumber)
{
    if(JoystickNumber < 0 || JoystickNumber >= int(g_joysticks.size()))
        return false;

    SDL_Joystick *joy = g_joysticks[size_t(JoystickNumber)];
    if(joy)
    {
        int balls = SDL_JoystickNumBalls(joy);
        int hats = SDL_JoystickNumHats(joy);
        int buttons = SDL_JoystickNumButtons(joy);
        int axes = SDL_JoystickNumAxes(joy);

        pLogDebug("==========================");
        pLogDebug("Josytick %s", SDL_JoystickName(joy));
        pLogDebug("--------------------------");
        pLogDebug("Axes:    %d", axes);
        pLogDebug("Balls:   %d", balls);
        pLogDebug("Hats:    %d", hats);
        pLogDebug("Buttons: %d", buttons);
        if(SDL_IsGameController(JoystickNumber))
            pLogDebug("Supported by the game controller interface!");
        pLogDebug("==========================");
        return true;
    }
    else
    {
        pLogWarning("==========================");
        pLogWarning("Can't open joystick #%d", JoystickNumber);
        pLogWarning("==========================");
        return false;
    }
}

void CloseJoysticks()
{
    for(size_t i = 0; i < g_joysticks.size(); ++i) // scan hats first
        SDL_JoystickClose(g_joysticks[i]);
    g_joysticks.clear();
}

bool PollJoystick(int joystick, KM_Key &key)
{
    if(joystick < 0 || joystick >= int(g_joysticks.size()))
        return false;
    return bindJoystickKey(g_joysticks[size_t(joystick)], key);
}

bool JoyIsKeyDown(int JoystickNumber, const KM_Key &key)
{
    bool val = false;
    if(JoystickNumber < 0 || JoystickNumber >= int(g_joysticks.size()))
        return false;

    updateJoyKey(g_joysticks[size_t(JoystickNumber)], val, key);
    return val;
}
