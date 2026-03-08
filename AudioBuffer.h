#ifndef AUDIOBUFFER_H
#define AUDIOBUFFER_H

#include <cstring>
#include <iostream>

using namespace std;

// Raw-pointer-based PCM audio buffer.
// Demonstrates manual memory management — no STL containers.
// All operations are in-place to minimize CPU cycles.
class AudioBuffer {
public:
  // Allocate a buffer for 'numFrames' interleaved stereo samples (2 floats per
  // frame)
  AudioBuffer(int numFrames) : numFrames(numFrames), numSamples(numFrames * 2) {
    data = new float[numSamples];
    clear();
  }

  // Destructor — manually free the raw pointer
  ~AudioBuffer() {
    delete[] data;
    data = nullptr;
  }

  // Zero out the entire buffer (hardware-level memset)
  void clear() { memset(data, 0, numSamples * sizeof(float)); }

  // Mix another buffer's samples into this one (additive, in-place)
  void mix(const AudioBuffer &other) {
    int limit = (other.numSamples < numSamples) ? other.numSamples : numSamples;
    float *dst = data;
    const float *src = other.data;
    for (int i = 0; i < limit; ++i) {
      dst[i] += src[i];
    }
  }

  // Scale all samples by a gain factor (in-place, no allocation)
  void scale(float gain) {
    float *ptr = data;
    for (int i = 0; i < numSamples; ++i) {
      ptr[i] *= gain;
    }
  }

  // Clamp all samples to [-1.0, 1.0] to prevent clipping
  void clamp() {
    float *ptr = data;
    for (int i = 0; i < numSamples; ++i) {
      if (ptr[i] > 1.0f)
        ptr[i] = 1.0f;
      if (ptr[i] < -1.0f)
        ptr[i] = -1.0f;
    }
  }

  // Direct raw pointer access (no bounds checking — mirrors hardware-level
  // access)
  float *raw() { return data; }
  const float *raw() const { return data; }

  int getNumFrames() const { return numFrames; }
  int getNumSamples() const { return numSamples; }

  // Transfer ownership of the internal buffer to another AudioBuffer.
  // After this call, 'this' owns nothing (data is nullptr).
  void transferOwnership(AudioBuffer &target) {
    delete[] target.data;
    target.data = data;
    target.numFrames = numFrames;
    target.numSamples = numSamples;
    data = nullptr;
    numFrames = 0;
    numSamples = 0;
  }

private:
  float *data;    // Raw PCM sample storage
  int numFrames;  // Number of stereo frames
  int numSamples; // numFrames * 2 (interleaved L/R)

  // Copy disabled — forces explicit ownership via transferOwnership()
  AudioBuffer(const AudioBuffer &);
  AudioBuffer &operator=(const AudioBuffer &);
};

#endif
