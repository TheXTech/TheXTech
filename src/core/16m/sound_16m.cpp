/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2023 Vitaly Novichkov <admin@wohlnet.ru>
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

#include <maxmod9.h>

#include <vector>

#include "sdl_proxy/sdl_stdinc.h"

#include "globals.h"
#include "global_dirs.h"
#include "frame_timer.h"

#include "load_gfx.h"
#include "core/msgbox.h"

#include "sound.h"

#include <Logger/logger.h>
#include <IniProcessor/ini_processing.h>
#include <Utils/files.h>
#include <Utils/strings.h>
#include <unordered_map>
#include <fmt_format_ne.h>

#include "pseudo_vb.h"

// Public musicPlaying As Boolean
bool musicPlaying = false;
// Public musicLoop As Integer
int musicLoop = 0;
// Public musicName As String
std::string musicName;

int playerHammerSFX = SFX_Fireball;

const AudioDefaults_t g_audioDefaults{0, 0, 0, 0};
AudioSetup_t g_audioSetup;

static bool g_mixerLoaded = false;

static int g_customLvlMusicId = 24;
static int g_customWldMusicId = 17;

//! Total count of loaded default sounds
static unsigned int g_totalSounds = 0;

//! Total count of level music
static unsigned int g_totalMusicLevel = 0;
//! Total count of world map music
static unsigned int g_totalMusicWorld = 0;
//! Total count of special music
static unsigned int g_totalMusicSpecial = 0;

//! Enable using the unique iceball SFX when available
static bool s_useIceBallSfx = false;
//! Enable using of the new ice SFX: NPC freeze and breaking of the frozen NPC
static bool s_useNewIceSfx = false;

static int s_curMusic = -1;
// static int s_curJingle = -1;
static std::vector<bool> s_soundLoaded;
static std::vector<int> s_worldMusicMods;
static std::vector<int> s_levelMusicMods;
static std::vector<int> s_specialMusicMods;
static std::vector<int> s_sfxEffects;

int CustomWorldMusicId()
{
    return g_customWldMusicId;
}

void InitMixerX()
{

    std::string SoundbankPath = AppPath + "soundbank.bin";

    if(g_mixerLoaded)
        return;

    mmInitDefault(const_cast<char*>(SoundbankPath.c_str()));
    g_mixerLoaded = true;
}

void QuitMixerX()
{
    if(!g_mixerLoaded)
        return;

    noSound = true;

    mmStop();

    if(s_curMusic != -1)
        mmUnload(s_curMusic);
    s_curMusic = -1;

    for(unsigned int i = 0; i < g_totalSounds; i++)
    {
        if(s_soundLoaded[i])
            mmUnloadEffect(i);
        s_soundLoaded[i] = false;
    }

    g_mixerLoaded = false;
}

void InitSoundDefaults()
{
}

void SoundPauseAll()
{
    if(noSound)
        return;

    mmPause();
    mmEffectCancelAll();
}

void SoundResumeAll()
{
    if(noSound)
        return;

    mmResume();
}

void PauseMusic()
{
    if(noSound)
        return;

    mmPause();
}

void ResumeMusic()
{
    if(noSound)
        return;

    mmResume();
}

void SoundPauseEngine(int paused)
{
    if(paused)
        SoundPauseAll();
    else
        SoundResumeAll();
}


static bool s_delayMusic = false;
static bool s_delayedMusicRequested = false;
static int  s_delayedMusicA = 0;
static int  s_delayedMusicFadeInMs = 0;

static int  s_fadeTarget = 0;
static int  s_fadeCurrent = 0;
static int  s_fadeRate = 0;

static void s_MusicSetupFade(int target, int ms)
{
    if(ms == 0)
    {
        s_fadeTarget = s_fadeCurrent = target;
        s_fadeRate = 0;
    }
    else
    {
        s_fadeTarget = target;
        s_fadeRate = (s_fadeTarget - s_fadeCurrent) * 15 / ms;
        s_fadeCurrent += s_fadeRate;
    }

    if(g_mixerLoaded)
        mmSetModuleVolume(s_fadeCurrent);
}

static void s_MusicUpdateFade()
{
    if(s_fadeTarget == s_fadeCurrent || s_fadeRate == 0)
        return;

    if((s_fadeRate > 0 && s_fadeCurrent + s_fadeRate > s_fadeTarget)
        || (s_fadeRate < 0 && s_fadeCurrent + s_fadeRate < s_fadeTarget))
    {
        s_fadeCurrent = s_fadeTarget;
        s_fadeRate = 0;
    }
    else
    {
        s_fadeCurrent += s_fadeRate;
    }

    if(g_mixerLoaded)
        mmSetModuleVolume(s_fadeCurrent);

    if(s_fadeCurrent == 0 && s_fadeTarget == 0)
        StopMusic();
}

static void s_playMusic(int mod, int fadeInMs)
{
    StopMusic();
    if(mod == -1)
        return;

    mmLoad(mod);
    s_curMusic = mod;
    s_MusicSetupFade(1024, fadeInMs);
    mmStart(mod, MM_PLAY_LOOP);
}

void setMusicStartDelay()
{
    s_delayedMusicRequested = false;
    s_delayMusic = true;
}

bool delayMusicIsSet()
{
    return s_delayMusic;
}

void delayedMusicStart()
{
    s_delayMusic = false;
    if(s_delayedMusicRequested)
    {
        D_pLogDebug("Restored delayed music request A=%d", s_delayedMusicA);
        StartMusic(s_delayedMusicA, s_delayedMusicFadeInMs);
        s_delayedMusicRequested = false;
    }
}

void delayedMusicReset()
{
    s_delayMusic = false;
    if(s_delayedMusicRequested)
        D_pLogDebug("Saved music request erase A=%d", s_delayedMusicA);
    s_delayedMusicRequested = false;
}

// only used for credits.
void PlayMusic(const std::string &Alias, int fadeInMs)
{
    if(noSound)
        return;

    StopMusic();

    if(Alias == "tmusic")
    {
        pLogDebug("Starting special music [3]");
        int index = -1;
        if(2 < s_specialMusicMods.size())
            index = s_specialMusicMods[2];
        s_playMusic(index, fadeInMs);
        musicName = "tmusic";
    }
}

void StartMusic(int A, int fadeInMs)
{
    if(s_delayMusic)
    {
        s_delayedMusicA = A;
        s_delayedMusicFadeInMs = fadeInMs;
        s_delayedMusicRequested = true;
        D_pLogDebug("Saved start music request A=%d", A);
        return;
    }

    D_pLogDebug("Start music A=%d", A);

    if(noSound)
    {
        // Keep world map music being remembered when sound disabled
        if((LevelSelect || WorldEditor) && !GameMenu && !GameOutro)
            curWorldMusic = A;
        return;
    }

    if((LevelSelect || WorldEditor) && !GameMenu && !GameOutro) // music on the world map
    {
        StopMusic();
        curWorldMusic = A;
        musicName = fmt::format_ne("wmusic{0}", A);
        if(curWorldMusic == g_customWldMusicId)
        {
        }
        else
        {
            int index = -1;
            if(A >= 1 && A - 1 < (int)s_worldMusicMods.size())
                index = s_worldMusicMods[A - 1];
            s_playMusic(index, fadeInMs);
        }
    }
    else if(A == -1) // P switch music
    {
        StopMusic();
        if(FreezeNPCs) {
            pLogDebug("Starting special music [2]");
            int index = -1;
            if(1 < s_specialMusicMods.size())
                index = s_specialMusicMods[1];
            s_playMusic(index, fadeInMs);
            musicName = "stmusic";
        } else {
            pLogDebug("Starting special music [1]");
            int index = -1;
            if(0 < s_specialMusicMods.size())
                index = s_specialMusicMods[0];
            s_playMusic(index, fadeInMs);
            musicName = "smusic";
        }
        curMusic = -1;
    }
    else if(PSwitchTime == 0 && PSwitchStop == 0) // level music
    {
        StopMusic();
        curMusic = bgMusic[A];
        std::string mus = fmt::format_ne("music{0}", curMusic);

        int index = -1;
        if(curMusic >= 1 && curMusic - 1 < (int)s_levelMusicMods.size())
            index = s_levelMusicMods[curMusic - 1];
        s_playMusic(index, fadeInMs);

        musicName = std::move(mus);
    }

    musicPlaying = true;
}

void StopMusic()
{
    if(!musicPlaying || noSound)
        return;

    pLogDebug("Stopping music");

    if(s_curMusic != -1)
    {
        mmStop();
        mmUnload(s_curMusic);
        s_curMusic = -1;
    }
    musicPlaying = false;
    g_stats.currentMusic.clear();
}

void FadeOutMusic(int ms)
{
    s_MusicSetupFade(0, ms);
}


void PlayInitSound()
{
    if(!g_mixerLoaded)
        return;

    // std::string doSound = AppPath + "sound/";
    IniProcessing sounds(AppPath + "sounds.ini");
    unsigned int totalSounds;
    sounds.beginGroup("sound-main");
    sounds.read("total", totalSounds, 0);
    sounds.endGroup();

    s_soundLoaded.resize(totalSounds);

    if(totalSounds >= SFX_Do)
    {
        sounds.beginGroup("sound-29");
        int effect;
        sounds.read("resolved-sfx", effect, -1);
        sounds.endGroup();

        if(effect != -1)
        {
            mmLoadEffect(effect);
            s_soundLoaded[SFX_Do] = true;
            mmEffect(effect);
        }
    }
}

static void loadMusicIni(const std::string &path, bool isLoadingCustom)
{
    IniProcessing musicSetup(path);
    if(!isLoadingCustom)
    {
        g_totalMusicLevel = 0;
        g_totalMusicWorld = 0;
        g_totalMusicSpecial = 0;
        musicSetup.beginGroup("music-main");
        musicSetup.read("total-level", g_totalMusicLevel, 0);
        musicSetup.read("total-world", g_totalMusicWorld, 0);
        musicSetup.read("total-special", g_totalMusicSpecial, 0);
        musicSetup.read("level-custom-music-id", g_customLvlMusicId, 0);
        musicSetup.read("world-custom-music-id", g_customWldMusicId, 0);
        musicSetup.endGroup();

        UpdateLoad();
    }

    int mod;
    s_levelMusicMods.clear();
    s_levelMusicMods.reserve(g_totalMusicLevel);
    s_worldMusicMods.clear();
    s_worldMusicMods.reserve(g_totalMusicWorld);
    s_specialMusicMods.clear();
    s_specialMusicMods.reserve(g_totalMusicSpecial);

    for(unsigned int i = 1; i <= g_totalMusicLevel; ++i)
    {
        std::string group = fmt::format_ne("level-music-{0}", i);
        musicSetup.beginGroup(group);
        musicSetup.read("resolved-mod", mod, -1);
        musicSetup.endGroup();

        s_levelMusicMods.push_back(mod);
    }

    if(!isLoadingCustom)
        UpdateLoad();
    for(unsigned int i = 1; i <= g_totalMusicWorld; ++i)
    {
        std::string group = fmt::format_ne("world-music-{0}", i);
        musicSetup.beginGroup(group);
        musicSetup.read("resolved-mod", mod, -1);
        musicSetup.endGroup();

        s_worldMusicMods.push_back(mod);
    }

    if(!isLoadingCustom)
        UpdateLoad();
    for(unsigned int i = 1; i <= g_totalMusicSpecial; ++i)
    {
        std::string group = fmt::format_ne("special-music-{0}", i);
        musicSetup.beginGroup(group);
        musicSetup.read("resolved-mod", mod, -1);
        musicSetup.endGroup();

        s_specialMusicMods.push_back(mod);
    }
}

void InitSound()
{
    if(noSound)
        return;

    std::string musicIni = AppPath + "music.ini";
    std::string sfxIni = AppPath + "sounds.ini";

    UpdateLoad();
    if(!Files::fileExists(musicIni) && !Files::fileExists(sfxIni))
    {
        pLogWarning("music.ini and sounds.ini are missing");
        XMsgBox::simpleMsgBox(XMsgBox::MESSAGEBOX_ERROR,
                     "music.ini and sounds.ini are missing",
                     "Files music.ini and sounds.ini are not exist, game will work without default music and SFX.");
        g_customLvlMusicId = 24;
        g_customWldMusicId = 17;
        return;
    }
    else if(!Files::fileExists(musicIni))
    {
        pLogWarning("music.ini is missing");
        XMsgBox::simpleMsgBox(XMsgBox::MESSAGEBOX_ERROR,
                     "music.ini is missing",
                     "File music.ini is not exist, game will work without default music.");
    }
    else if(!Files::fileExists(sfxIni))
    {
        pLogWarning("sounds.ini is missing");
        XMsgBox::simpleMsgBox(XMsgBox::MESSAGEBOX_ERROR,
                     "sounds.ini is missing",
                     "File sounds.ini is not exist, game will work without SFX.");
    }

    loadMusicIni(musicIni, false);

    UpdateLoad();
    IniProcessing sounds(sfxIni);
    sounds.beginGroup("sound-main");
    sounds.read("total", g_totalSounds, 0);
    sounds.read("use-iceball-sfx", s_useIceBallSfx, false);
    sounds.read("use-new-ice-sfx", s_useNewIceSfx, false);
    bool playerUseNPCHammer;
    bool playerUseOwnHammer;
    sounds.read("player-use-npc-hammer-sfx", playerUseNPCHammer, false);
    sounds.read("player-use-own-hammer-sfx", playerUseOwnHammer, false);
    sounds.endGroup();

    int real_effects = 0;
    s_sfxEffects.reserve(g_totalSounds);
    s_soundLoaded.resize(g_totalSounds);
    int effect;
    UpdateLoad();
    for(unsigned int i = 1; i <= g_totalSounds; ++i)
    {
        std::string group = fmt::format_ne("sound-{0}", i);
        sounds.beginGroup(group);
        sounds.read("resolved-sfx", effect, -1);
        sounds.endGroup();

        s_sfxEffects.push_back(effect);

        if(effect != -1)
            real_effects++;
        // maybe load the effects, maybe not!
    }
    UpdateLoad();

    if(playerUseOwnHammer)
        playerHammerSFX = SFX_PlayerHeavy;
    else if(playerUseNPCHammer)
        playerHammerSFX = SFX_Throw;
    else
        playerHammerSFX = SFX_Fireball;

    UpdateLoad();
}

static const std::unordered_map<int, int> s_soundDelays =
{
    {2, 12}, {3, 12},  {4, 12},  {5, 30}, {8, 10},  {9, 4},
    {10, 8}, {12, 10}, {17, 10}, {26, 8}, {31, 20}, {37, 10},
    {42, 16},{50, 8},  {54, 8},  {71, 9}, {74, 8},  {81, 5},
    {86, 8}, {SFX_Icebreak, 4}
};

static void s_resetSoundDelay(int A)
{
    // set the delay before a sound can be played again
    auto i = s_soundDelays.find(A);
    if(i == s_soundDelays.end())
        SoundPause[A] = 4;
    else
        SoundPause[A] = i->second;
}

static const std::unordered_map<int, int> s_soundFallback =
{
    {SFX_Iceball, SFX_Fireball},
    {SFX_Freeze, SFX_ShellHit},
    {SFX_Icebreak, SFX_ShellHit},
    {SFX_SproutVine, SFX_ItemEmerge},
    {SFX_FireBossKilled, SFX_SickBossKilled},
};

static int getFallbackSfx(int A)
{
    auto fb = s_soundFallback.find(A);
    if(fb != s_soundFallback.end())
        A = fb->second;
    return A;
}

void PlaySound(int A, int loops, int volume)
{
    if(noSound)
        return;

    if(GameMenu || GameOutro) // || A == 26 || A == 27 || A == 29)
        return;

    if(A > (int)g_totalSounds) // Play fallback sound for the missing SFX
        A = getFallbackSfx(A);
    else if(!s_useIceBallSfx && A == SFX_Iceball)
        A = SFX_Fireball; // Fell back into fireball when iceball sound isn't preferred
    else if(!s_useNewIceSfx && (A == SFX_Freeze || A == SFX_Icebreak))
        A = SFX_ShellHit; // Restore the old behavior

    if(g_ClonedPlayerMode)
        SoundPause[10] = 1;

    UNUSED(volume);
    PlaySoundMenu(A, loops);
}

bool HasSound(int A)
{
    return A <= (int)g_totalSounds;
}

void PlaySoundMenu(int A, int loops)
{
    if(noSound)
        return;

    UNUSED(loops);

    if(SoundPause[A] == 0) // if the sound wasn't just played
    {
        s_resetSoundDelay(A);

        int index = -1;
        if(A >= 1 && A - 1 < (int)s_sfxEffects.size())
            index = s_sfxEffects[A - 1];

        if(index == -1)
            return;

        if(!s_soundLoaded[A])
            mmLoadEffect(index);
        s_soundLoaded[A] = true;
        mm_sfxhand handle = mmEffect(index);
        UNUSED(handle);
        UNUSED(loops);
        // mmEffectVolume(hand, volume);
    }
}

// stops all sound from being played for 10 cycles
void BlockSound()
{
    For(A, 1, numSounds)
    {
        SoundPause[A] = 10;
    }
}

void UpdateSound()
{
    if(noSound)
        return;
    For(A, 1, numSounds)
    {
        if(SoundPause[A] > 0)
            SoundPause[A] -= 1;
    }

    s_MusicUpdateFade();
}

void LoadCustomSound()
{
    if(noSound)
        return;

    if(GameMenu || GameOutro)
        return; // Don't load custom music in menu mode
}

void UnloadCustomSound()
{
    if(noSound)
        return;

    UnloadExtSounds();
}

void UpdateYoshiMusic()
{
    return;
}

void PreloadExtSound(const std::string& path)
{
    UNUSED(path);
    if(noSound)
        return;
}

void UnloadExtSounds()
{
    if(noSound)
        return;
}

void PlayExtSound(const std::string &path, int loops, int volume)
{
    UNUSED(path);
    UNUSED(loops);
    UNUSED(volume);
    // int play_ch = -1;

    if(noSound)
        return;
}

void StopExtSound(const std::string& path)
{
    UNUSED(path);
    if(noSound)
        return;
}

void StopAllExtSounds()
{
    if(noSound)
        return;
}

void StopAllSounds()
{
    if(noSound)
        return;

    mmEffectCancelAll();
}

void ResetSoundFX()
{
}

void UpdateSoundFX(int recentSection)
{
    UNUSED(recentSection);
}

