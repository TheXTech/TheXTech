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

#include <AppPath/app_path.h>

#include <Utils/files.h>
#include <DirManager/dirman.h>
#include <Logger/logger.h>
#include <IniProcessor/ini_processing.h>

#include "main/asset_pack.h"
#include "core/render.h"

#include "presetup.h"
#include "gfx.h"
#include "load_gfx.h"
#include "game_main.h"

static std::vector<AssetPack_t> s_asset_packs;
static bool s_found_asset_packs = false;

std::string g_AssetPackID;

static void appendSlash(std::string &path)
{
#if defined(__EMSCRIPTEN__)
    // fix emscripten bug of duplicated worlds
    if(path.empty() || path.back() != '/')
        path.push_back('/');
#else
    if(!path.empty() && path.back() != '/')
        path.push_back('/');
#endif
}

static inline void s_load_image(StdPicture& dest, std::string nameWithPng)
{
    dest.reset();

#if defined(X_IMG_EXT)
    if(nameWithPng.size() > 4)
    {
        size_t base_size = nameWithPng.size() - 4;

        std::string orig_ext = nameWithPng.substr(base_size);
        nameWithPng.resize(base_size);

        nameWithPng += X_IMG_EXT;

        if(Files::fileExists(nameWithPng))
            XRender::lazyLoadPicture(dest, nameWithPng);

        nameWithPng.resize(base_size);
        nameWithPng += orig_ext;
    }
#endif

#if !defined(X_NO_PNG_GIF)
    if(!dest.inited)
        XRender::lazyLoadPicture(dest, nameWithPng);
#endif
}

static AssetPack_t s_scan_asset_pack(const std::string& path, const char* default_name, bool skip_graphics = false)
{
    AssetPack_t ret;
    ret.path = path;
    ret.gfx.reset(new AssetPack_t::Gfx());

    appendSlash(ret.path);

    // based on Launcher.java:updateOverlook()
    const char* assets_icon = "graphics/ui/icon/thextech_128.png";
    const char* logo_image_path_default = "graphics/ui/MenuGFX2.png";
    const char* bg_image_path_default = "graphics/background2/background2-2.png";

    std::string logo_image_path;
    std::string bg_image_path;

    ret.gfx->bg_frames = 1;
    ret.gfx->bg_frame_ticks = 125;

    std::string gi_path = ret.path + "gameinfo.ini";

    if(Files::fileExists(gi_path))
    {
        IniProcessing gameinfo(gi_path);

        gameinfo.beginGroup("game");
        gameinfo.read("id", ret.id, ret.id);
        bool provides_smbx64_default = ret.id.empty();
        gameinfo.read("provides-smbx64", ret.provides_smbx64, provides_smbx64_default);
        gameinfo.endGroup();

        gameinfo.beginGroup("android");
        gameinfo.read("logo", logo_image_path, logo_image_path_default);
        gameinfo.read("background", bg_image_path, bg_image_path_default);
        gameinfo.read("background-frames", ret.gfx->bg_frames, ret.gfx->bg_frames);
        gameinfo.read("background-delay", ret.gfx->bg_frame_ticks, ret.gfx->bg_frame_ticks);
        gameinfo.endGroup();
    }

    if(ret.id.empty())
        ret.id = default_name;

    // don't allow any dirsep characters in asset pack ID
    for(char& c : ret.id)
    {
        if(c == '/')
            c = '_';

        if(c == '\\')
            c = '_';
    }

    // ms to frames (approximately)
    ret.gfx->bg_frame_ticks /= 15;

    // load graphics
    if(!skip_graphics)
    {
        s_load_image(ret.gfx->icon, ret.path + assets_icon);
        s_load_image(ret.gfx->logo, ret.path + logo_image_path);
        s_load_image(ret.gfx->background, ret.path + bg_image_path);

        if(!ret.gfx->logo.inited && logo_image_path != logo_image_path_default)
            s_load_image(ret.gfx->logo, ret.path + logo_image_path_default);

        if(!ret.gfx->background.inited && bg_image_path != bg_image_path_default)
            s_load_image(ret.gfx->background, ret.path + bg_image_path_default);
    }

    return ret;
}

static void s_find_asset_packs()
{
    pLogDebug("Looking for asset packs...");

    DirMan assets;
    std::vector<std::string> subdirList;
    std::string subdir;

    for(const std::string& root : AppPathManager::assetsSearchPath())
    {
        D_pLogDebug(" Checking %s", root.c_str());
        if(DirMan::exists(root + "graphics/ui/"))
        {
            AssetPack_t pack = s_scan_asset_pack(root, "");
            if(!pack.gfx || !pack.gfx->logo.inited)
                pLogWarning("Could not load UI assets from possible asset pack [%s], ignoring", pack.path.c_str());
            else
                s_asset_packs.push_back(std::move(pack));
        }

        if(DirMan::exists(root + "assets/"))
        {
            assets.setPath(root + "assets/");
            assets.getListOfFolders(subdirList);

            for(const std::string& sub : subdirList)
            {
                subdir = root + "assets/" + sub;
                D_pLogDebug("  Checking %s", subDir.c_str());
                if(DirMan::exists(subdir + "/graphics/ui/"))
                {
                    AssetPack_t pack = s_scan_asset_pack(subdir, sub.c_str());
                    if(!pack.gfx || !pack.gfx->logo.inited)
                        pLogWarning("Could not load UI assets from possible asset pack [%s], ignoring", pack.path.c_str());
                    else
                        s_asset_packs.push_back(std::move(pack));
                }
            }
        }
    }

    if(!s_asset_packs.empty())
    {
        pLogDebug("Found asset packs:");

        for(const AssetPack_t& pack : s_asset_packs)
            pLogDebug("- %s (%s)", pack.id.c_str(), pack.path.c_str());
    }
    else
        pLogCritical("Could not find any asset packs.");

    s_found_asset_packs = true;
}

static AssetPack_t s_find_pack_init(const std::string& id)
{
    if(id.find('/') != std::string::npos)
        return s_scan_asset_pack(id, "", true);

    bool id_is_smbx64 = (id == "smbx64");

    DirMan assets;
    std::vector<std::string> subdirList;
    std::string subdir;

    for(const std::string& root : AppPathManager::assetsSearchPath())
    {
        if(DirMan::exists(root + "graphics/ui/"))
        {
            AssetPack_t maybe = s_scan_asset_pack(root, "", true);
            if(id.empty() || maybe.id == id || (id_is_smbx64 && maybe.provides_smbx64))
                return maybe;
        }

        if(DirMan::exists(root + "assets/"))
        {
            assets.setPath(root + "assets/");
            assets.getListOfFolders(subdirList);

            for(const std::string& pack : subdirList)
            {
                subdir = root + "assets/" + pack;
                if(DirMan::exists(subdir + "/graphics/ui/"))
                {
                    AssetPack_t maybe = s_scan_asset_pack(subdir, pack.c_str(), true);
                    if(id.empty() || maybe.id == id || (id_is_smbx64 && maybe.provides_smbx64))
                        return maybe;
                }
            }
        }
    }

    if(id.empty())
        pLogCritical("Could not find any asset packs.");

    return AssetPack_t();
}

static std::vector<AssetPack_t>::iterator s_find_pack(const std::string& id)
{
    if(!s_found_asset_packs)
        s_find_asset_packs();

    for(auto it = s_asset_packs.begin(); it != s_asset_packs.end(); ++it)
    {
        if(it->path == id || it->id == id)
            return it;
    }

    return s_asset_packs.end();
}



bool ReloadAssetsFrom(const std::string& id)
{
    pLogDebug("= Trying to load asset pack %s", id.c_str());

    auto it = s_find_pack(id);
    if(it == s_asset_packs.end())
    {
        pLogWarning("Couldn't find asset pack");
        return false;
    }

    const AssetPack_t& pack = *it;

    std::string OldAppPath = AppPath;
    std::string OldAssetPackID = g_AssetPackID;

    AppPathManager::setCurrentAssetPack(pack.id, pack.path);
    AppPath = AppPathManager::assetsRoot();
    g_AssetPackID = pack.id;

    UnloadCustomGFX();
    UnloadWorldCustomGFX();

    GFX.unLoad();

    pLogDebug("Loading UI assets from [%s]", AppPath.c_str());

    if(!GFX.load())
    {
        pLogWarning("Failed to load UI assets");

        // restore original GFX
        GFX.unLoad();
        AppPathManager::setCurrentAssetPack(OldAssetPackID, OldAppPath);
        AppPath = OldAppPath;
        g_AssetPackID = OldAssetPackID;
        GFX.load();

        // also, remove from list of valid asset packs
        if(it != s_asset_packs.end())
        {
            pLogDebug("Removed %s from asset pack list because it is missing UI assets", pack.path.c_str());
            s_asset_packs.erase(it);
        }

        return false;
    }

    pLogDebug("Successfully loaded UI assets; now loading all other assets from [%s]", AppPath.c_str());

    MainLoadAll(true);
    return true;
}


bool InitUIAssetsFrom(const std::string& id)
{
    pLogDebug("Searching for asset packs in:");
    for(const std::string& root : AppPathManager::assetsSearchPath())
    {
        pLogDebug("- %s", root.c_str());
        pLogDebug("- %sassets/*", root.c_str());
    }

    AssetPack_t pack;
    if(!id.empty())
    {
        pLogDebug("Trying to load CLI-specified asset pack [%s]", id.c_str());
        pack = s_find_pack_init(id);

        if(pack.path.empty())
            pack.path = id;
    }
    else
    {
        pLogDebug("Trying to load most recent asset pack [ID: %s]", g_preSetup.assetPack.c_str());
        pack = s_find_pack_init(g_preSetup.assetPack);
        if(pack.path.empty())
        {
            pLogDebug("Couldn't find recent asset pack, loading any other assets");
            pack = s_find_pack_init("");
        }
    }

    if(pack.path.empty())
        pack.path = ".";

    AppPathManager::setCurrentAssetPack(pack.id, pack.path);
    AppPath = AppPathManager::assetsRoot();
    g_AssetPackID = pack.id;

    pLogDebug("Loading assets from %s", AppPath.c_str());

    if(!GFX.load())
        return false;

    return true;
}


const std::vector<AssetPack_t>& GetAssetPacks()
{
    if(!s_found_asset_packs)
        s_find_asset_packs();

    return s_asset_packs;
}
