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


#include <malloc.h>

#include <gccore.h>

#include <set>

#include <Logger/logger.h>

#include "globals.h"
#include "video.h"
#include "frame_timer.h"
#include "core/render.h"


namespace XRender
{

uint32_t s_current_frame = 0;
bool g_in_frame = false;

#define DEFAULT_FIFO_SIZE   (256*1024)

void *frameBuffer[2] = { NULL, NULL};
int cur_buffer = 0;
GXRModeObj *rmode;
Mtx view;

int s_num_textures_loaded = 0;
int s_num_big_pictures_loaded = 0;
std::set<StdPicture*> s_big_pictures;

int s_viewport_x = 0;
int s_viewport_y = 0;
int s_viewport_w = 0;
int s_viewport_h = 0;
int s_viewport_offset_x = 0;
int s_viewport_offset_y = 0;
int s_viewport_offset_x_bak = 0;
int s_viewport_offset_y_bak = 0;
bool s_viewport_offset_ignore = false;

void s_loadTexture(StdPicture &target, int i)
{
    if(target.d.texture_file_init[i])
    {
        u32 info;
        TPL_GetTextureInfo(&target.d.texture_file[i], 0, &info, &target.d.tex_w[i], &target.d.tex_h[i]);

        if(!target.w)
        {
            target.w = target.d.tex_w[i]*2;
            target.h = target.d.tex_h[i]*2;
        }

        if(TPL_GetTexture(&target.d.texture_file[i], 0, &target.d.texture[i]) != 0)
        {
            TPL_CloseTPLFile(&target.d.texture_file[i]);
            target.d.texture_file_init[i] = false;
        }
        else
        {
            GX_InitTexObjFilterMode(&target.d.texture[i], GX_NEAR, GX_NEAR);
            target.d.texture_init[i] = true;
        }
    }
}

bool init()
{
    Mtx44 perspective;
    f32 yscale;
    u32 xfbHeight;

    GXColor backgroundColor = {0, 0, 0, 255};
    void *fifoBuffer = NULL;
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
    if(rmode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();

    // setup the fifo and then init the flipper
    void *gp_fifo = NULL;
    gp_fifo = memalign(32,DEFAULT_FIFO_SIZE);
    memset(gp_fifo,0,DEFAULT_FIFO_SIZE);

    GX_Init(gp_fifo,DEFAULT_FIFO_SIZE);

    // clears the bg to color and clears the z buffer
    GX_SetCopyClear(background, 0x00ffffff);

    // other gx setup
    GX_SetViewport(0,0,rmode->fbWidth,rmode->efbHeight,0,1);
    yscale = GX_GetYScaleFactor(rmode->efbHeight,rmode->xfbHeight);
    xfbHeight = GX_SetDispCopyYScale(yscale);
    GX_SetScissor(0,0,rmode->fbWidth,rmode->efbHeight);
    GX_SetDispCopySrc(0,0,rmode->fbWidth,rmode->efbHeight);
    GX_SetDispCopyDst(rmode->fbWidth,xfbHeight);
    GX_SetCopyFilter(rmode->aa,rmode->sample_pattern,GX_TRUE,rmode->vfilter);
    GX_SetFieldMode(rmode->field_rendering,((rmode->viHeight==2*rmode->xfbHeight)?GX_ENABLE:GX_DISABLE));
    GX_SetColorUpdate(1);
    GX_SetAlphaUpdate(1);
    GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET);


    printf("%d %d\n", (int)rmode->viWidth, (int)rmode->viHeight);

    GX_SetCullMode(GX_CULL_NONE);
    GX_CopyDisp(frameBuffer[cur_buffer],GX_TRUE);
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


    // setup our projection matrix
    f32 w = rmode->viWidth;
    f32 h = rmode->viHeight;
    guOrtho(perspective, 0.0f, 480.0f, 0.0f, 640.0f, -1.0f, 1.0f);
    GX_LoadProjectionMtx(perspective, GX_ORTHOGRAPHIC);

    s_viewport_x = s_viewport_y = 0;
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
    GX_SetViewport(0,0,rmode->fbWidth,rmode->efbHeight,0,1);
    // load the view matrix into matrix memory
    GX_LoadPosMtxImm(view, GX_PNMTX0);

    g_in_frame = true;
    s_current_frame ++;
}

void setTargetScreen()
{
}

void clearBuffer()
{
}

void repaint()
{
    // do this stuff after drawing
    GX_DrawDone();

    cur_buffer ^= 1;        // flip framebuffer
    GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
    GX_SetColorUpdate(GX_TRUE);
    GX_CopyDisp(frameBuffer[cur_buffer],GX_TRUE);

    g_microStats.start_sleep();
    if(g_videoSettings.scaleMode == SCALE_DYNAMIC_LINEAR)
        VIDEO_WaitVSync();
    g_microStats.start_task(MicroStats::Graphics);

    VIDEO_SetNextFramebuffer(frameBuffer[cur_buffer]);

    VIDEO_Flush();

    g_in_frame = false;
}

void mapToScreen(int x, int y, int *dx, int *dy)
{
    *dx = x * 2;
    *dy = y * 2;
}

void mapFromScreen(int scr_x, int scr_y, int *window_x, int *window_y)
{
    *window_x = scr_x / 2;
    *window_y = scr_y / 2;
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

    if(!GameIsActive)
        return target; // do nothing when game is closed

    target.inited = false;
    target.l.path = path;
    if(target.l.path.empty())
        return target;

    target.inited = true;
    target.l.lazyLoaded = false;

    if(TPL_OpenTPLFromFile(&target.d.texture_file[0], target.l.path.c_str()) == 1)
    {
        target.d.texture_file_init[0] = true;
        s_loadTexture(target, 0);
        s_num_textures_loaded ++;
    }

    if(!target.d.hasTexture())
    {
        pLogWarning("FAILED TO LOAD!!! %s\n", path.c_str());
        target.d.destroy();
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

void lazyLoad(StdPicture &target)
{
    if(!target.inited || !target.l.lazyLoaded || target.d.hasTexture())
        return;

    std::string suppPath;

    target.inited = true;
    target.l.lazyLoaded = false;

    if(TPL_OpenTPLFromFile(&target.d.texture_file[0], target.l.path.c_str()) != 1)
    {
        pLogWarning("Permanently failed to load %s", target.l.path.c_str());
        pLogWarning("Error: %d (%s)", errno, strerror(errno));
        target.inited = false;
        return;
    }

    target.d.texture_file_init[0] = true;
    s_loadTexture(target, 0);

    if(!target.d.hasTexture())
    {
        pLogWarning("Permanently failed to load %s", target.l.path.c_str());
        pLogWarning("Error: %d (%s)", errno, strerror(errno));
        target.d.destroy();
        target.inited = false;
        return;
    }

    if(target.h > 2048)
    {
        suppPath = target.l.path + '1';
        if(TPL_OpenTPLFromFile(&target.d.texture_file[1], suppPath.c_str()) != 1)
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
    if(target.h > 4096)
    {
        suppPath = target.l.path + '2';
        if(TPL_OpenTPLFromFile(&target.d.texture_file[2], suppPath.c_str()) != 1)
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

    s_num_textures_loaded++;

    if(target.w >= 256 || target.h >= 256)
    {
        s_big_pictures.insert(&target);
        s_num_big_pictures_loaded++;
    }
}

void lazyPreLoad(StdPicture &target)
{
    lazyLoad(target);
}

void lazyUnLoad(StdPicture &target)
{
    if(!target.inited || !target.l.lazyLoaded || !target.d.hasTexture())
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

    if(tx.d.hasTexture())
        s_num_textures_loaded--;

    tx.d.destroy();

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
    return std::nearbyintf(std::roundf((float)x / 2.0f));
}

inline float FLOORDIV2(float x)
{
    return std::floor(x / 2.0f);
}

void renderRect(int x, int y, int w, int h, float red, float green, float blue, float alpha, bool filled)
{
    int x_div = ROUNDDIV2(x);
    int w_div = ROUNDDIV2(x + w) - x_div;

    x_div += s_viewport_offset_x;

    int y_div = ROUNDDIV2(y);
    int h_div = ROUNDDIV2(y + h) - y_div;

    x_div += s_viewport_offset_y;

    uint8_t r = red * 255.0f + 0.5f;
    uint8_t g = green * 255.0f + 0.5f;
    uint8_t b = blue * 255.0f + 0.5f;
    uint8_t a = alpha * 255.0f + 0.5f;

    GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORDNULL, GX_TEXMAP_NULL, GX_COLOR0);
    GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);

    GX_Begin(GX_QUADS, GX_VTXFMT0, 4);          // Draw A Quad
        GX_Position3s16(x_div, y_div, 0); // Top Left
        GX_Color4u8(r, g, b, a);           // Set The Color To Blue
        GX_TexCoord2u16(0, 0);

        GX_Position3s16( x_div + w_div, y_div, 0);        // Top Right
        GX_Color4u8(r, g, b, a);           // Set The Color To Blue
        GX_TexCoord2u16(0, 0);

        GX_Position3s16( x_div + w_div, y_div + h_div, 0); // Bottom Right
        GX_Color4u8(r, g, b, a);           // Set The Color To Blue
        GX_TexCoord2u16(0, 0);

        GX_Position3s16( x_div, y_div + h_div, 0);  // Bottom Left
        GX_Color4u8(r, g, b, a);           // Set The Color To Blue
        GX_TexCoord2u16(0, 0);
    GX_End();                                   // Done Drawing The Quad
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
    if(radius <= 0)
        return; // Nothing to draw

    double line_size = 4;
    double dy = line_size;

    do
    {
        double dx = std::floor(std::sqrt((2.0 * radius * dy) - (dy * dy)));

        renderRectBR(cx - radius, cy + dy - radius - line_size, cx - dx, cy + dy - radius + line_size,
            red, green, blue, alpha);

        renderRectBR(cx + dx, cy + dy - radius - line_size, cx + radius, cy + dy - radius + line_size,
            red, green, blue, alpha);

        if(dy < radius) // Don't cross lines
        {
            renderRectBR(cx - radius, cy - dy + radius - line_size, cx + radius, cy - dy + radius + line_size,
                red, green, blue, alpha);

            renderRectBR(cx + dx, cy - dy + radius - line_size, cx + radius, cy - dy + radius + line_size,
                red, green, blue, alpha);
        }

        dy += line_size * 2;
    } while(dy + line_size <= radius);
}

inline bool GX_DrawImage_Custom(GXTexObj* img,
    int16_t x, int16_t y, uint16_t w, uint16_t h,
    uint16_t src_x, uint16_t src_y, uint16_t src_w, uint16_t src_h,
    unsigned int flip,
    float _r, float _g, float _b, float _a)
{
    uint16_t u1 = src_x;
    uint16_t u2 = src_x + src_w;
    uint16_t v1 = src_y;
    uint16_t v2 = src_y + src_h;

    if(flip & X_FLIP_HORIZONTAL)
        std::swap(u1, u2);
    if(flip & X_FLIP_VERTICAL)
        std::swap(v1, v2);

    uint8_t r = _r * 255.0f + 0.5f;
    uint8_t g = _g * 255.0f + 0.5f;
    uint8_t b = _b * 255.0f + 0.5f;
    uint8_t a = _a * 255.0f + 0.5f;

    GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE);
    GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GX_LoadTexObj(img, GX_TEXMAP0);

    GX_Begin(GX_QUADS, GX_VTXFMT0, 4);          // Draw A Quad
        GX_Position3s16(x, y, 0); // Top Left
        GX_Color4u8(r, g, b, a);
        GX_TexCoord2u16(u1, v1);

        GX_Position3s16(x + w, y, 0);        // Top Right
        GX_Color4u8(r, g, b, a);
        GX_TexCoord2u16(u2, v1);

        GX_Position3s16(x + w, y + h, 0); // Bottom Right
        GX_Color4u8(r, g, b, a);
        GX_TexCoord2u16(u2, v2);

        GX_Position3s16(x, y + h, 0);  // Bottom Left
        GX_Color4u8(r, g, b, a);
        GX_TexCoord2u16(u1, v2);
    GX_End();                                   // Done Drawing The Quad

    return true;
}

inline bool GX_DrawImage_Custom_Rotated(GXTexObj* img,
    float x, float y, float w, float h,
    float src_x, float src_y, float src_w, float src_h,
    unsigned int flip, FPoint_t *center, float angle,
    float r, float g, float b, float a)
{
    return true;
}

inline void i_renderTexturePrivate(int16_t xDst, int16_t yDst, int16_t wDst, int16_t hDst,
                             StdPicture &tx,
                             int16_t xSrc, int16_t ySrc, int16_t wSrc, int16_t hSrc,
                             float rotateAngle, FPoint_t *center, unsigned int flip,
                             float red, float green, float blue, float alpha)
{
    if(!tx.inited)
        return;

    if(!tx.d.hasTexture() && tx.l.lazyLoaded)
        lazyLoad(tx);

    tx.d.last_draw_frame = s_current_frame;

    if(!tx.d.hasTexture())
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
    if(xDst < 0)
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
        xDst = 0;
    }
    else if(xDst + wDst > s_viewport_w)
    {
        if(flip & X_FLIP_HORIZONTAL)
            xSrc += (wDst + xDst - s_viewport_w) * wSrc / wDst;
        wSrc = (s_viewport_w - xDst) * wSrc / wDst;
        wDst = (s_viewport_w - xDst);
    }

    if(yDst < 0)
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
        yDst = 0;
    }
    else if(yDst + hDst > s_viewport_h)
    {
        if(flip & X_FLIP_VERTICAL)
            ySrc += (hDst + yDst - s_viewport_h) * hSrc / hDst;
        hSrc = (s_viewport_h - yDst) * hSrc / hDst;
        hDst = (s_viewport_h - yDst);
    }

    GXTexObj* to_draw = nullptr;
    GXTexObj* to_draw_2 = nullptr;

    // Don't go more than size of texture
    // Failure conditions should only happen if texture is smaller than expected
    if(xSrc + wSrc > tx.w / 2)
    {
        wDst = (tx.w / 2 - xSrc) * wDst / wSrc;
        wSrc = tx.w / 2 - xSrc;
        if(wDst < 0)
            return;
    }
    if(ySrc + hSrc > tx.h / 2)
    {
        hDst = (tx.h / 2 - ySrc) * hDst / hSrc;
        hSrc = tx.h / 2 - ySrc;
        if(hDst < 0)
            return;
    }

    if(ySrc + hSrc > 1024)
    {
        if(ySrc + hSrc > 2048)
        {
            if(tx.d.texture_init[2])
                to_draw = &tx.d.texture[2];
            if(ySrc < 2048 && tx.d.texture_init[1])
                to_draw_2 = &tx.d.texture[1];
            ySrc -= 1024;
        }
        else
        {
            if(tx.d.texture_init[1])
                to_draw = &tx.d.texture[1];
            if(ySrc < 1024)
                to_draw_2 = &tx.d.texture[0];
        }
        // draw the top pic
        if(to_draw_2 != nullptr)
        {
            if(rotateAngle != 0.0)
                GX_DrawImage_Custom_Rotated(to_draw_2, xDst + s_viewport_offset_x, yDst + s_viewport_offset_y, wDst, (1024 - ySrc) * hDst / hSrc,
                                    xSrc, ySrc, wSrc, 1024 - ySrc, flip, center, rotateAngle, red, green, blue, alpha);
            else
                GX_DrawImage_Custom(to_draw_2, xDst + s_viewport_offset_x, yDst + s_viewport_offset_y, wDst, (1024 - ySrc) * hDst / hSrc,
                                    xSrc, ySrc, wSrc, 1024 - ySrc, flip, red, green, blue, alpha);
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
    }

    if(to_draw == nullptr) return;

    if(rotateAngle != 0.0)
        GX_DrawImage_Custom_Rotated(to_draw, xDst + s_viewport_offset_x, yDst + s_viewport_offset_y, wDst, hDst,
                             xSrc, ySrc, wSrc, hSrc, flip, center, rotateAngle, red, green, blue, alpha);
    else
        GX_DrawImage_Custom(to_draw, xDst + s_viewport_offset_x, yDst + s_viewport_offset_y, wDst, hDst,
                            xSrc, ySrc, wSrc, hSrc, flip, red, green, blue, alpha);
}

// public draw methods

void renderTextureScale(double xDst, double yDst, double wDst, double hDst,
                            StdPicture &tx,
                            int xSrc, int ySrc, int wSrc, int hSrc,
                            float red, float green, float blue, float alpha)
{
    auto div_x = ROUNDDIV2(xDst), div_y = ROUNDDIV2(yDst);
    i_renderTexturePrivate(
        div_x, div_y, ROUNDDIV2(xDst + wDst) - div_x, ROUNDDIV2(yDst + hDst) - div_y,
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
    auto div_x = ROUNDDIV2(xDst), div_y = ROUNDDIV2(yDst);
    auto div_w = ROUNDDIV2(xDst + wDst) - div_x;
    auto div_h = ROUNDDIV2(yDst + hDst) - div_y;

    i_renderTexturePrivate(
        div_x, div_y, div_w, div_h,
        tx,
        ROUNDDIV2(xSrc), ROUNDDIV2(ySrc), div_w, div_h,
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
        0, 0, w, h,
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
    auto div_x = ROUNDDIV2(xDst), div_y = ROUNDDIV2(yDst);
    auto div_w = ROUNDDIV2(xDst + wDst) - div_x;
    auto div_h = ROUNDDIV2(yDst + hDst) - div_y;

    i_renderTexturePrivate(
        div_x, div_y, div_w, div_h,
        tx,
        0.0f, 0.0f, tx.w / 2, tx.h / 2,
        0.0f, nullptr, X_FLIP_NONE,
        red, green, blue, alpha);
}

void renderTextureFL(double xDst, double yDst, double wDst, double hDst,
                          StdPicture &tx,
                          int xSrc, int ySrc,
                          double rotateAngle, FPoint_t *center, unsigned int flip,
                          float red, float green, float blue, float alpha)
{
    auto div_x = ROUNDDIV2(xDst), div_y = ROUNDDIV2(yDst);
    auto div_w = ROUNDDIV2(xDst + wDst) - div_x;
    auto div_h = ROUNDDIV2(yDst + hDst) - div_y;

    i_renderTexturePrivate(
        div_x, div_y, div_w, div_h,
        tx,
        ROUNDDIV2(xSrc), ROUNDDIV2(ySrc), div_w, div_h,
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
    auto div_x = ROUNDDIV2(xDst), div_y = ROUNDDIV2(yDst);
    auto div_w = ROUNDDIV2(xDst + wDst) - div_x;
    auto div_h = ROUNDDIV2(yDst + hDst) - div_y;

    auto div_sx = ROUNDDIV2(xSrc), div_sy = ROUNDDIV2(ySrc);
    auto div_sw = ROUNDDIV2(xSrc + wSrc) - div_sx;
    auto div_sh = ROUNDDIV2(ySrc + hSrc) - div_sy;

    i_renderTexturePrivate(
        div_x, div_y, div_w, div_h,
        tx,
        div_sx, div_sy, div_sw, div_sh,
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
