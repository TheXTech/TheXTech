/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2022 Vitaly Novichkov <admin@wohlnet.ru>
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
#ifndef SOUND_H
#define SOUND_H

#include <string>

#include "global_constants.h"

// Public musicPlaying As Boolean
extern bool musicPlaying;
// Public musicLoop As Integer
extern int musicLoop;
// Public musicName As String
extern std::string musicName;

extern int playerHammerSFX;

extern struct AudioSetup_t
{
    bool disableSound = false;
    int sampleRate = 44100;
    int channels = 2;
    int bufferSize = 512;
    uint16_t format = 0x8120;
} g_audioSetup;


enum
{
    SFX_Jump            = 1,
    SFX_Stomp,
    SFX_BlockHit,
    SFX_BlockSmashed,
    SFX_PlayerShrink,
    SFX_PlayerGrow,
    SFX_Mushroom,
    SFX_PlayerDied,
    SFX_ShellHit,
    SFX_Skid            = 10,
    SFX_DropItem,
    SFX_GotItem,
    SFX_Camera,
    SFX_Coin,
    SFX_1up,
    SFX_Lava,
    SFX_Warp,
    SFX_Fireball,
    SFX_CardRouletteClear,
    SFX_BossBeat        = 20,
    SFX_DungeonClear,
    SFX_Bullet,
    SFX_Grab,
    SFX_Spring,
    SFX_HammerToss,
    SFX_Slide,
    SFX_NewPath,
    SFX_LevelSelect,
    SFX_Do,
    SFX_Pause           = 30,
    SFX_Key,
    SFX_PSwitch,
    SFX_Tail,
    SFX_Raccoon,
    SFX_Boot,
    SFX_Smash,
    SFX_Twomp,
    SFX_BirdoSpit,
    SFX_BirdoHit,
    SFX_CrystalBallExit = 40,
    SFX_BirdoBeat,
    SFX_BigFireball,
    SFX_Fireworks,
    SFX_BowserKilled,
    SFX_GameBeat,
    SFX_Door,
    SFX_Message,
    SFX_Yoshi,
    SFX_YoshiHurt,
    SFX_YoshiTongue     = 50,
    SFX_YoshiEgg,
    SFX_GotStar,
    SFX_ZeldaKill,
    SFX_PlayerDied2,
    SFX_YoshiSwallow,
    SFX_SonicRing,
    SFX_DryBones,
    SFX_Checkpoint,
    SFX_DraginCoin,
    SFX_TapeExit        = 60,
    SFX_Blaarg,
    SFX_WartBubbles,
    SFX_WartKilled,
    SFX_SMBlockHit,
    SFX_SMKilled,
    SFX_SMHurt,
    SFX_SMGlass,
    SFX_SMBossHit,
    SFX_SMCry,
    SFX_SMExplosion     = 70,
    SFX_Climbing,
    SFX_Swim,
    SFX_Grab2,
    SFX_Saw,
    SFX_Throw,
    SFX_PlayerHit,
    SFX_ZeldaStab,
    SFX_ZeldaHurt,
    SFX_ZeldaHeart,
    SFX_ZeldaDied       = 80,
    SFX_ZeldaRupee,
    SFX_ZeldaFire,
    SFX_ZeldaItem,
    SFX_ZeldaKey,
    SFX_ZeldaShield,
    SFX_ZeldaDash,
    SFX_ZeldaFairy,
    SFX_ZeldaGrass,
    SFX_ZeldaHit,
    SFX_ZeldaSwordBeam  = 90,
    SFX_Bubble,

    // non-SMBX64, Extra
    SFX_PSwitchTimeout,
    SFX_SwooperFlap,
    SFX_Iceball,
    SFX_Freeze,
    SFX_Icebreak,
    SFX_PlayerHammer, // Reserved
    SFX_SproutVine
};

int CustomWorldMusicId();

// Public Sub InitMixerX()
void InitMixerX();
// Public Sub QuitMixerX()
void QuitMixerX();
// Public Sub SetMusicVolume(Alias As String, Volume As Long)
void SetMusicVolume(const std::string &Alias, long Volume);
// Public Sub SoundPauseAll()
void SoundPauseAll();
// Public Sub SoundResumeAll()
void SoundResumeAll();
void SoundPauseEngine(int paused);
// Public Sub PlayMusic(Alias As String)
void PlayMusic(const std::string &Alias, int fadeInMs = 0);
// Public Sub PlaySfx(Alias As String)
void PlaySfx(const std::string &Alias, int loops = 0);
// Public Sub StopSfx(Alias As String)
void StopSfx(const std::string &Alias);
// Public Sub StartMusic(A As Integer) 'play music
void setMusicStartDelay();
void delayedMusicStart();
void delayedMusicReset();
bool delayMusicIsSet();
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
void PlaySoundMenu(int A, int loops = 0);
// Public Sub BlockSound() 'stops all sound from being played for 10 cycles
// stops all sound from being played for 10 cycles
void BlockSound();
// Public Sub UpdateSound() 'checks to loop music and update the soundpause variable
// checks to loop music and update the soundpause variable
void UpdateSound();

void UpdateYoshiMusic();
// EXTRA: Fade out music
void FadeOutMusic(int ms);
// EXTRA: load custom sounds.ini and music.ini from episode and custom folder!
void LoadCustomSound();
// EXTRA: Unload custom-loaded music and sounds, and restore originals
void UnloadCustomSound();

void PlayExtSound(const std::string &path);

#ifdef THEXTECH_ENABLE_AUDIO_FX
struct SoundFXEchoSetup
{
    int echoOn = 0;
    int echoDelay = 0;
    int echoFeedBack = 0;

    int echoMainVolL = 0;
    int echoMainVolR = 0;

    int echoVolL = 0;
    int echoVolR = 0;

    int echoFir[8] = {0, 0, 0, 0, 0, 0, 0, 0};
};
void SoundFX_SetEcho(const SoundFXEchoSetup &setup);

struct SoundFXReverb
{
    float mode         = 0.0f; // Normal (0) or Freeze (>0.5)
    float roomSize     = 0.7f;
    float damping      = 0.5f; // 0.0...1.0
    float wetLevel     = 0.2f;
    float dryLevel     = 0.4f;
    float width        = 1.0f; // 0.0...1.0
};
void SoundFX_SetReverb(const SoundFXReverb &setup);

void SoundFX_Clear();
#endif // THEXTECH_ENABLE_AUDIO_FX

void ResetSoundFX();

void UpdateSoundFX(int recentSection);

#endif // SOUND_H
