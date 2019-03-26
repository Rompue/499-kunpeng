#ifndef CHIRP_THREAD_SAFE_MAP_H_
#define CHIRP_THREAD_SAFE_MAP_H_

#include <mutex>
#include <shared_mutex>
#include <string>
#include <thread>
#include <unordered_map>

// A exclusive write, shared read unordered hashmap
class ThreadSafeMap {
 public:
  // Constructor
  ThreadSafeMap() : mutex_() {}
  // return the value of the `key` in the hashmap, return optional string
  std::optional<std::string> get(const std::string& key) const;
  // insert the `key` `value` pair into the map, update value if the key exists
  int put(const std::string& key, const std::string& value);
  // delete the `key` and its value if the `key` exists
  void deletekey(const std::string& key);

 private:
  // shared mutex for thread safety
  mutable std::shared_mutex mutex_;
  // internal unordered_map storing kay value pairs
  std::unordered_map<std::string, std::string> map_;
};

#endif  // CHIRP_THREAD_SAFE_MAP_H_