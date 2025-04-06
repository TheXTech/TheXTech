/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2025 Vitaly Novichkov <admin@wohlnet.ru>
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

#include "pseudo_vb.h"
#include "globals.h"
#include "global_dirs.h"
#include "frame_timer.h"

#include "load_gfx.h"
#include "core/msgbox.h"

#include "sound.h"
#include "config.h"

#include <SDL2/SDL_rwops.h>

#include <Logger/logger.h>
#include <IniProcessor/ini_processing.h>
#include <Utils/files.h>
#include <Utils/files_ini.h>
#include <Utils/strings.h>
#include <unordered_map>
#include <fmt_format_ne.h>

#include "core/16m/sound_stream_16m.h"

static constexpr int s_max_effects = 1920;
static constexpr int s_max_modules =  128;
static int s_soundbank_effects = 0;
static int s_soundbank_modules = 0;

static mm_word s_soundbank_pointers[s_max_effects + s_max_modules] = {0};
static SDL_RWops* s_soundbank_rwops = nullptr;

// Public musicPlaying As Boolean
bool musicPlaying = false;
// Public musicLoop As Integer
int musicLoop = 0;
// Public musicName As String
std::string musicName;

int playerHammerSFX = SFX_Fireball;

const AudioDefaults_t g_audioDefaults{0, 0, 0, 0};

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

static int s_loaderEffect = -1;

static int s_curMusic = -1;
static uint16_t s_customSection = 0;
// static int s_curJingle = -1;
static std::vector<bool> s_soundLoaded;
static std::vector<int> s_worldMusicMods;
static std::vector<int> s_levelMusicMods;
static std::vector<int> s_specialMusicMods;
static std::vector<char*> s_worldMusicQoas;
static std::vector<char*> s_levelMusicQoas;
static std::vector<char*> s_specialMusicQoas;
static std::vector<int> s_sfxEffects;

static uint8_t* s_maxmod_rwops_load(int id)
{
    if(!s_soundbank_rwops)
        return nullptr;

    SDL_RWseek(s_soundbank_rwops, 12 + id * 4, RW_SEEK_SET);

    uint32_t off = 0;
    SDL_RWread(s_soundbank_rwops, &off, sizeof(off), 1);
    SDL_RWseek(s_soundbank_rwops, off, RW_SEEK_SET);

    uint32_t size = 0;
    SDL_RWread(s_soundbank_rwops, &size, sizeof(off), 1);
    SDL_RWseek(s_soundbank_rwops, off, RW_SEEK_SET);
    size += 8; // header

    uint8_t* mem = (uint8_t*)malloc(size);
    if(!mem)
    {
        pLogWarning("Sound: failed to allocate 0x%lx bytes to load #%d", size, id);
        return nullptr;
    }

    if(SDL_RWread(s_soundbank_rwops, mem, 1, size) != size)
    {
        pLogWarning("Sound: failed to load #%d", id);
        free(mem);
        return nullptr;
    }

    return mem;
}

static mm_word s_maxmod_loader(mm_word msg, mm_word param)
{
    switch(msg)
    {
        case MMCB_SAMPREQUEST:
            return reinterpret_cast<mm_word>(s_maxmod_rwops_load((int)param));
        case MMCB_SONGREQUEST:
            return reinterpret_cast<mm_word>(s_maxmod_rwops_load((int)param + s_soundbank_effects));
        case MMCB_DELETESAMPLE:
        case MMCB_DELETESONG:
            free(reinterpret_cast<void*>(param));
            break;
        default:
            break;
    }
    return 0;
}


int CustomWorldMusicId()
{
    return g_customWldMusicId;
}

void InitMixerX()
{
    if(g_mixerLoaded)
        return;

    mm_ds_system sys;
    sys.mod_count    = s_max_modules;
    sys.samp_count   = s_max_effects;
    sys.mem_bank     = s_soundbank_pointers;
#ifndef __CALICO__
    sys.fifo_channel = FIFO_MAXMOD;
#else
    sys.fifo_channel = 0;
#endif

    mmInit(&sys);
    mmSetCustomSoundBankHandler(s_maxmod_loader);
    g_mixerLoaded = true;
}

void LoadSoundBank()
{
    if(s_soundbank_rwops)
        return;

    std::string SoundbankPath = AppPath + "soundbank.bin";

    s_soundbank_rwops = Files::open_file(SoundbankPath, "rb");

    uint16_t nsamps, nmods;

    if(!s_soundbank_rwops || SDL_RWread(s_soundbank_rwops, &nsamps, 2, 1) != 1 || SDL_RWread(s_soundbank_rwops, &nmods, 2, 1) != 1)
    {
        pLogWarning("Could not open soundbank [%s]", SoundbankPath.c_str());

        if(s_soundbank_rwops)
            SDL_RWclose(s_soundbank_rwops);

        s_soundbank_rwops = nullptr;

        return;
    }

    s_soundbank_modules = nmods;
    s_soundbank_effects = nsamps;

    pLogDebug("Opened soundbank [%s] with %d modules and %d samples", SoundbankPath.c_str(), s_soundbank_modules, s_soundbank_effects);
}

void QuitMixerX()
{
    // maxmod doesn't provide us any way to fully unload
}

void RestartMixerX()
{
}

static void s_clearQoaLists()
{
    for(char* c : s_worldMusicQoas)
        free(c);
    s_worldMusicQoas.clear();

    for(char* c : s_levelMusicQoas)
        free(c);
    s_levelMusicQoas.clear();

    for(char* c : s_specialMusicQoas)
        free(c);
    s_specialMusicQoas.clear();
}

void UnloadSound()
{
    if(!g_mixerLoaded)
        return;

    mmStop();
    mmEffectCancelAll();

    StopMusic();

    for(unsigned int i = 0; i < g_totalSounds; i++)
    {
        if(s_soundLoaded[i])
            mmUnloadEffect(s_sfxEffects[i]);
        s_soundLoaded[i] = false;
    }

    if(s_soundbank_rwops)
    {
        SDL_RWclose(s_soundbank_rwops);
        s_soundbank_rwops = nullptr;
    }

    s_soundbank_effects = 0;
    s_soundbank_modules = 0;

    s_clearQoaLists();

    s_soundLoaded.clear();
    s_worldMusicMods.clear();
    s_levelMusicMods.clear();
    s_specialMusicMods.clear();
    s_sfxEffects.clear();
}


void InitSoundDefaults()
{
}

void SoundPauseAll()
{
    if(!g_mixerLoaded)
        return;

    mmPause();
    mmEffectCancelAll();
}

void SoundResumeAll()
{
    if(!g_mixerLoaded)
        return;

    mmResume();
}

void PauseMusic()
{
    if(!g_mixerLoaded)
        return;

    mmPause();
}

void ResumeMusic()
{
    if(!g_mixerLoaded)
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

static void s_playMusic(int mod, const char* qoa, int fadeInMs)
{
    StopMusic();
    if(mod == -2 && s_customSection > maxSections)
        return;

    if(mod == -1 && qoa == nullptr)
        return;

    s_curMusic = mod;

    if(qoa)
        s_curMusic = -4;

    if(!g_mixerLoaded)
        return;

    if(qoa)
    {
        g_stats.currentMusicFile = qoa;

        std::string music_path = AppPath + "music/" + qoa;
        pLogDebug("Trying to play %s from %s...", qoa, music_path.c_str());
        Sound_StreamStart(Files::open_file(music_path, "rb"));
    }
    else if(mod > -1)
    {
        mmLoad(mod);
        s_MusicSetupFade(1024, fadeInMs);
        mmStart(mod, MM_PLAY_LOOP);
    }
    else if(mod == -2 || mod == -3)
    {
        std::string p = ((mod == -3) ? (curWorldMusicFile) : CustomMusic[s_customSection]);

        // remove pipe and add qoa
        for(size_t i = 0; i < p.size(); i++)
        {
            if(p[i] == '|')
            {
                p.resize(i);
                break;
            }
        }
        p += ".qoa";

        g_stats.currentMusicFile = p;

        p = g_dirEpisode.resolveFileCaseExistsAbs(p);

        Sound_StreamStart(Files::open_file(p, "rb"));
    }
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
    if(!g_mixerLoaded)
        return;

    StopMusic();

    if(Alias == "tmusic")
    {
        pLogDebug("Starting special music [3]");
        int index = -1;
        const char* qoa = nullptr;
        if(2 < s_specialMusicMods.size())
        {
            index = s_specialMusicMods[2];
            qoa = s_specialMusicQoas[2];
        }
        s_playMusic(index, qoa, fadeInMs);
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

    if(!g_mixerLoaded)
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

        int index = -1;
        const char* qoa = nullptr;
        if(curWorldMusic == g_customWldMusicId)
            index = -3;
        else if(A >= 1 && A - 1 < (int)s_worldMusicMods.size())
        {
            index = s_worldMusicMods[A - 1];
            qoa = s_worldMusicQoas[A - 1];
        }

        s_playMusic(index, qoa, fadeInMs);
    }
    else if(A == -1) // P switch music
    {
        StopMusic();
        if(FreezeNPCs)
        {
            pLogDebug("Starting special music [2]");
            int index = -1;
            const char* qoa = nullptr;
            if(1 < s_specialMusicMods.size())
            {
                index = s_specialMusicMods[1];
                qoa = s_specialMusicQoas[1];
            }
            s_playMusic(index, qoa, fadeInMs);
            musicName = "stmusic";
        }
        else if(InvincibilityTime && !PSwitchTime && s_specialMusicMods.size() >= 4)
        {
            pLogDebug("Starting special music [4]");
            int index = s_specialMusicMods[3];
            const char* qoa = s_specialMusicQoas[3];
            s_playMusic(index, qoa, fadeInMs);
            musicName = "smusic4";
        }
        else
        {
            pLogDebug("Starting special music [1]");
            int index = -1;
            const char* qoa = nullptr;
            if(0 < s_specialMusicMods.size())
            {
                index = s_specialMusicMods[0];
                qoa = s_specialMusicQoas[0];
            }
            s_playMusic(index, qoa, fadeInMs);
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
        const char* qoa = nullptr;
        if(curMusic == g_customLvlMusicId)
        {
            s_customSection = A;
            index = -2;
        }
        else if(curMusic >= 1 && curMusic - 1 < (int)s_levelMusicMods.size())
        {
            index = s_levelMusicMods[curMusic - 1];
            qoa = s_levelMusicQoas[curMusic - 1];
        }

        s_playMusic(index, qoa, fadeInMs);

        musicName = std::move(mus);
    }

    musicPlaying = true;
}

void StartMusicIfOnscreen(int section)
{
    for(int i = 0; i < l_screen->player_count; i++)
    {
        if(Player[l_screen->players[i]].Section == section)
        {
            StartMusic(section);
            break;
        }
    }
}

void StopMusic()
{
    if(!musicPlaying || !g_mixerLoaded)
        return;

    pLogDebug("Stopping music");

    if(s_curMusic > -1)
    {
        mmPause();
        mmStop();
        // busy-wait on the main core
        while(mmActive()) {}
        mmUnload(s_curMusic);
    }
    else if(s_curMusic < -1)
        Sound_StreamStop();

    s_curMusic = -1;
    musicPlaying = false;
    g_stats.currentMusic.clear();
}

void FadeOutMusic(int ms)
{
    if(s_curMusic < -1)
        StopMusic();
    else
        s_MusicSetupFade(0, ms);
}


void UpdateMusicVolume()
{
    // currently unused
}

void PlayInitSound()
{
    if(!g_mixerLoaded)
        return;

    if(!s_soundbank_rwops)
        LoadSoundBank();

    if(!s_soundbank_rwops)
        return;

    if(s_loaderEffect != -1)
    {
        mmUnloadEffect(s_loaderEffect);
        s_loaderEffect = -1;
    }

    // std::string doSound = AppPath + "sound/";
    IniProcessing sounds = Files::load_ini(AppPath + "sounds.ini");
    unsigned int totalSounds;
    sounds.beginGroup("sound-main");
    sounds.read("total", totalSounds, 0);
    sounds.endGroup();

    if(totalSounds >= SFX_Do)
    {
        sounds.beginGroup("sound-29");
        int effect;
        sounds.read("resolved-sfx", effect, -1);
        sounds.endGroup();

        if(effect != -1)
        {
            mmLoadEffect(effect);
            mmEffect(effect);
            s_loaderEffect = effect;
        }
    }
}

static void loadMusicIni(const std::string &path, bool isLoadingCustom)
{
    IniProcessing musicSetup = Files::load_ini(path);
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

    s_clearQoaLists();
    s_levelMusicQoas.reserve(g_totalMusicLevel);
    s_worldMusicQoas.reserve(g_totalMusicWorld);
    s_specialMusicQoas.reserve(g_totalMusicSpecial);

    std::string group;
    std::string got_qoa;
    for(unsigned int i = 1; i <= g_totalMusicLevel; ++i)
    {
        group = fmt::format_ne("level-music-{0}", i);
        musicSetup.beginGroup(group);
        musicSetup.read("resolved-mod", mod, -1);
        musicSetup.read("file-qoa", got_qoa, "");
        musicSetup.endGroup();

        s_levelMusicMods.push_back(mod);

        char* qoa = nullptr;
        if(!got_qoa.empty())
            qoa = strdup(got_qoa.c_str());
        s_levelMusicQoas.push_back(qoa);
    }

    if(!isLoadingCustom)
        UpdateLoad();
    for(unsigned int i = 1; i <= g_totalMusicWorld; ++i)
    {
        std::string group = fmt::format_ne("world-music-{0}", i);
        musicSetup.beginGroup(group);
        musicSetup.read("resolved-mod", mod, -1);
        musicSetup.read("file-qoa", got_qoa, "");
        musicSetup.endGroup();

        s_worldMusicMods.push_back(mod);

        char* qoa = nullptr;
        if(!got_qoa.empty())
            qoa = strdup(got_qoa.c_str());
        s_worldMusicQoas.push_back(qoa);
    }

    if(!isLoadingCustom)
        UpdateLoad();
    for(unsigned int i = 1; i <= g_totalMusicSpecial; ++i)
    {
        std::string group = fmt::format_ne("special-music-{0}", i);
        musicSetup.beginGroup(group);
        musicSetup.read("resolved-mod", mod, -1);
        musicSetup.read("file-qoa", got_qoa, "");
        musicSetup.endGroup();

        s_specialMusicMods.push_back(mod);

        char* qoa = nullptr;
        if(!got_qoa.empty())
            qoa = strdup(got_qoa.c_str());
        s_specialMusicQoas.push_back(qoa);
    }
}

void InitSound()
{
    if(!g_mixerLoaded)
        return;

    if(s_loaderEffect != -1)
    {
        mmUnloadEffect(s_loaderEffect);
        s_loaderEffect = -1;
    }

    if(!s_soundbank_rwops)
        LoadSoundBank();

    if(!s_soundbank_rwops)
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
    IniProcessing sounds = Files::load_ini(sfxIni);
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
    s_sfxEffects.resize(0);
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

void PlaySoundInternal(int A, int loops, int volume, int l, int r);

void PlaySound(int A, int loops, int volume)
{
    if(GameMenu || GameOutro) // || A == 26 || A == 27 || A == 29)
        return;

    UNUSED(volume);
    PlaySoundInternal(A, loops, 128, 255, 255);
}

void PlaySoundSpatial(int A, int l, int t, int r, int b, int loops, int volume)
{
    if(GameMenu || GameOutro) // || A == 26 || A == 27 || A == 29)
        return;

    uint8_t left = 255, right = 255;

    if(g_config.sfx_spatial_audio)
        Sound_ResolveSpatialMod(left, right, l, t, r, b);

    PlaySoundInternal(A, loops, volume, left, right);
}

bool HasSound(int A)
{
    return A <= (int)g_totalSounds;
}

void PlaySoundMenu(int A, int loops)
{
    PlaySoundInternal(A, loops, 128, 255, 255);
}

void PlaySoundInternal(int A, int loops, int volume, int l, int r)
{
    if(!g_mixerLoaded)
        return;

    UNUSED(loops);

    if(A > (int)g_totalSounds || !g_config.sfx_modern) // Play fallback sound for the missing SFX
        A = getFallbackSfx(A);
    else if(!s_useIceBallSfx && A == SFX_Iceball)
        A = SFX_Fireball; // Fell back into fireball when iceball sound isn't preferred
    else if(!s_useIceBallSfx && A == SFX_HeroIce)
        A = SFX_HeroFire;
    else if(!s_useNewIceSfx && (A == SFX_Freeze || A == SFX_Icebreak))
        A = SFX_ShellHit; // Restore the old behavior

    if(g_ClonedPlayerMode && A == SFX_Skid)
        return;

    if(SoundPause[A] == 0) // if the sound wasn't just played
    {
        s_resetSoundDelay(A);

        int index = -1;
        if(A >= 1 && A - 1 < (int)s_sfxEffects.size())
            index = s_sfxEffects[A - 1];

        if(index == -1)
            return;

        if(!s_soundLoaded[A - 1])
            mmLoadEffect(index);
        s_soundLoaded[A - 1] = true;

        mm_sound_effect effect_spec;
        effect_spec.id = index;
        effect_spec.rate = 1024;
        effect_spec.handle = 0;
        effect_spec.volume = volume * (l + r) / 256; // volume out of 128, l and r out of 256
        effect_spec.panning = 128 + (r - l) * 127 / (l + r);

        mm_sfxhand handle = mmEffectEx(&effect_spec);
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
    if(!g_mixerLoaded)
        return;

    For(A, 1, numSounds)
    {
        if(SoundPause[A] > 0)
            SoundPause[A] -= 1;
    }

    Sound_StreamUpdate();

    s_MusicUpdateFade();
}

void LoadCustomSound()
{
    if(!g_mixerLoaded)
        return;

    if(GameMenu || GameOutro)
        return; // Don't load custom music in menu mode
}

void UnloadCustomSound()
{
    if(!g_mixerLoaded)
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
    if(!g_mixerLoaded)
        return;
}

void UnloadExtSounds()
{
    if(!g_mixerLoaded)
        return;
}

void PlayExtSound(const std::string &path, int loops, int volume)
{
    UNUSED(path);
    UNUSED(loops);
    UNUSED(volume);
    // int play_ch = -1;

    if(!g_mixerLoaded)
        return;
}

void StopExtSound(const std::string& path)
{
    UNUSED(path);
    if(!g_mixerLoaded)
        return;
}

void StopAllExtSounds()
{
    if(!g_mixerLoaded)
        return;
}

void StopAllSounds()
{
    if(!g_mixerLoaded)
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

