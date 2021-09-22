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
#define Mix_LoadWAV(file)   MixPlatform_LoadWAV(file);
Mix_Chunk* MixPlatform_LoadWAV(const char* path);

#define Mix_GetError MixPlatform_GetError
const char* MixPlatform_GetError();

#define Mix_PlayChannel(channel,chunk,loops) MixPlatform_PlayChannelTimed(channel,chunk,loops,-1)
int MixPlatform_PlayChannelTimed(int channel, Mix_Chunk *chunk, int loops, int ticks);

#define Mix_PlayChannelVol(channel,chunk,loops,vol) MixPlatform_PlayChannelTimedVolume(channel,chunk,loops,-1,vol)
int MixPlatform_PlayChannelTimedVolume(int which, Mix_Chunk *chunk, int loops, int ticks, int volume);

extern "C" {

extern int Mix_ReserveChannels(int channels);
extern void Mix_PauseAudio(int pause);


extern Mix_Music* Mix_LoadMUS(const char* path);

extern int Mix_VolumeMusicStream(Mix_Music* music, int volume);
extern int Mix_HaltMusicStream(Mix_Music* music);
extern int Mix_FadeOutMusicStream(Mix_Music* music, int ms);
extern int Mix_HaltChannel(int channel);

extern int Mix_PlayingMusicStream(Mix_Music* music);

extern int Mix_PlayMusic(Mix_Music *music, int loops);
extern int Mix_FadeInMusic(Mix_Music* music, int loops, int fadeInMs);
extern int Mix_SetFreeOnStop(Mix_Music *music, int free_on_stop);

extern int Mix_PlayMusicStream(Mix_Music *music, int loops);

extern int Mix_GetMusicTracks(Mix_Music* music);

extern int Mix_SetMusicTrackMute(Mix_Music* music, int track, int mute);

extern void Mix_FreeMusic(Mix_Music* music);
extern void Mix_FreeChunk(Mix_Chunk* chunk);

} // extern "C"
