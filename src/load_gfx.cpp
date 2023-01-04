/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2023 Vitaly Novichkov <admin@wohlnet.ru>
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

#include "gfx.h"
#include "load_gfx.h"
#include "graphics.h" // SuperPrint
#include "core/render.h"
#include "core/events.h"
#include <Utils/files.h>
#include <Utils/dir_list_ci.h>
#include <DirManager/dirman.h>
#ifdef THEXTECH_INTERPROC_SUPPORTED
#   include <InterProcess/intproc.h>
#endif
#include <fmt_format_ne.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include <set>

bool gfxLoaderTestMode = false;
bool gfxLoaderThreadingMode = false;
#ifndef PGE_NO_THREADING
static SDL_mutex *gfxLoaderDebugMutex = nullptr;
#endif
static std::string gfxLoaderDebugString;
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
    int *remote_width = nullptr;
    int *remote_height = nullptr;
    bool *remote_isCustom = nullptr;
    StdPicture *remote_texture = nullptr;
    int width = 0;
    int height = 0;
    StdPicture texture;
};

static std::vector<GFXBackup_t> g_defaultLevelGfxBackup;
static std::vector<GFXBackup_t> g_defaultWorldGfxBackup;

static DirListCI s_dirFallback;

static std::string getGfxDir()
{
    return AppPath + "graphics/";
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
                     int *width, int *height, bool& isCustom, StdPicture &texture,
                     bool world = false,
                     bool skipMask = false)
{
    std::string loadedPath;
    bool success = false;
    StdPicture newTexture;

    GFXBackup_t backup;
    backup.remote_width = width;
    backup.remote_height = height;
    backup.remote_isCustom = &isCustom;
    backup.remote_texture = &texture;
    if(width)
        backup.width = *width;
    if(height)
        backup.height = *height;

    bool isGif = false;

#if defined(X_IMG_EXT) && !defined(X_NO_PNG_GIF)
    // look for the image file: ext in custom, png in custom, gif in custom, ext in episode, png in episode, gif in episode
    std::string imgToUse = g_dirCustom.resolveFileCaseExistsAbs(fName + X_IMG_EXT);
    if(imgToUse.empty())
    {
        imgToUse = g_dirCustom.resolveFileCaseExistsAbs(fName + ".png");
    }
    if(imgToUse.empty())
    {
        imgToUse = g_dirCustom.resolveFileCaseExistsAbs(fName + ".gif");
        isGif = true;
    }
    if(imgToUse.empty())
    {
        imgToUse = g_dirEpisode.resolveFileCaseExistsAbs(fName + X_IMG_EXT);
        isGif = false;
    }
    if(imgToUse.empty())
    {
        imgToUse = g_dirEpisode.resolveFileCaseExistsAbs(fName + ".png");
    }
    if(imgToUse.empty())
    {
        imgToUse = g_dirEpisode.resolveFileCaseExistsAbs(fName + ".gif");
        isGif = true;
    }
#elif defined(X_IMG_EXT)
    // look for the image file: ext in custom, ext in episode
    std::string imgToUse = g_dirCustom.resolveFileCaseExistsAbs(fName + X_IMG_EXT);
    if(imgToUse.empty())
    {
        imgToUse = g_dirEpisode.resolveFileCaseExistsAbs(fName + X_IMG_EXT);
        isGif = false;
    }
#else
    // look for the image file: png in custom, gif in custom, png in episode, gif in episode
    std::string imgToUse = g_dirCustom.resolveFileCaseExistsAbs(fName + ".png");
    if(imgToUse.empty())
    {
        imgToUse = g_dirCustom.resolveFileCaseExistsAbs(fName + ".gif");
        isGif = true;
    }
    if(imgToUse.empty())
    {
        imgToUse = g_dirEpisode.resolveFileCaseExistsAbs(fName + ".png");
        isGif = false;
    }
    if(imgToUse.empty())
    {
        imgToUse = g_dirEpisode.resolveFileCaseExistsAbs(fName + ".gif");
        isGif = true;
    }
#endif

    if(imgToUse.empty())
        return; // Nothing to do

    if(isGif && !skipMask)
    {
        // look for the mask file: custom, episode, fallback
        std::string maskToUse = g_dirCustom.resolveFileCaseExistsAbs(fName + "m.gif");
        if(maskToUse.empty())
            maskToUse = g_dirEpisode.resolveFileCaseExistsAbs(fName + "m.gif");
        if(maskToUse.empty())
            maskToUse = s_dirFallback.resolveFileCaseExistsAbs(fName + "m.gif");

#ifdef DEBUG_BUILD
        pLogDebug("Trying to load custom GFX: %s with mask %s", imgToUse.c_str(), maskToUse.c_str());
#endif
        newTexture = XRender::lazyLoadPicture(imgToUse, maskToUse, origPath);
        success = newTexture.inited;
        loadedPath = imgToUse;
    }
    else
    {
#ifdef DEBUG_BUILD
        pLogDebug("Trying to load custom GFX: %s", imgToUse.c_str());
#endif
        newTexture = XRender::lazyLoadPicture(imgToUse);
        success = newTexture.inited;
        loadedPath = imgToUse;
    }

    if(success)
    {
        // don't allow texture to be incorrectly tracked as loaded
        XRender::lazyUnLoad(texture);

        pLogDebug("Loaded custom GFX: %s", loadedPath.c_str());
        isCustom = true;

        backup.texture = texture;
        texture = newTexture;

        if(width)
            *width = newTexture.w;
        if(height)
            *height = newTexture.h;

        if(world)
            g_defaultWorldGfxBackup.push_back(backup);
        else
            g_defaultLevelGfxBackup.push_back(backup);
    }
}

#ifdef PGE_MIN_PORT
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
static void loadImageFromList(FILE* f, const std::string& dir,
                    StdPicture &texture,
                    int *width, int *height, bool &is_custom_loc,
                    bool world = false, bool this_is_custom = false)
{
    StdPicture newTexture = XRender::lazyLoadPictureFromList(f, dir);

    if(!newTexture.inited)
        return;

    if(this_is_custom)
    {
        GFXBackup_t backup;
        backup.remote_width = width;
        backup.remote_height = height;
        backup.remote_isCustom = &is_custom_loc;
        backup.remote_texture = &texture;
        XRender::lazyUnLoad(texture);
        if(width)
            backup.width = *width;
        if(height)
            backup.height = *height;
        backup.texture = texture;

        if(world)
            g_defaultWorldGfxBackup.push_back(backup);
        else
            g_defaultLevelGfxBackup.push_back(backup);
    }

    if(this_is_custom)
    {
        // pLogDebug("Loaded custom GFX: %s", newTexture.l.path.c_str());
        is_custom_loc = true;
    }

    texture = newTexture;
    if(width)
        *width = newTexture.w;
    if(height)
        *height = newTexture.h;
}

bool LoadGFXFromList(std::string source_dir, bool custom, bool skip_world)
{
    std::string path = source_dir + "graphics.list";

    FILE* f = fopen(path.c_str(), "r");

    if(!f)
        return false;

    char type_buf[12];
    int A;

    bool failed = false;

    while(true)
    {
        // advance the file to the next entry
        if(failed)
        {
            char out = '\n';

            while(out == '\n')
            {
                // pLogDebug("skipping failed lines at %d", ftell(f));
                if(fscanf(f, "%*[^\n]%c", &out) != 1)
                    break;
            }

            if(fgetc(f) != '\n')
            {
                pLogWarning("No more entries after failure");
                break;
            }
        }

        failed = true;

        // read the entry!
        SDL_memset(type_buf, 0, sizeof(type_buf));
        if(fscanf(f, "%11s %d", type_buf, &A) != 2 || fgetc(f) != '\n')
        {
            if(feof(f))
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

            loadImageFromList(f, source_dir,
                GFXBackground2BMP[A], &GFXBackground2Width[A], &GFXBackground2Height[A], GFXBackground2Custom[A],
                false, custom);
        }
        else if(type_buf[9] == 'd')
        {
            if(A > maxBackgroundType)
            {
                pLogWarning("Received load request for invalid background %s %d", type_buf, A);
                continue;
            }

            loadImageFromList(f, source_dir,
                GFXBackgroundBMP[A], &GFXBackgroundWidth[A], &GFXBackgroundHeight[A], GFXBackgroundCustom[A],
                false, custom);

            if(!custom)
            {
                BackgroundWidth[A] = GFXBackgroundWidth[A];
                BackgroundHeight[A] = GFXBackgroundHeight[A];
            }
        }
        else if(type_buf[0] == 'b')
        {
            if(A > maxBlockType)
            {
                pLogWarning("Received load request for invalid block %s %d", type_buf, A);
                continue;
            }

            loadImageFromList(f, source_dir,
                GFXBlockBMP[A], nullptr, nullptr, GFXBlockCustom[A],
                false, custom);
        }
        else if(type_buf[0] == 'n')
        {
            if(A > maxNPCType)
            {
                pLogWarning("Received load request for invalid NPC %s %d", type_buf, A);
                continue;
            }

            loadImageFromList(f, source_dir,
                GFXNPCBMP[A], &GFXNPCWidth[A], &GFXNPCHeight[A], GFXNPCCustom[A],
                false, custom);
        }
        else if(type_buf[0] == 'e')
        {
            if(A > maxEffectType)
            {
                pLogWarning("Received load request for invalid effect %s %d", type_buf, A);
                continue;
            }

            loadImageFromList(f, source_dir,
                GFXEffectBMP[A], &GFXEffectWidth[A], &GFXEffectHeight[A], GFXEffectCustom[A],
                false, custom);
        }
        else if(type_buf[0] == 'y' && type_buf[5] == 't')
        {
            if(A > maxYoshiGfx)
            {
                pLogWarning("Received load request for invalid yoshitop %s %d", type_buf, A);
                continue;
            }

            loadImageFromList(f, source_dir,
                GFXYoshiTBMP[A], nullptr, nullptr, GFXYoshiTCustom[A],
                false, custom);
        }
        else if(type_buf[0] == 'y')
        {
            if(A > maxYoshiGfx)
            {
                pLogWarning("Received load request for invalid yoshib %s %d", type_buf, A);
                continue;
            }

            loadImageFromList(f, source_dir,
                GFXYoshiBBMP[A], nullptr, nullptr, GFXYoshiBCustom[A],
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

            loadImageFromList(f, source_dir,
                GFXLevelBMP[A], &GFXLevelWidth[A], &GFXLevelHeight[A], GFXLevelCustom[A],
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

            loadImageFromList(f, source_dir,
                GFXTileBMP[A], &GFXTileWidth[A], &GFXTileHeight[A], GFXTileCustom[A],
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

            loadImageFromList(f, source_dir,
                GFXSceneBMP[A], &GFXSceneWidth[A], &GFXSceneHeight[A], GFXSceneCustom[A],
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

            loadImageFromList(f, source_dir,
                GFXPlayerBMP[A], &GFXPlayerWidth[A], &GFXPlayerHeight[A], GFXPlayerCustom[A],
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

            loadImageFromList(f, source_dir,
                GFXPathBMP[A], &GFXPathWidth[A], &GFXPathHeight[A], GFXPathCustom[A],
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

            loadImageFromList(f, source_dir,
                (*GFXCharacterBMP[c])[A], &(*GFXCharacterWidth[c])[A], &(*GFXCharacterHeight[c])[A], (*GFXCharacterCustom[c])[A],
                true, custom);
        }

        failed = false;
    }

    fclose(f);

    return true;
}

#endif // #ifdef PGE_MIN_PORT

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
            *t.remote_isCustom = false;
        SDL_assert_release(t.remote_texture);
        XRender::deleteTexture(*t.remote_texture);
        *t.remote_texture = t.texture;
    }
    g_defaultLevelGfxBackup.clear();
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
            *t.remote_isCustom = false;
        SDL_assert_release(t.remote_texture);
        XRender::deleteTexture(*t.remote_texture);
        *t.remote_texture = t.texture;
    }
    g_defaultWorldGfxBackup.clear();
}


static inline void s_find_image(std::string& dest, DirListCI& CurDir, std::string basename)
{
#if defined(X_IMG_EXT) && !defined(X_NO_PNG_GIF)
    int s = basename.size();
    basename += X_IMG_EXT;
    dest = CurDir.resolveFileCaseExistsAbs(basename);
    if(dest.empty())
    {
        basename.resize(s);
        basename += ".png";
        dest = CurDir.resolveFileCaseExistsAbs(basename);
    }
#elif defined(X_IMG_EXT)
    basename += X_IMG_EXT;
    dest = CurDir.resolveFileCaseExistsAbs(basename);
#else
    basename += ".png";
    dest = CurDir.resolveFileCaseExistsAbs(basename);
#endif
}

void LoadGFX()
{
#ifdef PGE_MIN_PORT
    if(LoadGFXFromList(getGfxDir(), false, false))
        return;
#endif

    std::string p;
    DirListCI CurDir;

    pLogDebug("Loading character textures");
    LoaderUpdateDebugString("Characters");
    for(int c = 0; c < numCharacters; ++c)
    {
        LoaderUpdateDebugString(fmt::format_ne("Character {0}", c));
        CurDir.setCurDir(getGfxDir() + GFXPlayerNames[c]);
        For(A, 1, 10)
        {
            s_find_image(p, CurDir, fmt::format_ne("{1}-{0}", A, GFXPlayerNames[c]));
            if(!p.empty())
            {
                (*GFXCharacterBMP[c])[A] = XRender::lazyLoadPicture(p);
                (*GFXCharacterWidth[c])[A] = (*GFXCharacterBMP[c])[A].w;
                (*GFXCharacterHeight[c])[A] = (*GFXCharacterBMP[c])[A].h;
            }
        }
        UpdateLoad();
    }

    pLogDebug("Loading block textures");
    LoaderUpdateDebugString("Blocks");
    CurDir.setCurDir(getGfxDir() + "block/");
    for(int A = 1; A <= maxBlockType; ++A)
    {
        LoaderUpdateDebugString(fmt::format_ne("Block {0}", A));
        s_find_image(p, CurDir, fmt::format_ne("block-{0}", A));
        if(!p.empty())
        {
            GFXBlockBMP[A] = XRender::lazyLoadPicture(p);
        }
        else
        {
            break;
        }
        if(A % 20 == 0)
            UpdateLoad();
    }
    UpdateLoad();

    pLogDebug("Loading BG2 textures");
    LoaderUpdateDebugString("Backgrounds");
    CurDir.setCurDir(getGfxDir() + "background2/");
    for(int A = 1; A <= numBackground2; ++A)
    {
        LoaderUpdateDebugString(fmt::format_ne("Background {0}", A));
        s_find_image(p, CurDir, fmt::format_ne("background2-{0}", A));
        if(!p.empty())
        {
            GFXBackground2BMP[A] = XRender::lazyLoadPicture(p);
            GFXBackground2Width[A] = GFXBackground2BMP[A].w;
            GFXBackground2Height[A] = GFXBackground2BMP[A].h;
        }
        else
        {
            GFXBackground2Width[A] = 0;
            GFXBackground2Height[A] = 0;
            break;
        }
        if(A % 10 == 0) UpdateLoad();
    }
    UpdateLoad();

    pLogDebug("Loading NPC textures");
    LoaderUpdateDebugString("NPC");
    CurDir.setCurDir(getGfxDir() + "npc/");
    for(int A = 1; A <= maxNPCType; ++A)
    {
        LoaderUpdateDebugString(fmt::format_ne("NPC {0}", A));
        s_find_image(p, CurDir, fmt::format_ne("npc-{0}", A));
        if(!p.empty())
        {
            GFXNPCBMP[A] = XRender::lazyLoadPicture(p);
            GFXNPCWidth[A] = GFXNPCBMP[A].w;
            GFXNPCHeight[A] = GFXNPCBMP[A].h;
            if(A % 20 == 0)
                UpdateLoad();
        }
        else
        {
            GFXNPCWidth[A] = 0;
            GFXNPCHeight[A] = 0;
            break;
        }
    }
    UpdateLoad();

    pLogDebug("Loading effect textures");
    LoaderUpdateDebugString("Effects");
    CurDir.setCurDir(getGfxDir() + "effect/");
    for(int A = 1; A <= maxEffectType; ++A)
    {
        LoaderUpdateDebugString(fmt::format_ne("Effect {0}", A));
        s_find_image(p, CurDir, fmt::format_ne("effect-{0}", A));
        if(!p.empty())
        {
            GFXEffectBMP[A] = XRender::lazyLoadPicture(p);
            GFXEffectWidth[A] = GFXEffectBMP[A].w;
            GFXEffectHeight[A] = GFXEffectBMP[A].h;
            if(A % 20 == 0)
                UpdateLoad();
        }
        else
        {
            GFXEffectWidth[A] = 0;
            GFXEffectHeight[A] = 0;
            break;
        }
    }
    UpdateLoad();

    pLogDebug("Loading mount textures");
    LoaderUpdateDebugString("Mounts");
    CurDir.setCurDir(getGfxDir() + "yoshi/");
    for(int A = 1; A <= maxYoshiGfx; ++A)
    {
        LoaderUpdateDebugString(fmt::format_ne("Mount B {0}", A));
        s_find_image(p, CurDir, fmt::format_ne("yoshib-{0}", A));
        if(!p.empty())
        {
            GFXYoshiBBMP[A] = XRender::lazyLoadPicture(p);
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
        LoaderUpdateDebugString(fmt::format_ne("Mount T {0}", A));
        s_find_image(p, CurDir, fmt::format_ne("yoshit-{0}", A));
        if(!p.empty())
        {
            GFXYoshiTBMP[A] = XRender::lazyLoadPicture(p);
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
        LoaderUpdateDebugString(fmt::format_ne("BGO {0}", A));
        s_find_image(p, CurDir, fmt::format_ne("background-{0}", A));
        if(!p.empty())
        {
            GFXBackgroundBMP[A] = XRender::lazyLoadPicture(p);
            GFXBackgroundWidth[A] = GFXBackgroundBMP[A].w;
            GFXBackgroundHeight[A] = GFXBackgroundBMP[A].h;
            BackgroundWidth[A] = GFXBackgroundWidth[A];
            BackgroundHeight[A] = GFXBackgroundHeight[A];
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
        LoaderUpdateDebugString(fmt::format_ne("Terrain {0}", A));
        s_find_image(p, CurDir, fmt::format_ne("tile-{0}", A));
        if(!p.empty())
        {
            GFXTileBMP[A] = XRender::lazyLoadPicture(p);
            GFXTileWidth[A] = GFXTileBMP[A].w;
            GFXTileHeight[A] = GFXTileBMP[A].h;
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
        LoaderUpdateDebugString(fmt::format_ne("Level {0}", A));
        s_find_image(p, CurDir, fmt::format_ne("level-{0}", A));
        if(!p.empty())
        {
            GFXLevelBMP[A] = XRender::lazyLoadPicture(p);
            GFXLevelWidth[A] = GFXLevelBMP[A].w;
            GFXLevelHeight[A] = GFXLevelBMP[A].h;
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
        LoaderUpdateDebugString(fmt::format_ne("Scenery {0}", A));
        s_find_image(p, CurDir, fmt::format_ne("scene-{0}", A));
        if(!p.empty())
        {
            GFXSceneBMP[A] = XRender::lazyLoadPicture(p);
            GFXSceneWidth[A] = GFXSceneBMP[A].w;
            GFXSceneHeight[A] = GFXSceneBMP[A].h;
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
        LoaderUpdateDebugString(fmt::format_ne("World character {0}", A));
        s_find_image(p, CurDir, fmt::format_ne("player-{0}", A));
        if(!p.empty())
        {
            GFXPlayerBMP[A] = XRender::lazyLoadPicture(p);
            GFXPlayerWidth[A] = GFXPlayerBMP[A].w;
            GFXPlayerHeight[A] = GFXPlayerBMP[A].h;
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
        LoaderUpdateDebugString(fmt::format_ne("Path cell {0}", A));
        s_find_image(p, CurDir, fmt::format_ne("path-{0}", A));
        if(!p.empty())
        {
            GFXPathBMP[A] = XRender::lazyLoadPicture(p);
            GFXPathWidth[A] = GFXPathBMP[A].w;
            GFXPathHeight[A] = GFXPathBMP[A].h;
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

void UnloadGFX()
{
    // Do nothing
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
             nullptr, nullptr, GFX.isCustom(ci++), GFX.BMVs, false, true);

    loadCGFX(uiRoot + "BMWin.png",
             "BMWin",
             nullptr, nullptr, GFX.isCustom(ci++), GFX.BMWin, false, true);

    For(i, 1, 3)
        loadCGFX(uiRoot + fmt::format_ne("Boot{0}.png", i),
                 fmt::format_ne("Boot{0}", i),
                 nullptr, nullptr, GFX.isCustom(ci++), GFX.Boot[i], false, true);

    For(i, 1, 5)
        loadCGFX(uiRoot + fmt::format_ne("CharacterName{0}.png", i),
                 fmt::format_ne("CharacterName{0}", i),
                 nullptr, nullptr, GFX.isCustom(ci++), GFX.CharacterName[i], false, true);

    loadCGFX(uiRoot + "Chat.png",
             "Chat",
             nullptr, nullptr, GFX.isCustom(ci++), GFX.Chat, false, true);

    For(i, 0, 2)
        loadCGFX(uiRoot + fmt::format_ne("Container{0}.png", i),
                 fmt::format_ne("Container{0}", i),
                 nullptr, nullptr, GFX.isCustom(ci++), GFX.Container[i], false, true);

    For(i, 1, 3)
        loadCGFX(uiRoot + fmt::format_ne("ECursor{0}.png", i),
                 fmt::format_ne("ECursor{0}", i),
                 nullptr, nullptr, GFX.isCustom(ci++), GFX.ECursor[i], false, true);

    For(i, 0, 9)
        loadCGFX(uiRoot + fmt::format_ne("Font1_{0}.png", i),
                 fmt::format_ne("Font1_{0}", i),
                 nullptr, nullptr, GFX.isCustom(ci++), GFX.Font1[i], false, true);

    For(i, 1, 3)
        loadCGFX(uiRoot + fmt::format_ne("Font2_{0}.png", i),
                 fmt::format_ne("Font2_{0}", i),
                 nullptr, nullptr, GFX.isCustom(ci++), GFX.Font2[i], false, true);

    loadCGFX(uiRoot + "Font2S.png",
             "Font2S",
             nullptr, nullptr, GFX.isCustom(ci++), GFX.Font2S, false, true);

    For(i, 1, 2)
        loadCGFX(uiRoot + fmt::format_ne("Heart{0}.png", i),
                 fmt::format_ne("Heart{0}", i),
                 nullptr, nullptr, GFX.isCustom(ci++), GFX.Heart[i], false, true);

    For(i, 0, 8)
        loadCGFX(uiRoot + fmt::format_ne("Interface{0}.png", i),
                 fmt::format_ne("Interface{0}", i),
                 nullptr, nullptr, GFX.isCustom(ci++), GFX.Interface[i], false, true);

    loadCGFX(uiRoot + "LoadCoin.png",
             "LoadCoin",
             nullptr, nullptr, GFX.isCustom(ci++), GFX.LoadCoin, false, true);

    loadCGFX(uiRoot + "Loader.png",
             "Loader",
             nullptr, nullptr, GFX.isCustom(ci++), GFX.Loader, false, true);

    For(i, 0, 3)
        loadCGFX(uiRoot + fmt::format_ne("MCursor{0}.png", i),
                 fmt::format_ne("MCursor{0}", i),
                 nullptr, nullptr, GFX.isCustom(ci++), GFX.MCursor[i], false, true);

    For(i, 1, 4)
        loadCGFX(uiRoot + fmt::format_ne("MenuGFX{0}.png", i),
                 fmt::format_ne("MenuGFX{0}", i),
                 nullptr, nullptr, GFX.isCustom(ci++), GFX.MenuGFX[i], false, true);

    loadCGFX(uiRoot + "Mount.png",
             "Mount",
             nullptr, nullptr, GFX.isCustom(ci++), GFX.Mount[2], false, true);

    For(i, 0, 7)
        loadCGFX(uiRoot + fmt::format_ne("nCursor{0}.png", i),
                 fmt::format_ne("nCursor{0}", i),
                 nullptr, nullptr, GFX.isCustom(ci++), GFX.nCursor[i], false, true);

    loadCGFX(uiRoot + "TextBox.png",
             "TextBox",
             nullptr, nullptr, GFX.isCustom(ci++), GFX.TextBox, false, true);

    For(i, 1, 2)
        loadCGFX(uiRoot + fmt::format_ne("Tongue{0}.png", i),
                 fmt::format_ne("Tongue{0}", i),
                 nullptr, nullptr, GFX.isCustom(ci++), GFX.Tongue[i], false, true);

    loadCGFX(uiRoot + "Warp.png",
             "Warp",
             nullptr, nullptr, GFX.isCustom(ci++), GFX.Warp, false, true);

    loadCGFX(uiRoot + "YoshiWings.png",
             "YoshiWings",
             nullptr, nullptr, GFX.isCustom(ci++), GFX.YoshiWings, false, true);

    loadCGFX(uiRoot + "EditorIcons.png",
             "EditorIcons",
             nullptr, nullptr, GFX.isCustom(ci++), GFX.EIcons, false, true);
}

void LoadCustomGFX(bool include_world)
{
    std::string GfxRoot = AppPath + "graphics/";

     // these should all have been set previously, but will do no harm
    g_dirEpisode.setCurDir(FileNamePath);
    g_dirCustom.setCurDir(FileNamePath + FileName);
    s_dirFallback.setCurDir(getGfxDir() + "fallback");

    loadCustomUIAssets();

#ifdef PGE_MIN_PORT
    bool success = LoadGFXFromList(g_dirEpisode.getCurDir(), true, !include_world);
    success |= LoadGFXFromList(g_dirCustom.getCurDir(), true, !include_world);
    if(success)
        return;
#endif

    for(int A = 1; A < maxBlockType; ++A)
    {
        loadCGFX(GfxRoot + fmt::format_ne("block/block-{0}.png", A),
                 fmt::format_ne("block-{0}", A),
                 nullptr, nullptr, GFXBlockCustom[A], GFXBlockBMP[A],
                 false, BlockHasNoMask[A]);
    }

    for(int A = 1; A < numBackground2; ++A)
    {
        loadCGFX(GfxRoot + fmt::format_ne("background2/background2-{0}.png", A),
                 fmt::format_ne("background2-{0}", A),
                 &GFXBackground2Width[A], &GFXBackground2Height[A], GFXBackground2Custom[A], GFXBackground2BMP[A],
                 false, true);
    }

    for(int A = 1; A < maxNPCType; ++A)
    {
        loadCGFX(GfxRoot + fmt::format_ne("npc/npc-{0}.png", A),
                 fmt::format_ne("npc-{0}", A),
                 &GFXNPCWidth[A], &GFXNPCHeight[A], GFXNPCCustom[A], GFXNPCBMP[A]);
    }

    for(int A = 1; A < maxEffectType; ++A)
    {
        loadCGFX(GfxRoot + fmt::format_ne("effect/effect-{0}.png", A),
                 fmt::format_ne("effect-{0}", A),
                 &GFXEffectWidth[A], &GFXEffectHeight[A], GFXEffectCustom[A], GFXEffectBMP[A]);

        if(GFXEffectCustom[A])
        {
            EffectWidth[A] = GFXEffectWidth[A];
            EffectHeight[A] = GFXEffectHeight[A] / EffectDefaults.EffectFrames[A];
        }
    }

    for(int A = 1; A < maxBackgroundType; ++A)
    {
        loadCGFX(GfxRoot + fmt::format_ne("background/background-{0}.png", A),
                 fmt::format_ne("background-{0}", A),
                 &GFXBackgroundWidth[A], &GFXBackgroundHeight[A], GFXBackgroundCustom[A], GFXBackgroundBMP[A],
                 false, BackgroundHasNoMask[A]);
    }

    for(int A = 1; A < maxYoshiGfx; ++A)
    {
        loadCGFX(GfxRoot + fmt::format_ne("yoshi/yoshib-{0}.png", A),
                 fmt::format_ne("yoshib-{0}", A),
                 nullptr, nullptr, GFXYoshiBCustom[A], GFXYoshiBBMP[A]);
    }

    for(int A = 1; A < maxYoshiGfx; ++A)
    {
        loadCGFX(GfxRoot + fmt::format_ne("yoshi/yoshit-{0}.png", A),
                 fmt::format_ne("yoshit-{0}", A),
                 nullptr, nullptr, GFXYoshiTCustom[A], GFXYoshiTBMP[A]);
    }

    for(int c = 0; c < numCharacters; ++c)
    {
        for(int A = 1; A <= numStates; ++A)
        {
            loadCGFX(GfxRoot + fmt::format_ne("{1}/{1}-{0}.png", A, GFXPlayerNames[c]),
                     fmt::format_ne("{1}-{0}", A, GFXPlayerNames[c]),
                     &(*GFXCharacterWidth[c])[A], &(*GFXCharacterHeight[c])[A],
                     (*GFXCharacterCustom[c])[A], (*GFXCharacterBMP[c])[A]);
        }
    }

    if(!include_world)
        return;


    for(int A = 1; A < maxTileType; ++A)
    {
        loadCGFX(GfxRoot + fmt::format_ne("tile/tile-{0}.png", A),
                 fmt::format_ne("tile-{0}", A),
                 &GFXTileWidth[A], &GFXTileHeight[A], GFXTileCustom[A], GFXTileBMP[A], true);
    }

    for(int A = 0; A < maxLevelType; ++A)
    {
        loadCGFX(GfxRoot + fmt::format_ne("level/level-{0}.png", A),
                 fmt::format_ne("level-{0}", A),
                 &GFXLevelWidth[A], &GFXLevelHeight[A], GFXLevelCustom[A], GFXLevelBMP[A], true);
    }

    for(int A = 1; A < maxSceneType; ++A)
    {
        loadCGFX(GfxRoot + fmt::format_ne("scene/scene-{0}.png", A),
                 fmt::format_ne("scene-{0}", A),
                 &GFXSceneWidth[A], &GFXSceneHeight[A], GFXSceneCustom[A], GFXSceneBMP[A], true);
    }

    for(int A = 1; A < numCharacters; ++A)
    {
        loadCGFX(GfxRoot + fmt::format_ne("player/player-{0}.png", A),
                 fmt::format_ne("player-{0}", A),
                 &GFXPlayerWidth[A], &GFXPlayerHeight[A], GFXPlayerCustom[A], GFXPlayerBMP[A], true);
    }

    for(int A = 1; A < maxPathType; ++A)
    {
        loadCGFX(GfxRoot + fmt::format_ne("path/path-{0}.png", A),
                 fmt::format_ne("path-{0}", A),
                 &GFXPathWidth[A], &GFXPathHeight[A], GFXPathCustom[A], GFXPathBMP[A], true);
    }
}


void UnloadCustomGFX()
{
    // Restore default sizes of custom effects
    for(int A = 1; A < maxEffectType; ++A)
    {
        EffectWidth[A] = EffectDefaults.EffectWidth[A];
        EffectHeight[A] = EffectDefaults.EffectHeight[A];
    }

    restoreLevelBackupTextures();
}



void UnloadWorldCustomGFX()
{
    restoreWorldBackupTextures();
}

void LoaderInit()
{
    gfxLoaderDebugStart = SDL_GetTicks();
    gfxLoaderDebugString.clear();
#ifndef PGE_NO_THREADING
    if(gfxLoaderDebugMutex)
        gfxLoaderDebugMutex = SDL_CreateMutex();
#endif
}

void LoaderFinish()
{
    gfxLoaderDebugStart = -1;
    gfxLoaderDebugString.clear();
#ifndef PGE_NO_THREADING
    if(gfxLoaderDebugMutex)
        SDL_DestroyMutex(gfxLoaderDebugMutex);
    gfxLoaderDebugMutex = nullptr;
#endif
}

void LoaderUpdateDebugString(const std::string &strig)
{
    if(gfxLoaderDebugStart == -1)
        return;

#ifndef PGE_NO_THREADING
    SDL_LockMutex(gfxLoaderDebugMutex);
#endif
    gfxLoaderDebugString = "Load: " + strig;
#ifndef PGE_NO_THREADING
    SDL_UnlockMutex(gfxLoaderDebugMutex);
#endif
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

    static float alphaFader = 1.0f;

    if(LoadCoinsT <= SDL_GetTicks())
    {
        LoadCoinsT = SDL_GetTicks() + 100;
        LoadCoins += 1;
        if(LoadCoins > 3)
            LoadCoins = 0;
        draw = true;
    }

    if(gfxLoaderThreadingMode && alphaFader >= 0.f)
    {
        alphaFader -= 0.04f;
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
        if(!gfxLoaderTestMode)
            XRender::renderTexture(0, 0, GFX.MenuGFX[4]);
        else
        {
            if(!state.empty())
                SuperPrint(state, 3, 10, 10);
            else
                SuperPrint("Loading data...", 3, 10, 10);
        }

        XRender::renderTexture(632, 576, GFX.Loader);
        XRender::renderTexture(760, 560, GFX.LoadCoin.w, GFX.LoadCoin.h / 4, GFX.LoadCoin, 0, 32 * LoadCoins);

        if(gfxLoaderThreadingMode && alphaFader >= 0.f)
            XRender::renderRect(0, 0, ScreenW, ScreenH, 0.f, 0.f, 0.f, alphaFader);

        if(!gfxLoaderDebugString.empty() && gfxLoaderDebugStart + c_gfxLoaderShowInterval < SDL_GetTicks())
        {
#ifndef PGE_NO_THREADING
            SDL_LockMutex(gfxLoaderDebugMutex);
#endif
            SuperPrint(gfxLoaderDebugString.c_str(), 3,
                       10, ScreenH - 24,
                       1.f, 1.f, 0.f, 0.5f);

#ifndef PGE_NO_THREADING
            SDL_UnlockMutex(gfxLoaderDebugMutex);
#endif
        }

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
    if(!gfxLoaderThreadingMode)
        UpdateLoadREAL();
}
