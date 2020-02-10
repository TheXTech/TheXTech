#ifndef JOYSTICK_H
#define JOYSTICK_H

// Public Sub UpdateControls() 'Gets players controls
// Gets players controls
void UpdateControls();
// Public Function StartJoystick(Optional ByVal JoystickNumber As Integer = 0) As Boolean
bool StartJoystick(int JoystickNumber);
// Public Sub PollJoystick()
void PollJoystick();


#endif // JOYSTICK_H
