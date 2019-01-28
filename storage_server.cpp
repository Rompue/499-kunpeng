#include "storage_server.h"

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


Status StorageImpl::put(ServerContext* context, const PutRequest* putRequest, PutReply* putReply) override {
  map_.put(putRequest->key(), putRequest->value());
  return Status::OK;  // TODO::status indicate successfulness
}

Status StorageImpl::get(ServerContext* context, ServerReaderWriter<GetReply, GetRequest>* stream) {
  // std::vector<GetRequest> received_requests;
  GetRequest getrequest;
  while (stream->Read(&getrequest)) {
      GetReply getReply;
      getReply.set_value(map_.get(getrequest.key()));
      stream->Write(getReply);
    // received_requests.push_back(getrequest);
  }
  return Status::OK;
}

Status StorageImpl::deletekey(ServerContext* context, const DeleteRequest* deleteRequest,
                 DeleteReply* deleteReply) override {
  map_.deletekey(deleteRequest->key());
  return Status::OK;
}

// Start the storage backend server at port 50000 on localhost
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