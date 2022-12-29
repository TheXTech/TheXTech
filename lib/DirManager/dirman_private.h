/*
DirMan - A small crossplatform class to manage directories

Copyright (c) 2017-2022 Vitaliy Novichkov <admin@wohlnet.ru>

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#ifndef DIRMAN_PRIVATE_H
#define DIRMAN_PRIVATE_H

#include <string>
#include <stack>
#include <vector>
#include <stdlib.h>

#include "dirman.h"

#ifdef _WIN32
typedef std::wstring    PathString;
#else
typedef std::string     PathString;
#endif

#ifndef PGE_NO_THREADING
#   ifdef PGE_SDL_MUTEX
#   include <SDL2/SDL_mutex.h>
#   include <SDL2/SDL_atomic.h>
static SDL_mutex *g_dirManMutex = nullptr;
static SDL_atomic_t g_dirManCounter = {0};

class MutexLocker
{
    SDL_mutex *m_mutex;

public:
    MutexLocker(SDL_mutex *mutex)
    {
        m_mutex = mutex;
        SDL_LockMutex(m_mutex);
    }

    ~MutexLocker()
    {
        SDL_UnlockMutex(m_mutex);
    }
};

#define PUT_THREAD_GUARD() \
    MutexLocker guard(g_dirManMutex); \
    (void)guard

#   else /*PGE_SDL_MUTEX*/
#   include <mutex>
static std::mutex g_dirManMutex;

#define PUT_THREAD_GUARD() \
    std::lock_guard<std::mutex> guard(g_dirManMutex);\
    (void)guard

#   endif /*PGE_SDL_MUTEX*/
#else /*PGE_NO_THREADING*/
#   define PUT_THREAD_GUARD() (void)0
#endif /*PGE_NO_THREADING*/


template<class CHAR>
static inline void delEnd(std::basic_string<CHAR> &dirPath, CHAR ch)
{
    if(!dirPath.empty())
    {
        CHAR last = dirPath[dirPath.size() - 1];
        if(last == ch)
            dirPath.resize(dirPath.size() - 1);
    }
}

extern bool matchSuffixFilters(const std::string &name, const std::vector<std::string> &suffixFilters);

class DirMan::DirMan_private
{
    friend class DirMan;

    std::string     m_dirPath;
#ifdef _WIN32
    std::wstring    m_dirPathW;
#endif

    struct DirWalkerState
    {
        std::stack<PathString>      digStack;
        std::vector<std::string>    suffix_filters;
    } m_walkerState;

    void setPath(const std::string &dirPath);
    bool getListOfFiles(std::vector<std::string> &list, const std::vector<std::string> &suffix_filters);
    bool getListOfFolders(std::vector<std::string> &list, const std::vector<std::string> &suffix_filters);
    bool fetchListFromWalker(std::string &curPath, std::vector<std::string> &list);

public:
#if !defined(PGE_NO_THREADING) && defined(PGE_SDL_MUTEX)
    DirMan_private()
    {
        if(SDL_AtomicGet(&g_dirManCounter) == 0 && g_dirManMutex == nullptr)
            g_dirManMutex = SDL_CreateMutex();
        SDL_AtomicAdd(&g_dirManCounter, 1);
    }

    DirMan_private(const DirMan_private &o)
    {
        m_dirPath = o.m_dirPath;
#ifdef _WIN32
        m_dirPathW = o.m_dirPathW;
#endif
        m_walkerState = o.m_walkerState;
        SDL_AtomicAdd(&g_dirManCounter, 1);
    }

    ~DirMan_private()
    {
        SDL_AtomicAdd(&g_dirManCounter, -1);
        if(SDL_AtomicGet(&g_dirManCounter) <= 0 && g_dirManMutex != nullptr)
        {
            SDL_DestroyMutex(g_dirManMutex);
            g_dirManMutex = nullptr;
        }
    }
#else
    DirMan_private() = default;
    DirMan_private(const DirMan_private &) = default;
    ~DirMan_private() = default;
#endif
};

#endif // DIRMAN_PRIVATE_H
