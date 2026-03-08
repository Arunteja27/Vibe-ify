#ifndef AUDIONODE_H
#define AUDIONODE_H

#include <iostream>
#include <string>

// Abstract base class for all audio-producing objects.
class AudioNode {
public:
  virtual ~AudioNode() {}

  virtual int process(float *buffer, int numFrames) = 0;
  virtual const char *getType() const = 0;
  virtual AudioNode *clone() const = 0;
  virtual void reset() = 0;
  virtual void print(std::ostream &os) const = 0;

  friend std::ostream &operator<<(std::ostream &os, const AudioNode &node) {
    node.print(os);
    return os;
  }
};

#endif
