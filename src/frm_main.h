#ifndef FRMMAIN_H
#define FRMMAIN_H

#include <string>
#include <set>
#include <SDL2/SDL.h>

#include "std_picture.h"

class FrmMain
{
    SDL_Event event;
    std::string WindowTitle = "A2XT v1.3";
    SDL_Window *window = nullptr;
    SDL_Renderer *m_gRenderer = nullptr;
    std::set<SDL_Texture *> m_textureBank;
    bool m_sdlLoaded = false;
    const Uint8 *m_keyboardState = nullptr;
public:
    int ScaleWidth = 800;
    int ScaleHeight = 600;

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

    void repaint();

    StdPicture LoadPicture(std::string path, std::string maskPath, std::string maskFallbackPath);
    void deleteTexture(StdPicture &tx);
    void clearAllTextures();

    void renderRect(int x, int y, int w, int h, float red = 1.f, float green = 1.f, float blue = 1.f, float alpha = 1.f, bool filled = true);
    void renderRectBR(int _left, int _top, int _right, int _bottom, float red, float green, float blue, float alpha);
private:
    void loadTexture(StdPicture &target, uint32_t width, uint32_t height, uint8_t *RGBApixels);

    //Need to calculate relative viewport position when screen was scaled
    float viewport_scale_x = 1.0f;
    float viewport_scale_y = 1.0f;

    SDL_Rect   scaledRectIS(float x, float y, int w, int h);
    SDL_Rect   scaledRect(float x, float y, float w, float h);
    SDL_Rect   scaledRectS(float left, float top, float right, float bottom);
};

#endif // FRMMAIN_H
