/*
 * Moondust, a free game engine for platform game making
 * Copyright (c) 2014-2022 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * This software is licensed under a dual license system (MIT or GPL version 3 or later).
 * This means you are free to choose with which of both licenses (MIT or GPL version 3 or later)
 * you want to use this software.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You can see text of MIT license in the LICENSE.mit file you can see in Engine folder,
 * or see https://mit-license.org/.
 *
 * You can see text of GPLv3 license in the LICENSE.gpl3 file you can see in Engine folder,
 * or see <http://www.gnu.org/licenses/>.
 */

#ifndef LOGGER_INTERNAL
#   error THIS FILE MUST NOT BE INCLUDED OUTSIDE THE LOGGER CODE ITSELF
#endif

#ifndef LOGGER_SETS_H
#include "logger_sets.h"
#endif

#include <fmt_format_ne.h>
#include <fmt/fmt_printf.h>
#include <stdarg.h>
#include <cstdio>

#ifndef PGE_NO_THREADING
#   ifdef PGE_SDL_MUTEX
#       include <SDL2/SDL_mutex.h>
#   else
#       include <mutex>
#   endif
#endif

#include <sstream>
#include <algorithm>

#ifdef _WIN32
#define OS_NEWLINE "\r\n"
#define OS_NEWLINE_LEN 2
#else
#define OS_NEWLINE "\n"
#define OS_NEWLINE_LEN 1
#endif

#ifdef PGE_NO_THREADING

#   define MUTEXLOCK(mn) (void)0

#else

class MutexLocker
{
#ifdef PGE_SDL_MUTEX
    SDL_mutex *m_mutex;
#else
    std::mutex *m_mutex;
#endif

public:
#ifdef PGE_SDL_MUTEX
    MutexLocker(SDL_mutex **mutex)
    {
        m_mutex = *mutex;
        SDL_LockMutex(m_mutex);
    }
#else
    MutexLocker(std::mutex *mutex)
    {
        m_mutex = mutex;
        m_mutex->lock();
    }
#endif

    ~MutexLocker()
    {
#ifdef PGE_SDL_MUTEX
        SDL_UnlockMutex(m_mutex);
#else
        m_mutex->unlock();
#endif
    }
};

#   define MUTEXLOCK(mn) \
    MutexLocker mn(&g_lockLocker); \
    (void)mn

#endif
