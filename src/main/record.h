// this module handles the control recording and playback functions
// and the gameplay stats recording functions

#ifndef RECORD_H
#define RECORD_H

extern bool g_recordControlReplay;
extern bool g_recordControlRecord;
extern int g_recordReplayId;

void record_preload();

void record_init();

void record_finish();

void record_sync();

#endif // #ifndef RECORD_H