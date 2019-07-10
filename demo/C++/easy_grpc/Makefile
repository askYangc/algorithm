SLIB=libeasygrpc.a

PROTOC_INC = /usr/local/protobuf3.8/include
PROTOC_LIB = /usr/local/protobuf3.8/lib

GRPC_INC = `pkg-config --cflags protobuf grpc`
GRPC_LIB = `pkg-config --libs protobuf grpc++ grpc`

CPPFLAGS += --std=c++11 $(GRPC_INC)

include Makefile.inc
