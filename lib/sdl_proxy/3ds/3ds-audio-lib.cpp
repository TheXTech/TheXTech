/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2024 Vitaly Novichkov <admin@wohlnet.ru>
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

#include <stdio.h>
#include <string.h>
#include "3ds-audio-lib.h"
#include <tremor/ivorbisfile.h>
#include <3ds.h>
#include <cstdlib>
#include <gme/gme.h>

#include <Logger/logger.h>

volatile SimpleChannel channels[NUM_CHANNELS];
char* audio_buffer = NULL;

LightEvent sound_event;
Thread sound_thread;
volatile bool sound_quit = false;  // Quit flag
void (*channel_done_callback)(int) = nullptr;


void myAudioCallback(void* const /*nul_*/)
{
    if(sound_quit)
        return;

    LightEvent_Signal(&sound_event);
}

inline SoundId getSoundId(volatile SimpleChannel* channel)
{
    return channel->channel_id + channel->index * 32;
}

inline volatile SimpleChannel* validateSoundId(SoundId id)
{
    if(id == INVALID_ID) return nullptr;

    int channel_id = id % 32;
    SoundId index = id / 32;

    if(channels[channel_id].index == index)
    {
        return &channels[channel_id];
    }

    return nullptr;
}

// should only be called by the audio thread if it is running
void closeChannel(volatile SimpleChannel* channel)
{
    if(channel->format == FORMAT_PCM_FILE && channel->data_pointer != nullptr)
    {
        fclose((FILE*)channel->data_pointer);
    }

    if(channel->format == FORMAT_OGG_FILE && channel->data_pointer != nullptr)
    {
        ov_clear((OggVorbis_File*)channel->data_pointer);
        free(channel->data_pointer);
        // OGG does not free its pointer.
    }

    if(channel->format == FORMAT_GME_FILE && channel->data_pointer != nullptr)
    {
        gme_delete((Music_Emu*)channel->data_pointer);
        // GME frees its pointer.
    }

    // DO NOT free the data pointer if it is a PCM_MEM because it is shared.
    channel->data_pointer = nullptr;
    channel->index ++;

    if(channel_done_callback)
        channel_done_callback(channel->channel_id);
}

// returns true on success, false on failure
bool fillBuffer(volatile SimpleChannel* channel, volatile ndspWaveBuf* wavebuf)
{
    if(wavebuf->status != NDSP_WBUF_DONE) return false;

    // load data into wavebuf...
    if(channel->format == FORMAT_PCM_FILE && channel->data_pointer != nullptr)
    {
        uint32_t* buffer_loc = (uint32_t*)wavebuf->data_pcm16;  // in 2x16-bit samples
        wavebuf->nsamples = 0;
        size_t samplesToRead = BUFFER_SIZE / sizeof(*buffer_loc);

        while(samplesToRead > 0)
        {
            int samples = fread(buffer_loc, sizeof(*buffer_loc), samplesToRead, (FILE*)channel->data_pointer);

            if(samples == 0)
            {
                if(channel->loops == 0)
                {
                    closeChannel(channel);
                    break;
                }
                else
                {
                    if(channel->loops > 0)
                        channel->loops -= 1;

                    fseek((FILE*)channel->data_pointer, channel->data_idx, SEEK_SET);
                }
            }

            wavebuf->nsamples += samples;
            buffer_loc += samples;
            samplesToRead -= samples;
        }

        if(channel->stereo)
        {
            ndspChnWaveBufAdd(channel->channel_id, (ndspWaveBuf*)wavebuf);
            DSP_FlushDataCache(wavebuf->data_pcm16,
                               wavebuf->nsamples * sizeof(*buffer_loc));
        }
        else
        {
            wavebuf->nsamples *= 2;
            ndspChnWaveBufAdd(channel->channel_id, (ndspWaveBuf*)wavebuf);
            DSP_FlushDataCache(wavebuf->data_pcm16,
                               wavebuf->nsamples / 2 * sizeof(*buffer_loc));
        }

        return true;
    }

    if(channel->format == FORMAT_PCM_MEM && channel->data_pointer != nullptr)
    {
        size_t bytesToCopy = (channel->data_size - channel->data_idx);

        if(bytesToCopy == 0)
        {
            if(channel->loops == 0)
            {
                closeChannel(channel);
                return true;
            }
            else
            {
                if(channel->loops > 0)
                    channel->loops -= 1;

                channel->data_idx = 0;
                bytesToCopy = channel->data_size;
            }
        }

        if(bytesToCopy > BUFFER_SIZE) bytesToCopy = BUFFER_SIZE;

        memcpy(wavebuf->data_pcm16, (char*)channel->data_pointer + channel->data_idx, bytesToCopy);
        channel->data_idx += bytesToCopy;

        if(channel->stereo)
        {
            wavebuf->nsamples = bytesToCopy / 4;
            ndspChnWaveBufAdd(channel->channel_id, (ndspWaveBuf*)wavebuf);
            DSP_FlushDataCache(wavebuf->data_pcm16, bytesToCopy);
        }
        else
        {
            wavebuf->nsamples = bytesToCopy / 2;
            ndspChnWaveBufAdd(channel->channel_id, (ndspWaveBuf*)wavebuf);
            DSP_FlushDataCache(wavebuf->data_pcm16, bytesToCopy);
        }

        return true;
    }

    if(channel->format == FORMAT_OGG_FILE && channel->data_pointer != nullptr)
    {
        char* buffer_loc = (char*)wavebuf->data_pcm16;  // in 2x16-bit samples
        wavebuf->nsamples = 0;
        size_t samplesToRead = BUFFER_SIZE;

        while(samplesToRead > 0)
        {
            // Decode bufferSize samples from opusFile_ into buffer,
            // storing the number of samples that were decoded (or error)
            int samples = ov_read((OggVorbis_File*)channel->data_pointer, buffer_loc, samplesToRead, nullptr);

            if(samples == 0)
            {
                if(channel->loops == 0)
                {
                    closeChannel(channel);
                    break;
                }
                else
                {
                    if(channel->loops > 0)
                        channel->loops -= 1;

                    ov_pcm_seek((OggVorbis_File*)channel->data_pointer, 0);
                }
            }

            wavebuf->nsamples += samples;
            buffer_loc += samples;
            samplesToRead -= samples;
        }

        // depends strongly on stereo-ness......
        if(channel->stereo)
        {
            wavebuf->nsamples /= 4;
            ndspChnWaveBufAdd(channel->channel_id, (ndspWaveBuf*)wavebuf);
            DSP_FlushDataCache(wavebuf->data_pcm16,
                               wavebuf->nsamples * 4);
        }
        else
        {
            wavebuf->nsamples /= 2;
            ndspChnWaveBufAdd(channel->channel_id, (ndspWaveBuf*)wavebuf);
            DSP_FlushDataCache(wavebuf->data_pcm16,
                               wavebuf->nsamples * 2);
        }

        return true;
    }

    if(channel->format == FORMAT_GME_FILE && channel->data_pointer != nullptr)
    {
        short* buffer_loc = (short*)wavebuf->data_pcm16;  // in 2x16-bit samples
        size_t samplesToRead = BUFFER_SIZE / sizeof(short);

        if(gme_track_ended((Music_Emu*)channel->data_pointer))
        {
            if(channel->loops == 0)
            {
                closeChannel(channel);
                return false;
            }
            else
            {
                if(channel->loops > 0)
                    channel->loops -= 1;

                gme_start_track((Music_Emu*)channel->data_pointer, 0);
            }
        }

        gme_play((Music_Emu*)channel->data_pointer, samplesToRead, buffer_loc);

        wavebuf->nsamples = samplesToRead / 2;
        ndspChnWaveBufAdd(channel->channel_id, (ndspWaveBuf*)wavebuf);
        DSP_FlushDataCache(wavebuf->data_pcm16,
                           wavebuf->nsamples * 4);
        return true;
    }

    return false;
}

void audioThread(void* /*nul_*/)
{
    while(!sound_quit)
    {
        for(size_t ci = 0; ci < NUM_CHANNELS; ci++)
        {
            volatile SimpleChannel* channel = &channels[ci];

            if(channel->kill && channel->format != FORMAT_LOADING)
            {
                closeChannel(channel);
                ndspChnWaveBufClear(channel->channel_id);

                channel->format = FORMAT_FREE;
                channel->kill = false;
            }
            else if(channel->format != FORMAT_FREE && channel->format != FORMAT_LOADING)
            {
                for(size_t wi = 0; wi < NUM_BUFFERS; wi++)
                {
                    ndspWaveBuf* wavebuf = (ndspWaveBuf*) & (channel->wavebufs[wi]);

                    if(wavebuf->status != NDSP_WBUF_DONE) continue;

                    if(!fillBuffer(channel, wavebuf) && !ndspChnIsPlaying(channel->channel_id))
                        channel->format = FORMAT_FREE;

                    // printf("Responding at %llu...\n", osGetTime());
                }
            }
        }

        LightEvent_Wait(&sound_event);
    }
}

int loadWaveFileData(FILE* f, uint16_t* nChannels, uint32_t* sampleRate, uint32_t* length, uint32_t* restart_pos)
{
    // all of this is little endian code
    fseek(f, 22, SEEK_SET);
    fread(nChannels, 2, 1, f);
    fread(sampleRate, 4, 1, f);

    uint32_t chunktype = 0;
    uint32_t chunksize = 0;

    int i = 0;

    // find correct header
    uint32_t read_pos = 36;

    do
    {
        read_pos += chunksize;
        fseek(f, read_pos, SEEK_SET);
        fread(&chunktype, 4, 1, f);
        fread(&chunksize, 4, 1, f);
        i++;
        read_pos += 8;
    }
    while(chunktype != 0x61746164 && i < 10);

    if(i == 10)
    {
        *nChannels = 0;
        chunksize = 0;
        read_pos = 0;
    }

    if(length)
        *length = chunksize;

    if(restart_pos)
        *restart_pos = read_pos;

    return 0;
}

SoundId playSoundWAV(const char* path, int loops)
{
    // printf("Playin sound %s\n", path);
    for(size_t ci = 0; ci < NUM_CHANNELS; ci++)
    {
        volatile SimpleChannel* channel = &channels[ci];

        if(channel->format != FORMAT_FREE) continue;

        FILE* f = fopen(path, "rb");

        if(!f)
            return INVALID_ID;

        uint16_t nChannels;
        uint32_t sampleRate;
        uint32_t restart_pos;

        loadWaveFileData(f, &nChannels, &sampleRate, nullptr, &restart_pos);

        if(nChannels == 0 || nChannels > 2 || restart_pos < 44)
        {
            fclose(f);
            return INVALID_ID;
        }

        ndspChnReset(ci);
        ndspChnSetInterp(ci, NDSP_INTERP_POLYPHASE);
        ndspChnSetRate(channel->channel_id, sampleRate);

        channel->data_pointer = f;
        channel->data_idx = restart_pos;

        if(nChannels == 2)
        {
            ndspChnSetFormat(channel->channel_id, NDSP_FORMAT_STEREO_PCM16);
            channel->stereo = true;
        }
        else
        {
            ndspChnSetFormat(channel->channel_id, NDSP_FORMAT_MONO_PCM16);
            channel->stereo = false;
        }

        channel->format = FORMAT_PCM_FILE;
        channel->loops = loops;
        LightEvent_Signal(&sound_event);
        return getSoundId(channel);
    }

    return INVALID_ID;
}

SoundId playSoundMem(const WaveObject* wave, int loops)
{
    for(size_t ci = 0; ci < NUM_CHANNELS; ci++)
    {
        volatile SimpleChannel* channel = &channels[ci];

        if(channel->format != FORMAT_FREE) continue;

        ndspChnReset(ci);
        ndspChnSetInterp(ci, NDSP_INTERP_POLYPHASE);
        channel->data_pointer = (void*) wave->data;
        channel->data_idx = 0;
        channel->data_size = wave->length;

        if(wave->stereo)
        {
            ndspChnSetFormat(channel->channel_id, NDSP_FORMAT_STEREO_PCM16);
            channel->stereo = true;
        }
        else
        {
            ndspChnSetFormat(channel->channel_id, NDSP_FORMAT_MONO_PCM16);
            channel->stereo = false;
        }

        ndspChnSetRate(channel->channel_id, wave->sampleRate);
        channel->format = FORMAT_PCM_MEM;
        channel->loops = loops;
        LightEvent_Signal(&sound_event);
        return getSoundId(channel);
    }

    return INVALID_ID;
}

void loadOGG_Thread(void* passed_val)
{
    volatile SimpleChannel* channel = (volatile SimpleChannel*) passed_val;
    FILE* f = (FILE*) channel->data_pointer;
    channel->data_pointer = nullptr;

    OggVorbis_File* vf = (OggVorbis_File*)malloc(sizeof(OggVorbis_File));

    if(!vf)
    {
        fclose(f);
        channel->format = FORMAT_FREE;
        return;
    }

    if(ov_open(f, vf, nullptr, 0))
    {
        free(vf);
        fclose(f);
        channel->format = FORMAT_FREE;
        return;
    }

    int ci = channel->channel_id;

    ndspChnReset(ci);
    ndspChnSetInterp(ci, NDSP_INTERP_POLYPHASE);

    vorbis_info* vi = ov_info(vf, -1);

    if(vi->channels == 2)
    {
        ndspChnSetFormat(channel->channel_id, NDSP_FORMAT_STEREO_PCM16);
        channel->stereo = true;
    }
    else if(vi->channels == 1)
    {
        ndspChnSetFormat(channel->channel_id, NDSP_FORMAT_MONO_PCM16);
        channel->stereo = false;
    }
    else
    {
        ov_clear(vf);
        free(vf);
        channel->format = FORMAT_FREE;
        return;
    }

    ndspChnSetRate(channel->channel_id, vi->rate);
    channel->data_pointer = (void*)vf;
    channel->format = FORMAT_OGG_FILE;
    LightEvent_Signal(&sound_event);
}

SoundId playSoundOGG(const char* path, int loops)
{
    // printf("Playin music %s\n", path);
    for(size_t ci = 0; ci < NUM_CHANNELS; ci++)
    {
        volatile SimpleChannel* channel = &channels[ci];

        if(channel->format != FORMAT_FREE) continue;

        FILE* f = fopen(path, "rb");

        if(!f) return INVALID_ID;

        channel->loops = loops;
        channel->format = FORMAT_LOADING;
        channel->data_pointer = (void*)f;
        SoundId uniquePlayId = getSoundId(channel);

        // Set the thread priority to the main thread's priority ...
        int32_t priority = 0x30;
        svcGetThreadPriority(&priority, CUR_THREAD_HANDLE);
        // ... then add 1, as higher number => lower actual priority ...
        priority += 1;
        // ... finally, clamp it between 0x18 and 0x3F to guarantee that it's valid.
        priority = priority < 0x18 ? 0x18 : priority;
        priority = priority > 0x3F ? 0x3F : priority;

        Thread load_thread;
        load_thread = threadCreate(loadOGG_Thread, (void*)channel,
                                   THREAD_STACK_SZ, priority,
                                   0, true);

        if(load_thread)
            return uniquePlayId;
        else
        {
            fclose(f);
            channel->data_pointer = nullptr;
            channel->format = FORMAT_FREE;
        }
    }

    return INVALID_ID;
}

// void loadChannelGME(volatile SimpleChannel* channel)
// {

// }

SoundId playSoundGME(const char* path, int loops)
{
    // printf("Playin music GME %s\n", path);
    for(size_t ci = 0; ci < NUM_CHANNELS; ci++)
    {
        volatile SimpleChannel* channel = &channels[ci];

        if(channel->format != FORMAT_FREE) continue;

        channel->format = FORMAT_LOADING;
        channel->loops = loops;
        gme_open_file(path, (Music_Emu**) & (channel->data_pointer), 24000);

        if(!channel->data_pointer)
        {
            channel->format = FORMAT_FREE;
            return INVALID_ID;
        }

        ndspChnReset(ci);
        ndspChnSetInterp(ci, NDSP_INTERP_POLYPHASE);

        ndspChnSetFormat(ci, NDSP_FORMAT_STEREO_PCM16);
        ndspChnSetRate(channel->channel_id, 24000);

        channel->format = FORMAT_GME_FILE;
        SoundId uniquePlayId = getSoundId(channel);
        return uniquePlayId;
    }

    return INVALID_ID;
}

// Thanks ThiefMaster!
// https://stackoverflow.com/questions/5309471/getting-file-extension-in-c
const char* get_filename_ext(const char* filename)
{
    const char* dot = strrchr(filename, '.');

    if(!dot || dot == filename) return "";

    return dot + 1;
}

SoundId playSoundAuto(const char* path, int loops)
{
    const char* ext = get_filename_ext(path);

    if(!strcasecmp(ext, "ogg"))
        return playSoundOGG(path, loops);

    if(!strcasecmp(ext, "mp3"))
        return playSoundOGG(path, loops);

    if(!strcasecmp(ext, "wav"))
        return playSoundWAV(path, loops);
    else
        return playSoundGME(path, loops);
}

bool audioInit()
{
    LightEvent_Init(&sound_event, RESET_ONESHOT);
    ndspInit();

    ndspSetOutputMode(NDSP_OUTPUT_STEREO);

    // Allocate audio buffer
    const size_t bufferSize = BUFFER_SIZE * NUM_CHANNELS * NUM_BUFFERS;
    audio_buffer = (char*)linearAlloc(bufferSize);

    if(!audio_buffer)
    {
        pLogWarning("Failed to allocate audio buffer");
        return false;
    }

    // Reset callback function
    channel_done_callback = nullptr;

    // initialize channels and wavebufs
    for(size_t ci = 0; ci < NUM_CHANNELS; ci++)
    {
        channels[ci].channel_id = ci;
        channels[ci].format = FORMAT_FREE;
        channels[ci].index = 1;
        channels[ci].data_pointer = NULL;

        ndspChnReset(ci);
        ndspChnSetInterp(ci, NDSP_INTERP_POLYPHASE);

        memset((void*) &channels[ci].wavebufs, 0, sizeof(channels[ci].wavebufs));

        for(size_t wi = 0; wi < NUM_BUFFERS; wi++)
        {
            channels[ci].wavebufs[wi].data_vaddr = audio_buffer + BUFFER_SIZE * (ci * NUM_BUFFERS + wi);
            channels[ci].wavebufs[wi].status = NDSP_WBUF_DONE;
        }
    }

    // begin setting up threading structure...
    ndspSetCallback(myAudioCallback, NULL);

    // Thanks to @mkst from sm64 3ds port
    int cpu = 0; // application core

    if(R_SUCCEEDED(APT_SetAppCpuTimeLimit(30)))
        cpu = 1; // system core

    // Set the thread priority to the main thread's priority ...
    int32_t priority = 0x30;
    svcGetThreadPriority(&priority, CUR_THREAD_HANDLE);
    // ... then subtract 1, as lower number => higher actual priority ...
    priority -= 1;
    // ... finally, clamp it between 0x18 and 0x3F to guarantee that it's valid.
    priority = priority < 0x18 ? 0x18 : priority;
    priority = priority > 0x3F ? 0x3F : priority;

    sound_thread = threadCreate(audioThread, NULL,
                                THREAD_STACK_SZ, priority,
                                cpu, true);

    if(sound_thread)
        pLogDebug("Created audio thread at %p on %s core", (void*)sound_thread, cpu ? "os" : "application");
    else
        pLogWarning("Failed to create audio thread");

    return (bool)sound_thread;
}

// Audio de-initialisation code
// Stops playback and frees the primary audio buffer
void audioExit()
{
    sound_quit = true;
    LightEvent_Signal(&sound_event);

    // Free the audio thread
    threadJoin(sound_thread, UINT64_MAX);
    threadFree(sound_thread);

    // reset channel done callback
    channel_done_callback = nullptr;

    for(size_t ci = 0; ci < NUM_CHANNELS; ci++)
    {
        closeChannel(&channels[ci]);
        ndspChnReset(channels[ci].channel_id);
    }

    linearFree(audio_buffer);

    ndspExit();
}

void audioPause()
{
    for(size_t ci = 0; ci < NUM_CHANNELS; ci++)
    {
        ndspChnSetPaused(ci, true);
    }
}

void audioResume()
{
    for(size_t ci = 0; ci < NUM_CHANNELS; ci++)
    {
        ndspChnSetPaused(ci, false);
    }
}

bool audioPauseSingle(SoundId soundId)
{
    volatile SimpleChannel* channel = validateSoundId(soundId);

    if(channel)
        ndspChnSetPaused(channel->channel_id, true);

    return channel;
}

bool audioResumeSingle(SoundId soundId)
{
    volatile SimpleChannel* channel = validateSoundId(soundId);

    if(channel)
        ndspChnSetPaused(channel->channel_id, false);

    return channel;
}

bool audioSoundPlaying(SoundId soundId)
{
    return validateSoundId(soundId);
}

void killSound(SoundId soundId)
{
    volatile SimpleChannel* channel = validateSoundId(soundId);

    if(channel) channel->kill = true;
}

WaveObject* audioLoadWave(const char* path)
{
    FILE* f = fopen(path, "rb");

    if(!f) return nullptr;

    WaveObject* wave = (WaveObject*) malloc(sizeof(WaveObject));
    uint16_t nChannels;

    loadWaveFileData(f, &nChannels, &wave->sampleRate, &wave->length, nullptr);

    if(nChannels == 2)
        wave->stereo = true;
    else if(nChannels == 1)
        wave->stereo = false;
    else
    {
        free(wave);
        fclose(f);
        return nullptr;
    }

    if(wave->length > 256 * 1024)
    {
        free(wave);
        fclose(f);
        return nullptr;
    }

    char* temp_buf = (char*) malloc(wave->length);

    if(!temp_buf)
    {
        free(wave);
        fclose(f);
        return nullptr;
    }

    wave->data = temp_buf;
    uint32_t to_read = wave->length;

    while(to_read)
    {
        unsigned int samples_read = fread(temp_buf, 1, to_read, f);

        if(!samples_read) break;

        to_read -= samples_read;
        temp_buf += samples_read;
    }

    fclose(f);

    if(!to_read)
    {
        return wave;
    }
    else
    {
        free((void*)wave->data);
        free(wave);
        return nullptr;
    }
}

void audioFreeWave(WaveObject* wave)
{
    free((void*)wave->data);
    free(wave);
}

void audioSetChannelDoneCallback(void (*cb)(int))
{
    channel_done_callback = cb;
}
