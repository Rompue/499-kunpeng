HOST_SYSTEM = $(shell uname | cut -f 1 -d_)
SYSTEM ?= $(HOST_SYSTEM)
CXX = g++
CPPFLAGS += `pkg-config --cflags protobuf grpc`
CXXFLAGS += -std=c++17
TESTFLAGS += -L/usr/local/lib -Lgtest/lib -lgtest -lpthread
ifeq ($(SYSTEM),Darwin)
LDFLAGS += -L/usr/local/lib `pkg-config --libs protobuf grpc++`\
           -lgrpc++_reflection\
           -ldl
else
LDFLAGS += -L/usr/local/lib `pkg-config --libs protobuf grpc++`\
           -Wl,--no-as-needed -lgrpc++_reflection -Wl,--as-needed\
           -ldl
endif
PROTOC = protoc
GRPC_CPP_PLUGIN = grpc_cpp_plugin
GRPC_CPP_PLUGIN_PATH ?= `which $(GRPC_CPP_PLUGIN)`

PROTOS_PATH = ./

vpath %.proto $(PROTOS_PATH)

all: storage_server service chirp

chirp: storage.pb.o storage.grpc.pb.o service.pb.o service.grpc.pb.o service.o storage_client.o storage_server.o cmd.o cmd_main.o thread_safe_map.o
	$(CXX) $^ $(LDFLAGS) -o $@

service: storage.pb.o storage.grpc.pb.o service.pb.o service.grpc.pb.o storage_client.o service.o service_main.o thread_safe_map.o
	$(CXX) $^ $(LDFLAGS) -o $@

storage_client: storage.pb.o storage.grpc.pb.o storage_client.o thread_safe_map.o storage_client_main.o
	$(CXX) $^ $(LDFLAGS) -o $@

storage_server: storage.pb.o storage.grpc.pb.o storage_server.o thread_safe_map.o storage_main.o 
	$(CXX) $^ $(LDFLAGS) -o $@

storage_test: storage.pb.o storage.grpc.pb.o storage_test.o storage_client.o storage_server.o thread_safe_map.o 
	$(CXX) $^ $(LDFLAGS) $(TESTFLAGS) -o $@

service_test: storage.pb.o storage.grpc.pb.o service.pb.o service.grpc.pb.o storage_client.o storage_server.o thread_safe_map.o service.o cmd.o service_test.o
	$(CXX) $^ $(LDFLAGS) $(TESTFLAGS) -o $@

.PRECIOUS: %.grpc.pb.cc
%.grpc.pb.cc: %.proto
	$(PROTOC) -I $(PROTOS_PATH) --grpc_out=. --plugin=protoc-gen-grpc=$(GRPC_CPP_PLUGIN_PATH) $<

.PRECIOUS: %.pb.cc
%.pb.cc: %.proto
	$(PROTOC) -I $(PROTOS_PATH) --cpp_out=. $<

clean:
	rm -f *.o *.pb.cc *.pb.h storage_server service chirp storage_test service_test

