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

#ifdef _WIN32
#include <windows.h>
#include <shlwapi.h>
#else
#include <stdio.h>
#endif

/* from files.cpp */
static inline FILE *utf8_fopen(const char *filePath, const char *modes)
{
#ifndef _WIN32
    return fopen(filePath, modes);
#else
    wchar_t wfile[MAX_PATH + 1];
    wchar_t wmode[21];
    int wfile_len = (int)strlen(filePath);
    int wmode_len = (int)strlen(modes);
    wfile_len = MultiByteToWideChar(CP_UTF8, 0, filePath, wfile_len, wfile, MAX_PATH);
    wmode_len = MultiByteToWideChar(CP_UTF8, 0, modes, wmode_len, wmode, 20);
    wfile[wfile_len] = L'\0';
    wmode[wmode_len] = L'\0';
    return _wfopen(wfile, wmode);
#endif
}

static int64_t s_file_size(SDL_RWops* stream)
{
    if(!stream || !stream->hidden.unknown.data1)
        return -1;

    long curpos = ftell(stream->hidden.unknown.data1);

    fseek(stream->hidden.unknown.data1, 0, SEEK_END);
    int64_t filesize = (int64_t)(ftell(stream->hidden.unknown.data1));

    fseek(stream->hidden.unknown.data1, curpos, SEEK_SET);

    return filesize;
}

static int64_t s_file_seek(SDL_RWops* stream, int64_t offset, int whence)
{
    if(whence == RW_SEEK_CUR && offset == 0)
    {
        // skip the seek
    }
    else if(fseek(stream->hidden.unknown.data1, offset, whence))
        return -1;

    return ftell(stream->hidden.unknown.data1);
}

static size_t s_file_read(SDL_RWops* stream, void* ptr, size_t size, size_t nmemb)
{
    return fread(ptr, size, nmemb, stream->hidden.unknown.data1);
}

static size_t s_file_write(SDL_RWops* stream, const void* ptr, size_t size, size_t nmemb)
{
    return fwrite(ptr, size, nmemb, stream->hidden.unknown.data1);
}

static int s_file_close(SDL_RWops* stream)
{
    return fclose(stream->hidden.unknown.data1);
}

SDL_RWops* SDL_RWFromFile(const char* pathname, const char* mode)
{
    FILE* f = utf8_fopen(pathname, mode);
    if(!f)
        return NULL;

    SDL_RWops* ret = SDL_AllocRW();

    if(!ret)
    {
        fclose(f);
        return NULL;
    }

    ret->size = s_file_size;
    ret->seek = s_file_seek;
    ret->read = s_file_read;
    ret->write = s_file_write;
    ret->close = s_file_close;

    ret->type = SDL_RWOPS_STDFILE;
    ret->hidden.unknown.data1 = f;

    return ret;
}
