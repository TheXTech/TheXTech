/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2026 Vitaly Novichkov <admin@wohlnet.ru>
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

#include <IniProcessor/ini_processing.h>
#include "Utils/files_ini.h"

#include "sdl_proxy/sdl_stdinc.h"

#include "saved_layers.h"
#include "global_dirs.h"

int numSavedLayers;
std::array<SavedLayer_t, maxSavedLayers> SavedLayers;

// Load and save procedures

// totally resets saved layer array
void ClearSavedLayers()
{
    numSavedLayers = 0;
    SDL_memset(&SavedLayers, 0, sizeof(decltype(SavedLayers)));
}

// loads default saved layer values from the episode's savedlayers.ini
bool LoadDefaultSavedLayers()
{
    ClearSavedLayers();

    // disable saved layers on v1.7.x release series
    return true;

    std::string savedlayers_ini_path = g_dirEpisode.resolveFileCaseExistsAbs("savedlayers.ini");
    if(savedlayers_ini_path.empty())
        return true;

    IniProcessing savedlayers_ini = Files::load_ini(savedlayers_ini_path);
    savedlayers_ini.beginGroup("General");

    for(const std::string& k : savedlayers_ini.allKeys())
    {
        if(numSavedLayers >= maxSavedLayers || k.size() > sizeof(SavedLayer_t::Name))
            return false;

        auto& target = SavedLayers[numSavedLayers++];

        memcpy(target.Name.data(), k.c_str(), k.size());

        savedlayers_ini.read(k.c_str(), target.Visible, false);
    }

    return true;
}

// loads a single saved layer entry from a gamesave
bool LoadSavedLayer(const void* /*userdata*/, savedLayerSaveEntry& obj)
{
    for(int i = 0; i < numSavedLayers; i++)
    {
        // SavedLayer_t::Name is a buffer but not necessarily a valid C string
        if(obj.first.size() > sizeof(SavedLayer_t::Name) || SDL_strncmp(obj.first.c_str(), SavedLayers[i].Name.data(), sizeof(SavedLayer_t::Name)) != 0)
            continue;

        SavedLayers[i].Visible = obj.second;
        break;
    }

    return true;
}

// exports a single saved layer entry to a gamesave
bool ExportSavedLayer(const void* /*userdata*/, savedLayerSaveEntry& obj, pge_size_t index)
{
    if((int)index >= numSavedLayers)
        return false;

    // SavedLayer_t::Name is a buffer but not necessarily a valid C string
    // find correct length of string, with the possibility that the size is the full length (15)
    size_t len = 0;
    for(; len < sizeof(SavedLayer_t::Name); len++)
    {
        if(SavedLayers[index].Name[len] == '\0')
            break;
    }

    obj.first.assign(SavedLayers[index].Name.data(), len);
    obj.second = SavedLayers[index].Visible;

    return true;
}
