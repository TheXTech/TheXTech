#include "globals.h"
#include "load_gfx.h"
#include <Utils/files.h>
#include <DirManager/dirman.h>
#include <fmt_format_ne.h>

#include <ctime>
#include <set>

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

static void loadCGFX(const std::set<std::string> &files,
                     const std::string &origPath,
                     const std::string &dEpisode,
                     const std::string &dData,
                     const std::string &fName,
                     int *width, int *height, bool& isCustom, StdPicture &texture,
                     bool world = false)
{
    std::string imgPath = dEpisode + fName + ".png";
    std::string gifPath = dEpisode + fName + ".gif";
    std::string maskPath = dEpisode + fName + "m.gif";

    std::string imgPathC = dEpisode + dData + "/" + fName + ".png";
    std::string gifPathC = dEpisode + dData + "/" + fName + ".gif";
    std::string maskPathC = dEpisode + dData + "/" + fName + "m.gif";

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

    if(isGif)
    {
        if(files.find(maskPathC) != files.end())
            maskToUse = maskPathC;
        else if(files.find(maskPath) != files.end())
            maskToUse = maskPath;

#ifdef DEBUG_BUILD
        pLogDebug("Trying to load custom GFX: %s with mask %s", imgToUse.c_str(), maskToUse.c_str());
#endif
        newTexture = frmMain.LoadPicture(imgToUse, maskToUse, origPath);
        success = newTexture.inited;
        loadedPath = imgToUse;
    }
    else
    {
#ifdef DEBUG_BUILD
        pLogDebug("Trying to load custom GFX: %s", imgToUse.c_str());
#endif
        newTexture = frmMain.LoadPicture(imgToUse);
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
            g_defaultWorldGfxBackup.push_back(backup);
        else
            g_defaultLevelGfxBackup.push_back(backup);
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
        assert(t.remote_texture);
        frmMain.deleteTexture(*t.remote_texture);
        *t.remote_texture = t.texture;
    }
    g_defaultLevelGfxBackup.clear();
}

static void restoreWorldBackupTextures()
{
    for(auto &t : g_defaultLevelGfxBackup)
    {
        if(t.remote_width)
            *t.remote_width = t.width;
        if(t.remote_height)
            *t.remote_height = t.height;
        if(t.remote_isCustom)
            *t.remote_isCustom = false;
        assert(t.remote_texture);
        frmMain.deleteTexture(*t.remote_texture);
        *t.remote_texture = t.texture;
    }
    g_defaultLevelGfxBackup.clear();
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
            GFXMarioBMP[A] = frmMain.LoadPicture(p);
            GFXMarioWidth[A] = GFXMarioBMP[A].w;
            GFXMarioHeight[A] = GFXMarioBMP[A].h;
        }

        p = GfxRoot + fmt::format_ne("luigi/luigi-{0}.png", A);
        if(Files::fileExists(p))
        {
            GFXLuigiBMP[A] = frmMain.LoadPicture(p);
            GFXLuigiWidth[A] = GFXLuigiBMP[A].w;
            GFXLuigiHeight[A] = GFXLuigiBMP[A].h;
        }

        p = GfxRoot + fmt::format_ne("peach/peach-{0}.png", A);
        if(Files::fileExists(p))
        {
            GFXPeachBMP[A] = frmMain.LoadPicture(p);
            GFXPeachWidth[A] = GFXPeachBMP[A].w;
            GFXPeachHeight[A] = GFXPeachBMP[A].h;
        }

        p = GfxRoot + fmt::format_ne("toad/toad-{0}.png", A);
        if(Files::fileExists(p))
        {
            GFXToadBMP[A] = frmMain.LoadPicture(p);
            GFXToadWidth[A] = GFXToadBMP[A].w;
            GFXToadHeight[A] = GFXToadBMP[A].h;
        }

        p = GfxRoot + fmt::format_ne("link/link-{0}.png", A);
        if(Files::fileExists(p))
        {
            GFXLinkBMP[A] = frmMain.LoadPicture(p);
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
            GFXBlockBMP[A] = frmMain.LoadPicture(p);
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
            GFXBackground2BMP[A] = frmMain.LoadPicture(p);
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
            GFXNPCBMP[A] = frmMain.LoadPicture(p);
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
            GFXEffectBMP[A] = frmMain.LoadPicture(p);
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
            GFXYoshiBBMP[A] = frmMain.LoadPicture(p);
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
            GFXYoshiTBMP[A] = frmMain.LoadPicture(p);
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
            GFXBackgroundBMP[A] = frmMain.LoadPicture(p);
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
            GFXTileBMP[A] = frmMain.LoadPicture(p);
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
            GFXLevelBMP[A] = frmMain.LoadPicture(p);
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
            GFXSceneBMP[A] = frmMain.LoadPicture(p);
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
            GFXPlayerBMP[A] = frmMain.LoadPicture(p);
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
            GFXPathBMP[A] = frmMain.LoadPicture(p);
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
    frmMain.clearAllTextures();
}

static void getExistingFiles(std::set<std::string> &existingFiles)
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
                 &GFXBackground2Width[A], &GFXBackground2Height[A], GFXBackground2Custom[A], GFXBackground2BMP[A]);
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
    }

    for(int A = 1; A < 10; ++A)
    {
        loadCGFX(existingFiles, GfxRoot + fmt::format_ne("luigi/luigi-{0}.png", A),
                 FileNamePath, FileName,
                 fmt::format_ne("luigi-{0}", A),
                 &GFXLuigiWidth[A], &GFXLuigiHeight[A], GFXLuigiCustom[A], GFXLuigiBMP[A]);
    }

    for(int A = 1; A < 10; ++A)
    {
        loadCGFX(existingFiles, GfxRoot + fmt::format_ne("peach/peach-{0}.png", A),
                 FileNamePath, FileName,
                 fmt::format_ne("peach-{0}", A),
                 &GFXPeachWidth[A], &GFXPeachHeight[A], GFXPeachCustom[A], GFXPeachBMP[A]);
    }

    for(int A = 1; A < 10; ++A)
    {
        loadCGFX(existingFiles, GfxRoot + fmt::format_ne("toad/toad-{0}.png", A),
                 FileNamePath, FileName,
                 fmt::format_ne("toad-{0}", A),
                 &GFXToadWidth[A], &GFXToadHeight[A], GFXToadCustom[A], GFXToadBMP[A]);
    }

    for(int A = 1; A < 10; ++A)
    {
        loadCGFX(existingFiles, GfxRoot + fmt::format_ne("link/link-{0}.png", A),
                 FileNamePath, FileName,
                 fmt::format_ne("link-{0}", A),
                 &GFXLinkWidth[A], &GFXLinkHeight[A], GFXLinkCustom[A], GFXLinkBMP[A]);
    }
}

void UnloadCustomGFX()
{
    restoreLevelBackupTextures();
}

void LoadCustomGFX2(std::string /*GFXFilePath*/)
{
    // Useless now
}

void LoadWorldCustomGFX()
{
    std::string GfxRoot = AppPath + "graphics/";
    std::set<std::string> existingFiles;
    getExistingFiles(existingFiles);

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
    if(LoadCoinsT <= SDL_GetTicks())
    {
        LoadCoinsT = SDL_GetTicks() + 100;
        LoadCoins += 1;
        if(LoadCoins > 3)
            LoadCoins = 0;

        frmMain.clearBuffer();
        frmMain.renderTexture(0, 0, GFX.MenuGFX[4]);
        frmMain.renderTexture(632, 576, GFX.Loader);
        frmMain.renderTexture(760, 560, GFX.LoadCoin.w, GFX.LoadCoin.h / 4, GFX.LoadCoin, 0, 32 * LoadCoins);

        frmMain.repaint();
        DoEvents();
    }
}
