#ifndef SOUND_H
#define SOUND_H

#include <string>

// Public Sub InitMixerX()
void InitMixerX();
// Public Sub QuitMixerX()
void QuitMixerX();
// Public Sub AddMusic(Alias As String, Path As String, FallBack As String)
void AddMusic(std::string Alias, std::string Path, std::string FallBack);
// Public Sub AddSfx(Alias As String, Path As String, FallBack As String)
void AddSfx(std::string Alias, std::string Path, std::string FallBack);
// Public Sub SetMusicVolume(Alias As String, Volume As Long)
void SetMusicVolume(std::string Alias, long Volume);
// Public Sub SoundPauseAll()
void SoundPauseAll();
// Public Sub SoundResumeAll()
void SoundResumeAll();
// Public Sub PlayMusic(Alias As String)
void PlayMusic(std::string Alias);
// Public Sub PlaySfx(Alias As String)
void PlaySfx(std::string Alias);
// Public Sub StopSfx(Alias As String)
void StopSfx(std::string Alias);
// Public Sub StartMusic(A As Integer) 'play music
// play music
void StartMusic(int A);
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
void PlaySound(int A);
// Public Sub BlockSound() 'stops all sound from being played for 10 cycles
// stops all sound from being played for 10 cycles
void BlockSound();
// Public Sub UpdateSound() 'checks to loop music and update the soundpause variable
// checks to loop music and update the soundpause variable
void UpdateSound();


#endif // SOUND_H
