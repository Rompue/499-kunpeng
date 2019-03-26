#include "thread_safe_map.h"
#include <iostream>

std::optional<std::string> ThreadSafeMap::get(const std::string& key) const {
  std::shared_lock lock(mutex_);
  if (map_.find(key) == map_.end()) {
    return std::nullopt;
  } else {
    return map_.find(key)->second;
  }
}

int ThreadSafeMap::put(const std::string& key, const std::string& value) {
  std::unique_lock lock(mutex_);
  map_[key] = value;
  return 0;
}

void ThreadSafeMap::deletekey(const std::string& key) {
  std::unique_lock lock(mutex_);
  map_.erase(key);
}
