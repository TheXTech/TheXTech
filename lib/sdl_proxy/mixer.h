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

#ifdef CUSTOM_AUDIO

struct Mix_Music;
struct Mix_Chunk;

#ifndef MIX_CHANNELS
#define MIX_CHANNELS    8
#endif

/* Good default values for a PC soundcard */
#define MIX_DEFAULT_FREQUENCY   44100
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
#define MIX_DEFAULT_FORMAT      AUDIO_S16LSB
#else
#define MIX_DEFAULT_FORMAT      AUDIO_S16MSB
#endif
#define MIX_DEFAULT_CHANNELS    2
#define MIX_MAX_VOLUME          SDL_MIX_MAXVOLUME /* Volume of a chunk */

extern "C" {

/**
 * Initialization flags
 */
typedef enum
{
    MIX_INIT_FLAC   = 0x00000001,
    MIX_INIT_MOD    = 0x00000002,
    MIX_INIT_MP3    = 0x00000008,
    MIX_INIT_OGG    = 0x00000010,
    MIX_INIT_MID    = 0x00000020,
    MIX_INIT_OPUS   = 0x00000040
} MIX_InitFlags;

/* OPL3 chip emulators for ADLMIDI */
typedef enum {
    ADLMIDI_OPL3_EMU_DEFAULT = -1,
    ADLMIDI_OPL3_EMU_NUKED = 0,
    ADLMIDI_OPL3_EMU_NUKED_1_7_4,
    ADLMIDI_OPL3_EMU_DOSBOX,
    ADLMIDI_OPL3_EMU_OPAL,
    ADLMIDI_OPL3_EMU_JAVA
} Mix_ADLMIDI_Emulator;

/* OPN2 chip emulators for OPNMIDI */
typedef enum {
    OPNMIDI_OPN2_EMU_DEFAULT = -1,
    OPNMIDI_OPN2_EMU_MAME_OPN2 = 0,
    OPNMIDI_OPN2_EMU_NUKED,
    OPNMIDI_OPN2_EMU_GENS,
    OPNMIDI_OPN2_EMU_GX, /* Caution: THIS emulator is inavailable by default */
    OPNMIDI_OPN2_EMU_NP2,
    OPNMIDI_OPN2_EMU_MAME_OPNA,
    OPNMIDI_OPN2_EMU_PMDWIN,
    /* Deprecated */
    OPNMIDI_OPN2_EMU_MIME = 0 /*!!!TYPO!!!*/
} Mix_OPNMIDI_Emulator;

extern int Mix_Init(int flags);

extern void Mix_Quit(void);

extern int Mix_OpenAudio(int frequency, Uint16 format, int channels, int chunksize);
extern void Mix_CloseAudio(void);

extern int Mix_VolumeMusic(int volume);
extern int Mix_AllocateChannels(int numchans);

extern int Mix_QuerySpecEx(SDL_AudioSpec *out_spec);

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

extern void Mix_ADLMIDI_setEmulator(int emu);
extern void Mix_ADLMIDI_setChipsCount(int chips);

extern void Mix_OPNMIDI_setEmulator(int emu);
extern void Mix_OPNMIDI_setChipsCount(int chips);

#endif // #ifdef THEXTECH_ENABLE_AUDIO_FX

} // extern "C"

#else // #ifdef NO_SDL

#include <SDL2/SDL_mixer_ext.h>

#endif

#endif // #ifndef MIXER_HHHHH
