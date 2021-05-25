#include <3ds.h>
#include "n3ds-clock.h"
#include <cstdio>

int n3ds_clocked = 0; // eventually move elsewhere

void InitClockSpeed()
{
    bool isN3DS;
    APT_CheckNew3DS(&isN3DS);
    if (!isN3DS) n3ds_clocked = -1;
    // I've made this configurable.
    // if (isN3DS) // make this configurable...
    // {
    //     SwapClockSpeed();
    //     printf("N3DS clock enabled\n");
    // }
}

void SwapClockSpeed()
{
    if (n3ds_clocked == -1) return;
    n3ds_clocked = !n3ds_clocked;
    osSetSpeedupEnable(n3ds_clocked);
}
