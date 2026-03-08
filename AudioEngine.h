#ifndef AUDIOENGINE_H
#define AUDIOENGINE_H

// clang-format off
#include <windows.h>
#include <mmsystem.h>
// clang-format on

#include "AudioBuffer.h"
#include "AudioEffect.h"
#include "AudioNode.h"
#include "SpectrumAnalyzer.h"

// Effect slot indices
enum EffectSlot {
  SLOT_ECHO = 0,
  SLOT_REVERB,
  SLOT_BASS_BOOST,
  SLOT_DISTORTION,
  SLOT_SPEED,
  SLOT_VOLUME,
  NUM_EFFECT_SLOTS
};

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

  // DSP effect controls (0.0 = off, higher = more effect)
  void setEcho(float level);
  void setReverb(float level);
  void setBassBoost(float level);
  void setDistortion(float level);
  void setSpeed(float factor);

  float getEcho() const;
  float getReverb() const;
  float getBassBoost() const;
  float getDistortion() const;
  float getSpeed() const;
  void clearEffects();

  bool isPlaying() const;
  bool isPaused() const;
  bool isInitialized() const;

  void renderBlock();
  const float *getLastBuffer() const;
  int getBufferFrames() const;
  const float *getSpectrumBands() const;
  int getSpectrumNumBands() const;

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
  AudioNode *rawSource;

  // Effect parameters (stored for rebuilding chain)
  float echoLevel;
  float reverbLevel;
  float bassLevel;
  float distortionLevel;
  float speedFactor;

  int sampleRate;
  int numChannels;
  int bufferFrames;
  int currentBuffer;

  bool initialized;
  bool playing;
  bool paused;
  float volume;

  SpectrumAnalyzer *spectrum;

  void rebuildEffectChain();
  void cleanupEffectChain();
  void prepareBuffer(int bufferIndex);
  void convertFloatToInt16(const float *src, short *dst, int numSamples);
  void resetEffectParams();
};

#endif
