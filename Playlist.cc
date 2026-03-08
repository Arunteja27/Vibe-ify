#include "Playlist.h"
#include <cstring>

using namespace std;

Playlist::Playlist(const string &name)
    : name(name), capacity(8), size(0), currentIndex(0) {
  tracks = new Track *[capacity];
  memset(tracks, 0, capacity * sizeof(Track *));
}

Playlist::~Playlist() {
  delete[] tracks;
  tracks = nullptr;
}

void Playlist::grow() {
  int newCap = capacity * 2;
  Track **newArr = new Track *[newCap];
  memset(newArr, 0, newCap * sizeof(Track *));
  for (int i = 0; i < size; ++i)
    newArr[i] = tracks[i];
  delete[] tracks;
  tracks = newArr;
  capacity = newCap;
}

bool Playlist::enqueue(Track *track) {
  if (!track)
    return false;
  if (size >= capacity)
    grow();
  tracks[size++] = track;
  return true;
}

Track *Playlist::dequeue() {
  if (size == 0)
    return nullptr;
  Track *front = tracks[0];
  for (int i = 1; i < size; ++i)
    tracks[i - 1] = tracks[i];
  tracks[--size] = nullptr;
  if (currentIndex > 0)
    --currentIndex;
  return front;
}

Track *Playlist::current() const {
  if (size == 0 || currentIndex < 0 || currentIndex >= size)
    return nullptr;
  return tracks[currentIndex];
}

bool Playlist::next() {
  if (currentIndex + 1 < size) {
    ++currentIndex;
    if (tracks[currentIndex])
      tracks[currentIndex]->reset();
    return true;
  }
  return false;
}

bool Playlist::prev() {
  if (currentIndex > 0) {
    --currentIndex;
    if (tracks[currentIndex])
      tracks[currentIndex]->reset();
    return true;
  }
  return false;
}

const string &Playlist::getName() const { return name; }
int Playlist::getSize() const { return size; }
int Playlist::getCurrentIndex() const { return currentIndex; }

Track *Playlist::getTrack(int index) const {
  if (index < 0 || index >= size)
    return nullptr;
  return tracks[index];
}

int Playlist::process(float *buffer, int numFrames) {
  Track *cur = current();
  if (!cur)
    return 0;

  int framesRead = cur->process(buffer, numFrames);
  if (framesRead == 0 && next())
    return process(buffer, numFrames);
  return framesRead;
}

const char *Playlist::getType() const { return "Playlist"; }

AudioNode *Playlist::clone() const {
  Playlist *copy = new Playlist(name);
  for (int i = 0; i < size; ++i)
    copy->enqueue(tracks[i]);
  copy->currentIndex = currentIndex;
  return copy;
}

void Playlist::reset() {
  currentIndex = 0;
  for (int i = 0; i < size; ++i)
    if (tracks[i])
      tracks[i]->reset();
}

void Playlist::print(ostream &os) const {
  os << "[Playlist] \"" << name << "\" (" << size << " tracks)";
  for (int i = 0; i < size; ++i) {
    os << "\n  ";
    if (i == currentIndex)
      os << ">> ";
    else
      os << "   ";
    os << (i + 1) << ". ";
    if (tracks[i])
      tracks[i]->print(os);
  }
}
