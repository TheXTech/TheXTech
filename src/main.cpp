#include "game_main.h"
#include <SDL2/SDL.h>
#include <AppPath/app_path.h>

extern "C"
int main(int argc, char**argv)
{
    AppPathManager::initAppPath();
    if(frmMain.initSDL())
    {
        frmMain.freeSDL();
        return 1;
    }
    int ret = GameMain(argc, argv);
    frmMain.freeSDL();
    return ret;
}
