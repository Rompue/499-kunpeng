/*
* Kun Peng
* A shared-read exclusive-write unordered hash map using shared_mutex
* for the CS499 chirp project
*/

#ifndef THREADSAFEMAP_H
#define THREADSAFEMAP_H

#include <string>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <unordered_map>


class ThreadSafeMap {
  public:
    // Constructor
    ThreadSafeMap() : mutex_() {}
    std::optional<std::string> get(const std::string& key) const;
    int put(const std::string& key, const std::string& value);
    void deletekey(const std::string& key);

  private:
    // shared mutex for thread safety
    mutable std::shared_mutex mutex_;
    // internal unordered_map storing kay value pairs
    std::unordered_map<std::string, std::string> map_; 
};

#endif