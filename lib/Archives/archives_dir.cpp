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

#include <vector>

#include <cstring>
#include <cstddef>

#include "archives.h"
#include "archives_priv.h"

#include "mbediso.h"

namespace Archives
{

static void s_next_iter(DirIterator::DirIter& iter)
{
    const auto* ent = mbediso_readdir(iter.dir);
    if(!ent)
    {
        iter.is_end = true;
        return;
    }

    iter.entry.name = (char*)ent->d_name;

    if(ent->d_type == MBEDISO_DT_REG)
        iter.entry.type = PATH_FILE;
    else if(ent->d_type == MBEDISO_DT_DIR)
        iter.entry.type = PATH_DIR;
    else
        iter.entry.type = PATH_NONE;
}

DirIterator::DirIter DirIterator::begin()
{
    DirIter ret;
    if(!dir || expired)
    {
        ret.is_end = true;
        return ret;
    }

    expired = true;

    ret.dir = dir;
    s_next_iter(ret);
    return ret;
}

DirIterator::DirIter DirIterator::end()
{
    DirIter ret;
    ret.is_end = true;
    return ret;
}

DirIterator::~DirIterator()
{
    if(dir)
        mbediso_closedir(dir);
    if(fs_to_free)
        mbediso_closefs(fs_to_free);

    dir = nullptr;
    fs_to_free = nullptr;
}

DirEntry& DirIterator::DirIter::operator*()
{
    return entry;
}

DirIterator::DirIter& DirIterator::DirIter::operator++()
{
    s_next_iter(*this);
    return *this;
}

bool DirIterator::DirIter::operator!=(const DirIterator::DirIter& o) const
{
    return !is_end || !o.is_end;
}

DirIterator list_dir(const char* name)
{
    DirIterator ret;

    if(!is_prefix(name[0]))
        return ret;

    mbediso_dir* d = nullptr;

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
            return ret;

        ptrdiff_t archive_path_size = archive_path_end - archive_path_start;

        char* archive_path = (char*)malloc(archive_path_size + 1);
        memcpy(archive_path, archive_path_start, archive_path_size);
        archive_path[archive_path_size] = '\0';

        mbediso_fs* tempfs = mbediso_openfs_file(archive_path, false);
        free(archive_path);

        if(!tempfs)
            return ret;

        const char* dir_path_begin = archive_path_end + 1;

        d = mbediso_opendir(tempfs, dir_path_begin);

        if(d)
            ret.fs_to_free = tempfs;
        else
            mbediso_closefs(tempfs);
    }
    else if(name[0] == ':' && (name[1] == 'a' || name[1] == 'e'))
    {
        mbediso_fs* fs = (name[1] == 'a') ? assets_mount : episode_mount;

        if(fs)
            d = mbediso_opendir(fs, name + 2);
    }

    ret.dir = d;

    return ret;
}

PathType exists(const char* name)
{
    if(!is_prefix(name[0]))
        return PATH_NONE;

    int found = 0;

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
            return PATH_NONE;

        ptrdiff_t archive_path_size = archive_path_end - archive_path_start;

        char* archive_path = (char*)malloc(archive_path_size + 1);
        memcpy(archive_path, archive_path_start, archive_path_size);
        archive_path[archive_path_size] = '\0';

        mbediso_fs* tempfs = mbediso_openfs_file(archive_path, false);
        free(archive_path);

        if(!tempfs)
            return PATH_NONE;

        const char* dir_path_begin = archive_path_end + 1;

        found = mbediso_exists(tempfs, dir_path_begin);

        mbediso_closefs(tempfs);
    }
    else if(name[0] == ':' && (name[1] == 'a' || name[1] == 'e'))
    {
        mbediso_fs* fs = (name[1] == 'a') ? assets_mount : episode_mount;

        if(fs)
            found = mbediso_exists(fs, name + 2);
    }

    if(found == MBEDISO_DT_DIR)
        return PATH_DIR;
    else if(found == MBEDISO_DT_REG)
        return PATH_FILE;
    else
        return PATH_NONE;
}

} // namespace Archives
