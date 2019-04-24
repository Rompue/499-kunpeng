#ifndef CHIRP_SERVICE_H_
#define CHIRP_SERVICE_H_

#include <string>
#include <vector>

#include <grpc/grpc.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include "service.grpc.pb.h"
#include "service_data.pb.h"
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
using chirp::ServiceLayer;
using chirp::Timestamp;

/* service layer server for chirp project */
class ServiceImpl final : public ServiceLayer::Service {
 public:
  // constructor
  explicit ServiceImpl()
      : storageclient_(grpc::CreateChannel(
            "localhost:50000", grpc::InsecureChannelCredentials())) {
    initializeStorage();
  }
  // constructor for testing
  ServiceImpl(bool testing) : testing_(testing), storageclient_(testing) {
    if (testing_) initializeStorage();
  }
  // return if the object is in test environment
  bool isTesting() { return testing_; }
  // register user in storage
  Status registeruser(ServerContext* context,
                      const RegisterRequest* registerRequest,
                      RegisterReply* registerReply) override;
  // post a chirp in storage
  Status chirp(ServerContext* context, const ChirpRequest* chirpRequest,
               ChirpReply* chirpReply) override;
  // store a following relationship in storage
  Status follow(ServerContext* context, const FollowRequest* followRequest,
                FollowReply* followReply);
  // fetch a chirp and its children from storage
  Status read(ServerContext* context, const ReadRequest* readRequest,
              ReadReply* readReply);
  // continuously fetch chirps from followed user
  Status monitor(ServerContext* context, const MonitorRequest* monitorRequest,
                 ServerWriter<MonitorReply>* writer);
  // testing wrapper of the monitor, returns reply to the client call
  std::vector<Chirp> monitorHandler() {
    initialmonitor_ =
        false;  // set initial monitor to false for future monitor calls
    return newchirps_;
  }
  // check if a user exists
  Status login(ServerContext* context, const RegisterRequest* registerRequest,
               RegisterReply* registerReply) override;
  // returns followed user list for testing purpose of the `username`
  std::optional<std::vector<std::string> > followedusernames(
      std::string username);

 private:
  StorageClient storageclient_;
  static const std::string kChirpIDKey;
  static const std::string kUserIDKey;
  static const std::string kUsernameEntryKeyPrefix;
  static const std::string kFollowingUserEntryKeyPrefix;
  static const std::string kChirpEntryKeyPrefix;
  static const std::string kChirpReplyEntryKeyPrefix;
  static const std::string kUserChirpEntryKeyPrefix;
  static const std::string kTagListKeyPrefix;
  static const int kPullingIntervalSeconds;
  // if this object is for testing
  bool testing_ = false;
  // for testing monitor behavior, stores posted chirps when last called monitor
  std::map<std::string, std::string> previouschirplists_;
  // for testing monitor behavior, stores the added chirps since last monitor
  std::vector<Chirp> newchirps_;
  // for testing monitor, if this is the first call to monitor
  // if it is, do not load from `previouschirplist_`, else load
  bool initialmonitor_ = true;
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

  // helper function to get a key to store in KV-store from a tag and a
  // timestamp
  std::string GetKey(const std::string& tag, const Timestamp& time);
  std::string GetKey(const std::string& tag, const uint64_t& second);

  // add a new chirp id to a specific tag list
  // here I used tag name and chirps' time as they key to store in the KV-store
  void AddToTagList(const std::string& tag, const Timestamp& time,
                    const std::string& chirp_id);

  // retrieve all chirp ids with a specific tag from a specific timestamp
  ServiceData::TagList GetChirpsByTagFromTime(const std::string& tag,
                                              const Timestamp& from);
};

#endif  // CHIRP_SERVICE_H_