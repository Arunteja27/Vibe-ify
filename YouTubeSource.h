#ifndef YOUTUBESOURCE_H
#define YOUTUBESOURCE_H

#include <string>

struct YouTubeResult {
  std::string title;
  std::string url;
  std::string videoId;
  std::string channel;
  int durationSec;
};

// Wraps yt-dlp CLI for YouTube search and WAV download.
class YouTubeSource {
public:
  YouTubeSource();
  ~YouTubeSource();

  bool isAvailable() const;
  int search(const std::string &query, YouTubeResult *results,
             int maxResults = 5);
  bool download(const std::string &url, const std::string &outputPath);
  static void parseTitle(const std::string &fullTitle, std::string &artist,
                         std::string &trackTitle);

private:
  bool ytdlpAvailable;
  bool ffmpegAvailable;
  std::string runCommand(const std::string &cmd);
};

#endif
