/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2021 Vitaly Novichkov <admin@wohlnet.ru>
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

#pragma once
#ifndef MIXER_HHHHH
#define MIXER_HHHHH

// for AudioSetup_t
#include "sound.h"
#include "sdl_audio.h"

bool MixPlatform_Init(AudioSetup_t& obtained);
void MixPlatform_Quit();

int  MixPlatform_PlayStream(int channel, const char* path, int loops);

#ifdef NO_SDL

struct Mix_Music;
struct Mix_Chunk;

extern "C" {

extern const char* Mix_GetError();

extern int Mix_ReserveChannels(int channels);
extern void Mix_PauseAudio(int pause);


extern Mix_Music* Mix_LoadMUS(const char* path);
extern Mix_Chunk* Mix_LoadWAV(const char* path);

extern int Mix_VolumeMusicStream(Mix_Music* music, int volume);
extern int Mix_HaltMusicStream(Mix_Music* music);
extern int Mix_FadeOutMusicStream(Mix_Music* music, int ms);
extern int Mix_HaltChannel(int channel);

extern int Mix_PlayChannel(int channel, Mix_Chunk *chunk, int loops);
extern int Mix_PlayChannelVol(int which, Mix_Chunk *chunk, int loops, int volume);

extern int Mix_PlayingMusicStream(Mix_Music* music);

extern int Mix_PlayMusic(Mix_Music *music, int loops);
extern int Mix_FadeInMusic(Mix_Music* music, int loops, int fadeInMs);
extern int Mix_SetFreeOnStop(Mix_Music *music, int free_on_stop);

extern int Mix_FadeOutMusic(int fadeInMs);

extern int Mix_PlayMusicStream(Mix_Music *music, int loops);

extern int Mix_PausedMusicStream(Mix_Music *music);
extern int Mix_PauseMusicStream(Mix_Music *music);
extern int Mix_ResumeMusicStream(Mix_Music *music);

extern int Mix_GetMusicTracks(Mix_Music* music);
extern const char* Mix_GetMusicTitle(Mix_Music* music);

extern int Mix_SetMusicTrackMute(Mix_Music* music, int track, int mute);

extern void Mix_FreeMusic(Mix_Music* music);
extern void Mix_FreeChunk(Mix_Chunk* chunk);

extern void Mix_ChannelFinished(void (*callback)(int));

#ifdef THEXTECH_ENABLE_AUDIO_FX

#define MIX_CHANNEL_POST   -2

typedef void( *  Mix_EffectFunc_t) (int chan, void *stream, int len, void *udata);
typedef void( *  Mix_EffectDone_t) (int chan, void *udata);

extern void Mix_GME_SetSpcEchoDisabled(Mix_Music* music, int disable);
extern void Mix_RegisterEffect(int chan, Mix_EffectFunc_t f, Mix_EffectDone_t d, void *arg);
extern void Mix_UnregisterEffect(int chan, Mix_EffectFunc_t f);

#endif // #ifdef THEXTECH_ENABLE_AUDIO_FX

} // extern "C"

#else // #ifdef NO_SDL

#include <SDL2/SDL_mixer_ext.h>

#endif

#endif // #ifndef MIXER_HHHHH
