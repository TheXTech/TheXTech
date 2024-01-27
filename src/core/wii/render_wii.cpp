﻿/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2024 Vitaly Novichkov <admin@wohlnet.ru>
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


#include <malloc.h>
#include <set>

// sigh...
#define BOOL FI_BOOL
#include <FreeImageLite.h>
#undef BOOL

#include <gccore.h>

#include <Graphics/graphics_funcs.h>
#include <Logger/logger.h>
#include <Utils/files.h>

#include "globals.h"
#include "video.h"
#include "frame_timer.h"

#include "main/cheat_code.h"

#include "core/window.h"

#include "core/render.h"
#include "core/minport/render_minport_shared.h"

#include "core/render_planes.h"


namespace XRender
{

bool g_in_frame = false;

static RenderPlanes_t s_render_planes;

#define DEFAULT_FIFO_SIZE   (256*1024)

void* frameBuffer[2] = { NULL, NULL};
int cur_buffer = 0;
GXRModeObj* rmode = nullptr;
Mtx view;

int s_num_textures_loaded = 0;

int g_rmode_w = 640;
int g_rmode_h = 480;

/**
 * Convert a raw BMP (ARGB) to 4x4RGBA.
 * @author DragonMinded, modifications by ds-sloth
*/
static void* s_RawTo4x4RGBA(const uint8_t* src, uint32_t width, uint32_t height, uint32_t pitch, uint32_t* wdst_out, uint32_t* hdst_out, bool downscale = true)
{
    // calculate destination dimensions, including downscaling and required padding
    int sf = (downscale ? 2 : 1);
    uint32_t wdst = (width + sf - 1) / sf;
    uint32_t hdst = (height + sf - 1) / sf;

    if(wdst & 3)
    {
        wdst += 4;
        wdst &= ~3;
    }

    if(hdst & 3)
    {
        hdst += 4;
        hdst &= ~3;
    }

    void* dst = memalign(32, wdst * hdst * 4);

    if(!dst)
    {
        pLogCritical("Memory allocation failed when converting texture to Wii hardware format");
        return dst;
    }

    u8* p = (u8*)dst;

    for(u32 block = 0; block < hdst; block += 4)
    {
        for(u32 i = 0; i < wdst; i += 4)
        {
            // Alpha and Red
            for(u8 c = 0; c < 4; ++c)
            {
                for(u8 argb = 0; argb < 4; ++argb)
                {
                    // new: padding
                    if((i + argb) * sf > width || (block + c) * sf > height)
                    {
                        *p++ = 0;
                        *p++ = 255;
                        continue;
                    }

                    // New: Alpha pixels
                    *p++ = src[(((i + argb) * sf * 4) + ((block + c) * sf * pitch)) + 3];
                    // Red pixels
                    *p++ = src[(((i + argb) * sf * 4) + ((block + c) * sf * pitch)) + 0];
                }
            }

            // Green and Blue
            for(u8 c = 0; c < 4; ++c)
            {
                for(u8 argb = 0; argb < 4; ++argb)
                {
                    // new: padding
                    if((i + argb) * sf > width || (block + c) * sf > height)
                    {
                        *p++ = 255;
                        *p++ = 255;
                        continue;
                    }

                    // Green pixels
                    *p++ = src[((((i + argb) * sf * 4) + ((block + c) * sf * pitch))) + 1];
                    // Blue pixels
                    *p++ = src[((((i + argb) * sf * 4) + ((block + c) * sf * pitch))) + 2];
                }
            }
        }
    }

    if(dst && wdst_out)
        *wdst_out = wdst;

    if(dst && hdst_out)
        *hdst_out = hdst;

    return dst;
}

int robust_TPL_GetTexture(TPLFile* file, int i, GXTexObj* gxtex)
{
    int ret = TPL_GetTexture(file, i, gxtex);

    if(ret == 0 || errno != ENOMEM)
        return ret;

    pLogWarning("Failed to load due to lack of memory");
    minport_freeTextureMemory();

    return TPL_GetTexture(file, i, gxtex);
}

FIBITMAP* robust_FILoad(const std::string& path, const std::string& maskPath, int* orig_w = nullptr, int* orig_h = nullptr)
{
    if(path.empty())
        return nullptr;

    // this is wasteful, but it lets us diagnose memory issue vs other issues
    FREE_IMAGE_FORMAT formato = FreeImage_GetFileType(path.c_str(), 0);

    if(formato == FIF_UNKNOWN)
    {
        return nullptr;
    }

    FIBITMAP* sourceImage = GraphicsHelps::loadImage(path);

    if(!sourceImage)
    {
        pLogWarning("FreeImageLite failed to load image due to lack of memory, trying to free some memory");
        minport_freeTextureMemory();
        sourceImage = GraphicsHelps::loadImage(path);

        if(!sourceImage)
            return nullptr;
    }

    if(!maskPath.empty())
        GraphicsHelps::mergeWithMask(sourceImage, "", maskPath);

    uint32_t w = static_cast<uint32_t>(FreeImage_GetWidth(sourceImage));
    uint32_t h = static_cast<uint32_t>(FreeImage_GetHeight(sourceImage));

    pLogDebug("loading %s, freeimage reports %u %u %u", path.c_str(), w, h, FreeImage_GetPitch(sourceImage));

    if(orig_w)
        *orig_w = w;

    if(orig_h)
        *orig_h = h;

    if((w == 0) || (h == 0))
    {
        GraphicsHelps::closeImage(sourceImage);
        pLogWarning("Error loading of image file:\n"
                    "Reason: %s."
                    "Zero image size!");
        return nullptr;
    }

    FreeImage_FlipVertical(sourceImage);

    return sourceImage;
}

void s_loadTexture(StdPicture& target, void* data, int width, int height, int pitch, bool mask)
{
    // downscale if logical width matches the actual width of the texture, otherwise don't
    bool downscale = (width >= target.w);

    int max_size = (downscale ? 2048 : 1024);

    if(width > max_size && height <= max_size)
        target.d.multi_horizontal = true;

    for(int i = 0; i < 3; i++)
    {
        int start_x, start_y;

        if(target.d.multi_horizontal)
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
            uint32_t wdst = 0, hdst = 0;
            target.d.backing_texture[i + 3 * mask] = s_RawTo4x4RGBA((uint8_t*)data + start_y * pitch + start_x * 4, w_i, h_i, pitch, &wdst, &hdst, downscale);

            if(target.d.backing_texture[i + 3 * mask])
            {
                DCFlushRange(target.d.backing_texture[i + 3 * mask], wdst * hdst * 4);
                GX_InitTexObj(&target.d.texture[i + 3 * mask], target.d.backing_texture[i + 3 * mask],
                              wdst, hdst, GX_TF_RGBA8, GX_CLAMP, GX_CLAMP, GX_FALSE);
                GX_InitTexObjFilterMode(&target.d.texture[i + 3 * mask], GX_NEAR, GX_NEAR);

                // printf("We initialized with %u %u\n", wdst, hdst);
                target.d.texture_init[i + 3 * mask] = true;
            }
            else
                break;
        }
    }
}

void s_loadTexture(StdPicture& target, int i)
{
    if(target.d.texture_file_init[i])
    {
        u32 info;

        if(!target.w)
        {
            uint16_t tex_w, tex_h;
            TPL_GetTextureInfo(&target.d.texture_file[i], 0, &info, &tex_w, &tex_h);
            target.w = tex_w * 2;
            target.h = tex_h * 2;
        }

        if(robust_TPL_GetTexture(&target.d.texture_file[i], 0, &target.d.texture[i]) != 0)
        {
            TPL_CloseTPLFile(&target.d.texture_file[i]);
            target.d.texture_file_init[i] = false;
        }
        else
        {
            GX_InitTexObjFilterMode(&target.d.texture[i], GX_NEAR, GX_NEAR);
            GX_InitTexObjWrapMode(&target.d.texture[i], GX_CLAMP, GX_CLAMP);
            target.d.texture_init[i] = true;
        }
    }
}

void gxDrawDoneCB()
{
    cur_buffer ^= 1;
}

void videoPreTraceCB(u32 /*retraceCnt*/)
{
    VIDEO_SetNextFramebuffer(frameBuffer[cur_buffer]);
    VIDEO_Flush();
}

bool init()
{
    f32 yscale;
    u32 xfbHeight;

    GXColor background = {0, 0, 0, 0xff};

    // init the vi.
    VIDEO_Init();

    rmode = VIDEO_GetPreferredMode(NULL);

    // allocate 2 framebuffers for double buffering
    frameBuffer[0] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
    frameBuffer[1] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));

    VIDEO_Configure(rmode);
    VIDEO_SetNextFramebuffer(frameBuffer[cur_buffer]);
    VIDEO_SetBlack(FALSE);
    VIDEO_Flush();
    VIDEO_WaitVSync();

    if(rmode->viTVMode & VI_NON_INTERLACE) VIDEO_WaitVSync();

    VIDEO_SetPreRetraceCallback(videoPreTraceCB);

    // setup the fifo and then init the flipper
    void* gp_fifo = NULL;
    gp_fifo = memalign(32, DEFAULT_FIFO_SIZE);
    memset(gp_fifo, 0, DEFAULT_FIFO_SIZE);

    GX_Init(gp_fifo, DEFAULT_FIFO_SIZE);

    // clears the bg to color and clears the z buffer
    GX_SetCopyClear(background, GX_MAX_Z24);
    GX_SetDrawDoneCallback(gxDrawDoneCB);

    // other gx setup
    GX_SetViewport(0, 0, rmode->fbWidth, rmode->efbHeight, 0, 1);
    yscale = GX_GetYScaleFactor(rmode->efbHeight, rmode->xfbHeight);
    xfbHeight = GX_SetDispCopyYScale(yscale);
    GX_SetScissor(0, 0, rmode->fbWidth, rmode->efbHeight);
    GX_SetDispCopySrc(0, 0, rmode->fbWidth, rmode->efbHeight);
    GX_SetDispCopyDst(rmode->fbWidth, xfbHeight);
    GX_SetCopyFilter(rmode->aa, rmode->sample_pattern, GX_TRUE, rmode->vfilter);
    GX_SetFieldMode(rmode->field_rendering, ((rmode->viHeight == 2 * rmode->xfbHeight) ? GX_ENABLE : GX_DISABLE));
    GX_SetColorUpdate(1);
    GX_SetAlphaUpdate(1);
    GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET);


    printf("Initialized video with rMode %d x %d\n", (int)rmode->viWidth, (int)rmode->viHeight);

    g_rmode_w = rmode->fbWidth;
    g_rmode_h = rmode->efbHeight;

    GX_SetCullMode(GX_CULL_NONE);
    GX_CopyDisp(frameBuffer[cur_buffer], GX_TRUE);
    GX_SetDispCopyGamma(GX_GM_1_0);


    // setup the vertex descriptor
    // tells the flipper to expect direct data
    GX_ClearVtxDesc();
    GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
    GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);

    // setup the vertex attribute table
    // describes the data
    // args: vat location 0-7, type of data, data format, size, scale
    // so for ex. in the first call we are sending position data with
    // 3 values X,Y,Z of size F32. scale sets the number of fractional
    // bits for non float data.
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_S16, 0);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_U16, 0);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);

    GX_SetNumChans(1);
    GX_SetNumTexGens(1);

    GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
    GX_SetTexCoordScaleManually(GX_TEXCOORD0, GX_TRUE, 1, 1);

    GX_InvVtxCache();
    GX_InvalidateTexAll();

    // setup our camera at the origin
    // looking down the -z axis with y up
    guVector cam = {0.0F, 0.0F, 0.0F},
             up = {0.0F, 1.0F, 0.0F},
             look = {0.0F, 0.0F, -1.0F};
    guLookAt(view, &cam, &up, &look);

    updateViewport();

    return true;
}

void quit()
{
}

void setTargetTexture()
{
    if(g_in_frame)
        return;

    // do this before drawing
    GX_SetViewport(g_screen_phys_x, g_screen_phys_y, g_screen_phys_w, g_screen_phys_h, 0, 1);
    // load the view matrix into matrix memory
    GX_LoadPosMtxImm(view, GX_PNMTX0);

    g_in_frame = true;

    struct mallinfo info = mallinfo();

    if(info.uordblks > 292000000)
    {
        pLogWarning("Memory low, triggering free texture memory");
        minport_freeTextureMemory();
    }

    minport_initFrame();
}

void setTargetScreen()
{
}

void setDrawPlane(uint8_t plane)
{
    s_render_planes.set_plane(plane);
}

void clearBuffer()
{
    if(!g_in_frame)
    {
        setTargetTexture();
        resetViewport();
        renderRect(0, 0, XRender::TargetW, XRender::TargetH, {0, 0, 0});
        repaint();
    }
}

void repaint()
{
    if(!g_in_frame)
        return;

    // do this stuff after drawing
    int next_buffer = cur_buffer ^ 1;
    GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
    GX_SetZCompLoc(GX_FALSE);
    GX_SetAlphaCompare(GX_GEQUAL, 0x08, GX_AOP_AND, GX_ALWAYS, 0);
    GX_SetColorUpdate(GX_TRUE);
    GX_CopyDisp(frameBuffer[next_buffer], GX_TRUE);
    GX_DrawDone();

    GX_Flush();

    s_render_planes.reset();

    g_microStats.start_sleep();

    // video settings: VSync
#if 0

    if(g_videoSettings.scaleMode == SCALE_DYNAMIC_LINEAR)
    {
        VIDEO_WaitVSync();

        if(rmode->viTVMode & VI_NON_INTERLACE)
            VIDEO_WaitVSync();
    }

#endif
    g_microStats.start_task(MicroStats::Graphics);

    g_in_frame = false;
}

void mapToScreen(int x, int y, int* dx, int* dy)
{
    *dx = (x - g_screen_phys_x) * XRender::TargetW / g_screen_phys_w;
    *dy = (y - g_screen_phys_y) * XRender::TargetH / g_screen_phys_h;
}

void mapFromScreen(int scr_x, int scr_y, int* window_x, int* window_y)
{
    *window_x = (scr_x * g_screen_phys_w / XRender::TargetW) + g_screen_phys_x;
    *window_y = (scr_y * g_screen_phys_h / XRender::TargetH) + g_screen_phys_y;
}

void minport_TransformPhysCoords()
{
    int hardware_w, hardware_h;
    XWindow::getWindowSize(&hardware_w, &hardware_h);

    hardware_w /= 2;
    hardware_h /= 2;

    if(CONF_GetAspectRatio()) // widescreen_stretch
    {
        g_screen_phys_h = g_screen_phys_h * g_rmode_h / hardware_h;

        if(g_screen_phys_h > g_rmode_h)
        {
            g_screen_phys_w = g_screen_phys_w * g_rmode_h / g_screen_phys_h;
            g_screen_phys_h = g_rmode_h;
        }

        if(g_screen_phys_w > g_rmode_w)
        {
            g_screen_phys_h = g_screen_phys_h * g_rmode_w / g_screen_phys_w;
            g_screen_phys_w = g_rmode_w;
        }

        pLogDebug("Phys screen stretched to %d x %d", g_screen_phys_w, g_screen_phys_h);
    }

    g_screen_phys_x = g_rmode_w / 2 - g_screen_phys_w / 2;
    g_screen_phys_y = g_rmode_h / 2 - g_screen_phys_h / 2;

    if(g_screen_phys_x < 0)
        g_screen_phys_x = 0;

    if(g_screen_phys_y < 0)
        g_screen_phys_y = 0;
}

void minport_ApplyPhysCoords()
{
    GXColor background = {0, 0, 0, 0xff};
    GX_SetCopyClear(background, GX_MAX_Z24);

    // setup our projection matrix
    GX_SetViewport(g_screen_phys_x, g_screen_phys_y, g_screen_phys_w, g_screen_phys_h, 0, 1);
    GX_SetScissor(g_screen_phys_x, g_screen_phys_y, g_screen_phys_w, g_screen_phys_h);

    Mtx44 perspective;
    guOrtho(perspective, 0.0f, XRender::TargetH / 2, 0.0f, XRender::TargetW / 2, -(1 << 15), (1 << 15));
    GX_LoadProjectionMtx(perspective, GX_ORTHOGRAPHIC);
}

void minport_ApplyViewport()
{
    int phys_offset_x = g_viewport_x * g_screen_phys_w * 2 / XRender::TargetW;
    int phys_width = g_viewport_w * g_screen_phys_w * 2 / XRender::TargetW;

    int phys_offset_y = g_viewport_y * g_screen_phys_h * 2 / XRender::TargetH;
    int phys_height = g_viewport_h * g_screen_phys_h * 2 / XRender::TargetH;

    if(g_screen_phys_x + phys_offset_x < 0)
        phys_offset_x = -g_screen_phys_x;

    if(g_screen_phys_y + phys_offset_y < 0)
        phys_offset_y = -g_screen_phys_y;

    GX_SetViewport(g_screen_phys_x + phys_offset_x, g_screen_phys_y + phys_offset_y, phys_width, phys_height, 0, 1);
    GX_SetScissor(g_screen_phys_x + phys_offset_x, g_screen_phys_y + phys_offset_y, phys_width, phys_height);

    Mtx44 perspective;

    if(g_viewport_offset_ignore)
        guOrtho(perspective, 0, g_viewport_h, 0, g_viewport_w, -(1 << 15), (1 << 15));
    else
        guOrtho(perspective,
                g_viewport_offset_y, g_viewport_h + g_viewport_offset_y,
                g_viewport_offset_x, g_viewport_w + g_viewport_offset_x,
                -(1 << 15), (1 << 15));

    GX_LoadProjectionMtx(perspective, GX_ORTHOGRAPHIC);

    // int ox = g_viewport_x + g_viewport_offset_x;
    // int oy = g_viewport_y + g_viewport_offset_y;

    // ox &= ~1;
    // oy &= ~1;

    // GX_SetScissorBoxOffset(-ox, -oy);
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

    if(fscanf(f, "%d\n%d\n", &w, &h) != 2 || w < 0 || w > 8192 || h < 0 || h > 8192)
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

        PGE_Size tSize;

        if(!GraphicsHelps::getImageMetrics(path, &tSize))
        {
            pLogWarning("Error loading of image file:\n"
                        "%s\n"
                        "Reason: %s.",
                        path.c_str(),
                        (Files::fileExists(path) ? "wrong image format" : "file not exist"));
        }
        else
        {
            target.w = tSize.w() * scaleFactor;
            target.h = tSize.h() * scaleFactor;
        }
    }
}

int robust_OpenTPLFromFile(TPLFile* target, const char* path)
{
    int ret = TPL_OpenTPLFromFile(target, path);

    if(ret == 1 || errno != ENOMEM)
        return ret;

    pLogWarning("Failed to load %s due to lack of memory", path);
    minport_freeTextureMemory();

    return TPL_OpenTPLFromFile(target, path);
}

void lazyLoad(StdPicture& target)
{
    if(!target.inited || !target.l.lazyLoaded || target.d.hasTexture())
        return;

    pLogDebug("Loading %s", target.l.path.c_str());

    std::string suppPath;

    target.inited = true;

    if(Files::hasSuffix(target.l.path, ".tpl"))
    {
        if(robust_OpenTPLFromFile(&target.d.texture_file[0], target.l.path.c_str()) != 1)
        {
            pLogWarning("Permanently failed to load %s", target.l.path.c_str());
            pLogWarning("Error: %d (%s)", errno, strerror(errno));
            target.inited = false;
            return;
        }

        target.d.texture_file_init[0] = true;
        s_loadTexture(target, 0);
    }
    else
    {
        FIBITMAP* FI_tex = robust_FILoad(target.l.path, "");

        if(!FI_tex)
        {
            pLogWarning("Permanently failed to load %s during image load", target.l.path.c_str());
            pLogWarning("Error: %d (%s)", errno, strerror(errno));
            target.inited = false;
            return;
        }

        FIBITMAP* FI_mask = nullptr;
        bool force_merge = false;

        if(Files::hasSuffix(target.l.mask_path, "m.gif"))
        {
            FI_mask = robust_FILoad(target.l.mask_path, "");
        }
        else if(!target.l.mask_path.empty())
        {
            FIBITMAP* FI_mask_rgba = robust_FILoad(target.l.mask_path, "");

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
    }

    if(!target.d.hasTexture())
    {
        pLogWarning("Permanently failed to load %s", target.l.path.c_str());
        pLogWarning("Error: %d (%s)", errno, strerror(errno));
        target.d.destroy();
        target.inited = false;
        return;
    }

    if(target.h > 2048 && target.d.texture_file_init[0])
    {
        suppPath = target.l.path + '1';

        if(robust_OpenTPLFromFile(&target.d.texture_file[1], suppPath.c_str()) != 1)
        {
            pLogWarning("Permanently failed to load %s", suppPath.c_str());
            pLogWarning("Error: %d (%s)", errno, strerror(errno));
        }
        else
        {
            target.d.texture_file_init[1] = true;
            s_loadTexture(target, 1);
        }
    }

    if(target.h > 4096 && target.d.texture_file_init[0])
    {
        suppPath = target.l.path + '2';

        if(robust_OpenTPLFromFile(&target.d.texture_file[2], suppPath.c_str()) != 1)
        {
            pLogWarning("Permanently failed to load %s", suppPath.c_str());
            pLogWarning("Error: %d (%s)", errno, strerror(errno));
        }
        else
        {
            target.d.texture_file_init[2] = true;
            s_loadTexture(target, 2);
        }
    }

    pLogDebug("Done!");

    s_num_textures_loaded++;
}

void lazyPreLoad(StdPicture& target)
{
    lazyLoad(target);
}

void loadTexture(StdPicture &target, uint32_t width, uint32_t height, uint8_t *RGBApixels, uint32_t pitch)
{
    s_loadTexture(target, RGBApixels, width, height, pitch, false);
    target.inited = true;
    target.l.lazyLoaded = false;
}

void unloadTexture(StdPicture& tx)
{
    minport_unlinkTexture(&tx);

    if(tx.d.hasTexture())
        s_num_textures_loaded--;

    tx.d.destroy();

    if(!tx.l.canLoad())
        static_cast<StdPicture_Sub&>(tx) = StdPicture_Sub();
}

inline int ROUNDDIV2(int x)
{
    return (x < 0) ? (x - 1) / 2 : x / 2;
}

inline float ROUNDDIV2(float x)
{
    return std::nearbyintf(std::roundf(x) / 2.0f);
}

inline float ROUNDDIV2(double x)
{
    return std::nearbyintf(std::roundf((float)x / 2.0f));
}

inline float FLOORDIV2(float x)
{
    return std::floor(x / 2.0f);
}

void wii_RenderBox(int x1, int y1, int x2, int y2, XTColor color, bool filled)
{
    uint8_t r = color.r;
    uint8_t g = color.g;
    uint8_t b = color.b;
    uint8_t a = color.a;

    GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORDNULL, GX_TEXMAP_NULL, GX_COLOR0);
    GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);

    int16_t z = s_render_planes.next();

    GX_Begin(filled ? GX_QUADS : GX_LINESTRIP, GX_VTXFMT0, filled ? 4 : 5);
    GX_Position3s16(x1, y1, z);
    GX_Color4u8(r, g, b, a);
    GX_TexCoord2u16(0, 0);

    GX_Position3s16(x2, y1, z);
    GX_Color4u8(r, g, b, a);
    GX_TexCoord2u16(0, 0);

    GX_Position3s16(x2, y2, z);
    GX_Color4u8(r, g, b, a);
    GX_TexCoord2u16(0, 0);

    GX_Position3s16(x1, y2, z);
    GX_Color4u8(r, g, b, a);
    GX_TexCoord2u16(0, 0);

    // complete the rect
    if(!filled)
    {
        GX_Position3s16(x1, y1, z);
        GX_Color4u8(r, g, b, a);
        GX_TexCoord2u16(0, 0);
    }

    GX_End();
}

void minport_RenderBoxFilled(int x1, int y1, int x2, int y2, XTColor color)
{
    wii_RenderBox(x1, y1, x2, y2, color, true);
}

void minport_RenderBoxUnfilled(int x1, int y1, int x2, int y2, XTColor color)
{
    wii_RenderBox(x1, y1, x2, y2, color, false);
}

inline bool GX_DrawImage_Custom(GXTexObj* img,
                                GXTexObj* mask,
                                int16_t x, int16_t y, uint16_t w, uint16_t h,
                                uint16_t src_x, uint16_t src_y, uint16_t src_w, uint16_t src_h,
                                unsigned int flip,
                                XTColor color)
{
    uint8_t r = color.r;
    uint8_t g = color.g;
    uint8_t b = color.b;
    uint8_t a = color.a;

    int16_t z = s_render_planes.next();

    GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE);
    GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);

    for(int i = 0; i < 2; i++)
    {
        uint16_t u1 = src_x;
        uint16_t u2 = src_x + src_w;
        uint16_t v1 = src_y;
        uint16_t v2 = src_y + src_h;

        int16_t x1 = x;
        int16_t x2 = x + w;
        int16_t y1 = y;
        int16_t y2 = y + h;

        if(mask && i == 0)
        {
            GX_SetBlendMode(GX_BM_LOGIC, GX_BL_ONE, GX_BL_ONE, GX_LO_AND);
            GX_LoadTexObj(mask, GX_TEXMAP0);
            uint16_t mask_w = GX_GetTexObjWidth(mask);
            uint16_t mask_h = GX_GetTexObjHeight(mask);

            if(u1 > mask_w || v1 > mask_h)
                continue;

            if(u2 > mask_w)
            {
                u2 = mask_w;
                x2 = x + (u2 - u1) * w / src_w;
            }

            if(v2 > mask_h)
            {
                v2 = mask_h;
                y2 = y + (v2 - v1) * h / src_h;
            }
        }
        else if(mask)
        {
            GX_SetBlendMode(GX_BM_LOGIC, GX_BL_ONE, GX_BL_ONE, GX_LO_OR);
            GX_LoadTexObj(img, GX_TEXMAP0);
        }
        else
        {
            GX_LoadTexObj(img, GX_TEXMAP0);
        }

        if(flip & X_FLIP_HORIZONTAL)
            std::swap(u1, u2);

        if(flip & X_FLIP_VERTICAL)
            std::swap(v1, v2);

        GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
        GX_Position3s16(x1, y1, z);
        GX_Color4u8(r, g, b, a);
        GX_TexCoord2u16(u1, v1);

        GX_Position3s16(x2, y1, z);
        GX_Color4u8(r, g, b, a);
        GX_TexCoord2u16(u2, v1);

        GX_Position3s16(x2, y2, z);
        GX_Color4u8(r, g, b, a);
        GX_TexCoord2u16(u2, v2);

        GX_Position3s16(x1, y2, z);
        GX_Color4u8(r, g, b, a);
        GX_TexCoord2u16(u1, v2);
        GX_End();

        if(!mask)
            break;
    }

    if(mask)
        GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET);

    return true;
}

inline bool GX_DrawImage_Custom_Rotated(GXTexObj* img,
                                        GXTexObj* mask,
                                        float x, float y, float w, float h,
                                        float src_x, float src_y, float src_w, float src_h,
                                        unsigned int flip, FPoint_t* center, float angle,
                                        XTColor color)
{
    Mtx rotated;

    float cx, cy;

    if(center)
    {
        cx = center->x / 2.0f;
        cy = center->y / 2.0f;
    }
    else
    {
        cx = w / 2.0f;
        cy = h / 2.0f;
    }

    for(int r = 0; r < 3; r++)
    {
        for(int c = 0; c < 4; c++)
            rotated[r][c] = view[r][c];
    }

    guMtxRotDeg(rotated, 'z', angle);
    guMtxTransApply(rotated, rotated, x + cx, y + cy, 0.0f);

    GX_LoadPosMtxImm(rotated, GX_PNMTX0);

    GX_DrawImage_Custom(img,
                        mask,
                        -cx, -cy, w, h,
                        src_x, src_y, src_w, src_h,
                        flip,
                        color);

    GX_LoadPosMtxImm(view, GX_PNMTX0);

    return true;
}

void minport_RenderTexturePrivate(int16_t xDst, int16_t yDst, int16_t wDst, int16_t hDst,
                                  StdPicture& tx,
                                  int16_t xSrc, int16_t ySrc, int16_t wSrc, int16_t hSrc,
                                  float rotateAngle, FPoint_t* center, unsigned int flip,
                                  XTColor color)
{
    if(!tx.inited)
        return;

    if(!tx.d.hasTexture() && tx.l.lazyLoaded)
        lazyLoad(tx);

    if(!tx.d.hasTexture())
        return;

    // automatic flipping based on SMBX style!
    unsigned int mode = 0;

    while(ySrc >= tx.h / 2 && mode < 3)
    {
        ySrc -= tx.h / 2;
        mode += 1;
    }

    flip ^= mode;

    GXTexObj* to_draw = nullptr;
    GXTexObj* to_draw_2 = nullptr;

    GXTexObj* to_mask = nullptr;
    GXTexObj* to_mask_2 = nullptr;

    if(tx.d.multi_horizontal && xSrc + wSrc > 1024)
    {
        if(wSrc > 1024)
        {
            // reduce it to be on viewport
            if(xDst < 0)
            {
                xSrc -= (int)xDst * wSrc / wDst;
                wSrc += (int)xDst * wSrc / wDst;
                wDst += xDst;
                xDst = 0;
            }

            if(wSrc > 1024)
            {
                wDst = (int)wDst * 1024 / wSrc;
                wSrc = 1024;
            }
        }

        if(xSrc + wSrc > 2048)
        {
            if(tx.d.texture_init[2])
            {
                to_draw = &tx.d.texture[2];

                if(tx.d.texture_init[5])
                    to_mask = &tx.d.texture[5];
            }

            if(xSrc < 2048 && tx.d.texture_init[1])
            {
                to_draw_2 = &tx.d.texture[1];

                if(tx.d.texture_init[4])
                    to_mask_2 = &tx.d.texture[4];
            }

            xSrc -= 1024;
        }
        else
        {
            if(tx.d.texture_init[1])
            {
                to_draw = &tx.d.texture[1];

                if(tx.d.texture_init[4])
                    to_mask = &tx.d.texture[4];
            }

            if(xSrc < 1024)
            {
                to_draw_2 = &tx.d.texture[0];

                if(tx.d.texture_init[3])
                    to_mask_2 = &tx.d.texture[3];
            }
        }

        // draw the left pic
        if(to_draw_2 != nullptr)
        {
            if(rotateAngle != 0.0)
            {
                // TODO: use correct center to support big textures being rotated
                GX_DrawImage_Custom_Rotated(to_draw_2, to_mask_2, xDst, yDst, (1024 - xSrc) * wDst / wSrc, hDst,
                                            xSrc, ySrc, 1024 - xSrc, hSrc, flip, center, rotateAngle, color);
            }
            else
                GX_DrawImage_Custom(to_draw_2, to_mask_2, xDst, yDst, (1024 - xSrc) * wDst / wSrc, hDst,
                                    xSrc, ySrc, 1024 - xSrc, hSrc, flip, color);

            xDst += (1024 - xSrc) * wDst / wSrc;
            wDst -= (1024 - xSrc) * wDst / wSrc;
            wSrc -= (1024 - xSrc);
            xSrc = 0;
        }
        else
            xSrc -= 1024;
    }
    else if(!tx.d.multi_horizontal && ySrc + hSrc > 1024)
    {
        if(ySrc + hSrc > 2048)
        {
            if(tx.d.texture_init[2])
            {
                to_draw = &tx.d.texture[2];

                if(tx.d.texture_init[5])
                    to_mask = &tx.d.texture[5];
            }

            if(ySrc < 2048 && tx.d.texture_init[1])
            {
                to_draw_2 = &tx.d.texture[1];

                if(tx.d.texture_init[4])
                    to_mask_2 = &tx.d.texture[4];
            }

            ySrc -= 1024;
        }
        else
        {
            if(tx.d.texture_init[1])
            {
                to_draw = &tx.d.texture[1];

                if(tx.d.texture_init[4])
                    to_mask = &tx.d.texture[4];
            }

            if(ySrc < 1024)
            {
                to_draw_2 = &tx.d.texture[0];

                if(tx.d.texture_init[3])
                    to_mask_2 = &tx.d.texture[3];
            }
        }

        // draw the top pic
        if(to_draw_2 != nullptr)
        {
            if(rotateAngle != 0.0)
            {
                // TODO: use correct center to support big textures being rotated
                GX_DrawImage_Custom_Rotated(to_draw_2, to_mask_2, xDst, yDst, wDst, (1024 - ySrc) * hDst / hSrc,
                                            xSrc, ySrc, wSrc, 1024 - ySrc, flip, center, rotateAngle, color);
            }
            else
                GX_DrawImage_Custom(to_draw_2, to_mask_2, xDst, yDst, wDst, (1024 - ySrc) * hDst / hSrc,
                                    xSrc, ySrc, wSrc, 1024 - ySrc, flip, color);

            yDst += (1024 - ySrc) * hDst / hSrc;
            hDst -= (1024 - ySrc) * hDst / hSrc;
            hSrc -= (1024 - ySrc);
            ySrc = 0;
        }
        else
            ySrc -= 1024;
    }
    else
    {
        to_draw = &tx.d.texture[0];

        if(tx.d.texture_init[3])
            to_mask = &tx.d.texture[3];
    }

    if(to_draw == nullptr) return;

    if(rotateAngle != 0.0)
        GX_DrawImage_Custom_Rotated(to_draw, to_mask, xDst, yDst, wDst, hDst,
                                    xSrc, ySrc, wSrc, hSrc, flip, center, rotateAngle, color);
    else
        GX_DrawImage_Custom(to_draw, to_mask, xDst, yDst, wDst, hDst,
                            xSrc, ySrc, wSrc, hSrc, flip, color);
}

} // namespace XRender
