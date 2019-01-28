/*	
 * Kun Peng
 * service layer implementation for CS 499 chirp project
 */
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



class ServiceImpl final : public ServiceLayer::Service {
	public:
	  explicit ServiceImpl(){
	  	storageclient_ = StorageClient(grpc::CreateChannel("localhost:50000",
                          grpc::InsecureChannelCredentials()));
	  }

	  Status registeruser(ServerContext* context, 
	  										const RegisterRequest* registerRequest, 
	  										RegisterReply* registerReply) override;

	  Status chirp(ServerContext* context, const ChirpRequest* chirpRequest, ChirpReply* chirpReply) override;

	  Status follow(ServerContext* context, const FollowRequest* followRequest, FollowReply* followReply) override;

	  Status read(ServerContext* context, const ReadRequest* readRequest, ReadReply* readReply) override;

	  Status monitor(ServerContext* context, const MonitorRequest* monitorRequest, MonitorReply* monitorReply) override;

	private:
	  Storage_client storageclient_;
};