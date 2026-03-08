#ifndef STREAMCACHE_H
#define STREAMCACHE_H

#include <string>

// Manages downloaded audio files in media/cache/ with size-limited LRU
// eviction.
class StreamCache {
public:
  StreamCache(const std::string &cacheDir = "media/cache", int maxFiles = 50);
  ~StreamCache();

  // Get cached file path for a video ID, or empty string if not cached
  std::string getCachedPath(const std::string &videoId) const;

  // Store a file in the cache, returns the cache path
  std::string store(const std::string &videoId);

  // Check if a video is already cached
  bool isCached(const std::string &videoId) const;

  // Ensure cache directory exists
  void ensureDir() const;

private:
  std::string cacheDir;
  int maxFiles;
};

#endif
