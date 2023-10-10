/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2023 Vitaly Novichkov <admin@wohlnet.ru>
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

#include <FreeImageLite.h>

// strangely undocumented import necessary to use the FreeImage handle functions
extern void SetDefaultIO(FreeImageIO *io);

#include <Graphics/graphics_funcs.h>

#include <Logger/logger.h>
#include <Utils/files.h>

#include "sdl_proxy/sdl_stdinc.h"

#include "globals.h"
#include "video.h"
#include "frame_timer.h"
#include "main/cheat_code.h"
#include "core/render.h"
#include "editor/new_editor.h"

#include "change_res.h"
#include "core/3ds/should_swap_screen.h"

#include "core/minport/render_minport_shared.h"

// #include "core/3ds/n3ds-clock.h"
// #include "second_screen.h"
#include "c2d_draw.h"

// used for crash prevention
extern u32 gpuCmdBufOffset, gpuCmdBufSize;

namespace XRender
{

static const C2D_SpriteSheet HEAP_MANAGED_TEXTURE = reinterpret_cast<C2D_SpriteSheet>(reinterpret_cast<void*>(0xffffffff));

uint32_t s_current_frame = 0;
float s_depth_slider = 0.;

bool g_in_frame = false;
bool g_screen_swapped = false;

C3D_RenderTarget* s_top_screen;
C3D_RenderTarget* s_right_screen;
C3D_RenderTarget* s_bottom_screen;

Tex3DS_SubTexture s_layer_subtexs[4];
C3D_Tex s_layer_texs[4];
C2D_Image s_layer_ims[4];
C3D_RenderTarget* s_layer_targets[4];
bool s_single_layer_mode = false;

int s_tex_w = 0;
int s_tex_show_w = 0;
int s_tex_h = 0;

int s_num_textures_loaded = 0;

C3D_RenderTarget* s_cur_target = nullptr;

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

    s_tex_w = ScreenW / 2;
    s_tex_h = ScreenH / 2;

    uint16_t mem_w = (s_tex_w <= 256) ? 256 : 512;
    uint16_t mem_h = (s_tex_h <= 256) ? 256 : 512;

    if(s_tex_w > 512)
        mem_w = 1024;

    if(s_tex_w > mem_w)
        s_tex_w = mem_w;

    if(s_tex_h > mem_h)
        s_tex_h = mem_h;

    if(mem_w >= 512 && mem_h == 512)
        s_single_layer_mode = true;
    else if(should_swap_screen())
        s_single_layer_mode = true;
    else
        s_single_layer_mode = false;

    for(int i = 0; i < 4; i++)
    {
        C3D_TexInitVRAM(&s_layer_texs[i], mem_w, mem_h, GPU_RGBA8);
        s_layer_targets[i] = C3D_RenderTargetCreateFromTex(&s_layer_texs[i], GPU_TEXFACE_2D, 0, GPU_RB_DEPTH24_STENCIL8);
        s_layer_subtexs[i] = {(uint16_t)s_tex_w, (uint16_t)s_tex_h, 0.0, 1.0, (float)((double)s_tex_w / (double)mem_w), 1.0f - (float)((double)s_tex_h / (double)mem_h)};
        s_layer_ims[i].tex = &s_layer_texs[i];
        s_layer_ims[i].subtex = &s_layer_subtexs[i];

        if(s_single_layer_mode)
            break;
    }

    // s_tex_show_w = s_tex_w - MAX_3D_OFFSET;
    s_tex_show_w = s_tex_w;
}

void s_ensureInFrame()
{
    if(!g_in_frame)
    {
        minport_initFrame();

        // if(g_screen_swapped != should_swap_screen())
        //     UpdateInternalRes();

        C3D_FrameBegin(0);

        for(int layer = 0; layer < 4; layer++)
        {
            C2D_TargetClear(s_layer_targets[layer], C2D_Color32(0, 0, 0, 0));

            if(s_single_layer_mode)
                break;
        }

        C2D_TargetClear(s_bottom_screen, C2D_Color32(0, 0, 0, 0));

        g_in_frame = true;
    }
}

void s_clearAllTextures()
{
    for(StdPicture* p = g_render_chain_tail; p != nullptr;)
    {
        StdPicture* last_p = p;
        p = p->d.next_texture;

        unloadTexture(*last_p);
    }
}

FIBITMAP* robust_FILoad(const std::string& path, int target_w)
{
    if(path.empty())
    {
        pLogWarning("FreeImageLite failed to load image due to empty path");
        return nullptr;
    }

    FILE *handle = Files::utf8_fopen(path.c_str(), "rb");

    FreeImageIO io;
    SetDefaultIO(&io);
    FREE_IMAGE_FORMAT formato = FreeImage_GetFileTypeFromHandle(&io, (fi_handle)handle);

    if(formato == FIF_UNKNOWN)
    {
        pLogWarning("FreeImageLite failed to load image due to unknown format");
        fclose(handle);
        return nullptr;
    }

    FIBITMAP *rawImage = FreeImage_LoadFromHandle(formato, &io, (fi_handle)handle);
    if(!rawImage)
    {
        pLogWarning("FreeImageLite failed to load image due to lack of memory, trying to free some memory");
        minport_freeTextureMemory();
        rawImage = FreeImage_LoadFromHandle(formato, &io, (fi_handle)handle);

        if(!rawImage)
        {
            fclose(handle);
            return nullptr;
        }

        pLogDebug("Loaded successfully!");
    }

    fclose(handle);

    int32_t w = static_cast<int32_t>(FreeImage_GetWidth(rawImage));
    int32_t h = static_cast<int32_t>(FreeImage_GetHeight(rawImage));

    pLogDebug("loading %s, freeimage reports %dx%d (%d-bit), pitch %u", path.c_str(), w, h, (int)FreeImage_GetBPP(rawImage), FreeImage_GetPitch(rawImage));

    if((w == 0) || (h == 0))
    {
        GraphicsHelps::closeImage(rawImage);
        pLogWarning("Error loading of image file:\n"
                    "Reason: %s."
                    "Zero image size!");
        return nullptr;
    }

    if(w < target_w && FreeImage_GetBPP(rawImage) == 32)
    {
        // don't rescale, just flip
        FreeImage_FlipVertical(rawImage);
        return rawImage;
    }

    FIBITMAP *sourceImage = GraphicsHelps::fastScaleDownAnd32Bit(rawImage, w >= target_w);
    if(!sourceImage)
    {
        pLogWarning("Failed to convert image due to lack of memory, trying to free some memory");
        minport_freeTextureMemory();
        sourceImage = GraphicsHelps::fastScaleDownAnd32Bit(rawImage, w >= target_w);

        if(!sourceImage)
        {
            FreeImage_Unload(rawImage);
            return nullptr;
        }

        pLogDebug("Converted successfully!");
    }

    FreeImage_Unload(rawImage);

    FreeImage_FlipVertical(sourceImage);

    return sourceImage;
}

static int s_nextPowerOfTwo(int val)
{
    int power = 8;
    while(power < val)
        power *= 2;
    return power;
}

static C2D_Image s_RawToSwizzledRGBA(const uint8_t* src, uint32_t wsrc, uint32_t hsrc, uint32_t pitch, bool mask, bool downscale = true)
{
    // calculate destination dimensions, including downscaling and required padding
    int sf = (downscale ? 2 : 1);
    uint32_t wdst = (wsrc + sf - 1) / sf;
    uint32_t hdst = (hsrc + sf - 1) / sf;

    uint32_t wtex = s_nextPowerOfTwo(wdst);
    uint32_t htex = s_nextPowerOfTwo(hdst);

    C2D_Image img;

    img.tex = new C3D_Tex;
    img.subtex = new Tex3DS_SubTexture({(u16)wdst, (u16)hdst, 0.0f, 1.0f, wdst / (float)wtex, 1.0f - (hdst / (float)htex)});

    pLogDebug("This part is w %d h %d p %d -> %d %d %f %f %f %f", (int)wsrc, (int)hsrc, (int)pitch, (int)wtex, (int)htex, img.subtex->left, img.subtex->top, img.subtex->right, img.subtex->bottom);

    if(!C3D_TexInit(img.tex, wtex, htex, GPU_RGBA8))
    {
        pLogDebug("Triggering free texture memory due to failed texture load (%u bytes free)", (unsigned)linearSpaceFree());
        minport_freeTextureMemory();

        if(!C3D_TexInit(img.tex, wtex, htex, GPU_RGBA8))
        {
            delete img.tex;
            delete img.subtex;
            img.tex = nullptr;
            img.subtex = nullptr;
            return img;
        }
    }

    C3D_TexSetFilter(img.tex, GPU_NEAREST, GPU_NEAREST);
    C3D_TexSetWrap(img.tex, GPU_CLAMP_TO_BORDER, GPU_CLAMP_TO_BORDER);
    img.tex->border = mask ? 0xFFFFFFFF : 0x00FFFFFF;

    for(u32 y = 0; y < hdst; y++)
    {
        for(u32 x = 0; x < wdst; x++)
        {
            const u32 dst_pixel = ((((y >> 3) * (wtex >> 3) + (x >> 3)) << 6) +
                                ((x & 1) | ((y & 1) << 1) | ((x & 2) << 1) | ((y & 2) << 2) |
                                ((x & 4) << 2) | ((y & 4) << 3))) * 4;

            const u32 src_pixel = (y * sf * pitch) + (x * sf * 4);

            ((uint8_t*)img.tex->data)[dst_pixel + 0] = src[src_pixel + 3];
            ((uint8_t*)img.tex->data)[dst_pixel + 1] = src[src_pixel + 0];
            ((uint8_t*)img.tex->data)[dst_pixel + 2] = src[src_pixel + 1];
            ((uint8_t*)img.tex->data)[dst_pixel + 3] = src[src_pixel + 2];
        }
    }

    // fill border if mask
    if(mask)
    {
        for(u32 y = 0; y < htex; y++)
        {
            for(u32 x = (y < hdst) ? wdst : 0; x < wtex; x++)
            {
                const u32 dst_pixel = ((((y >> 3) * (wtex >> 3) + (x >> 3)) << 6) +
                                    ((x & 1) | ((y & 1) << 1) | ((x & 2) << 1) | ((y & 2) << 2) |
                                    ((x & 4) << 2) | ((y & 4) << 3))) * 4;

                ((uint8_t*)img.tex->data)[dst_pixel + 0] = 255;
                ((uint8_t*)img.tex->data)[dst_pixel + 1] = 255;
                ((uint8_t*)img.tex->data)[dst_pixel + 2] = 255;
                ((uint8_t*)img.tex->data)[dst_pixel + 3] = 255;
            }
        }
    }

    return img;
}

void s_loadTexture(StdPicture& target, void* data, int width, int height, int pitch, bool mask)
{
    // downscale if logical width matches the actual width of the texture, otherwise don't
    bool downscale = (width >= target.w);

    int max_size = (downscale ? 2048 : 1024);

    pLogDebug("Loading %s %d, w %d h %d p %d", target.l.path.c_str(), (int)mask, width, height, pitch);

    // if(width > max_size && height <= max_size)
    //     target.d.multi_horizontal = true;

    for(int i = 0; i < 3; i++)
    {
        int start_x, start_y;

        // if(target.d.multi_horizontal)
        if(false)
        {
            start_y = 0;
            start_x = i * max_size;
        }
        else
        {
            start_x = 0;
            start_y = i * max_size;
        }

        int w_i = width - start_x;
        int h_i = height - start_y;

        if(w_i > max_size)
            w_i = max_size;

        if(h_i > max_size)
            h_i = max_size;

        if(w_i > 0 && h_i > 0)
        {
            target.d.image[i + 3 * mask] = s_RawToSwizzledRGBA((uint8_t*)data + (start_y * width + start_x) * 4, w_i, h_i, pitch, mask, downscale);

            if(target.d.image[i + 3 * mask].tex)
            {
                // printf("We initialized with %u %u\n", wdst, hdst);
                target.d.texture[i + 3 * mask] = HEAP_MANAGED_TEXTURE;
            }
            else
                break;
        }
    }
}

void s_loadTexture(StdPicture& target, C2D_SpriteSheet& sheet)
{
    C2D_Image im = C2D_SpriteSheetGetImage(sheet, 0);

    target.d.texture[0] = sheet;
    target.d.image[0] = im;

    if(!target.w)
    {
        target.w = im.subtex->width * 2;
        target.h = im.subtex->height * 2;
    }
}

void s_loadTexture2(StdPicture& target, C2D_SpriteSheet& sheet)
{
    C2D_Image im = C2D_SpriteSheetGetImage(sheet, 0);

    target.d.texture[1] = sheet;
    target.d.image[1] = im;
}

void s_loadTexture3(StdPicture& target, C2D_SpriteSheet& sheet)
{
    C2D_Image im = C2D_SpriteSheetGetImage(sheet, 0);

    target.d.texture[2] = sheet;
    target.d.image[2] = im;
}

static inline void s_resetBlend()
{
    C3D_AlphaBlend(GPU_BLEND_ADD, GPU_BLEND_ADD, GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA, GPU_ONE_MINUS_DST_ALPHA, GPU_ONE);
}

static inline void s_mergeBlend()
{
    C3D_AlphaBlend(GPU_BLEND_ADD, GPU_BLEND_ADD, GPU_ONE, GPU_ONE_MINUS_SRC_ALPHA, GPU_ONE, GPU_ONE_MINUS_SRC_ALPHA);
}

bool init()
{
    // 3ds libs
    gfxInitDefault();

    gfxSet3D(true); // Enable stereoscopic 3D

    // default command buffer size is 0x40000 (256kb) but this is insufficient in extreme cases such as levels that paint the screen with small masked BGOs
    size_t cmdbuf_size = 0xc0000;
    C3D_Init(cmdbuf_size);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    C2D_Prepare();

    C2D_SetTintMode(C2D_TintMult);
    s_resetBlend();

    // consoleInit(GFX_BOTTOM, NULL);

    s_top_screen = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
    s_right_screen = C2D_CreateScreenTarget(GFX_TOP, GFX_RIGHT);
    s_bottom_screen = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);
    // s_bottom_screen = s_right_screen;

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

    if(!s_single_layer_mode)
    {
        // screen plane layer
        C2D_SceneBegin(s_layer_targets[2]);
        s_cur_target = s_layer_targets[2];
    }
    else
    {
        C2D_SceneBegin(s_layer_targets[0]);
        s_cur_target = s_layer_targets[0];
    }

    C2D_ViewReset();
}

void setTargetScreen()
{
}

void setTargetMainScreen()
{
    s_ensureInFrame();

    C2D_TargetClear(s_top_screen, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));

    C2D_SceneBegin(s_top_screen);
    s_cur_target = s_top_screen;

    C2D_ViewReset();
}

void setTargetSubScreen()
{
    s_ensureInFrame();

    C2D_TargetClear(s_bottom_screen, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));

    C2D_SceneBegin(s_bottom_screen);
    s_cur_target = s_bottom_screen;

    C2D_ViewReset();
}

void setTargetLayer(int layer)
{
    s_ensureInFrame();

    if(!s_single_layer_mode)
    {
        C2D_SceneBegin(s_layer_targets[layer]);
        s_cur_target = s_layer_targets[layer];
    }
    else
    {
        C2D_SceneBegin(s_layer_targets[0]);
        s_cur_target = s_layer_targets[0];
    }

    minport_ApplyViewport();
}

void clearBuffer()
{
    if(!g_in_frame)
    {
        C3D_FrameBegin(0);
        C2D_ViewReset();

        C2D_TargetClear(s_top_screen, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
        C2D_SceneBegin(s_top_screen);
        renderRect(0, 0, 400, 240, 0.0f, 0.0f, 0.0f, 1.0f, true);

        C2D_TargetClear(s_right_screen, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
        C2D_SceneBegin(s_right_screen);
        renderRect(0, 0, 400, 240, 0.0f, 0.0f, 0.0f, 1.0f, true);

        C2D_TargetClear(s_bottom_screen, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
        C2D_SceneBegin(s_bottom_screen);
        renderRect(0, 0, 320, 240, 0.0f, 0.0f, 0.0f, 1.0f, true);

        C3D_FrameEnd(0);
    }
}

void repaint()
{
    if(!g_in_frame)
        return;

    constexpr int shift = 0;
    // constexpr int shift = MAX_3D_OFFSET / 2;
    constexpr double shift_i[] = {shift, shift * 0.4, 0, shift * -0.4};

    s_depth_slider = osGet3DSliderState();

    s_cur_target = nullptr;
    C2D_ViewReset();

    C2D_Flush();
    s_mergeBlend();

    // in this case, the level graphics have already been rescaled to the bottom screen
    if(g_screen_swapped && (LevelEditor || MagicHand) && editorScreen.active)
    {
        C2D_TargetClear(s_top_screen, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
        C2D_SceneBegin(s_top_screen);

        for(int layer = 0; layer < 4; layer++)
        {
            C2D_DrawImage_Custom(s_layer_ims[layer],
                                 g_screen_phys_x + 40, g_screen_phys_y, g_screen_phys_w, g_screen_phys_h,
                                 shift, 0, s_tex_show_w, s_tex_h,
                                 X_FLIP_NONE, 1.0f, 1.0f, 1.0f, 1.0f);

            if(s_single_layer_mode)
                break;
        }
    }
    else if(g_screen_swapped)
    {
        C2D_TargetClear(s_bottom_screen, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
        C2D_SceneBegin(s_bottom_screen);

        for(int layer = 0; layer < 4; layer++)
        {
            C2D_DrawImage_Custom(s_layer_ims[layer],
                                 g_screen_phys_x, g_screen_phys_y, g_screen_phys_w, g_screen_phys_h,
                                 shift, 0, s_tex_show_w, s_tex_h,
                                 X_FLIP_NONE, 1.0f, 1.0f, 1.0f, 1.0f);

            if(s_single_layer_mode)
                break;
        }
    }
    // normally in editor mode, just center the level graphics
    else if(LevelEditor && !editorScreen.active)
    {
        C2D_TargetClear(s_bottom_screen, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
        C2D_SceneBegin(s_bottom_screen);

        for(int layer = 0; layer < 4; layer++)
        {
            C2D_DrawImage_Custom(s_layer_ims[layer],
                                 g_screen_phys_x - 40, g_screen_phys_y, g_screen_phys_w, g_screen_phys_h,
                                 shift, 0, s_tex_show_w, s_tex_h,
                                 X_FLIP_NONE, 1.0f, 1.0f, 1.0f, 1.0f);

            if(s_single_layer_mode)
                break;
        }
    }
    else if(s_depth_slider <= 0.05 || s_single_layer_mode)
    {
        C2D_TargetClear(s_top_screen, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
        C2D_SceneBegin(s_top_screen);

        for(int layer = 0; layer < 4; layer++)
        {
            C2D_DrawImage_Custom(s_layer_ims[layer],
                                 g_screen_phys_x, g_screen_phys_y, g_screen_phys_w, g_screen_phys_h,
                                 shift, 0, s_tex_show_w, s_tex_h,
                                 X_FLIP_NONE, 1.0f, 1.0f, 1.0f, 1.0f);

            if(s_single_layer_mode)
                break;
        }
    }
    else
    {
        C2D_TargetClear(s_top_screen, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
        C2D_SceneBegin(s_top_screen);

        for(int layer = 0; layer < 4; layer++)
        {
            C2D_DrawImage_Custom(s_layer_ims[layer],
                                 g_screen_phys_x, g_screen_phys_y, g_screen_phys_w, g_screen_phys_h,
                                 shift + (int)(shift_i[layer] * s_depth_slider), 0, s_tex_show_w, s_tex_h,
                                 X_FLIP_NONE, 1.0f, 1.0f, 1.0f, 1.0f);
        }

        C2D_TargetClear(s_right_screen, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
        C2D_SceneBegin(s_right_screen);

        for(int layer = 0; layer < 4; layer++)
        {
            C2D_DrawImage_Custom(s_layer_ims[layer],
                                 g_screen_phys_x, g_screen_phys_y, g_screen_phys_w, g_screen_phys_h,
                                 shift - (int)(shift_i[layer] * s_depth_slider), 0, s_tex_show_w, s_tex_h,
                                 X_FLIP_NONE, 1.0f, 1.0f, 1.0f, 1.0f);
        }
    }

    s_resetBlend();

    s_current_frame ++;
    g_in_frame = false;

    // leave the draw context and wait for vblank...
    g_microStats.start_sleep();
    // if(g_videoSettings.renderMode == RENDER_ACCELERATED_VSYNC)
    //     C3D_FrameSync();

    g_microStats.start_task(MicroStats::Graphics);
    C3D_FrameEnd(0);
}

void mapToScreen(int x, int y, int* dx, int* dy)
{
    // lower screen to upper screen conversion
    if((LevelEditor || MagicHand) && editorScreen.active)
    {
        *dx = x * 2;
        *dy = y * 2;
        return;
    }

    if(!g_screen_swapped)
        x += 40;

    *dx = (x - g_screen_phys_x) * ScreenW / g_screen_phys_w;
    *dy = (y - g_screen_phys_y) * ScreenH / g_screen_phys_h;
}

void mapFromScreen(int scr_x, int scr_y, int* window_x, int* window_y)
{
    // lower screen to upper screen conversion
    if((LevelEditor || MagicHand) && editorScreen.active)
    {
        *window_x = scr_x / 2;
        *window_y = scr_y / 2;
        return;
    }

    *window_x = (scr_x * g_screen_phys_w / ScreenW) + g_screen_phys_x;
    *window_y = (scr_y * g_screen_phys_h / ScreenH) + g_screen_phys_y;

    if(!g_screen_swapped)
        *window_x -= 40;
}

void minport_TransformPhysCoords() {}

void minport_ApplyPhysCoords()
{
    int tex_w = ScreenW / 2;
    int tex_h = ScreenH / 2;

    if(tex_w != s_tex_w || tex_h != s_tex_h || g_screen_swapped != should_swap_screen())
        s_createSceneTargets();

    g_screen_swapped = should_swap_screen();

    GPU_TEXTURE_FILTER_PARAM filter = (g_videoSettings.scaleMode == SCALE_DYNAMIC_LINEAR || g_videoSettings.scaleMode == SCALE_FIXED_05X)
        ? GPU_LINEAR
        : GPU_NEAREST;

    for(int layer = 0; layer < 4; layer++)
    {
        C3D_TexSetFilter(&s_layer_texs[layer], filter, filter);

        if(s_single_layer_mode)
            break;
    }
}

void minport_ApplyViewport()
{
    if(!g_in_frame)
        return;

    C2D_Flush();

    C2D_ViewReset();

    if(!g_viewport_offset_ignore)
        C2D_ViewTranslate(g_viewport_offset_x, g_viewport_offset_y);

    if(s_cur_target)
    {
        int hw_viewport_w, hw_viewport_h;
        int hw_viewport_x, hw_viewport_y;

        // rotated 90deg clockwise
        if(s_cur_target->linked)
        {
            hw_viewport_x = SDL_min(SDL_max(g_viewport_y, 0), s_cur_target->frameBuf.width - 1);
            hw_viewport_y = SDL_min(SDL_max(g_viewport_x, 0), s_cur_target->frameBuf.height - 1);
            hw_viewport_w = SDL_min(s_cur_target->frameBuf.width - hw_viewport_x, g_viewport_h);
            hw_viewport_h = SDL_min(s_cur_target->frameBuf.height - hw_viewport_y, g_viewport_w);
        }
        else
        {
            hw_viewport_x = SDL_min(SDL_max(g_viewport_x, 0), s_cur_target->frameBuf.width - 1);
            hw_viewport_y = SDL_min(SDL_max(g_viewport_y, 0), s_cur_target->frameBuf.height - 1);
            hw_viewport_w = SDL_min(s_cur_target->frameBuf.width - hw_viewport_x, g_viewport_w);
            hw_viewport_h = SDL_min(s_cur_target->frameBuf.height - hw_viewport_y, g_viewport_h);
        }

        if(s_cur_target->linked)
        {
            C3D_SetViewport(s_cur_target->frameBuf.width - hw_viewport_x - hw_viewport_w, hw_viewport_y, hw_viewport_w, hw_viewport_h);
            C2D_SceneSize(g_viewport_h, g_viewport_w, s_cur_target->linked);
        }
        else
        {
            C3D_SetViewport(hw_viewport_x, s_cur_target->frameBuf.height - hw_viewport_y - hw_viewport_h, hw_viewport_w, hw_viewport_h);
            C2D_SceneSize(g_viewport_w, g_viewport_h, s_cur_target->linked);
        }
    }
}

void lazyLoadPictureFromList(StdPicture_Sub& target, FILE* f, const std::string& dir)
{
    if(!GameIsActive)
        return; // do nothing when game is closed

    int length;

    char filename[256];

    if(fscanf(f, "%255[^\n]%n%*[^\n]\n", filename, &length) != 1)
    {
        pLogWarning("Could not load image path from load list");
        return;
    }

    if(length == 255)
    {
        pLogWarning("Image path %s was truncated in load list", filename);
        return;
    }

    target.inited = true;
    target.l.path = dir;
    target.l.path += filename;
    target.l.lazyLoaded = true;

    int w, h;

    if((fscanf(f, "%d\n%d\n", &w, &h) != 2) || (w < 0) || (w > 8192) || (h < 0) || (h > 8192))
    {
        pLogWarning("Could not load image %s dimensions from load list", filename);
        target.inited = false;
        return;
    }

    // pLogDebug("Successfully loaded %s (%d %d)", target.l.path.c_str(), w, h);

    target.w = w;
    target.h = h;

    return;
}


void lazyLoadPicture(StdPicture_Sub& target, const std::string& path, int scaleFactor, const std::string& maskPath, const std::string& maskFallbackPath)
{
    if(!GameIsActive)
        return; // do nothing when game is closed

    target.inited = false;
    target.l.path = path;

    if(target.l.path.empty())
        return;

    if(maskPath.empty() && !maskFallbackPath.empty() && Files::fileExists(maskFallbackPath))
        target.l.mask_path = maskFallbackPath;
    else
        target.l.mask_path = maskPath;

    target.inited = true;

    target.l.lazyLoaded = true;

    if(Files::hasSuffix(target.l.path, ".t3x"))
    {
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
        else
        {
            pLogWarning("lazyLoadPicture: Couldn't open size file. Giving up.");
            target.inited = false;
            return;
        }
    }
    else
    {
        PGE_Size tSize;

        if(!GraphicsHelps::getImageMetrics(path, &tSize))
        {
            pLogWarning("Error loading of image file:\n"
                        "%s\n"
                        "Reason: %s.",
                        path.c_str(),
                        (Files::fileExists(path) ? "wrong image format" : "file not exist"));

            target.inited = false;
            target.l.path.clear();
            target.l.mask_path.clear();
        }
        else
        {
            target.w = tSize.w() * scaleFactor;
            target.h = tSize.h() * scaleFactor;
        }
    }
}

static C2D_SpriteSheet s_tryHardToLoadC2D_SpriteSheet(const char* path)
{
    C2D_SpriteSheet sourceImage = C2D_SpriteSheetLoad(path);

    if(!sourceImage)
    {
        if(linearSpaceFree() < 15000000)
            minport_freeTextureMemory();

        sourceImage = C2D_SpriteSheetLoad(path);
    }

    return sourceImage;
}

void lazyLoad(StdPicture& target)
{
    if(!target.inited || !target.l.lazyLoaded || target.d.hasTexture())
        return;

    if(!Files::hasSuffix(target.l.path, ".t3x"))
    {
        FIBITMAP* FI_tex = robust_FILoad(target.l.path, target.w);

        if(!FI_tex)
        {
            pLogWarning("Permanently failed to load %s during image load, %lu free", target.l.path.c_str(), linearSpaceFree());
            pLogWarning("Error: %d (%s)", errno, strerror(errno));
            target.inited = false;
            return;
        }

        FIBITMAP* FI_mask = nullptr;
        bool force_merge = false;

        if(Files::hasSuffix(target.l.mask_path, "m.gif"))
        {
            FI_mask = robust_FILoad(target.l.mask_path, target.w);
        }
        else if(!target.l.mask_path.empty())
        {
            FIBITMAP* FI_mask_rgba = robust_FILoad(target.l.mask_path, target.w);

            if(FI_mask_rgba)
            {
                GraphicsHelps::getMaskFromRGBA(FI_mask_rgba, FI_mask);
                GraphicsHelps::closeImage(FI_mask_rgba);
            }

            // marginally faster, but inaccurate
            force_merge = true;
        }

        if(FI_mask && (force_merge || g_ForceBitmaskMerge || !GraphicsHelps::validateBitmaskRequired(FI_tex, FI_mask, target.l.path)))
        {
            GraphicsHelps::mergeWithMask(FI_tex, FI_mask);
            GraphicsHelps::closeImage(FI_mask);
            FI_mask = nullptr;
        }

        if(target.l.colorKey) // Apply transparent color for key pixels
        {
            PGE_Pix colSrc = {target.l.keyRgb[0],
                              target.l.keyRgb[1],
                              target.l.keyRgb[2], 0xFF
                             };
            PGE_Pix colDst = {target.l.keyRgb[0],
                              target.l.keyRgb[1],
                              target.l.keyRgb[2], 0x00
                             };
            GraphicsHelps::replaceColor(FI_tex, colSrc, colDst);
        }

        s_loadTexture(target, FreeImage_GetBits(FI_tex), FreeImage_GetWidth(FI_tex), FreeImage_GetHeight(FI_tex), FreeImage_GetPitch(FI_tex), false);
        FreeImage_Unload(FI_tex);

        if(FI_mask)
        {
            s_loadTexture(target, FreeImage_GetBits(FI_mask), FreeImage_GetWidth(FI_mask), FreeImage_GetHeight(FI_mask), FreeImage_GetPitch(FI_mask), true);
            FreeImage_Unload(FI_mask);
        }

        if(!target.d.hasTexture())
        {
            pLogWarning("Permanently failed to load %s during texture load (upload to GPU failed), %lu free", target.l.path.c_str(), linearSpaceFree());
            pLogWarning("Error: %d (%s)", errno, strerror(errno));
            target.inited = false;
            return;
        }
    }
    else
    {
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
            sourceImage = s_tryHardToLoadC2D_SpriteSheet(suppPath.c_str());

            if(!sourceImage)
            {
                pLogWarning("Permanently failed to load %s, %lu free", suppPath.c_str(), linearSpaceFree());
                pLogWarning("Error: %d (%s)", errno, strerror(errno));
            }
            else
                s_loadTexture3(target, sourceImage);
        }
    }

    s_num_textures_loaded++;

    if(linearSpaceFree() < 4194304)
    {
        pLogDebug("Triggering free texture memory due to low memory (%u bytes)", (unsigned)linearSpaceFree());
        minport_freeTextureMemory();
    }
}

void lazyPreLoad(StdPicture& target)
{
    lazyLoad(target);
}

void loadTexture(StdPicture& target, uint32_t width, uint32_t height, uint8_t *RGBApixels, uint32_t pitch)
{
    s_loadTexture(target, RGBApixels, width, height, pitch, false);
    target.inited = true;
    target.l.lazyLoaded = false;
}

void unloadTexture(StdPicture& tx)
{
    minport_unlinkTexture(&tx);

    if(tx.d.hasTexture())
        s_num_textures_loaded --;

    for(int i = 0; i < 6; i++)
    {
        if(tx.d.texture[i] == HEAP_MANAGED_TEXTURE)
        {
            C3D_TexDelete(tx.d.image[i].tex);
            delete tx.d.image[i].tex;
            delete tx.d.image[i].subtex;
        }
        else if(tx.d.texture[i])
        {
            C2D_SpriteSheetFree(tx.d.texture[i]);
        }

        tx.d.image[i] = C2D_Image();
        tx.d.texture[i] = nullptr;
    }

    if(!tx.l.canLoad())
        static_cast<StdPicture_Sub&>(tx) = StdPicture_Sub();
}

void minport_RenderBoxFilled(int x1, int y1, int x2, int y2, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
{
    uint32_t clr = C2D_Color32(red, green, blue, alpha);

    C2D_DrawRectSolid(x1, y1, 0, x2 - x1, y2 - y1, clr);
}

void minport_RenderTexturePrivate(int16_t xDst, int16_t yDst, int16_t wDst, int16_t hDst,
                                  StdPicture& tx,
                                  int16_t xSrc, int16_t ySrc, int16_t wSrc, int16_t hSrc,
                                  float rotateAngle, FPoint_t* center, unsigned int flip,
                                  float red, float green, float blue, float alpha)
{
    if(!tx.inited)
        return;

    if(!tx.d.hasTexture() && tx.l.lazyLoaded)
        lazyLoad(tx);

    if(!tx.d.hasTexture())
        return;

    // don't exceed 90%, ever
    if(gpuCmdBufSize > 0 && gpuCmdBufOffset * 10 > gpuCmdBufSize * 9)
        return;

    // automatic flipping based on SMBX style!
    unsigned int mode = 0;

    while(ySrc >= tx.h / 2 && mode < 3)
    {
        ySrc -= tx.h / 2;
        mode += 1;
    }

    flip ^= mode;

    C3D_Mtx prev_view;

    // handle rotation NOW
    if(rotateAngle)
    {
        int16_t cx, cy;

        if(center)
        {
            cx = center->x / 2.0f + 0.5f;
            cy = center->y / 2.0f + 0.5f;
        }
        else
        {
            cx = wDst / 2;
            cy = hDst / 2;
        }

        C2D_ViewSave(&prev_view);

        C2D_ViewTranslate(xDst + cx, yDst + cy);
        C2D_ViewRotateDegrees(rotateAngle);

        xDst = -cx;
        yDst = -cy;
    }

    // texture boundaries
    // this never happens unless there was an invalid input
    // if((xSrc < 0.0f) || (ySrc < 0.0f)) return;

    // TODO: graphics tests for how offscreen draws interact with flips
    //       handling rotations properly is probably impossible

    C2D_Image* to_draw = nullptr;
    C2D_Image* to_draw_2 = nullptr;

    C2D_Image* to_mask = nullptr;
    C2D_Image* to_mask_2 = nullptr;

    // Don't go more than size of texture
    // Failure conditions should only happen if texture is smaller than expected

    if(ySrc + hSrc > 1024)
    {
        if(ySrc + hSrc > 2048)
        {
            if(tx.d.texture[2])
            {
                to_draw = &tx.d.image[2];
                if(tx.d.texture[5])
                    to_mask = &tx.d.image[5];
            }

            if(ySrc < 2048 && tx.d.texture[1])
            {
                to_draw_2 = &tx.d.image[1];
                if(tx.d.texture[4])
                    to_mask_2 = &tx.d.image[4];
            }

            ySrc -= 1024;
        }
        else
        {
            if(tx.d.texture[1])
            {
                to_draw = &tx.d.image[1];
                if(tx.d.texture[4])
                    to_mask = &tx.d.image[4];
            }

            if(ySrc < 1024)
            {
                to_draw_2 = &tx.d.image[0];
                if(tx.d.texture[3])
                    to_mask_2 = &tx.d.image[3];
            }
        }

        // draw the top pic
        if(to_draw_2 != nullptr)
        {
            if(to_mask_2)
            {
                C2D_Flush();
                C3D_ColorLogicOp(GPU_LOGICOP_AND);
                C2D_DrawImage_Custom(*to_mask_2, xDst, yDst, wDst, (1024 - ySrc) * hDst / hSrc,
                                     xSrc, ySrc, wSrc, 1024 - ySrc, flip, red, green, blue, alpha);
                C2D_Flush();
                C3D_ColorLogicOp(GPU_LOGICOP_OR);
            }

            C2D_DrawImage_Custom(*to_draw_2, xDst, yDst, wDst, (1024 - ySrc) * hDst / hSrc,
                                 xSrc, ySrc, wSrc, 1024 - ySrc, flip, red, green, blue, alpha);

            if(to_mask_2)
            {
                C2D_Flush();
                s_resetBlend();
            }

            yDst += (1024 - ySrc) * hDst / hSrc;
            hDst -= (1024 - ySrc) * hDst / hSrc;
            hSrc -= (1024 - ySrc);
            ySrc = 0.0f;
        }
        else
            ySrc -= 1024.0f;
    }
    else
    {
        to_draw = &tx.d.image[0];
        if(tx.d.texture[3])
            to_mask = &tx.d.image[3];
    }

    if(to_draw != nullptr)
    {
        if(to_mask)
        {
            C2D_Flush();
            C3D_ColorLogicOp(GPU_LOGICOP_AND);
            C2D_DrawImage_Custom(*to_mask, xDst, yDst, wDst, hDst,
                                 xSrc, ySrc, wSrc, hSrc, flip, red, green, blue, alpha);
            C2D_Flush();
            C3D_ColorLogicOp(GPU_LOGICOP_OR);
        }

        C2D_DrawImage_Custom(*to_draw, xDst, yDst, wDst, hDst,
                             xSrc, ySrc, wSrc, hSrc, flip, red, green, blue, alpha);

        if(to_mask)
        {
            C2D_Flush();
            s_resetBlend();
        }
    }

    // Finalize rotation HERE
    if(rotateAngle)
        C2D_ViewRestore(&prev_view);
}

} // namespace XRender
