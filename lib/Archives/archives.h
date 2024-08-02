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


#pragma once

#ifndef THEXTECH_ARCHIVES_H
#define THEXTECH_ARCHIVES_H

#include <string>

struct SDL_RWops;
struct mbediso_fs;
struct mbediso_dir;

namespace Archives
{

enum PathType
{
    PATH_NONE = 0,
    PATH_FILE,
    PATH_DIR,
};

struct DirEntry
{
    const char* name;
    PathType type;
};

struct DirIterator
{
    mbediso_dir* dir = nullptr;
    mbediso_fs* fs_to_free = nullptr;
    bool expired = false;

    struct DirIter
    {
        bool is_end = false;
        mbediso_dir* dir = nullptr;
        DirEntry entry;

        DirEntry& operator*();
        DirIter& operator++();
        bool operator!=(const DirIter& o) const;
    };

    DirIter begin();
    DirIter end();
    ~DirIterator();
};

inline bool is_prefix(char letter)
{
    return letter == ':' || letter == '@';
}

inline bool has_prefix(const std::string& s)
{
    return is_prefix(s.c_str()[0]);
}

bool mount_assets(const char* archive_path);
void unmount_assets();
const std::string& assets_archive_path();

bool mount_episode(const char* archive_path);
void unmount_episode();

SDL_RWops* open_file(const char* name);
DirIterator list_dir(const char* name);
PathType exists(const char* name);

} // namespace Archives

#endif // #ifndef THEXTECH_ARCHIVES_H
