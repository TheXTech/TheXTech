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

#ifdef __3DS__
#   include <unistd.h>
#   include <sys/types.h>
#   include <sys/stat.h>
#   include <fcntl.h>
#endif

#include <mutex>
#include <memory>
#include "file_proc.h"
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_thread.h>
#include <SDL2/SDL_atomic.h>
#include <SDL2/SDL_rwops.h>
#include "core/render.h"
#include <Utils/files.h>
#include <Logger/logger.h>
#include <AppPath/app_path.h>


enum FileTaskType
{
    FILE_TASK_WRITE,
    FILE_TASK_DELETE
};

struct FileTask
{
    FileTaskType    type;
    std::string     out_data; // NOTE: may store binary data
    std::string     out_path;

    struct FileTask *next = NULL;
};

static FileTask *s_tasksQueue = NULL;

// receives ownership of in
static void s_tasksEnque(FileTask *in)
{
    FileTask** tail = &s_tasksQueue;
    while(*tail)
        tail = &((*tail)->next);

    *tail = in;
}

// yields ownership of return value
static FileTask *s_tasksDeque(void)
{
    FileTask *ret = s_tasksQueue;

    if(ret)
    {
        s_tasksQueue = ret->next;
        ret->next = NULL;
    }

    return ret;
}

static SDL_mutex* s_tasksQueueMutex = nullptr;
static bool s_queueThreadCheckingEntries = false;
static SDL_Thread *s_queueThread = NULL;

static int s_fileQueue(void *)
{
    FileTask *task;

    while(true)
    {
        // SYNCHRONIZED SECTION: find task, or if no tasks are present, signal shutdown to main thread
        SDL_LockMutex(s_tasksQueueMutex);
        task = s_tasksDeque();

        if(!task)
            s_queueThreadCheckingEntries = false;

        SDL_UnlockMutex(s_tasksQueueMutex);

        if(!task)
            break;

        // END SYNCHRONIZED SECTION

        switch(task->type)
        {
        case FILE_TASK_WRITE:
        {
#ifdef __3DS__
            int dst = open(task->out_path.c_str(), O_WRONLY | O_CREAT | O_DIRECT);

            if(dst)
            {
                ssize_t wrote = write(dst, task->out_data.c_str(), task->out_data.size());

                if(wrote < (ssize_t)task->out_data.size())
                    pLogWarning("Failed to write data into file: %s", task->out_path.c_str());
                else
                    ftruncate(dst, wrote);

                close(dst);
            }
            else
                pLogWarning("Unable to open file for write: %s", task->out_path.c_str());
#else
            std::string tmpPath = task->out_path + ".new.tmp";
            SDL_RWops *dst = SDL_RWFromFile(tmpPath.c_str(), "wb");

            if(dst)
            {
                size_t wrote = SDL_RWwrite(dst, task->out_data.c_str(), 1, task->out_data.size());

                if(wrote < task->out_data.size())
                    pLogWarning("Failed to write data into file: %s", tmpPath.c_str());

                SDL_RWclose(dst);

                if(wrote == task->out_data.size())
                {
                    if(Files::fileExists(task->out_path))
                        Files::deleteFile(task->out_path);

                    // Put a new file instead of old one, but only valid one!
                    Files::moveFile(task->out_path, tmpPath, true);
                    AppPathManager::syncFs();
                }
                else
                    Files::deleteFile(tmpPath); // Delete invalid temp file
            }
            else
                pLogWarning("Unable to open file for write: %s", tmpPath.c_str());
#endif
            break;
        }
        case FILE_TASK_DELETE:
        {
#ifdef __3DS__
            if(access(task->out_path.c_str(), F_OK) == 0)
            {
                if(unlink(task->out_path.c_str()) < 0)
                    pLogWarning("Unable to delete a file: %s", task->out_path.c_str());
            }
#else
            if(Files::fileExists(task->out_path))
                Files::deleteFile(task->out_path);
#endif
            break;
        }
        }

        if(task)
            delete task;

        AppPathManager::syncFs();
    }

    return 0;
}

bool FileProc::isActive()
{
    return s_queueThreadCheckingEntries;
}

void FileProc::waitFinished()
{
    if(s_queueThread)
    {
        SDL_WaitThread(s_queueThread, NULL);
        s_queueThread = NULL;
    }

    if(s_tasksQueueMutex)
    {
        SDL_DestroyMutex(s_tasksQueueMutex);
        s_tasksQueueMutex = nullptr;
    }
}

void FileProc::drawDisquette()
{
    const int w = 32, h = 32;
    const int dstX = XRender::TargetW - (w + XRender::TargetCameraOverscanX);
    const int dstY = XRender::TargetH - h;
    static bool alpha = false;

    alpha = !alpha;

    // Colours
    XTColor c_blue(0, 0, 255, alpha ? 255 : 128);
    XTColor c_black(0, 0, 0, alpha ? 255 : 128);
    XTColor c_grey(175, 175, 175, alpha ? 255 : 128);
    XTColor c_white(255, 255, 255, alpha ? 255 : 128);

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

// TO BE CALLED FROM MAIN THREAD
static void s_enqueueAndRun(FileTask* task) // receives ownership of task
{
    // SYNCHRONIZED SECTION
    if(!s_tasksQueueMutex)
        s_tasksQueueMutex = SDL_CreateMutex();

    SDL_LockMutex(s_tasksQueueMutex);

    s_tasksEnque(task); // OWNERSHIP TRANSFER

    // IMPORTANT: this is called while s_tasksQueueMutex is held, so
    // if s_queueThreadCheckingEntries is true, the added entry WILL be checked
    if(!s_queueThreadCheckingEntries)
    {
        if(s_queueThread)
            SDL_WaitThread(s_queueThread, NULL); // Finalise previous thread

        s_queueThread = SDL_CreateThread(&s_fileQueue, "FileSaveQueue", NULL);

        // no race condition: the thread won't enter its first synchronized section
        // until after s_tasksQueueMutex is released
        if(s_queueThread)
            s_queueThreadCheckingEntries = true;
    }

    SDL_UnlockMutex(s_tasksQueueMutex);

    // force the task to run synchronously if thread creation failed
    if(!s_queueThread)
        s_fileQueue(NULL);
}

void FileProc::addFileSave(std::string&& data, const std::string &outPath)
{
    FileTask *task = new FileTask;
    task->out_data = std::move(data);
    task->out_path = outPath; // NOTE: memory leak if this throws
    task->type = FILE_TASK_WRITE;

    s_enqueueAndRun(task);
}

void FileProc::addDeleteFile(const std::string &outPath)
{
    FileTask *task = new FileTask;
    task->out_path = outPath; // NOTE: memory leak if this throws
    task->type = FILE_TASK_DELETE;

    s_enqueueAndRun(task);
}
