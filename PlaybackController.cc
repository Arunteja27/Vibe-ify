#include "PlaybackController.h"
#include <cstdlib>
#include <sstream>

using namespace std;

PlaybackController::PlaybackController(TrackLibrary *library,
                                       AudioEngine *engine)
    : library(library), engine(engine) {
  playlist = new Playlist("Now Playing");
}

PlaybackController::~PlaybackController() {
  delete playlist;
  playlist = nullptr;
}

void PlaybackController::printBanner() const {
  cout << endl;
  cout << "  ╔══════════════════════════════════════╗" << endl;
  cout << "  ║     🎵  V I B E - I F Y  🎵         ║" << endl;
  cout << "  ║     Spotify Backend Clone             ║" << endl;
  cout << "  ║     Low-Latency C++ Music Engine      ║" << endl;
  cout << "  ╚══════════════════════════════════════╝" << endl;
  cout << endl;
}

void PlaybackController::printHelp() const {
  cout << "  Playback:" << endl;
  cout << "    list                 - Show all tracks" << endl;
  cout << "    play <id>            - Play track by ID" << endl;
  cout << "    pause                - Pause playback" << endl;
  cout << "    resume               - Resume playback" << endl;
  cout << "    stop                 - Stop playback" << endl;
  cout << "    skip                 - Skip to next track" << endl;
  cout << "    prev                 - Go to previous track" << endl;
  cout << "    queue <id>           - Add track to queue" << endl;
  cout << "    search <query>       - Search tracks" << endl;
  cout << "    volume <0-100>       - Set volume" << endl;
  cout << "    now                  - Now playing info" << endl;
  cout << endl;
  cout << "  Effects:" << endl;
  cout << "    echo <0-100>         - Echo/delay" << endl;
  cout << "    reverb <0-100>       - Reverb" << endl;
  cout << "    bass <0-100>         - Bass boost" << endl;
  cout << "    distort <0-100>      - Distortion" << endl;
  cout << "    speed <25-400>       - Playback speed %" << endl;
  cout << "    effects              - Show active effects" << endl;
  cout << "    clear                - Reset all effects" << endl;
  cout << "    spectrum             - Show frequency visualizer" << endl;
  cout << endl;
  cout << "  General:" << endl;
  cout << "    help                 - Show this help" << endl;
  cout << "    quit                 - Exit Vibe-ify" << endl;
  cout << endl;
}

void PlaybackController::printPrompt() const {
  cout << "vibeify> ";
  cout.flush();
}

void PlaybackController::printNowPlaying() const {
  if (engine->isPlaying()) {
    Track *cur = playlist->current();
    if (cur) {
      cout << "  ♪ Now Playing: ";
      cur->print(cout);
      cout << endl;
      cout << "  Volume: " << (int)(engine->getVolume() * 100) << "%" << endl;
      if (engine->isPaused())
        cout << "  [PAUSED]" << endl;
    }
  } else {
    cout << "  No track playing." << endl;
  }
}

void PlaybackController::run() {
  printBanner();
  cout << "  Loaded " << library->getSize() << " tracks." << endl;
  cout << "  Type 'help' for available commands." << endl;
  cout << endl;

  string line;
  while (true) {
    printPrompt();
    if (!getline(cin, line))
      break;

    size_t start = line.find_first_not_of(" \t\r\n");
    if (start == string::npos)
      continue;
    line = line.substr(start);
    size_t end = line.find_last_not_of(" \t\r\n");
    if (end != string::npos)
      line = line.substr(0, end + 1);
    if (line.empty())
      continue;

    string cmd, args;
    size_t spacePos = line.find(' ');
    if (spacePos != string::npos) {
      cmd = line.substr(0, spacePos);
      args = line.substr(spacePos + 1);
      start = args.find_first_not_of(" \t");
      if (start != string::npos)
        args = args.substr(start);
    } else {
      cmd = line;
    }

    for (int i = 0; i < (int)cmd.length(); ++i)
      if (cmd[i] >= 'A' && cmd[i] <= 'Z')
        cmd[i] += 32;

    if (cmd == "quit" || cmd == "exit" || cmd == "q") {
      engine->stop();
      cout << "  Shutting down Vibe-ify... Goodbye!" << endl;
      break;
    } else if (cmd == "help" || cmd == "h") {
      printHelp();
    } else if (cmd == "list" || cmd == "ls") {
      cmdList();
    } else if (cmd == "play") {
      cmdPlay(args);
    } else if (cmd == "pause") {
      cmdPause();
    } else if (cmd == "resume" || cmd == "unpause") {
      cmdResume();
    } else if (cmd == "stop") {
      cmdStop();
    } else if (cmd == "skip" || cmd == "next") {
      cmdSkip();
    } else if (cmd == "prev" || cmd == "back") {
      cmdPrev();
    } else if (cmd == "queue" || cmd == "add") {
      cmdQueue(args);
    } else if (cmd == "search" || cmd == "find") {
      cmdSearch(args);
    } else if (cmd == "volume" || cmd == "vol") {
      cmdVolume(args);
    } else if (cmd == "now" || cmd == "np") {
      cmdNowPlaying();
    } else if (cmd == "echo") {
      cmdEcho(args);
    } else if (cmd == "reverb") {
      cmdReverb(args);
    } else if (cmd == "bass") {
      cmdBass(args);
    } else if (cmd == "distort" || cmd == "distortion") {
      cmdDistort(args);
    } else if (cmd == "speed") {
      cmdSpeed(args);
    } else if (cmd == "effects" || cmd == "fx") {
      cmdEffects();
    } else if (cmd == "clear") {
      cmdClearEffects();
    } else if (cmd == "spectrum" || cmd == "fft") {
      cmdSpectrum();
    } else {
      cout << "  Unknown command: '" << cmd << "'. Type 'help' for commands."
           << endl;
    }
  }
}

void PlaybackController::cmdList() const { library->print(cout); }

void PlaybackController::cmdPlay(const string &args) {
  if (args.empty()) {
    if (engine->isPaused()) {
      cmdResume();
      return;
    }
    cout << "  Usage: play <track_id>" << endl;
    return;
  }

  int id = atoi(args.c_str());
  if (id < 1 || id > library->getSize()) {
    cout << "  Invalid track ID. Use 'list' to see available tracks." << endl;
    return;
  }

  Track *track = library->getTrack(id - 1);
  if (!track || !track->isLoaded()) {
    cout << "  Track not loaded." << endl;
    return;
  }

  delete playlist;
  playlist = new Playlist("Now Playing");
  track->reset();
  playlist->enqueue(track);

  if (engine->play(playlist)) {
    cout << "  ▶ Playing: ";
    track->print(cout);
    cout << endl;
  } else {
    cout << "  Error: could not start playback." << endl;
  }
}

void PlaybackController::cmdPause() {
  if (engine->isPlaying() && !engine->isPaused()) {
    engine->pause();
    cout << "  ⏸ Paused." << endl;
  } else {
    cout << "  Nothing to pause." << endl;
  }
}

void PlaybackController::cmdResume() {
  if (engine->isPaused()) {
    engine->resume();
    cout << "  ▶ Resumed." << endl;
  } else {
    cout << "  Nothing to resume." << endl;
  }
}

void PlaybackController::cmdStop() {
  engine->stop();
  cout << "  ⏹ Stopped." << endl;
}

void PlaybackController::cmdSkip() {
  if (playlist->next()) {
    Track *cur = playlist->current();
    if (cur) {
      cout << "  ⏭ Skipped to: ";
      cur->print(cout);
      cout << endl;
    }
  } else {
    cout << "  No more tracks in queue." << endl;
  }
}

void PlaybackController::cmdPrev() {
  if (playlist->prev()) {
    Track *cur = playlist->current();
    if (cur) {
      cout << "  ⏮ Previous: ";
      cur->print(cout);
      cout << endl;
    }
  } else {
    cout << "  Already at the first track." << endl;
  }
}

void PlaybackController::cmdQueue(const string &args) {
  if (args.empty()) {
    cout << "  Usage: queue <track_id>" << endl;
    return;
  }

  int id = atoi(args.c_str());
  if (id < 1 || id > library->getSize()) {
    cout << "  Invalid track ID." << endl;
    return;
  }

  Track *track = library->getTrack(id - 1);
  if (!track) {
    cout << "  Track not found." << endl;
    return;
  }

  track->reset();
  playlist->enqueue(track);
  cout << "  + Queued: ";
  track->print(cout);
  cout << " (" << playlist->getSize() << " in queue)" << endl;

  if (!engine->isPlaying()) {
    engine->play(playlist);
    cout << "  ▶ Started playback." << endl;
  }
}

void PlaybackController::cmdSearch(const string &args) const {
  if (args.empty()) {
    cout << "  Usage: search <query>" << endl;
    return;
  }

  Track **results = nullptr;
  int count = library->findByArtist(args, results);

  if (count == 0 && results) {
    delete[] results;
    count = library->findByTitle(args, results);
  }

  if (count == 0) {
    cout << "  No results for '" << args << "'." << endl;
  } else {
    cout << "  Found " << count << " result(s) for '" << args << "':" << endl;
    for (int i = 0; i < count; ++i) {
      for (int j = 0; j < library->getSize(); ++j) {
        if (library->getTrack(j) == results[i]) {
          cout << "    " << (j + 1) << ". ";
          break;
        }
      }
      results[i]->print(cout);
      cout << endl;
    }
  }

  if (results)
    delete[] results;
}

void PlaybackController::cmdVolume(const string &args) {
  if (args.empty()) {
    cout << "  Volume: " << (int)(engine->getVolume() * 100) << "%" << endl;
    return;
  }

  int vol = atoi(args.c_str());
  if (vol < 0)
    vol = 0;
  if (vol > 100)
    vol = 100;

  engine->setVolume(vol / 100.0f);
  cout << "  🔊 Volume: " << vol << "%" << endl;
}

void PlaybackController::cmdNowPlaying() const {
  printNowPlaying();
  if (playlist->getSize() > 1) {
    cout << "  Queue:" << endl;
    playlist->print(cout);
    cout << endl;
  }
}

void PlaybackController::cmdEcho(const string &args) {
  if (args.empty()) {
    cout << "  Echo: " << (int)(engine->getEcho() * 100) << "%" << endl;
    return;
  }
  int val = atoi(args.c_str());
  if (val < 0)
    val = 0;
  if (val > 100)
    val = 100;
  engine->setEcho(val / 100.0f);
  cout << "  🔊 Echo: " << val << "%" << (val == 0 ? " (off)" : "") << endl;
}

void PlaybackController::cmdReverb(const string &args) {
  if (args.empty()) {
    cout << "  Reverb: " << (int)(engine->getReverb() * 100) << "%" << endl;
    return;
  }
  int val = atoi(args.c_str());
  if (val < 0)
    val = 0;
  if (val > 100)
    val = 100;
  engine->setReverb(val / 100.0f);
  cout << "  🔊 Reverb: " << val << "%" << (val == 0 ? " (off)" : "") << endl;
}

void PlaybackController::cmdBass(const string &args) {
  if (args.empty()) {
    cout << "  Bass Boost: " << (int)(engine->getBassBoost() * 100) << "%"
         << endl;
    return;
  }
  int val = atoi(args.c_str());
  if (val < 0)
    val = 0;
  if (val > 100)
    val = 100;
  engine->setBassBoost(val / 100.0f);
  cout << "  🔊 Bass Boost: " << val << "%" << (val == 0 ? " (off)" : "")
       << endl;
}

void PlaybackController::cmdDistort(const string &args) {
  if (args.empty()) {
    cout << "  Distortion: " << (int)(engine->getDistortion() * 100) << "%"
         << endl;
    return;
  }
  int val = atoi(args.c_str());
  if (val < 0)
    val = 0;
  if (val > 100)
    val = 100;
  engine->setDistortion(val / 100.0f);
  cout << "  🔊 Distortion: " << val << "%" << (val == 0 ? " (off)" : "")
       << endl;
}

void PlaybackController::cmdSpeed(const string &args) {
  if (args.empty()) {
    cout << "  Speed: " << (int)(engine->getSpeed() * 100) << "%" << endl;
    return;
  }
  int val = atoi(args.c_str());
  if (val < 25)
    val = 25;
  if (val > 400)
    val = 400;
  engine->setSpeed(val / 100.0f);
  cout << "  🔊 Speed: " << val << "%" << endl;
}

void PlaybackController::cmdEffects() const {
  cout << "  Active Effects:" << endl;
  cout << "    Echo:       " << (int)(engine->getEcho() * 100) << "%" << endl;
  cout << "    Reverb:     " << (int)(engine->getReverb() * 100) << "%" << endl;
  cout << "    Bass Boost: " << (int)(engine->getBassBoost() * 100) << "%"
       << endl;
  cout << "    Distortion: " << (int)(engine->getDistortion() * 100) << "%"
       << endl;
  cout << "    Speed:      " << (int)(engine->getSpeed() * 100) << "%" << endl;
  cout << "    Volume:     " << (int)(engine->getVolume() * 100) << "%" << endl;
}

void PlaybackController::cmdClearEffects() {
  engine->clearEffects();
  cout << "  All effects cleared." << endl;
}

void PlaybackController::cmdSpectrum() const {
  const float *bands = engine->getSpectrumBands();
  int numBands = engine->getSpectrumNumBands();

  if (!bands || numBands == 0 || !engine->isPlaying()) {
    cout << "  No spectrum data (play a track first)." << endl;
    return;
  }

  // Print ASCII bar visualization (show 32 bands, 12 rows tall)
  int displayBands = 32;
  if (displayBands > numBands)
    displayBands = numBands;
  int step = numBands / displayBands;
  int height = 12;

  cout << endl;
  for (int row = height; row >= 1; --row) {
    float threshold = (float)row / height;
    cout << "  ";
    for (int b = 0; b < displayBands; ++b) {
      float val = bands[b * step];
      if (val >= threshold)
        cout << "██";
      else
        cout << "  ";
    }
    cout << endl;
  }
  cout << "  ";
  for (int b = 0; b < displayBands; ++b)
    cout << "──";
  cout << endl;
  cout << "  Low ────────────────────────── High" << endl;
  cout << endl;
}
