#ifndef YOUTUBESOURCE_H
#define YOUTUBESOURCE_H

#include <string>

// Result from a YouTube search
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

  // Check if yt-dlp and ffmpeg are available
  bool isAvailable() const;

  // Search YouTube, returns number of results (max 5)
  int search(const std::string &query, YouTubeResult *results,
             int maxResults = 5);

  // Download a YouTube URL as WAV to the given path
  // Returns true on success
  bool download(const std::string &url, const std::string &outputPath);

  // Get artist and title from a YouTube video title string
  static void parseTitle(const std::string &fullTitle, std::string &artist,
                         std::string &trackTitle);

private:
  bool ytdlpAvailable;
  bool ffmpegAvailable;

  // Run a command and capture stdout
  std::string runCommand(const std::string &cmd);
};

#endif
