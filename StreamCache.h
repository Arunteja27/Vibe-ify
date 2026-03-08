#ifndef STREAMCACHE_H
#define STREAMCACHE_H

#include <string>

// Manages downloaded audio files in media/cache/.
class StreamCache {
public:
  StreamCache(const std::string &cacheDir = "media/cache", int maxFiles = 50);
  ~StreamCache();

  std::string getCachedPath(const std::string &videoId) const;
  std::string store(const std::string &videoId);
  bool isCached(const std::string &videoId) const;
  void ensureDir() const;

private:
  std::string cacheDir;
  int maxFiles;
};

#endif
