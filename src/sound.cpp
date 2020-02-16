#include "globals.h"
#include "load_gfx.h"

#include "sound.h"

#include <Logger/logger.h>
#include <IniProcessor/ini_processing.h>
#include <SDL2/SDL_mixer_ext.h>
#include <unordered_map>
#include <fmt_format_ne.h>

#include "pseudo_vb.h"

// Public musicPlaying As Boolean
bool musicPlaying = false;
// Public musicLoop As Integer
int musicLoop = 0;
// Public musicName As String
std::string musicName;

static Mix_Music *g_curMusic = nullptr;
static bool g_mixerLoaded = false;

static int g_customLvlMusicId = 24;
static int g_customWldMusicId = 17;
static int g_reservedChannels = 0;

static std::string MusicRoot;
static std::string SfxRoot;

struct Music_t
{
    std::string path;
    int volume = 52;
};

struct SFX_t
{
    std::string path;
    Mix_Chunk *chunk = nullptr;
    int volume = 128;
    int channel = -1;
};

static std::unordered_map<std::string, Music_t> music;
static std::unordered_map<std::string, SFX_t>   sound;

static const int maxSfxChannels = 91;

void InitMixerX()
{
    MusicRoot = AppPath + "music/";
    SfxRoot = AppPath + "sound/";

    if(g_mixerLoaded)
        return;
    pLogDebug("Opening sound...");
    Mix_Init(MIX_INIT_MID|MIX_INIT_MOD|MIX_INIT_FLAC|MIX_INIT_OGG|MIX_INIT_OPUS|MIX_INIT_MP3);
    if(Mix_OpenAudio(44100, AUDIO_S16LSB, 2, 2048) < 0)
    {
        pLogCritical("Can't open audio stream, continuing without audio: (%s)", Mix_GetError());
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

static void AddMusic(IniProcessing &ini, std::string alias, std::string group, int volume)
{
    std::string f;
    ini.beginGroup(group);
    ini.read("file", f, std::string());
    if(!f.empty())
    {
        Music_t m;
        m.path = MusicRoot + f;
        m.volume = volume;
        pLogDebug("Adding music [%s] '%s'", alias.c_str(), m.path.c_str());
        music.insert({alias, m});
    }
    ini.endGroup();
}

static void AddSfx(IniProcessing &ini, std::string alias, std::string group)
{
    std::string f;
    ini.beginGroup(group);
    ini.read("file", f, std::string());
    if(!f.empty())
    {
        SFX_t m;
        m.path = SfxRoot + f;
        m.volume = 128;
        pLogDebug("Adding SFX [%s] '%s'", alias.c_str(), m.path.c_str());
        m.chunk = Mix_LoadWAV(m.path.c_str());
        m.channel = -1;
        if(m.chunk)
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
    pLogDebug("Pause all sound");
    Mix_Pause(-1);
    Mix_PauseMusic();
}

void SoundResumeAll()
{
    pLogDebug("Resume all sound");
    Mix_Resume(-1);
    Mix_ResumeMusic();
}

void PlayMusic(std::string Alias)
{
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
        g_curMusic = Mix_LoadMUS(m.path.c_str());
        if(!g_curMusic)
        {
            pLogWarning("Music '%s' opening error: %s", m.path.c_str(), Mix_GetError());
        }
        else
        {
            Mix_PlayMusic(g_curMusic, -1);
            Mix_VolumeMusic(m.volume);
        }
    }
}

void PlaySfx(std::string Alias)
{
    auto sfx = sound.find(Alias);
    if(sfx != sound.end())
    {
        auto &s = sfx->second;
        Mix_PlayChannel(s.channel, s.chunk, 0);
    }
}

void StopSfx(std::string Alias)
{
    auto sfx = sound.find(Alias);
    if(sfx != sound.end())
    {
        auto &s = sfx->second;
        Mix_HaltChannel(s.channel);
    }
}

void StartMusic(int A)
{
    if(noSound)
        return;

    if(LevelSelect && !GameMenu) // music on the world map
    {
        StopMusic();
        std::string mus = fmt::format_ne("wmusic{0}", A);
        pLogDebug("Starting world music [%s]", mus.c_str());
        PlayMusic(mus);
        musicName = mus;
        curWorldMusic = A;
    }
    else if(A == -1) // P switch music
    {
        StopMusic();
        if(FreezeNPCs) {
            pLogDebug("Starting special music [stmusic]");
            PlayMusic("stmusic");
            musicName = "stmusic";
        } else {
            pLogDebug("Starting special music [smusic]");
            PlayMusic("smusic");
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
            g_curMusic = Mix_LoadMUS((FileNamePath + "/" + CustomMusic[A]).c_str());
            Mix_PlayMusic(g_curMusic, -1);
            Mix_VolumeMusic(52);
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
        }
    }
}

void InitSound()
{
    if(noSound)
        return;

    UpdateLoad();
    IniProcessing music(AppPath + "music.ini");
    unsigned int totalLevel;
    unsigned int totalWorld;
    unsigned int totalSpecial;
    music.beginGroup("music-main");
    music.read("total-level", totalLevel, 0);
    music.read("total-world", totalWorld, 0);
    music.read("total-special", totalSpecial, 0);
    music.read("level-custom-music-id", g_customLvlMusicId, 0);
    music.read("world-custom-music-id", g_customWldMusicId, 0);
    music.endGroup();

    UpdateLoad();
    for(unsigned int i = 1; i <= totalLevel; ++i)
    {
        std::string alias = fmt::format_ne("music{0}", i);
        std::string group = fmt::format_ne("level-music-{0}", i);
        AddMusic(music, alias, group, 52);
    }

    UpdateLoad();
    for(unsigned int i = 1; i <= totalWorld; ++i)
    {
        std::string alias = fmt::format_ne("wmusic{0}", i);
        std::string group = fmt::format_ne("world-music-{0}", i);
        AddMusic(music, alias, group, 64);
    }

    UpdateLoad();
    for(unsigned int i = 1; i <= totalSpecial; ++i)
    {
        std::string alias = fmt::format_ne("smusic{0}", i);
        if(i == 1)
            alias = "smusic";
        else if(i == 2)
            alias = "stmusic";
        else if(i == 3)
            alias = "tmusic";
        std::string group = fmt::format_ne("special-music-{0}", i);
        AddMusic(music, alias, group, 64);
    }

    UpdateLoad();
    IniProcessing sounds(AppPath + "sounds.ini");
    unsigned int totalSounds;
    sounds.beginGroup("sound-main");
    sounds.read("total", totalSounds, 0);
    sounds.endGroup();

    UpdateLoad();
    for(unsigned int i = 1; i <= totalSounds; ++i)
    {
        std::string alias = fmt::format_ne("sound{0}", i);
        std::string group = fmt::format_ne("sound-{0}", i);
        AddSfx(sounds, alias, group);
    }
    UpdateLoad();
    Mix_ReserveChannels(g_reservedChannels);
}

void PlaySound(int A)
{
    if(noSound)
        return;

    if((!GameMenu || A == 26 || A == 29) && !GameOutro)
    {
        if(numPlayers > 2)
            SoundPause[10] = 1;

        if(SoundPause[A] == 0) // if the sound wasn't just played
        {
            std::string alias = fmt::format_ne("sound{0}", A);
            PlaySfx(alias);

            switch(A) // set the delay before a sound can be played again
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
        }
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
