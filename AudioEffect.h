#ifndef AUDIOEFFECT_H
#define AUDIOEFFECT_H

#include "AudioNode.h"

// Base class — wraps a source AudioNode and transforms its output.
class AudioEffect : public AudioNode {
public:
  AudioEffect(AudioNode *source) : source(source) {}
  virtual ~AudioEffect() {}

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

#endif
