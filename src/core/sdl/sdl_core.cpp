/*
 * Moondust, a free game engine for platform game making
 * Copyright (c) 2014-2024 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * This software is licensed under a dual license system (MIT or GPL version 3 or later).
 * This means you are free to choose with which of both licenses (MIT or GPL version 3 or later)
 * you want to use this software.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You can see text of MIT license in the LICENSE.mit file you can see in Engine folder,
 * or see https://mit-license.org/.
 *
 * You can see text of GPLv3 license in the LICENSE.gpl3 file you can see in Engine folder,
 * or see <http://www.gnu.org/licenses/>.
 */

#include <locale.h>
#include <SDL2/SDL.h>
#include <Logger/logger.h>

#include "sdl_core.h"


bool CoreSDL::init(const CmdLineSetup_t &setup)
{
    (void)(setup);

    bool res;

    // if(g_config.background_work)
    // apply this unconditionally -- otherwise, need to restart game for background-work to function as expected
    SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
    
    // Disable the audio capture at all (some systems do ask microphone permission because of that)
    SDL_SetHint("SDL_AUDIO_DISABLE_CAPTURE", "1");

#if defined(__ANDROID__) || (defined(__APPLE__) && (defined(TARGET_IPHONE_SIMULATOR) || defined(TARGET_OS_IPHONE)))
    // Restrict the landscape orientation only
    SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");
#endif

#if defined(__ANDROID__)
    SDL_setenv("SDL_AUDIODRIVER", "openslES", 1);
    SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0");
    SDL_SetHint(SDL_HINT_MOUSE_TOUCH_EVENTS, "0");
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 6);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
#endif

    Uint32 sdlInitFlags = 0;
    // Prepare flags for SDL initialization
#if !defined(__EMSCRIPTEN__) && !defined(SDL_TIMERS_DISABLED)
    sdlInitFlags |= SDL_INIT_TIMER;
#endif
#if !defined(SDL_AUDIO_DISABLED)
    sdlInitFlags |= SDL_INIT_AUDIO;
#endif
#if !defined(__WII__) && !defined(__3DS__)
    sdlInitFlags |= SDL_INIT_VIDEO;
    sdlInitFlags |= SDL_INIT_EVENTS;
#endif
#if !defined(__WII__) && !defined(__3DS__) && !defined(SDL_JOYSTICK_DISABLED)
    sdlInitFlags |= SDL_INIT_JOYSTICK;
    sdlInitFlags |= SDL_INIT_GAMECONTROLLER;
#endif
#if !defined(__WII__) && !defined(__3DS__) && !defined(SDL_HAPTIC_DISABLED)
    sdlInitFlags |= SDL_INIT_HAPTIC;
#endif

    // Initialize SDL
    res = (SDL_Init(sdlInitFlags) >= 0);

    // Workaround: https://discourse.libsdl.org/t/26995
    setlocale(LC_NUMERIC, "C");

    const char *error = SDL_GetError();
    if(*error != '\0')
        pLogWarning("Error while SDL Initialization: %s", error);
    SDL_ClearError();

    SDL_version compiled, runtime;
    SDL_VERSION(&compiled);
    SDL_GetVersion(&runtime);
    pLogDebug("Compiled with SDL %d.%d.%d headers, running with SDL %d.%d.%d",
        compiled.major, compiled.minor, compiled.patch,
        runtime.major, runtime.minor, runtime.patch);

    return res;
}

void CoreSDL::quit()
{
    SDL_Quit();
}
