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

#ifndef LOAD_GFX_H
#define LOAD_GFX_H

#include <string>

extern bool gfxLoaderTestMode;

// Public Sub LoadGFX()
void LoadGFX();
// Public Sub UnloadGFX()
void UnloadGFX();
// Public Sub LoadCustomGFX()
void LoadCustomGFX();
// Public Sub UnloadCustomGFX()
void UnloadCustomGFX();
// Public Sub LoadCustomGFX2(GFXFilePath As String)
//void LoadCustomGFX2(std::string GFXFilePath);
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
