#include "globals.h"
#include "load_gfx.h"

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

}

void UnloadGFX()
{

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
    }

    frmMain.renderTexture(0, 0, GFX.MenuGFX[4]);
    frmMain.renderTexture(632, 576, GFX.Loader);
    frmMain.renderTexture(760, 560, GFX.LoadCoin.w, GFX.LoadCoin.h / 4, GFX.LoadCoin, 0, 32 * LoadCoins);

    frmMain.repaint();
    DoEvents();
}
