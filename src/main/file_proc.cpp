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

#include <deque>
#include <mutex>
#include "file_proc.h"
#include <SDL_stdinc.h>
#include <SDL_thread.h>
#include <SDL_atomic.h>
#include "core/render.h"

#include <Utils/files.h>
#include <Logger/logger.h>
#include <fmt_format_ne.h>
#include <inttypes.h>
#include <Utils/elapsed_timer.h>
#include <AppPath/app_path.h>

enum FileTaskType
{
    FILE_TASK_WRITE,
    FILE_TASK_DELETE
};

struct FileTask
{
    FileTaskType    type;
    uint8_t         *out_data;
    size_t          out_data_size;
    std::string     out_path;
};

static std::deque<FileTask> s_tasksQueue;
static std::mutex s_tasksQueueMutex;
static SDL_Thread *s_queueThread = NULL;
static SDL_atomic_t s_queueIsActive = {0};

static int s_fileQueue(void *)
{
    FileTask task;
    ElapsedTimer timer;

    while(true)
    {
        s_tasksQueueMutex.lock();
        if(s_tasksQueue.empty())
            break;

        task = s_tasksQueue.front();
        s_tasksQueue.pop_front();
        s_tasksQueueMutex.unlock();

        switch(task.type)
        {
        case FILE_TASK_WRITE:
        {
            // std::string log_out;
            timer.restart();
            SDL_RWops *dst = SDL_RWFromFile(task.out_path.c_str(), "wb");
            pLogDebug("- File Queue: FILE_TASK_WRITE: SDL_RWFromFile [%s]: %d ms (%" PRId64 " ns)\n", task.out_path.c_str(), timer.elapsed(), timer.nanoelapsed());

            if(dst)
            {
#if defined(__3DS__)
                char buffer[65536];
                setvbuf(dst->hidden.stdio.fp, buffer, _IOFBF, sizeof(buffer));
#endif

                timer.restart();
                SDL_RWwrite(dst, task.out_data, 1, task.out_data_size);
                pLogDebug("- File Queue: FILE_TASK_WRITE: SDL_RWwrite: %d ms (%" PRId64 " ns)\n", timer.elapsed(), timer.nanoelapsed());

#if defined(__3DS__)
                timer.restart();
                std::fflush(dst->hidden.stdio.fp);
                pLogDebug("- File Queue: FILE_TASK_WRITE: fflush: %d ms (%" PRId64 " ns)\n", timer.elapsed(), timer.nanoelapsed());
#endif

                timer.restart();
                SDL_RWclose(dst);
                /*log_out += fmt::sprintf_ne*/
                pLogDebug("- File Queue: FILE_TASK_WRITE: SDL_RWclose: %d ms (%" PRId64 " ns)\n", timer.elapsed(), timer.nanoelapsed());

                SDL_free(task.out_data);
            }

            AppPathManager::syncFs();
            // pLogDebug("Log messages:\n%s", log_out.c_str());
            break;
        }
        case FILE_TASK_DELETE:
        {
            timer.restart();
            if(Files::fileExists(task.out_path))
            {
                pLogDebug("File Queue: FILE_TASK_DELETE: fileExists (ret true): %d ms (%" PRId64 " ns)", timer.elapsed(), timer.nanoelapsed());

                timer.restart();
                Files::deleteFile(task.out_path);
                pLogDebug("File Queue: FILE_TASK_DELETE: deleteFile: %d ms (%" PRId64 " ns)", timer.elapsed(), timer.nanoelapsed());
            }
            else
            {
                pLogDebug("File Queue: FILE_TASK_DELETE: fileExists (ret false): %d ms (%" PRId64 " ns)", timer.elapsed(), timer.nanoelapsed());
            }

            AppPathManager::syncFs();
            break;
        }
        }
    }

    SDL_AtomicSet(&s_queueIsActive, 0);
    s_tasksQueueMutex.unlock();

    return 0;
}

bool FileProc::isActive()
{
    return SDL_AtomicGet(&s_queueIsActive);
}

void FileProc::waitFinished()
{
    if(s_queueThread && SDL_AtomicGet(&s_queueIsActive))
        SDL_WaitThread(s_queueThread, NULL);
}

void FileProc::drawDisquette()
{
    const int w = 32, h = 32;
    const int dstX = XRender::TargetW - (w + XRender::TargetCameraOverscanX);
    const int dstY = XRender::TargetH - h;
    static bool alpha = false;

    alpha = !alpha;

    // Colours
    XTColor c_blue(0, 0, 255, alpha ? 255 : 0);
    XTColor c_black(0, 0, 0, alpha ? 255 : 0);
    XTColor c_grey(175, 175, 175, alpha ? 255 : 0);
    XTColor c_white(255, 255, 255, alpha ? 255 : 0);

    // Draw the blue surface
    XRender::renderRect(dstX, dstY, w - 4, h, c_blue, true);
    XRender::renderRect(dstX + w - 4, dstY + 2, 2, h - 2, c_blue, true);

    // Draw the metallic protector
    XRender::renderRect(dstX + 8, dstY, 16, 12, c_grey, true);
    XRender::renderRect(dstX + 18, dstY + 4, 4, 6, c_blue, true);

    // Draw the paper sticker
    XRender::renderRect(dstX + 4, dstY + 16, 24, 16, c_white, true);
    // Imitation of the text label
    XRender::renderRect(dstX + 6, dstY + 18, 20, 2, c_grey, true);
    XRender::renderRect(dstX + 6, dstY + 22, 20, 2, c_grey, true);
    XRender::renderRect(dstX + 6, dstY + 26, 20, 2, c_grey, true);

    // Draw borders

    // Borders of disquette itself
    XRender::renderRect(dstX, dstY, w - 4, 2, c_black, true);
    XRender::renderRect(dstX + w - 4, dstY + 2, 2, 2, c_black, true);
    XRender::renderRect(dstX + w - 2, dstY + 4, 2, h - 4, c_black, true);
    XRender::renderRect(dstX, dstY, 2, h, c_black, true);
    XRender::renderRect(dstX, dstY + h - 2, w, 2, c_black, true);

    // Borders of Metallic protector
    XRender::renderRect(dstX + 6, dstY + 2, 2, 10, c_black, true);
    XRender::renderRect(dstX + 24, dstY + 2, 2, 10, c_black, true);
    XRender::renderRect(dstX + 8, dstY + 12, 16, 2, c_black, true);
}

static void s_launch()
{
    if(!SDL_AtomicGet(&s_queueIsActive))
    {
        SDL_AtomicSet(&s_queueIsActive, 1);
        s_queueThread = SDL_CreateThread(&s_fileQueue, "FileSaveQueue", NULL);
    }
}

void FileProc::addFileSave(const uint8_t *data, size_t size, const std::string &outPath)
{
    s_tasksQueueMutex.lock();

    FileTask task;
    task.out_data = (uint8_t*)SDL_malloc(size);
    SDL_memcpy(task.out_data, data, size);
    task.out_data_size = size;
    task.out_path = outPath;
    task.type = FILE_TASK_WRITE;
    s_tasksQueue.push_back(std::move(task));
    s_launch();

    s_tasksQueueMutex.unlock();
}

void FileProc::addDeleteFile(const std::string &outPath)
{
    s_tasksQueueMutex.lock();

    FileTask task;
    task.out_path = outPath;
    task.type = FILE_TASK_DELETE;
    s_tasksQueue.push_back(std::move(task));
    s_launch();

    s_tasksQueueMutex.unlock();
}
