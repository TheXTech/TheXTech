#include <3ds.h>

#ifndef NUM_CHANNELS
// must *never* exceed 32
#define NUM_CHANNELS 16
#define NUM_BUFFERS 4
#define BUFFER_SIZE (512 * (44100 / 1000))

#define THREAD_STACK_SZ 32 * 1024

#endif

typedef uintptr_t SoundId;

typedef enum _CHANNEL_FORMAT {
 FORMAT_FREE = 1 << 0,
 FORMAT_LOADING = 1 << 1,
 FORMAT_PCM_FILE = 1 << 2,
 FORMAT_OGG_FILE = 1 << 3,
 FORMAT_PCM_MEM = 1 << 4,
 FORMAT_GME_FILE = 1 << 5,
} CHANNEL_FORMAT;

typedef struct _SimpleChannel {
    int channel_id;
    CHANNEL_FORMAT format;
    void *data_pointer;
    size_t data_idx;
    size_t data_size;
    ndspWaveBuf wavebufs[NUM_BUFFERS];
    bool kill;
    bool stereo;
    int loops;
    SoundId index;
} SimpleChannel;

typedef struct _WaveObject {
    bool stereo;
    uint32_t sampleRate;
    const char* data;
    uint32_t length;
} WaveObject;

static const SoundId INVALID_ID = -1;

bool audioInit();

// Audio de-initialisation code
// Stops playback and frees the primary audio buffer
void audioExit();

void audioPause();
void audioResume();

const char* get_filename_ext(const char* filename);

SoundId playSoundWAV(const char* path, int loops=0);

SoundId playSoundMem(const WaveObject* wave, int loops=0);

SoundId playSoundOGG(const char* path, int loops=-1);

SoundId playSoundGME(const char* path, int loops=-1);

SoundId playSoundAuto(const char* path, int loops=-1);

void killSound(SoundId soundId);

WaveObject* audioLoadWave(const char* path);

void audioFreeWave(WaveObject* wave);

void audioSetChannelDoneCallback(void (*cb)(int));
