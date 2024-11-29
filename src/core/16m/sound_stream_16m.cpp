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


#include <SDL2/SDL_rwops.h>
#include <maxmod9.h>

#include "core/16m/sound_stream_16m.h"

#define QOA_IMPLEMENTATION
#define QOA_NO_STDIO
#include "core/16m/qoa.h"


static constexpr size_t s_max_qoa_frame_size = QOA_FRAME_SIZE(2, QOA_SLICES_PER_FRAME);

struct Sound_Stream
{
    SDL_RWops* rwops = nullptr;
    qoa_desc desc;
    uint8_t encoded_frame[s_max_qoa_frame_size];
    unsigned int encoded_bytes = 0;
    int first_frame_filepos = 0;
};

static Sound_Stream s_stream;

void Sound_StreamLoadData();;


mm_word s_fill_stream(mm_word length, mm_addr dest, mm_stream_formats /*format*/)
{
    if(length < QOA_FRAME_LEN)
        return 0;

#ifdef __CALICO__
    if(!s_stream.encoded_bytes)
        Sound_StreamLoadData();
#endif

    if(!s_stream.encoded_bytes)
        return 0;

    unsigned int samples_decoded;
    qoa_decode_frame(s_stream.encoded_frame, s_stream.encoded_bytes, &s_stream.desc, (int16_t*)dest, &samples_decoded);

    // required a multiple of 4
    samples_decoded &= ~3;

    s_stream.encoded_bytes = 0;

    return samples_decoded;
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

    uint8_t qoa_header[QOA_MIN_FILESIZE];
    int header_read = SDL_RWread(rwops, qoa_header, QOA_MIN_FILESIZE, 1);
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

    SDL_RWseek(rwops, s_stream.first_frame_filepos, RW_SEEK_SET);
    s_stream.rwops = rwops;
    s_stream.encoded_bytes = 0;


    // qoa header successfully loaded

    mm_stream stream;

    stream.sampling_rate = s_stream.desc.samplerate;
    stream.buffer_length = QOA_FRAME_LEN * 2;
    stream.callback = s_fill_stream;

    stream.format = MM_STREAM_16BIT_STEREO;
    if(s_stream.desc.channels == 1)
        stream.format = MM_STREAM_16BIT_MONO;

#ifdef __CALICO__
    stream.timer = 0;
    stream.manual = 0;
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

    s_stream.encoded_bytes = SDL_RWread(s_stream.rwops, s_stream.encoded_frame, 1, frame_size);

    while(s_stream.encoded_bytes != 0 && s_stream.encoded_bytes != frame_size)
    {
        int new_bytes = SDL_RWread(s_stream.rwops, s_stream.encoded_frame + s_stream.encoded_bytes, 1, frame_size - s_stream.encoded_bytes);

        if(new_bytes == 0)
            break;
        else
            printf("Stream: read %d bytes first and %d bytes second.", (int)s_stream.encoded_bytes, new_bytes);

        s_stream.encoded_bytes += new_bytes;
    }

    if(s_stream.encoded_bytes != frame_size)
    {
        // return to stream start
        SDL_RWseek(s_stream.rwops, s_stream.first_frame_filepos, RW_SEEK_SET);

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
