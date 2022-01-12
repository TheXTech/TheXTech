#include "globals.h"
#include "layers.h"
#include "../lunaplayer.h"
#include "../lunalayer.h"
#include "Docopoper-AbstractAssault.h"


#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define CLAMP(x, l, h) (((x) > (h)) ? (h) : (((x) < (l)) ? (l) : (x)))


void AbstractAssaultCode()
{
    Player_t *demo = PlayerF::Get(1);
    Layer_t *layerDefault = LayerF::Get(0);
    Layer_t *layerStartingPlatform = LayerF::Get(3);

    static float hspeed = 0, vspeed = 0;
    static char gameStarted = 0;
    static unsigned short noControlTimer = 0;

    bool press_up = demo->Controls.Up;
    bool press_left = demo->Controls.Left;
    bool press_down = demo->Controls.Down;
    bool press_right = demo->Controls.Right;

    auto &powerup = demo->State;

    if(layerStartingPlatform -> SpeedY == 0)
    {
        demo -> Character = 1; //Demo
        //player_id_set = 1;
        gameStarted = 0;
        powerup = 1;
    }
    else if(!gameStarted)
    {
        powerup = 6;
        demo -> SpinJump = 0;
        demo -> Hearts = 3;
        vspeed = -16;
        hspeed = -4;
        gameStarted = 1;
        noControlTimer = 30;
        demo->Character = 5; //Sheath
    }

    if(gameStarted)
    {
        if(layerStartingPlatform -> SpeedY == 0)
            gameStarted = 0;

        demo -> SpinJump = 0;

        layerDefault -> SpeedX = (float) MAX(layerDefault -> SpeedX - 0.015, -2.5);

        if(demo -> Hearts > 1)
            powerup = 6;

        demo -> Direction = 1;
        demo -> Multiplier %= 9;

        if(noControlTimer == 0)
        {
            vspeed = (float) CLAMP(vspeed + (press_down - press_up) * 0.5, -10, 10);
            hspeed = (float) CLAMP(hspeed + (press_right - press_left) * 0.5, -10, 10);
        }
        else
            noControlTimer--;

        demo->Location.SpeedY = -0.4 + vspeed;
        demo->Location.SpeedX = hspeed;

        hspeed *= (float) 0.9;
        vspeed *= (float) 0.9;
    }
}
