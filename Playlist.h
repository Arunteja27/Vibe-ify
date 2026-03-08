#ifndef PLAYLIST_H
#define PLAYLIST_H

#include "AudioNode.h"
#include "Track.h"

// Queue of Track* references (non-owning), playable as an AudioNode.
class Playlist : public AudioNode {
public:
  Playlist(const std::string &name);
  ~Playlist();

  bool enqueue(Track *track);
  Track *dequeue();
  Track *current() const;
  bool next();
  bool prev();

  const std::string &getName() const;
  int getSize() const;
  int getCurrentIndex() const;
  Track *getTrack(int index) const;

  int process(float *buffer, int numFrames) override;
  const char *getType() const override;
  AudioNode *clone() const override;
  void reset() override;
  void print(std::ostream &os) const override;

private:
  std::string name;
  Track **tracks;
  int capacity;
  int size;
  int currentIndex;

  void grow();
};

#endif
