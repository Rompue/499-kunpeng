#include "cmd.h"

#include <iostream>
#include <string>
#include <vector>

#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>

int main(int argc, char const *argv[]) {
  CommandLineInterface commandLineInterface(grpc::CreateChannel(
      "localhost:50001", grpc::InsecureChannelCredentials()));
  if (argc < 2) {
    std::cerr << "please run with arguments" << std::endl;
    return 1;
  }

  std::vector<std::string> arguments(argv + 1, argv + argc);
  int argumentsize = arguments.size();

  // register user
  if (arguments[0] == "--register") {
    if (argumentsize != 2) {
      std::cerr << "usage: --user <username>" << std::endl;
      return 1;
    } else {
      commandLineInterface.registeruser(arguments[1]);
    }
    // other operation
  } else if (arguments[0] == "--user") {
    // check if user exists
    if (argumentsize < 3) {
      std::cerr << "invalid arguments" << std::endl;
      return 1;
    } else {
      bool uservalid = commandLineInterface.login(arguments[1]);
      if (!uservalid) {
        std::cerr << "invalid username" << std::endl;
        return 1;
      }
      commandLineInterface.set_username(arguments[1]);
      // successfully login
      if (argumentsize == 2) return 0;

      // post new chirp
      if (arguments[2] == "--chirp") {
        if (argumentsize < 4) {
          std::cerr << "invalid arguments" << std::endl;
          return 1;
        }
        std::string text = arguments[3];
        // if (text[0] != '"' || text[text.size()-1] != '"') {
        // 	std::cerr << "invalid chirp text" << text[0] <<
        // text[text.size()-1] << std::endl; 	return 1;
        // }

        int replyid = -1;
        // check if the new chirp is a reply
        if (argumentsize > 4) {
          if (argumentsize != 6 || arguments[4] != "--reply") {
            std::cerr << "invalid arguments" << std::endl;
            return 1;
          }
          replyid = std::stoi(arguments[5]);
        }

        bool success;
        if (replyid == -1) {
          success = commandLineInterface.post(text);
        } else {
          success = commandLineInterface.post(text, replyid);
        }
        if (!success) {
          std::cerr << "fail to post chirp " << std::endl;
          return 1;
        } else {
          return 0;
        }
      }

      // follow other user
      if (arguments[2] == "--follow") {
        if (argumentsize != 4) {
          std::cerr << "invalid arguments" << std::endl;
          return 1;
        }
        std::string username = arguments[3];
        bool success = commandLineInterface.followuser(username);
        if (!success) {
          std::cerr << "fail to follow " << arguments[3] << std::endl;
          return 1;
        } else {
          return 0;
        }
      }

      // monitor
      if (arguments[2] == "--monitor") {
        if (argumentsize > 3) {
          std::cerr << "invalid arguments" << std::endl;
          return 1;
        }
        commandLineInterface.monitor();
      }
    }
    // read a single chirp without login
  } else if (arguments[0] == "--read") {
    if (argumentsize != 2) {
      std::cerr << "invalid arguments" << std::endl;
      return 1;
    }
    int chirpid = std::stoi(arguments[1]);
    bool success = commandLineInterface.read(chirpid);
    if (!success) {
      std::cerr << "fail to read chirp " << chirpid << std::endl;
      return 1;
    }
  } else {
    std::cerr << "invalid arguments" << std::endl;
    return 1;
  }

  return 0;
}