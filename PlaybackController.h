#ifndef PLAYBACKCONTROLLER_H
#define PLAYBACKCONTROLLER_H

#include "AudioEngine.h"
#include "Playlist.h"
#include "TrackLibrary.h"
#include <string>


using namespace std;

// PlaybackController: terminal CLI for the music engine.
// Connects TrackLibrary, Playlist, and AudioEngine.
// Commands: play, pause, resume, stop, skip, prev, queue, list, search, volume,
// quit
class PlaybackController {
public:
  PlaybackController(TrackLibrary *library, AudioEngine *engine);
  ~PlaybackController();

  // Main command loop — runs until user types "quit"
  void run();

private:
  TrackLibrary *library; // Non-owning reference to the track library
  AudioEngine *engine;   // Non-owning reference to the audio engine
  Playlist *playlist;    // Owning pointer to the active playlist

  void printBanner() const;
  void printHelp() const;
  void printPrompt() const;
  void printNowPlaying() const;

  // Command handlers
  void cmdList() const;
  void cmdPlay(const string &args);
  void cmdPause();
  void cmdResume();
  void cmdStop();
  void cmdSkip();
  void cmdPrev();
  void cmdQueue(const string &args);
  void cmdSearch(const string &args) const;
  void cmdVolume(const string &args);
  void cmdNowPlaying() const;
};

#endif
