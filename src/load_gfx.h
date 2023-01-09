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

#pragma once
#ifndef LOAD_GFX_H
#define LOAD_GFX_H

#include <string>

extern bool gfxLoaderTestMode;
extern bool gfxLoaderThreadingMode;

// Public Sub LoadGFX()
void LoadGFX();
// Public Sub UnloadGFX()
void UnloadGFX();
// Public Sub LoadCustomGFX()
void LoadCustomGFX(bool include_world = false);
// Public Sub UnloadCustomGFX()
void UnloadCustomGFX();
// Public Sub LoadCustomGFX2(GFXFilePath As String)
//void LoadCustomGFX2(std::string GFXFilePath);

// now a subfunction of LoadCustomGFX
// Public Sub LoadWorldCustomGFX()
// void LoadWorldCustomGFX();
// Public Sub UnloadWorldCustomGFX()
void UnloadWorldCustomGFX();

// Private Sub cBlockGFX(A As Integer)
// Private Sub cNPCGFX(A As Integer)
// Private Sub cBackgroundGFX(A As Integer)
// Private Sub cTileGFX(A As Integer)
// Private Sub cSceneGFX(A As Integer)
// Private Sub cLevelGFX(A As Integer)
// Private Sub cPathGFX(A As Integer)

void LoaderInit();
void LoaderFinish();
void LoaderUpdateDebugString(const std::string &strig);

// Public Sub UpdateLoad()
void UpdateLoadREAL();
void UpdateLoad();


#endif // LOAD_GFX_H
