/*
 * TheXTech - A platform game engine ported from old source code for VB6
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

#include "../globals.h"
#include "../game_main.h"
#include "../graphics.h"
#include "../control/joystick.h"
#include "../sound.h"
#include "../editor/editor.h"

#include <AppPath/app_path.h>
#include <Logger/logger.h>
#include <Utils/files.h>
#include <Utils/elapsed_timer.h>
#include <DirManager/dirman.h>
#include <chrono>
#include <fmt_format_ne.h>
#include "../editor/new_editor.h"


#include <vitasdk.h>
#include <vitaGL.h>


#include "../frm_main.h"

// typedef struct SDL_Point
// {
//     float x, y;
// } SDL_Point;

FrmMain::FrmMain()
{
    ScaleWidth = 960;
    ScaleHeight = 544;
}

bool FrmMain::initSDL(const CmdLineSetup_t &setup)
{
    return false;
}

void FrmMain::freeSDL()
{
    GFX.unLoad();
}

void FrmMain::show()
{}

void FrmMain::hide()
{}

bool FrmMain::isWindowActive() 
{return true;}

bool FrmMain::hasWindowMouseFocus()
{return true;}

void FrmMain::eventResize()
{
    // update viewport?
    SetupScreens();
}

int FrmMain::setFullScreen(bool fs)
{
    return 0;
}

bool FrmMain::isSdlError()
{
    return false;
}

void FrmMain::setTargetTexture() {}
void FrmMain::setTargetScreen() {}

void FrmMain::processEvent()
{return;}
void FrmMain::waitEvents()
{doEvents();}

void FrmMain::doEvents()
{
    // Get keys held, or just borrow code from SDL version?

    // Only accept touch input for level editor.
    if(!LevelEditor)
        return;

    // Check for touch input on vita for the level editor.
}

void FrmMain::repaint()
{
    // paint from render target to screen.
}

/// INCOMPLETE
StdPicture FrmMain::LoadPicture(std::string path)
{
    StdPicture target;
    if(!GameIsActive) return target;

    target.inited = false;
    target.path = path;

    if(target.path.empty()) return target;

    target.inited = true;
    target.lazyLoaded = false;

    // Get source raw image data.
    // srcImage = LoadImageDataNatively();

    // Get width and height
    int width = 0, int height = 0;
    loadTexture(target, width, height, 0);

    num_textures_loaded++;

    if(!target.texture)
        printf("FAILED TO LOAD!!!! %s\n", path.c_str());
    return target;
}

StdPicture FrmMain::lazyLoadPicture(std::string path)
{
    StdPicture target;
    if(!GameIsActive)
        return target;

    target.inited = false;
    target.path = path;
    if(target.path.empty())
        return target;

    target.inited = true;
    target.lazyLoaded = true;

    // Check for existing .size file 
    std::string sizePath = path + ".size";
    FILE *fs;
    fs = fopen(sizePath.c_str(), "r");

    char contents[10];
    if(fs != nullptr)
    {
        fread(&contents[0], 1, 10, fs);

        // null terminate for atoi to convert
        contents[4] = '\0';
        contents[9] = '\0';

        target.w = atoi(&contents[0]);
        target.h = atoi(&contents[5]);

        if(fclose(fs)) printf("lazyLoadPicture: Couldn't close file.\n");
    }
    else 
    {
        lazyLoad(target);
        lazyUnLoad(target);
        printf("lazyLoadPicture: Couldn't open size file.\n");
    }

    return target;
}

void FrmMain::loadTexture(StdPicture &target, uint32_t width, uint32_t height, uint8_t *RGBApixels)
{
    // TODO: Override StdPicture definition and change
    //      .texture to type GLuint (OpenGL texture ID).
    // TODO: Override m_textureBank type and change
    //      SDL_Texture* to GLuint (OpenGL texture ID).

    // Interpret raw image data into a GL_TEXTURE_2D.
    // Set the target.texture to the returned GLuint texture ID.
    // Insert the texture into our bank.

    target.inited = false;
}

void FrmMain::lazyLoad(StdPicture &target)
{
    if(!target.inited || !target.lazyLoaded || target.texture)
        return;

    // Try and load source image data from disk.
    // EG:
    // sourcePixels = LoadNativeData(target.path.c_str());

    int i;
    if(!sourceImage) {
        printf("[lazyLoad] Failed to load %s. Not implemented or no free memory.\n", target.path.c_str());
        target.inited = false;
        return;
    }

    int width = 0, height = 0, sourcePixels = 0;
    loadTexture(target, width, height, sourcePixels);

    // TODO: Do I need to track "big textures" and have them
    // split like the 3DS version?
}

void FrmMain::lazyUnLoad(StdPicture &target)
{
    if(!target.inited || !target.lazyLoaded || !target.texture)
        return;

    deleteTexture(target, true);
}

SDL_Point MapToScr(int x, int y)
{
    return {
        static_cast<int>((static_cast<float>(x) - offset_x) / viewport_scale_x),
        static_cast<int>((static_cast<float>(y) - offset_y) / viewport_scale_y)
    };
}

void FrmMain::deleteTexture(StdPicture &tx, bool lazyUnload)
{
    if(!tx.inited)
        return;

    if(tx.texture)
    {
        // num_textures_loaded--;
        if(m_textureBank.find(tx.texture) != m_textureBank.end())
            m_textureBank.erase(tx.texture);

        // Free sprite from memory.
        
        // For good measure.
        tx.texture = nullptr;

    }

    if(!lazyUnload)
    {
        tx.inited = false;
        tx.lazyLoaded = false;
        tx.w = 0;
        tx.h = 0;
        tx.frame_w = 0;
        tx.frame_h = 0;
    }
}

void FrmMain::clearAllTextures()
{
    // for(SDL_Texture tx : m_textureBank)
    // {
        // Loop and clear the textures from memory.f
    // }

    m_textureBank.clear();
}

void FrmMain::renderRect(int x, int y, int w, int h, float red, float green, float blue, float alpha, bool filled)
{
    // uint32_t clr = C2D_Color32f(red, green, blue, alpha);

    // Filled is always True in this game
    // if (filled)
    //     C2D_DrawRectSolid(x/2+viewport_offset_x,
    //                       y/2+viewport_offset_y,
    //                       0, w/2, h/2, clr);
    // else
    // {
    //     C2D_DrawRectangle(x/2+viewport_offset_x,
    //                       y/2+viewport_offset_y,
    //                       0, 1, h/2, clr, clr, clr, clr);
    //     C2D_DrawRectangle(x/2+viewport_offset_x+w/2-1,
    //                       y/2+viewport_offset_y,
    //                       0, 1, h/2, clr, clr, clr, clr);
    //     C2D_DrawRectangle(x/2+viewport_offset_x,
    //                       y/2+viewport_offset_y,
    //                       0, w/2, 1, clr, clr, clr, clr);
    //     C2D_DrawRectangle(x/2+viewport_offset_x,
    //                       y/2+viewport_offset_y+h/2-1,
    //                       0, w/2, 1, clr, clr, clr, clr);
    // }
}

void FrmMain::renderRectBR(int _left, int _top, int _right, int _bottom, float red, float green, float blue, float alpha)
{
    renderRect(_left, _top, _right-_left, _bottom-_top, red, green, blue, alpha, true);
}

void FrmMain::renderTexturePrivate(float xDst, float yDst, float wDst, float hDst,
                             StdPicture &tx,
                             float xSrc, float ySrc, float wSrc, float hSrc,
                             float rotateAngle = 0.f, SDL_Point *center = nullptr, unsigned int flip = SDL_FLIP_NONE,
                             float red = 1.f, float green = 1.f, float blue = 1.f, float alpha = 1.f)
{
    // This is mostly lifted from the 3DS version so thank you, ds-sloth <3


    if(!tx.inited)
        return;

    if(!tx.texture && tx.lazyLoaded)
        lazyLoad(tx);

    // tx.lastDrawFrame = currentFrame;

    if(!tx.texture)
        return;
    if(xDst > viewport_w || yDst > viewport_h)
        return;

    unsigned int mode = 0;
    while(ySrc >= tx.h / 2 && mode < 3)
    {
        ySrc -= tx.h / 2;
        mode += 1;
    }
    flip ^= mode;

    // texture boundaries
    // this never happens unless there was an invalid input
    // if((xSrc < 0.f) || (ySrc < 0.f)) return;

    // TODO: graphics tests for how offscreen draws interact with flips
    //       handling rotations properly is probably impossible
    if(xDst < 0.f)
    {
        if(!(flip & SDL_FLIP_HORIZONTAL))
            xSrc -= xDst * wSrc/wDst;
        if (wDst+xDst > viewport_w)
        {
            if(flip & SDL_FLIP_HORIZONTAL)
                xSrc += (wDst+xDst-viewport_w) * wSrc/wDst;
            wSrc = viewport_w * wSrc/wDst;
            wDst = viewport_w;
        }
        else
        {
            wSrc += xDst * wSrc/wDst;
            wDst += xDst;
        }
        xDst = 0.f;
    }
    else if(xDst + wDst > viewport_w)
    {
        if(flip & SDL_FLIP_HORIZONTAL)
            xSrc += (wDst+xDst-viewport_w) * wSrc/wDst;
        wSrc = (viewport_w - xDst) * wSrc/wDst;
        wDst = (viewport_w - xDst);
    }
    if(yDst < 0.f)
    {
        if(!(flip & SDL_FLIP_VERTICAL))
            ySrc -= yDst * hSrc/hDst;
        if (hDst+yDst > viewport_h)
        {
            if(flip & SDL_FLIP_VERTICAL)
                ySrc += (hDst+yDst-viewport_h) * hSrc/hDst;
            hSrc = viewport_h * hSrc/hDst;
            hDst = viewport_h;
        }
        else
        {
            hSrc += yDst * hSrc/hDst;
            hDst += yDst;
        }
        yDst = 0.f;
    }
    else if(yDst + hDst > viewport_h)
    {
        if(flip & SDL_FLIP_VERTICAL)
            ySrc += (hDst+yDst-viewport_h) * hSrc/hDst;
        hSrc = (viewport_h - yDst) * hSrc/hDst;
        hDst = (viewport_h - yDst);
    }

    // figure out GLuint textures to draw.
    GLuint to_draw = 0;
    // TODO: Should I be rendering large images in multiple parts?

    // Don't go more than size of texture
    // Failure conditions should only happen if texture is smaller than expected
    if(xSrc + wSrc > tx.w/2)
    {
        wDst = (tx.w/2 - xSrc) * wDst/wSrc;
        wSrc = tx.w/2 - xSrc;
        if(wDst < 0.f)
            return;
    }
    if(ySrc + hSrc > tx.h/2)
    {
        hDst = (tx.h/2 - ySrc) * hDst/hSrc;
        hSrc = tx.h/2 - ySrc;
        if(hDst < 0.f)
            return;
    }

    if(tx.texture) to_draw = tx.texture;

    // VitaGL_DrawImage_Custom code here.
    // Todo also in the future, rotation ?!? !

}

inline int ROUNDDIV2(int x)
{
    return (x<0)?(x-1)/2:x/2;
}

inline float ROUNDDIV2(float x)
{
    return std::nearbyintf(std::roundf(x)/2.f);
}

inline float ROUNDDIV2(double x)
{
    return std::nearbyintf(std::roundf((float)x)/2.f);
}

inline float FLOORDIV2(float x)
{
    return std::floor(x/2.f);
}

void FrmMain::renderTextureI(int xDst, int yDst, int wDst, int hDst,
                        StdPicture &tx,
                        int xSrc, int ySrc,
                        double rotateAngle, SDL_Point *center, unsigned int flip,
                        float red, float green, float blue, float alpha)
{
    int w = wDst/2;
    int h = hDst/2;
    renderTexturePrivate(
        ROUNDDIV2(xDst), ROUNDDIV2(yDst), w, h,
        tx,
        xSrc/2, ySrc/2, w, h,
        rotateAngle, center, flip,
        red, green, blue, alpha);
}

void FrmMain::renderTextureScaleI(int xDst, int yDst, int wDst, int hDst,
                             StdPicture &tx,
                             int xSrc, int ySrc,
                             int wSrc, int hSrc,
                             double rotateAngle, SDL_Point *center, unsigned int flip,
                             float red, float green, float blue, float alpha)
{
    renderTexturePrivate(
        ROUNDDIV2(xDst), ROUNDDIV2(yDst), wDst/2, hDst/2,
        tx,
        xSrc/2, ySrc/2, wSrc/2, hSrc/2,
        rotateAngle, center, flip,
        red, green, blue, alpha);
}

// public draw methods

void FrmMain::renderTextureScale(double xDst, double yDst, double wDst, double hDst,
                            StdPicture &tx,
                            int xSrc, int ySrc, int wSrc, int hSrc,
                            float red, float green, float blue, float alpha)
{
    renderTexturePrivate(
        ROUNDDIV2(xDst), ROUNDDIV2(yDst), ROUNDDIV2(wDst), ROUNDDIV2(hDst),
        tx,
        xSrc/2, ySrc/2, wSrc/2, hSrc/2,
        0.f, nullptr, SDL_FLIP_NONE,
        red, green, blue, alpha);
}

void FrmMain::renderTexture(double xDst, double yDst, double wDst, double hDst,
                            StdPicture &tx,
                            int xSrc, int ySrc,
                            float red, float green, float blue, float alpha)
{
    float w = ROUNDDIV2(wDst);
    float h = ROUNDDIV2(hDst);
    renderTexturePrivate(
        ROUNDDIV2(xDst), ROUNDDIV2(yDst), w, h,
        tx,
        xSrc/2, ySrc/2, w, h,
        0.f, nullptr, SDL_FLIP_NONE,
        red, green, blue, alpha);
}

void FrmMain::renderTexture(int xDst, int yDst, StdPicture &tx, float red, float green, float blue, float alpha)
{
    int w = tx.w/2;
    int h = tx.h/2;
    renderTexturePrivate(
        ROUNDDIV2(xDst), ROUNDDIV2(yDst), w, h,
        tx,
        0.f, 0.f, w, h,
        0.f, nullptr, SDL_FLIP_NONE,
        red, green, blue, alpha);
}

void FrmMain::renderTextureScale(int xDst, int yDst, int wDst, int hDst, StdPicture &tx, float red, float green, float blue, float alpha)
{
    renderTexturePrivate(
        ROUNDDIV2(xDst), ROUNDDIV2(yDst), tx.w/2, tx.h/2,
        tx,
        0.f, 0.f, wDst/2, hDst/2,
        0.f, nullptr, SDL_FLIP_NONE,
        red, green, blue, alpha);
}

size_t FrmMain::lazyLoadedBytes()
{
    return 0;
}

void FrmMain::lazyLoadedBytesReset()
{
}