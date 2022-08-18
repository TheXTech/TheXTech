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

#ifndef __3DS__
#error This file should only be built on 3DS.
#endif

#include <string>
#include <set>

#include <cstring>
#include <cstdlib>

#include "core/mixer.h"
#include "3ds-audio-lib.h"

static SoundId* cur_sound = nullptr;

std::set<const std::string*> sound_stream_paths;

bool MixPlatform_Init()
{
    return audioInit();
}

void MixPlatform_Quit()
{
    if(cur_sound)
    {
        free(cur_sound);
        cur_sound = nullptr;
    }
    audioExit();
}

bool MixPlatform_NoPreload(const char* path)
{
    // if path is not WAV, for now
    const char* ext = get_filename_ext(path);
    if(strcasecmp(ext, "wav"))
        return true;
    else
        return false;
}

int MixPlatform_PlayStream(int channel, const char* path, int loops)
{
    if(!cur_sound || channel < 0)
    {
        playSoundAuto(path, loops);
    }
    else
    {
        killSound(cur_sound[channel]);
        cur_sound[channel] = playSoundAuto(path, loops);
    }
    return 0;
}

Mix_Chunk* MixPlatform_LoadWAV(const char* path)
{
    // remove arguments from path
    char* path2 = strdup(path);
    if(path2)
    {
        for(int i = 0; path2[i] != '\0'; i++)
        {
            if(path2[i] == '|')
                path2[i] = '\0';
        }
    }
    const char* path_use = path2 ? path2 : path;

    Mix_Chunk* ret;
    if(MixPlatform_NoPreload(path_use) || (ret = (Mix_Chunk*)audioLoadWave(path_use)) == nullptr)
    {
        const std::string* s = new(std::nothrow) const std::string(path_use);
        if(!s)
            return nullptr;

        sound_stream_paths.insert(s);
        ret = (Mix_Chunk*)s;
    }

    if(path2)
        free(path2);

    return ret;
}

const char* MixPlatform_GetError()
{
    return "";
}

inline SoundId playSoundMaybeStream(Mix_Chunk* chunk, int loops)
{
    auto it = sound_stream_paths.find((const std::string*)chunk);
    if(it != sound_stream_paths.end())
    {
        return playSoundAuto(((const std::string*)chunk)->c_str(), loops);
    }

    return playSoundMem((WaveObject*)chunk, loops);
}

int MixPlatform_PlayChannelTimed(int channel, Mix_Chunk *chunk, int loops, int ticks)
{
    (void)ticks;
    if(!cur_sound || channel < 0)
    {
        playSoundMaybeStream(chunk, loops);
        return 0;
    }
    killSound(cur_sound[channel]);
    cur_sound[channel] = playSoundMaybeStream(chunk, loops);
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
    cur_sound = (SoundId*)malloc(sizeof(SoundId)*channels);
    for(int i = 0; i < channels; i++)
    {
        cur_sound[i] = INVALID_ID;
    }
    return 0;
}

void Mix_PauseAudio(int pause)
{
    if(pause)
        audioPause();
    else
        audioResume();
}

Mix_Music* Mix_LoadMUS(const char* path)
{
    // remove arguments from path
    char* path2 = strdup(path);
    if(path2)
    {
        for(int i = 0; path2[i] != '\0'; i++)
        {
            if(path2[i] == '|')
                path2[i] = '\0';
        }
    }
    const char* path_use = path2 ? path2 : path;

    Mix_Music* ret = (Mix_Music*)playSoundAuto(path_use);

    if(path2)
        free(path2);

    return ret;
}

int Mix_VolumeMusicStream(Mix_Music* music, int volume)
{
    (void)music;
    (void)volume;
    return 0;
}

int Mix_HaltMusicStream(Mix_Music* music)
{
    killSound((SoundId)music);
    return 0;
}
int Mix_FadeOutMusicStream(Mix_Music* music, int ms)
{
    (void)ms;
    killSound((SoundId)music);
    return 0;
}
int Mix_HaltChannel(int channel)
{
    if(cur_sound && channel >= 0)
        killSound(cur_sound[channel]);
    return 0;
}

int Mix_PlayingMusicStream(Mix_Music* music)
{
    (void)music;
    // no problem on 3DS
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
    killSound((SoundId)music);
}
void Mix_FreeChunk(Mix_Chunk* chunk)
{
    auto it = sound_stream_paths.find((const std::string*)chunk);
    if(it != sound_stream_paths.end())
    {
        sound_stream_paths.erase(it);
        delete (const std::string*)chunk;
        return;
    }

    audioFreeWave((WaveObject*) chunk);
}

void Mix_ChannelFinished(void (*cb)(int))
{
    audioSetChannelDoneCallback(cb);
}

} // extern "C"
