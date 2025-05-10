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

#include "sdl_proxy/sdl_timer.h"
#include "sdl_proxy/sdl_assert.h"
#include "sdl_proxy/sdl_stdinc.h"
#include "sdl_proxy/sdl_types.h"

#ifndef PGE_NO_THREADING
#include <SDL2/SDL_mutex.h>
#endif

#include "globals.h"
#include "global_dirs.h"

#include "blk_id.h"
#include "eff_id.h"

#include "gfx.h"
#include "load_gfx.h"
#include "graphics.h" // SuperPrint
#include "graphics/gfx_frame.h" // FrameBorderInfo, loadFrameInfo
#include "core/render.h"
#include "core/events.h"
#include "main/asset_pack.h"
#include "main/screen_asset_pack.h"

#include <IniProcessor/ini_processing.h>
#include <Utils/files.h>
#include <Utils/files_ini.h>
#include <Utils/dir_list_ci.h>
#include <Utils/strings.h>
#include <DirManager/dirman.h>
#ifdef THEXTECH_INTERPROC_SUPPORTED
#   include <InterProcess/intproc.h>
#endif
#include <fmt_format_ne.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#if defined(PGE_MIN_PORT) || defined(THEXTECH_CLI_BUILD)
#   include <PGE_File_Formats/file_formats.h>
#endif

#include <set>

bool gfxLoaderTestMode = false;
bool gfxLoaderThreadingMode = false;
#ifndef PGE_NO_THREADING
static SDL_mutex *gfxLoaderDebugMutex = nullptr;
#endif
static std::string gfxLoaderDebugString;
static bool        gfxLoaderDebugStringUpdated = false;
static Sint64 gfxLoaderDebugStart = -1;
static const Sint64 c_gfxLoaderShowInterval = 500;

//// Private Sub cBlockGFX(A As Integer)
//void cBlockGFX(int A);
//// Private Sub cNPCGFX(A As Integer)
//void cNPCGFX(int A);
//// Private Sub cBackgroundGFX(A As Integer)
//void cBackgroundGFX(int A);
//// Private Sub cTileGFX(A As Integer)
//void cTileGFX(int A);
//// Private Sub cSceneGFX(A As Integer)
//void cSceneGFX(int A);
//// Private Sub cLevelGFX(A As Integer)
//void cLevelGFX(int A);
//// Private Sub cPathGFX(A As Integer)
//void cPathGFX(int A);

struct GFXBackup_t
{
    vbint_t *remote_width = nullptr;
    vbint_t *remote_height = nullptr;
    bool *remote_isCustom = nullptr;
    StdPicture *remote_texture = nullptr;
    vbint_t width = 0;
    vbint_t height = 0;
    bool isCustom = false;
    StdPicture_Sub texture_backup;
};

static std::vector<GFXBackup_t> g_defaultLevelGfxBackup;
static std::vector<GFXBackup_t> g_defaultWorldGfxBackup;

// track which backups are previews of player graphics
static size_t s_previewPlayersBegin = 0;
static size_t s_previewPlayersEnd = 0;

struct FrameBorderInfoBackup_t
{
    FrameBorderInfo *remote_info = nullptr;
    FrameBorderInfo info_backup;
};

static std::vector<FrameBorderInfoBackup_t> g_defaultBorderInfoBackup;

static DirListCI s_dirFallback;

static std::string getGfxDir()
{
    return AppPath + "graphics/";
}

static bool s_useLangDir = false;
static std::string s_langSubDir;
static bool s_useLangDirEp = false;
static std::string s_langSubDirEp;

static void cgfx_initLangDir()
{
    std::string langDir;
    s_useLangDir = false;
    s_langSubDir.clear();
    s_useLangDirEp = false;
    s_langSubDirEp.clear();

    if(!CurrentLanguage.empty())
    {
        if(!CurrentLangDialect.empty())
        {
            langDir = CurrentLanguage + "-" + CurrentLangDialect;
            // Data directory
            if(DirMan::exists(g_dirCustom.getCurDir() + "i18n/" + langDir))
                s_langSubDir = "i18n/" + langDir + "/";
            else if(DirMan::exists(g_dirCustom.getCurDir() + "i18n/" + CurrentLanguage))
                s_langSubDir = "i18n/" + CurrentLanguage + "/";
            else if(DirMan::exists(g_dirCustom.getCurDir() + langDir))
                s_langSubDir = langDir + "/";
            else if(DirMan::exists(g_dirCustom.getCurDir() + CurrentLanguage))
                s_langSubDir = CurrentLanguage + "/";

            // Episode directory
            if(DirMan::exists(g_dirEpisode.getCurDir() + "i18n/" + langDir))
                s_langSubDirEp = "i18n/" + langDir + "/";
            else if(DirMan::exists(g_dirEpisode.getCurDir() + "i18n/" + CurrentLanguage))
                s_langSubDirEp = "i18n/" + CurrentLanguage + "/";
            else if(DirMan::exists(g_dirEpisode.getCurDir() + langDir))
                s_langSubDirEp = langDir + "/";
            else if(DirMan::exists(g_dirEpisode.getCurDir() + CurrentLanguage))
                s_langSubDirEp = CurrentLanguage + "/";
        }
        else
        {
            // Data directory
            if(DirMan::exists(g_dirCustom.getCurDir() + "i18n/" + CurrentLanguage))
                s_langSubDir = "i18n/" + CurrentLanguage + "/";
            else if(DirMan::exists(g_dirCustom.getCurDir() + CurrentLanguage))
                s_langSubDir = CurrentLanguage + "/";

            // Episode directory
            if(DirMan::exists(g_dirEpisode.getCurDir() + "i18n/" + CurrentLanguage))
                s_langSubDirEp = "i18n/" + CurrentLanguage + "/";
            else if(DirMan::exists(g_dirEpisode.getCurDir() + CurrentLanguage))
                s_langSubDirEp = CurrentLanguage + "/";
        }

        s_useLangDir = !s_langSubDir.empty();
        s_useLangDirEp = !s_langSubDirEp.empty();
    }
}

SDL_FORCE_INLINE bool s_resolveFile(const char **extList,
                                    const char *extGif,
                                    const std::string &fName,
                                    std::string &imgToUse,
                                    bool &isGif)
{
    const char **ext_p;
    const char *ext;

    if(s_useLangDir)
    {
        ext_p = extList;
        while((ext = *ext_p++) != nullptr)
        {
            imgToUse = g_dirCustom.resolveFileCaseExistsAbs(s_langSubDir + fName + ext);
            isGif = (ext == extGif);
            if(!imgToUse.empty())
                return true; // Found that we looked for
        }
    }

    ext_p = extList;
    while((ext = *ext_p++) != nullptr)
    {
        imgToUse = g_dirCustom.resolveFileCaseExistsAbs(fName + ext);
        isGif = (ext == extGif);
        if(!imgToUse.empty())
            return true; // Found that we looked for
    }

    if(s_useLangDirEp)
    {
        ext_p = extList;
        while((ext = *ext_p++) != nullptr)
        {
            imgToUse = g_dirEpisode.resolveFileCaseExistsAbs(s_langSubDirEp + fName + ext);
            isGif = (ext == extGif);
            if(!imgToUse.empty())
                return true; // Found that we looked for
        }
    }

    ext_p = extList;
    while((ext = *ext_p++) != nullptr)
    {
        imgToUse = g_dirEpisode.resolveFileCaseExistsAbs(fName + ext);
        isGif = (ext == extGif);
        if(!imgToUse.empty())
            return true; // Found that we looked for
    }

    return false; // Nothing was found
}

/*!
 * \brief Load the custom GFX sprite
 * \param origPath Path to original texture
 * \param fName file name for custommization target
 * \param width Reference to width field (optional)
 * \param height Reference to height field (optional)
 * \param isCustom Reference to the "is custom" boolean
 * \param texture Target texture to load
 * \param world Is a world map
 * \param skipMask Don't even try to load a masked GIF sprite
 */
static void loadCGFX(const std::string &origPath,
                     const std::string &fName,
                     vbint_t *width, vbint_t *height, bool* isCustom, StdPicture &texture,
                     bool world = false,
                     bool skipMask = false)
{
    std::string loadedPath;
    bool success = false;
    StdPicture_Sub newTexture;

    GFXBackup_t backup;
    backup.remote_width = width;
    backup.remote_height = height;
    backup.remote_isCustom = isCustom;
    backup.remote_texture = &texture;
    if(width)
        backup.width = *width;
    if(height)
        backup.height = *height;
    if(isCustom)
        backup.isCustom = *isCustom;

    bool isGif = false;

#if defined(X_IMG_EXT) && !defined(X_NO_PNG_GIF)
    // look for the image file: ext in custom, png in custom, gif in custom, ext in episode, png in episode, gif in episode
    const char *extsList[] = {X_IMG_EXT, ".png", ".gif", nullptr};
    const char *extGif = extsList[2];
#elif defined(X_IMG_EXT)
    // look for the image file: ext in custom, ext in episode
    const char *extsList[] = {X_IMG_EXT, nullptr};
    const char *extGif = nullptr;
#else
    // look for the image file: png in custom, gif in custom, png in episode, gif in episode
    const char *extsList[] = {".png", ".gif", nullptr};
    const char *extGif = extsList[1];
#endif

    std::string imgToUse;

    if(!s_resolveFile(extsList, extGif, fName, imgToUse, isGif))
        return; // Nothing to do

    if(isGif && !skipMask)
    {
        // look for the mask file: custom, episode, fallback
        std::string maskToUse;

        if(s_useLangDir)
            maskToUse = g_dirCustom.resolveFileCaseExistsAbs(s_langSubDir + fName + "m.gif");

        if(maskToUse.empty())
            maskToUse = g_dirCustom.resolveFileCaseExistsAbs(fName + "m.gif");

        if(s_useLangDirEp && maskToUse.empty())
            maskToUse = g_dirEpisode.resolveFileCaseExistsAbs(s_langSubDirEp + fName + "m.gif");

        if(maskToUse.empty())
            maskToUse = g_dirEpisode.resolveFileCaseExistsAbs(fName + "m.gif");

        if(maskToUse.empty())
            maskToUse = s_dirFallback.resolveFileCaseExistsAbs(fName + "m.gif");

#ifdef DEBUG_BUILD
        pLogDebug("Trying to load custom GFX: %s with mask %s", imgToUse.c_str(), maskToUse.c_str());
#endif
        XRender::lazyLoadPicture(newTexture, imgToUse, maskToUse, origPath);
        success = newTexture.inited;
        loadedPath = imgToUse;
    }
    else
    {
#ifdef DEBUG_BUILD
        pLogDebug("Trying to load custom GFX: %s", imgToUse.c_str());
#endif
        XRender::lazyLoadPicture(newTexture, imgToUse);
        success = newTexture.inited;
        loadedPath = imgToUse;
    }

    if(success)
    {
        // don't keep state from old texture
        XRender::unloadTexture(texture);

        pLogDebug("Loaded custom GFX: %s", loadedPath.c_str());
        if(isCustom)
            *isCustom = true;

        backup.texture_backup = std::move(static_cast<StdPicture_Sub&>(texture));
        static_cast<StdPicture_Sub&>(texture) = std::move(newTexture);

        if(width)
            *width = newTexture.w;
        if(height)
            *height = newTexture.h;

        if(world)
            g_defaultWorldGfxBackup.push_back(std::move(backup));
        else
            g_defaultLevelGfxBackup.push_back(std::move(backup));
    }
}

/* load a single custom border */
static void loadCBorder(const std::string &origPath,
    const std::string &fName,
    bool* isCustom,
    FrameBorder &border)
{
    loadCGFX(origPath, fName, nullptr, nullptr, isCustom, border.tex, false, true);

    // find the frame border info
    std::string res;

    res = g_dirCustom.resolveFileCaseExistsAbs(fName + ".ini");

    if(res.empty())
        res = g_dirEpisode.resolveFileCaseExistsAbs(fName + ".ini");

    // attempt to load frame border info
    if(!res.empty())
    {
        // backup the frame border info
        FrameBorderInfoBackup_t bak;
        bak.remote_info = &border;
        bak.info_backup =  border;
        g_defaultBorderInfoBackup.push_back(bak);

        // load the frame border info
        IniProcessing ini = Files::load_ini(res);
        loadFrameInfo(ini, border);
    }

    // warn if invalid
    const FrameBorderInfo& i = border;
    if(i.le + i.li + i.ri + i.re > border.tex.w)
        pLogWarning("Invalid border: total internal/external width is %d but texture [%s] is only %dpx wide.", i.le + i.li + i.ri + i.re, fName.c_str(), border.tex.w);
    if(i.te + i.ti + i.bi + i.be > border.tex.h)
        pLogWarning("Invalid border: total internal/external height is %d but texture [%s] is only %dpx tall.", i.te + i.ti + i.bi + i.be, fName.c_str(), border.tex.h);
}

#if defined(PGE_MIN_PORT) || defined(THEXTECH_CLI_BUILD)
/*!
 * \brief Load the custom GFX from a load list
 * \param f The load list
 * \param dir The directory to which paths are relative
 * \param texture Target texture to modify
 * \param width Reference to width field (optional)
 * \param height Reference to height field (optional)
 * \param is_custom_ref Reference to the "is custom" boolean
 * \param world Is a world map
 * \param this_is_custom Is custom in the current load context
 */
static void loadImageFromList(PGE_FileFormats_misc::TextInput& t, std::string& line_buf, const std::string& dir,
                    StdPicture &texture,
                    vbint_t *width, vbint_t *height, bool *is_custom_loc,
                    bool world = false, bool this_is_custom = false)
{
    StdPicture_Sub newTexture;
    XRender::lazyLoadPictureFromList(newTexture, t, line_buf, dir);

    if(!newTexture.inited)
        return;

    if(this_is_custom)
    {
        GFXBackup_t backup;
        backup.remote_width = width;
        backup.remote_height = height;
        backup.remote_isCustom = is_custom_loc;
        backup.remote_texture = &texture;
        XRender::unloadTexture(texture);
        if(width)
            backup.width = *width;
        if(height)
            backup.height = *height;
        if(is_custom_loc)
            backup.isCustom = *is_custom_loc;
        backup.texture_backup = static_cast<StdPicture_Sub&>(texture);

        if(world)
            g_defaultWorldGfxBackup.push_back(backup);
        else
            g_defaultLevelGfxBackup.push_back(backup);
    }

    if(this_is_custom && is_custom_loc)
    {
        // pLogDebug("Loaded custom GFX: %s", newTexture.l.path.c_str());
        *is_custom_loc = true;
    }

    static_cast<StdPicture_Sub&>(texture) = newTexture;
    if(width)
        *width = newTexture.w;
    if(height)
        *height = newTexture.h;
}

bool LoadGFXFromList(std::string source_dir, bool custom, bool skip_world)
{
    std::string path = source_dir + "graphics.list";

    SDL_RWops* f = Files::open_file(path, "rb");

    if(!f)
        return false;

    PGE_FileFormats_misc::RWopsTextInput in(f, path);
    std::string line_buf;

    char type_buf[12];
    int A;

    bool failed = false;

    while(true)
    {
        // advance the file to the next entry
        if(failed)
        {
            do
            {
                in.readLine(line_buf);
            } while(!line_buf.empty());

            if(in.eof())
            {
                pLogWarning("No more entries after failure");
                break;
            }
        }

        in.readLine(line_buf);
        if(line_buf.empty() && !in.eof())
            continue;

        failed = true;

        // read the entry!
        SDL_memset(type_buf, 0, sizeof(type_buf));
        int pos;
        if(sscanf(line_buf.c_str(), "%11s %d%n", type_buf, &A, &pos) != 2 || pos != (int)line_buf.size())
        {
            if(in.eof())
                break;
            else
            {
                pLogDebug("Failed to read entry header of the load list");
                continue;
            }
        }


        // load the entry
        if(type_buf[10] == '2')
        {
            if(A > numBackground2)
            {
                pLogWarning("Received load request for invalid bg2 %s %d", type_buf, A);
                continue;
            }

            loadImageFromList(in, line_buf, source_dir,
                GFXBackground2BMP[A], nullptr, nullptr, nullptr,
                false, custom);
        }
        else if(type_buf[9] == 'd')
        {
            if(A > maxBackgroundType)
            {
                pLogWarning("Received load request for invalid background %s %d", type_buf, A);
                continue;
            }

            loadImageFromList(in, line_buf, source_dir,
                GFXBackgroundBMP[A], nullptr, nullptr, nullptr,
                false, custom);

            if(!custom)
            {
                BackgroundWidth[A] = GFXBackground[A].w;
                BackgroundHeight[A] = GFXBackground[A].h;
            }
        }
        else if(type_buf[0] == 'b')
        {
            if(A > maxBlockType || A == BLKID_CONVEYOR_L_CONV || A == BLKID_CONVEYOR_R_CONV)
            {
                pLogWarning("Received load request for invalid block %s %d", type_buf, A);
                continue;
            }

            loadImageFromList(in, line_buf, source_dir,
                GFXBlockBMP[A], nullptr, nullptr, nullptr,
                false, custom);
        }
        else if(type_buf[0] == 'n')
        {
            if(A > maxNPCType)
            {
                pLogWarning("Received load request for invalid NPC %s %d", type_buf, A);
                continue;
            }

            loadImageFromList(in, line_buf, source_dir,
                GFXNPCBMP[A], nullptr, nullptr, nullptr,
                false, custom);

            // load converted conveyer block graphics from conveyer NPC graphics
            if(A == NPCID_CONVEYOR)
            {
                if(custom)
                {
                    for(int i = BLKID_CONVEYOR_L_CONV; i <= BLKID_CONVEYOR_R_CONV; i += BLKID_CONVEYOR_R_CONV - BLKID_CONVEYOR_L_CONV)
                    {
                        GFXBackup_t backup;
                        backup.remote_texture = &GFXBlockBMP[i];
                        XRender::unloadTexture(GFXBlockBMP[i]);
                        backup.texture_backup = static_cast<StdPicture_Sub&>(GFXBlockBMP[i]);
                        g_defaultLevelGfxBackup.push_back(backup);
                    }
                }

                (StdPicture_Sub&)GFXBlockBMP[BLKID_CONVEYOR_L_CONV] = (const StdPicture_Sub&)GFXNPCBMP[NPCID_CONVEYOR];
                (StdPicture_Sub&)GFXBlockBMP[BLKID_CONVEYOR_R_CONV] = (const StdPicture_Sub&)GFXNPCBMP[NPCID_CONVEYOR];
            }
        }
        else if(type_buf[0] == 'e')
        {
            if(A > maxEffectType)
            {
                pLogWarning("Received load request for invalid effect %s %d", type_buf, A);
                continue;
            }

            loadImageFromList(in, line_buf, source_dir,
                GFXEffectBMP[A], &EffectWidth[A], &EffectHeight[A], &GFXEffectCustom[A],
                false, custom);

            // update calculation (but still rely on backup made above)
            if(GFXEffectCustom[A])
                EffectHeight[A] = GFXEffectBMP[A].h / EffectDefaults.EffectFrames[A];
        }
        else if(type_buf[0] == 'y' && type_buf[5] == 't')
        {
            if(A > maxYoshiGfx)
            {
                pLogWarning("Received load request for invalid yoshitop %s %d", type_buf, A);
                continue;
            }

            loadImageFromList(in, line_buf, source_dir,
                GFXYoshiTBMP[A], nullptr, nullptr, nullptr,
                false, custom);
        }
        else if(type_buf[0] == 'y')
        {
            if(A > maxYoshiGfx)
            {
                pLogWarning("Received load request for invalid yoshib %s %d", type_buf, A);
                continue;
            }

            loadImageFromList(in, line_buf, source_dir,
                GFXYoshiBBMP[A], nullptr, nullptr, nullptr,
                false, custom);
        }
        else if(type_buf[0] == 'l' && type_buf[1] == 'e')
        {
            if(skip_world)
                continue;

            if(A > maxLevelType)
            {
                pLogWarning("Received load request for invalid level %s %d", type_buf, A);
                continue;
            }

            loadImageFromList(in, line_buf, source_dir,
                GFXLevelBMP[A], nullptr, nullptr, nullptr,
                true, custom);
        }
        else if(type_buf[0] == 't' && type_buf[1] == 'i')
        {
            if(skip_world)
                continue;

            if(A > maxTileType)
            {
                pLogWarning("Received load request for invalid tile %s %d", type_buf, A);
                continue;
            }

            loadImageFromList(in, line_buf, source_dir,
                GFXTileBMP[A], nullptr, nullptr, nullptr,
                true, custom);
        }
        else if(type_buf[0] == 's')
        {
            if(skip_world)
                continue;

            if(A > maxSceneType)
            {
                pLogWarning("Received load request for invalid scene %s %d", type_buf, A);
                continue;
            }

            loadImageFromList(in, line_buf, source_dir,
                GFXSceneBMP[A], nullptr, nullptr, nullptr,
                true, custom);
        }
        else if(type_buf[0] == 'p' && type_buf[1] == 'l')
        {
            if(skip_world)
                continue;

            if(A > numCharacters)
            {
                pLogWarning("Received load request for invalid worldplayer %s %d", type_buf, A);
                continue;
            }

            loadImageFromList(in, line_buf, source_dir,
                GFXPlayerBMP[A], nullptr, nullptr, nullptr,
                true, custom);
        }
        else if(type_buf[0] == 'p' && type_buf[1] == 'a')
        {
            if(skip_world)
                continue;

            if(A > maxPathType)
            {
                pLogWarning("Received load request for invalid path %s %d", type_buf, A);
                continue;
            }

            loadImageFromList(in, line_buf, source_dir,
                GFXPathBMP[A], nullptr, nullptr, nullptr,
                true, custom);
        }
        // character graphics
        else
        {
            int c;
            if(type_buf[0] == 'l' && type_buf[4] == '\0')
                c = 4;
            else if(type_buf[0] == 'l')
                c = 1;
            else if(type_buf[0] == 't')
                c = 3;
            else if(type_buf[0] == 'm')
                c = 0;
            else if(type_buf[0] == 'p')
                c = 2;
            else
            {
                pLogWarning("Received unrecognized load request category %s", type_buf);
                continue;
            }

            if(A > numStates)
            {
                pLogWarning("Received load request for invalid player state %s %d", type_buf, A);
                continue;
            }

            loadImageFromList(in, line_buf, source_dir,
                (*GFXCharacterBMP[c])[A], nullptr, nullptr, nullptr,
                false, custom);
        }

        failed = false;
    }

    return true;
}

#endif // #if defined(PGE_MIN_PORT) || defined(THEXTECH_CLI_BUILD)

static void s_UnloadPreviewPlayers()
{
    if(s_previewPlayersEnd == 0 || s_previewPlayersEnd == s_previewPlayersBegin)
    {
        s_previewPlayersBegin = 0;
        s_previewPlayersEnd = 0;
        return;
    }

    // check that no textures have been loaded since player preview
    SDL_assert_release(s_previewPlayersEnd == g_defaultLevelGfxBackup.size());

    for(auto it = g_defaultLevelGfxBackup.begin() + s_previewPlayersEnd;
        it > g_defaultLevelGfxBackup.begin() + s_previewPlayersBegin;
        )
    {
        --it;

        auto &t = *it;

        if(t.remote_width)
            *t.remote_width = t.width;
        if(t.remote_height)
            *t.remote_height = t.height;
        if(t.remote_isCustom)
            *t.remote_isCustom = t.isCustom;
        SDL_assert_release(t.remote_texture);
        XRender::unloadTexture(*t.remote_texture);
        *static_cast<StdPicture_Sub*>(t.remote_texture) = std::move(t.texture_backup);
    }

    g_defaultLevelGfxBackup.resize(s_previewPlayersBegin);

    s_previewPlayersBegin = 0;
    s_previewPlayersEnd = 0;

#ifndef LOW_MEM
    // Restore default sizes of custom player effects (should no longer be required, now that backups are made directly)
    for(int A = 1; A < maxEffectType; ++A)
    {
        if(GFXEffectCustom[A])
        {
            SDL_assert_release(EffectWidth[A] == GFXEffectBMP[A].w);
            SDL_assert_release(EffectHeight[A] == GFXEffectBMP[A].h / EffectDefaults.EffectFrames[A]);
        }
        else
        {
            SDL_assert_release(EffectWidth[A] == EffectDefaults.EffectWidth[A]);
            SDL_assert_release(EffectHeight[A] == EffectDefaults.EffectHeight[A]);
        }
    }
#endif // #ifndef LOW_MEM
}

static void restoreLevelBackupTextures()
{
    for(auto it = g_defaultLevelGfxBackup.rbegin(); it != g_defaultLevelGfxBackup.rend(); ++it)
    {
        auto &t = *it;

        if(t.remote_width)
            *t.remote_width = t.width;
        if(t.remote_height)
            *t.remote_height = t.height;
        if(t.remote_isCustom)
            *t.remote_isCustom = t.isCustom;
        SDL_assert_release(t.remote_texture);
        XRender::unloadTexture(*t.remote_texture);
        *static_cast<StdPicture_Sub*>(t.remote_texture) = std::move(t.texture_backup);
    }
    g_defaultLevelGfxBackup.clear();
    s_previewPlayersBegin = 0;
    s_previewPlayersEnd = 0;
}

static void restoreWorldBackupTextures()
{
    for(auto it = g_defaultWorldGfxBackup.rbegin(); it != g_defaultWorldGfxBackup.rend(); ++it)
    {
        auto &t = *it;

        if(t.remote_width)
            *t.remote_width = t.width;
        if(t.remote_height)
            *t.remote_height = t.height;
        if(t.remote_isCustom)
            *t.remote_isCustom = t.isCustom;
        SDL_assert_release(t.remote_texture);
        XRender::unloadTexture(*t.remote_texture);
        *static_cast<StdPicture_Sub*>(t.remote_texture) = std::move(t.texture_backup);
    }
    g_defaultWorldGfxBackup.clear();
}


static inline void s_find_image(std::string& dest, DirListCI& CurDir, std::string baseName)
{
#if defined(X_IMG_EXT) && !defined(X_NO_PNG_GIF)
    int s = baseName.size();
    baseName += X_IMG_EXT;
    dest = CurDir.resolveFileCaseExistsAbs(baseName);
    if(dest.empty())
    {
        baseName.resize(s);
        baseName += ".png";
        dest = CurDir.resolveFileCaseExistsAbs(baseName);
    }
#elif defined(X_IMG_EXT)
    baseName += X_IMG_EXT;
    dest = CurDir.resolveFileCaseExistsAbs(baseName);
#else
    baseName += ".png";
    dest = CurDir.resolveFileCaseExistsAbs(baseName);
#endif
}

void LoadGFX()
{
#if defined(PGE_MIN_PORT) || defined(THEXTECH_CLI_BUILD)
    if(LoadGFXFromList(getGfxDir(), false, false))
        return;
#endif

    std::string p;
    DirListCI CurDir;

    pLogDebug("Loading character textures");
    LoaderUpdateDebugString("Characters");
    for(int c = 0; c < numCharacters; ++c)
    {
        LoaderUpdateDebugString(fmt::sprintf_ne("Character %d", c));
        CurDir.setCurDir(getGfxDir() + GFXPlayerNames[c]);
        For(A, 1, numStates)
        {
            s_find_image(p, CurDir, fmt::sprintf_ne("%s-%d", GFXPlayerNames[c], A));
            if(!p.empty())
                XRender::lazyLoadPicture((*GFXCharacterBMP[c])[A], p);
        }
        UpdateLoad();
    }

    pLogDebug("Loading block textures");
    LoaderUpdateDebugString("Blocks");
    CurDir.setCurDir(getGfxDir() + "block/");
    for(int A = 1; A <= maxBlockType; ++A)
    {
        if(A == BLKID_CONVEYOR_L_CONV || A == BLKID_CONVEYOR_R_CONV)
            continue;

        LoaderUpdateDebugString(fmt::sprintf_ne("Block %d", A));
        s_find_image(p, CurDir, fmt::sprintf_ne("block-%d", A));

        if(!p.empty())
            XRender::lazyLoadPicture(GFXBlockBMP[A], p);
        else
            break;

        if(A % 20 == 0)
            UpdateLoad();
    }
    UpdateLoad();

    pLogDebug("Loading BG2 textures");
    LoaderUpdateDebugString("Backgrounds");
    CurDir.setCurDir(getGfxDir() + "background2/");
    for(int A = 1; A <= numBackground2; ++A)
    {
        LoaderUpdateDebugString(fmt::sprintf_ne("Background %d", A));
        s_find_image(p, CurDir, fmt::sprintf_ne("background2-%d", A));

        if(!p.empty())
            XRender::lazyLoadPicture(GFXBackground2BMP[A], p);
        else
            break;

        if(A % 10 == 0)
            UpdateLoad();
    }
    UpdateLoad();

    pLogDebug("Loading NPC textures");
    LoaderUpdateDebugString("NPC");
    CurDir.setCurDir(getGfxDir() + "npc/");
    for(int A = 1; A <= maxNPCType; ++A)
    {
        LoaderUpdateDebugString(fmt::sprintf_ne("NPC %d", A));
        s_find_image(p, CurDir, fmt::sprintf_ne("npc-%d", A));

        if(!p.empty())
        {
            XRender::lazyLoadPicture(GFXNPCBMP[A], p);

            // load converted conveyer block graphics from conveyer NPC graphics
            if(A == NPCID_CONVEYOR)
            {
                XRender::lazyLoadPicture(GFXBlockBMP[BLKID_CONVEYOR_L_CONV], p);
                XRender::lazyLoadPicture(GFXBlockBMP[BLKID_CONVEYOR_R_CONV], p);
            }

            // GFXNPCWidth(A) = GFXNPCBMP[A].w;
            // GFXNPCHeight(A) = GFXNPCBMP[A].h;
            if(A % 20 == 0)
                UpdateLoad();
        }
        else
        {
            // GFXNPCWidth(A) = 0;
            // GFXNPCHeight(A) = 0;
            break;
        }
    }
    UpdateLoad();

    pLogDebug("Loading effect textures");
    LoaderUpdateDebugString("Effects");
    CurDir.setCurDir(getGfxDir() + "effect/");
    for(int A = 1; A <= maxEffectType; ++A)
    {
        LoaderUpdateDebugString(fmt::sprintf_ne("Effect %d", A));
        s_find_image(p, CurDir, fmt::sprintf_ne("effect-%d", A));

        if(!p.empty())
        {
            XRender::lazyLoadPicture(GFXEffectBMP[A], p);
            if(A % 20 == 0)
                UpdateLoad();
        }
        else
            break;
    }
    UpdateLoad();

    pLogDebug("Loading mount textures");
    LoaderUpdateDebugString("Mounts");
    CurDir.setCurDir(getGfxDir() + "yoshi/");
    for(int A = 1; A <= maxYoshiGfx; ++A)
    {
        LoaderUpdateDebugString(fmt::sprintf_ne("Mount B %d", A));
        s_find_image(p, CurDir, fmt::sprintf_ne("yoshib-%d", A));

        if(!p.empty())
        {
            XRender::lazyLoadPicture(GFXYoshiBBMP[A], p);
            if(A % 20 == 0)
                UpdateLoad();
        }
        else
        {
            break;
        }
    }
    UpdateLoad();

    for(int A = 1; A <= maxYoshiGfx; ++A)
    {
        LoaderUpdateDebugString(fmt::sprintf_ne("Mount T %d", A));
        s_find_image(p, CurDir, fmt::sprintf_ne("yoshit-%d", A));

        if(!p.empty())
        {
            XRender::lazyLoadPicture(GFXYoshiTBMP[A], p);
            if(A % 20 == 0)
                UpdateLoad();
        }
        else
        {
            break;
        }
    }
    UpdateLoad();

    pLogDebug("Loading background textures");
    LoaderUpdateDebugString("BGO");
    CurDir.setCurDir(getGfxDir() + "background/");
    for(int A = 1; A <= maxBackgroundType; ++A)
    {
        LoaderUpdateDebugString(fmt::sprintf_ne("BGO %d", A));
        s_find_image(p, CurDir, fmt::sprintf_ne("background-%d", A));

        if(!p.empty())
        {
            XRender::lazyLoadPicture(GFXBackgroundBMP[A], p);
            BackgroundWidth[A] = GFXBackgroundBMP[A].w;
            BackgroundHeight[A] = GFXBackgroundBMP[A].h;
        }
        else
        {
            break;
        }

        if(A % 20 == 0)
            UpdateLoad();
    }
    UpdateLoad();


// 'world map
    pLogDebug("Loading tile textures");
    LoaderUpdateDebugString("Terrain");
    CurDir.setCurDir(getGfxDir() + "tile/");
    for(int A = 1; A <= maxTileType; ++A)
    {
        LoaderUpdateDebugString(fmt::sprintf_ne("Terrain %d", A));
        s_find_image(p, CurDir, fmt::sprintf_ne("tile-%d", A));
        if(!p.empty())
        {
            XRender::lazyLoadPicture(GFXTileBMP[A], p);
            if(A % 20 == 0)
                UpdateLoad();
        }
        else
        {
            break;
        }
    }
    UpdateLoad();

    pLogDebug("Loading level textures");
    LoaderUpdateDebugString("Level entries");
    CurDir.setCurDir(getGfxDir() + "level/");
    for(int A = 0; A <= maxLevelType; ++A)
    {
        LoaderUpdateDebugString(fmt::sprintf_ne("Level %d", A));
        s_find_image(p, CurDir, fmt::sprintf_ne("level-%d", A));
        if(!p.empty())
        {
            XRender::lazyLoadPicture(GFXLevelBMP[A], p);
            if(A % 20 == 0)
                UpdateLoad();
        }
        else
        {
            break;
        }
    }
    UpdateLoad();

    pLogDebug("Loading scene textures");
    LoaderUpdateDebugString("Scenery");
    CurDir.setCurDir(getGfxDir() + "scene/");
    for(int A = 1; A <= maxSceneType; ++A)
    {
        LoaderUpdateDebugString(fmt::sprintf_ne("Scenery %d", A));
        s_find_image(p, CurDir, fmt::sprintf_ne("scene-%d", A));
        if(!p.empty())
        {
            XRender::lazyLoadPicture(GFXSceneBMP[A], p);
            if(A % 20 == 0)
                UpdateLoad();
        }
        else
        {
            break;
        }
    }
    UpdateLoad();

    pLogDebug("Loading world player textures");
    LoaderUpdateDebugString("World characters");
    CurDir.setCurDir(getGfxDir() + "player/");
    for(int A = 1; A <= numCharacters; ++A)
    {
        LoaderUpdateDebugString(fmt::sprintf_ne("World character %d", A));
        s_find_image(p, CurDir, fmt::sprintf_ne("player-%d", A));
        if(!p.empty())
        {
            XRender::lazyLoadPicture(GFXPlayerBMP[A], p);
            if(A % 20 == 0)
                UpdateLoad();
        }
        else
        {
            break;
        }
    }
    UpdateLoad();

    pLogDebug("Loading path textures");
    LoaderUpdateDebugString("Path cells");
    CurDir.setCurDir(getGfxDir() + "path/");
    for(int A = 1; A <= maxPathType; ++A)
    {
        LoaderUpdateDebugString(fmt::sprintf_ne("Path cell %d", A));
        s_find_image(p, CurDir, fmt::sprintf_ne("path-%d", A));
        if(!p.empty())
        {
            XRender::lazyLoadPicture(GFXPathBMP[A], p);
            if(A % 20 == 0)
                UpdateLoad();
        }
        else
        {
            break;
        }
    }
    UpdateLoad();
}

void UnloadGFX(bool reload)
{
    if(!reload)
    {
        // Do nothing at game exit
        return;
    }

    UnloadCustomGFX();
    UnloadWorldCustomGFX();

    for(int c = 0; c < numCharacters; ++c)
    {
        For(A, 1, 10)
        {
            (*GFXCharacterBMP[c])[A].reset();
        }
    }

    for(int A = 1; A <= maxBlockType; ++A)
        GFXBlockBMP[A].reset();

    for(int A = 1; A <= numBackground2; ++A)
        GFXBackground2BMP[A].reset();

    for(int A = 1; A <= maxNPCType; ++A)
        GFXNPCBMP[A].reset();

    for(int A = 1; A <= maxEffectType; ++A)
        GFXEffectBMP[A].reset();

    for(int A = 1; A <= maxYoshiGfx; ++A)
    {
        GFXYoshiBBMP[A].reset();
        GFXYoshiTBMP[A].reset();
    }

    for(int A = 1; A <= maxBackgroundType; ++A)
        GFXBackgroundBMP[A].reset();

// 'world map
    for(int A = 1; A <= maxTileType; ++A)
        GFXTileBMP[A].reset();

    for(int A = 1; A <= maxLevelType; ++A)
        GFXLevelBMP[A].reset();

    for(int A = 1; A <= maxSceneType; ++A)
        GFXSceneBMP[A].reset();

    for(int A = 1; A <= numCharacters; ++A)
        GFXPlayerBMP[A].reset();

    for(int A = 1; A <= maxPathType; ++A)
        GFXPathBMP[A].reset();
}

static void loadCustomUIAssets()
{
    std::string uiRoot = AppPath + "graphics/ui/";
    size_t ci = 0;

     // these should all have been set previously, but will do no harm
    g_dirEpisode.setCurDir(FileNamePath);
    g_dirCustom.setCurDir(FileNamePath + FileName);
    s_dirFallback.setCurDir(getGfxDir() + "fallback");

    loadCGFX(uiRoot + "BMVs.png",
             "BMVs",
             nullptr, nullptr, &GFX.isCustom(ci++), GFX.BMVs, false, true);

    loadCGFX(uiRoot + "BMWin.png",
             "BMWin",
             nullptr, nullptr, &GFX.isCustom(ci++), GFX.BMWin, false, true);

    For(i, 1, 3)
        loadCGFX(uiRoot + fmt::sprintf_ne("Boot%d.png", i),
                 fmt::sprintf_ne("Boot%d", i),
                 nullptr, nullptr, &GFX.isCustom(ci++), GFX.Boot[i], false, true);

    For(i, 1, 5)
        loadCGFX(uiRoot + fmt::sprintf_ne("CharacterName%d.png", i),
                 fmt::sprintf_ne("CharacterName%d", i),
                 nullptr, nullptr, &GFX.isCustom(ci++), GFX.CharacterName[i], false, true);

    loadCGFX(uiRoot + "Chat.png",
             "Chat",
             nullptr, nullptr, &GFX.isCustom(ci++), GFX.Chat, false, true);

    For(i, 0, 2)
        loadCGFX(uiRoot + fmt::sprintf_ne("Container%d.png", i),
                 fmt::sprintf_ne("Container%d", i),
                 nullptr, nullptr, &GFX.isCustom(ci++), GFX.Container[i], false, true);

    For(i, 1, 3)
        loadCGFX(uiRoot + fmt::sprintf_ne("ECursor%d.png", i),
                 fmt::sprintf_ne("ECursor%d", i),
                 nullptr, nullptr, &GFX.isCustom(ci++), GFX.ECursor[i], false, true);

    For(i, 0, 9)
        loadCGFX(uiRoot + fmt::sprintf_ne("Font1_%d.png", i),
                 fmt::sprintf_ne("Font1_%d", i),
                 nullptr, nullptr, &GFX.isCustom(ci++), GFX.Font1[i], false, true);

    For(i, 1, 3)
        loadCGFX(uiRoot + fmt::sprintf_ne("Font2_%d.png", i),
                 fmt::sprintf_ne("Font2_%d", i),
                 nullptr, nullptr, &GFX.isCustom(ci++), GFX.Font2[i], false, true);

    loadCGFX(uiRoot + "Font2S.png",
             "Font2S",
             nullptr, nullptr, &GFX.isCustom(ci++), GFX.Font2S, false, true);

    For(i, 1, 2)
        loadCGFX(uiRoot + fmt::sprintf_ne("Heart%d.png", i),
                 fmt::sprintf_ne("Heart%d", i),
                 nullptr, nullptr, &GFX.isCustom(ci++), GFX.Heart[i], false, true);

    For(i, 0, 8)
        loadCGFX(uiRoot + fmt::sprintf_ne("Interface%d.png", i),
                 fmt::sprintf_ne("Interface%d", i),
                 nullptr, nullptr, &GFX.isCustom(ci++), GFX.Interface[i], false, true);

    loadCGFX(uiRoot + "LoadCoin.png",
             "LoadCoin",
             nullptr, nullptr, &GFX.isCustom(ci++), GFX.LoadCoin, false, true);

    loadCGFX(uiRoot + "Loader.png",
             "Loader",
             nullptr, nullptr, &GFX.isCustom(ci++), GFX.Loader, false, true);

    For(i, 0, 3)
        loadCGFX(uiRoot + fmt::sprintf_ne("MCursor%d.png", i),
                 fmt::sprintf_ne("MCursor%d", i),
                 nullptr, nullptr, &GFX.isCustom(ci++), GFX.MCursor[i], false, true);

    For(i, 1, 4)
        loadCGFX(uiRoot + fmt::sprintf_ne("MenuGFX%d.png", i),
                 fmt::sprintf_ne("MenuGFX%d", i),
                 nullptr, nullptr, &GFX.isCustom(ci++), GFX.MenuGFX[i], false, true);

    loadCGFX(uiRoot + "Mount.png",
             "Mount",
             nullptr, nullptr, &GFX.isCustom(ci++), GFX.Mount[2], false, true);

    For(i, 0, 7)
        loadCGFX(uiRoot + fmt::sprintf_ne("nCursor%d.png", i),
                 fmt::sprintf_ne("nCursor%d", i),
                 nullptr, nullptr, &GFX.isCustom(ci++), GFX.nCursor[i], false, true);

    loadCGFX(uiRoot + "TextBox.png",
             "TextBox",
             nullptr, nullptr, &GFX.isCustom(ci++), GFX.TextBox, false, true);

    For(i, 1, 2)
        loadCGFX(uiRoot + fmt::sprintf_ne("Tongue%d.png", i),
                 fmt::sprintf_ne("Tongue%d", i),
                 nullptr, nullptr, &GFX.isCustom(ci++), GFX.Tongue[i], false, true);

    // loadCGFX(uiRoot + "Warp.png",
    //          "Warp",
    //          nullptr, nullptr, &GFX.isCustom(ci++), GFX.Warp, false, true);

    loadCGFX(uiRoot + "YoshiWings.png",
             "YoshiWings",
             nullptr, nullptr, &GFX.isCustom(ci++), GFX.YoshiWings, false, true);

    loadCGFX(uiRoot + "CycloneAcc.png",
             "CycloneAcc",
             nullptr, nullptr, &GFX.isCustom(ci++), GFX.CycloneAcc, false, true);

    loadCGFX(uiRoot + "EditorIcons.png",
             "EditorIcons",
             nullptr, nullptr, &GFX.isCustom(ci++), GFX.EIcons, false, true);

    loadCGFX(uiRoot + "PCursor.png",
             "PCursor",
             nullptr, nullptr, &GFX.isCustom(ci++), GFX.PCursor, false, true);

    loadCGFX(uiRoot + "Medals.png",
             "Medals",
             nullptr, nullptr, &GFX.isCustom(ci++), GFX.Medals, false, true);

    loadCGFX(uiRoot + "CharSelIcons.png",
             "CharSelIcons",
             nullptr, nullptr, &GFX.isCustom(ci++), GFX.CharSelIcons, false, true);

    loadCBorder(uiRoot + "CharSelFrame.png",
             "CharSelFrame",
             &GFX.isCustom(ci++), GFX.CharSelFrame);

    loadCGFX(uiRoot + "Backdrop.png",
             "Backdrop",
             nullptr, nullptr, &GFX.isCustom(ci++), GFX.Backdrop, false, true);

    loadCBorder(uiRoot + "Backdrop_Border.png",
             "Backdrop_Border",
             &GFX.isCustom(ci++), GFX.Backdrop_Border);

    loadCGFX(uiRoot + "WorldMapFrame_Tile.png",
             "WorldMapFrame_Tile",
             nullptr, nullptr, &GFX.isCustom(ci++), GFX.WorldMapFrame_Tile, false, true);

    loadCBorder(uiRoot + "WorldMapFrame_Border.png",
             "WorldMapFrame_Border",
             &GFX.isCustom(ci++), GFX.WorldMapFrame_Border);

    loadCGFX(uiRoot + "Camera.png",
             "Camera",
             nullptr, nullptr, &GFX.isCustom(ci++), GFX.Camera, false, true);

    loadCGFX(uiRoot + "Balance.png",
             "Balance",
             nullptr, nullptr, &GFX.isCustom(ci++), GFX.Balance, false, true);

    // Add new optional assets above this line. Also update gfx.cpp: GFX_t::load(), and gfx.h: GFX_t::m_isCustomVolume.
}

void LoadCustomGFX(bool include_world, const char* preview_players_from)
{
    std::string GfxRoot = AppPath + "graphics/";

    // this should have been set previously, but will do no harm
    s_dirFallback.setCurDir(getGfxDir() + "fallback");

    // unload any previously loaded player previews
    s_UnloadPreviewPlayers();

    if(!preview_players_from)
    {
        // these should all have been set previously, but will do no harm
        g_dirEpisode.setCurDir(FileNamePath);
        g_dirCustom.setCurDir(FileNamePath + FileName);

        cgfx_initLangDir();

        loadCustomUIAssets();

#if defined(PGE_MIN_PORT) || defined(THEXTECH_CLI_BUILD)
        bool success = LoadGFXFromList(g_dirEpisode.getCurDir(), true, !include_world);

        if(s_useLangDirEp)
            success |= LoadGFXFromList(g_dirEpisode.getCurDir() + s_langSubDirEp, true, !include_world);

        success |= LoadGFXFromList(g_dirCustom.getCurDir(), true, !include_world);

        if(s_useLangDir)
            success |= LoadGFXFromList(g_dirCustom.getCurDir() + s_langSubDir, true, !include_world);

        if(success)
            return;
#endif
    }
    else
    {
        s_previewPlayersBegin = g_defaultLevelGfxBackup.size();

        g_dirEpisode.setCurDir(preview_players_from);
        g_dirCustom.setCurDir("");

        loadCGFX(AppPath + "graphics/ui/Interface5.png",
                 "Interface5",
                 nullptr, nullptr, &GFX.isCustom(38), GFX.Interface[5], false, true);
    }


    for(int A = 1; A <= maxYoshiGfx; ++A)
    {
        loadCGFX(GfxRoot + fmt::sprintf_ne("yoshi/yoshib-%d.png", A),
                 fmt::sprintf_ne("yoshib-%d", A),
                 nullptr, nullptr, nullptr, GFXYoshiBBMP[A]);
    }

    for(int A = 1; A <= maxYoshiGfx; ++A)
    {
        loadCGFX(GfxRoot + fmt::sprintf_ne("yoshi/yoshit-%d.png", A),
                 fmt::sprintf_ne("yoshit-%d", A),
                 nullptr, nullptr, nullptr, GFXYoshiTBMP[A]);
    }

    for(int c = 0; c < numCharacters; ++c)
    {
        for(int A = 1; A <= numStates; ++A)
        {
            loadCGFX(GfxRoot + fmt::sprintf_ne("{1}/{1}-{0}.png", A, GFXPlayerNames[c]),
                     fmt::format_ne("{1}-{0}", A, GFXPlayerNames[c]),
                     nullptr, nullptr,
                     nullptr, (*GFXCharacterBMP[c])[A]);
        }
    }

    for(int A = 1; A <= maxEffectType; ++A)
    {
        if(preview_players_from && A != EFFID_CHAR1_DIE && A != EFFID_CHAR2_DIE && A != EFFID_CHAR3_DIE && A != EFFID_CHAR4_DIE && A != EFFID_CHAR5_DIE)
            continue;

        loadCGFX(GfxRoot + fmt::format_ne("effect/effect-{0}.png", A),
                 fmt::format_ne("effect-{0}", A),
                 &EffectWidth[A], &EffectHeight[A], &GFXEffectCustom[A], GFXEffectBMP[A]);

        // update calculation (but still rely on backup made above)
        if(GFXEffectCustom[A])
            EffectHeight[A] = GFXEffectBMP[A].h / EffectDefaults.EffectFrames[A];
    }


    if(preview_players_from)
    {
        s_previewPlayersEnd = g_defaultLevelGfxBackup.size();
        return;
    }


    for(int A = 1; A <= maxBlockType; ++A)
    {
        if(A == BLKID_CONVEYOR_L_CONV || A == BLKID_CONVEYOR_R_CONV)
            continue;

        loadCGFX(GfxRoot + fmt::format_ne("block/block-{0}.png", A),
                 fmt::format_ne("block-{0}", A),
                 nullptr, nullptr, nullptr, GFXBlockBMP[A],
                 false, BlockHasNoMask[A]);
    }

    for(int A = 1; A <= numBackground2; ++A)
    {
        loadCGFX(GfxRoot + fmt::format_ne("background2/background2-{0}.png", A),
                 fmt::format_ne("background2-{0}", A),
                 nullptr, nullptr, nullptr, GFXBackground2BMP[A],
                 false, true);
    }

    for(int A = 1; A <= maxNPCType; ++A)
    {
        std::string npc_path = GfxRoot + fmt::format_ne("npc/npc-{0}.png", A);
        std::string npc_fn = fmt::format_ne("npc-{0}", A);

        loadCGFX(npc_path,
                 npc_fn,
                 nullptr, nullptr, nullptr, GFXNPCBMP[A]);

        // load converted conveyer block graphics from conveyer NPC graphics
        if(A == NPCID_CONVEYOR)
        {
            loadCGFX(npc_path,
                     npc_fn,
                     nullptr, nullptr, nullptr, GFXBlockBMP[BLKID_CONVEYOR_L_CONV]);

            loadCGFX(npc_path,
                     npc_fn,
                     nullptr, nullptr, nullptr, GFXBlockBMP[BLKID_CONVEYOR_R_CONV]);
        }
    }

    for(int A = 1; A <= maxBackgroundType; ++A)
    {
        loadCGFX(GfxRoot + fmt::format_ne("background/background-{0}.png", A),
                 fmt::format_ne("background-{0}", A),
                 nullptr, nullptr, nullptr, GFXBackgroundBMP[A],
                 false, BackgroundHasNoMask[A]);
    }


    if(!include_world)
        return;


    for(int A = 1; A <= maxTileType; ++A)
    {
        loadCGFX(GfxRoot + fmt::format_ne("tile/tile-{0}.png", A),
                 fmt::format_ne("tile-{0}", A),
                 nullptr, nullptr, nullptr, GFXTileBMP[A], true);
    }

    for(int A = 0; A <= maxLevelType; ++A)
    {
        loadCGFX(GfxRoot + fmt::format_ne("level/level-{0}.png", A),
                 fmt::format_ne("level-{0}", A),
                 nullptr, nullptr, nullptr, GFXLevelBMP[A], true);
    }

    for(int A = 1; A <= maxSceneType; ++A)
    {
        loadCGFX(GfxRoot + fmt::format_ne("scene/scene-{0}.png", A),
                 fmt::format_ne("scene-{0}", A),
                 nullptr, nullptr, nullptr, GFXSceneBMP[A], true);
    }

    for(int A = 1; A <= numCharacters; ++A)
    {
        loadCGFX(GfxRoot + fmt::format_ne("player/player-{0}.png", A),
                 fmt::format_ne("player-{0}", A),
                 nullptr, nullptr, nullptr, GFXPlayerBMP[A], true);
    }

    for(int A = 1; A <= maxPathType; ++A)
    {
        loadCGFX(GfxRoot + fmt::format_ne("path/path-{0}.png", A),
                 fmt::format_ne("path-{0}", A),
                 nullptr, nullptr, nullptr, GFXPathBMP[A], true);
    }
}


void UnloadCustomGFX()
{
#ifndef RENDER_CUSTOM
    // reset bitmask warning flag on SDL platforms
    XRender::g_BitmaskTexturePresent = false;
#endif

    // unload custom frame border info
    for(auto it = g_defaultBorderInfoBackup.rbegin(); it != g_defaultBorderInfoBackup.rend(); ++it)
    {
        auto &t = *it;

        if(t.remote_info)
            *t.remote_info = t.info_backup;
    }
    g_defaultBorderInfoBackup.clear();

    restoreLevelBackupTextures();

#ifndef LOW_MEM
    // Restore default sizes of custom effects (should no longer be required, now that backups are made directly)
    for(int A = 1; A < maxEffectType; ++A)
    {
        SDL_assert_release(EffectWidth[A] == EffectDefaults.EffectWidth[A]);
        SDL_assert_release(EffectHeight[A] == EffectDefaults.EffectHeight[A]);
    }
#endif // #ifndef LOW_MEM
}

void UnloadPlayerPreviewGFX()
{
    s_UnloadPreviewPlayers();
}


void UnloadWorldCustomGFX()
{
    restoreWorldBackupTextures();
}

void LoaderInit()
{
    gfxLoaderDebugStart = SDL_GetTicks();
    Strings::dealloc(gfxLoaderDebugString);
    gfxLoaderDebugStringUpdated = true;
    gfxLoaderDebugString.reserve(1024);

#ifndef PGE_NO_THREADING
    if(!gfxLoaderDebugMutex)
        gfxLoaderDebugMutex = SDL_CreateMutex();
#endif
}

void LoaderFinish()
{
    gfxLoaderDebugStart = -1;
    Strings::dealloc(gfxLoaderDebugString);
    gfxLoaderDebugStringUpdated = true;

#ifndef PGE_NO_THREADING
    if(gfxLoaderDebugMutex)
        SDL_DestroyMutex(gfxLoaderDebugMutex);
    gfxLoaderDebugMutex = nullptr;
#endif
}

void LoaderUpdateDebugString(const std::string &strig, bool forceUpdate)
{
    if(gfxLoaderDebugStart == -1)
        return;

#ifndef PGE_NO_THREADING
    SDL_LockMutex(gfxLoaderDebugMutex);
#endif
    gfxLoaderDebugString = "Load: " + strig;
    if(forceUpdate)
        gfxLoaderDebugStringUpdated = true;
#ifndef PGE_NO_THREADING
    SDL_UnlockMutex(gfxLoaderDebugMutex);
#endif

    if(!gfxLoaderThreadingMode)
        UpdateLoadREAL();
}


void UpdateLoadREAL()
{
    std::string state;
    bool draw = false;

#ifdef THEXTECH_INTERPROC_SUPPORTED
    if(IntProc::isEnabled())
    {
        state = IntProc::getState();
        draw = true;
    }
#endif // THEXTECH_INTERPROC_SUPPORTED

    bool assets_reload = (ScreenAssetPack::g_LoopActive && g_AssetsLoaded);

    static uint8_t alphaFader = 255;

    if(assets_reload)
        alphaFader = 0;

    if(gfxLoaderDebugStringUpdated)
    {
        gfxLoaderDebugStringUpdated = false;
        draw = true;
    }

    if(LoadCoinsT <= SDL_GetTicks())
    {
        LoadCoinsT = SDL_GetTicks() + 100;
        LoadCoins += 1;
        if(LoadCoins > 3)
            LoadCoins = 0;
        draw = true;
    }

    if(gfxLoaderThreadingMode && alphaFader >= 10)
    {
        alphaFader -= 10;
        draw = true;
    }
    else if(gfxLoaderThreadingMode && alphaFader != 0)
    {
        alphaFader = 0;
        draw = true;
    }

#ifdef USE_RENDER_BLOCKING
    if(XRender::renderBlocked())
        return;
#endif

    if(draw)
    {
        XRender::setTargetTexture();
        XRender::clearBuffer();

        int sh_w = XRender::TargetW / 2;
        int gh_w = GFX.MenuGFX[4].w / 2;
        int sh_h = XRender::TargetH / 2;
        int gh_h = GFX.MenuGFX[4].h / 2;

        // use default coordinates during reload
        if(assets_reload)
        {
            gh_w = 400;
            gh_h = 300;
        }

        int Left    = sh_w - gh_w;
        int Top     = sh_h - gh_h;
        int Right   = sh_w + gh_w;
        int Bottom  = sh_h + gh_h;

        if(Left < 0)
            Left = 0;

        if(Top < 0)
            Top = 0;

        if(Right > XRender::TargetW)
            Right = XRender::TargetW;

        if(Bottom > XRender::TargetH)
            Bottom = XRender::TargetH;

        Left += XRender::TargetOverscanX;
        Right -= XRender::TargetOverscanX;

        if(assets_reload)
        {
            ScreenAssetPack::DrawBackground(1.0);
            DrawDeviceBattery();
        }
        else if(!gfxLoaderTestMode)
        {
            XRender::renderTextureBasic(sh_w - gh_w, sh_h - gh_h, GFX.MenuGFX[4]);
        }
        else
        {
            if(!state.empty())
                SuperPrint(state, 3, Left + 10, Top + 10);
            else
                SuperPrint("Loading data...", 3, Left + 10, Top + 10);
        }

        XRender::renderTextureBasic(Right - 168, Bottom - 24, GFX.Loader);
        XRender::renderTextureBasic(Right - 40, Bottom - 40, GFX.LoadCoin.w, GFX.LoadCoin.h / 4, GFX.LoadCoin, 0, 32 * LoadCoins);

        if(gfxLoaderThreadingMode && alphaFader > 0)
            XRender::renderRect(0, 0, XRender::TargetW, XRender::TargetH, {0, 0, 0, alphaFader});

#ifndef PGE_NO_THREADING
        SDL_LockMutex(gfxLoaderDebugMutex);
#endif

        if(!gfxLoaderDebugString.empty() && gfxLoaderDebugStart + c_gfxLoaderShowInterval < SDL_GetTicks())
        {
            SuperPrint(gfxLoaderDebugString.c_str(), 3,
                       Left + 10, Bottom - 24,
                       {255, 255, 0, 127});
        }

#ifndef PGE_NO_THREADING
        SDL_UnlockMutex(gfxLoaderDebugMutex);
#endif

        XRender::repaint();
        XRender::setTargetScreen();

        XEvents::doEvents();
#ifdef __EMSCRIPTEN__
        emscripten_sleep(1); // To repaint screenn, it's required to send a sleep signal
#endif
    }
}

void UpdateLoad()
{
    if(LoadingInProcess && !gfxLoaderThreadingMode)
        UpdateLoadREAL();
}
