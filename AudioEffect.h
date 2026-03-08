#ifndef AUDIOEFFECT_H
#define AUDIOEFFECT_H

#include "AudioNode.h"

// AudioEffect: wraps a source AudioNode* and transforms its output.
// Demonstrates polymorphic chaining — effects are AudioNodes themselves.
class AudioEffect : public AudioNode {
public:
  // Takes a non-owning pointer to the source node
  AudioEffect(AudioNode *source) : source(source) {}
  virtual ~AudioEffect() {}

protected:
  AudioNode *source; // Raw pointer to the upstream node (non-owning)
};

// VolumeEffect: scales all samples by a gain factor.
// In-place buffer math for minimal CPU overhead.
class VolumeEffect : public AudioEffect {
public:
  VolumeEffect(AudioNode *source, float gain);
  ~VolumeEffect();

  int process(float *buffer, int numFrames) override;
  const char *getType() const override;
  AudioNode *clone() const override;
  void reset() override;
  void print(ostream &os) const override;

  void setGain(float g);
  float getGain() const;

private:
  float gain; // 0.0 = silence, 1.0 = unity, >1.0 = boost
};

// FadeEffect: applies linear fade-in or fade-out over a frame range.
// Computed per-sample for smooth transitions.
class FadeEffect : public AudioEffect {
public:
  // fadeIn: if true, fades from 0→1; if false, fades from 1→0
  // durationFrames: number of frames over which the fade occurs
  FadeEffect(AudioNode *source, bool fadeIn, int durationFrames);
  ~FadeEffect();

  int process(float *buffer, int numFrames) override;
  const char *getType() const override;
  AudioNode *clone() const override;
  void reset() override;
  void print(ostream &os) const override;

private:
  bool fadeIn;
  int durationFrames;
  int frameCounter; // Tracks how many frames have been processed
};

#endif
