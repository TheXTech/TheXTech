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

#include <SDL2/SDL_messagebox.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_mixer_ext.h>

#include <Logger/logger.h>
#include <fmt_format_ne.h>

#include "sound_internal.h"
#include "sound.h"

static Mix_Music *g_curMusic = nullptr;

static const int maxSfxChannels = 91;

bool SI_Init()
{
	int ret;

	const int initFlags = MIX_INIT_MID|MIX_INIT_MOD|MIX_INIT_FLAC|MIX_INIT_OGG|MIX_INIT_OPUS|MIX_INIT_MP3;

	pLogDebug("Opening sound...");
	ret = Mix_Init(initFlags);

	if(ret != initFlags)
	{
	    pLogWarning("MixerX: Some modules aren't properly initialized");
	    if((initFlags & MIX_INIT_MID) != MIX_INIT_MID)
	        pLogWarning("MixerX: Failed to initialize MIDI module");
	    if((initFlags & MIX_INIT_MOD) != MIX_INIT_MOD)
	        pLogWarning("MixerX: Failed to initialize Tracker music module");
	    if((initFlags & MIX_INIT_FLAC) != MIX_INIT_FLAC)
	        pLogWarning("MixerX: Failed to initialize FLAC module");
	    if((initFlags & MIX_INIT_OGG) != MIX_INIT_OGG)
	        pLogWarning("MixerX: Failed to initialize OGG Vorbis module");
	    if((initFlags & MIX_INIT_OPUS) != MIX_INIT_OPUS)
	        pLogWarning("MixerX: Failed to initialize Opus module");
	    if((initFlags & MIX_INIT_MP3) != MIX_INIT_MP3)
	        pLogWarning("MixerX: Failed to initialize MP3 module");
	}

#ifdef VITA
	Mix_ADLMIDI_setEmulator(ADLMIDI_OPL3_EMU_DOSBOX);
    Mix_OPNMIDI_setEmulator(OPNMIDI_OPN2_EMU_MIME);
#endif

	ret = Mix_OpenAudio(g_audioSetup.sampleRate,
	                    g_audioSetup.format,
	                    g_audioSetup.channels,
	                    g_audioSetup.bufferSize);

	if(ret < 0)
	{
	    std::string msg = fmt::format_ne("Can't open audio stream, continuing without audio: ({0})", Mix_GetError());
	    pLogCritical(msg.c_str());
#ifndef VITA // Note: I think this has to remain for now.
	    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Sound opening error", msg.c_str(), nullptr);
#endif
	    return false;
	}

	Mix_VolumeMusic(MIX_MAX_VOLUME);
	Mix_AllocateChannels(maxSfxChannels);



	return true;
}

void SI_Quit()
{
	Mix_CloseAudio();
	Mix_Quit();
}

bool SI_NoPreload(const char* path)
{
	(void)path;
	return false;
}

SI_Chunk* SI_LoadWAV(const char* path)
{
	SI_Chunk* ret = (SI_Chunk*)Mix_LoadWAV(path);
	if(!ret)
	{
		pLogWarning("ERROR: SFX '%s' loading error: %s", path, Mix_GetError());
	}
	return ret;
}

void SI_FreeChunk(SI_Chunk* chunk)
{
	Mix_FreeChunk((Mix_Chunk*)chunk);
}

void SI_PauseAll()
{
	Mix_Pause(-1);
	Mix_PauseMusic();
}

void SI_ResumeAll()
{
	Mix_Resume(-1);
	Mix_ResumeMusic();
}

void SI_KillMusic()
{
	if(g_curMusic)
	{
	    Mix_HaltMusic();
	    Mix_FreeMusic(g_curMusic);
	    g_curMusic = nullptr;
	}
}

void SI_FadeOutMusic(int fadeOutMs)
{
	Mix_FadeOutMusic(fadeOutMs);
}

void SI_PlayMusic(const char* path, int volume, int fadeInMs)
{
	g_curMusic = Mix_LoadMUS(path);
	if(!g_curMusic)
	{
	    pLogWarning("Music '%s' opening error: %s", path, Mix_GetError());
	}
	else
	{
	    Mix_VolumeMusicStream(g_curMusic, volume);
	    if(fadeInMs > 0)
	        Mix_FadeInMusic(g_curMusic, -1, fadeInMs);
	    else
	        Mix_PlayMusic(g_curMusic, -1);
	}
}

void SI_KillSound(int channel, uint32_t playingSoundId)
{
	(void)playingSoundId;
	Mix_HaltChannel(channel);
}

uint32_t SI_PlaySound(SI_Chunk* chunk, const char* backup_path, int loops,
	int channel, uint32_t playingSoundId)
{
	(void)backup_path;
	(void)playingSoundId;
	if(chunk)
		Mix_PlayChannel(channel, (Mix_Chunk*)chunk, loops);
	return 0;
}

void SI_PlayLoadSound(const char* path)
{
	g_curMusic = Mix_LoadMUS(path);
	if(g_curMusic)
		Mix_PlayMusic(g_curMusic, 0);
}

bool SI_PlayingLoadSound()
{
	return (g_curMusic && Mix_PlayingMusicStream(g_curMusic));
}

void SI_ReserveChannels(int n_channels)
{
	Mix_ReserveChannels(n_channels);
}
