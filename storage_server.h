/*
 * Kun Peng
 * The backend storage server for the CS499 chirp project.
 * Receive remote call to store key-value pair to internal thread-safe map.
 */
#include <grpc/grpc.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include <grpcpp/security/server_credentials.h>
#include "storage.grpc.pb.h"
#include "thread_safe_map.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using grpc::Status;
using chirp::PutRequest;
using chirp::PutReply;
using chirp::GetRequest;
using chirp::GetReply;
using chirp::DeleteRequest;
using chirp::DeleteReply;
using chirp::KeyValueStore;


class StorageImpl final : public KeyValueStore::Service {
  public:
    explicit StorageImpl() : map_() {}

    Status put(ServerContext* context, const PutRequest* putRequest, PutReply* putReply) override;

    Status get(ServerContext* context, ServerReaderWriter<GetReply, GetRequest>* stream);

    Status deletekey(ServerContext* context, const DeleteRequest* deleteRequest,
                     DeleteReply* deleteReply) override;

  private:
    ThreadSafeMap map_;
};