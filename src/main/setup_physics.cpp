/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2025 Vitaly Novichkov <admin@wohlnet.ru>
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

#include "../globals.h"
#include "../game_main.h"

void SetupPhysics()
{
    Physics.PlayerJumpVelocity = -(num_t)5.7_nf; // Jump velocity
    Physics.PlayerJumpHeight = 20;          // Jump height
    Physics.PlayerBlockJumpHeight = 25;     // Jump height off bouncy blocks
    Physics.PlayerHeadJumpHeight = 22;      // Jump height off another players head
    Physics.PlayerNPCJumpHeight = 22;       // Jump height off a NPC
    Physics.PlayerSpringJumpHeight = 55;    // Jump height off a Spring
    Physics.PlayerRunSpeed = 6;             // Max run speed
    Physics.PlayerWalkSpeed = 3;            // Max walk speed
    Physics.PlayerGravity = (num_t)0.4_nf;  // Player's gravity
    Physics.PlayerTerminalVelocity = 12;    // Max falling speed
    Physics.PlayerHeight[1][1] = 30;        // Little Mario
    Physics.PlayerWidth[1][1] = 24;         // ------------
    Physics.PlayerGrabSpotX[1][1] = 18;     // ---------
    Physics.PlayerGrabSpotY[1][1] = -2;     // ---------

    for(int i = 2; i <= numStates; i++)
    {
        Physics.PlayerHeight[1][i] = 54;        // Big Char1
        Physics.PlayerWidth[1][i] = 24;         // ---------
        Physics.PlayerDuckHeight[1][i] = 30;    // ---------
        Physics.PlayerGrabSpotX[1][i] = 18;     // ---------
        Physics.PlayerGrabSpotY[1][i] = 16;     // ---------
    }

    Physics.PlayerHeight[2][1] = 30;        // Little Luigi
    Physics.PlayerWidth[2][1] = 24;         // ------------
    Physics.PlayerGrabSpotX[2][1] = 16;     // ---------
    Physics.PlayerGrabSpotY[2][1] = -4;     // ---------

    for(int i = 2; i <= numStates; i++)
    {
        Physics.PlayerHeight[2][i] = 60;        // Big Char2
        Physics.PlayerWidth[2][i] = 24;         // ---------
        Physics.PlayerDuckHeight[2][i] = 30;    // ---------
        Physics.PlayerGrabSpotX[2][i] = 18;     // ---------
        Physics.PlayerGrabSpotY[2][i] = 16;     // ---------
    }

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

    for(int i = 3; i <= numStates; i++)
    {
        Physics.PlayerHeight[3][i] = 60;        // Power Char3
        Physics.PlayerWidth[3][i] = 24;         // ---------
        Physics.PlayerDuckHeight[3][i] = 30;    // ---------
        Physics.PlayerGrabSpotX[3][i] = 18;
        Physics.PlayerGrabSpotY[3][i] = 16;
    }

    Physics.PlayerHeight[4][1] = 30;        // Little Toad
    Physics.PlayerWidth[4][1] = 24;         // ------------
    Physics.PlayerDuckHeight[4][1] = 26;    // ---------
    Physics.PlayerGrabSpotX[4][1] = 18;     // ---------
    Physics.PlayerGrabSpotY[4][1] = -2;     // ---------

    for(int i = 2; i <= numStates; i++)
    {
        Physics.PlayerHeight[4][i] = 50;        // Big Char3
        Physics.PlayerWidth[4][i] = 24;         // ---------
        Physics.PlayerDuckHeight[4][i] = 30;    // ---------
        Physics.PlayerGrabSpotX[4][i] = 18;     // ---------
        Physics.PlayerGrabSpotY[4][i] = 16;     // ---------
    }

    for(int i = 1; i <= numStates; i++)
    {
        Physics.PlayerHeight[5][i] = 54;        // Char5
        Physics.PlayerWidth[5][i] = 22;         // ---------
        Physics.PlayerDuckHeight[5][i] = 44;    // ---------
        Physics.PlayerGrabSpotX[5][i] = 18;     // ---------
        Physics.PlayerGrabSpotY[5][i] = 16;     // ---------
    }

    Physics.NPCTimeOffScreen = 180;                     // How long NPCs are active offscreen before being reset
    Physics.NPCShellSpeed = (num_t)7.1_nf;              // Speed of kicked shells
    Physics.NPCShellSpeedY = 11;                        // Vertical Speed of kicked shells
    Physics.NPCCanHurtWait = 30;                        // How long to wait before NPCs can hurt players
    Physics.NPCGravityReal = 0.26_nf;                   // NPC Gravity
    Physics.NPCGravity = (num_t)Physics.NPCGravityReal; // NPC Gravity
    Physics.NPCWalkingSpeed = (num_t)1.2_nf;            // NPC Walking Speed
    Physics.NPCWalkingOnSpeed = 1;                      // NPC that can be walked on walking speed
    Physics.NPCMushroomSpeed = (num_t)1.8_nf;           // Mushroom X Speed
    Physics.NPCPSwitch = 777;                           // P Switch time
}
