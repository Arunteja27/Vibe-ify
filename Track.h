#ifndef TRACK_H
#define TRACK_H

#include "AudioNode.h"
#include <string>

// Concrete AudioNode — loads and plays a WAV file.
class Track : public AudioNode {
public:
  Track(const std::string &title, const std::string &artist,
        const std::string &genre, const std::string &filePath);
  ~Track();

  bool loadFromFile(const std::string &path);

  int process(float *buffer, int numFrames) override;
  const char *getType() const override;
  AudioNode *clone() const override;
  void reset() override;
  void print(std::ostream &os) const override;

  void seek(int frame);

  const std::string &getTitle() const;
  const std::string &getArtist() const;
  const std::string &getGenre() const;
  const std::string &getFilePath() const;
  int getTotalFrames() const;
  int getCursor() const;
  int getSampleRate() const;
  int getChannels() const;
  bool isLoaded() const;

private:
  std::string title;
  std::string artist;
  std::string genre;
  std::string filePath;

  float *pcmData;
  int totalFrames;
  int totalSamples;
  int cursor;
  int sampleRate;
  int channels;
  bool loaded;
};

#endif
