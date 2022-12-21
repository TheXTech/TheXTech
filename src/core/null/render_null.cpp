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

#include <set>

#include <Logger/logger.h>

#include "globals.h"
#include "video.h"
#include "frame_timer.h"
#include "core/render.h"

#ifdef THEXTECH_CLI_BUILD
#include <Graphics/image_size.h>
#endif

namespace XRender
{

bool init()
{
    return true;
}

void quit()
{}

void setTargetTexture()
{}

void setTargetScreen()
{}

void clearBuffer()
{}

void repaint()
{}

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
    else
    {
#ifdef THEXTECH_CLI_BUILD
        uint32_t w, h;
        if(PGE_ImageInfo::getImageSize(path, &w, &h, nullptr))
        {
            target.w = w;
            target.h = h;
        }
        else
        {
            pLogWarning("loadPicture: Couldn't open file.");
            target.inited = false;
            return target;
        }
#else
        // this will work if it's a PNG
        FILE* fpng = fopen(path.c_str(), "rb");
        if(!fpng)
        {
            pLogWarning("loadPicture: Couldn't open size file.");
            target.inited = false;
            return target;
        }

        fseek(fpng, 16, SEEK_SET);

        uint32_t w, h;
        if(fread(&w, 4, 1, fpng) == 1 && fread(&h, 4, 1, fpng) == 1)
        {
            w = SDL_SwapLE32(w);
            h = SDL_SwapLE32(h);

            w = static_cast<uint32_t>(((w << 24) | ((w << 8) & 0x00FF0000) |
                ((w >> 8) & 0x0000FF00) | (w >> 24)));
            h = static_cast<uint32_t>(((h << 24) | ((h << 8) & 0x00FF0000) |
                ((h >> 8) & 0x0000FF00) | (h >> 24)));

            target.w = w;
            target.h = h;
        }
        else
        {
            target.inited = false;
        }

        fclose(fpng);
#endif
    }

    return target;
}


StdPicture lazyLoadPicture(const std::string& path, const std::string& maskPath, const std::string& maskFallbackPath)
{
    return LoadPicture(path, maskPath, maskFallbackPath);
}

StdPicture lazyLoadPictureFromList(FILE* f, const std::string& dir)
{
    StdPicture target;
    if(!GameIsActive)
        return target; // do nothing when game is closed

    int length;

    char filename[256];
    if(fscanf(f, "%255[^\n]%n%*[^\n]\n", filename, &length) != 1)
    {
        pLogWarning("Could not load image path from load list");
        return target;
    }

    if(length == 255)
    {
        pLogWarning("Image path %s was truncated in load list", filename);
        return target;
    }

    target.inited = true;
    target.l.path = dir;
    target.l.path += filename;
    target.l.lazyLoaded = true;

    int w, h, flags;
    if((fscanf(f, "%d\n%d\n%d\n", &w, &h, &flags) != 3) || (w < 0) || (w > 8192) || (h < 0) || (h > 8192))
    {
        pLogWarning("Could not load image %s dimensions from load list", filename);
        target.inited = false;
        return target;
    }

    // pLogDebug("Successfully loaded %s (%d %d)", target.l.path.c_str(), w, h);

    target.w = w;
    target.h = h;

    return target;
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

} // namespace XRender
