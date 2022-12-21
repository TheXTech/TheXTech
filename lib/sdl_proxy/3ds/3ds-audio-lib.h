/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2022 Vitaly Novichkov <admin@wohlnet.ru>
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

#include <3ds.h>

#ifndef NUM_CHANNELS
// must *never* exceed 32
#define NUM_CHANNELS 16
#define NUM_BUFFERS 4
#define BUFFER_SIZE (128 * (44100 / 1000))

#define THREAD_STACK_SZ 32 * 1024

#endif

typedef uintptr_t SoundId;

typedef enum _CHANNEL_FORMAT
{
    FORMAT_FREE = 1 << 0,
    FORMAT_LOADING = 1 << 1,
    FORMAT_PCM_FILE = 1 << 2,
    FORMAT_OGG_FILE = 1 << 3,
    FORMAT_PCM_MEM = 1 << 4,
    FORMAT_GME_FILE = 1 << 5,
} CHANNEL_FORMAT;

typedef struct _SimpleChannel
{
    int channel_id;
    CHANNEL_FORMAT format;
    void* data_pointer;
    size_t data_idx;
    size_t data_size;
    ndspWaveBuf wavebufs[NUM_BUFFERS];
    bool kill;
    bool stereo;
    int loops;
    SoundId index;
} SimpleChannel;

typedef struct _WaveObject
{
    bool stereo;
    uint32_t sampleRate;
    const char* data;
    uint32_t length;
} WaveObject;

static const SoundId INVALID_ID = 0;

bool audioInit();

// Audio de-initialisation code
// Stops playback and frees the primary audio buffer
void audioExit();

void audioPause();
void audioResume();

bool audioPauseSingle(SoundId soundId);
bool audioResumeSingle(SoundId soundId);

bool audioSoundPlaying(SoundId soundId);

const char* get_filename_ext(const char* filename);

SoundId playSoundWAV(const char* path, int loops = 0);

SoundId playSoundMem(const WaveObject* wave, int loops = 0);

SoundId playSoundOGG(const char* path, int loops = -1);

SoundId playSoundGME(const char* path, int loops = -1);

SoundId playSoundAuto(const char* path, int loops = -1);

void killSound(SoundId soundId);

WaveObject* audioLoadWave(const char* path);

void audioFreeWave(WaveObject* wave);

void audioSetChannelDoneCallback(void (*cb)(int));
