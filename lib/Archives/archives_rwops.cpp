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


#include <cstring>

#include <SDL2/SDL_rwops.h>
#include "sdl_proxy/sdl_assert.h"

#include "mbediso.h"

#include "archives.h"
#include "archives_priv.h"

namespace Archives
{

static int64_t s_file_size(SDL_RWops* stream)
{
    if(!stream || !stream->hidden.unknown.data1)
        return -1;

    mbediso_file* f = static_cast<mbediso_file*>(stream->hidden.unknown.data1);

    return mbediso_fsize(f);
}

static int64_t s_file_seek(SDL_RWops* stream, int64_t offset, int whence)
{
    if(!stream || !stream->hidden.unknown.data1)
        return -1;

    mbediso_file* f = static_cast<mbediso_file*>(stream->hidden.unknown.data1);

    return mbediso_fseek(f, offset, whence);
}

static size_t s_file_read(SDL_RWops* stream, void* ptr, size_t size, size_t nmemb)
{
    if(!stream || !stream->hidden.unknown.data1)
        return 0;

    mbediso_file* f = static_cast<mbediso_file*>(stream->hidden.unknown.data1);

    return (size_t)mbediso_fread(f, ptr, size, nmemb);
}

static size_t s_file_write(SDL_RWops*, const void*, size_t, size_t)
{
    return 0;
}

static int s_file_close_normal(SDL_RWops* stream)
{
    if(!stream || !stream->hidden.unknown.data1)
        return -1;

    mbediso_file* f = static_cast<mbediso_file*>(stream->hidden.unknown.data1);

    mbediso_fclose(f);

    return 0;
}

static int s_file_close_decref(SDL_RWops* stream)
{
    if(s_file_close_normal(stream))
        return -1;

    SDL_assert_release(temp_refs > 0);
    temp_refs--;

    return 0;
}

SDL_RWops* open_file(const char* name)
{
    if(!is_prefix(name[0]))
        return nullptr;

    bool has_temp_ref = false;
    mbediso_file* f = nullptr;

    if(name[0] == '@')
    {
        const char* archive_path_start = name + 1;
        const char* archive_path_end = archive_path_start;
        // don't check for path end until after the first slash
        while(*archive_path_end != '\0' && *archive_path_end != '/' && *archive_path_end != '\\')
            ++archive_path_end;
        while(*archive_path_end != '\0' && *archive_path_end != ':')
            ++archive_path_end;

        if(archive_path_end == archive_path_start || *archive_path_end == '\0')
            return nullptr;

        ptrdiff_t archive_path_size = archive_path_end - archive_path_start;

        char* archive_path = (char*)malloc(archive_path_size + 1);
        memcpy(archive_path, archive_path_start, archive_path_size);
        archive_path[archive_path_size] = '\0';

        bool mounted = mount_temp(archive_path);
        free(archive_path);

        if(!mounted || !temp_mount)
            return nullptr;

        const char* file_path_begin = archive_path_end + 1;

        f = mbediso_fopen(temp_mount, file_path_begin);
        has_temp_ref = true;
    }
    else if(name[0] == ':' && (name[1] == 'a' || name[1] == 'e'))
    {
        mbediso_fs* fs = (name[1] == 'a') ? assets_mount : episode_mount;

        if(fs)
            f = mbediso_fopen(fs, name + 2);
    }

    if(!f)
        return nullptr;

    SDL_RWops* ret = SDL_AllocRW();

    if(!ret)
    {
        mbediso_fclose(f);
        return nullptr;
    }

    ret->size = s_file_size;
    ret->seek = s_file_seek;
    ret->read = s_file_read;
    ret->write = s_file_write;
    ret->close = (has_temp_ref) ? s_file_close_decref : s_file_close_normal;

    if(has_temp_ref)
        temp_refs++;

    ret->type = SDL_RWOPS_UNKNOWN;
    ret->hidden.unknown.data1 = f;

    return ret;
}

} // namespace Archives
