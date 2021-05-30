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

#include <cstring>

#include "../sound_internal.h"
#include "3ds-audio-lib.h"

static uint32_t g_curMusic = -1;

bool SI_Init()
{
    return audioInit();
}

void SI_Quit()
{
    audioExit();
}

bool SI_NoPreload(const char* path)
{
    // if path is not WAV, for now
    const char* ext = get_filename_ext(path);
    if (strcasecmp(ext, "wav"))
        return true;
    else
        return false;
}

SI_Chunk* SI_LoadWAV(const char* path)
{
    return (SI_Chunk*)audioLoadWave(path);
}

void SI_FreeChunk(SI_Chunk* chunk)
{
    audioFreeWave((WaveObject*)chunk);
}

void SI_PauseAll()
{
    audioPause();
}

void SI_ResumeAll()
{
    audioResume();
}

void SI_KillMusic()
{
    if(g_curMusic != INVALID_ID)
    {
        killSound(g_curMusic);
        g_curMusic = INVALID_ID;
    }
}

void SI_FadeOutMusic(int fadeOutMs)
{
    SI_KillMusic();
}

void SI_PlayMusic(const char* path, int volume, int fadeInMs)
{
    g_curMusic = playSoundAuto(path, -1);
}

void SI_KillSound(int channel, uint32_t playingSoundId)
{
    killSound(playingSoundId);
}

uint32_t SI_PlaySound(SI_Chunk* chunk, const char* backup_path, int loops,
    int channel, uint32_t playingSoundId)
{
    killSound(playingSoundId);

    if(chunk)
    {
        uint32_t ret = playSoundMem((WaveObject*)chunk, loops);
        if (ret != INVALID_ID)
            return ret;
    }
    return playSoundAuto(backup_path, loops);
}

void SI_PlayLoadSound(const char* path)
{
    playSoundAuto(path, 0);
}

bool SI_PlayingLoadSound()
{
    return false;
}

void SI_ReserveChannels(int n_channels) {}