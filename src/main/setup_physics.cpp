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

#include "../globals.h"
#include "../game_main.h"

void SetupPhysics()
{
    Physics.PlayerJumpVelocity = -5.7f;     // Jump velocity
    Physics.PlayerJumpHeight = 20;          // Jump height
    Physics.PlayerBlockJumpHeight = 25;     // Jump height off bouncy blocks
    Physics.PlayerHeadJumpHeight = 22;      // Jump height off another players head
    Physics.PlayerNPCJumpHeight = 22;       // Jump height off a NPC
    Physics.PlayerSpringJumpHeight = 55;    // Jump height off a Spring
    Physics.PlayerRunSpeed = 6;             // Max run speed
    Physics.PlayerWalkSpeed = 3;            // Max walk speed
    Physics.PlayerGravity = 0.4f;            // Player// s gravity
    Physics.PlayerTerminalVelocity = 12;    // Max falling speed
    Physics.PlayerHeight[1][1] = 30;        // Little Mario
    Physics.PlayerWidth[1][1] = 24;         // ------------
    Physics.PlayerGrabSpotX[1][1] = 18;     // ---------
    Physics.PlayerGrabSpotY[1][1] = -2;     // ---------
    Physics.PlayerHeight[1][2] = 54;        // Big Mario
    Physics.PlayerWidth[1][2] = 24;         // ---------
    Physics.PlayerDuckHeight[1][2] = 30;    // ---------
    Physics.PlayerGrabSpotX[1][2] = 18;     // ---------
    Physics.PlayerGrabSpotY[1][2] = 16;     // ---------
    Physics.PlayerHeight[1][3] = 54;        // Fire Mario
    Physics.PlayerWidth[1][3] = 24;         // ---------
    Physics.PlayerDuckHeight[1][3] = 30;    // ---------
    Physics.PlayerGrabSpotX[1][3] = 18;     // ---------
    Physics.PlayerGrabSpotY[1][3] = 16;     // ---------
    Physics.PlayerHeight[1][7] = 54;        // Ice Mario
    Physics.PlayerWidth[1][7] = 24;         // ---------
    Physics.PlayerDuckHeight[1][7] = 30;    // ---------
    Physics.PlayerGrabSpotX[1][7] = 18;     // ---------
    Physics.PlayerGrabSpotY[1][7] = 16;     // ---------
    Physics.PlayerHeight[1][4] = 54;        // Racoon Mario
    Physics.PlayerWidth[1][4] = 24;         // ---------
    Physics.PlayerDuckHeight[1][4] = 30;    // ---------
    Physics.PlayerGrabSpotX[1][4] = 18;     // ---------
    Physics.PlayerGrabSpotY[1][4] = 16;     // ---------
    Physics.PlayerHeight[1][5] = 54;        // Tanooki Mario
    Physics.PlayerWidth[1][5] = 24;         // ---------
    Physics.PlayerDuckHeight[1][5] = 30;    // ---------
    Physics.PlayerGrabSpotX[1][5] = 18;     // ---------
    Physics.PlayerGrabSpotY[1][5] = 16;     // ---------
    Physics.PlayerHeight[1][6] = 54;        // Hammer Mario
    Physics.PlayerWidth[1][6] = 24;         // ---------
    Physics.PlayerDuckHeight[1][6] = 30;    // ---------
    Physics.PlayerGrabSpotX[1][6] = 18;     // ---------
    Physics.PlayerGrabSpotY[1][6] = 16;     // ---------

    Physics.PlayerHeight[2][1] = 30;        // Little Luigi
    Physics.PlayerWidth[2][1] = 24;         // ------------
    Physics.PlayerGrabSpotX[2][1] = 16;     // ---------
    Physics.PlayerGrabSpotY[2][1] = -4;     // ---------
    Physics.PlayerHeight[2][2] = 60;        // Big Luigi
    Physics.PlayerWidth[2][2] = 24;         // ---------
    Physics.PlayerDuckHeight[2][2] = 30;    // ---------
    Physics.PlayerGrabSpotX[2][2] = 18;     // ---------
    Physics.PlayerGrabSpotY[2][2] = 16;     // ---------
    Physics.PlayerHeight[2][3] = 60;        // Fire Luigi
    Physics.PlayerWidth[2][3] = 24;         // ---------
    Physics.PlayerDuckHeight[2][3] = 30;    // ---------
    Physics.PlayerGrabSpotX[2][3] = 18;     // ---------
    Physics.PlayerGrabSpotY[2][3] = 16;     // ---------
    Physics.PlayerHeight[2][4] = 60;        // Racoon Luigi
    Physics.PlayerWidth[2][4] = 24;         // ---------
    Physics.PlayerDuckHeight[2][4] = 30;    // ---------
    Physics.PlayerGrabSpotX[2][4] = 18;     // ---------
    Physics.PlayerGrabSpotY[2][4] = 16;     // ---------
    Physics.PlayerHeight[2][5] = 60;        // Tanooki Luigi
    Physics.PlayerWidth[2][5] = 24;         // ---------
    Physics.PlayerDuckHeight[2][5] = 30;    // ---------
    Physics.PlayerGrabSpotX[2][5] = 18;     // ---------
    Physics.PlayerGrabSpotY[2][5] = 16;     // ---------
    Physics.PlayerHeight[2][6] = 60;        // Tanooki Luigi
    Physics.PlayerWidth[2][6] = 24;         // ---------
    Physics.PlayerDuckHeight[2][6] = 30;    // ---------
    Physics.PlayerGrabSpotX[2][6] = 18;     // ---------
    Physics.PlayerGrabSpotY[2][6] = 16;     // ---------
    Physics.PlayerHeight[2][7] = 60;        // Ice Luigi
    Physics.PlayerWidth[2][7] = 24;         // ---------
    Physics.PlayerDuckHeight[2][7] = 30;    // ---------
    Physics.PlayerGrabSpotX[2][7] = 18;     // ---------
    Physics.PlayerGrabSpotY[2][7] = 16;     // ---------

    Physics.PlayerHeight[3][1] = 38;        // Little Peach
    Physics.PlayerDuckHeight[3][1] = 26;    // ---------
    Physics.PlayerWidth[3][1] = 24;         // ------------
    Physics.PlayerGrabSpotX[3][1] = 0;      // ---------
    Physics.PlayerGrabSpotY[3][1] = 0;      // ---------
    Physics.PlayerHeight[3][2] = 60;        // Big Peach
    Physics.PlayerWidth[3][2] = 24;         // ---------
    Physics.PlayerDuckHeight[3][2] = 30;    // ---------
    Physics.PlayerGrabSpotX[3][2] = 0;     // ---------
    Physics.PlayerGrabSpotY[3][2] = 0;     // ---------
    Physics.PlayerHeight[3][3] = 60;        // Fire Peach
    Physics.PlayerWidth[3][3] = 24;         // ---------
    Physics.PlayerDuckHeight[3][3] = 30;    // ---------
    Physics.PlayerGrabSpotX[3][3] = 18;
    Physics.PlayerGrabSpotY[3][3] = 16;

    Physics.PlayerHeight[3][4] = 60;        // Racoon Peach
    Physics.PlayerWidth[3][4] = 24;         // ---------
    Physics.PlayerDuckHeight[3][4] = 30;    // ---------
    Physics.PlayerGrabSpotX[3][4] = 18;
    Physics.PlayerGrabSpotY[3][4] = 16;

    Physics.PlayerHeight[3][5] = 60;        // Tanooki Peach
    Physics.PlayerWidth[3][5] = 24;         // ---------
    Physics.PlayerDuckHeight[3][5] = 30;    // ---------
    Physics.PlayerGrabSpotX[3][5] = 18;
    Physics.PlayerGrabSpotY[3][5] = 16;

    Physics.PlayerHeight[3][6] = 60;        // Hammer Peach
    Physics.PlayerWidth[3][6] = 24;         // ---------
    Physics.PlayerDuckHeight[3][6] = 30;    // ---------
    Physics.PlayerGrabSpotX[3][6] = 18;
    Physics.PlayerGrabSpotY[3][6] = 16;


    Physics.PlayerHeight[3][7] = 60;        // Ice Peach
    Physics.PlayerWidth[3][7] = 24;         // ---------
    Physics.PlayerDuckHeight[3][7] = 30;    // ---------
    Physics.PlayerGrabSpotX[3][7] = 18;
    Physics.PlayerGrabSpotY[3][7] = 16;

    Physics.PlayerHeight[4][1] = 30;        // Little Toad
    Physics.PlayerWidth[4][1] = 24;         // ------------
    Physics.PlayerDuckHeight[4][1] = 26;    // ---------
    Physics.PlayerGrabSpotX[4][1] = 18;     // ---------
    Physics.PlayerGrabSpotY[4][1] = -2;     // ---------
    Physics.PlayerHeight[4][2] = 50;        // Big Toad
    Physics.PlayerWidth[4][2] = 24;         // ---------
    Physics.PlayerDuckHeight[4][2] = 30;    // ---------
    Physics.PlayerGrabSpotX[4][2] = 18;     // ---------
    Physics.PlayerGrabSpotY[4][2] = 16;     // ---------
    Physics.PlayerHeight[4][3] = 50;        // Fire Toad
    Physics.PlayerWidth[4][3] = 24;         // ---------
    Physics.PlayerDuckHeight[4][3] = 30;    // ---------
    Physics.PlayerGrabSpotX[4][3] = 18;     // ---------
    Physics.PlayerGrabSpotY[4][3] = 16;     // ---------

    Physics.PlayerHeight[4][4] = 50;        // Racoon Toad
    Physics.PlayerWidth[4][4] = 24;         // ---------
    Physics.PlayerDuckHeight[4][4] = 30;    // ---------
    Physics.PlayerGrabSpotX[4][4] = 18;     // ---------
    Physics.PlayerGrabSpotY[4][4] = 16;     // ---------

    Physics.PlayerHeight[4][5] = 50;        // Tanooki Toad
    Physics.PlayerWidth[4][5] = 24;         // ---------
    Physics.PlayerDuckHeight[4][5] = 30;    // ---------
    Physics.PlayerGrabSpotX[4][5] = 18;     // ---------
    Physics.PlayerGrabSpotY[4][5] = 16;     // ---------

    Physics.PlayerHeight[4][6] = 50;        // Hammer Toad
    Physics.PlayerWidth[4][6] = 24;         // ---------
    Physics.PlayerDuckHeight[4][6] = 30;    // ---------
    Physics.PlayerGrabSpotX[4][6] = 18;     // ---------
    Physics.PlayerGrabSpotY[4][6] = 16;     // ---------

    Physics.PlayerHeight[4][7] = 50;        // Ice Toad
    Physics.PlayerWidth[4][7] = 24;         // ---------
    Physics.PlayerDuckHeight[4][7] = 30;    // ---------
    Physics.PlayerGrabSpotX[4][7] = 18;     // ---------
    Physics.PlayerGrabSpotY[4][7] = 16;     // ---------

    Physics.PlayerHeight[5][1] = 54;        // Green Link
    Physics.PlayerWidth[5][1] = 22;         // ---------
    Physics.PlayerDuckHeight[5][1] = 44;    // ---------
    Physics.PlayerGrabSpotX[5][1] = 18;     // ---------
    Physics.PlayerGrabSpotY[5][1] = 16;     // ---------

    Physics.PlayerHeight[5][2] = 54;        // Green Link
    Physics.PlayerWidth[5][2] = 22;         // ---------
    Physics.PlayerDuckHeight[5][2] = 44;    // ---------
    Physics.PlayerGrabSpotX[5][2] = 18;     // ---------
    Physics.PlayerGrabSpotY[5][2] = 16;     // ---------

    Physics.PlayerHeight[5][3] = 54;        // Fire Link
    Physics.PlayerWidth[5][3] = 22;         // ---------
    Physics.PlayerDuckHeight[5][3] = 44;    // ---------
    Physics.PlayerGrabSpotX[5][3] = 18;     // ---------
    Physics.PlayerGrabSpotY[5][3] = 16;     // ---------

    Physics.PlayerHeight[5][4] = 54;        // Blue Link
    Physics.PlayerWidth[5][4] = 22;         // ---------
    Physics.PlayerDuckHeight[5][4] = 44;    // ---------
    Physics.PlayerGrabSpotX[5][4] = 18;     // ---------
    Physics.PlayerGrabSpotY[5][4] = 16;     // ---------

    Physics.PlayerHeight[5][5] = 54;        // IronKnuckle Link
    Physics.PlayerWidth[5][5] = 22;         // ---------
    Physics.PlayerDuckHeight[5][5] = 44;    // ---------
    Physics.PlayerGrabSpotX[5][5] = 18;     // ---------
    Physics.PlayerGrabSpotY[5][5] = 16;     // ---------

    Physics.PlayerHeight[5][6] = 54;        // Shadow Link
    Physics.PlayerWidth[5][6] = 22;         // ---------
    Physics.PlayerDuckHeight[5][6] = 44;    // ---------
    Physics.PlayerGrabSpotX[5][6] = 18;     // ---------
    Physics.PlayerGrabSpotY[5][6] = 16;     // ---------

    Physics.PlayerHeight[5][7] = 54;        // Ice Link
    Physics.PlayerWidth[5][7] = 22;         // ---------
    Physics.PlayerDuckHeight[5][7] = 44;    // ---------
    Physics.PlayerGrabSpotX[5][7] = 18;     // ---------
    Physics.PlayerGrabSpotY[5][7] = 16;     // ---------

    Physics.NPCTimeOffScreen = 180;         // How long NPCs are active offscreen before being reset
    Physics.NPCShellSpeed = 7.1f;            // Speed of kicked shells
    Physics.NPCShellSpeedY = 11;            // Vertical Speed of kicked shells
    Physics.NPCCanHurtWait = 30;            // How long to wait before NPCs can hurt players
    Physics.NPCGravity = 0.26f;              // NPC Gravity
    Physics.NPCGravityReal = 0.26f;              // NPC Gravity
    Physics.NPCWalkingSpeed = 1.2f;          // NPC Walking Speed
    Physics.NPCWalkingOnSpeed = 1;          // NPC that can be walked on walking speed
    Physics.NPCMushroomSpeed = 1.8f;         // Mushroom X Speed
    Physics.NPCPSwitch = 777;               // P Switch time
}
