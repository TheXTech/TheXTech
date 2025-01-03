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


#pragma once

#ifndef THEXTECH_ARCHIVES_PRIV_H
#define THEXTECH_ARCHIVES_PRIV_H

namespace Archives
{

extern mbediso_fs* assets_mount;
extern mbediso_fs* episode_mount;
extern mbediso_fs* temp_mount;
extern int temp_refs;

bool mount_temp(const char* archive_path);

} // namespace Archives

#endif // #ifndef THEXTECH_ARCHIVES_PRIV_H
