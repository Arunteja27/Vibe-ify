#include "YouTubeSource.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

using namespace std;

YouTubeSource::YouTubeSource() {
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

static string decodeUnicode(const string &s) {
  string result;
  for (size_t i = 0; i < s.size(); ++i) {
    if (i + 5 < s.size() && s[i] == '\\' && s[i + 1] == 'u') {
      char hex[5] = {s[i + 2], s[i + 3], s[i + 4], s[i + 5], 0};
      unsigned int cp = (unsigned int)strtol(hex, nullptr, 16);
      if (cp < 0x80) {
        result += (char)cp;
      } else if (cp < 0x800) {
        result += (char)(0xC0 | (cp >> 6));
        result += (char)(0x80 | (cp & 0x3F));
      } else {
        result += (char)(0xE0 | (cp >> 12));
        result += (char)(0x80 | ((cp >> 6) & 0x3F));
        result += (char)(0x80 | (cp & 0x3F));
      }
      i += 5;
    } else {
      result += s[i];
    }
  }
  return result;
}

static string extractField(const string &json, size_t searchFrom,
                           size_t searchEnd, const string &key) {
  string needle = "\"" + key + "\":";
  size_t pos = json.find(needle, searchFrom);
  if (pos == string::npos || pos >= searchEnd)
    return "";

  pos += needle.size();
  while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t'))
    ++pos;
  if (pos >= json.size())
    return "";

  if (json[pos] == '"') {
    size_t start = pos + 1;
    size_t end = json.find('"', start);
    if (end == string::npos)
      return "";
    return decodeUnicode(json.substr(start, end - start));
  }

  size_t start = pos;
  while (pos < json.size() && json[pos] != ',' && json[pos] != '}' &&
         json[pos] != ' ' && json[pos] != '\n')
    ++pos;
  string val = json.substr(start, pos - start);
  if (val == "null")
    return "";
  return val;
}

int YouTubeSource::search(const string &query, YouTubeResult *results,
                          int maxResults) {
  if (!ytdlpAvailable)
    return 0;

  int fetchCount = maxResults * 3;
  string cmd = "yt-dlp --flat-playlist --dump-single-json \"ytsearch" +
               to_string(fetchCount) + ":" + query + "\" 2>nul";

  string output = runCommand(cmd);
  if (output.empty())
    return 0;

  int count = 0;
  size_t pos = 0;

  while (count < maxResults && pos < output.size()) {
    pos = output.find("\"url\":", pos);
    if (pos == string::npos)
      break;

    size_t nextBlock = output.find("\"url\":", pos + 6);
    if (nextBlock == string::npos)
      nextBlock = output.size();

    string url = extractField(output, pos, nextBlock, "url");
    string title = extractField(output, pos, nextBlock, "title");
    string videoId = extractField(output, pos, nextBlock, "id");
    string channel = extractField(output, pos, nextBlock, "channel");
    string durStr = extractField(output, pos, nextBlock, "duration");
    int duration = durStr.empty() ? 0 : atoi(durStr.c_str());

    pos = nextBlock;

    if (title.empty() || duration <= 0)
      continue;
    if (videoId.empty())
      videoId = url;

    YouTubeResult &r = results[count];
    r.title = title;
    r.videoId = videoId;
    r.channel = channel;
    r.durationSec = duration;

    if (url.find("http") == string::npos)
      r.url = "https://www.youtube.com/watch?v=" + videoId;
    else
      r.url = url;

    ++count;
  }

  return count;
}

bool YouTubeSource::download(const string &url, const string &outputPath) {
  if (!ytdlpAvailable) {
    cerr << "  yt-dlp not found." << endl;
    return false;
  }

  string videoId;
  size_t vPos = url.find("v=");
  if (vPos != string::npos) {
    videoId = url.substr(vPos + 2);
    size_t amp = videoId.find('&');
    if (amp != string::npos)
      videoId = videoId.substr(0, amp);
  }

  if (videoId.empty()) {
    cerr << "  Could not extract video ID." << endl;
    return false;
  }

  string tempFile = videoId + ".wav";

  string cmd = "yt-dlp -x --audio-format wav "
               "--no-write-thumbnail --no-embed-thumbnail "
               "-o \"" +
               videoId +
               ".%(ext)s\" "
               "\"" +
               url + "\" 2>&1";

  cout << "  Downloading..." << endl;
  runCommand(cmd);

  FILE *test = fopen(tempFile.c_str(), "rb");
  if (!test) {
    cerr << "  Download failed." << endl;
    return false;
  }
  fclose(test);

  remove(outputPath.c_str());
  if (rename(tempFile.c_str(), outputPath.c_str()) != 0) {
    string cpCmd =
        "copy /Y \"" + tempFile + "\" \"" + outputPath + "\" >nul 2>&1";
    runCommand(cpCmd);
    remove(tempFile.c_str());
  }

  test = fopen(outputPath.c_str(), "rb");
  if (test) {
    fclose(test);
    return true;
  }

  cerr << "  Download failed." << endl;
  return false;
}

void YouTubeSource::parseTitle(const string &fullTitle, string &artist,
                               string &trackTitle) {
  size_t dash = fullTitle.find(" - ");
  if (dash != string::npos) {
    artist = fullTitle.substr(0, dash);
    trackTitle = fullTitle.substr(dash + 3);
  } else {
    artist = "YouTube";
    trackTitle = fullTitle;
  }

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
      while (!trackTitle.empty() && trackTitle.back() == ' ')
        trackTitle.pop_back();
    }
  }
}
