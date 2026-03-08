#include "Track.h"
#include <cmath>
#include <cstring>
#include <fstream>


Track::Track(const string &title, const string &artist, const string &genre,
             const string &filePath)
    : title(title), artist(artist), genre(genre), filePath(filePath),
      pcmData(nullptr), totalFrames(0), totalSamples(0), cursor(0),
      sampleRate(44100), channels(2), loaded(false) {}

Track::~Track() {
  // Manual cleanup of raw PCM buffer
  if (pcmData) {
    delete[] pcmData;
    pcmData = nullptr;
  }
}

bool Track::loadFromFile(const string &path) {
  // Free any previously loaded data
  if (pcmData) {
    delete[] pcmData;
    pcmData = nullptr;
  }

  ifstream file(path, ios::binary);
  if (!file.is_open()) {
    cerr << "Track::loadFromFile: cannot open '" << path << "'" << endl;
    loaded = false;
    return false;
  }

  // ---- Parse WAV header ----
  char chunkId[4];
  file.read(chunkId, 4);
  if (memcmp(chunkId, "RIFF", 4) != 0) {
    cerr << "Track::loadFromFile: not a valid WAV (missing RIFF)" << endl;
    file.close();
    return false;
  }

  int chunkSize;
  file.read(reinterpret_cast<char *>(&chunkSize), 4);

  char format[4];
  file.read(format, 4);
  if (memcmp(format, "WAVE", 4) != 0) {
    cerr << "Track::loadFromFile: not a valid WAV (missing WAVE)" << endl;
    file.close();
    return false;
  }

  // Read subchunks until we find "fmt " and "data"
  short audioFormat = 0;
  short numChannels = 0;
  int sr = 0;
  int byteRate = 0;
  short blockAlign = 0;
  short bitsPerSample = 0;
  int dataSize = 0;
  char *rawData = nullptr;
  bool foundFmt = false;
  bool foundData = false;

  while (!foundData && file.good()) {
    char subchunkId[4];
    int subchunkSize;
    file.read(subchunkId, 4);
    file.read(reinterpret_cast<char *>(&subchunkSize), 4);

    if (memcmp(subchunkId, "fmt ", 4) == 0) {
      file.read(reinterpret_cast<char *>(&audioFormat), 2);
      file.read(reinterpret_cast<char *>(&numChannels), 2);
      file.read(reinterpret_cast<char *>(&sr), 4);
      file.read(reinterpret_cast<char *>(&byteRate), 4);
      file.read(reinterpret_cast<char *>(&blockAlign), 2);
      file.read(reinterpret_cast<char *>(&bitsPerSample), 2);
      // Skip any extra fmt bytes
      if (subchunkSize > 16) {
        file.seekg(subchunkSize - 16, ios::cur);
      }
      foundFmt = true;
    } else if (memcmp(subchunkId, "data", 4) == 0) {
      dataSize = subchunkSize;
      rawData = new char[dataSize];
      file.read(rawData, dataSize);
      foundData = true;
    } else {
      // Skip unknown subchunk
      file.seekg(subchunkSize, ios::cur);
    }
  }

  file.close();

  if (!foundFmt || !foundData || audioFormat != 1) {
    // Only support uncompressed PCM (audioFormat == 1)
    if (rawData)
      delete[] rawData;
    cerr << "Track::loadFromFile: unsupported WAV format" << endl;
    return false;
  }

  // ---- Convert raw bytes to float* PCM ----
  channels = numChannels;
  sampleRate = sr;
  int bytesPerSample = bitsPerSample / 8;
  totalSamples = dataSize / bytesPerSample;
  totalFrames = totalSamples / channels;

  pcmData = new float[totalSamples];

  if (bitsPerSample == 16) {
    short *samples16 = reinterpret_cast<short *>(rawData);
    for (int i = 0; i < totalSamples; ++i) {
      pcmData[i] = samples16[i] / 32768.0f;
    }
  } else if (bitsPerSample == 8) {
    unsigned char *samples8 = reinterpret_cast<unsigned char *>(rawData);
    for (int i = 0; i < totalSamples; ++i) {
      pcmData[i] = (samples8[i] - 128) / 128.0f;
    }
  } else if (bitsPerSample == 32) {
    int *samples32 = reinterpret_cast<int *>(rawData);
    for (int i = 0; i < totalSamples; ++i) {
      pcmData[i] = samples32[i] / 2147483648.0f;
    }
  }

  delete[] rawData;

  cursor = 0;
  loaded = true;
  return true;
}

int Track::process(float *buffer, int numFrames) {
  if (!loaded || !pcmData)
    return 0;

  int framesAvailable = totalFrames - cursor;
  if (framesAvailable <= 0)
    return 0;

  int framesToCopy =
      (numFrames < framesAvailable) ? numFrames : framesAvailable;
  int samplesToCopy = framesToCopy * channels;
  int srcOffset = cursor * channels;

  // Direct pointer-based copy for minimal CPU overhead
  float *dst = buffer;
  float *src = pcmData + srcOffset;
  for (int i = 0; i < samplesToCopy; ++i) {
    dst[i] = src[i];
  }

  // If source is mono but output expects stereo, duplicate channel
  if (channels == 1) {
    // Work backwards to avoid overwriting
    for (int f = framesToCopy - 1; f >= 0; --f) {
      buffer[f * 2 + 1] = buffer[f];
      buffer[f * 2] = buffer[f];
    }
  }

  cursor += framesToCopy;
  return framesToCopy;
}

const char *Track::getType() const { return "Track"; }

AudioNode *Track::clone() const {
  Track *copy = new Track(title, artist, genre, filePath);
  if (loaded && pcmData) {
    copy->totalFrames = totalFrames;
    copy->totalSamples = totalSamples;
    copy->sampleRate = sampleRate;
    copy->channels = channels;
    copy->cursor = 0;
    copy->loaded = true;
    copy->pcmData = new float[totalSamples];
    memcpy(copy->pcmData, pcmData, totalSamples * sizeof(float));
  }
  return copy;
}

void Track::reset() { cursor = 0; }

void Track::seek(int frame) {
  if (frame < 0)
    frame = 0;
  if (frame > totalFrames)
    frame = totalFrames;
  cursor = frame;
}

void Track::print(ostream &os) const {
  os << "[Track] \"" << title << "\" by " << artist;
  if (!genre.empty())
    os << " (" << genre << ")";
  if (loaded) {
    float durationSec = (float)totalFrames / sampleRate;
    int mins = (int)durationSec / 60;
    int secs = (int)durationSec % 60;
    os << " [" << mins << ":" << (secs < 10 ? "0" : "") << secs << "]";
  } else {
    os << " [not loaded]";
  }
}

const string &Track::getTitle() const { return title; }
const string &Track::getArtist() const { return artist; }
const string &Track::getGenre() const { return genre; }
const string &Track::getFilePath() const { return filePath; }
int Track::getTotalFrames() const { return totalFrames; }
int Track::getCursor() const { return cursor; }
int Track::getSampleRate() const { return sampleRate; }
int Track::getChannels() const { return channels; }
bool Track::isLoaded() const { return loaded; }
