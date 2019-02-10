#include "cmd.h"

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

bool CommandLineInterface::registeruser(std::string username) {
	if (username.empty()) return false;
	RegisterRequest request;
	request.set_username(username);
	RegisterReply reply;
	ClientContext context;
	Status status = stub_->registeruser(&context, request, &reply);
  if (!status.ok()) {
    std::cout << "register user failed." << std::endl;
    return false;
  } else {
  	return true;
  }
}

bool CommandLineInterface::post(std::string text) {
	ChirpRequest request;
	request.set_username(myUsername);
	request.set_text(text);
	ChirpReply reply;
	ClientContext context;
	Status status = stub_->chirp(&context, request, &reply);
  if (!status.ok()) {
    std::cout << "post failed." << std::endl;
    // TODO
    return false;
  } else {
  	return true;
  }
}

bool CommandLineInterface::post(std::string text, int parentid){
	ChirpRequest request;
	request.set_username(myUsername);
	request.set_text(text);
	std::string idstr = std::to_string(parentid);
	request.set_parent_id(idstr);
	ChirpReply reply;
	ClientContext context;
	Status status = stub_->chirp(&context, request, &reply);
  if (!status.ok()) {
    std::cout << "post failed." << std::endl;
    return false;
  } else {
  	return true;
  }
}

bool CommandLineInterface::followuser(std::string username){
	if (username.empty()) return false;
	FollowRequest request;
	request.set_username(myUsername);
	request.set_to_follow(username);
	FollowReply reply;
	ClientContext context;
	Status status = stub_->follow(&context, request, &reply);
  if (!status.ok()) {
    std::cout << "follow failed." << std::endl;
    return false;
  } else {
  	return true;
  }
}

bool CommandLineInterface::read(int chirpid) {
	ReadRequest request;
	std::string idstr = std::to_string(chirpid);
	request.set_chirp_id(idstr);
	ReadReply reply;
	ClientContext context;
	Status status = stub_->read(&context, request, &reply);
	if (!status.ok()) {
    std::cout << "read failed." << std::endl;
    return false;
  } else {
  	::google::protobuf::RepeatedPtrField< ::chirp::Chirp > chirps = reply.chirps();
		for (auto chirp : chirps) {
			//TODO display the chirps to command line
		}
		return true;
  }
}

void CommandLineInterface::monitor() {
	MonitorRequest request;
	request.set_username(myUsername);
	MonitorReply reply;
	ClientContext context;
	std::unique_ptr<ClientReader<MonitorReply> > reader(
    stub_->monitor(&context, request));

	while (reader->Read(&reply)) {
		if (reply.has_chirp()) {
			// TODO display the chirp to command line
		}
	}
	Status status = reader->Finish();
}

bool CommandLineInterface::login(std::string username) {
	// TODO: check if the user exists
	return true;
}