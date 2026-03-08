#include "AudioEngine.h"
#include "PlaybackController.h"
#include "TrackLibrary.h"
#include <cstdlib>
#include <direct.h>
#include <iostream>
#include <string>

using namespace std;

int main() {
  SetConsoleOutputCP(65001);
  cout << "  Initializing Vibe-ify Music Engine..." << endl;

  _mkdir("media");
  _mkdir("media/samples");

  TrackLibrary *library = new TrackLibrary();
  int loaded = library->loadFromDirectory("media/samples");

  if (loaded == 0) {
    cerr << "  Error: No tracks found in media/samples/" << endl;
    cerr << "  Place .wav files in media/samples/ (format: Artist - Title.wav)"
         << endl;
    delete library;
    return 1;
  }

  AudioEngine *engine = new AudioEngine();
  if (!engine->init(44100, 2, 4096)) {
    cerr << "  Error: Could not initialize audio engine." << endl;
    delete engine;
    delete library;
    return 1;
  }

  PlaybackController *controller = new PlaybackController(library, engine);
  controller->run();

  delete controller;
  engine->shutdown();
  delete engine;
  delete library;

  return 0;
}
