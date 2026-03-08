#ifndef TRACK_H
#define TRACK_H

#include "AudioNode.h"
#include <string>

using namespace std;

// Concrete AudioNode representing a single audio track.
// Owns raw PCM sample data loaded from a WAV file.
// Demonstrates: dynamic memory (new[]/delete[]), raw pointers, references.
class Track : public AudioNode {
public:
  // Construct from metadata only (no audio loaded yet)
  Track(const string &title, const string &artist, const string &genre,
        const string &filePath);

  // Destructor — frees raw PCM data
  ~Track();

  // Load WAV file from disk, parse header, store samples as float*
  bool loadFromFile(const string &path);

  // AudioNode interface
  int process(float *buffer, int numFrames) override;
  const char *getType() const override;
  AudioNode *clone() const override;
  void reset() override;
  void print(ostream &os) const override;

  // Seek to a specific frame position
  void seek(int frame);

  // Getters (return by const reference to avoid copies)
  const string &getTitle() const;
  const string &getArtist() const;
  const string &getGenre() const;
  const string &getFilePath() const;
  int getTotalFrames() const;
  int getCursor() const;
  int getSampleRate() const;
  int getChannels() const;
  bool isLoaded() const;

private:
  string title;
  string artist;
  string genre;
  string filePath;

  float *pcmData;   // Raw PCM sample buffer (interleaved)
  int totalFrames;  // Total number of audio frames
  int totalSamples; // totalFrames * channels
  int cursor;       // Current playback position (frame index)
  int sampleRate;   // e.g. 44100
  int channels;     // 1 (mono) or 2 (stereo)
  bool loaded;      // Whether audio data has been successfully loaded
};

#endif
