#include "TrackLibrary.h"
#include <cstring>
#include <dirent.h>

TrackLibrary::TrackLibrary() : capacity(16), size(0) {
  tracks = new Track *[capacity];
  memset(tracks, 0, capacity * sizeof(Track *));
}

TrackLibrary::~TrackLibrary() {
  // Owning — walk the array and delete every Track
  for (int i = 0; i < size; ++i) {
    delete tracks[i];
    tracks[i] = nullptr;
  }
  delete[] tracks;
  tracks = nullptr;
}

void TrackLibrary::grow() {
  int newCap = capacity * 2;
  Track **newArr = new Track *[newCap];
  memset(newArr, 0, newCap * sizeof(Track *));

  for (int i = 0; i < size; ++i) {
    newArr[i] = tracks[i];
  }

  delete[] tracks;
  tracks = newArr;
  capacity = newCap;
}

bool TrackLibrary::addTrack(Track *track) {
  if (!track)
    return false;
  if (size >= capacity)
    grow();
  tracks[size++] = track;
  return true;
}

bool TrackLibrary::removeTrack(int index) {
  if (index < 0 || index >= size)
    return false;

  delete tracks[index];

  // Shift remaining pointers
  for (int i = index; i < size - 1; ++i) {
    tracks[i] = tracks[i + 1];
  }
  tracks[--size] = nullptr;
  return true;
}

int TrackLibrary::findByArtist(const string &artist, Track **&results) const {
  // Caller must delete[] the results array (not the Track objects)
  results = new Track *[size];
  int count = 0;

  for (int i = 0; i < size; ++i) {
    // Case-insensitive substring match
    string trackArtist = tracks[i]->getArtist();
    string query = artist;

    // Convert both to lowercase for comparison
    for (int j = 0; j < (int)trackArtist.length(); ++j)
      if (trackArtist[j] >= 'A' && trackArtist[j] <= 'Z')
        trackArtist[j] += 32;
    for (int j = 0; j < (int)query.length(); ++j)
      if (query[j] >= 'A' && query[j] <= 'Z')
        query[j] += 32;

    if (trackArtist.find(query) != string::npos) {
      results[count++] = tracks[i];
    }
  }
  return count;
}

int TrackLibrary::findByTitle(const string &query, Track **&results) const {
  results = new Track *[size];
  int count = 0;

  for (int i = 0; i < size; ++i) {
    string trackTitle = tracks[i]->getTitle();
    string q = query;

    for (int j = 0; j < (int)trackTitle.length(); ++j)
      if (trackTitle[j] >= 'A' && trackTitle[j] <= 'Z')
        trackTitle[j] += 32;
    for (int j = 0; j < (int)q.length(); ++j)
      if (q[j] >= 'A' && q[j] <= 'Z')
        q[j] += 32;

    if (trackTitle.find(q) != string::npos) {
      results[count++] = tracks[i];
    }
  }
  return count;
}

int TrackLibrary::findByGenre(const string &genre, Track **&results) const {
  results = new Track *[size];
  int count = 0;

  for (int i = 0; i < size; ++i) {
    string trackGenre = tracks[i]->getGenre();
    string q = genre;

    for (int j = 0; j < (int)trackGenre.length(); ++j)
      if (trackGenre[j] >= 'A' && trackGenre[j] <= 'Z')
        trackGenre[j] += 32;
    for (int j = 0; j < (int)q.length(); ++j)
      if (q[j] >= 'A' && q[j] <= 'Z')
        q[j] += 32;

    if (trackGenre.find(q) != string::npos) {
      results[count++] = tracks[i];
    }
  }
  return count;
}

Track *TrackLibrary::getTrack(int index) const {
  if (index < 0 || index >= size)
    return nullptr;
  return tracks[index];
}

int TrackLibrary::getSize() const { return size; }

void TrackLibrary::print(ostream &os) const {
  os << "=== Track Library (" << size << " tracks) ===" << endl;
  for (int i = 0; i < size; ++i) {
    os << "  " << (i + 1) << ". ";
    tracks[i]->print(os);
    os << endl;
  }
}

int TrackLibrary::loadFromDirectory(const string &dirPath) {
  int loaded = 0;
  DIR *dir = opendir(dirPath.c_str());
  if (!dir) {
    cerr << "TrackLibrary::loadFromDirectory: cannot open '" << dirPath << "'"
         << endl;
    return 0;
  }

  struct dirent *entry;
  while ((entry = readdir(dir)) != nullptr) {
    string filename = entry->d_name;

    // Only process .wav files
    if (filename.length() < 5)
      continue;
    string ext = filename.substr(filename.length() - 4);
    for (int i = 0; i < 4; ++i)
      if (ext[i] >= 'A' && ext[i] <= 'Z')
        ext[i] += 32;
    if (ext != ".wav")
      continue;

    string fullPath = dirPath + "/" + filename;

    // Extract title from filename (remove extension)
    string title = filename.substr(0, filename.length() - 4);

    // Replace underscores with spaces for display
    for (int i = 0; i < (int)title.length(); ++i) {
      if (title[i] == '_')
        title[i] = ' ';
    }

    // Parse artist and title from "Artist - Title.wav" format
    string artist = "Unknown";
    string trackTitle = title;
    size_t dashPos = title.find(" - ");
    if (dashPos != string::npos) {
      artist = title.substr(0, dashPos);
      trackTitle = title.substr(dashPos + 3);
    }

    Track *track = new Track(trackTitle, artist, "Audio", fullPath);
    if (track->loadFromFile(fullPath)) {
      addTrack(track);
      ++loaded;
    } else {
      delete track;
    }
  }

  closedir(dir);
  return loaded;
}
