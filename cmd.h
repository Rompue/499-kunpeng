#ifndef CHIRP_CMD_H_
#define CHIRP_CMD_H_

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
#include "service.h"

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
using chirp::ServiceLayer;
using chirp::Timestamp;
using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
using grpc::Status;

// A service layer client, used by the command line tool for chirp project
class CommandLineInterface {
 public:
  CommandLineInterface(std::shared_ptr<Channel> channel)
      : stub_(ServiceLayer::NewStub(channel)) {}
  // constructor for testing
  CommandLineInterface(bool testing)
      : testing_(testing), testingservice_(testing) {}
  // return if the object is for testing
  bool isTesting() { return testing_; }
  // set the username for this user
  void set_username(const std::string& username) { username_ = username; }
  // register a new user with `username`
  bool registeruser(const std::string& username);
  // post a chirp, the chirp does not have a parent
  bool post(const std::string& text);
  // test handler function for the above function with reply as output for
  // testing
  bool postTest(const std::string& text, ChirpReply& reply);
  // post a chirp having parent
  bool post(const std::string& text, int parentid);
  // test handler function for the above function with reply as output for
  // testing
  bool postTest(const std::string& text, int parentid, ChirpReply& reply);
  // start to follow the user with `username`
  bool followuser(const std::string& username);
  // returns followed user list for testing purpose
  std::optional<std::vector<std::string> > followedusernames();
  // read a chirp with `chirpid` from the database
  bool read(int chirpid);
  // test handler function for the above function with reply as output for
  // testing
  bool readTest(int chirpid, ReadReply& reply);
  // start to monitor the followed user and output chirps to console
  void monitor();

  std::vector<Chirp> monitorTest();
  // validify the username, if valid, login the user
  bool login(const std::string& username);

 private:
  std::unique_ptr<ServiceLayer::Stub> stub_;
  std::string username_;  // the logined username
  bool testing_ = false;  // is this object for testing
  ServiceImpl testingservice_ =
      ServiceImpl(false);            // hold a service implementation if testing
  static const int kIndentSpaceNum;  // the number of spaces a chirp indented
                                     // when outputed as a reply
  /* output the content of a single `chirp` into standard output, indent
   3*`indentlevel` spaces on the left side */
  void displaySingleChirp(const Chirp& chirp, int indentlevel);
};

#endif  // CHIRP_CMD_H_