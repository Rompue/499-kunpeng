#include <string>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <unordered_map>


class ThreadSafeMap {
public:
  ThreadSafeMap() = default;

  std::string get(std::string key) const {
    std::shared_lock lock(mutex_);
    if (map_.find(key) == map_.end()) {
      return std::string empty();
    } else {
      return map_.find(key)->second;
    }
  }

  int put(std::string key, std::string value) {
    std::unique_lock lock(mutex_);
    map_[key] = value;
    return 0;
  }

  void deletekey(std::string key) {
    std::unique_lock lock(mutex_);
    map_.erase(key);
  }

private:
  mutable std::shared_mutex mutex_;
  std::unordered_map<std::string, std::string> map_;

}