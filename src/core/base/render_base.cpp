/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2023 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <SDL2/SDL_thread.h>
#include <SDL2/SDL_power.h>
#include <SDL2/SDL_rwops.h>

#define USE_SDL_POWER
#ifdef THEXTECH_BIG_ENDIAN
# define GIF_H_BIG_ENDIAN
#endif

#include <FreeImageLite.h>

#include <AppPath/app_path.h>
#include <Logger/logger.h>
#include <Graphics/graphics_funcs.h>
#include <Utils/elapsed_timer.h>
#include <DirManager/dirman.h>
#include <Utils/files.h>
#include <fmt_time_ne.h>
#include <fmt_format_ne.h>
#include <gif.h>
#include <pge_delay.h>

#include <chrono>

#include "render_base.h"
#include "../render.h"
#include "video.h"
#include "globals.h"
#include "sound.h"
#include "graphics.h"

#ifdef USE_SCREENSHOTS_AND_RECS
#include <deque>
#endif


AbstractRender_t* g_render = nullptr;

size_t AbstractRender_t::m_lazyLoadedBytes = 0;
int    AbstractRender_t::m_maxTextureWidth = 0;
int    AbstractRender_t::m_maxTextureHeight = 0;

int    AbstractRender_t::ScaleWidth = 0;
int    AbstractRender_t::ScaleHeight = 0;

#ifdef USE_RENDER_BLOCKING
bool   AbstractRender_t::m_blockRender = false;
#endif


#ifdef USE_SCREENSHOTS_AND_RECS

static int makeShot_action(void *_pixels);
static SDL_Thread *s_screenshot_thread = nullptr;

static int processRecorder_action(void *_recorder);

GifRecorder *AbstractRender_t::m_gif = nullptr;


struct PGE_GL_shoot
{
    uint8_t *pixels = nullptr;
    int pitch = 0;
    int w = 0, h = 0;
};

struct GifRecorder
{
    AbstractRender_t *m_self = nullptr;
    GIF_H::GifWriter  writer      = {nullptr, nullptr, true, false};
    SDL_Thread *worker      = nullptr;
    uint32_t    delay       = 4;
    uint32_t    delayTimer  = 0;
    bool        enabled     = false;
    unsigned char padding[7] = {0, 0, 0, 0, 0, 0, 0};
    bool        fadeForward = true;
    float       fadeValue = 0.5f;

    std::deque<PGE_GL_shoot> queue;
    SDL_mutex  *mutex = nullptr;
    bool        doFinalize = false;

    void init(AbstractRender_t *self);
    void quit();

    void drawRecCircle();
    bool hasSome();
    void enqueue(const PGE_GL_shoot &entry);
    PGE_GL_shoot dequeue();
};

#endif // USE_SCREENSHOTS_AND_RECS




AbstractRender_t::AbstractRender_t()
{
#ifdef USE_SCREENSHOTS_AND_RECS
    m_gif = new GifRecorder();
#endif
}

AbstractRender_t::~AbstractRender_t()
{
#ifdef USE_SCREENSHOTS_AND_RECS
    delete m_gif;
    m_gif = nullptr;
#endif
}

bool AbstractRender_t::init()
{
    ScaleWidth = ScreenW;
    ScaleHeight = ScreenH;

#ifdef USE_SCREENSHOTS_AND_RECS
    m_gif->init(this);
#endif
    return true;
}

void AbstractRender_t::close()
{
#ifdef USE_SCREENSHOTS_AND_RECS
    m_gif->quit();
#endif
}

StdPicture AbstractRender_t::LoadPicture(const std::string &path,
                                         const std::string &maskPath,
                                         const std::string &maskFallbackPath)
{
    StdPicture target;
    FIBITMAP *sourceImage;
    FIBITMAP *maskImage = nullptr;
    bool useMask = true;

    if(!GameIsActive)
        return target; // do nothing when game is closed

    if(path.empty())
        return target;

#ifdef DEBUG_BUILD
    target.origPath = path;
#endif

    sourceImage = GraphicsHelps::loadImage(path);

    // Don't load mask if PNG image is used
    if(Files::hasSuffix(path, ".png"))
        useMask = false;

    if(!sourceImage)
    {
        pLogWarning("Error loading of image file:\n"
                    "%s\n"
                    "Reason: %s.",
                    path.c_str(),
                    (Files::fileExists(path) ? "wrong image format" : "file not exist"));
        // target = g_renderer->getDummyTexture();
        return target;
    }

#ifdef DEBUG_BUILD
    ElapsedTimer totalTime;
    ElapsedTimer maskMergingTime;
    ElapsedTimer bindingTime;
    ElapsedTimer unloadTime;
    totalTime.start();
    int64_t maskElapsed = 0;
    int64_t bindElapsed = 0;
    int64_t unloadElapsed = 0;
#endif

    //Apply Alpha mask
    if(useMask && !maskPath.empty() && Files::fileExists(maskPath))
    {
        // load mask
        maskImage = GraphicsHelps::loadMask(maskPath, false);

#ifdef DEBUG_BUILD
            maskMergingTime.start();
#endif

        // merge it with image if masks are unsupported or the mask could be properly represented with RGBA
        if(!g_render->textureMaskSupported()
            || !GraphicsHelps::validateBitmaskRequired(sourceImage, maskImage, path))
        {
            GraphicsHelps::mergeWithMask(sourceImage, maskImage);
            GraphicsHelps::closeImage(maskImage);
            maskImage = nullptr;
        }

#ifdef DEBUG_BUILD
            maskElapsed = maskMergingTime.nanoelapsed();
#endif
    }
    //Use fallback PNG mask
    else if(useMask && !maskFallbackPath.empty())
    {
#ifdef DEBUG_BUILD
        maskMergingTime.start();
#endif

        GraphicsHelps::mergeWithMask(sourceImage, "", maskFallbackPath);

#ifdef DEBUG_BUILD
        maskElapsed = maskMergingTime.nanoelapsed();
#endif
    }

    uint32_t w = static_cast<uint32_t>(FreeImage_GetWidth(sourceImage));
    uint32_t h = static_cast<uint32_t>(FreeImage_GetHeight(sourceImage));
    uint32_t pitch = static_cast<uint32_t>(FreeImage_GetPitch(sourceImage));

    if((w == 0) || (h == 0))
    {
        FreeImage_Unload(sourceImage);
        pLogWarning("Error loading of image file:\n"
                    "%s\n"
                    "Reason: %s.",
                    path.c_str(),
                    "Zero image size!");
        //target = g_renderer->getDummyTexture();
        return target;
    }

#ifdef DEBUG_BUILD
    bindingTime.start();
#endif

    RGBQUAD upperColor;
    FreeImage_GetPixelColor(sourceImage, 0, 0, &upperColor);
    target.ColorUpper.r = upperColor.rgbRed / 255.0f;
    target.ColorUpper.g = upperColor.rgbGreen / 255.0f;
    target.ColorUpper.b = upperColor.rgbBlue / 255.0f;

    RGBQUAD lowerColor;
    FreeImage_GetPixelColor(sourceImage, 0, static_cast<unsigned int>(h - 1), &lowerColor);
    target.ColorLower.r = lowerColor.rgbRed / 255.0f;
    target.ColorLower.b = lowerColor.rgbBlue / 255.0f;
    target.ColorLower.g = lowerColor.rgbGreen / 255.0f;

    FreeImage_FlipVertical(sourceImage);
    if(maskImage)
        FreeImage_FlipVertical(maskImage);

    target.w = static_cast<int>(w);
    target.h = static_cast<int>(h);
    target.frame_w = static_cast<int>(w);
    target.frame_h = static_cast<int>(h);

    uint8_t *textura = reinterpret_cast<uint8_t *>(FreeImage_GetBits(sourceImage));
    g_render->loadTexture(target, w, h, textura, pitch);

    if(maskImage || !GraphicsHelps::validateForDepthTest(sourceImage, path))
        target.d.invalidateDepthTest();

#ifdef DEBUG_BUILD
    bindElapsed = bindingTime.nanoelapsed();
    unloadTime.start();
#endif
    //SDL_FreeSurface(sourceImage);
    GraphicsHelps::closeImage(sourceImage);

#ifdef DEBUG_BUILD
    unloadElapsed = unloadTime.nanoelapsed();
#endif

    if(maskImage)
    {
        uint32_t w_mask = static_cast<uint32_t>(FreeImage_GetWidth(maskImage));
        uint32_t h_mask = static_cast<uint32_t>(FreeImage_GetHeight(maskImage));
        uint32_t pitch_mask = static_cast<uint32_t>(FreeImage_GetPitch(maskImage));

        textura = reinterpret_cast<uint8_t *>(FreeImage_GetBits(maskImage));
        g_render->loadTextureMask(target, w_mask, h_mask, textura, pitch_mask, w, h);
        GraphicsHelps::closeImage(maskImage);
    }

#ifdef DEBUG_BUILD
    pLogDebug("Mask merging of %s passed in %d nanoseconds", path.c_str(), static_cast<int>(maskElapsed));
    pLogDebug("Binding time of %s passed in %d nanoseconds", path.c_str(), static_cast<int>(bindElapsed));
    pLogDebug("Unload time of %s passed in %d nanoseconds", path.c_str(), static_cast<int>(unloadElapsed));
    pLogDebug("Total Loading of texture %s passed in %d nanoseconds (%dx%d)",
              path.c_str(),
              static_cast<int>(totalTime.nanoelapsed()),
              static_cast<int>(w),
              static_cast<int>(h));
#endif

    return target;
}

StdPicture AbstractRender_t::LoadPicture_1x(const std::string &path,
                                         const std::string &maskPath,
                                         const std::string &maskFallbackPath)
{
    StdPicture target = LoadPicture(path, maskPath, maskFallbackPath);

    if(target.inited)
    {
        target.l.w_orig = target.w;
        target.l.h_orig = target.h;
        target.l.w_scale /= 2;
        target.l.h_scale /= 2;

        target.w *= 2;
        target.h *= 2;
        target.frame_w *= 2;
        target.frame_h *= 2;
    }

    return target;
}

void AbstractRender_t::loadTextureMask(StdPicture &target,
                         uint32_t mask_width,
                         uint32_t mask_height,
                         uint8_t *RGBApixels,
                         uint32_t pitch,
                         uint32_t image_width,
                         uint32_t image_height)
{
    /* unimplemented */
}

bool AbstractRender_t::textureMaskSupported()
{
    return false;
}

void AbstractRender_t::loadTexture_1x(StdPicture &target,
                         uint32_t width,
                         uint32_t height,
                         uint8_t *RGBApixels,
                         uint32_t pitch)
{
    loadTexture(target, width, height, RGBApixels, pitch);
//    if(target.inited)
//    {
//        target.l.w_orig = target.w;
//        target.l.h_orig = target.h;
//        target.l.w_scale /= 2;
//        target.l.h_scale /= 2;

//        target.w *= 2;
//        target.h *= 2;
//        target.frame_w *= 2;
//        target.frame_h *= 2;
//    }
}

static void dumpFullFile(std::vector<char> &dst, const std::string &path)
{
    dst.clear();
    SDL_RWops *f;

    f = SDL_RWFromFile(path.c_str(), "rb");
    if(!f)
        return;

    Sint64 fSize = SDL_RWsize(f);
    if(fSize < 0)
    {
        pLogWarning("Failed to get size of the file: %s", path.c_str());
        SDL_RWclose(f);
        return;
    }

    dst.resize(size_t(fSize));
    if(SDL_RWread(f, dst.data(), 1, fSize) != size_t(fSize))
        pLogWarning("Failed to dump file on read operation: %s", path.c_str());

    SDL_RWclose(f);
}

StdPicture AbstractRender_t::lazyLoadPicture(const std::string &path,
                                             const std::string &maskPath,
                                             const std::string &maskFallbackPath)
{
    StdPicture target;
    PGE_Size tSize;
    bool useMask = true;

    if(!GameIsActive)
        return target; // do nothing when game is closed

    if(path.empty())
        return target;

#ifdef DEBUG_BUILD
    target.origPath = path;
#endif

    // Don't load mask if PNG image is used
    if(Files::hasSuffix(path, ".png"))
        useMask = false;

    if(!GraphicsHelps::getImageMetrics(path, &tSize))
    {
        pLogWarning("Error loading of image file:\n"
                    "%s\n"
                    "Reason: %s.",
                    path.c_str(),
                    (Files::fileExists(path) ? "wrong image format" : "file not exist"));
        // target = g_renderer->getDummyTexture();
        return target;
    }

    target.w = tSize.w();
    target.h = tSize.h();

    dumpFullFile(target.l.raw, path);

    //Apply Alpha mask
    if(useMask && !maskPath.empty() && Files::fileExists(maskPath))
    {
        dumpFullFile(target.l.rawMask, maskPath);
        target.l.isMaskPng = false; //-V1048
    }
    else if(useMask && !maskFallbackPath.empty())
    {
        dumpFullFile(target.l.rawMask, maskFallbackPath);
        target.l.isMaskPng = true;
    }

    target.inited = true;
    target.l.lazyLoaded = true;
    target.d.clear();

    return target;
}

void AbstractRender_t::setTransparentColor(StdPicture& target, uint32_t rgb)
{
    target.l.colorKey = true;
    target.l.keyRgb[0] = (rgb >> 0) & 0xFF;
    target.l.keyRgb[1] = (rgb >> 8) & 0xFF;
    target.l.keyRgb[2] = (rgb >> 16) & 0xFF;
}

void AbstractRender_t::lazyLoad(StdPicture &target)
{
    if(!target.inited || !target.l.lazyLoaded || target.d.hasTexture())
        return;

    FIBITMAP *sourceImage = GraphicsHelps::loadImage(target.l.raw);
    if(!sourceImage)
    {
        pLogCritical("Lazy-decompress has failed: invalid image data");
        return;
    }

    FIBITMAP *maskImage = nullptr;
    if(!target.l.rawMask.empty())
    {
        // load mask
        maskImage = GraphicsHelps::loadMask(target.l.rawMask, target.l.isMaskPng);

        // merge it with image if PNG, masks are unsupported, or the mask could be properly represented with RGBA
        if(target.l.isMaskPng || !g_render->textureMaskSupported()
            || !GraphicsHelps::validateBitmaskRequired(sourceImage, maskImage, StdPictureGetOrigPath(target)))
        {
            GraphicsHelps::mergeWithMask(sourceImage, maskImage);
            GraphicsHelps::closeImage(maskImage);
            maskImage = nullptr;
        }
    }

    uint32_t w = static_cast<uint32_t>(FreeImage_GetWidth(sourceImage));
    uint32_t h = static_cast<uint32_t>(FreeImage_GetHeight(sourceImage));
    uint32_t pitch = static_cast<uint32_t>(FreeImage_GetPitch(sourceImage));

    if((w == 0) || (h == 0))
    {
        GraphicsHelps::closeImage(sourceImage);
        pLogWarning("Error lazy-decompressing of image file:\n"
                    "Reason: %s."
                    "Zero image size!");
        //target = g_renderer->getDummyTexture();
        return;
    }

    m_lazyLoadedBytes += (w * h * 4);
    if(!target.l.rawMask.empty())
        m_lazyLoadedBytes += (w * h * 4);

    RGBQUAD upperColor;
    FreeImage_GetPixelColor(sourceImage, 0, static_cast<unsigned int>(h - 1), &upperColor);
    target.ColorUpper.r = float(upperColor.rgbRed) / 255.0f;
    target.ColorUpper.b = float(upperColor.rgbBlue) / 255.0f;
    target.ColorUpper.g = float(upperColor.rgbGreen) / 255.0f;

    RGBQUAD lowerColor;
    FreeImage_GetPixelColor(sourceImage, 0, 0, &lowerColor);
    target.ColorLower.r = float(lowerColor.rgbRed) / 255.0f;
    target.ColorLower.b = float(lowerColor.rgbBlue) / 255.0f;
    target.ColorLower.g = float(lowerColor.rgbGreen) / 255.0f;

    if(target.l.colorKey) // Apply transparent color for key pixels
    {
        PGE_Pix colSrc = {target.l.keyRgb[0],
                          target.l.keyRgb[1],
                          target.l.keyRgb[2], 0xFF};
        PGE_Pix colDst = {target.l.keyRgb[0],
                          target.l.keyRgb[1],
                          target.l.keyRgb[2], 0x00};
        GraphicsHelps::replaceColor(sourceImage, colSrc, colDst);
    }

    FreeImage_FlipVertical(sourceImage);
    if(maskImage)
        FreeImage_FlipVertical(maskImage);

    target.w = static_cast<int>(w);
    target.h = static_cast<int>(h);
    target.frame_w = static_cast<int>(w);
    target.frame_h = static_cast<int>(h);

    bool shrink2x;
    switch(g_videoSettings.scaleDownTextures)
    {
    case VideoSettings_t::SCALE_ALL:
        shrink2x = true;
        break;
    case VideoSettings_t::SCALE_SAFE:
        shrink2x = GraphicsHelps::validateFor2xScaleDown(sourceImage, StdPictureGetOrigPath(target));
        if(maskImage)
            shrink2x &= GraphicsHelps::validateFor2xScaleDown(maskImage, StdPictureGetOrigPath(target));
        break;
    case VideoSettings_t::SCALE_NONE:
    default:
        shrink2x = false;
        break;
    }

    if(shrink2x)
    {
        target.l.w_orig = int(w);
        target.l.h_orig = int(h);
        w /= 2;
        h /= 2;
    }

    bool wLimitExcited = m_maxTextureWidth > 0 && w > Uint32(m_maxTextureWidth);
    bool hLimitExcited = m_maxTextureHeight > 0 && h > Uint32(m_maxTextureHeight);

    if(wLimitExcited || hLimitExcited || shrink2x)
    {
        if(!shrink2x)
        {
            target.l.w_orig = int(w);
            target.l.h_orig = int(h);
        }

        // WORKAROUND: down-scale too big textures
        if(wLimitExcited)
            w = Uint32(m_maxTextureWidth);
        if(hLimitExcited)
            h = Uint32(m_maxTextureHeight);

        if(wLimitExcited || hLimitExcited)
        {
            pLogWarning("Texture is too big for a given hardware limit (%dx%d). "
                        "Shrinking texture to %dx%d, quality may be distorted!",
                        m_maxTextureWidth, m_maxTextureHeight,
                        w, h);
        }

        FIBITMAP *d = FreeImage_Rescale(sourceImage, int(w), int(h), FILTER_BOX);
        if(d)
        {
            GraphicsHelps::closeImage(sourceImage);
            sourceImage = d;
        }

        target.l.w_scale = float(w) / float(target.l.w_orig);
        target.l.h_scale = float(h) / float(target.l.h_orig);
        pitch = FreeImage_GetPitch(d);

        if(maskImage)
        {
            d = FreeImage_Rescale(maskImage, int(w), int(h), FILTER_BOX);
            if(d)
            {
                GraphicsHelps::closeImage(maskImage);
                maskImage = d;
            }
        }
    }

    uint8_t *textura = reinterpret_cast<uint8_t *>(FreeImage_GetBits(sourceImage));

    g_render->loadTexture(target, w, h, textura, pitch);
    if(maskImage || !GraphicsHelps::validateForDepthTest(sourceImage, StdPictureGetOrigPath(target)))
        target.d.invalidateDepthTest();

    GraphicsHelps::closeImage(sourceImage);

    if(maskImage)
    {
        uint32_t w_mask = static_cast<uint32_t>(FreeImage_GetWidth(maskImage));
        uint32_t h_mask = static_cast<uint32_t>(FreeImage_GetHeight(maskImage));
        uint32_t pitch_mask = static_cast<uint32_t>(FreeImage_GetPitch(maskImage));

        textura = reinterpret_cast<uint8_t *>(FreeImage_GetBits(maskImage));

        g_render->loadTextureMask(target, w_mask, h_mask, textura, pitch_mask, w, h);

        GraphicsHelps::closeImage(maskImage);
    }
}

void AbstractRender_t::lazyUnLoad(StdPicture &target)
{
    if(!target.inited || !target.l.lazyLoaded || !target.d.hasTexture())
        return;
    XRender::deleteTexture(target, true);
}

void AbstractRender_t::lazyPreLoad(StdPicture &target)
{
    if(!target.d.hasTexture() && target.l.lazyLoaded)
        lazyLoad(target);
}

size_t AbstractRender_t::lazyLoadedBytes()
{
    return m_lazyLoadedBytes;
}

void AbstractRender_t::lazyLoadedBytesReset()
{
    m_lazyLoadedBytes = 0;
}


#ifdef USE_RENDER_BLOCKING
bool AbstractRender_t::renderBlocked()
{
    return m_blockRender;
}

void AbstractRender_t::setBlockRender(bool b)
{
    m_blockRender = b;
}
#endif // USE_RENDER_BLOCKING


#ifdef USE_DRAW_BATTERY_STATUS
void AbstractRender_t::drawBatteryStatus()
{
#ifdef USE_SDL_POWER
    int secs, pct, status;
    // Battery status
    int bw = 40;
    int bh = 22;
    int bx = ScreenW - (bw + 8);
    int by = 24;
    int segmentsFullLen = 14;
    int segments = 0;
    float alhpa = 0.7f;
    float alhpaB = 0.8f;
    float r = 0.4f, g = 0.4f, b = 0.4f;
    float br = 0.0f, bg = 0.0f, bb = 0.0f;
    bool isLow = false;

#ifndef RENDER_FULLSCREEN_ALWAYS
    const bool isFullScreen = resChanged;
#endif

    if(g_videoSettings.batteryStatus == BATTERY_STATUS_OFF)
        return;

    status = SDL_GetPowerInfo(&secs, &pct);

    if(status == SDL_POWERSTATE_NO_BATTERY || status == SDL_POWERSTATE_UNKNOWN)
        return;

    isLow = (pct <= 35);

    if(status == SDL_POWERSTATE_CHARGED)
    {
        br = 0.f;
        bg = 1.f;
        bb = 0.f;
    }
    else if(status == SDL_POWERSTATE_CHARGING)
    {
        br = 1.f;
        bg = 0.64f;
        bb = 0.f;
    }
    else if(isLow)
        br = 1.f;

    segments = ((pct * segmentsFullLen) / 100) * 2;
    if(segments == 0)
        segments = 2;

    bool showBattery = false;

    showBattery |= (g_videoSettings.batteryStatus == BATTERY_STATUS_ALWAYS_ON);
    showBattery |= (g_videoSettings.batteryStatus == BATTERY_STATUS_ANY_WHEN_LOW && isLow);
#ifndef RENDER_FULLSCREEN_ALWAYS
    showBattery |= (g_videoSettings.batteryStatus == BATTERY_STATUS_FULLSCREEN_WHEN_LOW && isLow && isFullScreen);
    showBattery |= (g_videoSettings.batteryStatus == BATTERY_STATUS_FULLSCREEN_ON && isFullScreen);
#else
    showBattery |= (g_videoSettings.batteryStatus == BATTERY_STATUS_FULLSCREEN_WHEN_LOW && isLow);
    showBattery |= (g_videoSettings.batteryStatus == BATTERY_STATUS_FULLSCREEN_ON);
#endif

    if(showBattery)
    {
        XRender::setTargetTexture();

        XRender::offsetViewportIgnore(true);
        XRender::renderRect(bx, by, bw - 4, bh, 0.f, 0.f, 0.f, alhpa, true);//Edge
        XRender::renderRect(bx + 2, by + 2, bw - 8, bh - 4, r, g, b, alhpa, true);//Box
        XRender::renderRect(bx + 36, by + 6, 4, 10, 0.f, 0.f, 0.f, alhpa, true);//Edge
        XRender::renderRect(bx + 34, by + 8, 4, 6, r, g, b, alhpa, true);//Box
        XRender::renderRect(bx + 4, by + 4, segments, 14, br, bg, bb, alhpaB / 2.f, true);//Level
        XRender::offsetViewportIgnore(false);

        XRender::setTargetScreen();
    }
#endif
}
#endif // USE_DRAW_BATTERY_STATUS


/* --------------- Screenshots and GIF recording (not for Emscripten!) ----------------- */
#ifdef USE_SCREENSHOTS_AND_RECS


static std::string shoot_getTimedString(const std::string &path, const char *ext = "png")
{
    auto now = std::chrono::system_clock::now();
    std::time_t in_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm t = fmt::localtime_ne(in_time_t);
    static int prevSec = 0;
    static int prevSecCounter = 0;

    if(prevSec != t.tm_sec)
    {
        prevSec = t.tm_sec;
        prevSecCounter = 0;
    }
    else
        prevSecCounter++;

    if(!prevSecCounter)
    {
        return fmt::sprintf_ne("%sScr_%04d-%02d-%02d_%02d-%02d-%02d.%s",
                               path,
                               (1900 + t.tm_year), (1 + t.tm_mon), t.tm_mday,
                               t.tm_hour, t.tm_min, t.tm_sec,
                               ext);
    }
    else
    {
        return fmt::sprintf_ne("%sScr_%04d-%02d-%02d_%02d-%02d-%02d_(%d).%s",
                               path,
                               (1900 + t.tm_year), (1 + t.tm_mon), t.tm_mday,
                               t.tm_hour, t.tm_min, t.tm_sec,
                               prevSecCounter,
                               ext);
    }
}



void AbstractRender_t::makeShot()
{
    if(!XRender::isWorking())
        return;

    const int w = ScaleWidth, h = ScaleHeight;
    uint8_t *pixels = new uint8_t[size_t(4 * w * h)];
    g_render->getScreenPixelsRGBA(0, 0, w, h, pixels);
    PGE_GL_shoot *shoot = new PGE_GL_shoot();
    shoot->pixels = pixels;
    shoot->w = w;
    shoot->h = h;
    shoot->pitch = w * 4;

#ifndef PGE_NO_THREADING
    s_screenshot_thread = SDL_CreateThread(makeShot_action, "scrn_maker", reinterpret_cast<void *>(shoot));
    SDL_DetachThread(s_screenshot_thread);
#else
    makeShot_action(reinterpret_cast<void *>(shoot));
#endif
}

static int makeShot_action(void *_pixels)
{
    PGE_GL_shoot *shoot = reinterpret_cast<PGE_GL_shoot *>(_pixels);
//    AbstractRender_t *me = shoot->me;
    FIBITMAP *shotImg = FreeImage_AllocateT(FIT_BITMAP, shoot->w, shoot->h, 32);

    if(!shotImg)
    {
        delete []shoot->pixels;
        shoot->pixels = nullptr;
        delete shoot;
        s_screenshot_thread = nullptr;
        return 0;
    }

    uint8_t *px = shoot->pixels;
    unsigned w = unsigned(shoot->w), x = 0;
    unsigned h = unsigned(shoot->h), y = 0;
    RGBQUAD p;

    for(y = 0; y < h; ++y)
    {
        for(x = 0; x < w; ++x)
        {
#if defined(THEXTECH_BIG_ENDIAN)
            p.rgbRed = px[3];
            p.rgbGreen = px[2];
            p.rgbBlue = px[1];
            p.rgbReserved = px[0];
#else
            p.rgbRed = px[0];
            p.rgbGreen = px[1];
            p.rgbBlue = px[2];
            p.rgbReserved = px[3];
#endif
            FreeImage_SetPixelColor(shotImg, x, (h - 1) - y, &p);
            px += 4;
        }
    }

    auto outDir = AppPathManager::screenshotsDir();

    if(!DirMan::exists(outDir))
        DirMan::mkAbsPath(outDir);

    std::string saveTo = shoot_getTimedString(outDir, "png");
    pLogDebug("%s %d %d", saveTo.c_str(), shoot->w, shoot->h);

    if(FreeImage_HasPixels(shotImg) == FALSE)
        pLogWarning("Can't save screenshot: no pixel data!");
    else
    {
        BOOL ret = FreeImage_Save(FIF_PNG, shotImg, saveTo.data(), PNG_Z_BEST_COMPRESSION);
        if(!ret)
        {
            pLogWarning("Failed to save screenshot!");
            Files::deleteFile(saveTo);
        }
    }

    FreeImage_Unload(shotImg);
    delete []shoot->pixels;
    shoot->pixels = nullptr;
    delete shoot;

    s_screenshot_thread = nullptr;
    return 0;
}

bool AbstractRender_t::recordInProcess()
{
    return m_gif->enabled;
}

void AbstractRender_t::toggleGifRecorder()
{
    UNUSED(GIF_H::GifOverwriteLastDelay);// shut up a warning about unused function

    if(!m_gif->enabled)
    {
        auto outDir = AppPathManager::gifRecordsDir();

        if(!DirMan::exists(outDir))
            DirMan::mkAbsPath(outDir);

        std::string saveTo = shoot_getTimedString(outDir, "gif");

        if(m_gif->worker)
            SDL_WaitThread(m_gif->worker, nullptr);
        m_gif->worker = nullptr;

        FILE *gifFile = Files::utf8_fopen(saveTo.data(), "wb");
        if(GIF_H::GifBegin(&m_gif->writer, gifFile, ScreenW, ScreenH, m_gif->delay, false))
        {
            m_gif->enabled = true;
            m_gif->doFinalize = false;
            PlaySoundMenu(SFX_PlayerGrow);
        }

        m_gif->worker = SDL_CreateThread(processRecorder_action, "gif_recorder", reinterpret_cast<void *>(m_gif));
    }
    else
    {
        if(!m_gif->doFinalize)
        {
            m_gif->doFinalize = true;
            SDL_DetachThread(m_gif->worker);
            m_gif->worker = nullptr;
            PlaySoundMenu(SFX_PlayerShrink);
        }
        else
        {
            PlaySoundMenu(SFX_BlockHit);
        }
    }
}

void AbstractRender_t::processRecorder()
{
    if(!m_gif->enabled)
        return;

    XRender::setTargetTexture();

    m_gif->delayTimer += int(1000.0 / 65.0);

    if(m_gif->delayTimer >= m_gif->delay * 10)
        m_gif->delayTimer = 0.0;

    if(m_gif->doFinalize || (m_gif->delayTimer != 0.0))
    {
        m_gif->drawRecCircle();
        XRender::setTargetScreen();
        return;
    }

    const int w = ScreenW, h = ScreenH;

    uint8_t *pixels = reinterpret_cast<uint8_t*>(SDL_malloc(size_t(4 * w * h) + 42));
    if(!pixels)
    {
        pLogCritical("Can't allocate memory for a next GIF frame: out of memory");
        XRender::setTargetScreen();
        return; // Drop frame (out of memory)
    }

    g_render->getScreenPixelsRGBA(0, 0, w, h, pixels);

    PGE_GL_shoot shoot;
    shoot.pixels = pixels;
    shoot.w = w;
    shoot.h = h;
    shoot.pitch = w * 4;

    m_gif->enqueue(shoot);

    m_gif->drawRecCircle();
    XRender::setTargetScreen();
}

static int processRecorder_action(void *_recorder)
{
    GifRecorder *recorder = reinterpret_cast<GifRecorder *>(_recorder);

    while(true)
    {
        if(!recorder->hasSome()) // Wait for a next frame
        {
            if(recorder->doFinalize)
                break;
            PGE_Delay(1);
            continue;
        }

        PGE_GL_shoot sh = recorder->dequeue();
        GifWriteFrame(&recorder->writer, sh.pixels,
                      unsigned(sh.w),
                      unsigned(sh.h),
                      recorder->delay/*uint32_t((ticktime)/10.0)*/, 8, false);
        SDL_free(sh.pixels);
        sh.pixels = nullptr;
    }

    // Once GIF recorder was been disabled, finalize it
    GIF_H::GifEnd(&recorder->writer);
    recorder->worker = nullptr;
    recorder->enabled = false;

    return 0;
}

void GifRecorder::init(AbstractRender_t *self)
{
    m_self = self;
    if(!mutex)
        mutex = SDL_CreateMutex();
}

void GifRecorder::quit()
{
    if(enabled)
    {
        enabled = false;
        doFinalize = true;
        if(worker) // Let worker complete it's mad job
            SDL_WaitThread(worker, nullptr);
        worker = nullptr; // and only then, quit a thing
    }

    if(mutex)
        SDL_DestroyMutex(mutex);
    mutex = nullptr;
}

void GifRecorder::drawRecCircle()
{
    if(fadeForward)
    {
        fadeValue += 0.01f;
        if(fadeValue >= 1.0f)
        {
            fadeValue = 1.0f;
            fadeForward = !fadeForward;
        }
    }
    else
    {
        fadeValue -= 0.01f;
        if(fadeValue < 0.5f)
        {
            fadeValue = 0.5f;
            fadeForward = !fadeForward;
        }
    }

    m_self->offsetViewportIgnore(true);

    if(doFinalize)
    {
        m_self->renderCircle(50, 50, 20, 0.f, 0.6f, 0.f, fadeValue, true);
        SuperPrint("SAVING", 3, 2, 80, 0.f, 0.6f, 0.f, fadeValue);
    }
    else
    {
        m_self->renderCircle(50, 50, 20, 1.f, 0.f, 0.f, fadeValue, true);
        SuperPrint("REC", 3, 25, 80, 1.f, 0.f, 0.f, fadeValue);
    }

    m_self->offsetViewportIgnore(false);
}

bool GifRecorder::hasSome()
{
    SDL_LockMutex(mutex);
    bool ret = !queue.empty();
    SDL_UnlockMutex(mutex);
    return ret;
}

void GifRecorder::enqueue(const PGE_GL_shoot &entry)
{
    SDL_LockMutex(mutex);
    queue.push_back(entry);
    SDL_UnlockMutex(mutex);
}

PGE_GL_shoot GifRecorder::dequeue()
{
    SDL_LockMutex(mutex);
    PGE_GL_shoot ret = queue.front();
    queue.pop_front();
    SDL_UnlockMutex(mutex);
    return ret;
}
#endif // USE_SCREENSHOTS_AND_RECS
