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
#include "service.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
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


// A service layer client, used by the command line tool for chirp project
class CommandLineInterface {
public:
	CommandLineInterface(std::shared_ptr<Channel> channel)
      : stub_(ServiceLayer::NewStub(channel)) {}
  // set the username for this user
  void set_username(std::string username) {
  	myUsername = username;
  }
  // register a new user with `username`
	bool registeruser(std::string username);
	// post a chirp, the chirp does not have a parent
	bool post(std::string text);
	// post a chirp having parent
	bool post(std::string text, int parentid);
	// start to follow the user with `username`
	bool followuser(std::string username);
	// read a chirp with `chirpid` from the database
	bool read(int chirpid);
	// start to monitor the followed user and output chirps to console
	void monitor();
	// validify the username, if valid, login the user
	bool login(std::string username);

private:
	std::unique_ptr<ServiceLayer::Stub> stub_;
	std::string myUsername;
};