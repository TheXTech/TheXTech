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

int Mix_Init(int flags)
{
    UNUSED(flags);
    return 0; // Always fail
}

void Mix_Quit()
{}

int Mix_OpenAudio(int frequency, Uint16 format, int channels, int chunksize)
{
    (void)frequency;
    (void)format;
    (void)channels;
    (void)chunksize;

    return -1; // Always fail
}

int Mix_QuerySpecEx(SDL_AudioSpec* out_spec)
{
    SDL_memset(out_spec, 0, sizeof(SDL_AudioSpec));
    out_spec->channels = 2;
    out_spec->samples = 2048;
    out_spec->freq = 32728;
    out_spec->format = AUDIO_S16SYS;
    return 0;
}

void Mix_CloseAudio()
{}

int Mix_VolumeMusic(int volume)
{
    (void)volume;
    return volume;
}

int Mix_AllocateChannels(int numchans)
{
    (void)numchans;
    return 0;
}

Mix_Chunk* Mix_LoadWAV(const char* path)
{
    UNUSED(path);
    return nullptr; // Always fail
}

const char* Mix_GetError()
{
    return "";
}

int Mix_PlayChannel(int channel, Mix_Chunk* chunk, int loops)
{
    UNUSED(channel);
    UNUSED(chunk);
    UNUSED(loops);
    return 0;
}

#ifndef Mix_PlayChannelVol
int Mix_PlayChannelVol(int channel, Mix_Chunk* chunk, int loops, int volume)
{
    UNUSED(channel);
    UNUSED(chunk);
    UNUSED(loops);
    UNUSED(volume);
    return -1;
}
#else
int Mix_PlayChannelTimedVolume(int channel, Mix_Chunk* chunk, int loops, int ticks, int volume)
{
    UNUSED(channel);
    UNUSED(chunk);
    UNUSED(loops);
    UNUSED(ticks);
    UNUSED(volume);
    return -1;
}
#endif

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
    return nullptr; // Always fail
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
    UNUSED(ms);
    UNUSED(music);
    return 0;
}
int Mix_HaltChannel(int channel)
{
    UNUSED(channel);
    return 0;
}

int Mix_PlayingMusicStream(Mix_Music* music)
{
    UNUSED(music);
    return -1;
}

int Mix_PausedMusicStream(Mix_Music* music)
{
    UNUSED(music);
    return -1;
}

int Mix_PauseMusicStream(Mix_Music* music)
{
    UNUSED(music);
    return 0;
}

int Mix_ResumeMusicStream(Mix_Music* music)
{
    UNUSED(music);
    return 0;
}

int Mix_PlayMusic(Mix_Music* music, int loops)
{
    UNUSED(music);
    UNUSED(loops);
    return 0;
}

int Mix_PlayMusicStream(Mix_Music* music, int loops)
{
    UNUSED(music);
    UNUSED(loops);
    return -1;
}

int Mix_SetFreeOnStop(Mix_Music* music, int free_on_stop)
{
    UNUSED(music);
    UNUSED(free_on_stop);
    return 0;
}

int Mix_FadeInMusic(Mix_Music* music, int loops, int fadeInMs)
{
    UNUSED(music);
    UNUSED(loops);
    UNUSED(fadeInMs);
    return -1;
}

const char* Mix_GetMusicTitle(Mix_Music* music)
{
    UNUSED(music);
    return "";
}

int Mix_GetMusicTracks(Mix_Music* music)
{
    UNUSED(music);
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
    UNUSED(music);
}

void Mix_FreeChunk(Mix_Chunk* chunk)
{
    UNUSED(chunk);
}

void Mix_GME_SetSpcEchoDisabled(Mix_Music* music, int disable)
{
    (void)music;
    (void)disable;
}

void Mix_RegisterEffect(int chan, Mix_EffectFunc_t f, Mix_EffectDone_t d, void* arg)
{
    (void)chan;
    (void)f;
    (void)d;
    (void)arg;
}

void Mix_UnregisterEffect(int chan, Mix_EffectFunc_t f)
{
    (void)chan;
    (void)f;
}

void Mix_ChannelFinished(void (*cb)(int))
{
    UNUSED(cb);
}

void Mix_ADLMIDI_setEmulator(int emu)
{
    (void)emu;
}

void Mix_ADLMIDI_setChipsCount(int chips)
{
    (void)chips;
}

void Mix_OPNMIDI_setEmulator(int emu)
{
    (void)emu;
}

void Mix_OPNMIDI_setChipsCount(int chips)
{
    (void)chips;
}
