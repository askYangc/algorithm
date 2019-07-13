CPPEXEC=server


PROTOC_INC = /usr/local/protobuf3.8/include
PROTOC_LIB = /usr/local/protobuf3.8/lib
PROTOS_PATH = ../protos/

GRPC_INC = `pkg-config --cflags protobuf grpc`
GRPC_LIB = `pkg-config --libs protobuf grpc++ grpc`

CPPFLAGS += -g -I../../ --std=c++11 $(GRPC_INC)
LDFLAGS += -L$(PROTOC_LIB) -L/usr/local/lib `pkg-config --libs protobuf grpc++ grpc`\
           -pthread\
           -Wl,--no-as-needed -lgrpc++_reflection -Wl,--as-needed\
           -ldl -L../../ -leasygrpc


include ../../Makefile.inc
