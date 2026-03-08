#ifndef TRACKLIBRARY_H
#define TRACKLIBRARY_H

#include "Track.h"
#include <string>

using namespace std;

// TrackLibrary: owns all Track objects via a raw Track** array.
// Manual memory management — destructor walks the array and deletes each
// Track*. No STL containers; dynamic resize via raw pointer reallocation.
class TrackLibrary {
public:
  TrackLibrary();
  ~TrackLibrary();

  // Add a new track (library takes ownership of the pointer)
  bool addTrack(Track *track);

  // Remove a track by index (deletes it, shifts array)
  bool removeTrack(int index);

  // Search by artist (returns array of raw pointers + count via out-param)
  int findByArtist(const string &artist, Track **&results) const;

  // Search by title substring
  int findByTitle(const string &query, Track **&results) const;

  // Search by genre
  int findByGenre(const string &genre, Track **&results) const;

  // Accessors
  Track *getTrack(int index) const;
  int getSize() const;

  // Print library contents
  void print(ostream &os) const;

  // Load all WAV files from a directory
  int loadFromDirectory(const string &dirPath);

private:
  Track **tracks; // Raw owning pointer array
  int capacity;
  int size;

  void grow(); // Double capacity
};

#endif
