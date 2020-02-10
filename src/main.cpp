#include "game_main.h"
#include <SDL2/SDL.h>

extern "C"
int main(int argc, char**argv)
{
    frmMain.initSDL();
    int ret = GameMain(argc, argv);
    frmMain.freeSDL();
    return ret;
}
