/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2026 Vitaly Novichkov <admin@wohlnet.ru>
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


#include <SDL2/SDL_rwops.h>
#include <string.h>
#include <maxmod9.h>

#ifdef __CALICO__
#include <calico.h>
#endif

#include "core/16m/sound_stream_16m.h"

#define QOA_IMPLEMENTATION
#define QOA_NO_STDIO
#include "core/16m/qoa.h"


static constexpr size_t s_max_qoa_frame_size = QOA_FRAME_SIZE(2, QOA_SLICES_PER_FRAME);
static constexpr size_t s_max_decoded_frame_size = 4 * QOA_FRAME_LEN;

struct Sound_Stream
{
    SDL_RWops* rwops = nullptr;
    qoa_desc desc;
    uint8_t encoded_frame[s_max_qoa_frame_size];
    unsigned int encoded_bytes = 0;

    uint8_t decoded_frame[s_max_decoded_frame_size];
    unsigned int decoded_start = 0;
    unsigned int decoded_end = 0;

    int first_frame_filepos = 0;
    uint32_t sample_pos = 0;

    uint32_t loop_start = 0;
    uint32_t loop_end = 0;

    // clip the current frame
    uint32_t play_start = 0;
};

static Sound_Stream s_stream;

static void s_rewind()
{
    size_t frame_size = s_max_qoa_frame_size;
    if(s_stream.desc.channels == 1)
        frame_size = QOA_FRAME_SIZE(1, QOA_SLICES_PER_FRAME);

    s_stream.sample_pos = s_stream.loop_start;
    unsigned want_frame = (s_stream.loop_start / QOA_FRAME_LEN);
    SDL_RWseek(s_stream.rwops, s_stream.first_frame_filepos + frame_size * want_frame, SEEK_SET);

    s_stream.play_start = s_stream.loop_start - (want_frame * QOA_FRAME_LEN);
}

void Sound_StreamLoadData();


mm_word s_fill_stream(mm_word length, mm_addr dest, mm_stream_formats /*format*/)
{
    if(s_stream.decoded_start >= s_stream.decoded_end)
    {
#ifdef __CALICO__
        if(!s_stream.encoded_bytes)
            Sound_StreamLoadData();
#endif

        if(!s_stream.encoded_bytes)
            return 0;

        unsigned int samples_decoded;
        qoa_decode_frame(s_stream.encoded_frame, s_stream.encoded_bytes, &s_stream.desc, (int16_t*)s_stream.decoded_frame, &samples_decoded);

        s_stream.decoded_start = 0;
        s_stream.decoded_end = samples_decoded * 2 * s_stream.desc.channels;

        if(s_stream.play_start)
        {
            samples_decoded -= s_stream.play_start;

            s_stream.decoded_start += s_stream.play_start * 2 * s_stream.desc.channels;

            s_stream.play_start = 0;
        }

        s_stream.sample_pos += samples_decoded;

        // clip to loop_end if reached
        if(s_stream.loop_end && s_stream.sample_pos > s_stream.loop_end)
            s_stream.decoded_end -= (s_stream.sample_pos - s_stream.loop_end) * 2 * s_stream.desc.channels;

        s_stream.encoded_bytes = 0;
    }

    if(s_stream.decoded_start >= s_stream.decoded_end)
        return 0;

    unsigned copy_bytes = length * 2 * s_stream.desc.channels;

    if(s_stream.decoded_end - s_stream.decoded_start < copy_bytes)
        copy_bytes = s_stream.decoded_end - s_stream.decoded_start;

    memcpy(dest, s_stream.decoded_frame + s_stream.decoded_start, copy_bytes);
    s_stream.decoded_start += copy_bytes;

    unsigned got_samples = copy_bytes / (2 * s_stream.desc.channels);

    // required a multiple of 4
    got_samples &= ~3;

    return got_samples;
}

void Sound_StreamStart(SDL_RWops* rwops)
{
    if(!rwops)
        return;

    if(s_stream.rwops)
    {
        SDL_RWclose(rwops);
        return;
    }

    memset((void*)&s_stream, 0, sizeof(Sound_Stream));

    // check whether we have an XQOA file
    const uint32_t XQOA_MAGIC = 0x58514f41; /* 'XQOA' */
    uint32_t xqoa_headsize = 0;
    uint8_t qoa_header[QOA_MIN_FILESIZE];
    int header_read = SDL_RWread(rwops, qoa_header, 8, 1);
    if(!header_read)
    {
        SDL_RWclose(rwops);
        return;
    }

    unsigned int junk_counter = 0;
    qoa_uint64_t top = qoa_read_u64(qoa_header, &junk_counter);
    if(top >> 32 == XQOA_MAGIC)
    {
        // XQOA header format (32-bit big endian unsigned ints)
        // magic, header_size, data_size, loop_start, loop_end, multitrack_channels, multitrack_tracks, <...tags...>
        xqoa_headsize = (uint32_t)(top);

        // read 16 bytes to get data including loop info
        header_read = SDL_RWread(rwops, qoa_header, QOA_MIN_FILESIZE, 1);
        junk_counter = 0;
        qoa_uint64_t loop_info = qoa_read_u64(qoa_header + 4, &junk_counter);

        s_stream.loop_start = (uint32_t)(loop_info >> 32);
        s_stream.loop_end = (uint32_t)(loop_info);

        // seek and re-read genuine QOA header
        SDL_RWseek(rwops, xqoa_headsize, RW_SEEK_SET);
        header_read = SDL_RWread(rwops, qoa_header, 8, 1);
    }

    header_read = SDL_RWread(rwops, qoa_header + 8, QOA_MIN_FILESIZE - 8, 1);
    if(!header_read)
    {
        SDL_RWclose(rwops);
        return;
    }

    s_stream.first_frame_filepos = qoa_decode_header(qoa_header, QOA_MIN_FILESIZE, &s_stream.desc);

    if(!s_stream.first_frame_filepos || s_stream.desc.channels > 2)
    {
        SDL_RWclose(rwops);
        return;
    }

    s_stream.first_frame_filepos += xqoa_headsize;

    SDL_RWseek(rwops, s_stream.first_frame_filepos, RW_SEEK_SET);
    s_stream.rwops = rwops;

    // qoa header successfully loaded

    mm_stream stream;

    stream.sampling_rate = s_stream.desc.samplerate;
    stream.buffer_length = QOA_FRAME_LEN;
    stream.callback = s_fill_stream;

    stream.format = MM_STREAM_16BIT_STEREO;
    if(s_stream.desc.channels == 1)
        stream.format = MM_STREAM_16BIT_MONO;

#ifdef __CALICO__
    stream.timer = 0;
    stream.manual = 0;
    stream.minus_thread_prio = -(MAIN_THREAD_PRIO - 1); // priority 1 higher than main thread
#else
    stream.timer = MM_TIMER3;
    stream.manual = 1;
#endif

    // open the stream
    mmStreamOpen(&stream);

#ifndef __CALICO__
    // start playing
    Sound_StreamUpdate();
    Sound_StreamUpdate();
#endif
}

void Sound_StreamUpdate()
{
    if(!s_stream.rwops)
        return;

#ifndef __CALICO__
    if(s_stream.encoded_bytes)
        mmStreamUpdate();
    else
        Sound_StreamLoadData();
#endif
}

void Sound_StreamLoadData()
{
    if(!s_stream.rwops)
        return;

    size_t frame_size = s_max_qoa_frame_size;
    if(s_stream.desc.channels == 1)
        frame_size = QOA_FRAME_SIZE(1, QOA_SLICES_PER_FRAME);

    if(!s_stream.loop_end || s_stream.sample_pos < s_stream.loop_end)
        s_stream.encoded_bytes = SDL_RWread(s_stream.rwops, s_stream.encoded_frame, 1, frame_size);

    while(s_stream.encoded_bytes != 0 && s_stream.encoded_bytes != frame_size)
    {
        int new_bytes = SDL_RWread(s_stream.rwops, s_stream.encoded_frame + s_stream.encoded_bytes, 1, frame_size - s_stream.encoded_bytes);

        if(new_bytes == 0)
            break;
        // else
        //     pLogDebug("Stream: read %d bytes first and %d bytes second.", (int)s_stream.encoded_bytes, new_bytes);

        s_stream.encoded_bytes += new_bytes;
    }

    if(s_stream.encoded_bytes != frame_size)
    {
        // return to stream/loop start
        s_rewind();

        // try to read again if there wasn't anything left
        if(s_stream.encoded_bytes == 0)
            s_stream.encoded_bytes = SDL_RWread(s_stream.rwops, s_stream.encoded_frame, 1, frame_size);

        // if still nothing, then panic and close the file
        if(s_stream.encoded_bytes == 0)
        {
            SDL_RWclose(s_stream.rwops);
            s_stream.rwops = nullptr;
        }
    }
}

void Sound_StreamStop()
{
    mmStreamClose();

    if(s_stream.rwops)
    {
        SDL_RWclose(s_stream.rwops);
        s_stream.rwops = nullptr;
    }
}
