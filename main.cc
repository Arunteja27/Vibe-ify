#include "ApiController.h"
#include "AudioEngine.h"
#include "HttpServer.h"
#include "PlaybackController.h"
#include "TrackLibrary.h"
#include "YouTubeSource.h"
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

  if (loaded == 0)
    cerr << "  Warning: No tracks in media/samples/ (use yt-search to find "
            "music)"
         << endl;

  AudioEngine *engine = new AudioEngine();
  if (!engine->init(44100, 2, 4096)) {
    cerr << "  Error: Could not initialize audio engine." << endl;
    delete engine;
    delete library;
    return 1;
  }

  YouTubeSource *youtube = new YouTubeSource();

  ApiController *apiCtrl = new ApiController(library, engine, youtube);
  HttpServer *httpServer = new HttpServer(8080);
  httpServer->start(apiCtrl);

  PlaybackController *cli = new PlaybackController(library, engine);
  cli->run();

  httpServer->stop();
  delete httpServer;
  delete apiCtrl;
  delete cli;
  delete youtube;
  engine->shutdown();
  delete engine;
  delete library;

  return 0;
}
