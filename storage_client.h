#ifndef CHIRP_STORAGE_CLIENT_H_
#define CHIRP_STORAGE_CLIENT_H_

#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <thread>

#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>
#include "storage.grpc.pb.h"
#include "thread_safe_map.h"

using chirp::DeleteReply;
using chirp::DeleteRequest;
using chirp::GetReply;
using chirp::GetRequest;
using chirp::KeyValueStore;
using chirp::PutReply;
using chirp::PutRequest;
using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
using grpc::Status;
using grpc::StatusCode;

// storage client, issues put / get / deletekey request to the server
class StorageClient {
 public:
  StorageClient(std::shared_ptr<Channel> channel)
      : stub_(KeyValueStore::NewStub(channel)) {}

  StorageClient(bool testing) : testing_(testing) {}

  // put `key` `value` pair to the storage server
  Status put(const std::string& key, const std::string& value);
  // get a stream of strings of the corresponding `keys` from the storage server
  std::vector<std::string> get(const std::vector<std::string>& keys);
  // delete the value under the `key` from the storage server
  Status deletekey(const std::string& key);
  // check if the map has a key
  std::optional<std::string> has(const std::string& key);
  // get a single value under the `key` from the map
  std::string get(const std::string& key);
  // return if the object is in testing environment
  bool isTesting() { return testing_; }

 private:
  std::unique_ptr<KeyValueStore::Stub> stub_;
  ThreadSafeMap map_;
  bool testing_ = false;
};

#endif  // CHIRP_STORAGE_CLIENT_H_
