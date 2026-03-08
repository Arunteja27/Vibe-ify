#include "PlaybackController.h"
#include <cstdlib>
#include <sstream>


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
  cout << "  Commands:" << endl;
  cout << "    list                 - Show all tracks in library" << endl;
  cout << "    play <id>            - Play track by ID" << endl;
  cout << "    pause                - Pause playback" << endl;
  cout << "    resume               - Resume playback" << endl;
  cout << "    stop                 - Stop playback" << endl;
  cout << "    skip                 - Skip to next track" << endl;
  cout << "    prev                 - Go to previous track" << endl;
  cout << "    queue <id>           - Add track to playlist queue" << endl;
  cout << "    search <query>       - Search tracks by artist/title" << endl;
  cout << "    volume <0-100>       - Set playback volume" << endl;
  cout << "    now                  - Show now playing info" << endl;
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

    // Trim whitespace
    size_t start = line.find_first_not_of(" \t\r\n");
    if (start == string::npos)
      continue;
    line = line.substr(start);
    size_t end = line.find_last_not_of(" \t\r\n");
    if (end != string::npos)
      line = line.substr(0, end + 1);
    if (line.empty())
      continue;

    // Parse command and arguments
    string cmd, args;
    size_t spacePos = line.find(' ');
    if (spacePos != string::npos) {
      cmd = line.substr(0, spacePos);
      args = line.substr(spacePos + 1);
      // Trim args
      start = args.find_first_not_of(" \t");
      if (start != string::npos)
        args = args.substr(start);
    } else {
      cmd = line;
    }

    // Convert command to lowercase
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
    } else {
      cout << "  Unknown command: '" << cmd << "'. Type 'help' for commands."
           << endl;
    }
  }
}

void PlaybackController::cmdList() const { library->print(cout); }

void PlaybackController::cmdPlay(const string &args) {
  if (args.empty()) {
    // If paused, resume; otherwise show usage
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

  // Reset the playlist and start with this track
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

  // If not currently playing, start playback
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
      // Find the original index in the library
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
