#include "AudioEngine.h"
#include "PlaybackController.h"
#include "TrackLibrary.h"
#include "WavGenerator.h"
#include <cstdlib>
#include <direct.h>
#include <iostream>
#include <string>


using namespace std;

// Generate demo WAV tracks if they don't exist yet
void generateDemoTracks() {
  // Create media/samples/ directory
  _mkdir("media");
  _mkdir("media/samples");

  struct DemoTrack {
    const char *filename;
    float freq;     // Base frequency in Hz
    float duration; // Seconds
  };

  // Musical note frequencies for recognizable tones
  DemoTrack demos[] = {
      {"Nirvana - Lithium.wav", 329.63f, 5.0f},           // E4
      {"Nirvana - In Bloom.wav", 349.23f, 5.0f},          // F4
      {"Nirvana - Drain You.wav", 392.00f, 5.0f},         // G4
      {"Weird Al - Foil.wav", 440.00f, 5.0f},             // A4
      {"Weird Al - Tacky.wav", 493.88f, 5.0f},            // B4
      {"Weird Al - Word Crimes.wav", 523.25f, 5.0f},      // C5
      {"The Dirty Nil - Pain.wav", 587.33f, 5.0f},        // D5
      {"The Dirty Nil - Always High.wav", 659.25f, 5.0f}, // E5
  };

  int numDemos = sizeof(demos) / sizeof(demos[0]);

  for (int i = 0; i < numDemos; ++i) {
    string path = "media/samples/" + string(demos[i].filename);

    // Check if file already exists
    ifstream test(path);
    if (test.good()) {
      test.close();
      continue;
    }

    cout << "  Generating: " << demos[i].filename << "..." << endl;
    WavGenerator::generateSineWav(path, demos[i].freq, demos[i].duration);
  }

  // Generate one chord track
  string chordPath = "media/samples/Vibe-ify - Demo Chord.wav";
  ifstream chordTest(chordPath);
  if (!chordTest.good()) {
    float chordFreqs[] = {261.63f, 329.63f, 392.00f}; // C major chord
    cout << "  Generating: Demo Chord..." << endl;
    WavGenerator::generateChordWav(chordPath, chordFreqs, 3, 6.0f);
  } else {
    chordTest.close();
  }
}

int main() {
  cout << "  Initializing Vibe-ify Music Engine..." << endl;

  // Generate demo tracks if needed
  generateDemoTracks();

  // Create the track library and load WAV files from media/samples/
  TrackLibrary *library = new TrackLibrary();
  int loaded = library->loadFromDirectory("media/samples");

  if (loaded == 0) {
    cerr << "  Error: No tracks found in media/samples/" << endl;
    delete library;
    return 1;
  }

  // Initialize the audio engine
  AudioEngine *engine = new AudioEngine();
  if (!engine->init(44100, 2, 4096)) {
    cerr << "  Error: Could not initialize audio engine." << endl;
    delete engine;
    delete library;
    return 1;
  }

  // Launch the CLI controller
  PlaybackController *controller = new PlaybackController(library, engine);
  controller->run();

  // Cleanup — explicit delete (showcasing manual memory management)
  delete controller;
  engine->shutdown();
  delete engine;
  delete library;

  return 0;
}