#include "SpectrumAnalyzer.h"

SpectrumAnalyzer::SpectrumAnalyzer(int fftSize, int numBands)
    : fftSize(fftSize), numBands(numBands) {
  realBuf = new float[fftSize];
  imagBuf = new float[fftSize];
  windowFunc = new float[fftSize];
  magnitudes = new float[fftSize / 2];
  bands = new float[numBands];

  memset(realBuf, 0, fftSize * sizeof(float));
  memset(imagBuf, 0, fftSize * sizeof(float));
  memset(magnitudes, 0, (fftSize / 2) * sizeof(float));
  memset(bands, 0, numBands * sizeof(float));

  buildWindow();
}

SpectrumAnalyzer::~SpectrumAnalyzer() {
  delete[] realBuf;
  delete[] imagBuf;
  delete[] windowFunc;
  delete[] magnitudes;
  delete[] bands;
}

void SpectrumAnalyzer::buildWindow() {
  // Hann window reduces spectral leakage
  for (int i = 0; i < fftSize; ++i)
    windowFunc[i] =
        0.5f * (1.0f - cosf(2.0f * (float)M_PI * i / (fftSize - 1)));
}

int SpectrumAnalyzer::bitReverse(int x, int bits) {
  int result = 0;
  for (int i = 0; i < bits; ++i) {
    result = (result << 1) | (x & 1);
    x >>= 1;
  }
  return result;
}

void SpectrumAnalyzer::fft(float *real, float *imag, int n) {
  // Determine number of bits for bit-reversal
  int bits = 0;
  int temp = n;
  while (temp > 1) {
    temp >>= 1;
    ++bits;
  }

  // Bit-reversal permutation
  for (int i = 0; i < n; ++i) {
    int j = bitReverse(i, bits);
    if (j > i) {
      float tr = real[i];
      real[i] = real[j];
      real[j] = tr;
      float ti = imag[i];
      imag[i] = imag[j];
      imag[j] = ti;
    }
  }

  // Cooley-Tukey butterfly stages
  for (int size = 2; size <= n; size *= 2) {
    int half = size / 2;
    float angle = -2.0f * (float)M_PI / size;

    for (int i = 0; i < n; i += size) {
      for (int k = 0; k < half; ++k) {
        float wr = cosf(angle * k);
        float wi = sinf(angle * k);

        int even = i + k;
        int odd = i + k + half;

        float tr = real[odd] * wr - imag[odd] * wi;
        float ti = real[odd] * wi + imag[odd] * wr;

        real[odd] = real[even] - tr;
        imag[odd] = imag[even] - ti;
        real[even] += tr;
        imag[even] += ti;
      }
    }
  }
}

void SpectrumAnalyzer::analyze(const float *pcmBuffer, int numFrames) {
  // Mix stereo to mono and apply Hann window
  int samples = (numFrames < fftSize) ? numFrames : fftSize;

  for (int i = 0; i < samples; ++i) {
    float mono = (pcmBuffer[i * 2] + pcmBuffer[i * 2 + 1]) * 0.5f;
    realBuf[i] = mono * windowFunc[i];
    imagBuf[i] = 0.0f;
  }

  // Zero-pad if buffer is smaller than FFT size
  for (int i = samples; i < fftSize; ++i) {
    realBuf[i] = 0.0f;
    imagBuf[i] = 0.0f;
  }

  // Run FFT
  fft(realBuf, imagBuf, fftSize);

  // Compute magnitudes (only first half — Nyquist)
  int halfFFT = fftSize / 2;
  float maxMag = 0.0001f;
  for (int i = 0; i < halfFFT; ++i) {
    magnitudes[i] = sqrtf(realBuf[i] * realBuf[i] + imagBuf[i] * imagBuf[i]);
    if (magnitudes[i] > maxMag)
      maxMag = magnitudes[i];
  }

  // Map FFT bins to frequency bands (logarithmic distribution)
  for (int b = 0; b < numBands; ++b) {
    // Log-scale bin mapping: lower bands get fewer bins, upper bands get more
    float t0 = (float)b / numBands;
    float t1 = (float)(b + 1) / numBands;

    // Exponential mapping for perceptually even band distribution
    int binStart = (int)(halfFFT * powf(t0, 2.0f));
    int binEnd = (int)(halfFFT * powf(t1, 2.0f));
    if (binEnd <= binStart)
      binEnd = binStart + 1;
    if (binEnd > halfFFT)
      binEnd = halfFFT;

    float sum = 0.0f;
    int count = 0;
    for (int i = binStart; i < binEnd; ++i) {
      sum += magnitudes[i];
      ++count;
    }

    bands[b] = (count > 0) ? (sum / count) / maxMag : 0.0f;
    if (bands[b] > 1.0f)
      bands[b] = 1.0f;
  }
}

const float *SpectrumAnalyzer::getBands() const { return bands; }
int SpectrumAnalyzer::getNumBands() const { return numBands; }
int SpectrumAnalyzer::getFFTSize() const { return fftSize; }
