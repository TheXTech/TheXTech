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
#include <unordered_map>

#include <cstring>
#include <cstdlib>

#include "../mixer.h"
#include "3ds-audio-lib.h"

static SoundId* cur_sound = nullptr;

static std::set<const std::string*> sound_stream_paths;
static std::set<const std::string*> music_stream_paths;
static std::unordered_map<const std::string*, SoundId> sound_id_music_map;

//bool MixPlatform_Init(AudioSetup_t& obtained)
//{
//    (void)obtained;
//    return audioInit();
//}

//void MixPlatform_Quit()
//{
//    if(cur_sound)
//    {
//        free(cur_sound);
//        cur_sound = nullptr;
//    }
//    audioExit();
//}

static bool MixPlatform_NoPreload(const char* path)
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


int Mix_Init(int flags)
{
    // These codecs only supported
    return flags & (MIX_INIT_OGG | MIX_INIT_MP3);
}

void Mix_Quit()
{
    for(auto* it : sound_stream_paths)
        delete it;

    sound_stream_paths.clear();

    for(auto* it : music_stream_paths)
        delete it;

    music_stream_paths.clear();

    sound_id_music_map.clear();
}

int Mix_OpenAudio(int frequency, Uint16 format, int channels, int chunksize)
{
    (void)frequency;
    (void)format;
    (void)channels;
    (void)chunksize;

    return audioInit() ? 0 : -1;
}

int Mix_QuerySpecEx(SDL_AudioSpec* out_spec)
{
    memset(out_spec, 0, sizeof(SDL_AudioSpec));
    out_spec->channels = 2;
    out_spec->samples = 2048;
    out_spec->freq = 32728;
    out_spec->format = AUDIO_S16SYS;
    return 0;
}

void Mix_CloseAudio()
{
    if(cur_sound)
    {
        free(cur_sound);
        cur_sound = nullptr;
    }

    audioExit();
}

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

const char* Mix_GetError()
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

int Mix_PlayChannel(int channel, Mix_Chunk* chunk, int loops)
{
    if(!cur_sound || channel < 0)
    {
        playSoundMaybeStream(chunk, loops);
        return 0;
    }

    killSound(cur_sound[channel]);
    cur_sound[channel] = playSoundMaybeStream(chunk, loops);
    return 0;
}

int Mix_PlayChannelVol(int channel, Mix_Chunk* chunk, int loops, int volume)
{
    (void)volume;
    return Mix_PlayChannel(channel, chunk, loops);
}

int Mix_ReserveChannels(int channels)
{
    cur_sound = (SoundId*)malloc(sizeof(SoundId) * channels);

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
    Mix_Music* ret;
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

    const std::string* s = new(std::nothrow) const std::string(path_use);

    if(!s)
        return nullptr;

    music_stream_paths.insert(s);
    ret = (Mix_Music*)s;

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
    return audioSoundPlaying((SoundId)music);
}

int Mix_PausedMusicStream(Mix_Music* music)
{
    return audioSoundPlaying((SoundId)music);
}

int Mix_PauseMusicStream(Mix_Music* music)
{
    return (bool)audioPauseSingle((SoundId)music);
}

int Mix_ResumeMusicStream(Mix_Music* music)
{
    return (bool)audioResumeSingle((SoundId)music);
}

int Mix_PlayMusic(Mix_Music* music, int loops)
{
    const std::string* path_use = reinterpret_cast<const std::string*>(music);

    SoundId ret = playSoundAuto(path_use->c_str(), loops);
    sound_id_music_map.insert({path_use, ret});

    return 0;
}

int Mix_PlayMusicStream(Mix_Music* music, int loops)
{
    return Mix_PlayMusic(music, loops);
}

int Mix_SetFreeOnStop(Mix_Music* music, int free_on_stop)
{
    const std::string* path_use = reinterpret_cast<const std::string*>(music);

    if(free_on_stop)
        sound_id_music_map.erase(path_use);

    return 0;
}

int Mix_FadeInMusic(Mix_Music* music, int loops, int fadeInMs)
{
    (void)fadeInMs;
    return Mix_PlayMusic(music, loops);
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
    const std::string* path_use = reinterpret_cast<const std::string*>(music);
    auto f = sound_id_music_map.find(path_use);

    if(f != sound_id_music_map.end())
    {
        killSound(f->second);
        sound_id_music_map.erase(f);
    }

    auto it = music_stream_paths.find(path_use);

    if(it != music_stream_paths.end())
    {
        music_stream_paths.erase(it);
        delete path_use;
    }
}

void Mix_FreeChunk(Mix_Chunk* chunk)
{
    auto it = sound_stream_paths.find(reinterpret_cast<const std::string*>(chunk));

    if(it != sound_stream_paths.end())
    {
        sound_stream_paths.erase(it);
        delete(const std::string*)chunk;
        return;
    }

    audioFreeWave(reinterpret_cast<WaveObject*>(chunk));
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
    audioSetChannelDoneCallback(cb);
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

