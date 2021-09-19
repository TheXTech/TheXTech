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

struct Mix_Music;
struct Mix_Chunk;

bool MixPlatform_Init();
void MixPlatform_Quit();

bool MixPlatform_NoPreload(const char* path);
void MixPlatform_PlayStream(int channel, const char* path, int loops);

// need to do this because LoadWAV is not properly declared in MixerX.
Mix_Chunk* MixPlatform_LoadWAV(const char* path);
const char* MixPlatform_GetError();
void MixPlatform_PlayChannel(int channel, Mix_Chunk* chunk, int loops);

extern "C" {

void Mix_ReserveChannels(int channels);
void Mix_PauseAudio(int pause);


Mix_Music* Mix_LoadMUS(const char* path);

int Mix_VolumeMusicStream(Mix_Music* music, int volume);
void Mix_HaltMusicStream(Mix_Music* music);
void Mix_FadeOutMusicStream(Mix_Music* music, int ms);
int Mix_HaltChannel(int channel);

int Mix_PlayingMusicStream(Mix_Music* music);

void Mix_PlayMusic(Mix_Music* music, int loops);
void Mix_FadeInMusic(Mix_Music* music, int loops, int fadeInMs);

int Mix_GetMusicTracks(Mix_Music* music);

void Mix_SetMusicTrackMute(Mix_Music* music, int track, int mute);

void Mix_FreeMusic(Mix_Music* music);
void Mix_FreeChunk(Mix_Chunk* chunk);


} // extern "C"
