#ifndef STORAGE_CLIENT_H
#define STORAGE_CLIENT_H

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

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
using grpc::Status;
using chirp::PutRequest;
using chirp::PutReply;
using chirp::GetRequest;
using chirp::GetReply;
using chirp::DeleteRequest;
using chirp::DeleteReply;
using chirp::KeyValueStore;

//storage client, issues put / get / deletekey request to the server
class StorageClient {
  public:
    StorageClient(std::shared_ptr<Channel> channel)
        : stub_(KeyValueStore::NewStub(channel)) {
    }

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

  private:
    std::unique_ptr<KeyValueStore::Stub> stub_;
};

#endif
