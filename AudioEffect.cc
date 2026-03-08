#include "AudioEffect.h"

using namespace std;

// ---- VolumeEffect ----

VolumeEffect::VolumeEffect(AudioNode *source, float gain)
    : AudioEffect(source), gain(gain) {}

VolumeEffect::~VolumeEffect() {}

int VolumeEffect::process(float *buffer, int numFrames) {
  if (!source)
    return 0;
  int framesRead = source->process(buffer, numFrames);
  float *ptr = buffer;
  int totalSamples = framesRead * 2;
  for (int i = 0; i < totalSamples; ++i)
    ptr[i] *= gain;
  return framesRead;
}

const char *VolumeEffect::getType() const { return "VolumeEffect"; }
AudioNode *VolumeEffect::clone() const {
  return new VolumeEffect(source, gain);
}
void VolumeEffect::reset() {
  if (source)
    source->reset();
}

void VolumeEffect::print(ostream &os) const {
  os << "[VolumeEffect] gain=" << gain;
  if (source) {
    os << " -> ";
    source->print(os);
  }
}

void VolumeEffect::setGain(float g) { gain = g; }
float VolumeEffect::getGain() const { return gain; }

// ---- FadeEffect ----

FadeEffect::FadeEffect(AudioNode *source, bool fadeIn, int durationFrames)
    : AudioEffect(source), fadeIn(fadeIn), durationFrames(durationFrames),
      frameCounter(0) {}

FadeEffect::~FadeEffect() {}

int FadeEffect::process(float *buffer, int numFrames) {
  if (!source)
    return 0;
  int framesRead = source->process(buffer, numFrames);
  float *ptr = buffer;
  for (int f = 0; f < framesRead; ++f) {
    float t;
    if (frameCounter < durationFrames) {
      t = (float)frameCounter / (float)durationFrames;
      if (!fadeIn)
        t = 1.0f - t;
    } else {
      t = fadeIn ? 1.0f : 0.0f;
    }
    ptr[f * 2] *= t;
    ptr[f * 2 + 1] *= t;
    ++frameCounter;
  }
  return framesRead;
}

const char *FadeEffect::getType() const { return "FadeEffect"; }
AudioNode *FadeEffect::clone() const {
  return new FadeEffect(source, fadeIn, durationFrames);
}

void FadeEffect::reset() {
  frameCounter = 0;
  if (source)
    source->reset();
}

void FadeEffect::print(ostream &os) const {
  os << "[FadeEffect] " << (fadeIn ? "fade-in" : "fade-out") << " over "
     << durationFrames << " frames";
  if (source) {
    os << " -> ";
    source->print(os);
  }
}

// ---- EchoEffect ----

EchoEffect::EchoEffect(AudioNode *source, int delaySamples, float decay)
    : AudioEffect(source), delaySize(delaySamples * 2), writePos(0),
      decay(decay) {
  delayBuffer = new float[delaySize];
  memset(delayBuffer, 0, delaySize * sizeof(float));
}

EchoEffect::~EchoEffect() { delete[] delayBuffer; }

int EchoEffect::process(float *buffer, int numFrames) {
  if (!source)
    return 0;
  int framesRead = source->process(buffer, numFrames);
  int totalSamples = framesRead * 2;

  for (int i = 0; i < totalSamples; ++i) {
    float delayed = delayBuffer[writePos];
    float output = buffer[i] + delayed * decay;
    delayBuffer[writePos] = buffer[i];
    buffer[i] = output;
    writePos = (writePos + 1) % delaySize;
  }

  return framesRead;
}

const char *EchoEffect::getType() const { return "EchoEffect"; }
AudioNode *EchoEffect::clone() const {
  return new EchoEffect(source, delaySize / 2, decay);
}

void EchoEffect::reset() {
  memset(delayBuffer, 0, delaySize * sizeof(float));
  writePos = 0;
  if (source)
    source->reset();
}

void EchoEffect::print(ostream &os) const {
  os << "[EchoEffect] delay=" << (delaySize / 2) << " decay=" << decay;
  if (source) {
    os << " -> ";
    source->print(os);
  }
}

void EchoEffect::setDecay(float d) { decay = d; }
float EchoEffect::getDecay() const { return decay; }

// ---- ReverbEffect ----

ReverbEffect::ReverbEffect(AudioNode *source, float roomSize, float wetDry)
    : AudioEffect(source), roomSize(roomSize), wetDry(wetDry) {
  // 4 delay taps at different prime-number intervals for diffusion
  int baseTaps[] = {1557, 1617, 1491, 1422}; // ~35-37ms at 44100Hz
  for (int t = 0; t < NUM_TAPS; ++t) {
    tapSizes[t] = (int)(baseTaps[t] * roomSize) * 2;
    if (tapSizes[t] < 2)
      tapSizes[t] = 2;
    tapBuffers[t] = new float[tapSizes[t]];
    memset(tapBuffers[t], 0, tapSizes[t] * sizeof(float));
    tapPositions[t] = 0;
  }
}

ReverbEffect::~ReverbEffect() {
  for (int t = 0; t < NUM_TAPS; ++t)
    delete[] tapBuffers[t];
}

int ReverbEffect::process(float *buffer, int numFrames) {
  if (!source)
    return 0;
  int framesRead = source->process(buffer, numFrames);
  int totalSamples = framesRead * 2;

  float tapDecays[] = {0.4f, 0.35f, 0.3f, 0.25f};

  for (int i = 0; i < totalSamples; ++i) {
    float wet = 0.0f;
    for (int t = 0; t < NUM_TAPS; ++t) {
      float delayed = tapBuffers[t][tapPositions[t]];
      wet += delayed * tapDecays[t];
      tapBuffers[t][tapPositions[t]] =
          buffer[i] + delayed * tapDecays[t] * 0.5f;
      tapPositions[t] = (tapPositions[t] + 1) % tapSizes[t];
    }
    buffer[i] = buffer[i] * (1.0f - wetDry) + wet * wetDry;
  }

  return framesRead;
}

const char *ReverbEffect::getType() const { return "ReverbEffect"; }
AudioNode *ReverbEffect::clone() const {
  return new ReverbEffect(source, roomSize, wetDry);
}

void ReverbEffect::reset() {
  for (int t = 0; t < NUM_TAPS; ++t) {
    memset(tapBuffers[t], 0, tapSizes[t] * sizeof(float));
    tapPositions[t] = 0;
  }
  if (source)
    source->reset();
}

void ReverbEffect::print(ostream &os) const {
  os << "[ReverbEffect] room=" << roomSize << " wet=" << wetDry;
  if (source) {
    os << " -> ";
    source->print(os);
  }
}

void ReverbEffect::setWetDry(float w) { wetDry = w; }
float ReverbEffect::getWetDry() const { return wetDry; }

// ---- BassBoostEffect ----

BassBoostEffect::BassBoostEffect(AudioNode *source, float boostGain,
                                 float cutoffFreq)
    : AudioEffect(source), boostGain(boostGain), cutoffFreq(cutoffFreq),
      prevLeft(0.0f), prevRight(0.0f) {
  // Single-pole low-pass filter coefficient
  float rc = 1.0f / (2.0f * (float)M_PI * cutoffFreq);
  float dt = 1.0f / 44100.0f;
  alpha = dt / (rc + dt);
}

BassBoostEffect::~BassBoostEffect() {}

int BassBoostEffect::process(float *buffer, int numFrames) {
  if (!source)
    return 0;
  int framesRead = source->process(buffer, numFrames);

  for (int f = 0; f < framesRead; ++f) {
    float left = buffer[f * 2];
    float right = buffer[f * 2 + 1];

    // Low-pass filter to extract bass frequencies
    prevLeft = prevLeft + alpha * (left - prevLeft);
    prevRight = prevRight + alpha * (right - prevRight);

    // Add amplified bass back to original signal
    buffer[f * 2] = left + prevLeft * boostGain;
    buffer[f * 2 + 1] = right + prevRight * boostGain;
  }

  return framesRead;
}

const char *BassBoostEffect::getType() const { return "BassBoostEffect"; }
AudioNode *BassBoostEffect::clone() const {
  return new BassBoostEffect(source, boostGain, cutoffFreq);
}

void BassBoostEffect::reset() {
  prevLeft = 0.0f;
  prevRight = 0.0f;
  if (source)
    source->reset();
}

void BassBoostEffect::print(ostream &os) const {
  os << "[BassBoostEffect] gain=" << boostGain << " cutoff=" << cutoffFreq
     << "Hz";
  if (source) {
    os << " -> ";
    source->print(os);
  }
}

void BassBoostEffect::setBoostGain(float g) { boostGain = g; }
float BassBoostEffect::getBoostGain() const { return boostGain; }

// ---- DistortionEffect ----

DistortionEffect::DistortionEffect(AudioNode *source, float drive)
    : AudioEffect(source), drive(drive) {}

DistortionEffect::~DistortionEffect() {}

int DistortionEffect::process(float *buffer, int numFrames) {
  if (!source)
    return 0;
  int framesRead = source->process(buffer, numFrames);
  int totalSamples = framesRead * 2;

  for (int i = 0; i < totalSamples; ++i) {
    float sample = buffer[i] * drive;
    // Soft clip via tanh
    buffer[i] = tanhf(sample);
  }

  return framesRead;
}

const char *DistortionEffect::getType() const { return "DistortionEffect"; }
AudioNode *DistortionEffect::clone() const {
  return new DistortionEffect(source, drive);
}

void DistortionEffect::reset() {
  if (source)
    source->reset();
}

void DistortionEffect::print(ostream &os) const {
  os << "[DistortionEffect] drive=" << drive;
  if (source) {
    os << " -> ";
    source->print(os);
  }
}

void DistortionEffect::setDrive(float d) { drive = d; }
float DistortionEffect::getDrive() const { return drive; }

// ---- SpeedEffect ----

SpeedEffect::SpeedEffect(AudioNode *source, float speedFactor)
    : AudioEffect(source), speedFactor(speedFactor), resampleBuffer(nullptr),
      resampleBufferSize(0) {}

SpeedEffect::~SpeedEffect() {
  if (resampleBuffer)
    delete[] resampleBuffer;
}

int SpeedEffect::process(float *buffer, int numFrames) {
  if (!source)
    return 0;

  // Read more or fewer frames from source based on speed
  int srcFrames = (int)(numFrames * speedFactor) + 1;

  // Ensure resample buffer is large enough
  int srcSamples = srcFrames * 2;
  if (srcSamples > resampleBufferSize) {
    if (resampleBuffer)
      delete[] resampleBuffer;
    resampleBufferSize = srcSamples;
    resampleBuffer = new float[resampleBufferSize];
  }

  memset(resampleBuffer, 0, srcSamples * sizeof(float));
  int framesRead = source->process(resampleBuffer, srcFrames);

  if (framesRead == 0)
    return 0;

  // Linear interpolation resampling
  int outFrames = (int)(framesRead / speedFactor);
  if (outFrames > numFrames)
    outFrames = numFrames;

  for (int f = 0; f < outFrames; ++f) {
    float srcPos = f * speedFactor;
    int idx = (int)srcPos;
    float frac = srcPos - idx;

    if (idx + 1 < framesRead) {
      buffer[f * 2] = resampleBuffer[idx * 2] * (1.0f - frac) +
                      resampleBuffer[(idx + 1) * 2] * frac;
      buffer[f * 2 + 1] = resampleBuffer[idx * 2 + 1] * (1.0f - frac) +
                          resampleBuffer[(idx + 1) * 2 + 1] * frac;
    } else {
      buffer[f * 2] = resampleBuffer[idx * 2];
      buffer[f * 2 + 1] = resampleBuffer[idx * 2 + 1];
    }
  }

  return outFrames;
}

const char *SpeedEffect::getType() const { return "SpeedEffect"; }
AudioNode *SpeedEffect::clone() const {
  return new SpeedEffect(source, speedFactor);
}

void SpeedEffect::reset() {
  if (source)
    source->reset();
}

void SpeedEffect::print(ostream &os) const {
  os << "[SpeedEffect] speed=" << speedFactor << "x";
  if (source) {
    os << " -> ";
    source->print(os);
  }
}

void SpeedEffect::setSpeed(float s) { speedFactor = s; }
float SpeedEffect::getSpeed() const { return speedFactor; }
