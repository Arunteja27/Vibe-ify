#include "StreamCache.h"
#include <cstdio>
#include <direct.h>
#include <fstream>

using namespace std;

StreamCache::StreamCache(const string &cacheDir, int maxFiles)
    : cacheDir(cacheDir), maxFiles(maxFiles) {
  ensureDir();
}

StreamCache::~StreamCache() {}

void StreamCache::ensureDir() const {
  _mkdir("media");
  _mkdir(cacheDir.c_str());
}

string StreamCache::getCachedPath(const string &videoId) const {
  string path = cacheDir + "/" + videoId + ".wav";
  ifstream test(path);
  if (test.good()) {
    test.close();
    return path;
  }
  return "";
}

string StreamCache::store(const string &videoId) {
  return cacheDir + "/" + videoId + ".wav";
}

bool StreamCache::isCached(const string &videoId) const {
  string path = cacheDir + "/" + videoId + ".wav";
  ifstream test(path);
  return test.good();
}
