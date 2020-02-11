#include "../globals.h"
#include "../game_main.h"

void SetupPhysics()
{
    physics.PlayerJumpVelocity = -5.7f;     // Jump velocity
    physics.PlayerJumpHeight = 20;          // Jump height
    physics.PlayerBlockJumpHeight = 25;     // Jump height off bouncy blocks
    physics.PlayerHeadJumpHeight = 22;      // Jump height off another players head
    physics.PlayerNPCJumpHeight = 22;       // Jump height off a NPC
    physics.PlayerSpringJumpHeight = 55;    // Jump height off a Spring
    physics.PlayerRunSpeed = 6;             // Max run speed
    physics.PlayerWalkSpeed = 3;            // Max walk speed
    physics.PlayerGravity = 0.4f;            // Player// s gravity
    physics.PlayerTerminalVelocity = 12;    // Max falling speed
    physics.PlayerHeight[1][1] = 30;        // Little Mario
    physics.PlayerWidth[1][1] = 24;         // ------------
    physics.PlayerGrabSpotX[1][1] = 18;     // ---------
    physics.PlayerGrabSpotY[1][1] = -2;     // ---------
    physics.PlayerHeight[1][2] = 54;        // Big Mario
    physics.PlayerWidth[1][2] = 24;         // ---------
    physics.PlayerDuckHeight[1][2] = 30;    // ---------
    physics.PlayerGrabSpotX[1][2] = 18;     // ---------
    physics.PlayerGrabSpotY[1][2] = 16;     // ---------
    physics.PlayerHeight[1][3] = 54;        // Fire Mario
    physics.PlayerWidth[1][3] = 24;         // ---------
    physics.PlayerDuckHeight[1][3] = 30;    // ---------
    physics.PlayerGrabSpotX[1][3] = 18;     // ---------
    physics.PlayerGrabSpotY[1][3] = 16;     // ---------
    physics.PlayerHeight[1][7] = 54;        // Ice Mario
    physics.PlayerWidth[1][7] = 24;         // ---------
    physics.PlayerDuckHeight[1][7] = 30;    // ---------
    physics.PlayerGrabSpotX[1][7] = 18;     // ---------
    physics.PlayerGrabSpotY[1][7] = 16;     // ---------
    physics.PlayerHeight[1][4] = 54;        // Racoon Mario
    physics.PlayerWidth[1][4] = 24;         // ---------
    physics.PlayerDuckHeight[1][4] = 30;    // ---------
    physics.PlayerGrabSpotX[1][4] = 18;     // ---------
    physics.PlayerGrabSpotY[1][4] = 16;     // ---------
    physics.PlayerHeight[1][5] = 54;        // Tanooki Mario
    physics.PlayerWidth[1][5] = 24;         // ---------
    physics.PlayerDuckHeight[1][5] = 30;    // ---------
    physics.PlayerGrabSpotX[1][5] = 18;     // ---------
    physics.PlayerGrabSpotY[1][5] = 16;     // ---------
    physics.PlayerHeight[1][6] = 54;        // Hammer Mario
    physics.PlayerWidth[1][6] = 24;         // ---------
    physics.PlayerDuckHeight[1][6] = 30;    // ---------
    physics.PlayerGrabSpotX[1][6] = 18;     // ---------
    physics.PlayerGrabSpotY[1][6] = 16;     // ---------

    physics.PlayerHeight[2][1] = 30;        // Little Luigi
    physics.PlayerWidth[2][1] = 24;         // ------------
    physics.PlayerGrabSpotX[2][1] = 16;     // ---------
    physics.PlayerGrabSpotY[2][1] = -4;     // ---------
    physics.PlayerHeight[2][2] = 60;        // Big Luigi
    physics.PlayerWidth[2][2] = 24;         // ---------
    physics.PlayerDuckHeight[2][2] = 30;    // ---------
    physics.PlayerGrabSpotX[2][2] = 18;     // ---------
    physics.PlayerGrabSpotY[2][2] = 16;     // ---------
    physics.PlayerHeight[2][3] = 60;        // Fire Luigi
    physics.PlayerWidth[2][3] = 24;         // ---------
    physics.PlayerDuckHeight[2][3] = 30;    // ---------
    physics.PlayerGrabSpotX[2][3] = 18;     // ---------
    physics.PlayerGrabSpotY[2][3] = 16;     // ---------
    physics.PlayerHeight[2][4] = 60;        // Racoon Luigi
    physics.PlayerWidth[2][4] = 24;         // ---------
    physics.PlayerDuckHeight[2][4] = 30;    // ---------
    physics.PlayerGrabSpotX[2][4] = 18;     // ---------
    physics.PlayerGrabSpotY[2][4] = 16;     // ---------
    physics.PlayerHeight[2][5] = 60;        // Tanooki Luigi
    physics.PlayerWidth[2][5] = 24;         // ---------
    physics.PlayerDuckHeight[2][5] = 30;    // ---------
    physics.PlayerGrabSpotX[2][5] = 18;     // ---------
    physics.PlayerGrabSpotY[2][5] = 16;     // ---------
    physics.PlayerHeight[2][6] = 60;        // Tanooki Luigi
    physics.PlayerWidth[2][6] = 24;         // ---------
    physics.PlayerDuckHeight[2][6] = 30;    // ---------
    physics.PlayerGrabSpotX[2][6] = 18;     // ---------
    physics.PlayerGrabSpotY[2][6] = 16;     // ---------
    physics.PlayerHeight[2][7] = 60;        // Ice Luigi
    physics.PlayerWidth[2][7] = 24;         // ---------
    physics.PlayerDuckHeight[2][7] = 30;    // ---------
    physics.PlayerGrabSpotX[2][7] = 18;     // ---------
    physics.PlayerGrabSpotY[2][7] = 16;     // ---------

    physics.PlayerHeight[3][1] = 38;        // Little Peach
    physics.PlayerDuckHeight[3][1] = 26;    // ---------
    physics.PlayerWidth[3][1] = 24;         // ------------
    physics.PlayerGrabSpotX[3][1] = 0;      // ---------
    physics.PlayerGrabSpotY[3][1] = 0;      // ---------
    physics.PlayerHeight[3][2] = 60;        // Big Peach
    physics.PlayerWidth[3][2] = 24;         // ---------
    physics.PlayerDuckHeight[3][2] = 30;    // ---------
    physics.PlayerGrabSpotX[3][2] = 0;     // ---------
    physics.PlayerGrabSpotY[3][2] = 0;     // ---------
    physics.PlayerHeight[3][3] = 60;        // Fire Peach
    physics.PlayerWidth[3][3] = 24;         // ---------
    physics.PlayerDuckHeight[3][3] = 30;    // ---------
    physics.PlayerGrabSpotX[3][3] = 18;
    physics.PlayerGrabSpotY[3][3] = 16;

    physics.PlayerHeight[3][4] = 60;        // Racoon Peach
    physics.PlayerWidth[3][4] = 24;         // ---------
    physics.PlayerDuckHeight[3][4] = 30;    // ---------
    physics.PlayerGrabSpotX[3][4] = 18;
    physics.PlayerGrabSpotY[3][4] = 16;

    physics.PlayerHeight[3][5] = 60;        // Tanooki Peach
    physics.PlayerWidth[3][5] = 24;         // ---------
    physics.PlayerDuckHeight[3][5] = 30;    // ---------
    physics.PlayerGrabSpotX[3][5] = 18;
    physics.PlayerGrabSpotY[3][5] = 16;

    physics.PlayerHeight[3][6] = 60;        // Hammer Peach
    physics.PlayerWidth[3][6] = 24;         // ---------
    physics.PlayerDuckHeight[3][6] = 30;    // ---------
    physics.PlayerGrabSpotX[3][6] = 18;
    physics.PlayerGrabSpotY[3][6] = 16;


    physics.PlayerHeight[3][7] = 60;        // Ice Peach
    physics.PlayerWidth[3][7] = 24;         // ---------
    physics.PlayerDuckHeight[3][7] = 30;    // ---------
    physics.PlayerGrabSpotX[3][7] = 18;
    physics.PlayerGrabSpotY[3][7] = 16;

    physics.PlayerHeight[4][1] = 30;        // Little Toad
    physics.PlayerWidth[4][1] = 24;         // ------------
    physics.PlayerDuckHeight[4][1] = 26;    // ---------
    physics.PlayerGrabSpotX[4][1] = 18;     // ---------
    physics.PlayerGrabSpotY[4][1] = -2;     // ---------
    physics.PlayerHeight[4][2] = 50;        // Big Toad
    physics.PlayerWidth[4][2] = 24;         // ---------
    physics.PlayerDuckHeight[4][2] = 30;    // ---------
    physics.PlayerGrabSpotX[4][2] = 18;     // ---------
    physics.PlayerGrabSpotY[4][2] = 16;     // ---------
    physics.PlayerHeight[4][3] = 50;        // Fire Toad
    physics.PlayerWidth[4][3] = 24;         // ---------
    physics.PlayerDuckHeight[4][3] = 30;    // ---------
    physics.PlayerGrabSpotX[4][3] = 18;     // ---------
    physics.PlayerGrabSpotY[4][3] = 16;     // ---------

    physics.PlayerHeight[4][4] = 50;        // Racoon Toad
    physics.PlayerWidth[4][4] = 24;         // ---------
    physics.PlayerDuckHeight[4][4] = 30;    // ---------
    physics.PlayerGrabSpotX[4][4] = 18;     // ---------
    physics.PlayerGrabSpotY[4][4] = 16;     // ---------

    physics.PlayerHeight[4][5] = 50;        // Tanooki Toad
    physics.PlayerWidth[4][5] = 24;         // ---------
    physics.PlayerDuckHeight[4][5] = 30;    // ---------
    physics.PlayerGrabSpotX[4][5] = 18;     // ---------
    physics.PlayerGrabSpotY[4][5] = 16;     // ---------

    physics.PlayerHeight[4][6] = 50;        // Hammer Toad
    physics.PlayerWidth[4][6] = 24;         // ---------
    physics.PlayerDuckHeight[4][6] = 30;    // ---------
    physics.PlayerGrabSpotX[4][6] = 18;     // ---------
    physics.PlayerGrabSpotY[4][6] = 16;     // ---------

    physics.PlayerHeight[4][7] = 50;        // Ice Toad
    physics.PlayerWidth[4][7] = 24;         // ---------
    physics.PlayerDuckHeight[4][7] = 30;    // ---------
    physics.PlayerGrabSpotX[4][7] = 18;     // ---------
    physics.PlayerGrabSpotY[4][7] = 16;     // ---------

    physics.PlayerHeight[5][1] = 54;        // Green Link
    physics.PlayerWidth[5][1] = 22;         // ---------
    physics.PlayerDuckHeight[5][1] = 44;    // ---------
    physics.PlayerGrabSpotX[5][1] = 18;     // ---------
    physics.PlayerGrabSpotY[5][1] = 16;     // ---------

    physics.PlayerHeight[5][2] = 54;        // Green Link
    physics.PlayerWidth[5][2] = 22;         // ---------
    physics.PlayerDuckHeight[5][2] = 44;    // ---------
    physics.PlayerGrabSpotX[5][2] = 18;     // ---------
    physics.PlayerGrabSpotY[5][2] = 16;     // ---------

    physics.PlayerHeight[5][3] = 54;        // Fire Link
    physics.PlayerWidth[5][3] = 22;         // ---------
    physics.PlayerDuckHeight[5][3] = 44;    // ---------
    physics.PlayerGrabSpotX[5][3] = 18;     // ---------
    physics.PlayerGrabSpotY[5][3] = 16;     // ---------

    physics.PlayerHeight[5][4] = 54;        // Blue Link
    physics.PlayerWidth[5][4] = 22;         // ---------
    physics.PlayerDuckHeight[5][4] = 44;    // ---------
    physics.PlayerGrabSpotX[5][4] = 18;     // ---------
    physics.PlayerGrabSpotY[5][4] = 16;     // ---------

    physics.PlayerHeight[5][5] = 54;        // IronKnuckle Link
    physics.PlayerWidth[5][5] = 22;         // ---------
    physics.PlayerDuckHeight[5][5] = 44;    // ---------
    physics.PlayerGrabSpotX[5][5] = 18;     // ---------
    physics.PlayerGrabSpotY[5][5] = 16;     // ---------

    physics.PlayerHeight[5][6] = 54;        // Shadow Link
    physics.PlayerWidth[5][6] = 22;         // ---------
    physics.PlayerDuckHeight[5][6] = 44;    // ---------
    physics.PlayerGrabSpotX[5][6] = 18;     // ---------
    physics.PlayerGrabSpotY[5][6] = 16;     // ---------

    physics.PlayerHeight[5][7] = 54;        // Ice Link
    physics.PlayerWidth[5][7] = 22;         // ---------
    physics.PlayerDuckHeight[5][7] = 44;    // ---------
    physics.PlayerGrabSpotX[5][7] = 18;     // ---------
    physics.PlayerGrabSpotY[5][7] = 16;     // ---------

    physics.NPCTimeOffScreen = 180;         // How long NPCs are active offscreen before being reset
    physics.NPCShellSpeed = 7.1f;            // Speed of kicked shells
    physics.NPCShellSpeedY = 11;            // Vertical Speed of kicked shells
    physics.NPCCanHurtWait = 30;            // How long to wait before NPCs can hurt players
    physics.NPCGravity = 0.26f;              // NPC Gravity
    physics.NPCGravityReal = 0.26f;              // NPC Gravity
    physics.NPCWalkingSpeed = 1.2f;          // NPC Walking Speed
    physics.NPCWalkingOnSpeed = 1;          // NPC that can be walked on walking speed
    physics.NPCMushroomSpeed = 1.8f;         // Mushroom X Speed
    physics.NPCPSwitch = 777;               // P Switch time
}
