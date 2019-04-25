#include "cmd.h"

#include <time.h>
#include <algorithm>
#include <iostream>
#include <sstream>

using chirp::Chirp;
using chirp::ChirpReply;
using chirp::ChirpRequest;
using chirp::FollowReply;
using chirp::FollowRequest;
using chirp::MonitorReply;
using chirp::MonitorRequest;
using chirp::StreamRequest;
using chirp::StreamReply;
using chirp::ReadReply;
using chirp::ReadRequest;
using chirp::RegisterReply;
using chirp::RegisterRequest;
using chirp::ServiceLayer;
using chirp::Timestamp;
using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
using grpc::Status;

const int CommandLineInterface::kIndentSpaceNum = 3;

bool CommandLineInterface::registeruser(const std::string& username) {
  if (username.empty()) {
    return false;
  }
  RegisterRequest request;
  request.set_username(username);
  RegisterReply reply;
  ClientContext context;
  // testing scenario
  if (testing_) {
    Status status = testingservice_.registeruser(nullptr, &request, &reply);
    if (status.ok()) {
      return true;
    } else {
      return false;
    }
  }
  // end of testing scenario
  Status status = stub_->registeruser(&context, request, &reply);
  if (!status.ok()) {
    std::cout << status.error_message() << std::endl;
    std::cout << "register user failed." << std::endl;
    return false;
  } else {
    return true;
  }
}

bool CommandLineInterface::post(const std::string& text) {
  ChirpRequest request;
  request.set_username(username_);
  request.set_text(text);
  ChirpReply reply;
  ClientContext context;

  Status status = stub_->chirp(&context, request, &reply);
  if (!status.ok()) {
    std::cout << "post failed." << std::endl;
    return false;
  } else {
    std::cout << "posted chirp: \n"
              << " " << reply.chirp().id() << " " << reply.chirp().username()
              << " " << reply.chirp().text() << " "
              << reply.chirp().timestamp().seconds() << " "
              << reply.chirp().timestamp().useconds() << std::endl;
    return true;
  }
}

bool CommandLineInterface::postTest(const std::string& text,
                                    ChirpReply& reply) {
  ChirpRequest request;
  request.set_username(username_);
  request.set_text(text);
  ClientContext context;
  Status status = testingservice_.chirp(nullptr, &request, &reply);
  if (!status.ok()) {
    std::cout << "post failed." << std::endl;
    return false;
  } else {
    std::cout << "posted chirp: \n"
              << " " << reply.chirp().id() << " " << reply.chirp().username()
              << " " << reply.chirp().text() << " "
              << reply.chirp().timestamp().seconds() << " "
              << reply.chirp().timestamp().useconds() << std::endl;
    return true;
  }
}

bool CommandLineInterface::post(const std::string& text, int parentid) {
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
    std::cout << "posted chirp replay to chirp id = " << parentid << "\n"
              << reply.chirp().id() << " " << reply.chirp().username() << " "
              << reply.chirp().text() << " "
              << reply.chirp().timestamp().seconds() << " "
              << reply.chirp().timestamp().useconds() << std::endl;
    return true;
  }
}

bool CommandLineInterface::postTest(const std::string& text, int parentid,
                                    ChirpReply& reply) {
  ChirpRequest request;
  request.set_username(username_);
  request.set_text(text);
  std::string idstr = std::to_string(parentid);
  request.set_parent_id(idstr);
  ClientContext context;
  Status status = testingservice_.chirp(nullptr, &request, &reply);
  if (!status.ok()) {
    std::cout << "post failed." << std::endl;
    return false;
  } else {
    std::cout << "posted chirp replay to chirp id = " << parentid << "\n"
              << reply.chirp().id() << " " << reply.chirp().username() << " "
              << reply.chirp().text() << " "
              << reply.chirp().timestamp().seconds() << " "
              << reply.chirp().timestamp().useconds() << std::endl;
    return true;
  }
}

bool CommandLineInterface::followuser(const std::string& username) {
  if (username.empty()) return false;
  FollowRequest request;
  request.set_username(username_);
  request.set_to_follow(username);
  FollowReply reply;
  // testing scenario
  if (testing_) {
    Status status = testingservice_.follow(nullptr, &request, &reply);
    if (status.ok()) {
      return true;
    } else {
      return false;
    }
  }
  // end of testing
  ClientContext context;
  Status status = stub_->follow(&context, request, &reply);
  if (!status.ok()) {
    std::cout << status.error_message() << std::endl;
    return false;
  } else {
    return true;
  }
}

std::optional<std::vector<std::string> >
CommandLineInterface::followedusernames() {
  if (testing_) {
    // directly retrieve the following user entry from backend storage and parse
    return testingservice_.followedusernames(username_);
  } else {
    return std::nullopt;
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
    ::google::protobuf::RepeatedPtrField< ::chirp::Chirp> chirps =
        reply.chirps();
    std::vector<std::string> chirpids;
    std::vector<int> indentlevels;
    for (auto chirp :
         chirps) {  // decide indent level according to reply relationship
      chirpids.push_back(chirp.id());
      auto pos = std::find(chirpids.begin(), chirpids.end(), chirp.parent_id());
      if (pos != chirpids.end()) {  // this chirp is a reply of another
        indentlevels.push_back(indentlevels[pos - chirpids.begin()] + 1);
      } else {
        indentlevels.push_back(0);
      }
    }
    for (int i = 0; i < chirps.size(); i++) {  // display chirps to command line
      displaySingleChirp(chirps[i], indentlevels[i]);
    }
    return true;
  }
}

bool CommandLineInterface::readTest(int chirpid, ReadReply& reply) {
  ReadRequest request;
  std::string idstr = std::to_string(chirpid);
  request.set_chirp_id(idstr);
  Status status = testingservice_.read(nullptr, &request, &reply);
  if (!status.ok()) {
    std::cout << "read failed." << std::endl;
    return false;
  } else {
    return true;
  }
}

void CommandLineInterface::displaySingleChirp(const Chirp& chirp,
                                              int indentlevel) {
  time_t t = chirp.timestamp().seconds();  // convert to time_t, ignores msec
  std::cout << std::string(indentlevel * kIndentSpaceNum, ' ') << "| "
            << chirp.username() << " posted at " << asctime(localtime(&t));
  std::cout << std::string(indentlevel * kIndentSpaceNum, ' ') << "|   "
            << chirp.text() << std::endl;
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
      displaySingleChirp(reply.chirp(), 0);
    }
  }
  Status status = reader->Finish();
}

void CommandLineInterface::stream(const std::string& tag) {
  StreamRequest request;
  StreamReply reply;

  // To check if the user includes `#`
  if (tag.front() == '#') {
    request.set_tag(tag.substr(1));
  } else {
    request.set_tag(tag);
  }

  ClientContext context;
  std::unique_ptr<ClientReader<StreamReply>> reader(stub_->stream(&context, request));

  while (reader->Read(&reply)) {
    int chirp_id = stoi(reply.chirp_id());
    read(chirp_id);
  }
}

std::vector<Chirp> CommandLineInterface::monitorTest() {
  MonitorRequest request;
  request.set_username(username_);
  MonitorReply reply;
  testingservice_.monitor(nullptr, &request, nullptr);
  auto addedchirps = testingservice_.monitorHandler();
  return addedchirps;
}

bool CommandLineInterface::login(const std::string& username) {
  if (username.empty()) {
    return false;
  }
  RegisterRequest request;
  request.set_username(username);
  RegisterReply reply;
  ClientContext context;
  // testing scenario
  if (testing_) {
    Status status = testingservice_.login(nullptr, &request, &reply);
    if (status.ok()) {
      set_username(username);
      return true;
    } else {
      return false;
    }
  }
  // end of testing scenario
  Status status = stub_->login(&context, request, &reply);
  if (!status.ok()) {
    std::cout << status.error_message() << std::endl;
    return false;
  } else {
    return true;
  }
}