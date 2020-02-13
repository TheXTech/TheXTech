#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <string>
#include "range_arr.hpp"

struct JOYCAPS_t
{
    //     wMid As Long
    long wMid = 0;
    //     wPid As Long
    long wPid = 0;
    //     szPname As String * 32
    std::string szPname;
    //     wXmin As Long
    long wXmin = 0;
    //     wXmax As Long
    long wXmax = 0;
    //     wYmin As Long
    long wYmin = 0;
    //     wYmax As Long
    long wYmax = 0;
    //     wZmin As Long
    long wZmin = 0;
    //     wZmax As Long
    long wZmax = 0;
    //     wNumButtons As Long
    long wNumButtons = 0;
    //     wPeriodMin As Long
    long wPeriodMin = 0;
    //     wPeriodMax As Long
    long wPeriodMax = 0;
    //     wRmin As Long
    long wRmin = 0;
    //     wRmax As Long
    long wRmax = 0;
    //     wUmin As Long
    long wUmin = 0;
    //     wUmax As Long
    long wUmax = 0;
    //     wVmin As Long
    long wVmin = 0;
    //     wVmax As Long
    long wVmax = 0;
    //     wCaps As Long
    long wCaps = 0;
    //     wMaxAxes As Long
    long wMaxAxes = 0;
    //     wNumAxes As Long
    long wNumAxes = 0;
    //     wMaxButtons As Long
    long wMaxButtons = 0;
    //     szRegKey As String * 32
    std::string szRegKey;
    //     szOEMVxD As String * 260
    std::string szOEMVxD;
};

struct JOYINFOEX_t
{
    //     dwSize As Long
    long dwSize = 0;
    //     dwFlags As Long
    long dwFlags = 0;
    //     dwXpos As Long
    long dwXpos = 0;
    //     dwYpos As Long
    long dwYpos = 0;
    //     dwZpos As Long
    long dwZpos = 0;
    //     dwRpos As Long
    long dwRpos = 0;
    //     dwUpos As Long
    long dwUpos = 0;
    //     dwVpos As Long
    long dwVpos = 0;
    //     dwButtons As Long
    long dwButtons = 0;
    //     dwButtonNumber As Long
    long dwButtonNumber = 0;
    //     dwPOV As Long
    long dwPOV = 0;
    //     dwReserved1 As Long
    long dwReserved1 = 0;
    //     dwReserved2 As Long
    long dwReserved2 = 0;
};


// Public JoyNum As Long
extern long JoyNum;
// Public MYJOYEX As JOYINFOEX
extern JOYINFOEX_t MYJOYEX;
// Public MYJOYCAPS As JOYCAPS
extern JOYCAPS_t MYJOYCAPS;
// Public CenterX(0 To 7) As Long
extern RangeArr<long, 0, 7> CenterX;
// Public CenterY(0 To 7) As Long
extern RangeArr<long, 0, 7> CenterY;
// Public JoyButtons(-15 To 15) As Boolean
extern RangeArr<bool, -15, 15> JoyButtons;
// Public CurrentJoyX As Long
extern long CurrentJoyX;
// Public CurrentJoyY As Long
extern long CurrentJoyY;
// Public CurrentJoyPOV As Long
extern long CurrentJoyPOV;


// Public Sub UpdateControls() 'Gets players controls
// Gets players controls
void UpdateControls();
// Public Function StartJoystick(Optional ByVal JoystickNumber As Integer = 0) As Boolean
bool StartJoystick(int JoystickNumber);
// Public Sub PollJoystick()
void PollJoystick();


#endif // JOYSTICK_H
