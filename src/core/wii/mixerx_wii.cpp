/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2021 Vitaly Novichkov <admin@wohlnet.ru>
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

#include <string>
#include <set>

#include <cstring>
#include <cstdlib>

#include "globals.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer_ext.h>

#include <Logger/logger.h>
#include <fmt_format_ne.h>
#include "sound.h"

static const int maxSfxChannels = 91;

bool MixPlatform_Init()
{
    if(SDL_Init(SDL_INIT_AUDIO) < 0)
    {
        pLogWarning("Couldn't initialize SDL audio: %s\n",SDL_GetError());
        return false;
    }

    int ret;
    const int initFlags = MIX_INIT_MID | MIX_INIT_MOD | MIX_INIT_FLAC | MIX_INIT_OGG | MIX_INIT_OPUS | MIX_INIT_MP3;
    pLogDebug("Opening sound...");
    ret = Mix_Init(initFlags);

    if(ret != initFlags)
    {
        pLogWarning("MixerX: Some modules aren't properly initialized");
        if((initFlags & MIX_INIT_MID) != MIX_INIT_MID)
            pLogWarning("MixerX: Failed to initialize MIDI module");
        if((initFlags & MIX_INIT_MOD) != MIX_INIT_MOD)
            pLogWarning("MixerX: Failed to initialize Tracker music module");
        if((initFlags & MIX_INIT_FLAC) != MIX_INIT_FLAC)
            pLogWarning("MixerX: Failed to initialize FLAC module");
        if((initFlags & MIX_INIT_OGG) != MIX_INIT_OGG)
            pLogWarning("MixerX: Failed to initialize OGG Vorbis module");
        if((initFlags & MIX_INIT_OPUS) != MIX_INIT_OPUS)
            pLogWarning("MixerX: Failed to initialize Opus module");
        if((initFlags & MIX_INIT_MP3) != MIX_INIT_MP3)
            pLogWarning("MixerX: Failed to initialize MP3 module");
    }

    ret = Mix_OpenAudio(g_audioSetup.sampleRate,
                        g_audioSetup.format,
                        g_audioSetup.channels,
                        g_audioSetup.bufferSize);

    if(ret < 0)
    {
        std::string msg = fmt::format_ne("Can't open audio stream, continuing without audio: ({0})", Mix_GetError());
        pLogCritical(msg.c_str());
        return false;
    }

    Mix_VolumeMusic(MIX_MAX_VOLUME);
    Mix_AllocateChannels(maxSfxChannels);

    return true;
}

void MixPlatform_Quit()
{
    Mix_CloseAudio();
    Mix_Quit();
}

int MixPlatform_PlayStream(int channel, const char* path, int loops)
{
    int ret;
    (void)channel;

    Mix_Music *mus = Mix_LoadMUS(path);

    if(!mus)
        return -1;

    ret = Mix_PlayMusicStream(mus, loops);

    if(ret < 0)
        Mix_FreeMusic(mus);
    else
        Mix_SetFreeOnStop(mus, 1);

    return ret;
}

Mix_Chunk* MixPlatform_LoadWAV(const char* path)
{
    return Mix_LoadWAV(path);
}

const char* MixPlatform_GetError()
{
    return Mix_GetError();
}

int MixPlatform_PlayChannelTimed(int channel, Mix_Chunk *chunk, int loops, int ticks)
{
    return Mix_PlayChannelTimed(channel, chunk, loops, ticks);
}

int MixPlatform_PlayChannelTimedVolume(int channel, Mix_Chunk *chunk, int loops, int ticks, int volume)
{
    return Mix_PlayChannelTimedVolume(channel, chunk, loops, ticks, volume);
}
