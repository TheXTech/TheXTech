/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2022 Vitaly Novichkov <admin@wohlnet.ru>
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
#ifndef SDL_SDL_RWOPS_H
#define SDL_SDL_RWOPS_H

#if defined(SDLRPOXY_NULL)

#include <cstdio>

#define RW_SEEK_CUR SEEK_CUR
#define RW_SEEK_SET SEEK_SET
#define RW_SEEK_END SEEK_END

struct SDL_RWops
{
    FILE* f = nullptr;
};

inline size_t SDL_RWread(SDL_RWops* stream, void* ptr, size_t size, size_t nmemb)
{
    return fread(ptr, size, nmemb, stream->f);
}

inline long SDL_RWtell(SDL_RWops* stream)
{
    return ftell(stream->f);
}

inline int SDL_RWseek(SDL_RWops* stream, long offset, int whence)
{
    return fseek(stream->f, offset, whence);
}

inline SDL_RWops* SDL_RWFromFile(const char* pathname, const char* mode)
{
    FILE* f = fopen(pathname, mode);
    if(!f)
        return nullptr;

    SDL_RWops* ret = new(std::nothrow) SDL_RWops;

    if(!ret)
    {
        fclose(f);
        return nullptr;
    }

    ret->f = f;

    return ret;
}

inline SDL_RWops* SDL_RWFromMem(char* mem, int size)
{
    return nullptr;
}

inline SDL_RWops* SDL_RWFromConstMem(const char* mem, int size)
{
    return nullptr;
}

inline int SDL_RWclose(SDL_RWops* stream)
{
    fclose(stream->f);
    delete stream;

    return 0;
}

#else

#include <SDL2/SDL_rwops.h>

#endif

#endif // #ifndef SDL_SDL_RWOPS_H
