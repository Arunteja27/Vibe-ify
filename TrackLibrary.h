#ifndef TRACKLIBRARY_H
#define TRACKLIBRARY_H

#include "Track.h"
#include <string>

// Owns all Track objects via a raw Track** array with manual memory management.
class TrackLibrary {
public:
  TrackLibrary();
  ~TrackLibrary();

  bool addTrack(Track *track);
  bool removeTrack(int index);

  int findByArtist(const std::string &artist, Track **&results) const;
  int findByTitle(const std::string &query, Track **&results) const;
  int findByGenre(const std::string &genre, Track **&results) const;

  Track *getTrack(int index) const;
  int getSize() const;

  void print(std::ostream &os) const;
  int loadFromDirectory(const std::string &dirPath);

private:
  Track **tracks;
  int capacity;
  int size;

  void grow();
};

#endif
