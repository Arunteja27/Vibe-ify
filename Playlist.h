#ifndef PLAYLIST_H
#define PLAYLIST_H

#include "AudioNode.h"
#include "Track.h"

// Playlist: a queue of Track* references (non-owning).
// Derives from AudioNode so the AudioEngine can play it polymorphically.
// Demonstrates: polymorphism (Playlist is-a AudioNode), raw pointers.
class Playlist : public AudioNode {
public:
  Playlist(const string &name);
  ~Playlist();

  // Queue management
  bool enqueue(Track *track); // Add a track to the end of the queue
  Track *dequeue();           // Remove and return the front track
  Track *current() const;     // Get the currently playing track
  bool next();                // Advance to the next track
  bool prev();                // Go back to the previous track

  // Accessors
  const string &getName() const;
  int getSize() const;
  int getCurrentIndex() const;
  Track *getTrack(int index) const; // Access by index (bounds-checked)

  // AudioNode interface
  int process(float *buffer, int numFrames) override;
  const char *getType() const override;
  AudioNode *clone() const override;
  void reset() override;
  void print(ostream &os) const override;

private:
  string name;
  Track **tracks; // Raw pointer array of Track* (non-owning)
  int capacity;
  int size;
  int currentIndex; // Index of currently playing track

  void grow(); // Double capacity when full
};

#endif
