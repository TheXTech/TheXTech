/*
 * A2xTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2020 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef FRMMAIN_H
#define FRMMAIN_H

#include <string>
#include <set>
#include <SDL2/SDL.h>

#include "std_picture.h"
#include "cmd_line_setup.h"

class FrmMain
{
    SDL_Event m_event;
    std::string m_windowTitle;
    SDL_Window *m_window = nullptr;
    SDL_Renderer *m_gRenderer = nullptr;
    std::set<SDL_Texture *> m_textureBank;
    bool m_sdlLoaded = false;
    const Uint8 *m_keyboardState = nullptr;
public:
    int ScaleWidth = 800;
    int ScaleHeight = 600;

    int MousePointer = 0;

    FrmMain();

    SDL_Window *getWindow();

    Uint8 getKeyState(SDL_Scancode key);

    bool initSDL(const CmdLineSetup_t &setup);
    void freeSDL();

    void show();
    void hide();
    void doEvents();
    void waitEvents();

    bool isWindowActive();

    void eventDoubleClick();
    void eventKeyDown(SDL_KeyboardEvent &evt);
    void eventKeyPress(SDL_Scancode KeyASCII);
    void eventKeyUp(SDL_KeyboardEvent &evt);
    void eventMouseDown(SDL_MouseButtonEvent &m_event);
    void eventMouseMove(SDL_MouseMotionEvent &m_event);
    void eventMouseUp(SDL_MouseButtonEvent &m_event);
    void eventResize();
    int setFullScreen(bool fs);
    bool isSdlError();

    void repaint();
    void updateViewport();
    void resetViewport();
    void setViewport(int x, int y, int w, int h);
    void offsetViewport(int x, int y); // for screen-shaking

    StdPicture LoadPicture(std::string path, std::string maskPath = std::string(), std::string maskFallbackPath = std::string());
    StdPicture lazyLoadPicture(std::string path, std::string maskPath = std::string(), std::string maskFallbackPath = std::string());
    void deleteTexture(StdPicture &tx, bool lazyUnload = false);
    void clearAllTextures();

    void clearBuffer();
    void renderRect(int x, int y, int w, int h, float red = 1.f, float green = 1.f, float blue = 1.f, float alpha = 1.f, bool filled = true);
    void renderRectBR(int _left, int _top, int _right, int _bottom, float red, float green, float blue, float alpha);

    // Similar to BitBlt, but without masks, just draw a texture or it's fragment!
    void renderTextureI(int xDst, int yDst, int wDst, int hDst,
                        StdPicture &tx,
                        int xSrc, int ySrc,
                        float red = 1.f, float green = 1.f, float blue = 1.f, float alpha = 1.f);
    void renderTexture(double xDst, double yDst, double wDst, double hDst,
                       StdPicture &tx,
                       int xSrc, int ySrc,
                       float red = 1.f, float green = 1.f, float blue = 1.f, float alpha = 1.f);

    void renderTexture(int xDst, int yDst, StdPicture &tx,
                       float red = 1.f, float green = 1.f, float blue = 1.f, float alpha = 1.f);

    void getScreenPixels(int x, int y, int w, int h, unsigned char *pixels);
    void getScreenPixelsRGBA(int x, int y, int w, int h, unsigned char *pixels);
    int  getPixelDataSize(const StdPicture &tx);
    void getPixelData(const StdPicture &tx, unsigned char *pixelData);

    void makeShot();

private:
    bool recordInProcess();
    void toggleGifRecorder();
    void processRecorder();
    static int processRecorder_action(void *_pixels);

    void processEvent();
    void loadTexture(StdPicture &target, uint32_t width, uint32_t height, uint8_t *RGBApixels);

    void lazyLoad(StdPicture &target);
    void lazyUnLoad(StdPicture &target);

    struct PGE_GL_shoot
    {
        FrmMain *me = nullptr;
        uint8_t *pixels = nullptr;
        int w = 0, h = 0;
    };

    std::string g_ScreenshotPath;

    static int makeShot_action(void *_pixels);

    SDL_Thread *m_screenshot_thread = nullptr;

    //Scale of virtual and window resolutuins
    float scale_x = 1.f;
    float scale_y = 1.f;
    //Side offsets to keep ratio
    float offset_x = 0.f;
    float offset_y = 0.f;
    //Offset to shake screen
    int viewport_offset_x = 0.f;
    int viewport_offset_y = 0.f;
    //Need to calculate relative viewport position when screen was scaled
    float viewport_scale_x = 1.0f;
    float viewport_scale_y = 1.0f;

    int viewport_x = 0;
    int viewport_y = 0;
    int viewport_w = 0;
    int viewport_h = 0;

    SDL_Rect   scaledRectIS(float x, float y, int w, int h);
    SDL_Rect   scaledRect(float x, float y, float w, float h);
    SDL_Rect   scaledRectS(float left, float top, float right, float bottom);

    SDL_Point MapToScr(int x, int y);
};

#endif // FRMMAIN_H
