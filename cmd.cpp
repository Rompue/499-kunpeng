#include "cmd.h"

#include <time.h>
#include <algorithm>

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
  	std::cout << status.error_message() << std::endl;
    std::cout << "register user failed." << std::endl;
    return false;
  } else {
  	return true;
  }
}

bool CommandLineInterface::post(std::string text) {
	ChirpRequest request;
	request.set_username(username_);
	request.set_text(text);
	ChirpReply reply;
	ClientContext context;
	Status status = stub_->chirp(&context, request, &reply);
  if (!status.ok()) {
    std::cout << "post failed." << std::endl;
    // TODO
    return false;
  } else {
  	std::cout << "posted chirp: \n" << " " << reply.chirp().id() << " " << reply.chirp().username() 
    << " " << reply.chirp().text()  << " " << reply.chirp().timestamp().seconds() << " " 
    << reply.chirp().timestamp().useconds() << std::endl;
  	return true;
  }
}

bool CommandLineInterface::post(std::string text, int parentid){
	ChirpRequest request;
	request.set_username(username_);
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
  	std::cout << "posted chirp replay to chirp id = " << parentid << "\n" << reply.chirp().id() << " " << reply.chirp().username() 
    << " " << reply.chirp().text()  << " " << reply.chirp().timestamp().seconds() << " " 
    << reply.chirp().timestamp().useconds() << std::endl;
  	return true;
  }
}

bool CommandLineInterface::followuser(std::string username){
	if (username.empty()) return false;
	FollowRequest request;
	request.set_username(username_);
	request.set_to_follow(username);
	FollowReply reply;
	ClientContext context;
	Status status = stub_->follow(&context, request, &reply);
  if (!status.ok()) {
    std::cout << status.error_message() << std::endl;
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
  	std::vector<std::string> chirpids;
  	std::vector<int> indentlevels;
		for (auto chirp : chirps) { // decide indent level according to reply relationship
			chirpids.push_back(chirp.id());
			auto pos = std::find(chirpids.begin(), chirpids.end(), chirp.parent_id());
			if (pos != chirpids.end()) { // this chirp is a reply of another
				indentlevels.push_back(indentlevels[pos - chirpids.begin()] + 1);
			} else {
				indentlevels.push_back(0);
			}
		}
		for (int i = 0; i < chirps.size(); i++) { // display chirps to command line
			displaySingleChirp(chirps[i], indentlevels[i]);
		}
		return true;
  }
}

void CommandLineInterface::displaySingleChirp(const Chirp& chirp, int indentlevel) {
	time_t t = chirp.timestamp().seconds(); // convert to time_t, ignores msec
	std::cout << std::string(indentlevel*3, ' ') << "| " << chirp.username() << " posted at " << asctime(localtime(&t));
	std::cout << std::string(indentlevel*3, ' ') << "|   " << chirp.text() << std::endl;
}

void CommandLineInterface::monitor() {
	MonitorRequest request;
	request.set_username(username_);
	MonitorReply reply;
	ClientContext context;
	std::unique_ptr<ClientReader<MonitorReply> > reader(
    stub_->monitor(&context, request));

	while (reader->Read(&reply)) {
		if (reply.has_chirp()) {
			// TODO display the chirp to command line
			displaySingleChirp(reply.chirp(), 0);
		}
	}
	Status status = reader->Finish();
}

bool CommandLineInterface::login(std::string username) {
	// TODO: check if the user exists
	if (username.empty()) return false;
	RegisterRequest request;
	request.set_username(username);
	RegisterReply reply;
	ClientContext context;
	Status status = stub_->login(&context, request, &reply);
  if (!status.ok()) {
  	std::cout << status.error_message() << std::endl;
    return false;
  } else {
  	return true;
  }
}