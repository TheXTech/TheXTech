// this module handles the control recording and playback functions
// and the gameplay stats recording functions

#ifndef RECORD_H
#define RECORD_H

namespace Record
{

extern FILE* record_file;
extern FILE* replay_file;

void LoadReplay(std::string recording_path);

void InitRecording();

void Sync();

void EndRecording();

}; // namespace Record

#endif // #ifndef RECORD_H