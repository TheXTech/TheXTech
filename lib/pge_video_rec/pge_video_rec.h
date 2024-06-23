/*
 *
 * Copyright (c) 2024 ds-sloth and Vitaly Novichkov <admin@wohlnet.ru>
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
#ifndef PGE_VIDEO_REC_H
#define PGE_VIDEO_REC_H

#include <cstdint>
#include <memory>
#include <vector>
#include <deque>

struct SDL_mutex;
struct SDL_Thread;

struct PGE_VideoFrame
{
    std::vector<uint8_t> pixels;
    uint64_t timestamp = 0;
    bool end_frame = false;

    PGE_VideoFrame(const PGE_VideoFrame&) = delete;
    PGE_VideoFrame& operator=(const PGE_VideoFrame&) = delete;

    PGE_VideoFrame() = default;
    PGE_VideoFrame(PGE_VideoFrame&&) = default;
    PGE_VideoFrame& operator=(PGE_VideoFrame&&) = default;
};

struct PGE_AudioChunk
{
    std::vector<uint8_t> audio_buffer;

    PGE_AudioChunk(const PGE_AudioChunk&) = delete;
    PGE_AudioChunk& operator=(const PGE_AudioChunk&) = delete;

    PGE_AudioChunk() = default;
    PGE_AudioChunk(PGE_AudioChunk&&) = default;
    PGE_AudioChunk& operator=(PGE_AudioChunk&&) = default;
};

struct PGE_VideoSpec
{
    //! width of frame in pixels
    int frame_w = 0;

    //! height of frame in pixels
    int frame_h = 0;

    //! pitch of frame in bytes
    int frame_pitch = 0;

    //! video framerate in FPS
    int frame_rate = 0;

    //! video quality for FFMPEG. lower is better
    int video_quality = 10;

    //! whether video should be scaled down by 2x
    bool downscale_video = false;

    //! whether the video should include audio
    bool audio_enabled = false;

    //! audio sample rate in Hz
    int audio_sample_rate = 0;

    //! audio channel count
    int audio_channel_count = 0;
};


/*!
 * \brief Abstract class representing a video sink (implemented in pge_video_sink.cpp)
 */
class PGE_VideoSink
{
private:
    std::deque<PGE_VideoFrame> queue_frame;
    SDL_mutex* mutex_frame = nullptr;

    std::deque<PGE_AudioChunk> queue_audio;
    SDL_mutex* mutex_audio = nullptr;

public:
    PGE_VideoSpec spec;

    PGE_VideoSink();
    virtual ~PGE_VideoSink();

    bool has_frame();
    int frame_backlog();
    void enqueue_frame(PGE_VideoFrame&& frame);
    PGE_VideoFrame dequeue_frame();

    bool has_audio();
    void enqueue_audio(PGE_AudioChunk&& chunk);
    PGE_AudioChunk dequeue_audio();
};

/*!
 * \brief Abstract class representing a video recording
 */
struct PGE_VideoRecording : public PGE_VideoSink
{
    bool exit_requested = false;

    // returns the best file extension for the recording type
    virtual const char* extension() const = 0;

    // should be called by a main thread, returns false on failure
    virtual bool initialize(const char* filename) = 0;

    // should be called by an encoding thread, terminates once the empty end frame has been dequeued.
    virtual bool encoding_thread() = 0;
};

std::unique_ptr<PGE_VideoRecording> PGE_new_recording_GIF(const PGE_VideoSpec& spec);
std::unique_ptr<PGE_VideoRecording> PGE_new_recording_VP8(const PGE_VideoSpec& spec);

#endif // PGE_VIDEO_REC_H
