// this module handles particular the control recording and playback functions
// and the gameplay stats recording functions

extern bool g_recordControlReplay;
extern bool g_recordControlRecord;
extern bool g_recordGameplay;
extern int g_recordReplayId;

void record_preload();

void record_init();

void record_finish();

void record_sync();
