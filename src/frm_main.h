#ifndef FRMMAIN_H
#define FRMMAIN_H

#include <string>
#include <set>
#include <SDL2/SDL.h>

#include "std_picture.h"

class FrmMain
{
    SDL_Event event;
    int ScaleWidth = 800;
    int ScaleHeight = 600;
    std::string WindowTitle = "A2XT v1.3";
    SDL_Window *window = nullptr;
    SDL_Renderer *m_gRenderer = nullptr;
    std::set<SDL_Texture *> m_textureBank;
    bool m_sdlLoaded = false;
    const Uint8 *m_keyboardState = nullptr;
public:
    bool LockSize = false;
    int MousePointer = 0;

    FrmMain();

    Uint8 getKeyState(SDL_Scancode key);

    bool initSDL();
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
    int setFullScreen(bool fs);
    bool isSdlError();

    StdPicture LoadPicture(std::string path, std::string maskPath, std::string maskFallbackPath);
    void deleteTexture(StdPicture &tx);
private:
    void loadTexture(StdPicture &target, uint32_t width, uint32_t height, uint8_t *RGBApixels);
};

#endif // FRMMAIN_H
