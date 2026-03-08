#ifndef WAVGENERATOR_H
#define WAVGENERATOR_H

#include <cmath>
#include <cstring>
#include <fstream>
#include <string>


#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Utility to generate synthetic WAV files for testing.
class WavGenerator {
public:
  static bool generateSineWav(const std::string &filePath, float freq,
                              float durationSec, int sampleRate = 44100) {
    int numChannels = 2;
    int bitsPerSample = 16;
    int numSamples = (int)(sampleRate * durationSec);
    int dataSize = numSamples * numChannels * (bitsPerSample / 8);

    short *pcmData = new short[numSamples * numChannels];

    for (int i = 0; i < numSamples; ++i) {
      float t = (float)i / sampleRate;
      float sample = sinf(2.0f * (float)M_PI * freq * t);

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

    std::ofstream file(filePath, std::ios::binary);
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

  static bool generateChordWav(const std::string &filePath, float *freqs,
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
      for (int f = 0; f < numFreqs; ++f)
        sample += sinf(2.0f * (float)M_PI * freqs[f] * t);
      sample /= numFreqs;

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

    std::ofstream file(filePath, std::ios::binary);
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
