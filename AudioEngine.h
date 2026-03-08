#ifndef AUDIOENGINE_H
#define AUDIOENGINE_H

// clang-format off
#include <windows.h>
#include <mmsystem.h>
// clang-format on

#include "AudioBuffer.h"
#include "AudioEffect.h"
#include "AudioNode.h"

// Real-time playback engine using Windows waveOut API with double buffering.
class AudioEngine {
public:
  AudioEngine();
  ~AudioEngine();

  bool init(int sampleRate = 44100, int channels = 2, int bufferFrames = 4096);
  void shutdown();

  bool play(AudioNode *node);
  void pause();
  void resume();
  void stop();

  void setVolume(float vol);
  float getVolume() const;

  bool isPlaying() const;
  bool isPaused() const;
  bool isInitialized() const;

  void renderBlock();

  static void CALLBACK waveOutCallback(HWAVEOUT hwo, UINT uMsg,
                                       DWORD_PTR dwInstance, DWORD_PTR dwParam1,
                                       DWORD_PTR dwParam2);

private:
  HWAVEOUT hWaveOut;
  WAVEFORMATEX waveFormat;
  WAVEHDR waveHeaders[2];
  short *outputBuffers[2];
  AudioBuffer *renderBuffers[2];

  AudioNode *currentNode;
  VolumeEffect *volumeEffect;

  int sampleRate;
  int numChannels;
  int bufferFrames;
  int currentBuffer;

  bool initialized;
  bool playing;
  bool paused;
  float volume;

  void prepareBuffer(int bufferIndex);
  void convertFloatToInt16(const float *src, short *dst, int numSamples);
};

#endif
