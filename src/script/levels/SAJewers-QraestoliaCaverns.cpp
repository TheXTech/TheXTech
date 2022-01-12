#include "SAJewers-QraestoliaCaverns.h"
#include "globals.h"
#include "../lunaplayer.h"
#include "../lunainput.h"

void QraestoliaCavernsCode()
{
    static int lastDownPress = 0;
    Player_t *demo = PlayerF::Get(1);
    if(demo == 0)
        return;

    // Player pressed down, what do we do?
    if(PlayerF::PressingDown(demo))
    {
        // Else, see if pressed down in the last 7 frames
        if(gFrames < lastDownPress + 10 && gFrames > lastDownPress + 1)
        {
            if(demo->Character == 2 && demo->MountType != 0)
                PlayerF::CycleLeft(demo);
            else
                PlayerF::CycleRight(demo);

            lastDownPress = gFrames - 9;
            return;
        }

        // Else, set last press frame as this one
        lastDownPress = gFrames;
    }
}
