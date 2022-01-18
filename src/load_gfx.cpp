/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2022 Vitaly Novichkov <admin@wohlnet.ru>
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

#include <SDL2/SDL_timer.h>

#include "globals.h"
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

static DirListCI s_dirEpisode;
static DirListCI s_dirCustom;

bool gfxLoaderTestMode = false;
bool gfxLoaderThreadingMode = false;

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
static std::set<std::string> g_customLevelCGFXPathsCache;

static std::vector<GFXBackup_t> g_defaultWorldGfxBackup;
static std::set<std::string> g_customWorldCGFXPathsCache;

static std::string getGfxDir()
{
    return AppPath + "graphics/";
}

static void loadCGFX(const std::set<std::string> &files,
                     const std::string &origPath,
                     const std::string &dEpisode,
                     const std::string &dData,
                     const std::string &fName,
                     int *width, int *height, bool& isCustom, StdPicture &texture,
                     bool world = false,
                     bool skipMask = false)
{
    std::string imgPath = dEpisode + s_dirEpisode.resolveFileCase(fName + ".png");
    std::string gifPath = dEpisode + s_dirEpisode.resolveFileCase(fName + ".gif");
    std::string maskPath = dEpisode + s_dirEpisode.resolveFileCase(fName + "m.gif");

    std::string imgPathC = dEpisode + dData + "/" + s_dirEpisode.resolveFileCase(fName + ".png");
    std::string gifPathC = dEpisode + dData + "/" + s_dirCustom.resolveFileCase(fName + ".gif");
    std::string maskPathC = dEpisode + dData + "/" + s_dirCustom.resolveFileCase(fName + "m.gif");

    std::string maskPathFall = getGfxDir() + "fallback/" + fName + "m.gif";

    bool alreadyLoaded = false;

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
    backup.texture = texture;

    std::string imgToUse;
    std::string maskToUse;
    bool isGif = false;

    if(files.find(imgPathC) != files.end()) {
        imgToUse = imgPathC;
    } else if(files.find(gifPathC) != files.end()) {
        isGif = true;
        imgToUse = gifPathC;
    } else if(files.find(imgPath) != files.end()) {
        imgToUse = imgPath;
    } else if(files.find(gifPath) != files.end()) {
        isGif = true;
        imgToUse = gifPath;
    }

    if(imgToUse.empty())
        return; // Nothing to do

    if(world)
        alreadyLoaded = g_customWorldCGFXPathsCache.find(imgToUse) != g_customWorldCGFXPathsCache.end();
    else
        alreadyLoaded = g_customLevelCGFXPathsCache.find(imgToUse) != g_customLevelCGFXPathsCache.end();

    if(alreadyLoaded)
        return; // This texture is already loaded

    if(isGif && !skipMask)
    {
        if(files.find(maskPathC) != files.end())
            maskToUse = maskPathC;
        else if(files.find(maskPath) != files.end())
            maskToUse = maskPath;
        else if(files.find(maskPathFall) != files.end())
            maskToUse = maskPathFall;

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
        pLogDebug("Loaded custom GFX: %s", loadedPath.c_str());
        isCustom = true;
        texture = newTexture;
        if(width)
            *width = newTexture.w;
        if(height)
            *height = newTexture.h;
        if(world)
        {
            g_defaultWorldGfxBackup.push_back(backup);
            g_customWorldCGFXPathsCache.insert(loadedPath);
        }
        else
        {
            g_defaultLevelGfxBackup.push_back(backup);
            g_customLevelCGFXPathsCache.insert(loadedPath);
        }
    }
}

static void restoreLevelBackupTextures()
{
    for(auto &t : g_defaultLevelGfxBackup)
    {
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
    g_customLevelCGFXPathsCache.clear();
    g_defaultLevelGfxBackup.clear();
}

static void restoreWorldBackupTextures()
{
    for(auto &t : g_defaultWorldGfxBackup)
    {
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
    g_customWorldCGFXPathsCache.clear();
    g_defaultWorldGfxBackup.clear();
}


void LoadGFX()
{
    std::string p;
    std::string GfxRoot = getGfxDir();

    for(int c = 0; c < numCharacters; ++c)
    {
        For(A, 1, 10)
        {
            p = GfxRoot + fmt::format_ne("{1}/{1}-{0}.png", A, GFXPlayerNames[c]);
            if(Files::fileExists(p))
            {
                (*GFXCharacterBMP[c])[A] = XRender::lazyLoadPicture(p);
                (*GFXCharacterWidth[c])[A] = GFXMarioBMP[A].w;
                (*GFXCharacterHeight[c])[A] = GFXMarioBMP[A].h;
            }
        }
        UpdateLoad();
    }

    for(int A = 1; A <= maxBlockType; ++A)
    {
        p = GfxRoot + fmt::format_ne("block/block-{0}.png", A);
        if(Files::fileExists(p))
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

    for(int A = 1; A <= numBackground2; ++A)
    {
        p = GfxRoot + fmt::format_ne("background2/background2-{0}.png", A);
        if(Files::fileExists(p))
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

    for(int A = 1; A <= maxNPCType; ++A)
    {
        p = GfxRoot + fmt::format_ne("npc/npc-{0}.png", A);
        if(Files::fileExists(p))
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

    for(int A = 1; A <= maxEffectType; ++A)
    {
        p = GfxRoot + fmt::format_ne("effect/effect-{0}.png", A);
        if(Files::fileExists(p))
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

    for(int A = 1; A <= maxYoshiGfx; ++A)
    {
        p = GfxRoot + fmt::format_ne("yoshi/yoshib-{0}.png", A);
        if(Files::fileExists(p))
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
        p = GfxRoot + fmt::format_ne("yoshi/yoshit-{0}.png", A);
        if(Files::fileExists(p))
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

    for(int A = 1; A <= maxBackgroundType; ++A)
    {
        p = GfxRoot + fmt::format_ne("background/background-{0}.png", A);
        if(Files::fileExists(p))
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
    for(int A = 1; A <= maxTileType; ++A)
    {
        p = GfxRoot + fmt::format_ne("tile/tile-{0}.png", A);
        if(Files::fileExists(p))
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

    for(int A = 0; A <= maxLevelType; ++A)
    {
        p = GfxRoot + fmt::format_ne("level/level-{0}.png", A);
        if(Files::fileExists(p))
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

    for(int A = 1; A <= maxSceneType; ++A)
    {
        p = GfxRoot + fmt::format_ne("scene/scene-{0}.png", A);
        if(Files::fileExists(p))
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

    for(int A = 1; A <= numCharacters; ++A)
    {
        p = GfxRoot + fmt::format_ne("player/player-{0}.png", A);
        if(Files::fileExists(p))
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

    for(int A = 1; A <= maxPathType; ++A)
    {
        p = GfxRoot + fmt::format_ne("path/path-{0}.png", A);
        if(Files::fileExists(p))
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

static SDL_INLINE void getExistingFiles(std::set<std::string> &existingFiles)
{
    DirMan searchDir(FileNamePath);
    std::vector<std::string> files;
    searchDir.getListOfFiles(files, {".png", ".gif"});

    for(auto &p : files)
        existingFiles.insert(FileNamePath + p);

    std::string epFileData = FileNamePath + FileName;
    if(DirMan::exists(epFileData))
    {
        DirMan searchDataDir(epFileData);
        searchDataDir.getListOfFiles(files, {".png", ".gif"});
        for(auto &p : files)
            existingFiles.insert(epFileData  + "/"+ p);
    }

    std::string fallBacksDir = getGfxDir() + "fallback";
    if(DirMan::exists(fallBacksDir))
    {
        DirMan searchDataDir(fallBacksDir);
        searchDataDir.getListOfFiles(files, {"m.gif"});
        for(auto &p : files)
            existingFiles.insert(fallBacksDir  + "/"+ p);
    }
}

void LoadCustomGFX()
{
    std::string GfxRoot = AppPath + "graphics/";
    std::set<std::string> existingFiles;
    getExistingFiles(existingFiles);

    s_dirEpisode.setCurDir(FileNamePath);
    s_dirCustom.setCurDir(FileNamePath + FileName);

    for(int A = 1; A < maxBlockType; ++A)
    {
        loadCGFX(existingFiles, GfxRoot + fmt::format_ne("block/block-{0}.png", A),
                 FileNamePath, FileName,
                 fmt::format_ne("block-{0}", A),
                 nullptr, nullptr, GFXBlockCustom[A], GFXBlockBMP[A]);
    }

    for(int A = 1; A < numBackground2; ++A)
    {
        loadCGFX(existingFiles, GfxRoot + fmt::format_ne("background2/background2-{0}.png", A),
                 FileNamePath, FileName,
                 fmt::format_ne("background2-{0}", A),
                 &GFXBackground2Width[A], &GFXBackground2Height[A], GFXBackground2Custom[A], GFXBackground2BMP[A],
                 false, true);
    }

    for(int A = 1; A < maxNPCType; ++A)
    {
        loadCGFX(existingFiles, GfxRoot + fmt::format_ne("npc/npc-{0}.png", A),
                 FileNamePath, FileName,
                 fmt::format_ne("npc-{0}", A),
                 &GFXNPCWidth[A], &GFXNPCHeight[A], GFXNPCCustom[A], GFXNPCBMP[A]);
    }

    for(int A = 1; A < maxEffectType; ++A)
    {
        loadCGFX(existingFiles, GfxRoot + fmt::format_ne("effect/effect-{0}.png", A),
                 FileNamePath, FileName,
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
        loadCGFX(existingFiles, GfxRoot + fmt::format_ne("background/background-{0}.png", A),
                 FileNamePath, FileName,
                 fmt::format_ne("background-{0}", A),
                 &GFXBackgroundWidth[A], &GFXBackgroundHeight[A], GFXBackgroundCustom[A], GFXBackgroundBMP[A]);
    }

    for(int A = 1; A < maxYoshiGfx; ++A)
    {
        loadCGFX(existingFiles, GfxRoot + fmt::format_ne("yoshi/yoshib-{0}.png", A),
                 FileNamePath, FileName,
                 fmt::format_ne("yoshib-{0}", A),
                 nullptr, nullptr, GFXYoshiBCustom[A], GFXYoshiBBMP[A]);
    }

    for(int A = 1; A < maxYoshiGfx; ++A)
    {
        loadCGFX(existingFiles, GfxRoot + fmt::format_ne("yoshi/yoshit-{0}.png", A),
                 FileNamePath, FileName,
                 fmt::format_ne("yoshit-{0}", A),
                 nullptr, nullptr, GFXYoshiTCustom[A], GFXYoshiTBMP[A]);
    }

    for(int c = 0; c < numCharacters; ++c)
    {
        for(int A = 1; A <= numStates; ++A)
        {
            loadCGFX(existingFiles, GfxRoot + fmt::format_ne("{1}/{1}-{0}.png", A, GFXPlayerNames[c]),
                     FileNamePath, FileName,
                     fmt::format_ne("{1}-{0}", A, GFXPlayerNames[c]),
                     &(*GFXCharacterWidth[c])[A], &(*GFXCharacterHeight[c])[A],
                     (*GFXCharacterCustom[c])[A], (*GFXCharacterBMP[c])[A]);
        }
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



void LoadWorldCustomGFX()
{
    std::string GfxRoot = AppPath + "graphics/";
    std::set<std::string> existingFiles;
    getExistingFiles(existingFiles);

    s_dirEpisode.setCurDir(FileNamePath);
    s_dirCustom.setCurDir(FileNamePath + FileName);

    for(int A = 1; A < maxTileType; ++A)
    {
        loadCGFX(existingFiles, GfxRoot + fmt::format_ne("tile/tile-{0}.png", A),
                 FileNamePath, FileName,
                 fmt::format_ne("tile-{0}", A),
                 &GFXTileWidth[A], &GFXTileHeight[A], GFXTileCustom[A], GFXTileBMP[A], true);
    }

    for(int A = 0; A < maxLevelType; ++A)
    {
        loadCGFX(existingFiles, GfxRoot + fmt::format_ne("level/level-{0}.png", A),
                 FileNamePath, FileName,
                 fmt::format_ne("level-{0}", A),
                 &GFXLevelWidth[A], &GFXLevelHeight[A], GFXLevelCustom[A], GFXLevelBMP[A], true);
    }

    for(int A = 1; A < maxSceneType; ++A)
    {
        loadCGFX(existingFiles, GfxRoot + fmt::format_ne("scene/scene-{0}.png", A),
                 FileNamePath, FileName,
                 fmt::format_ne("scene-{0}", A),
                 &GFXSceneWidth[A], &GFXSceneHeight[A], GFXSceneCustom[A], GFXSceneBMP[A], true);
    }

    for(int A = 1; A < numCharacters; ++A)
    {
        loadCGFX(existingFiles, GfxRoot + fmt::format_ne("player/player-{0}.png", A),
                 FileNamePath, FileName,
                 fmt::format_ne("player-{0}", A),
                 &GFXPlayerWidth[A], &GFXPlayerHeight[A], GFXPlayerCustom[A], GFXPlayerBMP[A], true);
    }

    for(int A = 1; A < maxPathType; ++A)
    {
        loadCGFX(existingFiles, GfxRoot + fmt::format_ne("path/path-{0}.png", A),
                 FileNamePath, FileName,
                 fmt::format_ne("path-{0}", A),
                 &GFXPathWidth[A], &GFXPathHeight[A], GFXPathCustom[A], GFXPathBMP[A], true);
    }
}

void UnloadWorldCustomGFX()
{
    restoreWorldBackupTextures();
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
