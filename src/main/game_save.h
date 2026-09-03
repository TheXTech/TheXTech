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

#pragma once
#ifndef GAME_SAVE_H
#define GAME_SAVE_H

#include <string>

void FindSaves();

extern std::string makeGameSavePath(std::string episode, std::string saveFile);

void SaveGame();

#ifdef THEXTECH_ENABLE_SDL_NET
void PreloadGame();
#endif
void LoadGame();
//! Removes gamesave file and restores initial state of all level objects
void ClearGame(bool punnish = false);
void DeleteSave(int world, int save);
void CopySave(int world, int src, int dst);

#endif // GAME_SAVE_H
