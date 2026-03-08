#include "AudioEffect.h"
#include <cstring>

using namespace std;

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
