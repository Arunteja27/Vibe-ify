#ifndef PLAYBACKCONTROLLER_H
#define PLAYBACKCONTROLLER_H

#include "AudioEngine.h"
#include "Playlist.h"
#include "TrackLibrary.h"
#include <string>

// Terminal CLI for the music engine.
class PlaybackController {
public:
  PlaybackController(TrackLibrary *library, AudioEngine *engine);
  ~PlaybackController();

  void run();

private:
  TrackLibrary *library;
  AudioEngine *engine;
  Playlist *playlist;

  void printBanner() const;
  void printHelp() const;
  void printPrompt() const;
  void printNowPlaying() const;

  void cmdList() const;
  void cmdPlay(const std::string &args);
  void cmdPause();
  void cmdResume();
  void cmdStop();
  void cmdSkip();
  void cmdPrev();
  void cmdQueue(const std::string &args);
  void cmdSearch(const std::string &args) const;
  void cmdVolume(const std::string &args);
  void cmdNowPlaying() const;
  void cmdEcho(const std::string &args);
  void cmdReverb(const std::string &args);
  void cmdBass(const std::string &args);
  void cmdDistort(const std::string &args);
  void cmdSpeed(const std::string &args);
  void cmdEffects() const;
  void cmdClearEffects();
  void cmdSpectrum() const;
};

#endif
