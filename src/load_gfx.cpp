/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2020 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "globals.h"
#include "load_gfx.h"
#include "graphics.h" // SuperPrint
#include <Utils/files.h>
#include <Utils/dir_list_ci.h>
#include <DirManager/dirman.h>
#include <InterProcess/intproc.h>
#include <fmt_format_ne.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include <set>

static DirListCI s_dirEpisode;
static DirListCI s_dirCustom;

bool gfxLoaderTestMode = false;

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

#ifdef DEBUG_BUILD
        pLogDebug("Trying to load custom GFX: %s with mask %s", imgToUse.c_str(), maskToUse.c_str());
#endif
        newTexture = frmMain.lazyLoadPicture(imgToUse, maskToUse, origPath);
        success = newTexture.inited;
        loadedPath = imgToUse;
    }
    else
    {
#ifdef DEBUG_BUILD
        pLogDebug("Trying to load custom GFX: %s", imgToUse.c_str());
#endif
        newTexture = frmMain.lazyLoadPicture(imgToUse);
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
        frmMain.deleteTexture(*t.remote_texture);
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
        frmMain.deleteTexture(*t.remote_texture);
        *t.remote_texture = t.texture;
    }
    g_customWorldCGFXPathsCache.clear();
    g_defaultWorldGfxBackup.clear();
}


void LoadGFX()
{
    std::string p;
    std::string GfxRoot = AppPath + "graphics/";

    For(A, 1, 10)
    {
        p = GfxRoot + fmt::format_ne("mario/mario-{0}.png", A);
        if(Files::fileExists(p))
        {
            GFXMarioBMP[A] = frmMain.lazyLoadPicture(p);
            GFXMarioWidth[A] = GFXMarioBMP[A].w;
            GFXMarioHeight[A] = GFXMarioBMP[A].h;
        }

        p = GfxRoot + fmt::format_ne("luigi/luigi-{0}.png", A);
        if(Files::fileExists(p))
        {
            GFXLuigiBMP[A] = frmMain.lazyLoadPicture(p);
            GFXLuigiWidth[A] = GFXLuigiBMP[A].w;
            GFXLuigiHeight[A] = GFXLuigiBMP[A].h;
        }

        p = GfxRoot + fmt::format_ne("peach/peach-{0}.png", A);
        if(Files::fileExists(p))
        {
            GFXPeachBMP[A] = frmMain.lazyLoadPicture(p);
            GFXPeachWidth[A] = GFXPeachBMP[A].w;
            GFXPeachHeight[A] = GFXPeachBMP[A].h;
        }

        p = GfxRoot + fmt::format_ne("toad/toad-{0}.png", A);
        if(Files::fileExists(p))
        {
            GFXToadBMP[A] = frmMain.lazyLoadPicture(p);
            GFXToadWidth[A] = GFXToadBMP[A].w;
            GFXToadHeight[A] = GFXToadBMP[A].h;
        }

        p = GfxRoot + fmt::format_ne("link/link-{0}.png", A);
        if(Files::fileExists(p))
        {
            GFXLinkBMP[A] = frmMain.lazyLoadPicture(p);
            GFXLinkWidth[A] = GFXLinkBMP[A].w;
            GFXLinkHeight[A] = GFXLinkBMP[A].h;
        }

        UpdateLoad();
    }

    for(int A = 1; A <= maxBlockType; ++A)
    {
        p = GfxRoot + fmt::format_ne("block/block-{0}.png", A);
        if(Files::fileExists(p))
        {
            GFXBlockBMP[A] = frmMain.lazyLoadPicture(p);
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
            GFXBackground2BMP[A] = frmMain.lazyLoadPicture(p);
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
            GFXNPCBMP[A] = frmMain.lazyLoadPicture(p);
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
            GFXEffectBMP[A] = frmMain.lazyLoadPicture(p);
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
            GFXYoshiBBMP[A] = frmMain.lazyLoadPicture(p);
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
            GFXYoshiTBMP[A] = frmMain.lazyLoadPicture(p);
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
            GFXBackgroundBMP[A] = frmMain.lazyLoadPicture(p);
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
            GFXTileBMP[A] = frmMain.lazyLoadPicture(p);
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
            GFXLevelBMP[A] = frmMain.lazyLoadPicture(p);
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
            GFXSceneBMP[A] = frmMain.lazyLoadPicture(p);
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
            GFXPlayerBMP[A] = frmMain.lazyLoadPicture(p);
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
            GFXPathBMP[A] = frmMain.lazyLoadPicture(p);
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

    if(DirMan::exists(FileNamePath + FileName))
    {
        DirMan searchDataDir(FileNamePath + FileName);
        searchDataDir.getListOfFiles(files, {".png", ".gif"});
        for(auto &p : files)
            existingFiles.insert(FileNamePath + FileName  + "/"+ p);
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

    for(int A = 1; A < 10; ++A)
    {
        loadCGFX(existingFiles, GfxRoot + fmt::format_ne("mario/mario-{0}.png", A),
                 FileNamePath, FileName,
                 fmt::format_ne("mario-{0}", A),
                 &GFXMarioWidth[A], &GFXMarioHeight[A], GFXMarioCustom[A], GFXMarioBMP[A]);

        // 'This is a REALLY drunken code, why it was here mixed with CGFX loading when different array is fetched?!
        // 'By this condition, B will always 1, because array can't go more than maximum players
        //If A < 150 Then
        //    B = 1
        //Else
        //    B = 2
        //End If

        // ' This code has no sense as it affects currently usued frames of a "state 1" of this character
        //MarioFrameX(A) = -(GFXMarioWidth(A) / 2 - Physics.PlayerWidth(1, B) / 2)
        //MarioFrameY(A) = -(GFXMarioHeight(A) - Physics.PlayerHeight(1, B) - 2)
    }

    for(int A = 1; A < 10; ++A)
    {
        loadCGFX(existingFiles, GfxRoot + fmt::format_ne("luigi/luigi-{0}.png", A),
                 FileNamePath, FileName,
                 fmt::format_ne("luigi-{0}", A),
                 &GFXLuigiWidth[A], &GFXLuigiHeight[A], GFXLuigiCustom[A], GFXLuigiBMP[A]);

        // ' Same as above at Mario
        //If A < 150 Then
        //    B = 1
        //Else
        //    B = 2
        //End If
        //LuigiFrameX(A) = -(GFXLuigiWidth(A) / 2 - Physics.PlayerWidth(1, B) / 2)
        //LuigiFrameY(A) = -(GFXLuigiHeight(A) - Physics.PlayerHeight(1, B) - 2)
    }

    for(int A = 1; A < 10; ++A)
    {
        loadCGFX(existingFiles, GfxRoot + fmt::format_ne("peach/peach-{0}.png", A),
                 FileNamePath, FileName,
                 fmt::format_ne("peach-{0}", A),
                 &GFXPeachWidth[A], &GFXPeachHeight[A], GFXPeachCustom[A], GFXPeachBMP[A]);
        // ' Same as above at Mario
        //If A < 150 Then
        //    B = 1
        //Else
        //    B = 2
        //End If
        //PeachFrameX(A) = -(GFXPeachWidth(A) / 2 - Physics.PlayerWidth(1, B) / 2)
        //PeachFrameY(A) = -(GFXPeachHeight(A) - Physics.PlayerHeight(1, B) - 2)
    }

    for(int A = 1; A < 10; ++A)
    {
        loadCGFX(existingFiles, GfxRoot + fmt::format_ne("toad/toad-{0}.png", A),
                 FileNamePath, FileName,
                 fmt::format_ne("toad-{0}", A),
                 &GFXToadWidth[A], &GFXToadHeight[A], GFXToadCustom[A], GFXToadBMP[A]);
        // ' Same as above at Mario
        //If A < 150 Then
        //    B = 1
        //Else
        //    B = 2
        //End If
        //ToadFrameX(A) = -(GFXToadWidth(A) / 2 - Physics.PlayerWidth(1, B) / 2)
        //ToadFrameY(A) = -(GFXToadHeight(A) - Physics.PlayerHeight(1, B) - 2)
    }

    for(int A = 1; A < 10; ++A)
    {
        loadCGFX(existingFiles, GfxRoot + fmt::format_ne("link/link-{0}.png", A),
                 FileNamePath, FileName,
                 fmt::format_ne("link-{0}", A),
                 &GFXLinkWidth[A], &GFXLinkHeight[A], GFXLinkCustom[A], GFXLinkBMP[A]);
        // ' Same as above at Mario
        //If A < 150 Then
        //    B = 1
        //Else
        //    B = 2
        //End If
        //LinkFrameX(A) = -(GFXLinkWidth(A) / 2 - Physics.PlayerWidth(1, B) / 2)
        //LinkFrameY(A) = -(GFXLinkHeight(A) - Physics.PlayerHeight(1, B) - 2)
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

void UpdateLoad()
{
    std::string state;
    bool draw = false;
    if(IntProc::isEnabled())
    {
        state = IntProc::getState();
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

    if(draw)
    {
        frmMain.clearBuffer();
        if(!gfxLoaderTestMode)
            frmMain.renderTexture(0, 0, GFX.MenuGFX[4]);
        else
        {
            if(!state.empty())
                SuperPrint(state, 3, 10, 10);
            else
                SuperPrint("Loading data...", 3, 10, 10);
        }
        frmMain.renderTexture(632, 576, GFX.Loader);
        frmMain.renderTexture(760, 560, GFX.LoadCoin.w, GFX.LoadCoin.h / 4, GFX.LoadCoin, 0, 32 * LoadCoins);

        frmMain.repaint();
        DoEvents();
#ifdef __EMSCRIPTEN__
        emscripten_sleep(1); // To repaint screenn, it's required to send a sleep signal
#endif
    }
}
