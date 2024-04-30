/*
 *
 * Copyright (c) 2020-2024 ds-sloth and Vitaly Novichkov <admin@wohlnet.ru>
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

#include <SDL2/SDL_mutex.h>
#include <Utils/files.h>
#include <pge_delay.h>

#include "gif.h"

#include "pge_video_rec.h"


struct PGE_VideoRecording_GIF : public PGE_VideoRecording
{
    GIF_H::GifWriter  writer      = {nullptr, nullptr, true, false};
    unsigned char padding[7] = {0, 0, 0, 0, 0, 0, 0};

    // returns the best file extension for the recording type
    virtual const char* extension() const override;

    // should be called by a main thread, returns false on failure
    virtual bool initialize(const char* filename) override;

    // should be called by an encoding thread, terminates once the empty end frame has been dequeued.
    virtual bool encoding_thread() override;
};

const char* PGE_VideoRecording_GIF::extension() const
{
    return "gif";
}

bool PGE_VideoRecording_GIF::initialize(const char* filename)
{
    FILE *gifFile = Files::utf8_fopen(filename, "wb");
    return GIF_H::GifBegin(&writer, gifFile, spec.frame_w, spec.frame_h, 100 / spec.frame_rate, false);
}

bool PGE_VideoRecording_GIF::encoding_thread()
{
    (void)(GIF_H::GifOverwriteLastDelay);// shut up a warning about unused function

    int frame_i = 0;

    while(true)
    {
        if(!has_frame())
        {
            if(exit_requested)
                break;

            PGE_Delay(1);
            continue;
        }

        auto sh = dequeue_frame();

        if(sh.end_frame)
            break;

        GifWriteFrame(&writer, sh.pixels.data(),
                      unsigned(spec.frame_w),
                      unsigned(spec.frame_h),
                      ((frame_i + 1) * 100 / spec.frame_rate) - (frame_i * 100 / spec.frame_rate), 8, false);

        frame_i++;
    }

    // Once GIF recorder was been disabled, finalize it
    GIF_H::GifEnd(&writer);

    return true;
}

std::unique_ptr<PGE_VideoRecording> PGE_new_recording_GIF(const PGE_VideoSpec& spec)
{
    std::unique_ptr<PGE_VideoRecording> ret(new PGE_VideoRecording_GIF());
    ret->spec = spec;
    ret->spec.audio_enabled = false;
    return ret;
}
