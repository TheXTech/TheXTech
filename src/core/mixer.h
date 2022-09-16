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

/* @{ */
#define AUDIO_U8        0x0008
#define AUDIO_S8        0x8008
#define AUDIO_U16LSB    0x0010
#define AUDIO_S16LSB    0x8010
#define AUDIO_U16MSB    0x1010
#define AUDIO_S16MSB    0x9010
#define AUDIO_U16       AUDIO_U16LSB
#define AUDIO_S16       AUDIO_S16LSB
/* @} */

/* @{ */
#define AUDIO_S32LSB    0x8020
#define AUDIO_S32MSB    0x9020
#define AUDIO_S32       AUDIO_S32LSB
/* @} */

/* @{ */
#define AUDIO_F32LSB    0x8120
#define AUDIO_F32MSB    0x9120
#define AUDIO_F32       AUDIO_F32LSB
/* @} */

/* @{ */
#ifndef THEXTECH_BIG_ENDIAN
#define AUDIO_U16SYS    AUDIO_U16LSB
#define AUDIO_S16SYS    AUDIO_S16LSB
#define AUDIO_S32SYS    AUDIO_S32LSB
#define AUDIO_F32SYS    AUDIO_F32LSB
#else
#define AUDIO_U16SYS    AUDIO_U16MSB
#define AUDIO_S16SYS    AUDIO_S16MSB
#define AUDIO_S32SYS    AUDIO_S32MSB
#define AUDIO_F32SYS    AUDIO_F32MSB
#endif
/* @} */

struct Mix_Music;
struct Mix_Chunk;

bool MixPlatform_Init(AudioSetup_t& obtained);
void MixPlatform_Quit();

int  MixPlatform_PlayStream(int channel, const char* path, int loops);

// need to do this because LoadWAV is not properly declared in MixerX.
#define Mix_LoadWAV(file)   MixPlatform_LoadWAV(file);
Mix_Chunk* MixPlatform_LoadWAV(const char* path);

#define Mix_GetError MixPlatform_GetError
const char* MixPlatform_GetError();

#define Mix_PlayChannel(channel,chunk,loops) MixPlatform_PlayChannelTimed(channel,chunk,loops,-1)
int MixPlatform_PlayChannelTimed(int channel, Mix_Chunk *chunk, int loops, int ticks);

#define Mix_PlayChannelVol(channel,chunk,loops,vol) MixPlatform_PlayChannelTimedVolume(channel,chunk,loops,-1,vol)
int MixPlatform_PlayChannelTimedVolume(int which, Mix_Chunk *chunk, int loops, int ticks, int volume);

extern "C" {

extern int Mix_ReserveChannels(int channels);
extern void Mix_PauseAudio(int pause);


extern Mix_Music* Mix_LoadMUS(const char* path);

extern int Mix_VolumeMusicStream(Mix_Music* music, int volume);
extern int Mix_HaltMusicStream(Mix_Music* music);
extern int Mix_FadeOutMusicStream(Mix_Music* music, int ms);
extern int Mix_HaltChannel(int channel);

extern int Mix_PlayingMusicStream(Mix_Music* music);

extern int Mix_PlayMusic(Mix_Music *music, int loops);
extern int Mix_FadeInMusic(Mix_Music* music, int loops, int fadeInMs);
extern int Mix_SetFreeOnStop(Mix_Music *music, int free_on_stop);

extern int Mix_FadeOutMusic(int fadeInMs);

extern int Mix_PlayMusicStream(Mix_Music *music, int loops);

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

#endif

} // extern "C"

#endif // #ifndef MIXER_HHHHH
