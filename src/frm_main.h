#ifndef FRMMAIN_H
#define FRMMAIN_H

#include <SDL2/SDL.h>

class FrmMain
{
    SDL_Event event;
    int ScaleWidth = 800;
    int ScaleHeight = 600;
public:
    bool LockSize = false;
    int MousePointer = 0;

    FrmMain();

    void initSDL();
    void freeSDL();

    void show();
    void hide();
    void doEvents();

    void eventDoubleClick();
    void eventKeyDown(SDL_KeyboardEvent &evt);
    void eventKeyPress(SDL_Keycode KeyASCII);
    void eventKeyUp(SDL_KeyboardEvent &evt);
    void eventMouseDown(SDL_MouseButtonEvent &event);
    void eventMouseMove(SDL_MouseMotionEvent &event);
    void eventMouseUp(SDL_MouseButtonEvent &event);
    void eventResize();
    void toggleFullScreen();
};

#endif // FRMMAIN_H
