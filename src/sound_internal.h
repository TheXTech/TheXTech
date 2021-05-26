/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2021 Vitaly Novichkov <admin@wohlnet.ru>
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