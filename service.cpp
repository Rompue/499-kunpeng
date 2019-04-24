#include "service.h"
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <iterator>
#include <map>
#include <sstream>
#include <string>
#include <vector>
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
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using grpc::Status;
using grpc::StatusCode;

const int ServiceImpl::kPullingIntervalSeconds = 3;
const std::string ServiceImpl::kChirpIDKey = "G_chirpid";
const std::string ServiceImpl::kUserIDKey = "G_userid";
const std::string ServiceImpl::kUsernameEntryKeyPrefix = "USER_";
const std::string ServiceImpl::kFollowingUserEntryKeyPrefix = "FOLLOW_";
const std::string ServiceImpl::kChirpEntryKeyPrefix = "CHIRP_";
const std::string ServiceImpl::kChirpReplyEntryKeyPrefix = "PARENT_CHIRP_";
const std::string ServiceImpl::kUserChirpEntryKeyPrefix = "USERCHIRPS_";
const std::string ServiceImpl::kTagListKeyPrefix = "TAGLIST_";

Status ServiceImpl::registeruser(ServerContext* context,
                                 const RegisterRequest* registerRequest,
                                 RegisterReply* registerReply) {
  std::string username =
      registerRequest->username();  // assume username is not empty
  std::cout << "[LOG] Attempting to register " << username << std::endl;
  if (storageclient_.has(kUsernameEntryKeyPrefix +
                         username)) {  // username exists
    std::cout << "[LOG] "
              << "username exists" << std::endl;
    Status status(StatusCode::ALREADY_EXISTS, "existed username");
    return status;
  } else {
    while (true) {
      std::string userid = storageclient_.get(kUserIDKey);
      if (userid == storageclient_.get(kUserIDKey)) {
        storageclient_.put(kUserIDKey, std::to_string(std::stoi(userid) + 1));
        storageclient_.put(kUsernameEntryKeyPrefix + username, userid);
        std::cout << "[LOG] "
                  << "username registered with id " << userid << std::endl;
        return Status::OK;
      }
    }
  }
}

Status ServiceImpl::login(ServerContext* context,
                          const RegisterRequest* registerRequest,
                          RegisterReply* registerReply) {
  std::string username =
      registerRequest->username();  // assume username is not empty
  if (storageclient_.has(kUsernameEntryKeyPrefix +
                         username)) {  // username exists
    std::cout << "[LOG] " << username << " signed in" << std::endl;
    return Status::OK;
  } else {
    Status status(StatusCode::NOT_FOUND, "user doesn't exist");
    return status;
  }
}

Status ServiceImpl::chirp(ServerContext* context,
                          const ChirpRequest* chirpRequest,
                          ChirpReply* chirpReply) {
  // if the chirp is a reply check if the parent exists
  std::string parentid = "";
  if (!chirpRequest->parent_id().empty()) {
    if (!storageclient_.has(kChirpEntryKeyPrefix + chirpRequest->parent_id())) {
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
  unsigned long long now =
      std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::system_clock::now().time_since_epoch())
          .count();
  timestamp->set_seconds(now / 1000);
  timestamp->set_useconds(now);
  chirp->set_allocated_timestamp(timestamp);

  // log info
  std::cout << "[LOG] "
            << "Attempting to post chirp: \n"
            << " " << chirp->username() << " " << chirp->text() << " "
            << timestamp->seconds() << " " << timestamp->useconds()
            << std::endl;

  // actually store the chirp to the database
  while (true) {
    std::string chirpid = storageclient_.get(kChirpIDKey);
    if (chirpid == storageclient_.get(kChirpIDKey)) {
      chirp->set_id(chirpid);
      std::string chirpbytes;
      std::string chirpkey = kChirpEntryKeyPrefix + chirpid;
      chirp->SerializeToString(&chirpbytes);
      Status idstatus = storageclient_.put(
          kChirpIDKey, std::to_string(std::stoi(chirpid) + 1));
      Status chirpstatus = storageclient_.put(chirpkey, chirpbytes);
      if (!idstatus.ok() || !chirpstatus.ok()) {
        std::cout << "[LOG] cannot store chirp with id=" << chirpkey
                  << std::endl;
        Status status(StatusCode::UNAVAILABLE,
                      idstatus.error_message() + chirpstatus.error_message());
        return status;
      } else {
        std::cout << "[LOG] chirp stored with id=" << chirpkey << std::endl;
        // store parent-child relationship
        if (!parentid.empty()) {
          std::optional<std::string> childrenlist =
              storageclient_.has(kChirpReplyEntryKeyPrefix + parentid);
          std::string newvalue = childrenlist
                                     ? (*childrenlist + "," + chirpid)
                                     : chirpid;  // append or make new entry
          Status inheritancestatus = storageclient_.put(
              kChirpReplyEntryKeyPrefix + parentid, newvalue);
          if (!inheritancestatus.ok())
            return Status(StatusCode::UNAVAILABLE,
                          inheritancestatus.error_message());
          else {
            std::cout << "[LOG] update parent-child: parentid:" << parentid
                      << " childrenlist: " << newvalue << std::endl;
          }
        }

        // register the chirp to its poster
        std::optional<std::string> chirplist =
            storageclient_.has(kUserChirpEntryKeyPrefix + chirp->username());
        std::string newvalue = chirplist ? (*chirplist + "," + chirpid)
                                         : chirpid;  // append or make new entry
        Status registerstatus = storageclient_.put(
            kUserChirpEntryKeyPrefix + chirp->username(), newvalue);
        if (!registerstatus.ok())
          return Status(StatusCode::UNAVAILABLE,
                        registerstatus.error_message());
        else {
          std::cout << "[LOG] registered the chirp to user: username:"
                    << chirp->username() << " chirp list: " << newvalue
                    << std::endl;
        }

        chirpReply->set_allocated_chirp(chirp);
        return Status::OK;
      }
    }
  }
}

Status ServiceImpl::follow(ServerContext* context,
                           const FollowRequest* followRequest,
                           FollowReply* followReply) {
  std::string username = followRequest->username();
  // check following username
  std::string followusername = followRequest->to_follow();
  if (!storageclient_.has(kUsernameEntryKeyPrefix +
                          followusername)) {  // username exists
    Status status(StatusCode::NOT_FOUND, "username doesn't exist");
    return status;
  }

  std::optional<std::string> followedusers =
      storageclient_.has(kFollowingUserEntryKeyPrefix + username);
  std::string newvalue = followedusers
                             ? (*followedusers + "\n" + followusername)
                             : followusername;  // append or make new entry
  Status followstatus =
      storageclient_.put(kFollowingUserEntryKeyPrefix + username, newvalue);
  if (!followstatus.ok())
    return Status(StatusCode::UNAVAILABLE, followstatus.error_message());
  else {
    std::cout << "[LOG] update follow list: username:" << username
              << " follow list: " << newvalue << std::endl;
  }
  return Status::OK;
}

std::optional<std::vector<std::string> > ServiceImpl::followedusernames(
    std::string username) {
  if (testing_) {
    // directly retrieve the following user entry from backend storage and parse
    std::optional<std::string> followedusers =
        storageclient_.has(kFollowingUserEntryKeyPrefix + username);
    if (!followedusers) {
      return std::nullopt;
    } else {
      std::string usernamesstr = followedusers.value();
      std::vector<std::string> usernamesvec;
      std::stringstream ss(usernamesstr);
      std::string tempusername;
      while (std::getline(ss, tempusername, '\n')) {
        usernamesvec.push_back(tempusername);
      }
      return usernamesvec;
    }
  } else {
    return std::nullopt;
  }
}

Status ServiceImpl::read(ServerContext* context, const ReadRequest* readRequest,
                         ReadReply* readReply) {
  std::string chirpid = readRequest->chirp_id();
  // check if the chirp exist
  std::optional<std::string> chirpstr =
      storageclient_.has(kChirpEntryKeyPrefix + chirpid);
  if (!chirpstr) {
    return Status(StatusCode::NOT_FOUND,
                  "chirp with id=" + chirpid + " not found");
  } else {
    populateReadReply(readReply, *chirpstr);
  }
  recursiveRead(readReply, chirpid);
  return Status::OK;
}

void ServiceImpl::recursiveRead(ReadReply* readReply,
                                const std::string& chirpid) {
  std::cout << "[LOG] Attempt to find parent" << chirpid << std::endl;
  std::optional<std::string> childrenlist =
      storageclient_.has(kChirpReplyEntryKeyPrefix + chirpid);
  if (childrenlist) {
    std::cout << "[LOG] find " << *childrenlist << " of parent " << chirpid
              << std::endl;
    std::vector<std::string> children = parseChirpList(*childrenlist);
    for (const auto& childid : children) {
      populateReadReply(readReply,
                        storageclient_.get(kChirpEntryKeyPrefix + childid));
      recursiveRead(readReply, childid);
    }
  } else {
    return;
  }
}

void ServiceImpl::populateReadReply(ReadReply* readReply,
                                    const std::string& chirpstr) {
  Chirp* newchirp = readReply->add_chirps();
  newchirp->ParseFromString(chirpstr);
  std::cout << "[LOG] read chirp "
            << "id=" << newchirp->id() << " from storage: " << newchirp->text()
            << std::endl;
}

std::vector<std::string> ServiceImpl::parseChirpList(
    const std::string& childrenlist) {
  std::stringstream ss(childrenlist);
  std::string token;
  std::vector<std::string> children;
  while (std::getline(ss, token, ',')) {
    children.push_back(token);
  }
  return children;
}

std::vector<std::string> ServiceImpl::parseUserList(
    const std::string& userlist) {
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
  std::optional<std::string> followedusers =
      storageclient_.has(kFollowingUserEntryKeyPrefix + username);
  if (followedusers) {
    auto usernames = parseUserList(*followedusers);
    // get chirps belonging to the followed users
    for (const std::string& username : usernames) {
      std::optional<std::string> chirplist =
          storageclient_.has(kUserChirpEntryKeyPrefix + username);
      if (chirplist) chirplists[username] = *chirplist;
    }
  }

  clock_t startTime = clock();
  int secondsPassed;
  while (true) {  // never ends
    secondsPassed = (clock() - startTime) / CLOCKS_PER_SEC;
    if (testing_ || secondsPassed >= kPullingIntervalSeconds) {
      if (!testing_ && context->IsCancelled()) {
        std::cout << "[LOG] " << username << " stoped monitoring" << std::endl;
        return Status::OK;
      }
      startTime = clock();  // reset clock
      // get following list
      std::optional<std::string> followedusers =
          storageclient_.has(kFollowingUserEntryKeyPrefix + username);
      if (!followedusers) continue;
      auto usernames = parseUserList(*followedusers);
      // get chirps belonging to the followed users
      if (testing_ &&
          !initialmonitor_) {  // if testing and not the first call to monitor,
                               // load previous status as starting point
        chirplists = previouschirplists_;
      }
      for (const std::string& username : usernames) {
        std::optional<std::string> newchirplist =
            storageclient_.has(kUserChirpEntryKeyPrefix + username);
        if (!newchirplist || (chirplists[username] == *newchirplist)) continue;
        auto oldchirps = parseChirpList(chirplists[username]);
        auto newchirps = parseChirpList(*newchirplist);
        std::vector<std::string> addedchirps;
        std::set_difference(newchirps.begin(), newchirps.end(),
                            oldchirps.begin(), oldchirps.end(),
                            std::back_inserter(addedchirps));
        for (const std::string& chirpid : addedchirps) {
          Chirp* newchirp = new Chirp();
          newchirp->ParseFromString(
              storageclient_.get(kChirpEntryKeyPrefix + chirpid));
          MonitorReply reply;
          reply.set_allocated_chirp(newchirp);
          if (testing_) {  // if testing, add the new chirps
            newchirps_.push_back(reply.chirp());
          } else {
            bool streamStatus = writer->Write(reply);
            if (!streamStatus) return Status::OK;
          }
        }
        chirplists[username] = *newchirplist;
      }
    }
    if (testing_) {  // don't loop if in testing scenario and store the current
                     // chirps
      previouschirplists_ = chirplists;
      break;
    }
  }
  return Status::OK;
}

void ServiceImpl::initializeStorage() {
  if (!storageclient_.has(kChirpIDKey)) {
    storageclient_.put(kChirpIDKey, "1");
    std::cout << "[LOG] inserted G_chirpid " << storageclient_.get(kChirpIDKey)
              << std::endl;
  } else {
    std::cout << "didn't insert G_chirpid" << std::endl;
  }

  if (!storageclient_.has(kUserIDKey)) {
    storageclient_.put(kUserIDKey, "1");
    std::cout << "[LOG] inserted G_userid " << storageclient_.get(kUserIDKey)
              << std::endl;
  } else {
    std::cout << "didn't insert G_userid" << std::endl;
  }
}

// This specifies the total length of the "time" part in the key since the key
// consists of the tag name and time
const size_t kKeyTimeLength = 10;
// This decides how to divide time to arrange them into different keys
const size_t kTimeInterval = 100;
std::string ServiceImpl::GetKey(const std::string& tag, const Timestamp& time) {
  return GetKey(tag, time.seconds());
}
std::string ServiceImpl::GetKey(const std::string& tag,
                                const uint64_t& second) {
  std::string time_key = std::to_string(second / kTimeInterval);
  // add padding '0's to make sure the `time_key` is always 10 chars width
  time_key.insert(0, kKeyTimeLength - time_key.size(), '0');

  return kTagListKeyPrefix + time_key + tag;
}

void ServiceImpl::AddToTagList(const std::string& tag, const Timestamp& time,
                               const std::string& chirp_id) {
  std::string key = GetKey(tag, time);

  ServiceData::TagList tag_list;
  if (storageclient_.has(key)) {
    std::string tmp = storageclient_.get(key);
    tag_list.ParseFromString(tmp);
  }
  tag_list.add_chirp_ids(chirp_id);

  std::string val;
  tag_list.SerializeToString(&val);
  storageclient_.put(key, val);
}

ServiceData::TagList ServiceImpl::GetChirpsByTagFromTime(
    const std::string& tag, const Timestamp& from) {
  ServiceData::TagList ret;

  uint64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
                     std::chrono::system_clock::now().time_since_epoch())
                     .count();
  uint64_t now_second = now / 1000;

  for (uint64_t second = from.seconds(); second < now_second;
       second += kTimeInterval) {
    std::string key = GetKey(tag, second);

    if (storageclient_.has(key)) {
      ServiceData::TagList tag_list;
      std::string tmp = storageclient_.get(key);
      tag_list.ParseFromString(tmp);

      for (int i = 0; i < tag_list.chirp_ids_size(); ++i) {
        ret.add_chirp_ids(tag_list.chirp_ids(i));
      }
    }
  }

  return ret;
}
