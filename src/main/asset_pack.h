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

#ifndef ASSET_PACK_H
#define ASSET_PACK_H

#include <string>
#include <vector>
#include <memory>

#include "std_picture.h"

extern std::string g_AssetPackID;

struct AssetPack_t
{
    struct Gfx
    {
        StdPicture logo;
        StdPicture icon;
        StdPicture background;

        int bg_frames = 1;
        int bg_frame_ticks = 8;
    };

    std::string id;
    std::string version;

    std::string path;

    std::unique_ptr<Gfx> gfx;

    bool show_id = false;
    bool show_version = false;
    bool logo_override = false;

    std::string full_id() const
    {
        if(id.empty() && version.empty())
            return "";
        else if(version.empty())
            return id;

        return id + "/" + version;
    }
};

//! reports currently discovered asset pack IDs
const std::vector<AssetPack_t>& GetAssetPacks();

//! changes the AppPath and reloads assets from a desired asset pack
bool ReloadAssetsFrom(const AssetPack_t& pack);

//! initalizes the AppPath and loads GFX from a desired asset pack
bool InitUIAssetsFrom(const std::string& id);

#endif // #ifndef ASSET_PACK_H
