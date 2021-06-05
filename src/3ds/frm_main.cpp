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
// #include <Logger/logger.h>
#include <Utils/files.h>
#include <Utils/elapsed_timer.h>
#include <DirManager/dirman.h>
#include <chrono>
#include <fmt_format_ne.h>
#include "../editor/new_editor.h"

#include <3ds.h>

#include "n3ds-clock.h"
#include "second_screen.h"
#include "c2d_draw.h"

#include "../frm_main.h"

FrmMain::FrmMain()
{
    ScaleWidth = ScreenW;
    ScaleHeight = ScreenH;
}

bool FrmMain::initSDL(const CmdLineSetup_t &setup)
{
    // 3ds libs
    aptInit();
    InitClockSpeed();
    gfxInitDefault();
    gfxSet3D(true); // Enable stereoscopic 3D
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    C2D_Prepare();
    // consoleInit(GFX_BOTTOM, NULL);

    top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
    right = C2D_CreateScreenTarget(GFX_TOP, GFX_RIGHT);
    bottom = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);
    // bottom = right;
    debugMode = false;
    inFrame = false;

    for (int i = 0; i < 4; i++)
    {
        C3D_TexInitVRAM(&layer_texs[i], 512, 256, GPU_RGBA8);
        layer_targets[i] = C3D_RenderTargetCreateFromTex(&layer_texs[i], GPU_TEXFACE_2D, 0, GPU_RB_DEPTH24_STENCIL8);
        constexpr int texwidth = ScreenW/2;
        constexpr int texheight = ScreenH/2;
        layer_subtexs[i] = {texwidth, texheight, 0., 1., (double)texwidth/512., 1-((double)texheight/256.)};
        layer_ims[i].tex = &layer_texs[i];
        layer_ims[i].subtex = &layer_subtexs[i];
    }

    viewport_x = viewport_y = 0;
    updateViewport();

    doEvents();

    return false;
}

void FrmMain::freeSDL()
{
    GFX.unLoad();
    clearAllTextures();

    for (int i = 0; i < 4; i++)
    {
        C3D_TexDelete(&layer_texs[i]);
    }
    // 3ds libs
    C2D_Fini();
    C3D_Fini();
    gfxExit();
    aptExit();

    // printf("<Application closed>");
    // CloseLog();
}

void FrmMain::show()
{
}

void FrmMain::hide()
{
}

void FrmMain::doEvents()
{
    hidScanInput();

    keys_held = hidKeysHeld();
    keys_pressed = hidKeysDown();
    keys_released = hidKeysUp();

    depthSlider = osGet3DSliderState();
    if (!aptMainLoop()) KillIt(); // could add panic save of some sort here...

    if (!LevelEditor)
        return;

    touchPosition touch;
    hidTouchRead(&touch);

#ifdef NEW_EDITOR
    if (!editorScreen.active)
    {
        if ((keys_pressed & KEY_TOUCH) && (touch.py > 20))
        {
            int distance2 = (m_lastMousePosition.px - touch.px)*(m_lastMousePosition.px - touch.px)
                + (m_lastMousePosition.py - touch.py)*(m_lastMousePosition.py - touch.py);

            if (distance2 < 64)
                EditorControls.MouseClick = true;
        }
        if (keys_released & KEY_TOUCH)
        {
            EditorControls.MouseClick = false;
            MenuMouseRelease = true;
        }
        if (keys_held & KEY_TOUCH)
        {
            // not selector bar
            if (touch.py > 20)
            {
                MenuMouseX = -50;
                MenuMouseY = -50;
                EditorCursor.X = 100 + touch.px * 2;
                EditorCursor.Y = touch.py * 2;
                m_lastMousePosition = touch;
            }
            // selector bar
            else
            {
                MenuMouseDown = keys_held & KEY_TOUCH;
                MenuMouseX = 100 + touch.px * 2;
                MenuMouseY = touch.py * 2;
                m_lastMousePosition = {0, 0};
            }
        }
        MouseMove(EditorCursor.X, EditorCursor.Y, true);
    }
    else
    {
        MenuMouseRelease |= keys_released & KEY_TOUCH;
        MenuMouseDown = keys_held & KEY_TOUCH;
        if (MenuMouseDown)
        {
            MenuMouseX = touch.px * 2;
            MenuMouseY = touch.py * 2;
        }
    }
#endif
}

void FrmMain::processEvent()
{
    return;
}

void FrmMain::waitEvents()
{
    doEvents();
}

bool FrmMain::isWindowActive()
{
    return true;
}

bool FrmMain::hasWindowMouseFocus()
{
    return true;
}

void FrmMain::eventResize()
{
    updateViewport();
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

void FrmMain::initDraw(int screen)
{
    if(!inFrame)
        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    // enter the draw context!
    if (screen == 0)
    {
        resetViewport();
        for (int layer = 0; layer < 4; layer++)
        {
            C2D_TargetClear(layer_targets[layer], C2D_Color32f(0.0f, 0.0f, 0.0f, 0.0f));
        }
        C2D_SceneBegin(layer_targets[2]); // screen plane target
    }
#ifdef NEW_EDITOR
    else if (LevelEditor && !editorScreen.active)
    {
        setViewport(80, 0, 640, 480);
        C2D_TargetClear(top, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
        C2D_SceneBegin(top);
    }
#endif
    else
    {
        setViewport(0, 0, 640, 480);
        C2D_TargetClear(bottom, C2D_Color32f(0.0f, 0.0f, 1.0f, 1.0f));
        C2D_SceneBegin(bottom);
    }
    inFrame = true;
}

void FrmMain::setLayer(int layer)
{
    C2D_SceneBegin(layer_targets[layer]);
}

void FrmMain::clearBuffer()
{
    if(!inFrame)
    {
        resetViewport();
        C3D_FrameBegin(0);
        C2D_SceneBegin(top);
        renderRect(0,0,ScreenW,ScreenH,0.f,0.f,0.f,1.f,true);
        C2D_SceneBegin(right);
        renderRect(0,0,ScreenW,ScreenH,0.f,0.f,0.f,1.f,true);
        C2D_SceneBegin(bottom);
        renderRect(0,0,ScreenW,ScreenH,0.f,0.f,0.f,1.f,true);
        C3D_FrameEnd(0);
    }
}

void FrmMain::toggleDebug()
{
    if (debugMode)
    {
        if(!inFrame)
            C3D_FrameBegin(0);
        C2D_TargetClear(bottom, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
        C2D_SceneBegin(bottom);
        renderRect(0,0,ScreenW,ScreenH,0.f,0.f,0.f,1.f,true);
        if(!inFrame)
        {
            C3D_FrameEnd(0);
        }
    }
    debugMode = !debugMode;
}

void FrmMain::repaint()
{
    if (!inFrame)
        return;
    constexpr int shift = Max3DOffset / 2;
    constexpr double bg_shift = shift;
    constexpr double mid_shift = shift * .4;
    // leave the draw context and wait for vblank...
#ifdef NEW_EDITOR
    if (LevelEditor && !editorScreen.active)
    {
        C2D_TargetClear(bottom, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
        C2D_SceneBegin(bottom);
        // can be a bigger offset than this...
        C2D_DrawImageAt(layer_ims[0], -40 - shift, 0, 0);
        C2D_DrawImageAt(layer_ims[1], -40 - shift, 0, 0);
        C2D_DrawImageAt(layer_ims[2], -40 - shift, 0, 0);
        C2D_DrawImageAt(layer_ims[3], -40 - shift, 0, 0);
    }
    else
#endif
    if (depthSlider <= 0.05)
    {
        C2D_TargetClear(top, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
        C2D_SceneBegin(top);
        C2D_DrawImageAt(layer_ims[0], -shift, 0, 0);
        C2D_DrawImageAt(layer_ims[1], -shift, 0, 0);
        C2D_DrawImageAt(layer_ims[2], -shift, 0, 0);
        C2D_DrawImageAt(layer_ims[3], -shift, 0, 0);
    }
    else
    {
        C2D_TargetClear(top, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
        C2D_SceneBegin(top);
        C2D_DrawImageAt(layer_ims[0], -shift - (int)(bg_shift * depthSlider), 0, 0);
        C2D_DrawImageAt(layer_ims[1], -shift - (int)(mid_shift * depthSlider), 0, 0);
        C2D_DrawImageAt(layer_ims[2], -shift, 0, 0);
        C2D_DrawImageAt(layer_ims[3], -shift + (int)(mid_shift * depthSlider), 0, 0);

        C2D_TargetClear(right, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
        C2D_SceneBegin(right);
        C2D_DrawImageAt(layer_ims[0], -shift + (int)(bg_shift * depthSlider), 0, 0);
        C2D_DrawImageAt(layer_ims[1], -shift + (int)(mid_shift * depthSlider), 0, 0);
        C2D_DrawImageAt(layer_ims[2], -shift, 0, 0);
        C2D_DrawImageAt(layer_ims[3], -shift - (int)(mid_shift * depthSlider), 0, 0);
    }
    currentFrame ++;
    inFrame = false;
    C3D_FrameEnd(0);
}

void FrmMain::updateViewport()
{
    resetViewport();
    offsetViewport(0, 0);
}

void FrmMain::resetViewport()
{
    setViewport(0, 0, ScreenW, ScreenH);
}

void FrmMain::setViewport(int x, int y, int w, int h)
{
    int offset_x = viewport_offset_x - viewport_x;
    int offset_y = viewport_offset_y - viewport_y;
    viewport_x = x/2;
    viewport_y = y/2;
    viewport_w = w/2;
    viewport_h = h/2;
    viewport_offset_x = viewport_x + offset_x;
    viewport_offset_y = viewport_y + offset_y;
}

void FrmMain::offsetViewport(int x, int y)
{
    viewport_offset_x = viewport_x+x/2;
    viewport_offset_y = viewport_y+y/2;
}

StdPicture FrmMain::LoadPicture(std::string path)
{
    StdPicture target;
    C2D_SpriteSheet sourceImage;
    if(!GameIsActive)
        return target; // do nothing when game is closed

    target.inited = false;
    target.path = path;
    if(target.path.empty())
        return target;

    target.inited = true;
    target.lazyLoaded = false;

    sourceImage = C2D_SpriteSheetLoad(target.path.c_str());
    loadTexture(target, sourceImage);

    num_textures_loaded ++;

    if(!target.texture)
        printf("FAILED TO LOAD!!! %s\n", path.c_str());
    return target;
}


StdPicture FrmMain::lazyLoadPicture(std::string path)
{
    StdPicture target;
    if(!GameIsActive)
        return target; // do nothing when game is closed

    target.inited = false;
    target.path = path;
    if(target.path.empty())
        return target;

    target.inited = true;

    target.lazyLoaded = true;

    // We need to figure out the height and width!
    std::string sizePath = path + ".size";
    FILE *fs;
    fs = fopen(sizePath.c_str(), "r");
    // NOT null-terminated: wwww\nhhhh\n
    char contents[10];
    if (fs != nullptr) {
        fread(&contents[0], 1, 10, fs);
        contents[4] = '\0';
        contents[9] = '\0';
        target.w = atoi(&contents[0]);
        target.h = atoi(&contents[5]);
        if (fclose(fs)) printf("lazyLoadPicture: Couldn't close file.\n");
    }
    // lazy load and unload to read dimensions if it doesn't exist.
    // unload is essential because lazy load would save the address incorrectly.
    else {
        lazyLoad(target);
        lazyUnLoad(target);
        printf("lazyLoadPicture: Couldn't open size file.\n");
    }

    return target;
}

void FrmMain::loadTexture(StdPicture &target, C2D_SpriteSheet &sheet)
{
    C2D_Image im = C2D_SpriteSheetGetImage(sheet, 0);

    target.texture = sheet;
    target.image = im;
    if (!target.w)
    {
        target.w = im.subtex->width*2;
        target.h = im.subtex->height*2;
    }

    m_textureBank.insert(sheet);
}

void FrmMain::loadTexture2(StdPicture &target, C2D_SpriteSheet &sheet)
{
    C2D_Image im = C2D_SpriteSheetGetImage(sheet, 0);

    target.texture2 = sheet;
    target.image2 = im;

    m_textureBank.insert(sheet);
}

void FrmMain::loadTexture3(StdPicture &target, C2D_SpriteSheet &sheet)
{
    C2D_Image im = C2D_SpriteSheetGetImage(sheet, 0);

    target.texture3 = sheet;
    target.image3 = im;

    m_textureBank.insert(sheet);
}

void FrmMain::lazyLoad(StdPicture &target)
{
    if(!target.inited || !target.lazyLoaded || target.texture)
        return;

    C2D_SpriteSheet sourceImage;
    std::string suppPath;

    sourceImage = C2D_SpriteSheetLoad(target.path.c_str()); // some other source image

    int i;

    if (!sourceImage) {
        // wish I knew the largest contiguous portion of memory
        // max tex should be 4194304
        for (i = 0; i < 10; i ++)
        {
            if (linearSpaceFree() > 4000000) break;
            if (!freeTextureMem()) break;
        }
        sourceImage = C2D_SpriteSheetLoad(target.path.c_str());

        if (!sourceImage) {
            for (i = 0; i < 10; i ++)
            {
                if (linearSpaceFree() > 8000000) break;
                if (!freeTextureMem()) break;
            }
            sourceImage = C2D_SpriteSheetLoad(target.path.c_str());

            if (!sourceImage) {
                for (i = 0; i < 25; i ++)
                {
                    if (linearSpaceFree() > 20000000) break;
                    if (!freeTextureMem()) break;
                }
                sourceImage = C2D_SpriteSheetLoad(target.path.c_str());

                if (!sourceImage) {
                    printf("Permanently failed to load %s, %lu free\n", target.path.c_str(), linearSpaceFree());
                    printf("Error: %d (%s)\n", errno, strerror(errno));
                    target.inited = false;
                    return;
                }
            }
        }
    }

    loadTexture(target, sourceImage);

    if (target.h > 2048) {
        suppPath = target.path + '1';
        sourceImage = C2D_SpriteSheetLoad(suppPath.c_str());
        if (!sourceImage) {
            for (i = 0; i < 10; i ++)
            {
                if (linearSpaceFree() > 16000000) break;
                if (!freeTextureMem()) break;
            }
            sourceImage = C2D_SpriteSheetLoad(suppPath.c_str());
        }
        if (sourceImage) loadTexture2(target, sourceImage);
    }
    if (target.h > 4096) {
        suppPath = target.path + '2';
        sourceImage = C2D_SpriteSheetLoad(suppPath.c_str());
        if (!sourceImage) {
            for (i = 0; i < 10; i ++)
            {
                if (linearSpaceFree() > 16000000) break;
                if (!freeTextureMem()) break;
            }
            sourceImage = C2D_SpriteSheetLoad(suppPath.c_str());
        }
        if (sourceImage) loadTexture3(target, sourceImage);
    }

    num_textures_loaded ++;

    if (target.w >= 256 || target.h >= 256)
    {
        m_bigPictures.insert(&target);
        num_big_textures_loaded ++;
    }

    if (linearSpaceFree() < 4194304) freeTextureMem();
}

bool FrmMain::freeTextureMem() // make it take an amount of memory, someday.....
{
    printf("Freeing texture memory...\n");
    StdPicture* oldest = nullptr;
    uint32_t earliestDraw = 0;
    StdPicture* second_oldest = nullptr;
    uint32_t second_earliestDraw = 0;
    for (StdPicture* poss : m_bigPictures)
    {
        if (poss->texture && poss->lazyLoaded && (poss->lastDrawFrame+30 < currentFrame))
        {
            if ((oldest == nullptr) || (poss->lastDrawFrame < earliestDraw))
            {
                second_oldest = oldest;
                second_earliestDraw = earliestDraw;
                oldest = poss;
                earliestDraw = poss->lastDrawFrame;
            }
            else if ((second_oldest == nullptr) || (poss->lastDrawFrame < second_earliestDraw))
            {
                second_oldest = poss;
                second_earliestDraw = poss->lastDrawFrame;
            }
        }
    }
    if (oldest == nullptr) return false;
    printf("Clearing %p, %p\n", oldest, second_oldest);
    printf("Clearing %s, %s\n", oldest->path.c_str(), (second_oldest) ? second_oldest->path.c_str() : "");
    lazyUnLoad(*oldest);
    if (second_oldest) lazyUnLoad(*second_oldest);
    return true;
}

void FrmMain::lazyUnLoad(StdPicture &target)
{
    if(!target.inited || !target.lazyLoaded || !target.texture)
        return;
    deleteTexture(target, true);
}


SDL_Point FrmMain::MapToScr(int x, int y)
{
    return {
        static_cast<int>((static_cast<float>(x) - offset_x) / viewport_scale_x),
        static_cast<int>((static_cast<float>(y) - offset_y) / viewport_scale_y)
    };
}

void FrmMain::deleteTexture(StdPicture &tx, bool lazyUnload)
{
    // printf("Clearing %s from %lu", tx.path.c_str(), linearSpaceFree());
    if(!tx.inited)
        return;

    if(m_bigPictures.find(&tx) != m_bigPictures.end())
    {
        m_bigPictures.erase(&tx);
        num_big_textures_loaded --;
    }

    if(tx.texture)
    {
        num_textures_loaded --;
        if(m_textureBank.find(tx.texture) != m_textureBank.end())
            m_textureBank.erase(tx.texture);
        C2D_SpriteSheetFree(tx.texture);
        tx.texture = nullptr;
    }

    if(tx.texture2)
    {
        if(m_textureBank.find(tx.texture2) != m_textureBank.end())
            m_textureBank.erase(tx.texture2);
        C2D_SpriteSheetFree(tx.texture2);
        tx.texture2 = nullptr;
    }

    if(tx.texture3)
    {
        if(m_textureBank.find(tx.texture3) != m_textureBank.end())
            m_textureBank.erase(tx.texture3);
        C2D_SpriteSheetFree(tx.texture3);
        tx.texture3 = nullptr;
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
    // printf(" to %lu\n", linearSpaceFree());
}

void FrmMain::clearAllTextures()
{
    for(C2D_SpriteSheet tx : m_textureBank)
        C2D_SpriteSheetFree(tx);
    m_textureBank.clear();
}

void FrmMain::renderRect(int x, int y, int w, int h, float red, float green, float blue, float alpha, bool filled)
{
    uint32_t clr = C2D_Color32f(red, green, blue, alpha);

    // Filled is always True in this game
    if (filled)
        C2D_DrawRectSolid(x/2+viewport_offset_x,
                          y/2+viewport_offset_y,
                          0, w/2, h/2, clr);
    else
    {
        C2D_DrawRectangle(x/2+viewport_offset_x,
                          y/2+viewport_offset_y,
                          0, 1, h/2, clr, clr, clr, clr);
        C2D_DrawRectangle(x/2+viewport_offset_x+w/2-1,
                          y/2+viewport_offset_y,
                          0, 1, h/2, clr, clr, clr, clr);
        C2D_DrawRectangle(x/2+viewport_offset_x,
                          y/2+viewport_offset_y,
                          0, w/2, 1, clr, clr, clr, clr);
        C2D_DrawRectangle(x/2+viewport_offset_x,
                          y/2+viewport_offset_y+h/2-1,
                          0, w/2, 1, clr, clr, clr, clr);
    }
}

void FrmMain::renderRectBR(int _left, int _top, int _right, int _bottom, float red, float green, float blue, float alpha)
{
    renderRect(_left, _top, _right-_left, _bottom-_top, red, green, blue, alpha, true);
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

void FrmMain::renderTexturePrivate(float xDst, float yDst, float wDst, float hDst,
                             StdPicture &tx,
                             float xSrc, float ySrc, float wSrc, float hSrc,
                             float rotateAngle, SDL_Point *center, unsigned int flip,
                             float red, float green, float blue, float alpha)
{
    if(!tx.inited)
        return;

    if(!tx.texture && tx.lazyLoaded)
        lazyLoad(tx);

    tx.lastDrawFrame = currentFrame;

    if(!tx.texture)
        return;
    if(xDst > viewport_w || yDst > viewport_h)
        return;

    // automatic flipping based on SMBX style!
    unsigned int mode = 0;
    while(ySrc >= tx.h/2 && mode < 3)
    {
        ySrc -= tx.h/2;
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

    C2D_Image* to_draw = nullptr;
    C2D_Image* to_draw_2 = nullptr;

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

    if(ySrc + hSrc > 1024.f)
    {
        if(ySrc + hSrc > 2048.f)
        {
            if(tx.texture3)
                to_draw = &tx.image3;
            if(ySrc < 2048.f && tx.texture2)
                to_draw_2 = &tx.image2;
            ySrc -= 1024.f;
        }
        else
        {
            if(tx.texture2)
                to_draw = &tx.image2;
            if(ySrc < 1024.f)
                to_draw_2 = &tx.image;
        }
        // draw the top pic
        if(to_draw_2 != nullptr)
        {
            if(rotateAngle != 0.0)
                C2D_DrawImage_Custom_Rotated(*to_draw_2, xDst+viewport_offset_x, yDst+viewport_offset_y, wDst, (1024.f-ySrc)*hDst/hSrc,
                                     xSrc, ySrc, wSrc, 1024.f-ySrc, flip, center, rotateAngle, red, green, blue, alpha);
            else
                C2D_DrawImage_Custom(*to_draw_2, xDst+viewport_offset_x, yDst+viewport_offset_y, wDst, (1024.f-ySrc)*hDst/hSrc,
                                     xSrc, ySrc, wSrc, 1024.f-ySrc, flip, red, green, blue, alpha);
            yDst += (1024.f - ySrc) * hDst/hSrc;
            hDst -= (1024.f - ySrc) * hDst/hSrc;
            hSrc -= (1024.f - ySrc);
            ySrc = 0.f;
        }
        else
            ySrc -= 1024.f;
    }
    else to_draw = &tx.image;

    if (to_draw == nullptr) return;

    if(rotateAngle != 0.0)
        C2D_DrawImage_Custom_Rotated(*to_draw, xDst+viewport_offset_x, yDst+viewport_offset_y, wDst, hDst,
                             xSrc, ySrc, wSrc, hSrc, flip, center, rotateAngle, red, green, blue, alpha);
    else
        C2D_DrawImage_Custom(*to_draw, xDst+viewport_offset_x, yDst+viewport_offset_y, wDst, hDst,
                             xSrc, ySrc, wSrc, hSrc, flip, red, green, blue, alpha);
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
