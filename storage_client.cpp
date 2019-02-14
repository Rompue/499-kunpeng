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

Status StorageClient::put(const std::string& key, const std::string& value) {
  PutRequest putRequest = MakePutRequest(key, value);
  PutReply putReply;
  ClientContext context;
  Status status = stub_->put(&context, putRequest, &putReply);
  return status;
  if (status.ok()) {
    // TODO decide what should return in this situation
  } else {
    // TODO decide what should return in this situation
  }
}

std::vector<std::string> StorageClient::get(const std::vector<std::string>& keys) {
  ClientContext context;

  std::shared_ptr<ClientReaderWriter<GetRequest, GetReply> > stream(
      stub_->get(&context));

  std::thread writer([stream, &keys, this]() {
    for (const std::string& key : keys) {
      stream->Write(MakeGetRequest(key));
    }
    stream->WritesDone();
  });

  GetReply getReply;
  std::vector<std::string> getReplys;
  while (stream->Read(&getReply)) {
    // if (getReply.)
    getReplys.push_back(getReply.value());
  }
  writer.join();
  Status status = stream->Finish();
  return getReplys;
  if (!status.ok()) {
    std::cout << "get failed" << std::endl;
    // TODO decide what should return in this situation
  } else {
    // TODO decide what should return in this situation
  }
}

std::string StorageClient::get(const std::string& key) {
  std::vector<std::string> keys;
  keys.push_back(key);
  std::vector<std::string> result = StorageClient::get(keys);
  if (!result.empty()) return result[0];
}



std::optional<std::string> StorageClient::has(const std::string& key) {
  ClientContext context;

  std::shared_ptr<ClientReaderWriter<GetRequest, GetReply> > stream(
      stub_->get(&context));

  std::thread writer([stream, &key, this]() {
    stream->Write(MakeGetRequest(key));
    stream->WritesDone();
  });

  GetReply getReply;
  std::string returnstr;
  while (stream->Read(&getReply)) {
    returnstr = getReply.value();
  }
  writer.join();
  Status status = stream->Finish();
  if (!status.ok()) {
    return {};
  } else {
    return returnstr;
  }
}


Status StorageClient::deletekey(const std::string& key) {
  DeleteRequest deleteRequest = MakeDeleteRequest(key);
  DeleteReply deleteReply;
  ClientContext context;
  Status status = stub_->deletekey(&context, deleteRequest, &deleteReply);
  return status;
  if (status.ok()) {
    // TODO decide what should return in this situation
  } else {
    // TODO decide what should return in this situation
  }
}
