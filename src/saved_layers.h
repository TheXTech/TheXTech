/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2025 Vitaly Novichkov <admin@wohlnet.ru>
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
#ifndef SAVED_LAYERS_H
#define SAVED_LAYERS_H

#include <array>

#include <PGE_File_Formats/save_filedata.h>

// NEW: saved layers. These are saved / loaded as an *unordered* lists of lower-cased strings corresponding to Visible and Hidden.
const int maxSavedLayers = 32;

struct SavedLayer_t
{
    std::array<char, 22> Name;
    const char null_term = '\0';
    bool Visible = false;
};

// These belong to the episode, NOT the level
extern int numSavedLayers;
extern std::array<SavedLayer_t, maxSavedLayers> SavedLayers;

// Load and save procedures

// totally resets saved layer array
void ClearSavedLayers();
// loads default saved layer values from the episode's savedlayers.ini
bool LoadDefaultSavedLayers();
// loads a single saved layer entry from a gamesave
bool LoadSavedLayer(const void* /*userdata*/, savedLayerSaveEntry& obj);
// exports a single saved layer entry to a gamesave
bool ExportSavedLayer(const void* /*userdata*/, savedLayerSaveEntry& obj, pge_size_t index);

#endif // #ifndef SAVED_LAYERS_H
