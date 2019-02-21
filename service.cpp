#include "service.h"
#include <ctime>
#include <cstdlib>
#include <chrono>
#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <sstream>
#include <algorithm>
#include <iterator>
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using grpc::Status;
using grpc::StatusCode;
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

const std::string ServiceImpl::CHIRP_ID_KEY = "G_chirpid";
const std::string ServiceImpl::USER_ID_KEY = "G_userid";

Status ServiceImpl::registeruser(ServerContext* context, 
                    const RegisterRequest* registerRequest, 
                    RegisterReply* registerReply) {
  std::string username = registerRequest->username(); // assume username is not empty
  std::cout << "[LOG] Attempting to register " << username << std::endl;
  if (storageclient_.has("USER_"+username)) { // username exists
    std::cout << "[LOG] " << "username exists" << std::endl;
    Status status(StatusCode::ALREADY_EXISTS, "existed username");
    return status;
  } else {
    while (true) {
      std::string userid = storageclient_.get("G_userid");
      if (userid == storageclient_.get("G_userid")) { //TODO lock this part
        storageclient_.put("G_userid", std::to_string(std::stoi(userid)+1));
        storageclient_.put("USER_" + username, userid);
        std::cout << "[LOG] " << "username registered with id " << userid << std::endl;
        return Status::OK;
      }
    }
  }
}

Status ServiceImpl::login(ServerContext* context, 
                          const RegisterRequest* registerRequest, 
                          RegisterReply* registerReply) {
  std::string username = registerRequest->username(); // assume username is not empty
  if (storageclient_.has("USER_"+username)) { // username exists
    std::cout << "[LOG] " << username << " signed in" << std::endl;
    return Status::OK;
  } else {
    Status status(StatusCode::NOT_FOUND, "user doesn't exist");
    return status;
  }
}

Status ServiceImpl::chirp(ServerContext* context, const ChirpRequest* chirpRequest, ChirpReply* chirpReply) {

  // if the chirp is a reply check if the parent exists
  std::string parentid = "";
  if (!chirpRequest->parent_id().empty()) {
    if(!storageclient_.has("CHIRP_"+chirpRequest->parent_id())) {
      return Status(StatusCode::INVALID_ARGUMENT,
                           "cannot find chirp with id=" + chirpRequest->parent_id());
    }
    parentid = chirpRequest->parent_id();
  }

  // populate the new Chirp object to store
  Chirp* chirp = new Chirp;
  chirp->set_username(chirpRequest->username());
  chirp->set_text(chirpRequest->text());
  chirp->set_parent_id(chirpRequest->parent_id());
  Timestamp* timestamp = new Timestamp;
  unsigned long long now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
  timestamp->set_seconds(now/1000);
  timestamp->set_useconds(now);
  chirp->set_allocated_timestamp(timestamp);

  // log info
  std::cout << "[LOG] " << "Attempting to post chirp: \n" << " " << chirp->username() 
    << " " << chirp->text()  << " " << timestamp->seconds() << " " 
    << timestamp->useconds() << std::endl;

  // actually store the chirp to the database
  while (true) {
    std::string chirpid = storageclient_.get("G_chirpid");
    if (chirpid == storageclient_.get("G_chirpid")) { //TODO lock this part
      chirp->set_id(chirpid);
      std::string chirpbytes;
      std::string chirpkey = "CHIRP_" + chirpid;
      chirp->SerializeToString(&chirpbytes);
      Status idstatus = storageclient_.put("G_chirpid", std::to_string(std::stoi(chirpid)+1));
      Status chirpstatus = storageclient_.put(chirpkey, chirpbytes);
      if (!idstatus.ok() || !chirpstatus.ok()) {
        std::cout << "[LOG] cannot store chirp with id=" << chirpkey << std::endl;
        Status status(StatusCode::UNAVAILABLE, idstatus.error_message() + chirpstatus.error_message());
        return status;
      } else {
        std::cout << "[LOG] chirp stored with id=" << chirpkey << std::endl;
        // store parent-child relationship
        if (!parentid.empty()) {
          std::optional<std::string> childrenlist = storageclient_.has("PARENT_CHIRP_" + parentid);
          std::string newvalue = childrenlist? (*childrenlist + "," + chirpid) : chirpid; // append or make new entry
          Status inheritancestatus = storageclient_.put("PARENT_CHIRP_" + parentid, newvalue);
          if (!inheritancestatus.ok()) 
            return Status(StatusCode::UNAVAILABLE, inheritancestatus.error_message());
          else {
            std::cout << "[LOG] update parent-child: parentid:" 
            << parentid << " childrenlist: " << newvalue << std::endl; 
          }
        }

        // register the chirp to its poster
        std::optional<std::string> chirplist = storageclient_.has("USERCHIRPS_" + chirp->username());
        std::string newvalue = chirplist? (*chirplist + "," + chirpid) : chirpid; // append or make new entry
        Status registerstatus = storageclient_.put("USERCHIRPS_" + chirp->username(), newvalue);
        if (!registerstatus.ok()) 
          return Status(StatusCode::UNAVAILABLE, registerstatus.error_message());
        else {
          std::cout << "[LOG] registered the chirp to user: username:" 
          << chirp->username() << " chirp list: " << newvalue << std::endl; 
        }

        chirpReply->set_allocated_chirp(chirp);
        return Status::OK;
      }
    }
  }
}

Status ServiceImpl::follow(ServerContext* context, const FollowRequest* followRequest, FollowReply* followReply) {
  std::string username = followRequest->username();
  // check following username
  std::string followusername = followRequest->to_follow();
  if (!storageclient_.has("USER_"+followusername)) { // username exists
    Status status(StatusCode::NOT_FOUND, "username doesn't exist");
    return status;
  }

  std::optional<std::string> followedusers = storageclient_.has("FOLLOW_" + username);
  std::string newvalue = followedusers? (*followedusers + "\n" + followusername) : followusername; // append or make new entry
  Status followstatus = storageclient_.put("FOLLOW_" + username, newvalue);
  if (!followstatus.ok()) 
    return Status(StatusCode::UNAVAILABLE, followstatus.error_message());
  else {
    std::cout << "[LOG] update follow list: username:" 
    << username << " follow list: " << newvalue << std::endl; 
  }
  return Status::OK;
}

Status ServiceImpl::read(ServerContext* context, const ReadRequest* readRequest, ReadReply* readReply) {
  std::string chirpid = readRequest->chirp_id();
  // check if the chirp exist
  std::optional<std::string> chirpstr = storageclient_.has("CHIRP_" + chirpid);
  if (!chirpstr) {
    return Status(StatusCode::NOT_FOUND, "chirp with id=" + chirpid + " not found");
  } else {
    populateReadReply(readReply, *chirpstr);
  }
  recursiveRead(readReply, chirpid);
  return Status::OK;
}

void ServiceImpl::recursiveRead(ReadReply* readReply, const std::string& chirpid) {
  std::cout << "[LOG] Attempt to find parent" << chirpid << std::endl;
  std::optional<std::string> childrenlist = storageclient_.has("PARENT_CHIRP_" + chirpid);
  if (childrenlist) {
    std::cout << "[LOG] find " << *childrenlist << " of parent " << chirpid << std::endl;
    std::vector<std::string> children = parseChirpList(*childrenlist);
    for (const auto& childid : children) {
      populateReadReply(readReply, storageclient_.get("CHIRP_" + childid));
      recursiveRead(readReply, childid);
    }
  } else {
    return;
  }
}

void ServiceImpl::populateReadReply(ReadReply* readReply, const std::string& chirpstr) {
  Chirp* newchirp = readReply->add_chirps();
  newchirp->ParseFromString(chirpstr);
  std::cout << "[LOG] read chirp " << "id=" << newchirp->id() << " from storage: " << newchirp->text() << std::endl;
}

std::vector<std::string> ServiceImpl::parseChirpList(const std::string& childrenlist) {
  std::stringstream ss(childrenlist);
  std::string token;
  std::vector<std::string> children;
  while (std::getline(ss, token, ',')) {
    children.push_back(token);
  }
  return children;
}

std::vector<std::string> ServiceImpl::parseUserList(const std::string& userlist) {
  std::stringstream ss(userlist);
  std::string token;
  std::vector<std::string> users;
  while (std::getline(ss, token, '\n')) {
    users.push_back(token);
  }
  return users;
}

Status ServiceImpl::monitor(ServerContext* context,
                            const MonitorRequest* monitorRequest,
                            ServerWriter<MonitorReply>* writer) {
  std::string username = monitorRequest->username();
  std::cout << "[LOG] " << username << " starts monitoring" << std::endl;

  // initial state, store past chirps (won't be displayed)
  std::map<std::string, std::string> chirplists;
  std::optional<std::string> followedusers = storageclient_.has("FOLLOW_" + username);
  if (followedusers) {
    auto usernames = parseUserList(*followedusers);
    // get chirps belonging to the followed users
    for (const std::string& username : usernames) {
      std::optional<std::string> chirplist = storageclient_.has("USERCHIRPS_" + username);
      if (chirplist) chirplists[username] = *chirplist;
    }
  }

  clock_t startTime = clock();
  int secondsPassed;
  int pullingInterval = 3;
  while (true) { // never ends
    secondsPassed = (clock() - startTime) / CLOCKS_PER_SEC;
    if(secondsPassed >= pullingInterval) {
      if (context->IsCancelled()) {
        std::cout << "[LOG] " << username << " stoped monitoring" << std::endl;
        return Status::OK;
      }
      startTime = clock(); // reset clock
      // get following list
      std::optional<std::string> followedusers = storageclient_.has("FOLLOW_" + username);
      if (!followedusers) continue;
      auto usernames = parseUserList(*followedusers);
      // get chirps belonging to the followed users
      for (const std::string& username : usernames) {
        std::optional<std::string> newchirplist = storageclient_.has("USERCHIRPS_" + username);
        if (!newchirplist || (chirplists[username] == *newchirplist)) continue;
        auto oldchirps = parseChirpList(chirplists[username]);
        auto newchirps = parseChirpList(*newchirplist);
        std::vector<std::string> addedchirps;
        std::set_difference(
            newchirps.begin(), newchirps.end(),
            oldchirps.begin(), oldchirps.end(),
            std::back_inserter(addedchirps)
        );
        for (const std::string& chirpid : addedchirps) {
          Chirp* newchirp = new Chirp();
          newchirp->ParseFromString(storageclient_.get("CHIRP_" + chirpid));
          MonitorReply reply;
          reply.set_allocated_chirp(newchirp);
          bool streamStatus = writer->Write(reply);
          if (!streamStatus) return Status::OK;
        }
        chirplists[username] = *newchirplist;
      }
    }
  }
  return Status::OK;
}

void ServiceImpl::initializeStorage(){
  if (!storageclient_.has(CHIRP_ID_KEY)) {
    storageclient_.put(CHIRP_ID_KEY, "1");
    std::cout << "[LOG] inserted G_chirpid " << storageclient_.get(CHIRP_ID_KEY)  << std::endl;
  } else {
    std::cout << "didn't insert G_chirpid" << std::endl;
  }

  if (!storageclient_.has(USER_ID_KEY)) {
    storageclient_.put(USER_ID_KEY, "1");
    std::cout << "[LOG] inserted G_userid " << storageclient_.get(USER_ID_KEY) << std::endl;
  } else {
    std::cout << "didn't insert G_userid" << std::endl;
  }
}