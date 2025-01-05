/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2025 Vitaly Novichkov <admin@wohlnet.ru>
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


#include <string>
#include <Logger/logger.h>

#include "archives.h"
#include "archives_priv.h"

#include "mbediso.h"

namespace Archives
{

mbediso_fs* assets_mount = nullptr;
mbediso_fs* episode_mount = nullptr;
mbediso_fs* temp_mount = nullptr;
static std::string s_assets_archive_path;
static std::string s_episode_archive_path;
static std::string s_temp_archive_path;

int temp_refs;

static void s_unmount(mbediso_fs*& target, std::string& loaded_path)
{
    loaded_path.clear();

    if(!target)
        return;

    mbediso_closefs(target);
    target = nullptr;
}

static bool s_mount(mbediso_fs*& target, std::string& loaded_path, const char* archive_path)
{
    if(loaded_path == archive_path)
        return true;

    s_unmount(target, loaded_path);

    if(s_temp_archive_path == archive_path && temp_refs == 0 && temp_mount != nullptr && mbediso_scanfs(temp_mount) == 0)
    {
        pLogDebug("Upgrading temporary mount to real mount");

        target = temp_mount;
        temp_mount = nullptr;

        loaded_path = std::move(s_temp_archive_path);
        s_temp_archive_path.clear();

        return true;
    }

    target = mbediso_openfs_file(archive_path, target != temp_mount);

    if(target)
        loaded_path = archive_path;

    return target;
}

bool mount_assets(const char* archive_path)
{
    return s_mount(assets_mount, s_assets_archive_path, archive_path);
}

void unmount_assets()
{
    return s_unmount(assets_mount, s_assets_archive_path);
}

const std::string& assets_archive_path()
{
    return s_assets_archive_path;
}

bool mount_episode(const char* archive_path)
{
    return s_mount(episode_mount, s_episode_archive_path, archive_path);
}

void unmount_episode()
{
    return s_unmount(episode_mount, s_episode_archive_path);
}

const std::string& episode_archive_path()
{
    return s_episode_archive_path;
}

bool mount_temp(const char* archive_path)
{
    if(s_temp_archive_path == archive_path)
        return true;

    if(temp_refs > 0)
    {
        pLogCritical("Can't mount new archive [%s]; items still open from prev archive [%s]", archive_path, s_temp_archive_path.c_str());
        return false;
    }

    return s_mount(temp_mount, s_temp_archive_path, archive_path);
}

void unmount_temp()
{
    if(temp_refs > 0)
    {
        pLogCritical("Can't unmount temp archive [%s;] items still open.", s_temp_archive_path.c_str());
        return;
    }

    return s_unmount(temp_mount, s_temp_archive_path);
}

} // namespace Archives
