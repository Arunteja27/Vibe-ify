#ifndef AUDIOEFFECT_H
#define AUDIOEFFECT_H

#include "AudioNode.h"
#include <cmath>
#include <cstring>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Base class — wraps a source AudioNode and transforms its output.
class AudioEffect : public AudioNode {
public:
  AudioEffect(AudioNode *source) : source(source) {}
  virtual ~AudioEffect() {}
  AudioNode *getSource() const { return source; }
  void setSource(AudioNode *s) { source = s; }

protected:
  AudioNode *source;
};

// Scales all samples by a gain factor.
class VolumeEffect : public AudioEffect {
public:
  VolumeEffect(AudioNode *source, float gain);
  ~VolumeEffect();

  int process(float *buffer, int numFrames) override;
  const char *getType() const override;
  AudioNode *clone() const override;
  void reset() override;
  void print(std::ostream &os) const override;

  void setGain(float g);
  float getGain() const;

private:
  float gain;
};

// Applies linear fade-in or fade-out over a frame range.
class FadeEffect : public AudioEffect {
public:
  FadeEffect(AudioNode *source, bool fadeIn, int durationFrames);
  ~FadeEffect();

  int process(float *buffer, int numFrames) override;
  const char *getType() const override;
  AudioNode *clone() const override;
  void reset() override;
  void print(std::ostream &os) const override;

private:
  bool fadeIn;
  int durationFrames;
  int frameCounter;
};

// Adds delayed copies of the signal back into the output.
class EchoEffect : public AudioEffect {
public:
  EchoEffect(AudioNode *source, int delaySamples, float decay);
  ~EchoEffect();

  int process(float *buffer, int numFrames) override;
  const char *getType() const override;
  AudioNode *clone() const override;
  void reset() override;
  void print(std::ostream &os) const override;

  void setDecay(float d);
  float getDecay() const;

private:
  float *delayBuffer;
  int delaySize;
  int writePos;
  float decay;
};

// Multi-tap delay network for room simulation.
class ReverbEffect : public AudioEffect {
public:
  ReverbEffect(AudioNode *source, float roomSize, float wetDry);
  ~ReverbEffect();

  int process(float *buffer, int numFrames) override;
  const char *getType() const override;
  AudioNode *clone() const override;
  void reset() override;
  void print(std::ostream &os) const override;

  void setWetDry(float w);
  float getWetDry() const;

private:
  static const int NUM_TAPS = 4;
  float *tapBuffers[NUM_TAPS];
  int tapSizes[NUM_TAPS];
  int tapPositions[NUM_TAPS];
  float roomSize;
  float wetDry;
};

// Boosts low frequencies via single-pole low-pass filter.
class BassBoostEffect : public AudioEffect {
public:
  BassBoostEffect(AudioNode *source, float boostGain,
                  float cutoffFreq = 200.0f);
  ~BassBoostEffect();

  int process(float *buffer, int numFrames) override;
  const char *getType() const override;
  AudioNode *clone() const override;
  void reset() override;
  void print(std::ostream &os) const override;

  void setBoostGain(float g);
  float getBoostGain() const;

private:
  float boostGain;
  float cutoffFreq;
  float prevLeft;
  float prevRight;
  float alpha;
};

// Soft/hard clipping distortion.
class DistortionEffect : public AudioEffect {
public:
  DistortionEffect(AudioNode *source, float drive);
  ~DistortionEffect();

  int process(float *buffer, int numFrames) override;
  const char *getType() const override;
  AudioNode *clone() const override;
  void reset() override;
  void print(std::ostream &os) const override;

  void setDrive(float d);
  float getDrive() const;

private:
  float drive;
};

// Playback speed change via linear interpolation resampling.
class SpeedEffect : public AudioEffect {
public:
  SpeedEffect(AudioNode *source, float speedFactor);
  ~SpeedEffect();

  int process(float *buffer, int numFrames) override;
  const char *getType() const override;
  AudioNode *clone() const override;
  void reset() override;
  void print(std::ostream &os) const override;

  void setSpeed(float s);
  float getSpeed() const;

private:
  float speedFactor;
  float *resampleBuffer;
  int resampleBufferSize;
};

#endif
