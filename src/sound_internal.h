/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2021 Vitaly Novichkov <admin@wohlnet.ru>
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

#ifndef SOUND_INTERNAL_H
#define SOUND_INTERNAL_H

typedef struct SI_Chunk SI_Chunk;

bool SI_Init();
void SI_Quit();

// if file exists but is not WAVE,
// don't load it into memory on certain systems
bool SI_NoPreload(const char* path);
SI_Chunk* SI_LoadWAV(const char* path);
void SI_FreeChunk(SI_Chunk* chunk);

void SI_PauseAll();
void SI_ResumeAll();

void SI_KillMusic();
void SI_FadeOutMusic(int fadeOutMs);
void SI_PlayMusic(const char* path, int volume, int fadeInMs);

void SI_KillSound(int channel, uint32_t playingSoundId);
// returns the new playingSoundId if applicable
uint32_t SI_PlaySound(SI_Chunk* chunk, const char* backup_path, int loops,
	int channel, uint32_t playingSoundId);

void SI_PlayLoadSound(const char* path);
bool SI_PlayingLoadSound();

void SI_ReserveChannels(int n_channels);

#endif // #ifndef SOUND_INTERNAL_H