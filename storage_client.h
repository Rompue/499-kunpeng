/*
* Kun Peng
* storage client, issues put / get / deletekey request to the server
*/

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

class StorageClient {
  public:
    StorageClient(std::shared_ptr<Channel> channel)
        : stub_(KeyValueStore::NewStub(channel)) {
    }

    void put(const std::string& key, const std::string& value)；

    std::vector<std::string> get(const std::vector<std::string>& keys)；

    void deletekey(const std::string& key)；

  private:
    std::unique_ptr<KeyValueStore::Stub> stub_;
};
