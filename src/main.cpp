#include "game_main.h"
#include <SDL2/SDL.h>
#include <AppPath/app_path.h>

extern "C"
int main(int argc, char**argv)
{
    AppPathManager::initAppPath();
    AppPath = AppPathManager::userAppDirSTD() + "a2xtech/";
    if(frmMain.initSDL())
    {
        frmMain.freeSDL();
        return 1;
    }

    int ret = GameMain(argc, argv);
    frmMain.freeSDL();
    return ret;
}
