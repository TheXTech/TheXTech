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

#include "pge_video_rec.h"
#include <SDL2/SDL_mutex.h>

PGE_VideoSink::PGE_VideoSink()
{
    mutex_frame = SDL_CreateMutex();
    mutex_audio = SDL_CreateMutex();
}

PGE_VideoSink::~PGE_VideoSink()
{
    if(mutex_frame)
        SDL_DestroyMutex(mutex_frame);

    mutex_frame = nullptr;

    if(mutex_audio)
        SDL_DestroyMutex(mutex_audio);

    mutex_audio = nullptr;
}

bool PGE_VideoSink::has_frame()
{
    SDL_LockMutex(mutex_frame);
    bool ret = !queue_frame.empty();
    SDL_UnlockMutex(mutex_frame);
    return ret;
}

int PGE_VideoSink::frame_backlog()
{
    SDL_LockMutex(mutex_frame);
    int ret = queue_frame.size();
    SDL_UnlockMutex(mutex_frame);
    return ret;
}

int PGE_VideoSink::enqueue_frame(PGE_VideoFrame&& frame, int max_backlog)
{
    SDL_LockMutex(mutex_frame);
    int backlog_size = queue_frame.size();

    if(max_backlog >= 0 && backlog_size < max_backlog)
    {
        queue_frame.push_back(std::move(frame));
        backlog_size += 1;
    }

    SDL_UnlockMutex(mutex_frame);

    return backlog_size;
}

PGE_VideoFrame PGE_VideoSink::dequeue_frame()
{
    SDL_LockMutex(mutex_frame);
    PGE_VideoFrame ret = std::move(queue_frame.front());
    queue_frame.pop_front();
    SDL_UnlockMutex(mutex_frame);
    return ret;
}

bool PGE_VideoSink::has_audio()
{
    SDL_LockMutex(mutex_audio);
    bool ret = !queue_audio.empty();
    SDL_UnlockMutex(mutex_audio);
    return ret;
}

void PGE_VideoSink::enqueue_audio(PGE_AudioChunk&& chunk)
{
    SDL_LockMutex(mutex_audio);
    queue_audio.push_back(std::move(chunk));
    SDL_UnlockMutex(mutex_audio);
}

PGE_AudioChunk PGE_VideoSink::dequeue_audio()
{
    SDL_LockMutex(mutex_audio);
    PGE_AudioChunk ret = std::move(queue_audio.front());
    queue_audio.pop_front();
    SDL_UnlockMutex(mutex_audio);
    return ret;
}
