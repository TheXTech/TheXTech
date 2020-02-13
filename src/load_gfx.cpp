#include "globals.h"
#include "load_gfx.h"
#include <Utils/files.h>
#include <DirManager/dirman.h>
#include <fmt_format_ne.h>

#include <ctime>

// Private Sub cBlockGFX(A As Integer)
void cBlockGFX(int A);
// Private Sub cNPCGFX(A As Integer)
void cNPCGFX(int A);
// Private Sub cBackgroundGFX(A As Integer)
void cBackgroundGFX(int A);
// Private Sub cTileGFX(A As Integer)
void cTileGFX(int A);
// Private Sub cSceneGFX(A As Integer)
void cSceneGFX(int A);
// Private Sub cLevelGFX(A As Integer)
void cLevelGFX(int A);
// Private Sub cPathGFX(A As Integer)
void cPathGFX(int A);


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

void LoadCustomGFX()
{

}

void UnloadCustomGFX()
{

}

void LoadCustomGFX2(std::string GFXFilePath)
{

}

void LoadWorldCustomGFX()
{

}

void UnloadWorldCustomGFX()
{

}

void cBlockGFX(int A)
{

}

void cNPCGFX(int A)
{

}

void cBackgroundGFX(int A)
{

}

void cTileGFX(int A)
{

}

void cSceneGFX(int A)
{

}

void cLevelGFX(int A)
{

}

void cPathGFX(int A)
{

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
