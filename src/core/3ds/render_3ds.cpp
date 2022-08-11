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

#include <3ds.h>
#include <citro2d.h>

#include <set>

#include <Logger/logger.h>

#include "globals.h"
#include "core/render.h"

// #include "core/3ds/n3ds-clock.h"
// #include "second_screen.h"
#include "c2d_draw.h"


namespace XRender
{

uint32_t s_current_frame = 0;
float s_depth_slider = 0.;

bool g_in_frame = false;

C3D_RenderTarget* s_top_screen;
C3D_RenderTarget* s_right_screen;
C3D_RenderTarget* s_bottom_screen;

Tex3DS_SubTexture s_layer_subtexs[4];
C3D_Tex s_layer_texs[4];
C2D_Image s_layer_ims[4];
C3D_RenderTarget* s_layer_targets[4];
bool s_single_layer_mode = false;

constexpr int s_hardware_w = 400;
constexpr int s_hardware_h = 240;
constexpr int s_max_3d_offset = 20;

int s_viewport_x = 0;
int s_viewport_y = 0;
int s_viewport_w = 0;
int s_viewport_h = 0;
int s_viewport_offset_x = 0;
int s_viewport_offset_y = 0;
int s_viewport_offset_x_bak = 0;
int s_viewport_offset_y_bak = 0;
bool s_viewport_offset_ignore = false;

int s_num_textures_loaded = 0;
int s_num_big_pictures_loaded = 0;
std::set<C2D_SpriteSheet> s_texture_bank;
std::set<StdPicture*> s_big_pictures;

static void s_destroySceneTargets()
{
    for(int i = 0; i < 4; i++)
    {
        if(s_layer_targets[i])
        {
            C3D_RenderTargetDelete(s_layer_targets[i]);
            C3D_TexDelete(&s_layer_texs[i]);
            s_layer_targets[i] = nullptr;
            s_layer_ims[i].tex = nullptr;
            s_layer_ims[i].subtex = nullptr;
        }
    }
}

static void s_createSceneTargets()
{
    s_destroySceneTargets();

    uint16_t tex_w = ScreenW / 2;
    uint16_t tex_h = ScreenH / 2;

    uint16_t mem_w = (tex_w <= 256) ? 256 : 512;
    uint16_t mem_h = (tex_h <= 256) ? 256 : 512;

    if(tex_w > mem_w)
        tex_w = mem_w;
    if(tex_h > mem_h)
        tex_h = mem_h;

    if(mem_w == 512 && mem_h == 512)
        s_single_layer_mode = true;
    else
        s_single_layer_mode = false;

    for(int i = 0; i < 4; i++)
    {
        C3D_TexInitVRAM(&s_layer_texs[i], mem_w, mem_h, GPU_RGBA8);
        s_layer_targets[i] = C3D_RenderTargetCreateFromTex(&s_layer_texs[i], GPU_TEXFACE_2D, 0, GPU_RB_DEPTH24_STENCIL8);
        s_layer_subtexs[i] = {tex_w, tex_h, 0.0, 1.0, (float)((double)tex_w/(double)mem_w), 1.0f - (float)((double)tex_h/(double)mem_h)};
        s_layer_ims[i].tex = &s_layer_texs[i];
        s_layer_ims[i].subtex = &s_layer_subtexs[i];

        if(s_single_layer_mode)
            break;
    }
}

static bool s_freeTextureMem() // make it take an amount of memory, someday.....
{
    D_pLogDebug("Freeing texture memory...");

    StdPicture* oldest = nullptr;
    uint32_t earliestDraw = 0;

    StdPicture* second_oldest = nullptr;
    uint32_t second_earliestDraw = 0;

    for(StdPicture* poss : s_big_pictures)
    {
        if(poss->d.texture && poss->l.lazyLoaded && (poss->d.last_draw_frame+30 < s_current_frame))
        {
            if((oldest == nullptr) || (poss->d.last_draw_frame < earliestDraw))
            {
                second_oldest = oldest;
                second_earliestDraw = earliestDraw;
                oldest = poss;
                earliestDraw = poss->d.last_draw_frame;
            }
            else if((second_oldest == nullptr) || (poss->d.last_draw_frame < second_earliestDraw))
            {
                second_oldest = poss;
                second_earliestDraw = poss->d.last_draw_frame;
            }
        }
    }

    if(oldest == nullptr)
        return false;

    D_pLogDebug("Clearing %p, %p", oldest, second_oldest);
    D_pLogDebug("Clearing %s, %s", oldest->path.c_str(), (second_oldest) ? second_oldest->path.c_str() : "");

    lazyUnLoad(*oldest);
    if(second_oldest)
        lazyUnLoad(*second_oldest);

    return true;
}

void s_ensureInFrame()
{
    if(!g_in_frame)
    {
        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);

        for(int layer = 0; layer < 4; layer++)
        {
            C2D_TargetClear(s_layer_targets[layer], C2D_Color32(0, 0, 0, 0));

            if(s_single_layer_mode)
                break;
        }

        g_in_frame = true;
    }
}

void s_clearAllTextures()
{
    for(C2D_SpriteSheet tx : s_texture_bank)
        C2D_SpriteSheetFree(tx);
    s_texture_bank.clear();
}

void s_loadTexture(StdPicture &target, C2D_SpriteSheet &sheet)
{
    C2D_Image im = C2D_SpriteSheetGetImage(sheet, 0);

    target.d.texture = sheet;
    target.d.image = im;
    if(!target.w)
    {
        target.w = im.subtex->width*2;
        target.h = im.subtex->height*2;
    }

    s_texture_bank.insert(sheet);
}

void s_loadTexture2(StdPicture &target, C2D_SpriteSheet &sheet)
{
    C2D_Image im = C2D_SpriteSheetGetImage(sheet, 0);

    target.d.texture2 = sheet;
    target.d.image2 = im;

    s_texture_bank.insert(sheet);
}

void s_loadTexture3(StdPicture &target, C2D_SpriteSheet &sheet)
{
    C2D_Image im = C2D_SpriteSheetGetImage(sheet, 0);

    target.d.texture3 = sheet;
    target.d.image3 = im;

    s_texture_bank.insert(sheet);
}

bool init()
{
    // 3ds libs
    gfxInitDefault();

    gfxSet3D(false); // Enable stereoscopic 3D

    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    C2D_Prepare();

    // consoleInit(GFX_BOTTOM, NULL);

    s_top_screen = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
    s_right_screen = C2D_CreateScreenTarget(GFX_TOP, GFX_RIGHT);
    s_bottom_screen = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);
    // bottom = right;

    s_createSceneTargets();

    s_viewport_x = s_viewport_y = 0;
    updateViewport();

    return true;
}

void quit()
{
    s_clearAllTextures();
    s_destroySceneTargets();

    // 3ds libs
    C2D_Fini();
    C3D_Fini();
    gfxExit();
    aptExit();
}

void setTargetTexture()
{
    s_ensureInFrame();

    if(s_single_layer_mode)
        C2D_SceneBegin(s_layer_targets[0]);
    else
        C2D_SceneBegin(s_layer_targets[2]); // screen plane target
}

void setTargetScreen()
{
}

void setLayer(int layer)
{
    if(!s_single_layer_mode)
        C2D_SceneBegin(s_layer_targets[layer]);
}

void clearBuffer()
{
    if(!g_in_frame)
    {
        C3D_FrameBegin(0);

        C2D_SceneBegin(s_top_screen);
        renderRect(0, 0, s_hardware_w, s_hardware_h, 0.0f, 0.0f, 0.0f, 1.0f, true);

        C2D_SceneBegin(s_right_screen);
        renderRect(0, 0, s_hardware_w, s_hardware_h, 0.0f, 0.0f, 0.0f, 1.0f, true);

        C2D_SceneBegin(s_bottom_screen);
        renderRect(0, 0, s_hardware_w, s_hardware_h, 0.0f, 0.0f, 0.0f, 1.0f, true);

        C3D_FrameEnd(0);
    }
}

void repaint()
{
    if(!g_in_frame)
        return;

    int tex_w = ScreenW / 2;
    int tex_h = ScreenH / 2;

    int base_off_x = s_hardware_w / 2 - tex_w / 2;
    int base_off_y = s_hardware_h / 2 - tex_h / 2;

    constexpr int shift = s_max_3d_offset / 2;
    constexpr double bg_shift = shift;
    constexpr double mid_shift = shift * .4;

    s_depth_slider = osGet3DSliderState();

    // leave the draw context and wait for vblank...
    if(s_depth_slider <= 0.05 || s_single_layer_mode)
    {
        C2D_TargetClear(s_top_screen, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
        C2D_SceneBegin(s_top_screen);
        C2D_DrawImageAt(s_layer_ims[0], base_off_x - shift, base_off_y, 0);
        if(!s_single_layer_mode)
        {
            C2D_DrawImageAt(s_layer_ims[1], base_off_x - shift, base_off_y, 0);
            C2D_DrawImageAt(s_layer_ims[2], base_off_x - shift, base_off_y, 0);
            C2D_DrawImageAt(s_layer_ims[3], base_off_x - shift, base_off_y, 0);
        }
    }
    else
    {
        C2D_TargetClear(s_top_screen, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
        C2D_SceneBegin(s_top_screen);
        C2D_DrawImageAt(s_layer_ims[0], base_off_x - shift - (int)(bg_shift * s_depth_slider), base_off_y, 0);
        C2D_DrawImageAt(s_layer_ims[1], base_off_x - shift - (int)(mid_shift * s_depth_slider), base_off_y, 0);
        C2D_DrawImageAt(s_layer_ims[2], base_off_x - shift, base_off_y, 0);
        C2D_DrawImageAt(s_layer_ims[3], base_off_x - shift + (int)(mid_shift * s_depth_slider), base_off_y, 0);

        C2D_TargetClear(s_right_screen, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
        C2D_SceneBegin(s_right_screen);
        C2D_DrawImageAt(s_layer_ims[0], base_off_x - shift + (int)(bg_shift * s_depth_slider), base_off_y, 0);
        C2D_DrawImageAt(s_layer_ims[1], base_off_x - shift + (int)(mid_shift * s_depth_slider), base_off_y, 0);
        C2D_DrawImageAt(s_layer_ims[2], base_off_x - shift, 0, 0);
        C2D_DrawImageAt(s_layer_ims[3], base_off_x - shift - (int)(mid_shift * s_depth_slider), base_off_y, 0);
    }
    s_current_frame ++;
    g_in_frame = false;
    C3D_FrameEnd(0);
}

void cancelFrame()
{
    if(!g_in_frame)
        return;

    C2D_SceneBegin(s_top_screen);
    renderRect(0, 0, s_hardware_w, s_hardware_h, 0.0f, 0.0f, 0.0f, 1.0f, true);

    C2D_SceneBegin(s_right_screen);
    renderRect(0, 0, s_hardware_w, s_hardware_h, 0.0f, 0.0f, 0.0f, 1.0f, true);

    C2D_SceneBegin(s_bottom_screen);
    renderRect(0, 0, s_hardware_w, s_hardware_h, 0.0f, 0.0f, 0.0f, 1.0f, true);

    C3D_FrameEnd(0);

    s_current_frame ++;
    g_in_frame = false;

    C3D_FrameEnd(0);
}

void mapToScreen(int x, int y, int *dx, int *dy)
{
    int base_off_x = s_hardware_w / 2 - ScreenW / 4;
    int base_off_y = s_hardware_h / 2 - ScreenH / 4;

    *dx = x - base_off_x;
    *dy = y - base_off_y;
}

void mapFromScreen(int scr_x, int scr_y, int *window_x, int *window_y)
{
    int base_off_x = s_hardware_w / 2 - ScreenW / 4;
    int base_off_y = s_hardware_h / 2 - ScreenH / 4;

    *window_x = scr_x + base_off_x;
    *window_y = scr_y + base_off_y;
}

void updateViewport()
{
    resetViewport();
    offsetViewport(0, 0);
}

void resetViewport()
{
    setViewport(0, 0, ScreenW, ScreenH);
}

void setViewport(int x, int y, int w, int h)
{
    int offset_x = s_viewport_offset_x - s_viewport_x;
    int offset_y = s_viewport_offset_y - s_viewport_y;
    int offset_x_bak = s_viewport_offset_x_bak - s_viewport_x;
    int offset_y_bak = s_viewport_offset_y_bak - s_viewport_y;

    s_viewport_x = x / 2;
    s_viewport_y = y / 2;
    s_viewport_w = w / 2;
    s_viewport_h = h / 2;

    s_viewport_offset_x = s_viewport_x + offset_x;
    s_viewport_offset_y = s_viewport_y + offset_y;
    s_viewport_offset_x_bak = s_viewport_x + offset_x_bak;
    s_viewport_offset_y_bak = s_viewport_y + offset_y_bak;
}

void offsetViewport(int x, int y)
{
    if(s_viewport_offset_ignore)
    {
        s_viewport_offset_x_bak = s_viewport_x + x / 2;
        s_viewport_offset_y_bak = s_viewport_y + y / 2;
    }
    else
    {
        s_viewport_offset_x = s_viewport_x + x / 2;
        s_viewport_offset_y = s_viewport_y + y / 2;
    }
}

void offsetViewportIgnore(bool en)
{
    if(s_viewport_offset_ignore == en)
        return;

    s_viewport_offset_ignore = en;

    if(en)
    {
        s_viewport_offset_x_bak = s_viewport_offset_x;
        s_viewport_offset_y_bak = s_viewport_offset_y;
    }
    else
    {
        s_viewport_offset_x = s_viewport_offset_x_bak;
        s_viewport_offset_y = s_viewport_offset_y_bak;
    }
}

void setTransparentColor(StdPicture &target, uint32_t rgb)
{
    UNUSED(target);
    UNUSED(rgb);
}

StdPicture LoadPicture(const std::string& path, const std::string& maskPath, const std::string& maskFallbackPath)
{
    (void)maskPath;
    (void)maskFallbackPath;

    StdPicture target;
    C2D_SpriteSheet sourceImage;

    if(!GameIsActive)
        return target; // do nothing when game is closed

    target.inited = false;
    target.l.path = path;
    if(target.l.path.empty())
        return target;

    target.inited = true;
    target.l.lazyLoaded = false;

    sourceImage = C2D_SpriteSheetLoad(target.l.path.c_str());
    if(sourceImage)
    {
        s_loadTexture(target, sourceImage);

        s_num_textures_loaded ++;
    }

    if(!target.d.texture)
    {
        pLogWarning("FAILED TO LOAD!!! %s\n", path.c_str());
        target.inited = false;
    }

    return target;
}


StdPicture lazyLoadPicture(const std::string& path, const std::string& maskPath, const std::string& maskFallbackPath)
{
    (void)maskPath;
    (void)maskFallbackPath;

    StdPicture target;
    if(!GameIsActive)
        return target; // do nothing when game is closed

    target.inited = false;
    target.l.path = path;
    if(target.l.path.empty())
        return target;

    target.inited = true;

    target.l.lazyLoaded = true;

    // We need to figure out the height and width!
    std::string sizePath = path + ".size";
    FILE* fs = fopen(sizePath.c_str(), "r");

    // NOT null-terminated: wwww\nhhhh\n
    char contents[10];

    if(fs != nullptr)
    {
        fread(&contents[0], 1, 10, fs);
        contents[4] = '\0';
        contents[9] = '\0';
        target.w = atoi(&contents[0]);
        target.h = atoi(&contents[5]);
        if(fclose(fs))
            pLogWarning("lazyLoadPicture: Couldn't close file.");
    }
    // lazy load and unload to read dimensions if it doesn't exist.
    // unload is essential because lazy load would save the address incorrectly.
    else
    {
        pLogWarning("lazyLoadPicture: Couldn't open size file.");
        lazyLoad(target);
        lazyUnLoad(target);
    }

    return target;
}

static C2D_SpriteSheet s_tryHardToLoadC2D_SpriteSheet(const char* path)
{
    C2D_SpriteSheet sourceImage = C2D_SpriteSheetLoad(path);
    int i;

    if(!sourceImage)
    {
        // wish I knew the largest contiguous portion of memory
        // max tex should be 4194304
        for(i = 0; i < 10; i ++)
        {
            if(linearSpaceFree() > 4000000) break;
            if(!s_freeTextureMem()) break;
        }
        sourceImage = C2D_SpriteSheetLoad(path);

        if(!sourceImage)
        {
            for(i = 0; i < 10; i ++)
            {
                if(linearSpaceFree() > 8000000) break;
                if(!s_freeTextureMem()) break;
            }
            sourceImage = C2D_SpriteSheetLoad(path);

            if(!sourceImage)
            {
                for(i = 0; i < 25; i ++)
                {
                    if(linearSpaceFree() > 20000000) break;
                    if(!s_freeTextureMem()) break;
                }
                sourceImage = C2D_SpriteSheetLoad(path);
            }
        }
    }

    return sourceImage;
}

void lazyLoad(StdPicture &target)
{
    if(!target.inited || !target.l.lazyLoaded || target.d.texture)
        return;

    C2D_SpriteSheet sourceImage;
    std::string suppPath;

    sourceImage = s_tryHardToLoadC2D_SpriteSheet(target.l.path.c_str()); // some other source image

    if(!sourceImage)
    {
        pLogWarning("Permanently failed to load %s, %lu free", target.l.path.c_str(), linearSpaceFree());
        pLogWarning("Error: %d (%s)", errno, strerror(errno));
        target.inited = false;
        return;
    }

    s_loadTexture(target, sourceImage);

    if(target.h > 2048)
    {
        suppPath = target.l.path + '1';
        sourceImage = s_tryHardToLoadC2D_SpriteSheet(suppPath.c_str());
        if(!sourceImage)
        {
            pLogWarning("Permanently failed to load %s, %lu free", suppPath.c_str(), linearSpaceFree());
            pLogWarning("Error: %d (%s)", errno, strerror(errno));
        }
        else
            s_loadTexture2(target, sourceImage);
    }
    if(target.h > 4096)
    {
        suppPath = target.l.path + '2';
        sourceImage = C2D_SpriteSheetLoad(suppPath.c_str());
        if(!sourceImage)
        {
            pLogWarning("Permanently failed to load %s, %lu free", suppPath.c_str(), linearSpaceFree());
            pLogWarning("Error: %d (%s)", errno, strerror(errno));
        }
        else
            s_loadTexture3(target, sourceImage);
    }

    s_num_textures_loaded++;

    if(target.w >= 256 || target.h >= 256)
    {
        s_big_pictures.insert(&target);
        s_num_big_pictures_loaded++;
    }

    if(linearSpaceFree() < 4194304)
        s_freeTextureMem();
}

void lazyPreLoad(StdPicture &target)
{
    lazyLoad(target);
}

void lazyUnLoad(StdPicture &target)
{
    if(!target.inited || !target.l.lazyLoaded || !target.d.texture)
        return;

    deleteTexture(target, true);
}

void deleteTexture(StdPicture &tx, bool lazyUnload)
{
    if(!tx.inited)
        return;

    if(s_big_pictures.find(&tx) != s_big_pictures.end())
    {
        s_big_pictures.erase(&tx);
        s_num_big_pictures_loaded --;
    }

    if(tx.d.texture)
    {
        s_num_textures_loaded --;
        if(s_texture_bank.find(tx.d.texture) != s_texture_bank.end())
            s_texture_bank.erase(tx.d.texture);
        C2D_SpriteSheetFree(tx.d.texture);
        tx.d.texture = nullptr;
    }

    if(tx.d.texture2)
    {
        if(s_texture_bank.find(tx.d.texture2) != s_texture_bank.end())
            s_texture_bank.erase(tx.d.texture2);
        C2D_SpriteSheetFree(tx.d.texture2);
        tx.d.texture2 = nullptr;
    }

    if(tx.d.texture3)
    {
        if(s_texture_bank.find(tx.d.texture3) != s_texture_bank.end())
            s_texture_bank.erase(tx.d.texture3);
        C2D_SpriteSheetFree(tx.d.texture3);
        tx.d.texture3 = nullptr;
    }

    if(!lazyUnload)
    {
        tx.inited = false;
        tx.l.lazyLoaded = false;
        tx.w = 0;
        tx.h = 0;
        tx.frame_w = 0;
        tx.frame_h = 0;
    }
}

void renderRect(int x, int y, int w, int h, float red, float green, float blue, float alpha, bool filled)
{
    uint32_t clr = C2D_Color32f(red, green, blue, alpha);

    // Filled is always True in this game
    if(filled)
        C2D_DrawRectSolid(x / 2 + s_viewport_offset_x,
                          y / 2 + s_viewport_offset_y,
                          0, w / 2, h / 2, clr);
    else
    {
        C2D_DrawRectangle(x / 2 + s_viewport_offset_x,
                          y / 2 + s_viewport_offset_y,
                          0, 1, h / 2, clr, clr, clr, clr);
        C2D_DrawRectangle(x / 2 + s_viewport_offset_x+w / 2 - 1,
                          y / 2 + s_viewport_offset_y,
                          0, 1, h / 2, clr, clr, clr, clr);
        C2D_DrawRectangle(x / 2 + s_viewport_offset_x,
                          y / 2 + s_viewport_offset_y,
                          0, w / 2, 1, clr, clr, clr, clr);
        C2D_DrawRectangle(x / 2 + s_viewport_offset_x,
                          y / 2 + s_viewport_offset_y+h / 2 - 1,
                          0, w / 2, 1, clr, clr, clr, clr);
    }
}

void renderRectBR(int _left, int _top, int _right, int _bottom, float red, float green, float blue, float alpha)
{
    renderRect(_left, _top, _right-_left, _bottom-_top, red, green, blue, alpha, true);
}

void renderCircle(int cx, int cy,
                  int radius,
                  float red , float green, float blue, float alpha,
                  bool filled)
{
}

void renderCircleHole(int cx, int cy,
                      int radius,
                      float red, float green, float blue, float alpha)
{
}

inline int ROUNDDIV2(int x)
{
    return (x<0)?(x - 1) / 2:x / 2;
}

inline float ROUNDDIV2(float x)
{
    return std::nearbyintf(std::roundf(x) / 2.0f);
}

inline float ROUNDDIV2(double x)
{
    return std::nearbyintf(std::roundf((float)x) / 2.0f);
}

inline float FLOORDIV2(float x)
{
    return std::floor(x / 2.0f);
}

inline void i_renderTexturePrivate(float xDst, float yDst, float wDst, float hDst,
                             StdPicture &tx,
                             float xSrc, float ySrc, float wSrc, float hSrc,
                             float rotateAngle, FPoint_t *center, unsigned int flip,
                             float red, float green, float blue, float alpha)
{
    if(!tx.inited)
        return;

    if(!tx.d.texture && tx.l.lazyLoaded)
        lazyLoad(tx);

    tx.d.last_draw_frame = s_current_frame;

    if(!tx.d.texture)
        return;
    if(xDst > s_viewport_w || yDst > s_viewport_h)
        return;

    // automatic flipping based on SMBX style!
    unsigned int mode = 0;
    while(ySrc >= tx.h / 2 && mode < 3)
    {
        ySrc -= tx.h / 2;
        mode += 1;
    }
    flip ^= mode;

    // texture boundaries
    // this never happens unless there was an invalid input
    // if((xSrc < 0.0f) || (ySrc < 0.0f)) return;

    // TODO: graphics tests for how offscreen draws interact with flips
    //       handling rotations properly is probably impossible
    if(xDst < 0.0f)
    {
        if(!(flip & X_FLIP_HORIZONTAL))
            xSrc -= xDst * wSrc / wDst;

        if(wDst+xDst > s_viewport_w)
        {
            if(flip & X_FLIP_HORIZONTAL)
                xSrc += (wDst + xDst - s_viewport_w) * wSrc / wDst;
            wSrc = s_viewport_w * wSrc / wDst;
            wDst = s_viewport_w;
        }
        else
        {
            wSrc += xDst * wSrc / wDst;
            wDst += xDst;
        }
        xDst = 0.0f;
    }
    else if(xDst + wDst > s_viewport_w)
    {
        if(flip & X_FLIP_HORIZONTAL)
            xSrc += (wDst + xDst - s_viewport_w) * wSrc / wDst;
        wSrc = (s_viewport_w - xDst) * wSrc / wDst;
        wDst = (s_viewport_w - xDst);
    }

    if(yDst < 0.0f)
    {
        if(!(flip & X_FLIP_VERTICAL))
            ySrc -= yDst * hSrc / hDst;

        if(hDst + yDst > s_viewport_h)
        {
            if(flip & X_FLIP_VERTICAL)
                ySrc += (hDst + yDst - s_viewport_h) * hSrc / hDst;
            hSrc = s_viewport_h * hSrc / hDst;
            hDst = s_viewport_h;
        }
        else
        {
            hSrc += yDst * hSrc / hDst;
            hDst += yDst;
        }
        yDst = 0.0f;
    }
    else if(yDst + hDst > s_viewport_h)
    {
        if(flip & X_FLIP_VERTICAL)
            ySrc += (hDst + yDst - s_viewport_h) * hSrc / hDst;
        hSrc = (s_viewport_h - yDst) * hSrc / hDst;
        hDst = (s_viewport_h - yDst);
    }

    C2D_Image* to_draw = nullptr;
    C2D_Image* to_draw_2 = nullptr;

    // Don't go more than size of texture
    // Failure conditions should only happen if texture is smaller than expected
    if(xSrc + wSrc > tx.w / 2)
    {
        wDst = (tx.w / 2 - xSrc) * wDst / wSrc;
        wSrc = tx.w / 2 - xSrc;
        if(wDst < 0.0f)
            return;
    }
    if(ySrc + hSrc > tx.h / 2)
    {
        hDst = (tx.h / 2 - ySrc) * hDst / hSrc;
        hSrc = tx.h / 2 - ySrc;
        if(hDst < 0.0f)
            return;
    }

    if(ySrc + hSrc > 1024.0f)
    {
        if(ySrc + hSrc > 2048.0f)
        {
            if(tx.d.texture3)
                to_draw = &tx.d.image3;
            if(ySrc < 2048.0f && tx.d.texture2)
                to_draw_2 = &tx.d.image2;
            ySrc -= 1024.0f;
        }
        else
        {
            if(tx.d.texture2)
                to_draw = &tx.d.image2;
            if(ySrc < 1024.0f)
                to_draw_2 = &tx.d.image;
        }
        // draw the top pic
        if(to_draw_2 != nullptr)
        {
            if(rotateAngle != 0.0)
                C2D_DrawImage_Custom_Rotated(*to_draw_2, xDst + s_viewport_offset_x, yDst + s_viewport_offset_y, wDst, (1024.0f - ySrc) * hDst / hSrc,
                                     xSrc, ySrc, wSrc, 1024.0f - ySrc, flip, center, rotateAngle, red, green, blue, alpha);
            else
                C2D_DrawImage_Custom(*to_draw_2, xDst + s_viewport_offset_x, yDst + s_viewport_offset_y, wDst, (1024.0f - ySrc) * hDst / hSrc,
                                     xSrc, ySrc, wSrc, 1024.0f - ySrc, flip, red, green, blue, alpha);
            yDst += (1024.0f - ySrc) * hDst / hSrc;
            hDst -= (1024.0f - ySrc) * hDst / hSrc;
            hSrc -= (1024.0f - ySrc);
            ySrc = 0.0f;
        }
        else
            ySrc -= 1024.0f;
    }
    else to_draw = &tx.d.image;

    if(to_draw == nullptr) return;

    if(rotateAngle != 0.0)
        C2D_DrawImage_Custom_Rotated(*to_draw, xDst + s_viewport_offset_x, yDst + s_viewport_offset_y, wDst, hDst,
                             xSrc, ySrc, wSrc, hSrc, flip, center, rotateAngle, red, green, blue, alpha);
    else
        C2D_DrawImage_Custom(*to_draw, xDst + s_viewport_offset_x, yDst + s_viewport_offset_y, wDst, hDst,
                             xSrc, ySrc, wSrc, hSrc, flip, red, green, blue, alpha);
}

// public draw methods

void renderTextureScale(double xDst, double yDst, double wDst, double hDst,
                            StdPicture &tx,
                            int xSrc, int ySrc, int wSrc, int hSrc,
                            float red, float green, float blue, float alpha)
{
    i_renderTexturePrivate(
        ROUNDDIV2(xDst), ROUNDDIV2(yDst), ROUNDDIV2(wDst), ROUNDDIV2(hDst),
        tx,
        xSrc / 2, ySrc / 2, wSrc / 2, hSrc / 2,
        0.0f, nullptr, X_FLIP_NONE,
        red, green, blue, alpha);
}

void renderTexture(double xDst, double yDst, double wDst, double hDst,
                            StdPicture &tx,
                            int xSrc, int ySrc,
                            float red, float green, float blue, float alpha)
{
    float w = ROUNDDIV2(wDst);
    float h = ROUNDDIV2(hDst);
    i_renderTexturePrivate(
        ROUNDDIV2(xDst), ROUNDDIV2(yDst), w, h,
        tx,
        xSrc / 2, ySrc / 2, w, h,
        0.0f, nullptr, X_FLIP_NONE,
        red, green, blue, alpha);
}

void renderTexture(float xDst, float yDst, StdPicture &tx,
                   float red, float green, float blue, float alpha)
{
    int w = tx.w / 2;
    int h = tx.h / 2;
    i_renderTexturePrivate(
        ROUNDDIV2(xDst), ROUNDDIV2(yDst), w, h,
        tx,
        0.0f, 0.0f, w, h,
        0.0f, nullptr, X_FLIP_NONE,
        red, green, blue, alpha);
}

void renderTexture(int xDst, int yDst, StdPicture &tx, float red, float green, float blue, float alpha)
{
    int w = tx.w / 2;
    int h = tx.h / 2;
    i_renderTexturePrivate(
        ROUNDDIV2(xDst), ROUNDDIV2(yDst), w, h,
        tx,
        0.0f, 0.0f, w, h,
        0.0f, nullptr, X_FLIP_NONE,
        red, green, blue, alpha);
}

void renderTextureScale(int xDst, int yDst, int wDst, int hDst, StdPicture &tx, float red, float green, float blue, float alpha)
{
    i_renderTexturePrivate(
        ROUNDDIV2(xDst), ROUNDDIV2(yDst), tx.w / 2, tx.h / 2,
        tx,
        0.0f, 0.0f, wDst / 2, hDst / 2,
        0.0f, nullptr, X_FLIP_NONE,
        red, green, blue, alpha);
}

void renderTextureFL(double xDst, double yDst, double wDst, double hDst,
                          StdPicture &tx,
                          int xSrc, int ySrc,
                          double rotateAngle, FPoint_t *center, unsigned int flip,
                          float red, float green, float blue, float alpha)
{
    float w = ROUNDDIV2(wDst);
    float h = ROUNDDIV2(hDst);
    i_renderTexturePrivate(
        ROUNDDIV2(xDst), ROUNDDIV2(yDst), w, h,
        tx,
        xSrc / 2, ySrc / 2, w, h,
        rotateAngle, center, flip,
        red, green, blue, alpha);
}

void renderTextureScaleEx(double xDst, double yDst, double wDst, double hDst,
                          StdPicture &tx,
                          int xSrc, int ySrc,
                          int wSrc, int hSrc,
                          double rotateAngle, FPoint_t *center, unsigned int flip,
                          float red, float green, float blue, float alpha)
{
    float w = ROUNDDIV2(wDst);
    float h = ROUNDDIV2(hDst);
    i_renderTexturePrivate(
        ROUNDDIV2(xDst), ROUNDDIV2(yDst), w, h,
        tx,
        xSrc / 2, ySrc / 2, wSrc / 2, hSrc / 2,
        rotateAngle, center, flip,
        red, green, blue, alpha);
}


size_t lazyLoadedBytes()
{
    return 0;
}

void lazyLoadedBytesReset()
{
}

}; // namespace XRender
