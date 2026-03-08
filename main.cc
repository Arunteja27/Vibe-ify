#include "ApiController.h"
#include "AudioEngine.h"
#include "HttpServer.h"
#include "PlaybackController.h"
#include "StreamCache.h"
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
  _mkdir("media/cache");

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
  StreamCache *cache = new StreamCache();

  ApiController *apiCtrl = new ApiController(library, engine, youtube, cache);
  HttpServer *httpServer = new HttpServer(8080);
  httpServer->start(apiCtrl);

  PlaybackController *cli = new PlaybackController(library, engine);
  cli->run();

  httpServer->stop();
  delete httpServer;
  delete apiCtrl;
  delete cli;
  delete youtube;
  delete cache;
  engine->shutdown();
  delete engine;
  delete library;

  return 0;
}
