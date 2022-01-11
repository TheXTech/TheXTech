#pragma once
#ifndef LUNAINPUT_H
#define LUNAINPUT_H

struct Player_t;

extern int gFrames;

extern int gLastDownPress;
extern int gDownTapped;
extern int gLastUpPress;
extern int gUpTapped;
extern int gLastLeftPress;
extern int gLeftTapped;
extern int gLastRightPress;
extern int gRightTapped;

extern int gLastJumpPress;
extern int gJumpTapped;
extern int gLastRunPress;
extern int gRunTapped;


namespace Input {

void CheckSpecialCheats();

void UpdateInputTasks();

void ResetTaps();
void ResetAll();
void UpdateKeyRecords(Player_t* player);

bool PressingUp();
bool PressingDown();
bool PressingLeft();
bool PressingRight();
bool PressingRun();
bool PressingJump();

bool UpThisFrame();
bool DownThisFrame();
bool LeftThisFrame();
bool RightThisFrame();
bool RunThisFrame();
bool JumpThisFrame();

}


#endif // LUNAINPUT_H
