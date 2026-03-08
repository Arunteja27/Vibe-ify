#ifndef APICONTROLLER_H
#define APICONTROLLER_H

#include "AudioEngine.h"
#include "TrackLibrary.h"
#include "YouTubeSource.h"
#include <string>

// Maps HTTP endpoints to engine commands with JSON I/O.
class ApiController {
public:
  ApiController(TrackLibrary *library, AudioEngine *engine,
                YouTubeSource *youtube);
  ~ApiController();

  std::string handleRequest(const std::string &method, const std::string &path,
                            const std::string &body);

private:
  TrackLibrary *library;
  AudioEngine *engine;
  YouTubeSource *youtube;
  Track *currentApiTrack;
  YouTubeResult lastResults[5];
  int lastResultCount;

  std::string getTracks();
  std::string getNowPlaying();
  std::string getSpectrum();
  std::string getEffects();
  std::string getQueue();

  std::string postPlay(const std::string &body);
  std::string postPause();
  std::string postResume();
  std::string postStop();
  std::string postVolume(const std::string &body);
  std::string postEffects(const std::string &body);
  std::string postSearch(const std::string &body);
  std::string postYtSearch(const std::string &body);
  std::string postYtPlay(const std::string &body);
  std::string postQueue(const std::string &body);

  std::string extractJsonValue(const std::string &json, const std::string &key);
  std::string escapeJson(const std::string &str);
};

#endif
