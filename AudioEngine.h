#ifndef AUDIOENGINE_H
#define AUDIOENGINE_H

#include "AudioBuffer.h"
#include "AudioEffect.h"
#include "AudioNode.h"
#include <mmsystem.h>
#include <windows.h>


// AudioEngine: the core real-time playback engine.
// Uses Windows waveOut API for low-latency audio output.
// Double-buffered: two AudioBuffer instances alternate for gapless playback.
class AudioEngine {
public:
  AudioEngine();
  ~AudioEngine();

  // Initialize the audio device (call once before play)
  bool init(int sampleRate = 44100, int channels = 2, int bufferFrames = 4096);

  // Shutdown audio device
  void shutdown();

  // Playback control
  bool
  play(AudioNode *node); // Start playing an AudioNode (polymorphic dispatch)
  void pause();
  void resume();
  void stop();

  // Volume (0.0 — 1.0)
  void setVolume(float vol);
  float getVolume() const;

  // State queries
  bool isPlaying() const;
  bool isPaused() const;
  bool isInitialized() const;

  // Render the next block of audio and send to device
  void renderBlock();

  // Static callback for waveOut — called when a buffer finishes playing
  static void CALLBACK waveOutCallback(HWAVEOUT hwo, UINT uMsg,
                                       DWORD_PTR dwInstance, DWORD_PTR dwParam1,
                                       DWORD_PTR dwParam2);

private:
  HWAVEOUT hWaveOut;
  WAVEFORMATEX waveFormat;
  WAVEHDR waveHeaders[2];        // Double buffer
  short *outputBuffers[2];       // 16-bit PCM output (what waveOut expects)
  AudioBuffer *renderBuffers[2]; // Float PCM working buffers

  AudioNode *currentNode;     // Polymorphic source (raw pointer, non-owning)
  VolumeEffect *volumeEffect; // Wraps currentNode for volume control

  int sampleRate;
  int numChannels;
  int bufferFrames;
  int currentBuffer; // Index of the buffer currently being filled

  bool initialized;
  bool playing;
  bool paused;
  float volume;

  void prepareBuffer(int bufferIndex);
  void convertFloatToInt16(const float *src, short *dst, int numSamples);
};

#endif
