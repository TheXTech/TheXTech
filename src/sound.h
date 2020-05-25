/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2020 Vitaly Novichkov <admin@wohlnet.ru>
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

#ifndef SOUND_H
#define SOUND_H

#include <string>

// Public musicPlaying As Boolean
extern bool musicPlaying;
// Public musicLoop As Integer
extern int musicLoop;
// Public musicName As String
extern std::string musicName;

int CustomWorldMusicId();

// Public Sub InitMixerX()
void InitMixerX();
// Public Sub QuitMixerX()
void QuitMixerX();
// Public Sub SetMusicVolume(Alias As String, Volume As Long)
void SetMusicVolume(std::string Alias, long Volume);
// Public Sub SoundPauseAll()
void SoundPauseAll();
// Public Sub SoundResumeAll()
void SoundResumeAll();
// Public Sub PlayMusic(Alias As String)
void PlayMusic(std::string Alias, int fadeInMs = 0);
// Public Sub PlaySfx(Alias As String)
void PlaySfx(std::string Alias, int loops = 0);
// Public Sub StopSfx(Alias As String)
void StopSfx(std::string Alias);
// Public Sub StartMusic(A As Integer) 'play music
// play music
void StartMusic(int A, int fadeInMs = 0);
// Public Sub StopMusic() 'stop playing music
// stop playing music
void StopMusic();
// Public Sub PlayInitSound()
void PlayInitSound();
// Public Sub InitSound() 'readys sound and music to be played
// readys sound and music to be played
void InitSound();
// Public Sub PlaySound(A As Integer) 'play a sound
// play a sound
void PlaySound(int A, int loops = 0);
// Public Sub BlockSound() 'stops all sound from being played for 10 cycles
// stops all sound from being played for 10 cycles
void BlockSound();
// Public Sub UpdateSound() 'checks to loop music and update the soundpause variable
// checks to loop music and update the soundpause variable
void UpdateSound();
// EXTRA: Fade out music
void FadeOutMusic(int ms);
// EXTRA: load custom sounds.ini and music.ini from episode and custom folder!
void LoadCustomSound();
// EXTRA: Unload custom-loaded music and sounds, and restore originals
void UnloadCustomSound();

#endif // SOUND_H
