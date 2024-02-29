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

#include <algorithm>

#include <AppPath/app_path.h>

#include <Utils/files.h>
#include <DirManager/dirman.h>
#include <Logger/logger.h>
#include <IniProcessor/ini_processing.h>

#include "main/asset_pack.h"
#include "core/render.h"
#include "core/msgbox.h"

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
        std::string orig_ext;
        size_t base_size = nameWithPng.find_last_of('.');

        if(base_size == std::string::npos)
            base_size = nameWithPng.size();
        else
        {
            orig_ext = nameWithPng.substr(base_size);
            nameWithPng.resize(base_size);
        }

        nameWithPng += X_IMG_EXT;

        if(Files::fileExists(nameWithPng))
        {
            XRender::lazyLoadPicture(dest, nameWithPng);

            if(dest.inited)
                return; // Success
        }

#   if !defined(X_NO_PNG_GIF)
        nameWithPng.resize(base_size);

        if(!orig_ext.empty())
            nameWithPng += orig_ext;
#   endif // !defined(X_NO_PNG_GIF)
    }
#endif // defined(X_IMG_EXT)

#if !defined(X_NO_PNG_GIF)
    if(!dest.inited && Files::fileExists(nameWithPng))
        XRender::lazyLoadPicture(dest, nameWithPng);
#endif
}

static AssetPack_t s_scan_asset_pack(const std::string& path, bool skip_graphics = false)
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
        gameinfo.read("version", ret.version, ret.version);
        gameinfo.read("show-id", ret.show_id, ret.show_id);
        gameinfo.endGroup();

        gameinfo.beginGroup("android");
        gameinfo.read("logo", logo_image_path, logo_image_path_default);
        gameinfo.read("background", bg_image_path, bg_image_path_default);
        gameinfo.read("background-frames", ret.gfx->bg_frames, ret.gfx->bg_frames);
        gameinfo.read("background-delay", ret.gfx->bg_frame_ticks, ret.gfx->bg_frame_ticks);
        gameinfo.endGroup();
    }

    if(logo_image_path != logo_image_path_default)
        ret.logo_override = true;

    // don't allow any dirsep characters in asset pack ID
    for(char& c : ret.id)
    {
        if(c == '/' || c == '\\')
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

static void s_strip_id(AssetPack_t& pack)
{
    if(!pack.id.empty() && !pack.version.empty())
        pack.version = pack.id + '-' + pack.version;
    else if(!pack.id.empty())
        pack.version = pack.id;

    pack.id.clear();
}

static void s_find_asset_packs()
{
    pLogDebug("Looking for asset packs...");

    std::string custom_root = AppPathManager::userAddedAssetsRoot();

    DirMan assets;
    std::vector<std::string> subdirList;
    std::string subdir;

    bool found_debug_any = false;
    bool found_debug_match = false;

    for(const std::string& root : AppPathManager::assetsSearchPath())
    {
        // check for a root passed via `-c`
        bool is_custom_root = !custom_root.empty() && root == custom_root;

        // Normally, root is a legacy asset pack, and <root>/assets/ contains modern asset packs.
        // If passed via -c, root must be a modern asset pack also.

        D_pLogDebug(" Checking %s", root.c_str());
        if(DirMan::exists(root + "graphics/ui/"))
        {
            AssetPack_t pack = s_scan_asset_pack(root);

            // strip the ID from a legacy asset pack
            if(!is_custom_root)
                s_strip_id(pack);

            if(!pack.gfx || !pack.gfx->logo.inited)
                pLogWarning("Could not load UI assets from %s asset pack [%s], ignoring", (is_custom_root) ? "user-specified" : "possible legacy", pack.path.c_str());
            else if(is_custom_root && pack.id.empty())
                pLogWarning("Could not read ID of user-specified asset pack [%s], ignoring", pack.path.c_str());
            else if(found_debug_match)
                pLogDebug("Current legacy/debug assets already found, ignoring assets at [%s]", pack.path.c_str());
            else if(found_debug_any && pack.full_id() != g_AssetPackID)
                pLogDebug("Generic legacy/debug assets already found, ignoring assets at [%s]", pack.path.c_str());
            else
            {
                found_debug_any = true;
                if(pack.full_id() == g_AssetPackID)
                    found_debug_match = true;

                s_asset_packs.push_back(std::move(pack));
            }
        }

        if(!is_custom_root && DirMan::exists(root + "assets/"))
        {
            assets.setPath(root + "assets/");
            assets.getListOfFolders(subdirList);

            for(const std::string& sub : subdirList)
            {
                subdir = root + "assets/" + sub;

                D_pLogDebug("  Checking %s", subdir.c_str());

                if(DirMan::exists(subdir + "/graphics/ui/"))
                {
                    AssetPack_t pack = s_scan_asset_pack(subdir);

                    if(!pack.gfx || !pack.gfx->logo.inited)
                        pLogWarning("Could not load UI assets from possible asset pack [%s], ignoring", pack.path.c_str());
                    else if(pack.id.empty())
                        pLogWarning("Could not read ID of possible asset pack [%s], ignoring", pack.path.c_str());
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
            pLogDebug("- %s (%s)", pack.full_id().c_str(), pack.path.c_str());
    }
    else
        pLogCritical("Could not find any asset packs.");

    // check for duplicates
    for(size_t i = 0; i < s_asset_packs.size(); i++)
    {
        for(size_t j = i + 1; j < s_asset_packs.size();)
        {
            if(s_asset_packs[i].id == s_asset_packs[j].id)
            {
                if(s_asset_packs[i].version == s_asset_packs[j].version)
                {
                    // erase a case with same ID and version
                    s_asset_packs.erase(s_asset_packs.begin() + j);
                    continue;
                }

                // if version is the distinguishing factor, display it
                s_asset_packs[i].show_version = true;
                s_asset_packs[j].show_version = true;
            }

            j++;
        }
    }

    s_found_asset_packs = true;
}

static AssetPack_t s_find_pack_init(const std::string& full_id)
{
    std::string id = full_id;
    std::string version;

    auto slash_pos = id.find('/');

    if(slash_pos != std::string::npos)
    {
        version = id.substr(slash_pos + 1);
        id.resize(slash_pos);
    }

    std::string id_as_path = full_id;

    appendSlash(id_as_path);

    std::string custom_root = AppPathManager::userAddedAssetsRoot();

    DirMan assets;
    std::vector<std::string> subdirList;
    std::string subdir;

    AssetPack_t any_pack;
    AssetPack_t id_match;

    for(const std::string& root : AppPathManager::assetsSearchPath())
    {
        // check for a root passed via `-c`
        bool is_custom_root = !custom_root.empty() && root == custom_root;

        // Normally, root is a legacy asset pack, and <root>/assets/ contains modern asset packs.
        // If passed via -c, root must be a modern asset pack also.

        // check for legacy debug assets
        if(DirMan::exists(root + "graphics/ui/"))
        {
            AssetPack_t pack = s_scan_asset_pack(root, true);

            // strip the ID from a legacy asset pack
            if(!is_custom_root)
                s_strip_id(pack);

            if(is_custom_root && pack.id.empty())
            {
                pLogWarning("Could not read ID of command-line specified asset pack [%s], ignoring", pack.path.c_str());
                pLogCritical("gameinfo.ini of command-line specified asset pack [%s] has not been updated to specify its asset pack ID. After updating, you will need to move your gamesaves to settings/gamesaves/<ID>.", pack.path.c_str());

                XMsgBox::errorMsgBox("Critical error",
                                     "gameinfo.ini of command-line specified asset pack has not been updated to specify its asset pack ID.\n"
                                     "After updating, you will need to manually move your gamesaves to settings/gamesaves/<ID>.\n");
            }
            else if(is_custom_root && pack.path == id_as_path)
                return pack;
            else if(pack.id == id && pack.version == version && !full_id.empty())
                return pack;
            else if(!pack.id.empty() && pack.id == id && id_match.path.empty())
                id_match = std::move(pack);
            else if(any_pack.path.empty())
                any_pack = std::move(pack);
        }

        if(!is_custom_root && DirMan::exists(root + "assets/"))
        {
            assets.setPath(root + "assets/");
            assets.getListOfFolders(subdirList);

            for(const std::string& sub : subdirList)
            {
                subdir = root + "assets/" + sub;

                if(DirMan::exists(subdir + "/graphics/ui/"))
                {
                    AssetPack_t pack = s_scan_asset_pack(subdir, true);

                    if(pack.id.empty())
                        pLogWarning("Could not read ID of possible asset pack [%s], ignoring", pack.path.c_str());
                    else if(pack.id == id && pack.version == version)
                        return pack;
                    else if(pack.id == id && id_match.path.empty())
                        id_match = std::move(pack);
                    else if(any_pack.path.empty())
                        any_pack = std::move(pack);
                }
            }
        }
    }

    if(!id_match.path.empty())
        return id_match;
    else if(full_id.empty() && !any_pack.path.empty())
        return any_pack;

    if(full_id.empty())
        pLogCritical("Could not find any asset packs.");

    return AssetPack_t();
}


bool ReloadAssetsFrom(const AssetPack_t& pack)
{
    pLogDebug("= Trying to load asset pack \"%s/\" from [%s]", pack.id.c_str(), pack.version.c_str(), pack.path.c_str());

    std::string OldAppPath = AppPath;
    std::string OldAssetPackID = g_AssetPackID;

    AppPathManager::setCurrentAssetPack(pack.id, pack.path);
    AppPath = AppPathManager::assetsRoot();
    g_AssetPackID = pack.full_id();

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
        for(auto it = s_asset_packs.begin(); it != s_asset_packs.end(); ++it)
        {
            // find the requested pack exactly (by identity, not contents)
            if(&*it != &pack)
                continue;

            pLogInfo("Removed %s from asset pack list because it is missing UI assets", pack.path.c_str());
            s_asset_packs.erase(it);
            break;
        }

        return false;
    }

    ConfigReloadRecentEpisodes();

    pLogDebug("Successfully loaded UI assets; now loading all other assets from [%s]", AppPath.c_str());

    MainLoadAll(true);
    return true;
}


bool InitUIAssetsFrom(const std::string& id)
{
    std::string custom_root = AppPathManager::userAddedAssetsRoot();

    pLogDebug("Searching for asset packs in:");
    for(const std::string& root : AppPathManager::assetsSearchPath())
    {
        // treat command-line specified locations as direct storage
        if(!custom_root.empty() && root == custom_root)
        {
            pLogDebug("- %s", root.c_str());
        }
        else
        {
            pLogDebug("- %s (legacy)", root.c_str());
            pLogDebug("- %sassets/*", root.c_str());
        }
    }

    AssetPack_t pack;
    if(!id.empty())
    {
        pLogDebug("Trying to load CLI-specified asset pack [%s]", id.c_str());
        pack = s_find_pack_init(id);

        // not found, perhaps it was not updated
        if(pack.path.empty())
        {
            XMsgBox::errorMsgBox("Fatal error",
                                 "Could not load command-line specified asset pack: " + id);
            pLogFatal("Command-line specified asset pack [%s] could not be loaded.", id.c_str());
            return false;
        }
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
    g_AssetPackID = pack.full_id();

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
