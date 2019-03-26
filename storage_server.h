#ifndef CHIRP_STORAGE_SERVER_H_
#define CHIRP_STORAGE_SERVER_H_

#include <grpc/grpc.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include "storage.grpc.pb.h"
#include "thread_safe_map.h"

using chirp::DeleteReply;
using chirp::DeleteRequest;
using chirp::GetReply;
using chirp::GetRequest;
using chirp::KeyValueStore;
using chirp::PutReply;
using chirp::PutRequest;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using grpc::Status;
using grpc::StatusCode;

// The backend storage server for the CS499 chirp project.
// Receive remote call to store key-value pair to internal thread-safe map.
class StorageImpl final : public KeyValueStore::Service {
 public:
  explicit StorageImpl() : map_() {}
  // store the key value pair in the putrequest to `map_`
  Status put(ServerContext* context, const PutRequest* putrequest,
             PutReply* putreply) override;
  // retrieve the stream of values under the stream of keys in getrequest in the
  // `stream`
  Status get(ServerContext* context,
             ServerReaderWriter<GetReply, GetRequest>* stream);
  // delete the key value pair of the key specified in `deleterequest` from
  // `map_`
  Status deletekey(ServerContext* context, const DeleteRequest* deleterequest,
                   DeleteReply* deletereply) override;

 private:
  ThreadSafeMap map_;
};

#endif  // CHIRP_STORAGE_SERVER_H_