#include "storage_client.h"
#include <iostream>
#include <vector>
#include <string>


int main(int argc, char const *argv[])
{
	StorageClient storageClient(grpc::CreateChannel("localhost:50000",
                          grpc::InsecureChannelCredentials()));

	storageClient.put("hello", "hi");
	storageClient.put("hi", "hello");
	storageClient.deletekey("hello");
	std::vector<std::string> s;
	s.push_back("hello");
	s.push_back("hi");
	std::cout << storageClient.get(s)[0] << storageClient.get(s)[1] << std::endl;

	return 0;
}