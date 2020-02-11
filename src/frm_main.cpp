#include "globals.h"
#include "game_main.h"
#include "graphics.h"
#include <SDL2/SDL_mixer_ext.h>
#include <Logger/logger.h>
#include <Utils/files.h>
#include <Graphics/graphics_funcs.h>
#include <FreeImageLite.h>

#include "frm_main.h"

static SDL_bool IsFullScreen(SDL_Window *win)
{
    Uint32 flags = SDL_GetWindowFlags(win);
    return (flags & SDL_WINDOW_FULLSCREEN_DESKTOP) ? SDL_TRUE : SDL_FALSE;
}

FrmMain::FrmMain()
{
    ScaleWidth = ScreenW;
    ScaleHeight = ScreenH;
}

Uint8 FrmMain::getKeyState(SDL_Scancode key)
{
    if(m_keyboardState)
        return m_keyboardState[key];
    return 0;
}

bool FrmMain::initSDL()
{
    bool res = false;

    LoadLogSettings(false);
    //Write into log the application start event
    pLogDebug("<Application started>");

    Uint32 sdlInitFlags = 0;
    int sdlMixerInitFlags = 0;
    // Prepare flags for SDL initialization
    sdlInitFlags |= SDL_INIT_TIMER;
    sdlInitFlags |= SDL_INIT_AUDIO;
    sdlInitFlags |= SDL_INIT_VIDEO;
    sdlInitFlags |= SDL_INIT_EVENTS;
    sdlInitFlags |= SDL_INIT_JOYSTICK;
    //(Cool thing, but is not needed yet)
    //sdlInitFlags |= SDL_INIT_HAPTIC;
    sdlInitFlags |= SDL_INIT_GAMECONTROLLER;

    sdlMixerInitFlags |= MIX_INIT_FLAC;
    sdlMixerInitFlags |= MIX_INIT_MOD;
    sdlMixerInitFlags |= MIX_INIT_MP3;
    sdlMixerInitFlags |= MIX_INIT_OGG;
    sdlMixerInitFlags |= MIX_INIT_MID;

    // Initialize SDL
    res = (SDL_Init(sdlInitFlags) < 0);
    m_sdlLoaded = !res;

    // Workaround: https://discourse.libsdl.org/t/26995
    setlocale(LC_NUMERIC, "C");

    const char *error = SDL_GetError();
    if(*error != '\0')
        pLogWarning("Error while SDL Initialization: %s", error);
    SDL_ClearError();

    // Initialize SDL Mixer
    res = (Mix_Init(sdlMixerInitFlags) < 0);

    SDL_ClearError();

    SDL_GL_ResetAttributes();

    window = SDL_CreateWindow(WindowTitle.c_str(),
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                          #ifdef __EMSCRIPTEN__ //Set canvas be 1/2 size for a faster rendering
                              ScaleWidth / 2, ScaleHeight / 2,
                          #else
                              ScaleWidth, ScaleHeight,
                          #endif //__EMSCRIPTEN__
                              SDL_WINDOW_RESIZABLE |
                              SDL_WINDOW_HIDDEN |
                              SDL_WINDOW_ALLOW_HIGHDPI);

    if(window == nullptr)
    {
        pLogCritical("Unable to create window!");
        SDL_ClearError();
        return false;
    }

    if(isSdlError())
    {
        pLogCritical("Unable to create window!");
        SDL_ClearError();
        return false;
    }

#ifdef __EMSCRIPTEN__ //Set canvas be 1/2 size for a faster rendering
    SDL_SetWindowMinimumSize(window, ScaleWidth / 2, ScaleHeight / 2);
#else
    SDL_SetWindowMinimumSize(window, ScaleWidth, ScaleHeight);
#endif //__EMSCRIPTEN__

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");


#ifdef _WIN32
    FIBITMAP *img[2];
    img[0] = GraphicsHelps::loadImage(AppPath + "/graphics/common/icon/cat_16.png");
    img[1] = GraphicsHelps::loadImage(AppPath + "/graphics/common/icon/cat_32.png");

    if(!GraphicsHelps::setWindowIcon(window, img[0], 16))
    {
        pLogWarning("Unable to setup window icon!");
        SDL_ClearError();
    }

    if(!GraphicsHelps::setWindowIcon(window, img[1], 32))
    {
        pLogWarning("Unable to setup window icon!");
        SDL_ClearError();
    }

    GraphicsHelps::closeImage(img[0]);
    GraphicsHelps::closeImage(img[1]);
#else//IF _WIN32

    FIBITMAP *img;
#   ifdef __APPLE__
    img = GraphicsHelps::loadImage(AppPath + "/graphics/common/icon/cat_256.png");
#   else
    img = GraphicsHelps::loadImage(AppPath + "/graphics/common/icon/cat_32.png");
#   endif //__APPLE__

    if(img)
    {
        SDL_Surface *sIcon = GraphicsHelps::fi2sdl(img);
        SDL_SetWindowIcon(window, sIcon);
        GraphicsHelps::closeImage(img);
        SDL_FreeSurface(sIcon);

        if(isSdlError())
        {
            pLogWarning("Unable to setup window icon!");
            SDL_ClearError();
        }
    }
#endif//IF _WIN32 #else

    pLogDebug("Init renderer settings...");

    m_gRenderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if(!m_gRenderer)
    {
        pLogCritical("Unable to create renderer!");
        freeSDL();
        return false;
    }

    SDL_SetRenderDrawBlendMode(m_gRenderer, SDL_BLENDMODE_BLEND);

    m_keyboardState = SDL_GetKeyboardState(nullptr);

    updateViewport();

    frmMain.renderRect(0, 0, frmMain.ScaleWidth, frmMain.ScaleHeight, 0.f, 0.f, 0.f, 0.f, true);
    repaint();
    doEvents();

    return res;
}

void FrmMain::freeSDL()
{
    clearAllTextures();
    if(m_gRenderer)
        SDL_DestroyRenderer(m_gRenderer);
    if(window)
        SDL_DestroyWindow(window);
    SDL_Quit();
    CloseLog();
}

void FrmMain::show()
{
    SDL_ShowWindow(window);
}

void FrmMain::hide()
{
    SDL_HideWindow(window);
    ShowCursor(1);
}

void FrmMain::doEvents()
{
    while(SDL_PollEvent(&event))
    {
        switch(event.type)
        {
        case SDL_QUIT:
            ShowCursor(1);
            KillIt();
            break;
        case SDL_WINDOWEVENT:
            if((event.window.event == SDL_WINDOWEVENT_RESIZED) || (event.window.event == SDL_WINDOWEVENT_MOVED))
                eventResize();
            break;
        case SDL_KEYDOWN:
            eventKeyDown(event.key);
            eventKeyPress(event.key.keysym.sym);
            break;
        case SDL_KEYUP:
            eventKeyUp(event.key);
            break;
        case SDL_MOUSEBUTTONDOWN:
            eventMouseDown(event.button);
            break;
        case SDL_MOUSEBUTTONUP:
            eventMouseUp(event.button);
            break;
        case SDL_MOUSEMOTION:
            eventMouseMove(event.motion);
            break;
        }
    }
}

void FrmMain::eventDoubleClick()
{

}

void FrmMain::eventKeyDown(SDL_KeyboardEvent &evt)
{
    int KeyCode = evt.keysym.scancode;
    inputKey = KeyCode;
    if(KeyCode == SDL_SCANCODE_RETURN || KeyCode == SDL_SCANCODE_KP_ENTER)
        keyDownEnter = true;
    else if(KeyCode == SDL_SCANCODE_LALT || KeyCode == SDL_SCANCODE_RALT)
        keyDownAlt = true;

    if(keyDownAlt && keyDownEnter && !TestLevel)
    {
        keyDownAlt = false;
        keyDownEnter = false;
        ChangeScreen();
    }

    if(KeyCode == SDL_SCANCODE_F12)
        TakeScreen = true;
}

void FrmMain::eventKeyPress(SDL_Keycode KeyASCII)
{
    switch(KeyASCII)
    {
    case SDLK_a: CheatCode("a"); break;
    case SDLK_b: CheatCode("b"); break;
    case SDLK_c: CheatCode("c"); break;
    case SDLK_d: CheatCode("d"); break;
    case SDLK_e: CheatCode("e"); break;
    case SDLK_f: CheatCode("f"); break;
    case SDLK_g: CheatCode("g"); break;
    case SDLK_h: CheatCode("h"); break;
    case SDLK_i: CheatCode("i"); break;
    case SDLK_j: CheatCode("j"); break;
    case SDLK_k: CheatCode("k"); break;
    case SDLK_l: CheatCode("l"); break;
    case SDLK_m: CheatCode("m"); break;
    case SDLK_n: CheatCode("n"); break;
    case SDLK_o: CheatCode("o"); break;
    case SDLK_p: CheatCode("p"); break;
    case SDLK_q: CheatCode("q"); break;
    case SDLK_r: CheatCode("r"); break;
    case SDLK_s: CheatCode("s"); break;
    case SDLK_t: CheatCode("t"); break;
    case SDLK_u: CheatCode("u"); break;
    case SDLK_v: CheatCode("v"); break;
    case SDLK_w: CheatCode("w"); break;
    case SDLK_x: CheatCode("x"); break;
    case SDLK_y: CheatCode("y"); break;
    case SDLK_z: CheatCode("z"); break;
    default: CheatCode(" "); break;
    }
}

void FrmMain::eventKeyUp(SDL_KeyboardEvent &evt)
{
    int KeyCode = evt.keysym.scancode;
    if(KeyCode == SDL_SCANCODE_RETURN || KeyCode == SDL_SCANCODE_KP_ENTER)
        keyDownEnter = false;
    else if(KeyCode == SDL_SCANCODE_LALT || KeyCode == SDL_SCANCODE_RALT)
        keyDownAlt = false;
}

void FrmMain::eventMouseDown(SDL_MouseButtonEvent &event)
{
    if(event.button == SDL_BUTTON_LEFT)
    {
        MenuMouseDown = true;
        MenuMouseMove = true;
    }
    else if(event.button == SDL_BUTTON_RIGHT)
    {
        MenuMouseBack = true;
    }
}

void FrmMain::eventMouseMove(SDL_MouseMotionEvent &event)
{
    MenuMouseX = int(event.x * ScreenW / ScaleWidth);
    MenuMouseY = int(event.y * ScreenH / ScaleHeight);
    MenuMouseMove = true;
}

void FrmMain::eventMouseUp(SDL_MouseButtonEvent &)
{
    MenuMouseDown = false;
    MenuMouseRelease = true;
}

void FrmMain::eventResize()
{
    updateViewport();
    SetupScreens();
}

void FrmMain::toggleFullScreen()
{

}

int FrmMain::setFullScreen(bool fs)
{
    if(window == nullptr)
        return -1;

    if(fs != IsFullScreen(window))
    {
        if(fs)
        {
            // Swith to FULLSCREEN mode
            if(SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP) < 0)
            {
                //Hide mouse cursor in full screen mdoe
                pLogWarning("Setting fullscreen failed: %s", SDL_GetError());
                return -1;
            }

            SDL_ShowCursor(SDL_DISABLE);
            return 1;
        }
        else
        {
            // Swith to WINDOWED mode
            if(SDL_SetWindowFullscreen(window, SDL_FALSE) < 0)
            {
                pLogWarning("Setting windowed failed: %s", SDL_GetError());
                return -1;
            }

            return 0;
        }
    }

    return 0;
}

bool FrmMain::isSdlError()
{
    const char *error = SDL_GetError();
    return (*error != '\0');
}

void FrmMain::repaint()
{
    SDL_RenderPresent(m_gRenderer);
}

void FrmMain::updateViewport()
{
    float w, w1, h, h1;
    int   wi, hi;
    SDL_GetWindowSize(window, &wi, &hi);
    w = wi;
    h = hi;
    w1 = w;
    h1 = h;
    scale_x = w / ScaleWidth;
    scale_y = h / ScaleHeight;
    viewport_scale_x = scale_x;
    viewport_scale_y = scale_y;

    if(scale_x > scale_y)
    {
        w1 = scale_y * ScaleWidth;
        viewport_scale_x = w1 / ScaleWidth;
    }
    else if(scale_x < scale_y)
    {
        h1 = scale_x * ScaleHeight;
        viewport_scale_y = h1 / ScaleHeight;
    }

    offset_x = (w - w1) / 2;
    offset_y = (h - h1) / 2;

    SDL_Rect topLeftViewport;
    topLeftViewport.x = static_cast<int>(offset_x);
    topLeftViewport.y = static_cast<int>(offset_y);
    topLeftViewport.w = static_cast<int>(w1);
    topLeftViewport.h = static_cast<int>(h1);
    SDL_RenderSetViewport(m_gRenderer, &topLeftViewport);
}

StdPicture FrmMain::LoadPicture(std::string path, std::string maskPath, std::string maskFallbackPath)
{
    StdPicture target;

    //SDL_Surface * sourceImage;
    FIBITMAP *sourceImage;

    if(path.empty())
        return target;

    // Load the OpenGL texture
    //sourceImage = GraphicsHelps::loadQImage(path); // Gives us the information to make the texture
    sourceImage = GraphicsHelps::loadImage(path);

    //Don't load mask if PNG image is used
    if(Files::hasSuffix(path, ".png"))
    {
        maskPath.clear();
        maskFallbackPath.clear();
    }

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
    if(!maskPath.empty() && Files::fileExists(maskPath))
    {
        #ifdef DEBUG_BUILD
        maskMergingTime.start();
        #endif
        GraphicsHelps::mergeWithMask(sourceImage, maskPath);
        #ifdef DEBUG_BUILD
        maskElapsed = maskMergingTime.nanoelapsed();
        #endif
    }
    else if(!maskFallbackPath.empty())
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
    target.ColorUpper.r = float(upperColor.rgbRed) / 255.0f;
    target.ColorUpper.b = float(upperColor.rgbBlue) / 255.0f;
    target.ColorUpper.g = float(upperColor.rgbGreen) / 255.0f;
    RGBQUAD lowerColor;
    FreeImage_GetPixelColor(sourceImage, 0, static_cast<unsigned int>(h - 1), &lowerColor);
    target.ColorLower.r = float(lowerColor.rgbRed) / 255.0f;
    target.ColorLower.b = float(lowerColor.rgbBlue) / 255.0f;
    target.ColorLower.g = float(lowerColor.rgbGreen) / 255.0f;
    FreeImage_FlipVertical(sourceImage);
    target.nOfColors = GL_RGBA;
    target.format = GL_BGRA;
    target.w = static_cast<int>(w);
    target.h = static_cast<int>(h);
    target.frame_w = static_cast<int>(w);
    target.frame_h = static_cast<int>(h);
    GLubyte *textura = reinterpret_cast<GLubyte *>(FreeImage_GetBits(sourceImage));
    loadTexture(target, w, h, textura);
#ifdef DEBUG_BUILD
    bindElapsed = bindingTime.nanoelapsed();
    unloadTime.start();
#endif
    //SDL_FreeSurface(sourceImage);
    GraphicsHelps::closeImage(sourceImage);
#ifdef DEBUG_BUILD
    unloadElapsed = unloadTime.nanoelapsed();
#endif
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

void FrmMain::loadTexture(StdPicture &target, uint32_t width, uint32_t height, uint8_t *RGBApixels)
{
    SDL_Surface *surface;
    SDL_Texture *texture;
    surface = SDL_CreateRGBSurfaceFrom(RGBApixels,
                                       static_cast<int>(width),
                                       static_cast<int>(height),
                                       32,
                                       static_cast<int>(width * 4),
                                       FI_RGBA_RED_MASK,
                                       FI_RGBA_GREEN_MASK,
                                       FI_RGBA_BLUE_MASK,
                                       FI_RGBA_ALPHA_MASK);
    texture = SDL_CreateTextureFromSurface(m_gRenderer, surface);
    SDL_FreeSurface(surface);
    if(!texture)
    {
        pLogWarning("Render SW-SDL: Failed to load texture!");
        return;
    }

    target.texture = texture;
    m_textureBank.insert(texture);

    target.inited = true;
}

SDL_Rect FrmMain::scaledRectIS(float x, float y, int w, int h)
{
    return
    {
        static_cast<int>(std::ceil(x * viewport_scale_x)),
        static_cast<int>(std::ceil(y * viewport_scale_y)),
        static_cast<int>(std::ceil(static_cast<float>(w) * viewport_scale_x)),
        static_cast<int>(std::ceil(static_cast<float>(h) * viewport_scale_y))
    };
}

SDL_Rect FrmMain::scaledRect(float x, float y, float w, float h)
{
    return
    {
        static_cast<int>(std::ceil(x * viewport_scale_x)),
        static_cast<int>(std::ceil(y * viewport_scale_y)),
        static_cast<int>(std::ceil(w * viewport_scale_x)),
        static_cast<int>(std::ceil(h * viewport_scale_y))
    };
}

SDL_Rect FrmMain::scaledRectS(float left, float top, float right, float bottom)
{
    return
    {
        static_cast<int>(std::ceil(left * viewport_scale_x)),
        static_cast<int>(std::ceil(top * viewport_scale_y)),
        static_cast<int>(std::ceil((right - left)*viewport_scale_x)),
        static_cast<int>(std::ceil((bottom - top)*viewport_scale_y))
    };
}

void FrmMain::deleteTexture(StdPicture &tx)
{
    if(!tx.inited || !tx.texture)
    {
        tx.inited = false;
        return;
    }

    if(!tx.texture)
    {
        tx.inited = false;
        return;
    }

    auto corpseIt = m_textureBank.find(tx.texture);
    if(corpseIt == m_textureBank.end())
    {
        if(tx.texture)
            SDL_DestroyTexture(tx.texture);
        tx.texture = nullptr;
        tx.inited = false;
        return;
    }

    SDL_Texture *corpse = *corpseIt;
    if(corpse)
        SDL_DestroyTexture(corpse);
    m_textureBank.erase(corpse);

    tx.texture = nullptr;
    tx.inited = false;

    tx.w = 0;
    tx.h = 0;
    tx.frame_w = 0;
    tx.frame_h = 0;
    tx.format = 0;
    tx.nOfColors = 0;
    tx.ColorUpper.r = 0;
    tx.ColorUpper.g = 0;
    tx.ColorUpper.b = 0;
    tx.ColorLower.r = 0;
    tx.ColorLower.g = 0;
    tx.ColorLower.b = 0;
}

void FrmMain::clearAllTextures()
{
    for(SDL_Texture *tx : m_textureBank)
        SDL_DestroyTexture(tx);
    m_textureBank.clear();
}

void FrmMain::renderRect(int x, int y, int w, int h, float red, float green, float blue, float alpha, bool filled)
{
    SDL_Rect aRect = scaledRect(x, y, w, h);
    SDL_SetRenderDrawColor(m_gRenderer,
                           static_cast<unsigned char>(255.f * red),
                           static_cast<unsigned char>(255.f * green),
                           static_cast<unsigned char>(255.f * blue),
                           static_cast<unsigned char>(255.f * alpha)
                          );

    if(filled)
        SDL_RenderFillRect(m_gRenderer, &aRect);
    else
        SDL_RenderDrawRect(m_gRenderer, &aRect);
}

void FrmMain::renderRectBR(int _left, int _top, int _right, int _bottom, float red, float green, float blue, float alpha)
{
    SDL_Rect aRect = scaledRectS(_left, _top, _right, _bottom);
    SDL_SetRenderDrawColor(m_gRenderer,
                           static_cast<unsigned char>(255.f * red),
                           static_cast<unsigned char>(255.f * green),
                           static_cast<unsigned char>(255.f * blue),
                           static_cast<unsigned char>(255.f * alpha)
                          );
    SDL_RenderFillRect(m_gRenderer, &aRect);
}
