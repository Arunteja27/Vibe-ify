#ifndef SPECTRUMANALYZER_H
#define SPECTRUMANALYZER_H

#include <cmath>
#include <cstring>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Real-time FFT spectrum analyzer (Cooley-Tukey radix-2).
class SpectrumAnalyzer {
public:
  SpectrumAnalyzer(int fftSize = 1024, int numBands = 64);
  ~SpectrumAnalyzer();

  void analyze(const float *pcmBuffer, int numFrames);
  const float *getBands() const;
  int getNumBands() const;
  int getFFTSize() const;

private:
  int fftSize;
  int numBands;
  float *realBuf;
  float *imagBuf;
  float *windowFunc;
  float *magnitudes;
  float *bands;

  void fft(float *real, float *imag, int n);
  void buildWindow();
  int bitReverse(int x, int bits);
};

#endif
