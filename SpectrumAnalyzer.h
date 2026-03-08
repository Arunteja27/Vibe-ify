#ifndef SPECTRUMANALYZER_H
#define SPECTRUMANALYZER_H

#include <cmath>
#include <cstring>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Real-time FFT spectrum analyzer using Cooley-Tukey radix-2.
// Operates on the last rendered PCM buffer to produce frequency bands.
class SpectrumAnalyzer {
public:
  SpectrumAnalyzer(int fftSize = 1024, int numBands = 64);
  ~SpectrumAnalyzer();

  // Analyze a stereo float buffer, stores result internally
  void analyze(const float *pcmBuffer, int numFrames);

  // Get the magnitude array (numBands floats, 0.0-1.0 normalized)
  const float *getBands() const;
  int getNumBands() const;
  int getFFTSize() const;

private:
  int fftSize;
  int numBands;

  // FFT working buffers
  float *realBuf;
  float *imagBuf;
  float *windowFunc;
  float *magnitudes;
  float *bands;

  // Cooley-Tukey in-place radix-2 FFT
  void fft(float *real, float *imag, int n);

  // Hann window to reduce spectral leakage
  void buildWindow();

  // Bit-reversal permutation
  int bitReverse(int x, int bits);
};

#endif
