#include "ApiController.h"
#include "Playlist.h"
#include <cstdlib>
#include <fstream>
#include <sstream>

using namespace std;

ApiController::ApiController(TrackLibrary *library, AudioEngine *engine,
                             YouTubeSource *youtube, StreamCache *cache)
    : library(library), engine(engine), youtube(youtube), cache(cache),
      currentApiTrack(nullptr), lastResultCount(0) {}

ApiController::~ApiController() {}

string ApiController::handleRequest(const string &method, const string &path,
                                    const string &body) {
  if (method == "GET") {
    if (path == "/api/tracks")
      return getTracks();
    if (path == "/api/now")
      return getNowPlaying();
    if (path == "/api/spectrum")
      return getSpectrum();
    if (path == "/api/effects")
      return getEffects();
    if (path == "/api/queue")
      return getQueue();
    if (path == "/api/status")
      return getNowPlaying();
  }

  if (method == "POST") {
    if (path == "/api/play")
      return postPlay(body);
    if (path == "/api/pause")
      return postPause();
    if (path == "/api/resume")
      return postResume();
    if (path == "/api/stop")
      return postStop();
    if (path == "/api/volume")
      return postVolume(body);
    if (path == "/api/effects")
      return postEffects(body);
    if (path == "/api/search")
      return postSearch(body);
    if (path == "/api/yt/search")
      return postYtSearch(body);
    if (path == "/api/yt/play")
      return postYtPlay(body);
    if (path == "/api/queue")
      return postQueue(body);
  }

  return "{\"error\": \"Not found\"}";
}

string ApiController::extractJsonValue(const string &json, const string &key) {
  string search = "\"" + key + "\"";
  size_t pos = json.find(search);
  if (pos == string::npos)
    return "";

  pos = json.find(":", pos);
  if (pos == string::npos)
    return "";
  ++pos;

  while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t'))
    ++pos;
  if (pos >= json.size())
    return "";

  if (json[pos] == '"') {
    size_t start = pos + 1;
    size_t end = json.find('"', start);
    if (end == string::npos)
      return "";
    return json.substr(start, end - start);
  }

  size_t start = pos;
  while (pos < json.size() && json[pos] != ',' && json[pos] != '}' &&
         json[pos] != ' ')
    ++pos;
  return json.substr(start, pos - start);
}

string ApiController::escapeJson(const string &str) {
  string result;
  for (size_t i = 0; i < str.size(); ++i) {
    char c = str[i];
    if (c == '"')
      result += "\\\"";
    else if (c == '\\')
      result += "\\\\";
    else if (c == '\n')
      result += "\\n";
    else
      result += c;
  }
  return result;
}

string ApiController::getTracks() {
  ostringstream json;
  json << "{\"tracks\": [";
  for (int i = 0; i < library->getSize(); ++i) {
    Track *t = library->getTrack(i);
    if (i > 0)
      json << ",";
    json << "{\"id\":" << (i + 1) << ",\"title\":\""
         << escapeJson(t->getTitle()) << "\""
         << ",\"artist\":\"" << escapeJson(t->getArtist()) << "\""
         << ",\"genre\":\"" << escapeJson(t->getGenre()) << "\""
         << ",\"frames\":" << t->getTotalFrames()
         << ",\"sampleRate\":" << t->getSampleRate()
         << ",\"loaded\":" << (t->isLoaded() ? "true" : "false") << "}";
  }
  json << "],\"count\":" << library->getSize() << "}";
  return json.str();
}

string ApiController::getNowPlaying() {
  ostringstream json;
  json << "{\"playing\":" << (engine->isPlaying() ? "true" : "false")
       << ",\"paused\":" << (engine->isPaused() ? "true" : "false")
       << ",\"volume\":" << (int)(engine->getVolume() * 100);

  if (currentApiTrack) {
    json << ",\"title\":\"" << escapeJson(currentApiTrack->getTitle()) << "\""
         << ",\"artist\":\"" << escapeJson(currentApiTrack->getArtist()) << "\""
         << ",\"cursor\":" << currentApiTrack->getCursor()
         << ",\"totalFrames\":" << currentApiTrack->getTotalFrames();
  }

  json << "}";
  return json.str();
}

string ApiController::getSpectrum() {
  const float *bands = engine->getSpectrumBands();
  int numBands = engine->getSpectrumNumBands();

  ostringstream json;
  json << "{\"bands\":[";
  if (bands && numBands > 0) {
    for (int i = 0; i < numBands; ++i) {
      if (i > 0)
        json << ",";
      json << (int)(bands[i] * 1000) / 1000.0f;
    }
  }
  json << "],\"count\":" << numBands << "}";
  return json.str();
}

string ApiController::getEffects() {
  ostringstream json;
  json << "{\"echo\":" << (int)(engine->getEcho() * 100)
       << ",\"reverb\":" << (int)(engine->getReverb() * 100)
       << ",\"bass\":" << (int)(engine->getBassBoost() * 100)
       << ",\"distortion\":" << (int)(engine->getDistortion() * 100)
       << ",\"speed\":" << (int)(engine->getSpeed() * 100)
       << ",\"volume\":" << (int)(engine->getVolume() * 100) << "}";
  return json.str();
}

string ApiController::getQueue() { return "{\"queue\": \"not implemented\"}"; }

string ApiController::postPlay(const string &body) {
  string idStr = extractJsonValue(body, "id");
  if (idStr.empty())
    return "{\"error\": \"missing id\"}";

  int id = atoi(idStr.c_str()) - 1;
  if (id < 0 || id >= library->getSize())
    return "{\"error\": \"invalid track id\"}";

  Track *track = library->getTrack(id);
  if (!track || !track->isLoaded())
    return "{\"error\": \"track not loaded\"}";

  track->reset();
  currentApiTrack = track;

  Playlist *pl = new Playlist("API");
  pl->enqueue(track);

  if (engine->play(pl)) {
    ostringstream json;
    json << "{\"status\":\"playing\""
         << ",\"title\":\"" << escapeJson(track->getTitle()) << "\""
         << ",\"artist\":\"" << escapeJson(track->getArtist()) << "\"}";
    return json.str();
  }

  delete pl;
  return "{\"error\": \"playback failed\"}";
}

string ApiController::postPause() {
  engine->pause();
  return "{\"status\":\"paused\"}";
}

string ApiController::postResume() {
  engine->resume();
  return "{\"status\":\"playing\"}";
}

string ApiController::postStop() {
  engine->stop();
  currentApiTrack = nullptr;
  return "{\"status\":\"stopped\"}";
}

string ApiController::postVolume(const string &body) {
  string levelStr = extractJsonValue(body, "level");
  if (levelStr.empty())
    return "{\"error\": \"missing level\"}";

  int level = atoi(levelStr.c_str());
  if (level < 0)
    level = 0;
  if (level > 100)
    level = 100;

  engine->setVolume(level / 100.0f);

  ostringstream json;
  json << "{\"volume\":" << level << "}";
  return json.str();
}

string ApiController::postEffects(const string &body) {
  string val;

  val = extractJsonValue(body, "echo");
  if (!val.empty())
    engine->setEcho(atoi(val.c_str()) / 100.0f);

  val = extractJsonValue(body, "reverb");
  if (!val.empty())
    engine->setReverb(atoi(val.c_str()) / 100.0f);

  val = extractJsonValue(body, "bass");
  if (!val.empty())
    engine->setBassBoost(atoi(val.c_str()) / 100.0f);

  val = extractJsonValue(body, "distortion");
  if (!val.empty())
    engine->setDistortion(atoi(val.c_str()) / 100.0f);

  val = extractJsonValue(body, "speed");
  if (!val.empty())
    engine->setSpeed(atoi(val.c_str()) / 100.0f);

  val = extractJsonValue(body, "clear");
  if (!val.empty() && val == "true")
    engine->clearEffects();

  return getEffects();
}

string ApiController::postSearch(const string &body) {
  string query = extractJsonValue(body, "query");
  if (query.empty())
    return "{\"error\": \"missing query\"}";

  Track **results = nullptr;
  int count = library->findByTitle(query, results);
  if (count == 0 && results) {
    delete[] results;
    results = nullptr;
  }
  if (count == 0)
    count = library->findByArtist(query, results);

  ostringstream json;
  json << "{\"results\":[";
  for (int i = 0; i < count; ++i) {
    if (i > 0)
      json << ",";
    int idx = -1;
    for (int j = 0; j < library->getSize(); ++j) {
      if (library->getTrack(j) == results[i]) {
        idx = j;
        break;
      }
    }
    json << "{\"id\":" << (idx + 1) << ",\"title\":\""
         << escapeJson(results[i]->getTitle()) << "\""
         << ",\"artist\":\"" << escapeJson(results[i]->getArtist()) << "\"}";
  }
  json << "],\"count\":" << count << "}";

  if (results)
    delete[] results;
  return json.str();
}

string ApiController::postYtSearch(const string &body) {
  string query = extractJsonValue(body, "query");
  if (query.empty())
    return "{\"error\": \"missing query\"}";

  if (!youtube->isAvailable())
    return "{\"error\": \"yt-dlp or ffmpeg not installed\"}";

  lastResultCount = youtube->search(query, lastResults, 5);

  ostringstream json;
  json << "{\"results\":[";
  for (int i = 0; i < lastResultCount; ++i) {
    if (i > 0)
      json << ",";
    json << "{\"index\":" << (i + 1) << ",\"title\":\""
         << escapeJson(lastResults[i].title) << "\""
         << ",\"url\":\"" << escapeJson(lastResults[i].url) << "\""
         << ",\"videoId\":\"" << escapeJson(lastResults[i].videoId) << "\""
         << ",\"channel\":\"" << escapeJson(lastResults[i].channel) << "\""
         << ",\"duration\":" << lastResults[i].durationSec << "}";
  }
  json << "],\"count\":" << lastResultCount << "}";
  return json.str();
}

string ApiController::postYtPlay(const string &body) {
  string idxStr = extractJsonValue(body, "index");
  if (idxStr.empty())
    return "{\"error\": \"missing index\"}";

  int idx = atoi(idxStr.c_str()) - 1;
  if (idx < 0 || idx >= lastResultCount)
    return "{\"error\": \"invalid index, search first\"}";

  YouTubeResult &result = lastResults[idx];

  string cachedPath = cache->getCachedPath(result.videoId);
  if (cachedPath.empty()) {
    string outputPath = cache->store(result.videoId);
    if (!youtube->download(result.url, outputPath))
      return "{\"error\": \"download failed\"}";
    cachedPath = outputPath;
  }

  string artist, title;
  YouTubeSource::parseTitle(result.title, artist, title);

  Track *track = new Track(title, artist, "YouTube", cachedPath);
  if (!track->isLoaded()) {
    if (!track->loadFromFile(cachedPath)) {
      delete track;
      return "{\"error\": \"failed to load audio\"}";
    }
  }

  string samplePath = "media/samples/" + artist + " - " + title + ".wav";
  ifstream checkSample(samplePath);
  if (!checkSample.good()) {
    ifstream src(cachedPath, ios::binary);
    ofstream dst(samplePath, ios::binary);
    if (src.good() && dst.good())
      dst << src.rdbuf();
  }

  library->addTrack(track);
  currentApiTrack = track;

  Playlist *pl = new Playlist("API-YT");
  pl->enqueue(track);
  engine->play(pl);

  ostringstream json;
  json << "{\"status\":\"playing\""
       << ",\"title\":\"" << escapeJson(title) << "\""
       << ",\"artist\":\"" << escapeJson(artist) << "\"}";
  return json.str();
}

string ApiController::postQueue(const string &body) {
  string idStr = extractJsonValue(body, "id");
  if (idStr.empty())
    return "{\"error\": \"missing id\"}";

  int id = atoi(idStr.c_str()) - 1;
  if (id < 0 || id >= library->getSize())
    return "{\"error\": \"invalid track id\"}";

  return "{\"status\": \"queued\", \"id\":" + to_string(id + 1) + "}";
}
