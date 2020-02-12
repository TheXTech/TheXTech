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
    void updateViewport();

    StdPicture LoadPicture(std::string path, std::string maskPath = std::string(), std::string maskFallbackPath = std::string());
    void deleteTexture(StdPicture &tx);
    void clearAllTextures();

    void renderRect(int x, int y, int w, int h, float red = 1.f, float green = 1.f, float blue = 1.f, float alpha = 1.f, bool filled = true);
    void renderRectBR(int _left, int _top, int _right, int _bottom, float red, float green, float blue, float alpha);

    // Similar to BitBlt, but without masks, just draw a texture or it's fragment!
    void renderTexture(int xDst, int yDst, int wDst, int hDst,
                       const StdPicture &tx,
                       int xSrc, int ySrc,
                       float red = 1.f, float green = 1.f, float blue = 1.f, float alpha = 1.f);

    void renderTexture(int xDst, int yDst, const StdPicture &tx,
                       float red = 1.f, float green = 1.f, float blue = 1.f, float alpha = 1.f);

private:
    void loadTexture(StdPicture &target, uint32_t width, uint32_t height, uint8_t *RGBApixels);

    //Scale of virtual and window resolutuins
    float scale_x = 1.f;
    float scale_y = 1.f;
    //Side offsets to keep ratio
    float offset_x = 0.f;
    float offset_y = 0.f;
    //Need to calculate relative viewport position when screen was scaled
    float viewport_scale_x = 1.0f;
    float viewport_scale_y = 1.0f;

    SDL_Rect   scaledRectIS(float x, float y, int w, int h);
    SDL_Rect   scaledRect(float x, float y, float w, float h);
    SDL_Rect   scaledRectS(float left, float top, float right, float bottom);

    SDL_Point MapToScr(int x, int y);
};

#endif // FRMMAIN_H
