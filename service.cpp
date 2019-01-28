#include "service.h"
#include <chrono>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using grpc::Status;

using chirp::Chirp;
using chirp::ChirpReply;
using chirp::ChirpRequest;
using chirp::FollowReply;
using chirp::FollowRequest;
using chirp::MonitorReply;
using chirp::MonitorRequest;
using chirp::ReadReply;
using chirp::ReadRequest;
using chirp::RegisterReply;
using chirp::RegisterRequest;
using chirp::Timestamp;



Status ServiceImpl::registeruser(ServerContext* context, 
                    const RegisterRequest* registerRequest, 
                    RegisterReply* registerReply) override {
  std::string username = registerRequest.username();
  // TODO actually store the user info to the database
  // storageclient_.put();
  return Status::OK;

}

Status ServiceImpl::chirp(ServerContext* context, const ChirpRequest* chirpRequest, ChirpReply* chirpReply) override {
  Chirp* chirp = new Chirp;
  chirp->set_username(chirpRequest->username());
  chirp->set_text(chirpRequest->text());
  chirp->set_id(); //TODO
  chirp->set_parent_id(chirpRequest->parent_id());
  Timestamp* timestamp = new Timestamp;
  int now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
  timestamp->set_seconds(now/1000);
  timestamp->set_useconds(now);
  chirp->set_allocated_timestamp(timestamp);
  chirpReply->set_allocated_chirp(chirp);

  // TODO: actually store the chirp to the database
  return Status::OK;
}

Status ServiceImpl::follow(ServerContext* context, const FollowRequest* followRequest, FollowReply* followReply) override {
  std::string username = followRequest->username();
  std::string followedusername = followRequest->to_follow();

  // TODO: Store the info to database
  return Status::OK;
}

Status ServiceImpl::read(ServerContext* context, const ReadRequest* readRequest, ReadReply* readReply) override {
  std::string chirpid = readReply->chirp_id();
  std::vector<Chirp> chirps;

    // TODO: get the chirp and sub-chirp from the db
  // storageclient_.get();
  for (Chirp chirp : chirps) {
    readReply->mutable_chirps()->Add(chirp); //https://developers.google.com/protocol-buffers/docs/reference/cpp/google.protobuf.repeated_field
  }

  return Status::OK;
}

Status ServiceImpl::monitor(ServerContext* context, const MonitorRequest* monitorRequest, MonitorReply* monitorReply) override{
  // TODO implement
  return Status::OK;
}