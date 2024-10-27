/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2024 Vitaly Novichkov <admin@wohlnet.ru>
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

#include <IniProcessor/ini_processing.h>
#include <AppPath/app_path.h>
#include <Logger/logger.h>
#include <Graphics/graphics_funcs.h>
#include <Utils/elapsed_timer.h>
#include <DirManager/dirman.h>
#include <Utils/files.h>
#include <Utils/files_ini.h>
#include <fmt_time_ne.h>
#include <fmt_format_ne.h>

#include <chrono>

#include "core/base/render_base.h"
#include "core/render.h"

#include "main/cheat_code.h"

#include "config.h"
#include "globals.h"
#include "sound.h"
#include "graphics.h"

#ifdef PGE_ENABLE_VIDEO_REC
#   include <SDL2/SDL_mixer_ext.h>
#   include "sdl_proxy/sdl_timer.h"
#   include "pge_video_rec/pge_video_rec.h"
#endif


int XRender::TargetW = 800;
int XRender::TargetH = 600;
bool XRender::g_BitmaskTexturePresent = false;

static const unsigned char blank_gif[] = "GIF89a\x01\x00\x01\x00\x80\x00\x00\xff\xff\xff\x00\x00\x00!\xf9\x04\x01\x00\x00\x00\x00,\x00\x00\x00\x00\x01\x00\x01\x00\x00\x02\2D\x01\x00;";

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


struct PGE_GL_shoot
{
    uint8_t *pixels = nullptr;
    int pitch = 0;
    int w = 0, h = 0;
};

#endif // USE_SCREENSHOTS_AND_RECS

#ifdef PGE_ENABLE_VIDEO_REC

static int processRecorder_action(void *_recorder);

GifRecorder *AbstractRender_t::m_gif = nullptr;

struct GifRecorder
{
    AbstractRender_t *m_self = nullptr;

    // used to lock recording and worker.
    // mutex not held for reads from main thread after confirming recording is active,
    // because any write processes are initiated from main thread
    SDL_mutex* mutex = nullptr;
    std::unique_ptr<PGE_VideoRecording> recording;
    SDL_Thread       *worker = nullptr;

    uint32_t    delay       = 4;
    uint32_t    delayTimer  = 0;
    bool        fadeForward = true;
    float       fadeValue = 0.5f;

    void init(AbstractRender_t *self);
    void quit();

    void drawRecCircle(bool saving, int frame_count);
};

#endif // PGE_ENABLE_VIDEO_REC




AbstractRender_t::AbstractRender_t()
{
#ifdef PGE_ENABLE_VIDEO_REC
    m_gif = new GifRecorder();
#endif
}

AbstractRender_t::~AbstractRender_t()
{
#ifdef PGE_ENABLE_VIDEO_REC
    delete m_gif;
    m_gif = nullptr;
#endif
}

bool AbstractRender_t::init()
{
    ScaleWidth = XRender::TargetW;
    ScaleHeight = XRender::TargetH;

#ifdef PGE_ENABLE_VIDEO_REC
    m_gif->init(this);
#endif
    return true;
}

void AbstractRender_t::close()
{
#ifdef PGE_ENABLE_VIDEO_REC
    m_gif->quit();
#endif
}

void AbstractRender_t::dumpFullFile(std::vector<char> &dst, const std::string &path)
{
    dst.clear();
    SDL_RWops *f;

    f = Files::open_file(path, "rb");
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

void AbstractRender_t::loadTextureMask(StdPicture &target,
                         uint32_t mask_width,
                         uint32_t mask_height,
                         uint8_t *RGBApixels,
                         uint32_t pitch,
                         uint32_t image_width,
                         uint32_t image_height)
{
    /* unimplemented */

    UNUSED(target);
    UNUSED(mask_width);
    UNUSED(mask_height);
    UNUSED(RGBApixels);
    UNUSED(pitch);
    UNUSED(image_width);
    UNUSED(image_height);
}

void AbstractRender_t::compileShaders(StdPicture &target)
{
    UNUSED(target);

    /* unimplemented */
}

bool AbstractRender_t::textureMaskSupported()
{
    return false;
}

bool AbstractRender_t::userShadersSupported()
{
    return false;
}

bool AbstractRender_t::depthTestSupported()
{
    return false;
}

void AbstractRender_t::lazyLoadPicture(StdPicture_Sub& target,
                                       const std::string &path,
                                       int scaleFactor,
                                       const std::string &maskPath,
                                       const std::string &maskFallbackPath)
{
    PGE_Size tSize;
    bool useMask = true;

    if(!GameIsActive)
        return; // do nothing when game is closed

    if(path.empty())
        return;

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
        return;
    }

    target.w = tSize.w() * scaleFactor;
    target.h = tSize.h() * scaleFactor;

    target.l.raw = Files::load_file(path);

    //Apply Alpha mask
    if(useMask && !maskPath.empty() && Files::fileExists(maskPath))
    {
        target.l.rawMask = Files::load_file(maskPath);
        target.l.isMaskPng = false; //-V1048
    }
    else if(useMask && !maskFallbackPath.empty())
    {
        target.l.rawMask = Files::load_file(maskFallbackPath);
        target.l.isMaskPng = true;
    }

    target.inited = true;
    target.l.lazyLoaded = true;

#ifdef THEXTECH_BUILD_GL_MODERN
    // load fragment shader if it exists
    if(Files::fileExists(path + ".frag"))
    {
        pLogDebug("Loading user shader [%s%s]...", path.c_str(), ".frag");
        target.l.fragmentShaderSource = Files::load_file(path + ".frag");
    }

    // load lighting info if it exists
    if(Files::fileExists(path + ".ini"))
    {
        IniProcessing ini = Files::load_ini(path + ".ini");
        std::string temp;

        for(const std::string& group : ini.childGroups())
        {
            GLLight temp_light;

            if(SDL_strncasecmp(group.c_str(), "light", 5) != 0)
                continue;

            ini.beginGroup(group);
            ini.read("type", temp, "");

            // load coordinates
            if(SDL_strcasecmp(temp.c_str(), "point") == 0)
            {
                temp_light.type = GLLightType::point;
                ini.read("x", temp_light.pos[0], target.w / 2);
                ini.read("y", temp_light.pos[1], target.h / 2);
            }
            else if(SDL_strcasecmp(temp.c_str(), "box") == 0)
            {
                temp_light.type = GLLightType::box;
                ini.read("left", temp_light.pos[0], 0);
                ini.read("top", temp_light.pos[1], 0);
                ini.read("right", temp_light.pos[2], target.w);
                ini.read("bottom", temp_light.pos[3], target.h);
            }
            else if(SDL_strcasecmp(temp.c_str(), "bar") == 0)
            {
                temp_light.type = GLLightType::bar;
                ini.read("x1", temp_light.pos[0], 0);
                ini.read("y1", temp_light.pos[1], 0);
                ini.read("x2", temp_light.pos[2], target.w);
                ini.read("y2", temp_light.pos[3], target.h);
            }
            else if(SDL_strcasecmp(temp.c_str(), "arc") == 0)
            {
                temp_light.type = GLLightType::arc;
                ini.read("x", temp_light.pos[0], target.w / 2);
                ini.read("y", temp_light.pos[1], target.h / 2);
                ini.read("angle", temp_light.pos[2], 270.0f);
                ini.read("spread", temp_light.pos[3], 120.0f);
                // convert from degrees to radians
                temp_light.pos[2] *= float(M_PI / 180.0);
                temp_light.pos[3] *= float(M_PI / 180.0);
            }
            else
            {
                ini.endGroup();
                continue;
            }

            ini.read("radius", temp_light.radius, 32);

            unsigned int temp_byte;
            ini.read("r", temp_byte, 64);
            temp_light.color.r = uint8_t(temp_byte);
            ini.read("g", temp_byte, 64);
            temp_light.color.g = uint8_t(temp_byte);
            ini.read("b", temp_byte, 64);
            temp_light.color.b = uint8_t(temp_byte);

            ini.endGroup();

            GLPictureLightInfoPtr temp = std::move(target.l.light_info);
            target.l.light_info.init();
            target.l.light_info->light = temp_light;
            target.l.light_info->next = std::move(temp);
        }
    }
#endif
}

void AbstractRender_t::LoadPictureShader(StdPicture& target, const std::string &path)
{
    if(!GameIsActive)
        return; // do nothing when game is closed

    if(path.empty())
        return;

#ifdef DEBUG_BUILD
    target.origPath = path;
#endif

    target.reset();

#ifdef THEXTECH_BUILD_GL_MODERN
    // load fragment shader if it exists
    if(Files::fileExists(path))
    {
        target.w = 1;
        target.h = 1;

        target.inited = true;
        target.l.lazyLoaded = true;

        // blank GIF of 1 pixel
        target.l.raw.init_from_mem(blank_gif, sizeof(blank_gif) - 1);

        pLogDebug("Loading user shader [%s]...", path.c_str());
        target.l.fragmentShaderSource = Files::load_file(path);

        // eagerly compile it to minimize stutter
        g_render->compileShaders(target);
    }
#endif
}

void AbstractRender_t::LoadPictureParticleSystem(StdPicture& target, const std::string &vertexPath, const std::string& fragPath, const std::string& imagePath)
{
    if(!GameIsActive)
        return; // do nothing when game is closed

    if(vertexPath.empty())
        return;

#ifdef DEBUG_BUILD
    target.origPath = vertexPath;
#endif

    target.reset();

#ifdef THEXTECH_BUILD_GL_MODERN
    bool valid = Files::fileExists(vertexPath)
        && (fragPath.empty() || Files::fileExists(fragPath))
        && (imagePath.empty() || Files::fileExists(imagePath));

    // load fragment shader if it exists
    if(valid)
    {
        if(!imagePath.empty())
        {
            lazyLoadPicture(target, imagePath);
        }
        else
        {
            target.w = 1;
            target.h = 1;

            target.inited = true;
            target.l.lazyLoaded = true;

            // blank GIF of 1 pixel
            target.l.raw.init_from_mem(blank_gif, sizeof(blank_gif) - 1);
        }

        pLogDebug("Loading particle system vertex shader [%s]...", vertexPath.c_str());
        target.l.particleVertexShaderSource = Files::load_file(vertexPath);

        if(!fragPath.empty())
        {
            pLogDebug("Loading particle system fragment shader [%s]...", fragPath.c_str());
            target.l.fragmentShaderSource = Files::load_file(fragPath);
        }

        // eagerly compile it to minimize stutter
        g_render->compileShaders(target);

        // immediately claim the first two uniform spots (so they can have constant engine-level locations)
        g_render->registerUniform(target, "u_particle_z");
        g_render->registerUniform(target, "u_camera_pos");
    }
#else
    UNUSED(fragPath);
    UNUSED(imagePath);
#endif
}

void AbstractRender_t::setTransparentColor(StdPicture& target, uint32_t rgb)
{
    target.l.colorKey = true;
    target.l.keyRgb[0] = (rgb >> 0) & 0xFF;
    target.l.keyRgb[1] = (rgb >> 8) & 0xFF;
    target.l.keyRgb[2] = (rgb >> 16) & 0xFF;
}

void AbstractRender_t::loadTexture(StdPicture &target, uint32_t width, uint32_t height, uint8_t *RGBApixels, uint32_t pitch)
{
    // validate for depth test before loading
    if(depthTestSupported())
    {
        // check whether there are any transparent pixels
        for(uint32_t y = 0; y < height; ++y)
        {
            for(uint32_t x = 0; x < width; ++x)
            {
                uint8_t *alpha = RGBApixels + (y * pitch) + (x * 4) + 3;

                // vanilla game used 5 bits per channel, so we set the alpha test as >= 0x08
                if(*alpha < 0x08 || *alpha >= 0xf8)
                    continue;

                D_pLogDebugNA("Externally loaded texture CANNOT use depth test");

                target.d.invalidateDepthTest();

                y = height - 1;
                break;
            }
        }
    }
    else
    {
        target.d.invalidateDepthTest();
    }

    loadTextureInternal(target, width, height, RGBApixels, pitch, 0, 0);
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

        if(!maskImage)
            pLogWarning("lazyLoad: failed to load mask image for texture at address %p [%s]", &target, StdPictureGetOrigPath(target).c_str());
    }

    // check if bitmask required / possible and possibly merge
    if(maskImage)
    {
        // check if bitmask cannot be properly represented with RGBA
        bool bitmask_required = GraphicsHelps::validateBitmaskRequired(sourceImage, maskImage, StdPictureGetOrigPath(target));

        XRender::g_BitmaskTexturePresent |= bitmask_required;

        // merge it with image masks are unsupported, merge is forced, or the mask could be properly represented with RGBA
        if(maskImage && (g_ForceBitmaskMerge || !g_render->textureMaskSupported() || !bitmask_required))
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
    target.ColorUpper.r = upperColor.rgbRed;
    target.ColorUpper.b = upperColor.rgbBlue;
    target.ColorUpper.g = upperColor.rgbGreen;
    target.ColorUpper.a = 255;

    RGBQUAD lowerColor;
    FreeImage_GetPixelColor(sourceImage, 0, 0, &lowerColor);
    target.ColorLower.r = lowerColor.rgbRed;
    target.ColorLower.b = lowerColor.rgbBlue;
    target.ColorLower.g = lowerColor.rgbGreen;
    target.ColorLower.a = 255;

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

    // don't touch texture info, that was set on original load
    // target.w = static_cast<int>(w);
    // target.h = static_cast<int>(h);

    bool shrink2x;
    switch(g_config.scale_down_textures)
    {
    case Config_t::SCALE_DOWN_ALL:
        // only do it if the texture isn't already downscaled
        shrink2x = (w >= Uint32(target.w) && h >= Uint32(target.h));
        break;
    case Config_t::SCALE_DOWN_SAFE:
        shrink2x = GraphicsHelps::validateFor2xScaleDown(sourceImage, StdPictureGetOrigPath(target));
        if(maskImage)
            shrink2x &= GraphicsHelps::validateFor2xScaleDown(maskImage, StdPictureGetOrigPath(target));
        break;
    case Config_t::SCALE_DOWN_NONE:
    default:
        shrink2x = false;
        break;
    }

    if(shrink2x)
    {
        w /= 2;
        h /= 2;
    }

    bool wLimitExcited = m_maxTextureWidth > 0 && w > Uint32(m_maxTextureWidth);
    bool hLimitExcited = m_maxTextureHeight > 0 && h > Uint32(m_maxTextureHeight);

    if(wLimitExcited || hLimitExcited || shrink2x)
    {
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

        FIBITMAP *d = (wLimitExcited || hLimitExcited) ? FreeImage_Rescale(sourceImage, int(w), int(h), FILTER_BOX) : GraphicsHelps::fast2xScaleDown(sourceImage);
        if(d)
        {
            GraphicsHelps::closeImage(sourceImage);
            sourceImage = d;
            pitch = FreeImage_GetPitch(d);
        }

        if(maskImage)
        {
            d = (wLimitExcited || hLimitExcited) ? FreeImage_Rescale(maskImage, int(w), int(h), FILTER_BOX) : GraphicsHelps::fast2xScaleDown(maskImage);
            if(d)
            {
                GraphicsHelps::closeImage(maskImage);
                maskImage = d;
            }
        }
    }

    if(!g_render->depthTestSupported() || maskImage || !GraphicsHelps::validateForDepthTest(sourceImage, StdPictureGetOrigPath(target)))
        target.d.invalidateDepthTest();

    uint32_t w_mask = 0;
    uint32_t h_mask = 0;

    if(maskImage)
    {
        w_mask = static_cast<uint32_t>(FreeImage_GetWidth(maskImage));
        h_mask = static_cast<uint32_t>(FreeImage_GetHeight(maskImage));
        uint32_t pitch_mask = static_cast<uint32_t>(FreeImage_GetPitch(maskImage));

        uint8_t* textura = reinterpret_cast<uint8_t *>(FreeImage_GetBits(maskImage));

        g_render->loadTextureMask(target, w_mask, h_mask, textura, pitch_mask, w, h);

        GraphicsHelps::closeImage(maskImage);
    }

    uint8_t *textura = reinterpret_cast<uint8_t *>(FreeImage_GetBits(sourceImage));

    g_render->loadTextureInternal(target, w, h, textura, pitch, w_mask, h_mask);

    GraphicsHelps::closeImage(sourceImage);

#ifdef THEXTECH_BUILD_GL_MODERN
    if(g_render->userShadersSupported() && (!target.l.particleVertexShaderSource.empty() || !target.l.fragmentShaderSource.empty()))
        g_render->compileShaders(target);
#endif
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

int AbstractRender_t::registerUniform(StdPicture &target, const char* name)
{
#ifdef THEXTECH_BUILD_GL_MODERN
    auto it = std::find(target.l.registeredUniforms.begin(), target.l.registeredUniforms.end(), name);

    if(it == target.l.registeredUniforms.end())
    {
        target.l.registeredUniforms.push_back(name);
        target.l.finalUniformState.push_back(UniformValue_t(0.0f));

        return (int)target.l.registeredUniforms.size() - 1;
    }

    return it - target.l.registeredUniforms.begin();

#else
    UNUSED(target);
    UNUSED(name);

    return -1;
#endif
}

void AbstractRender_t::assignUniform(StdPicture &target, int index, const UniformValue_t& value)
{
#ifdef THEXTECH_BUILD_GL_MODERN
    if(index >= 0 && index < (int)target.l.finalUniformState.size())
        target.l.finalUniformState[index] = value;
#else
    UNUSED(target);
    UNUSED(index);
    UNUSED(value);
#endif
}

void AbstractRender_t::spawnParticle(StdPicture &target, double worldX, double worldY, ParticleVertexAttrs_t attrs)
{
    // no-op

    UNUSED(target);
    UNUSED(worldX);
    UNUSED(worldY);
    UNUSED(attrs);
}

#ifdef THEXTECH_BUILD_GL_MODERN
void AbstractRender_t::addLight(const GLLight &light)
{
    // no-op

    UNUSED(light);
}

void AbstractRender_t::setupLighting(const GLLightSystem &system)
{
    // no-op

    UNUSED(system);
}

void AbstractRender_t::renderLighting()
{
    // no-op
}
#endif

void AbstractRender_t::renderParticleSystem(StdPicture &tx, double camX, double camY)
{
    // no-op

    UNUSED(tx);
    UNUSED(camX);
    UNUSED(camY);
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

#endif // USE_SCREENSHOTS_AND_RECS

#ifdef PGE_ENABLE_VIDEO_REC

bool AbstractRender_t::recordInProcess()
{
    SDL_LockMutex(m_gif->mutex);
    bool ret = m_gif->recording.get();
    SDL_UnlockMutex(m_gif->mutex);

    return ret;
}

void SDLCALL store_audio_chunk(void* _recorder, uint8_t* stream, int len)
{
    GifRecorder *recorder = reinterpret_cast<GifRecorder *>(_recorder);

    PGE_AudioChunk ch;
    ch.audio_buffer.resize(len);
    memcpy((uint8_t*)ch.audio_buffer.data(), stream, len & ~1);

    SDL_LockMutex(recorder->mutex);

    // this condition is extremely likely but should be confirmed before enqueueing
    if(recorder->recording && !recorder->recording->exit_requested && recorder->recording->spec.audio_enabled)
        recorder->recording->enqueue_audio(std::move(ch));

    SDL_UnlockMutex(recorder->mutex);
}

void AbstractRender_t::toggleGifRecorder()
{
    SDL_LockMutex(m_gif->mutex);
    bool recording_present = m_gif->recording.get();
    bool recording_active = m_gif->recording && !m_gif->recording->exit_requested;

    // this should never happen
    if(!recording_present && m_gif->worker)
    {
        SDL_DetachThread(m_gif->worker);
        m_gif->worker = nullptr;
    }

    SDL_UnlockMutex(m_gif->mutex);

    if(!recording_present)
    {
        auto outDir = AppPathManager::gifRecordsDir();

        if(!DirMan::exists(outDir))
            DirMan::mkAbsPath(outDir);

        PGE_VideoSpec spec;
        spec.frame_w = XRender::TargetW;
        spec.frame_h = XRender::TargetH;
        spec.frame_pitch = XRender::TargetW * 4;

        std::unique_ptr<PGE_VideoRecording> recording;

#ifdef PGE_VIDEO_REC_WEBM_SUPPORTED
        if(g_config.webm_recording)
        {
            spec.frame_rate = 120;
            spec.video_quality = 10;
            spec.audio_enabled = g_config.audio_enable;
            spec.audio_sample_rate = g_config.audio_sample_rate.obtained;
            spec.audio_channel_count = g_config.audio_channels.obtained;
            spec.audio_sample_format = g_config.audio_format.obtained;
            recording = PGE_new_recording_VP8(spec);
        }
        else
#endif
        {
            spec.frame_rate = 25;
            recording = PGE_new_recording_GIF(spec);
        }

        std::string saveTo;

        if(recording)
            saveTo = shoot_getTimedString(outDir, recording->extension());

        if(recording && recording->initialize(saveTo.c_str()))
        {
            SDL_LockMutex(m_gif->mutex);
            m_gif->recording = std::move(recording);
            m_gif->worker = SDL_CreateThread(processRecorder_action, "gif_recorder", reinterpret_cast<void *>(m_gif));
            SDL_UnlockMutex(m_gif->mutex);

            if(m_gif->recording->spec.audio_enabled)
                Mix_SetPostMix(store_audio_chunk, reinterpret_cast<void *>(m_gif));
            else
                PlaySoundMenu(SFX_PlayerGrow);
        }
    }
    else if(recording_active)
    {
        Mix_SetPostMix(nullptr, nullptr);

        PGE_VideoFrame final_timestamp;
        final_timestamp.timestamp = SDL_GetMicroTicks();
        final_timestamp.end_frame = true;

        m_gif->recording->enqueue_frame(std::move(final_timestamp), -1);
        m_gif->recording->exit_requested = true;

        if(m_gif->recording->spec.audio_enabled)
            PlaySoundMenu(SFX_GotItem);
        else
            PlaySoundMenu(SFX_PlayerShrink);
    }
    else
    {
        PlaySoundMenu(SFX_BlockHit);
    }
}

void AbstractRender_t::processRecorder()
{
    SDL_LockMutex(m_gif->mutex);
    bool recording_present = m_gif->recording.get();
    bool recording_active = m_gif->recording && !m_gif->recording->exit_requested;
    SDL_UnlockMutex(m_gif->mutex);

    if(!recording_present)
        return;

    XRender::setTargetTexture();

    m_gif->delayTimer += int(1000.0 / 65.0);

    if(m_gif->recording->spec.audio_enabled || int(m_gif->delayTimer) >= 1000 / m_gif->recording->spec.frame_rate)
        m_gif->delayTimer = 0.0;

    if(!recording_active || (m_gif->delayTimer != 0.0))
    {
        int frame_count = m_gif->recording->frame_backlog();
        m_gif->drawRecCircle(!recording_active, frame_count);
        XRender::setTargetScreen();
        return;
    }

    const int w = XRender::TargetW, h = XRender::TargetH;

    PGE_VideoFrame shoot;
    shoot.pixels.resize(4 * w * h);
    shoot.timestamp = SDL_GetMicroTicks();

    g_render->getScreenPixelsRGBA(0, 0, w, h, shoot.pixels.data());

    int frame_count = m_gif->recording->enqueue_frame(std::move(shoot), 65);

    m_gif->drawRecCircle(!recording_active, frame_count);
    XRender::setTargetScreen();
}

static int processRecorder_action(void *_recorder)
{
    GifRecorder *recorder = reinterpret_cast<GifRecorder *>(_recorder);

    int ret = -1;

    if(recorder->recording)
        ret = recorder->recording->encoding_thread();

    SDL_LockMutex(recorder->mutex);

    recorder->recording.reset();
    if(recorder->worker)
        SDL_DetachThread(recorder->worker);
    recorder->worker = nullptr;

    SDL_UnlockMutex(recorder->mutex);

    return ret;
}

void GifRecorder::init(AbstractRender_t *self)
{
    m_self = self;
    if(!mutex)
        mutex = SDL_CreateMutex();
}

void GifRecorder::quit()
{
    SDL_Thread* worker_to_kill = nullptr;

    SDL_LockMutex(mutex);
    if(recording)
    {
        recording->exit_requested = true;
        worker_to_kill = worker;
        worker = nullptr;
    }
    SDL_UnlockMutex(mutex);

    if(worker_to_kill) // Let worker complete it's mad job
        SDL_WaitThread(worker_to_kill, nullptr);

    if(mutex)
        SDL_DestroyMutex(mutex);
    mutex = nullptr;
}

void GifRecorder::drawRecCircle(bool saving, int frame_count)
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

    XTColor circ_color = (saving) ? XTColorF(0.f, 0.6f, 0.f, fadeValue) : XTColorF(1.f, 0.f, 0.f, fadeValue);
    const char* text = (saving) ? "SAVING" : "REC";
    int text_offset = (saving) ? 2 : 25;

    if(frame_count == 1)
        frame_count = 0;

    m_self->renderCircle(50, 50, 20, XTColorF(0.5f, 0.5f, 0.5f, fadeValue), true);
    m_self->renderCircle(50, 50, 20 * (65 - frame_count) / 65, circ_color, true);

    SuperPrint(text, 3, text_offset, 80, circ_color);

    m_self->offsetViewportIgnore(false);
}

#endif // PGE_ENABLE_VIDEO_REC
