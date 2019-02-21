#ifndef SERVICE_H
#define SERVICE_H

#include <vector>
#include <string>

#include <grpc/grpc.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include <grpcpp/security/server_credentials.h>
#include "service.grpc.pb.h"
#include "storage_client.h"

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
using chirp::ServiceLayer;

/* service layer server for chirp project */
class ServiceImpl final : public ServiceLayer::Service {
	public:
		// constructor
	  explicit ServiceImpl() : storageclient_(grpc::CreateChannel("localhost:50000",
                          grpc::InsecureChannelCredentials())) {
	  	initializeStorage();
	  }
	  // register user in storage
	  Status registeruser(ServerContext* context, 
	  										const RegisterRequest* registerRequest, 
	  										RegisterReply* registerReply) override;
	  // post a chirp in storage
	  Status chirp(ServerContext* context, const ChirpRequest* chirpRequest, ChirpReply* chirpReply) override;
	  // store a following relationship in storage
	  Status follow(ServerContext* context, const FollowRequest* followRequest, FollowReply* followReply);
	  // fetch a chirp and its children from storage
	  Status read(ServerContext* context, const ReadRequest* readRequest, ReadReply* readReply);
	  // continuously fetch chirps from followed user
		Status monitor(ServerContext* context, const MonitorRequest* monitorRequest, ServerWriter<MonitorReply>* writer);
		// check if a user exists
	  Status login(ServerContext* context, const RegisterRequest* registerRequest, RegisterReply* registerReply) override;
	private:
	  StorageClient storageclient_;
	  static const std::string CHIRP_ID_KEY;
	  static const std::string USER_ID_KEY;
	  // initialize chirp id in storage
	  void initializeStorage();
	  // fill `readReply` with a serialized chirp
	  void populateReadReply(ReadReply* readReply, const std::string& chirpstr);
	  // fill `readReply` with chirp (given id) and its children
	  void recursiveRead(ReadReply* readReply, const std::string& chirpid);
	  // parse a list of children to individual ids
	  std::vector<std::string> parseChirpList(const std::string& childrenlist);
	  // parse a list of usernames to individual usernames
	  std::vector<std::string> parseUserList(const std::string& userlist);

};

#endif