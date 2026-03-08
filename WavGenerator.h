#ifndef WAVGENERATOR_H
#define WAVGENERATOR_H

#include <cmath>
#include <cstring>
#include <fstream>
#include <string>


#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace std;

// WavGenerator: utility to create synthetic WAV files for demo purposes.
// Generates sine wave tones at specific frequencies so the engine has
// real audio data to play. All memory managed via raw new[]/delete[].
class WavGenerator {
public:
  // Generate a WAV file with a sine wave tone
  // freq: frequency in Hz (e.g. 440 for A4)
  // durationSec: length in seconds
  // sampleRate: samples per second (default 44100)
  // Returns true on success
  static bool generateSineWav(const string &filePath, float freq,
                              float durationSec, int sampleRate = 44100) {
    int numChannels = 2;
    int bitsPerSample = 16;
    int numSamples = (int)(sampleRate * durationSec);
    int dataSize = numSamples * numChannels * (bitsPerSample / 8);

    // Allocate raw buffer for PCM data
    short *pcmData = new short[numSamples * numChannels];

    // Generate sine wave with slight stereo panning for richness
    for (int i = 0; i < numSamples; ++i) {
      float t = (float)i / sampleRate;
      float sample = sinf(2.0f * (float)M_PI * freq * t);

      // Apply a gentle amplitude envelope (attack + sustain + release)
      float envelope = 1.0f;
      float attackTime = 0.05f;
      float releaseTime = 0.1f;
      float releaseStart = durationSec - releaseTime;

      if (t < attackTime) {
        envelope = t / attackTime;
      } else if (t > releaseStart) {
        envelope = (durationSec - t) / releaseTime;
      }

      sample *= envelope * 0.7f; // Scale to 70% to avoid clipping

      short s = (short)(sample * 32767.0f);
      pcmData[i * 2] = s;     // Left channel
      pcmData[i * 2 + 1] = s; // Right channel
    }

    // Write WAV file
    ofstream file(filePath, ios::binary);
    if (!file.is_open()) {
      delete[] pcmData;
      return false;
    }

    int chunkSize = 36 + dataSize;
    int subchunk1Size = 16;
    short audioFormat = 1; // PCM
    short nc = (short)numChannels;
    int byteRate = sampleRate * numChannels * (bitsPerSample / 8);
    short blockAlign = (short)(numChannels * (bitsPerSample / 8));
    short bps = (short)bitsPerSample;

    // RIFF header
    file.write("RIFF", 4);
    file.write(reinterpret_cast<char *>(&chunkSize), 4);
    file.write("WAVE", 4);

    // fmt subchunk
    file.write("fmt ", 4);
    file.write(reinterpret_cast<char *>(&subchunk1Size), 4);
    file.write(reinterpret_cast<char *>(&audioFormat), 2);
    file.write(reinterpret_cast<char *>(&nc), 2);
    file.write(reinterpret_cast<char *>(&sampleRate), 4);
    file.write(reinterpret_cast<char *>(&byteRate), 4);
    file.write(reinterpret_cast<char *>(&blockAlign), 2);
    file.write(reinterpret_cast<char *>(&bps), 2);

    // data subchunk
    file.write("data", 4);
    file.write(reinterpret_cast<char *>(&dataSize), 4);
    file.write(reinterpret_cast<char *>(pcmData), dataSize);

    file.close();
    delete[] pcmData;
    return true;
  }

  // Generate a chord (multiple sine waves mixed together)
  static bool generateChordWav(const string &filePath, float *freqs,
                               int numFreqs, float durationSec,
                               int sampleRate = 44100) {
    int numChannels = 2;
    int bitsPerSample = 16;
    int numSamples = (int)(sampleRate * durationSec);
    int dataSize = numSamples * numChannels * (bitsPerSample / 8);

    short *pcmData = new short[numSamples * numChannels];

    for (int i = 0; i < numSamples; ++i) {
      float t = (float)i / sampleRate;
      float sample = 0.0f;

      // Mix all frequencies
      for (int f = 0; f < numFreqs; ++f) {
        sample += sinf(2.0f * (float)M_PI * freqs[f] * t);
      }
      sample /= numFreqs; // Normalize

      // Envelope
      float envelope = 1.0f;
      if (t < 0.05f)
        envelope = t / 0.05f;
      else if (t > durationSec - 0.1f)
        envelope = (durationSec - t) / 0.1f;

      sample *= envelope * 0.7f;
      short s = (short)(sample * 32767.0f);
      pcmData[i * 2] = s;
      pcmData[i * 2 + 1] = s;
    }

    ofstream file(filePath, ios::binary);
    if (!file.is_open()) {
      delete[] pcmData;
      return false;
    }

    int chunkSize = 36 + dataSize;
    int subchunk1Size = 16;
    short audioFormat = 1;
    short nc = (short)numChannels;
    int byteRate = sampleRate * numChannels * (bitsPerSample / 8);
    short blockAlign = (short)(numChannels * (bitsPerSample / 8));
    short bps = (short)bitsPerSample;

    file.write("RIFF", 4);
    file.write(reinterpret_cast<char *>(&chunkSize), 4);
    file.write("WAVE", 4);
    file.write("fmt ", 4);
    file.write(reinterpret_cast<char *>(&subchunk1Size), 4);
    file.write(reinterpret_cast<char *>(&audioFormat), 2);
    file.write(reinterpret_cast<char *>(&nc), 2);
    file.write(reinterpret_cast<char *>(&sampleRate), 4);
    file.write(reinterpret_cast<char *>(&byteRate), 4);
    file.write(reinterpret_cast<char *>(&blockAlign), 2);
    file.write(reinterpret_cast<char *>(&bps), 2);
    file.write("data", 4);
    file.write(reinterpret_cast<char *>(&dataSize), 4);
    file.write(reinterpret_cast<char *>(pcmData), dataSize);

    file.close();
    delete[] pcmData;
    return true;
  }
};

#endif
