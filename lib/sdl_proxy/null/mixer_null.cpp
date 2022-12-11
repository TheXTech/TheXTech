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

#include "../mixer.h"
#include "globals.h"

bool MixPlatform_Init(AudioSetup_t& obtained)
{
    UNUSED(obtained);
    return true;
}

void MixPlatform_Quit()
{
}

int MixPlatform_PlayStream(int channel, const char* path, int loops)
{
    return 0;
}

Mix_Chunk* MixPlatform_LoadWAV(const char* path)
{
    return (Mix_Chunk*)1;
}

const char* MixPlatform_GetError()
{
    return "";
}

int MixPlatform_PlayChannelTimed(int channel, Mix_Chunk *chunk, int loops, int ticks)
{
    return 0;
}

int MixPlatform_PlayChannelTimedVolume(int channel, Mix_Chunk *chunk, int loops, int ticks, int volume)
{
    (void)volume;
    return MixPlatform_PlayChannelTimed(channel, chunk, loops, ticks);
}


extern "C" {

int Mix_ReserveChannels(int channels)
{
    UNUSED(channels);

    return 0;
}

void Mix_PauseAudio(int pause)
{
    UNUSED(pause);
}

Mix_Music* Mix_LoadMUS(const char* path)
{
    UNUSED(path);
    return (Mix_Music*)1;
}

int Mix_VolumeMusicStream(Mix_Music* music, int volume)
{
    (void)music;
    (void)volume;
    return 0;
}

int Mix_HaltMusicStream(Mix_Music* music)
{
    UNUSED(music);
    return 0;
}
int Mix_FadeOutMusicStream(Mix_Music* music, int ms)
{
    UNUSED(music);
    (void)ms;
    return 0;
}
int Mix_HaltChannel(int channel)
{
    UNUSED(channel);
    return 0;
}

int Mix_PlayingMusicStream(Mix_Music* music)
{
    (void)music;
    return 0;
}

int Mix_PlayMusic(Mix_Music* music, int loops)
{
    (void)music;
    (void)loops;
    return 0;
}
int Mix_PlayMusicStream(Mix_Music* music, int loops)
{
    (void)music;
    (void)loops;
    return 0;
}
int Mix_SetFreeOnStop(Mix_Music* music, int loops)
{
    (void)music;
    (void)loops;
    return 0;
}
int Mix_FadeInMusic(Mix_Music* music, int loops, int fadeInMs)
{
    (void)music;
    (void)loops;
    (void)fadeInMs;
    return 0;
}

int Mix_PausedMusicStream(Mix_Music *music)
{
    return 0;
}
int Mix_PauseMusicStream(Mix_Music *music)
{
    return 0;
}
int Mix_ResumeMusicStream(Mix_Music *music)
{
    return 0;
}


const char* Mix_GetMusicTitle(Mix_Music* music)
{
    (void)music;
    return "";
}

int Mix_GetMusicTracks(Mix_Music* music)
{
    (void)music;
    return 1;
}

int Mix_SetMusicTrackMute(Mix_Music* music, int track, int mute)
{
    (void)music;
    (void)track;
    (void)mute;
    return 0;
}

void Mix_FreeMusic(Mix_Music* music)
{
    (void)music;
}
void Mix_FreeChunk(Mix_Chunk* chunk)
{
    (void)chunk;
}

void Mix_ChannelFinished(void (*cb)(int))
{
    (void)cb;
}

} // extern "C"
