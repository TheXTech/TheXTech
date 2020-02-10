#ifndef LOAD_GFX_H
#define LOAD_GFX_H

#include <string>

// Public Sub LoadGFX()
void LoadGFX();
// Public Sub UnloadGFX()
void UnloadGFX();
// Public Sub LoadCustomGFX()
void LoadCustomGFX();
// Public Sub UnloadCustomGFX()
void UnloadCustomGFX();
// Public Sub LoadCustomGFX2(GFXFilePath As String)
void LoadCustomGFX2(std::string GFXFilePath);
// Public Sub LoadWorldCustomGFX()
void LoadWorldCustomGFX();
// Public Sub UnloadWorldCustomGFX()
void UnloadWorldCustomGFX();

// Private Sub cBlockGFX(A As Integer)
// Private Sub cNPCGFX(A As Integer)
// Private Sub cBackgroundGFX(A As Integer)
// Private Sub cTileGFX(A As Integer)
// Private Sub cSceneGFX(A As Integer)
// Private Sub cLevelGFX(A As Integer)
// Private Sub cPathGFX(A As Integer)

// Public Sub UpdateLoad()
void UpdateLoad();


#endif // LOAD_GFX_H
