#ifndef AUDIOBUFFER_H
#define AUDIOBUFFER_H

#include <cstring>
#include <iostream>


// Manages a raw float* PCM buffer with in-place operations.
class AudioBuffer {
public:
  AudioBuffer(int frames, int channels = 2)
      : numFrames(frames), numChannels(channels) {
    int total = frames * channels;
    data = new float[total];
    memset(data, 0, total * sizeof(float));
  }

  ~AudioBuffer() { delete[] data; }

  float *raw() { return data; }
  const float *raw() const { return data; }
  int getNumFrames() const { return numFrames; }
  int getNumChannels() const { return numChannels; }

  void clear() { memset(data, 0, numFrames * numChannels * sizeof(float)); }

  void mix(const AudioBuffer &other) {
    int total = numFrames * numChannels;
    for (int i = 0; i < total; ++i)
      data[i] += other.data[i];
  }

  void scale(float factor) {
    int total = numFrames * numChannels;
    for (int i = 0; i < total; ++i)
      data[i] *= factor;
  }

  void clamp() {
    int total = numFrames * numChannels;
    for (int i = 0; i < total; ++i) {
      if (data[i] > 1.0f)
        data[i] = 1.0f;
      if (data[i] < -1.0f)
        data[i] = -1.0f;
    }
  }

private:
  float *data;
  int numFrames;
  int numChannels;
};

#endif
