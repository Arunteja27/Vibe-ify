#include "YouTubeSource.h"
#include <cstdio>
#include <cstring>
#include <iostream>
#include <sstream>

using namespace std;

YouTubeSource::YouTubeSource() {
  // Check if yt-dlp is in PATH
  string check = runCommand("where yt-dlp 2>nul");
  ytdlpAvailable = !check.empty();

  check = runCommand("where ffmpeg 2>nul");
  ffmpegAvailable = !check.empty();
}

YouTubeSource::~YouTubeSource() {}

bool YouTubeSource::isAvailable() const {
  return ytdlpAvailable && ffmpegAvailable;
}

string YouTubeSource::runCommand(const string &cmd) {
  string result;
  FILE *pipe = _popen(cmd.c_str(), "r");
  if (!pipe)
    return result;

  char buffer[256];
  while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
    result += buffer;

  _pclose(pipe);
  return result;
}

int YouTubeSource::search(const string &query, YouTubeResult *results,
                          int maxResults) {
  if (!ytdlpAvailable)
    return 0;

  // Use yt-dlp to search YouTube and get JSON output
  string cmd = "yt-dlp --flat-playlist --dump-single-json \"ytsearch" +
               to_string(maxResults) + ":" + query + "\" 2>nul";

  string output = runCommand(cmd);
  if (output.empty())
    return 0;

  // Simple JSON parsing (no library — manually extract fields)
  int count = 0;
  size_t pos = 0;

  while (count < maxResults) {
    // Find next entry block
    pos = output.find("\"url\":", pos);
    if (pos == string::npos)
      break;

    YouTubeResult &r = results[count];

    // Extract URL
    size_t urlStart = output.find("\"", pos + 6) + 1;
    size_t urlEnd = output.find("\"", urlStart);
    if (urlStart != string::npos && urlEnd != string::npos)
      r.url = output.substr(urlStart, urlEnd - urlStart);

    // Extract video ID from URL or find "id" field nearby
    size_t idPos = output.find("\"id\":", pos);
    if (idPos != string::npos && idPos < pos + 500) {
      size_t idStart = output.find("\"", idPos + 5) + 1;
      size_t idEnd = output.find("\"", idStart);
      if (idStart != string::npos && idEnd != string::npos)
        r.videoId = output.substr(idStart, idEnd - idStart);
    }

    // Extract title
    size_t titlePos = output.find("\"title\":", pos);
    if (titlePos != string::npos && titlePos < pos + 500) {
      size_t titleStart = output.find("\"", titlePos + 8) + 1;
      size_t titleEnd = output.find("\"", titleStart);
      if (titleStart != string::npos && titleEnd != string::npos)
        r.title = output.substr(titleStart, titleEnd - titleStart);
    }

    // Extract channel/uploader
    size_t chPos = output.find("\"channel\":", pos);
    if (chPos != string::npos && chPos < pos + 800) {
      size_t chStart = output.find("\"", chPos + 10) + 1;
      size_t chEnd = output.find("\"", chStart);
      if (chStart != string::npos && chEnd != string::npos)
        r.channel = output.substr(chStart, chEnd - chStart);
    }

    // Extract duration
    size_t durPos = output.find("\"duration\":", pos);
    if (durPos != string::npos && durPos < pos + 500) {
      size_t durStart = durPos + 11;
      r.durationSec = atoi(output.c_str() + durStart);
    } else {
      r.durationSec = 0;
    }

    pos = urlEnd + 1;
    ++count;
  }

  return count;
}

bool YouTubeSource::download(const string &url, const string &outputPath) {
  if (!ytdlpAvailable) {
    cerr << "  yt-dlp not found. Install from: https://github.com/yt-dlp/yt-dlp"
         << endl;
    return false;
  }

  // Download and convert to WAV (44100Hz stereo 16-bit PCM)
  string cmd = "yt-dlp -x --audio-format wav "
               "--postprocessor-args \"-ar 44100 -ac 2 -sample_fmt s16\" "
               "-o \"" +
               outputPath +
               "\" "
               "\"" +
               url + "\" 2>&1";

  cout << "  Downloading..." << endl;
  string output = runCommand(cmd);

  // Check if output file exists
  FILE *test = fopen(outputPath.c_str(), "rb");
  if (test) {
    fclose(test);
    return true;
  }

  // yt-dlp may add .wav extension — check that too
  string withExt = outputPath + ".wav";
  test = fopen(withExt.c_str(), "rb");
  if (test) {
    fclose(test);
    // Rename to expected path
    rename(withExt.c_str(), outputPath.c_str());
    return true;
  }

  cerr << "  Download failed." << endl;
  return false;
}

void YouTubeSource::parseTitle(const string &fullTitle, string &artist,
                               string &trackTitle) {
  // Try to split on " - " (common YouTube music format: "Artist - Song")
  size_t dash = fullTitle.find(" - ");
  if (dash != string::npos) {
    artist = fullTitle.substr(0, dash);
    trackTitle = fullTitle.substr(dash + 3);
  } else {
    artist = "YouTube";
    trackTitle = fullTitle;
  }

  // Clean up common YouTube suffixes
  const char *suffixes[] = {"(Official Video)",
                            "(Official Music Video)",
                            "(Official Audio)",
                            "(Lyric Video)",
                            "(Lyrics)",
                            "(Audio)",
                            "[Official Video]",
                            "[Official Music Video]",
                            "[Official Audio]",
                            "(HD)",
                            "(HQ)"};

  for (int i = 0; i < 11; ++i) {
    size_t pos = trackTitle.find(suffixes[i]);
    if (pos != string::npos) {
      trackTitle = trackTitle.substr(0, pos);
      // Trim trailing spaces
      while (!trackTitle.empty() && trackTitle.back() == ' ')
        trackTitle.pop_back();
    }
  }
}
