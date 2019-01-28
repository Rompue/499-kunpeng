#include "storage_client.h"

PutRequest MakePutRequest(const std::string& key, const std::string& value) {
  PutRequest request;
  request.set_key(key);
  request.set_value(value);
  return request;
}

GetRequest MakeGetRequest(const std::string& key) {
  GetRequest request;
  request.set_key(key);
  return request;
}

DeleteRequest MakeDeleteRequest(const std::string& key) {
  DeleteRequest request;
  request.set_key(key);
  return request;
}

void StorageClient::put(const std::string& key, const std::string& value) {
  PutRequest putRequest = MakePutRequest(key, value);
  PutReply putReply;
  ClientContext context;
  Status status = stub_->put(&context, putRequest, &putReply);
  if (status.ok()) {
    // TODO
  } else {
    // TODO
  }
}

std::vector<std::string> StorageClient::get(const std::vector<std::string>& keys) {
  ClientContext context;

  std::shared_ptr<ClientReaderWriter<GetRequest, GetReply> > stream(
      stub_->get(&context));

  std::thread writer([stream]() {
    for (const std::string& key : keys) {
      stream->Write(MakeGetRequest(key));
    }
    stream->WritesDone();
  });

  GetReply getReply;
  std::vector<std::string> getReplys;
  while (stream->Read(&getReply)) {
    getReplys.push_back(getReply.value());
  }
  writer.join();
  Status status = stream->Finish();
  if (!status.ok()) {
    std::cout << "get failed" << std::endl;
    // TODO
  } else {
    // TODO
  }
}

void StorageClient::deletekey(const std::string& key) {
  DeleteRequest deleteRequest = MakeDeleteRequest(key);
  DeleteReply deleteReply;
  ClientContext context;
  Status status = stub_->Delete(&context, deleteRequest, &deleteReply);
  if (status.ok()) {
    // TODO
  } else {
    // TODO
  }
}


// int main(int argc, char** argv) {
//   // Expect only arg: --db_path=path/to/route_guide_db.json.
//   std::string db = routeguide::GetDbFileContent(argc, argv);
//   RouteGuideClient guide(
//       grpc::CreateChannel("localhost:50051",
//                           grpc::InsecureChannelCredentials()),
//       db);

//   std::cout << "-------------- GetFeature --------------" << std::endl;
//   guide.GetFeature();
//   std::cout << "-------------- ListFeatures --------------" << std::endl;
//   guide.ListFeatures();
//   std::cout << "-------------- RecordRoute --------------" << std::endl;
//   guide.RecordRoute();
//   std::cout << "-------------- RouteChat --------------" << std::endl;
//   guide.RouteChat();

//   return 0;
// }