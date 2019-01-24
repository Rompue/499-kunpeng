/*
 *
 * Copyright 2015 gRPC authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <memory>
#include <string>

#include <grpc/grpc.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include <grpcpp/security/server_credentials.h>
#include "storage.grpc.pb.h"
#include "thread_safe_map.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using grpc::Status;
using chirp::PutRequest;
using chirp::PutReply;
using chirp::GetRequest;
using chirp::GetReply;
using chirp::DeleteRequest;
using chirp::DeleteReply;
using chirp::KeyValueStore;


class StorageImpl final : public KeyValueStore::Service {
 public:
  explicit StorageImpl() : map_() {}

  Status put(ServerContext* context, const PutRequest* putRequest, PutReply* putReply) override {
    map_.put(putRequest->key(), putRequest->value());
    return Status::OK;  // TODO::status indicate successfulness
  }

  Status get(ServerContext* context, ServerReaderWriter<GetReply, GetRequest>* stream) {
    std::vector<GetRequest> received_requests;
    GetRequest getrequest;
    while (stream->Read(&getrequest)) {
      for (const GetRequest& request : received_requests) {
        GetReply getReply;
        getReply.set_value(map_.get(request.key()));
        stream->Write(getReply);
      }
      received_requests.push_back(getrequest);
    }
    
    return Status::OK;
  }

  Status deletekey(ServerContext* context, const DeleteRequest* deleteRequest,
                   DeleteReply* deleteReply) override {
    map_.deletekey(deleteRequest->key());
    return Status::OK;
  }

private:
  ThreadSafeMap map_;

};



void RunServer() {
  std::string server_address("0.0.0.0:50000");
  StorageImpl service;

  ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;
  server->Wait();
}

int main(int argc, char** argv) {
  RunServer();

  return 0;
}