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

#include <SDL2/SDL_messagebox.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_mixer_ext.h>

#include "globals.h"
#include "load_gfx.h"
#include "pge_delay.h"

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

AudioSetup_t g_audioSetup;

static Mix_Music *g_curMusic = nullptr;
static bool g_mixerLoaded = false;

static int g_customLvlMusicId = 24;
static int g_customWldMusicId = 17;
static int g_reservedChannels = 0;

//! Total count of loaded default sounds
static unsigned int g_totalSounds = 0;
//! Are custom sounds was loaded from the level/world data folder?
static bool g_customSoundsInDataFolder = false;
//! Are custom music files was loaded from the level/world data folder?
static bool g_customMusicInDataFolder = false;
//! Total count of level music
static unsigned int g_totalMusicLevel = 0;
//! Total count of world map music
static unsigned int g_totalMusicWorld = 0;
//! Total count of special music
static unsigned int g_totalMusicSpecial = 0;
//! Enable using the unique iceball SFX when available
static bool s_useIceBallSfx = false;

static int g_errorsSfx = 0;
// static int g_errorsMusic = 0; // Unued yet

static std::string MusicRoot;
static std::string SfxRoot;

static std::string musicIni = "music.ini";
static std::string sfxIni = "sounds.ini";

struct Music_t
{
    std::string path;
    int volume = 52;
};

struct SFX_t
{
    std::string path;
    std::string customPath;
    Mix_Chunk *chunk = nullptr;
    Mix_Chunk *chunkOrig = nullptr;
    bool isCustom = false;
    bool isSilent = false;
    bool isSilentOrig = false;
    int volume = 128;
    int channel = -1;
};

static std::unordered_map<std::string, Music_t> music;
static std::unordered_map<std::string, SFX_t>   sound;

static const int maxSfxChannels = 91;

int CustomWorldMusicId()
{
    return g_customWldMusicId;
}

void InitMixerX()
{
    int ret;
    const int initFlags = MIX_INIT_MID|MIX_INIT_MOD|MIX_INIT_FLAC|MIX_INIT_OGG|MIX_INIT_OPUS|MIX_INIT_MP3;
    MusicRoot = AppPath + "music/";
    SfxRoot = AppPath + "sound/";

    if(g_mixerLoaded)
        return;

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

    ret = Mix_OpenAudio(g_audioSetup.sampleRate,
                        g_audioSetup.format,
                        g_audioSetup.channels,
                        g_audioSetup.bufferSize);

    if(ret < 0)
    {
        std::string msg = fmt::format_ne("Can't open audio stream, continuing without audio: ({0})", Mix_GetError());
        pLogCritical(msg.c_str());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Sound opening error", msg.c_str(), nullptr);
        noSound = true;
    }

    Mix_VolumeMusic(MIX_MAX_VOLUME);
    Mix_AllocateChannels(maxSfxChannels);

    g_mixerLoaded = true;
}

void QuitMixerX()
{
    if(!g_mixerLoaded)
        return;

    noSound = true;
    if(g_curMusic)
        Mix_FreeMusic(g_curMusic);
    g_curMusic = nullptr;

    for(auto it = sound.begin(); it != sound.end(); ++it)
    {
        auto &s = it->second;
        if(s.chunk)
            Mix_FreeChunk(s.chunk);
    }
    sound.clear();
    music.clear();
    Mix_CloseAudio();
    Mix_Quit();
}

static void AddMusic(const std::string &root,
                     IniProcessing &ini,
                     std::string alias,
                     std::string group,
                     int volume)
{
    std::string f;
    ini.beginGroup(group);
    ini.read("file", f, std::string());
    if(!f.empty())
    {
        Music_t m;
        m.path = root + f;
        m.volume = volume;
        pLogDebug("Adding music [%s] '%s'", alias.c_str(), m.path.c_str());
        auto a = music.find(alias);
        if(a == music.end())
            music.insert({alias, m});
        else
            a->second = m;
    }
    ini.endGroup();
}

static void RestoreSfx(SFX_t &u)
{
    if(u.isCustom)
    {
        if((u.chunk || u.isSilent) && (u.chunkOrig || u.isSilentOrig))
        {
            if(u.chunk)
                Mix_FreeChunk(u.chunk);
            u.chunk = u.chunkOrig;
            u.isSilent = u.isSilentOrig;
            u.chunkOrig = nullptr;
        }
        u.isCustom = false;
    }
}

static void AddSfx(const std::string &root,
                   IniProcessing &ini,
                   std::string alias,
                   std::string group,
                   bool isCustom = false)
{
    std::string f;
    bool isSilent;
    ini.beginGroup(group);
    ini.read("file", f, std::string());
    ini.read("silent", isSilent, false);
    if(!f.empty() || isSilent)
    {
        if(isCustom)
        {
            auto s = sound.find(alias);
            if(s != sound.end())
            {
                auto &m = s->second;
                std::string newPath = root + f;
                if(!isSilent && m.isCustom && newPath == m.customPath)
                {
                    ini.endGroup();
                    return;  // Don't load the same file twice!
                }

                Mix_Chunk *backup = m.chunk;
                bool backup_isSilent = m.isSilent;
                m.customPath = newPath;
                if(!isSilent)
                    m.chunk = Mix_LoadWAV((root + f).c_str());
                if(m.chunk || isSilent)
                {
                    if(!m.isCustom && !m.chunkOrig)
                    {
                        m.chunkOrig = backup;
                        m.isSilentOrig = backup_isSilent;
                    }
                    else if(backup)
                        Mix_FreeChunk(backup);
                    m.isCustom = true;
                    m.isSilent = isSilent;
                }
                else
                {
                    m.chunk = backup;
                    m.isSilent = backup_isSilent;
                    pLogWarning("ERROR: SFX '%s' loading error: %s", m.path.c_str(), Mix_GetError());
                }
            }
        }
        else
        {
            SFX_t m;
            m.path = root + f;
            m.volume = 128;
            m.isSilent = isSilent;
            pLogDebug("Adding SFX [%s] '%s'", alias.c_str(), isSilent ? "<silence>" : m.path.c_str());
            if(!isSilent)
                m.chunk = Mix_LoadWAV(m.path.c_str());
            m.channel = -1;
            if(m.chunk || isSilent)
            {
                bool isSingleChannel = false;
                ini.read("single-channel", isSingleChannel, false);
                if(isSingleChannel)
                    m.channel = g_reservedChannels++;
                sound.insert({alias, m});
            }
            else
            {
                pLogWarning("ERROR: SFX '%s' loading error: %s", m.path.c_str(), Mix_GetError());
                if(!isCustom)
                    g_errorsSfx++;
            }
        }
    }
    ini.endGroup();
}

void SetMusicVolume(std::string Alias, long Volume)
{
    auto mus = music.find(Alias);
    if(mus != music.end())
    {
        mus->second.volume = int(Volume);
    }
}

void SoundPauseAll()
{
    if(noSound)
        return;
    pLogDebug("Pause all sound");
    Mix_Pause(-1);
    Mix_PauseMusic();
}

void SoundResumeAll()
{
    if(noSound)
        return;
    pLogDebug("Resume all sound");
    Mix_Resume(-1);
    Mix_ResumeMusic();
}

static void processPathArgs(std::string &path,
                            const std::string &episodeRoot,
                            const std::string &dataDirName)
{
    if(path.find('|') == std::string::npos)
        return; // Nothing to do
    Strings::List p;
    Strings::split(p, path, '|');
    Strings::replaceInAll(p[1], "{e}", episodeRoot);
    Strings::replaceInAll(p[1], "{d}", episodeRoot + dataDirName);
    Strings::replaceInAll(p[1], "{r}", MusicRoot);
    path = p[0] + "|" + p[1];
}

void PlayMusic(std::string Alias, int fadeInMs)
{
    if(noSound)
        return;
    if(g_curMusic)
    {
        Mix_HaltMusic();
        Mix_FreeMusic(g_curMusic);
        g_curMusic = nullptr;
    }

    auto mus = music.find(Alias);
    if(mus != music.end())
    {
        auto &m = mus->second;
        std::string p = m.path;
        processPathArgs(p, FileNamePath + "/", FileName + "/");
        g_curMusic = Mix_LoadMUS(p.c_str());
        if(!g_curMusic)
        {
            pLogWarning("Music '%s' opening error: %s", m.path.c_str(), Mix_GetError());
        }
        else
        {
            Mix_VolumeMusicStream(g_curMusic, m.volume);
            if(fadeInMs > 0)
                Mix_FadeInMusic(g_curMusic, -1, fadeInMs);
            else
                Mix_PlayMusic(g_curMusic, -1);
        }
    }
}

void PlaySfx(std::string Alias, int loops)
{
    auto sfx = sound.find(Alias);
    if(sfx != sound.end())
    {
        auto &s = sfx->second;
        if(!s.isSilent)
            Mix_PlayChannel(s.channel, s.chunk, loops);
    }
}

void StopSfx(std::string Alias)
{
    auto sfx = sound.find(Alias);
    if(sfx != sound.end())
    {
        auto &s = sfx->second;
        if(!s.isSilent)
            Mix_HaltChannel(s.channel);
    }
}

void StartMusic(int A, int fadeInMs)
{
    if(noSound)
        return;

    if(LevelSelect && !GameMenu && !GameOutro) // music on the world map
    {
        StopMusic();
        curWorldMusic = A;
        std::string mus = fmt::format_ne("wmusic{0}", A);
        if(curWorldMusic == g_customWldMusicId)
        {
            pLogDebug("Starting custom music [%s]", curWorldMusicFile.c_str());
            if(g_curMusic)
                Mix_FreeMusic(g_curMusic);
            std::string p = FileNamePath + "/" + curWorldMusicFile;
            processPathArgs(p, FileNamePath + "/", FileName + "/");
            g_curMusic = Mix_LoadMUS(p.c_str());
            Mix_VolumeMusicStream(g_curMusic, 64);
            if(fadeInMs > 0)
                Mix_FadeInMusic(g_curMusic, -1, fadeInMs);
            else
                Mix_PlayMusic(g_curMusic, -1);
        }
        else
        {
            pLogDebug("Starting world music [%s]", mus.c_str());
            PlayMusic(mus, fadeInMs);
        }
        musicName = mus;
    }
    else if(A == -1) // P switch music
    {
        StopMusic();
        if(FreezeNPCs) {
            pLogDebug("Starting special music [stmusic]");
            PlayMusic("stmusic", fadeInMs);
            musicName = "stmusic";
        } else {
            pLogDebug("Starting special music [smusic]");
            PlayMusic("smusic", fadeInMs);
            musicName = "smusic";
        }
        curMusic = -1;
    }
    else if(PSwitchTime == 0 && PSwitchStop == 0) // level music
    {
        StopMusic();
        curMusic = bgMusic[A];
        std::string mus = fmt::format_ne("music{0}", curMusic);
        if(curMusic == g_customLvlMusicId)
        {
            pLogDebug("Starting custom music [%s]", CustomMusic[A].c_str());
            if(g_curMusic)
                Mix_FreeMusic(g_curMusic);
            std::string p = FileNamePath + "/" + CustomMusic[A];
            processPathArgs(p, FileNamePath + "/", FileName + "/");
            g_curMusic = Mix_LoadMUS(p.c_str());
            Mix_VolumeMusicStream(g_curMusic, 52);
            if(fadeInMs > 0)
                Mix_FadeInMusic(g_curMusic, -1, fadeInMs);
            else
                Mix_PlayMusic(g_curMusic, -1);
        }
        else
        {
            pLogDebug("Starting level music [%s]", mus.c_str());
            PlayMusic(mus);
        }
        musicName = mus;
    }

    musicPlaying = true;
}

void StopMusic()
{
    if(!musicPlaying || noSound)
        return;

    pLogDebug("Stopping music");

    Mix_HaltMusic();
    if(g_curMusic)
        Mix_FreeMusic(g_curMusic);
    g_curMusic = nullptr;
    musicPlaying = false;
}

void FadeOutMusic(int ms)
{
    if(!musicPlaying || noSound)
        return;
    pLogDebug("Fading out music");
    Mix_FadeOutMusic(ms);
    musicPlaying = false;
}


void PlayInitSound()
{
    std::string doSound = AppPath + "sound/";
    IniProcessing sounds(AppPath + "sounds.ini");
    unsigned int totalSounds;
    sounds.beginGroup("sound-main");
    sounds.read("total", totalSounds, 0);
    sounds.endGroup();

    if(totalSounds >= 29)
    {
        std::string p;
        sounds.beginGroup("sound-29");
        sounds.read("file", p, std::string());
        sounds.endGroup();

        if(!p.empty())
        {
            g_curMusic = Mix_LoadMUS((SfxRoot + p).c_str());
            Mix_PlayMusic(g_curMusic, 0);
            do // Synchroniously play the loading sound to don't distort it during the SFX loading
            {
                PGE_Delay(15);
                UpdateLoadREAL();
            } while(Mix_PlayingMusicStream(g_curMusic));
        }
    }
}

static void loadMusicIni(const std::string &root, const std::string &path, bool isLoadingCustom)
{
    IniProcessing musicSetup(path);
    if(!isLoadingCustom)
    {
        music.clear();
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
    }

    if(!isLoadingCustom)
        UpdateLoad();
    for(unsigned int i = 1; i <= g_totalMusicLevel; ++i)
    {
        std::string alias = fmt::format_ne("music{0}", i);
        std::string group = fmt::format_ne("level-music-{0}", i);
        AddMusic(root, musicSetup, alias, group, 52);
    }

    if(!isLoadingCustom)
        UpdateLoad();
    for(unsigned int i = 1; i <= g_totalMusicWorld; ++i)
    {
        std::string alias = fmt::format_ne("wmusic{0}", i);
        std::string group = fmt::format_ne("world-music-{0}", i);
        AddMusic(root, musicSetup, alias, group, 64);
    }

    if(!isLoadingCustom)
        UpdateLoad();
    for(unsigned int i = 1; i <= g_totalMusicSpecial; ++i)
    {
        std::string alias = fmt::format_ne("smusic{0}", i);
        if(i == 1)
            alias = "smusic";
        else if(i == 2)
            alias = "stmusic";
        else if(i == 3)
            alias = "tmusic";
        std::string group = fmt::format_ne("special-music-{0}", i);
        AddMusic(root, musicSetup, alias, group, 64);
    }
}

static void loadCustomSfxIni(const std::string &root, const std::string &path)
{
    IniProcessing sounds(path);
    for(unsigned int i = 1; i <= g_totalSounds; ++i)
    {
        std::string alias = fmt::format_ne("sound{0}", i);
        std::string group = fmt::format_ne("sound-{0}", i);
        AddSfx(root, sounds, alias, group, true);
    }
}

static void restoreDefaultSfx()
{
    for(auto &s : sound)
    {
        auto &u = s.second;
        RestoreSfx(u);
    }
}

void InitSound()
{
    if(noSound)
        return;

    musicIni = AppPath + "music.ini";
    sfxIni = AppPath + "sounds.ini";

    UpdateLoad();
    if(!Files::fileExists(musicIni) && !Files::fileExists(sfxIni))
    {
        pLogWarning("music.ini and sounds.ini are missing");
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
                                 "music.ini and sounds.ini are missing",
                                 "Files music.ini and sounds.ini are not exist, game will work without default music and SFX.",
                                 frmMain.getWindow());
        g_customLvlMusicId = 24;
        g_customWldMusicId = 17;
        return;
    }
    else if(!Files::fileExists(musicIni))
    {
        pLogWarning("music.ini is missing");
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
                                 "music.ini is missing",
                                 "File music.ini is not exist, game will work without default music.",
                                 frmMain.getWindow());
    }
    else if(!Files::fileExists(sfxIni))
    {
        pLogWarning("sounds.ini is missing");
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
                                 "sounds.ini is missing",
                                 "File sounds.ini is not exist, game will work without SFX.",
                                 frmMain.getWindow());
    }

    loadMusicIni(MusicRoot, musicIni, false);

    UpdateLoad();
    IniProcessing sounds(sfxIni);
    sounds.beginGroup("sound-main");
    sounds.read("total", g_totalSounds, 0);
    sounds.read("use-iceball-sfx", s_useIceBallSfx, false);
    bool playerUseNPCHammer;
    bool playerUseOwnHammer;
    sounds.read("player-use-npc-hammer-sfx", playerUseNPCHammer, false);
    sounds.read("player-use-own-hammer-sfx", playerUseOwnHammer, false);
    sounds.endGroup();

    if(playerUseOwnHammer)
        playerHammerSFX = SFX_PlayerHammer;
    else if(playerUseNPCHammer)
        playerHammerSFX = SFX_HammerToss;
    else
        playerHammerSFX = SFX_Fireball;

    UpdateLoad();
    for(unsigned int i = 1; i <= g_totalSounds; ++i)
    {
        std::string alias = fmt::format_ne("sound{0}", i);
        std::string group = fmt::format_ne("sound-{0}", i);
        AddSfx(SfxRoot, sounds, alias, group);
    }
    UpdateLoad();
    Mix_ReserveChannels(g_reservedChannels);

    if(g_errorsSfx > 0)
    {
        std::string msg = fmt::format_ne("Failed to load some SFX assets. Loo a log file to get more details:\n{0}", getLogFilePath());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Sounds loading error", msg.c_str(), frmMain.getWindow());
    }
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
#if 0
    switch(A)
    {
    case 2: SoundPause[A] = 12; break;
    case 3: SoundPause[A] = 12; break;
    case 4: SoundPause[A] = 12; break;
    case 5: SoundPause[A] = 30; break;
    case 8: SoundPause[A] = 10; break;
    case 9: SoundPause[A] = 4; break;
    case 10: SoundPause[A] = 8; break;
    case 12: SoundPause[A] = 10; break;
    case 17: SoundPause[A] = 10; break;
    case 26: SoundPause[A] = 8; break;
    case 31: SoundPause[A] = 20; break;
    case 37: SoundPause[A] = 10; break;
    case 42: SoundPause[A] = 16; break;
    case 50: SoundPause[A] = 8; break;
    case 54: SoundPause[A] = 8; break;
    case 71: SoundPause[A] = 9; break;
    case 74: SoundPause[A] = 8; break;
    case 81: SoundPause[A] = 5; break;
    case 86: SoundPause[A] = 8; break;
    default: SoundPause[A] = 4; break;
    }
#endif
}

static const std::unordered_map<int, int> s_soundFallback =
{
    {SFX_Iceball, SFX_Fireball},
    {SFX_Freeze, SFX_ShellHit},
    {SFX_Icebreak, SFX_ShellHit},
};

void PlaySound(int A, int loops)
{
    if(noSound)
        return;

    if(GameMenu || GameOutro) // || A == 26 || A == 27 || A == 29)
        return;

    if(A == SFX_Iceball && !s_useIceBallSfx)
        A = SFX_Fireball; // Fell back into fireball when iceball sound isn't preferred

    if(A > (int)g_totalSounds) // Play fallback sound for the missing SFX
    {
        auto fb = s_soundFallback.find(A);
        if(fb != s_soundFallback.end())
            A = fb->second;
    }

    if(numPlayers > 2)
        SoundPause[10] = 1;

    if(SoundPause[A] == 0) // if the sound wasn't just played
    {
        std::string alias = fmt::format_ne("sound{0}", A);
        PlaySfx(alias, loops);
        s_resetSoundDelay(A);
    }
}

void PlaySoundMenu(int A, int loops)
{
    if(SoundPause[A] == 0) // if the sound wasn't just played
    {
        std::string alias = fmt::format_ne("sound{0}", A);
        PlaySfx(alias, loops);
        s_resetSoundDelay(A);
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
}

void LoadCustomSound()
{
    if(noSound)
        return;

    if(GameMenu)
        return; // Don't load custom music in menu mode

    std::string mIni = FileNamePath + "music.ini";
    std::string sIni = FileNamePath + "sounds.ini";
    std::string mIniC = FileNamePath + FileName + "/music.ini";
    std::string sIniC = FileNamePath + FileName + "/sounds.ini";

    // To avoid bugs like custom local sounds was transferred into another level, it's need to clean-up old one if that was
    if(g_customMusicInDataFolder)
    {
        loadMusicIni(MusicRoot, musicIni, true);
        g_customMusicInDataFolder = false;
    }

    if(g_customSoundsInDataFolder)
    {
        restoreDefaultSfx();
        g_customSoundsInDataFolder = false;
    }

    if(FileNamePath == AppPath)
        return; // Don't treat default music/sounds ini as custom

    if(Files::fileExists(mIni)) // Load music.ini from an episode folder
        loadMusicIni(FileNamePath, mIni, true);

    if(Files::fileExists(mIniC)) // Load music.ini from a level/world custom folder
    {
        loadMusicIni(FileNamePath + FileName + "/", mIniC, true);
        g_customMusicInDataFolder = true;
    }

    if(Files::fileExists(sIni)) // Load sounds.ini from an episode folder
        loadCustomSfxIni(FileNamePath, sIni);

    if(Files::fileExists(sIniC)) // Load sounds.ini from a level/world custom folder
    {
        loadCustomSfxIni(FileNamePath + FileName + "/", sIniC);
        g_customSoundsInDataFolder = true;
    }
}

void UnloadCustomSound()
{
    if(noSound)
        return;
    loadMusicIni(MusicRoot, musicIni, true);
    restoreDefaultSfx();
    g_customMusicInDataFolder = false;
    g_customSoundsInDataFolder = false;
}
