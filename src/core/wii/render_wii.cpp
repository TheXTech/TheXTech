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

bool g_in_frame = false;

#define DEFAULT_FIFO_SIZE   (256*1024)

void *frameBuffer[2] = { NULL, NULL};
int cur_buffer = 0;
GXRModeObj *rmode;
Mtx view;

uint8_t frame = 0;

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

    printf("%d %d\n", (int)rmode->viWidth, (int)rmode->viHeight);

    GX_SetCullMode(GX_CULL_NONE);
    GX_CopyDisp(frameBuffer[cur_buffer],GX_TRUE);
    GX_SetDispCopyGamma(GX_GM_1_0);


    // setup the vertex descriptor
    // tells the flipper to expect direct data
    GX_ClearVtxDesc();
    GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
    GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    // GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);

    // setup the vertex attribute table
    // describes the data
    // args: vat location 0-7, type of data, data format, size, scale
    // so for ex. in the first call we are sending position data with
    // 3 values X,Y,Z of size F32. scale sets the number of fractional
    // bits for non float data.
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_S16, 0);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);

    GX_SetNumChans(1);
    GX_SetNumTexGens(0);
    GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORDNULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);

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
    frame++;
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

    VIDEO_SetNextFramebuffer(frameBuffer[cur_buffer]);

    VIDEO_Flush();

    VIDEO_WaitVSync();
    g_in_frame = false;
}

void mapToScreen(int x, int y, int *dx, int *dy)
{
    *dx = x;
    *dy = y;
}

void mapFromScreen(int scr_x, int scr_y, int *window_x, int *window_y)
{
    *window_x = scr_x;
    *window_y = scr_y;
}

void updateViewport()
{
}

void resetViewport()
{
}

void setViewport(int x, int y, int w, int h)
{
    UNUSED(x);
    UNUSED(y);
    UNUSED(w);
    UNUSED(h);
}

void offsetViewport(int x, int y)
{
    UNUSED(x);
    UNUSED(y);
}

void offsetViewportIgnore(bool en)
{
    UNUSED(en);
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

    target.inited = true;

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
            pLogWarning("loadPicture: Couldn't close file.");
    }
    // lazy load and unload to read dimensions if it doesn't exist.
    // unload is essential because lazy load would save the address incorrectly.
    else
    {
        pLogWarning("loadPicture: Couldn't open size file.");
        target.inited = false;
    }

    return target;
}


StdPicture lazyLoadPicture(const std::string& path, const std::string& maskPath, const std::string& maskFallbackPath)
{
    return LoadPicture(path, maskPath, maskFallbackPath);
}

void lazyLoad(StdPicture &target)
{
    UNUSED(target);
    return;
}

void lazyPreLoad(StdPicture &target)
{
    lazyLoad(target);
}

void lazyUnLoad(StdPicture &target)
{
    deleteTexture(target, true);
}

void deleteTexture(StdPicture &tx, bool lazyUnload)
{
    if(!lazyUnload)
    {
        tx.inited = false;
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
    GX_Begin(GX_QUADS, GX_VTXFMT0, 4);          // Draw A Quad
        GX_Position3s16((int16_t)x, (int16_t)y, 0); // Top Left
        GX_Color4u8(127+frame,127+frame,255+frame,255);           // Set The Color To Blue
        GX_Position3s16( (int16_t)(x + w), (int16_t)(y), 0);        // Top Right
        GX_Color4u8(127+frame,255+frame,255+frame,255);           // Set The Color To Blue
        GX_Position3s16( (int16_t)(x + w),(int16_t)(y + h), 0); // Bottom Right
        GX_Color4u8(127+frame,127+frame,255+frame,127);           // Set The Color To Blue
        GX_Position3s16((int16_t)(x), (int16_t)(y + h), 0);  // Bottom Left
        GX_Color4u8(255+frame,127+frame,255+frame,255);           // Set The Color To Blue
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

inline void i_renderTexturePrivate(float xDst, float yDst, float wDst, float hDst,
                             StdPicture &tx,
                             float xSrc, float ySrc, float wSrc, float hSrc,
                             float rotateAngle, FPoint_t *center, unsigned int flip,
                             float red, float green, float blue, float alpha)
{
    renderRect(xDst, yDst, wDst, hDst);
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
        xSrc / 2, ySrc / 2, div_w, div_h,
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
    auto div_x = ROUNDDIV2(xDst), div_y = ROUNDDIV2(yDst);
    auto div_w = ROUNDDIV2(xDst + wDst) - div_x;
    auto div_h = ROUNDDIV2(yDst + hDst) - div_y;

    i_renderTexturePrivate(
        div_x, div_y, div_w, div_h,
        tx,
        xSrc / 2, ySrc / 2, div_w, div_h,
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

    i_renderTexturePrivate(
        div_x, div_y, div_w, div_h,
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
