#include <Logger/logger.h>

#include "globals.h"
#include "joystick.h"

#include "pseudo_vb.h"

// 'this module handles the players controls, both keyboard and joystick

//Public Declare Function joyGetPosEx Lib "winmm.dll" (ByVal uJoyID As Integer, pji As JOYINFOEX) As Integer
//Public Declare Function joyGetDevCapsA Lib "winmm.dll" (ByVal uJoyID As Integer, pjc As JOYCAPS, ByVal cjc As Integer) As Integer

// Public JoyNum As Long
long JoyNum = 0;
// Public MYJOYEX As JOYINFOEX
JOYINFOEX_t MYJOYEX;
// Public MYJOYCAPS As JOYCAPS
JOYCAPS_t MYJOYCAPS;
// Public CenterX(0 To 7) As Long
RangeArr<long, 0, 7> CenterX;
// Public CenterY(0 To 7) As Long
RangeArr<long, 0, 7> CenterY;
// Public JoyButtons(-15 To 15) As Boolean
RangeArr<bool, -255, 255> JoyButtons;
// Public CurrentJoyX As Long
long CurrentJoyX = 0;
// Public CurrentJoyY As Long
long CurrentJoyY = 0;
// Public CurrentJoyPOV As Long
long CurrentJoyPOV = 0;

static std::vector<SDL_Joystick*> g_joysticks;


void UpdateControls()
{
    // Dim A As Integer
    int A = 0;
    // Dim B As Integer
    int B = 0;
    // Dim C As Integer
    int C = 0;

//    If TestLevel = True And Not (nPlay.Online = True And nPlay.Mode = 0) Then
    if(TestLevel)
    {
//        If GetKeyState(vbKeyEscape) And KEY_PRESSED Then
        if(getKeyState(vbKeyEscape) == KEY_PRESSED)
        {
//            EndLevel = True
            EndLevel = true;
//        End If
        }
//    End If
    }

    if(numJoysticks > 0 && (useJoystick[1] || useJoystick[2]))
        SDL_JoystickUpdate();

//    For B = 1 To numPlayers
    For(B, 1, numPlayers)
    {
//        If B = 2 And numPlayers = 2 Then
        if(B == 2 && numPlayers == 2) {
//            A = 2
            A = 2;
//        Else
        } else {
//            A = 1
            A = 1;
//        End If
        }
//        With Player(A).Controls
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
//            If useJoystick(A) > 0 Then 'There is a joystick
            if(useJoystick[A] > 0)
            {
//                JoyNum = useJoystick(A) - 1
                JoyNum = useJoystick[A] - 1;
//                PollJoystick
                PollJoystick(useJoystick[A] - 1);
//                If CurrentJoyX < CenterX(JoyNum) - CenterX(JoyNum) * 0.3 Or (CurrentJoyPOV >= 22500 And CurrentJoyPOV <= 31500) Then
                if(CurrentJoyX < CenterX[JoyNum] - CenterX[JoyNum] * 0.3 ||
                   (CurrentJoyPOV >= 22500 && CurrentJoyPOV <= 31500)) {
//                    .Left = True
                    c.Left = true;
//                ElseIf CurrentJoyX > CenterX(JoyNum) + CenterX(JoyNum) * 0.3 Or (CurrentJoyPOV >= 4500 And CurrentJoyPOV <= 13500) Then
                } else if(CurrentJoyX > CenterX[JoyNum] + CenterX[JoyNum] * 0.3 ||
                         (CurrentJoyPOV >= 4500 && CurrentJoyPOV <= 13500)) {
//                    .Right = True
                    c.Right = true;
//                End If
                }
//                If CurrentJoyY < CenterY(JoyNum) - CenterY(JoyNum) * 0.3 Or (CurrentJoyPOV >= 0 And CurrentJoyPOV <= 4500) Or CurrentJoyPOV = 31500 Then
                if(CurrentJoyY < CenterY[JoyNum] - CenterY[JoyNum] * 0.3 ||
                         (CurrentJoyPOV >= 0 && CurrentJoyPOV <= 4500) || CurrentJoyPOV == 31500) {
//                    .Up = True
                    c.Up = true;
//                ElseIf CurrentJoyY > CenterY(JoyNum) + CenterY(JoyNum) * 0.3 Or (CurrentJoyPOV >= 13500 And CurrentJoyPOV <= 22500) Then
                } else if(CurrentJoyY > CenterY[JoyNum] + CenterY[JoyNum] * 0.3 ||
                          (CurrentJoyPOV >= 13500 && CurrentJoyPOV <= 22500)) {
//                    .Down = True
                    c.Down = true;
//                End If
                }

//                If JoyButtons(conJoystick(A).Jump) = True Then
                if(JoyButtons[conJoystick[A].Jump]) {
//                    .Jump = True
                    c.Jump = true;
//                End If
                }
//                If JoyButtons(conJoystick(A).Run) = True Then
                if(JoyButtons[conJoystick[A].Run]) {
//                    .Run = True
                    c.Run = true;
//                End If
                }
//                If JoyButtons(conJoystick(A).Drop) = True Then
                if(JoyButtons[conJoystick[A].Drop]) {
//                    .Drop = True
                    c.Drop = true;
//                End If
                }
//                If JoyButtons(conJoystick(A).Start) = True Then
                if(JoyButtons[conJoystick[A].Start]) {
//                    .Start = True
                    c.Start = true;
//                End If
                }
//                If JoyButtons(conJoystick(A).AltRun) = True Then
                if(JoyButtons[conJoystick[A].AltRun]) {
//                    .AltRun = True
                    c.AltRun = true;
//                End If
                }
//                If JoyButtons(conJoystick(A).AltJump) = True Then
                if(JoyButtons[conJoystick[A].AltJump]) {
//                    .AltJump = True
                    c.AltJump = true;
//                End If
                }
//            End If
            }
//            If useJoystick(A) = 0 Then            'Keyboard controls
            if(useJoystick[A] == 0)
            {
//                If GetKeyState(conKeyboard(A).Up) And KEY_PRESSED Then
                if(getKeyStateI(conKeyboard[A].Up)) {
//                    .Up = True
                    c.Up = true;
//                End If
                }
//                If GetKeyState(conKeyboard(A).Down) And KEY_PRESSED Then
                if(getKeyStateI(conKeyboard[A].Down)) {
//                    .Down = True
                    c.Down = true;
//                End If
                }
//                If GetKeyState(conKeyboard(A).Left) And KEY_PRESSED Then
                if(getKeyStateI(conKeyboard[A].Left)) {
//                    .Left = True
                    c.Left = true;
//                End If
                }
//                If GetKeyState(conKeyboard(A).Right) And KEY_PRESSED Then
                if(getKeyStateI(conKeyboard[A].Right)) {
//                    .Right = True
                    c.Right = true;
//                End If
                }
//                If GetKeyState(conKeyboard(A).Jump) And KEY_PRESSED Then
                if(getKeyStateI(conKeyboard[A].Jump)) {
//                    .Jump = True
                    c.Jump = true;
//                End If
                }
//                If GetKeyState(conKeyboard(A).Run) And KEY_PRESSED Then
                if(getKeyStateI(conKeyboard[A].Run)) {
//                    .Run = True
                    c.Run = true;
//                End If
                }
//                If GetKeyState(conKeyboard(A).Drop) And KEY_PRESSED Then
                if(getKeyStateI(conKeyboard[A].Drop)) {
//                    .Drop = True
                    c.Drop = true;
//                End If
                }
//                If GetKeyState(conKeyboard(A).Start) And KEY_PRESSED Then
                if(getKeyStateI(conKeyboard[A].Start)) {
//                    .Start = True
                    c.Start = true;
//                End If
                }
//                If GetKeyState(conKeyboard(A).AltJump) And KEY_PRESSED Then
                if(getKeyStateI(conKeyboard[A].AltJump)) {
//                    .AltJump = True
                    c.AltJump = true;
//                End If
                }
//                If GetKeyState(conKeyboard(A).AltRun) And KEY_PRESSED Then
                if(getKeyStateI(conKeyboard[A].AltRun)) {
//                    .AltRun = True
                    c.AltRun = true;
//                End If
                }
//            End If
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

//            If Player(A).Controls.Start = False And Player(A).Controls.Jump = False Then
            if(!Player[A].Controls.Start && !Player[A].Controls.Jump) {
//                Player(A).UnStart = True
                Player[A].UnStart = true;
//            End If
            }
//            If .Up = True And .Down = True Then
            if(c.Up && c.Down) {
//                .Up = False
                c.Up = false;
//                .Down = False
                c.Down = false;
//            End If
            }

//            If .Left = True And .Right = True Then
            if(c.Left && c.Right) {
//                .Left = False
                c.Left = false;
//                .Right = False
                c.Right = false;
//            End If
            }

//            If Not (Player(A).State = 5 And Player(A).Mount = 0) And .AltRun = True Then .Run = True
            if(!(Player[A].State == 5 && Player[A].Mount == 0) && c.AltRun)
                c.Run = true;
//            If ForcedControls = True And GamePaused = False Then
            if(ForcedControls && !GamePaused ) {
//                Player(A).Controls = ForcedControl
                Player[A].Controls = ForcedControl;
//            End If
            }
//        End With
        }
//    Next B
    }

//    If SingleCoop > 0 Then
    if(SingleCoop > 0)
    {
//        If numPlayers = 1 Or numPlayers > 2 Then SingleCoop = 0
        if(numPlayers == 1 || numPlayers > 2)
            SingleCoop = 0;
//        Dim tempControls As Controls
        Controls_t tempControls;
//        If SingleCoop = 1 Then
        if(SingleCoop == 1) {
//            Player(2).Controls = tempControls
            Player[2].Controls = tempControls;
//        Else
        } else {
//            Player(2).Controls = Player(1).Controls
            Player[2].Controls = Player[1].Controls;
//            Player(1).Controls = tempControls
            Player[1].Controls = tempControls;
//        End If
        }
//    End If
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

//    For A = 1 To numPlayers
    For(A, 1, numPlayers)
    {
//        With Player(A)
        {
            Player_t &p = Player[A];
//            If .SpinJump = True Then
            if(p.SpinJump)
            {
//                If .SpinFrame < 4 Or .SpinFrame > 9 Then
                if(p.SpinFrame < 4 || p.SpinFrame > 9) {
//                    .Direction = -1
                    p.Direction = -1;
//                Else
                } else {
//                    .Direction = 1
                    p.Direction = 1;
//                End If
                }
//            End If
            }
//        End With
        }
//    Next A
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
    Sint32 val = 0;
    Sint16 val_initial = 0;
    if(JoystickNumber < 0 || JoystickNumber >= int(g_joysticks.size()))
        return false;

    JoyNum = JoystickNumber;
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

        CenterX[JoystickNumber] = 0;
        CenterY[JoystickNumber] = 0;
        CurrentJoyX = 0;
        CurrentJoyY = 0;
        CurrentJoyPOV = -1;

        for(int i = 0; i < axes; i++)
        {
            val_initial = 0;
            val = 0;
            //Note: The SDL 2.0.6 and higher is requires to support this function
            if(SDL_JoystickGetAxisInitialState(joy, i, &val_initial) == SDL_FALSE)
                break;
            val = SDL_JoystickGetAxis(joy, i);
            if(i == 0)
            {
                CenterX[JoystickNumber] = val_initial;
                CurrentJoyX = val;
            }
            else if(i == 1)
            {
                CenterY[JoystickNumber] = val_initial;
                CurrentJoyY = val;
            }
        }

        for(int i = 0; i < buttons && i < 255; i++)
        {
            val = SDL_JoystickGetButton(joy, i);
            JoyButtons[i] = (val != 0);
        }

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

void PollJoystick(int joystick)
{
    Sint32 val = 0;
//    int dx = 0, dy = 0;

    if(joystick < 0 || joystick >= int(g_joysticks.size()))
        return;

    SDL_Joystick *joy = g_joysticks[size_t(joystick)];

//    int balls = SDL_JoystickNumBalls(joy);
    int buttons = SDL_JoystickNumButtons(joy);
    int axes = SDL_JoystickNumAxes(joy);
    int hats = SDL_JoystickNumHats(joy);

    // Reset to zero until scan
    CurrentJoyX = 0;
    CurrentJoyY = 0;

    for(int i = 0; i < hats; ++i) // scan hats first
    {
        val = SDL_JoystickGetHat(joy, i);
        switch(val)
        {
        default:
        case SDL_HAT_CENTERED:
            CurrentJoyX = 0;
            CurrentJoyY = 0;
            break;
        case SDL_HAT_LEFT:
            CurrentJoyX = -1;
            CurrentJoyY = 0;
            break;
        case SDL_HAT_RIGHT:
            CurrentJoyX = 1;
            CurrentJoyY = 0;
            break;
        case SDL_HAT_UP:
            CurrentJoyX = 0;
            CurrentJoyY = -1;
            break;
        case SDL_HAT_DOWN:
            CurrentJoyX = 0;
            CurrentJoyY = 1;
            break;
        case SDL_HAT_LEFTUP:
            CurrentJoyX = -1;
            CurrentJoyY = -1;
            break;
        case SDL_HAT_LEFTDOWN:
            CurrentJoyX = -1;
            CurrentJoyY = 1;
            break;
        case SDL_HAT_RIGHTUP:
            CurrentJoyX = 1;
            CurrentJoyY = -1;
            break;
        case SDL_HAT_RIGHTDOWN:
            CurrentJoyX = 1;
            CurrentJoyY = 1;
            break;
        }

        if(CurrentJoyX != 0 || CurrentJoyY != 0)
            break;
    }

    if(CurrentJoyX == 0 || CurrentJoyY == 0) // Scan axes if no hats toggled
    {
        for(int i = 0; i < axes; ++i)
        {
            val = SDL_JoystickGetAxis(joy, i);
            if(i == 0)
            {
                CurrentJoyX = val;
            }
            else if(i == 1)
            {
                CurrentJoyY = val;
            }
            else
                break;
        }
    }

    for(int i = 0; i < buttons && i < 255; i++) // Scan buttons
    {
        val = SDL_JoystickGetButton(joy, i);
        JoyButtons[i] = (val != 0);
    }
}
