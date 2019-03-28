#include "storage_server.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <memory>
#include <string>
#include "thread_safe_map.h"

#include <grpc/grpc.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include "storage.grpc.pb.h"

using chirp::DeleteReply;
using chirp::DeleteRequest;
using chirp::GetReply;
using chirp::GetRequest;
using chirp::KeyValueStore;
using chirp::PutReply;
using chirp::PutRequest;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using grpc::Status;

Status StorageImpl::put(ServerContext* context, const PutRequest* putrequest,
                        PutReply* putreply) {
  map_.put(putrequest->key(), putrequest->value());
  std::cout << "[LOG] [PUT] " << putrequest->key() << " | "
            << putrequest->value() << std::endl;
  return Status::OK;
}

Status StorageImpl::get(ServerContext* context,
                        ServerReaderWriter<GetReply, GetRequest>* stream) {
  // std::vector<GetRequest> received_requests;
  GetRequest getrequest;
  while (stream->Read(&getrequest)) {
    GetReply getReply;
    std::optional<std::string> value = map_.get(getrequest.key());
    if (value) {
      getReply.set_value(*value);
      std::cout << "[LOG] [GET] " << getrequest.key() << " | " << *value
                << std::endl;
    } else {
      Status status(grpc::StatusCode::NOT_FOUND,
                    "cannot find the value of given key");
      std::cout << "[LOG] [GET] " << getrequest.key() << std::endl;
      return status;
    }
    stream->Write(getReply);
    // received_requests.push_back(getrequest);
  }
  return Status::OK;
}

Status StorageImpl::deletekey(ServerContext* context,
                              const DeleteRequest* deleterequest,
                              DeleteReply* deletereply) {
  map_.deletekey(deleterequest->key());
  return Status::OK;
}

// Start the storage backend server at port 50000 on localhost
