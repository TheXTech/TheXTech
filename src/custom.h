/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2024 Vitaly Novichkov <admin@wohlnet.ru>
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
#ifndef CUSTOM_H
#define CUSTOM_H

#include <string>


void SavePlayerDefaults();
void LoadPlayerDefaults();

// Public Sub SaveNPCDefaults()
void SaveNPCDefaults();

// Public Sub LoadNPCDefaults()
void LoadNPCDefaults();


void FindCustomPlayers(const char* preview_players_from = nullptr);
// Public Sub FindCustomNPCs(Optional cFilePath As String = "")
//void FindCustomNPCs(std::string cFilePath = "");
void FindCustomNPCs();

// Private Sub LoadCustomNPC(A As Integer, cFileName As String)

// public-facing functions used by the main menu to load and unload previews of the custom players
void LoadCustomPlayerPreviews(const char* preview_players_from);
void UnloadCustomPlayerPreviews();

#endif // CUSTOM_H
