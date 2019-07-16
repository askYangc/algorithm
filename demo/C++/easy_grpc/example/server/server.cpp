#include <memory>
#include <iostream>
#include <string>
#include <thread>
#include <stdio.h>
#include <unistd.h>

#include <grpcpp/grpcpp.h>
#include <grpc/support/log.h>

#include "grpc_server.h"
#include "helloworld.grpc.pb.h"

using grpc::Server;
using grpc::ServerAsyncResponseWriter;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerCompletionQueue;
using grpc::Status;
using helloworld::HelloRequest;
using helloworld::HelloReply;
using helloworld::Greeter;

class ServerImpl final {
 public:
  ~ServerImpl() {
    server_->Shutdown();
    // Always shutdown the completion queue after the server.
    cq_->Shutdown();
  }

  // There is no shutdown handling in this code.
  void Run() {
    std::string server_address("0.0.0.0:50051");

    ServerBuilder builder;
    // Listen on the given address without any authentication mechanism.
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    // Register "service_" as the instance through which we'll communicate with
    // clients. In this case it corresponds to an *asynchronous* service.
    builder.RegisterService(&service_);
    // Get hold of the completion queue used for the asynchronous communication
    // with the gRPC runtime.
    cq_ = builder.AddCompletionQueue();
    // Finally assemble the server.
    server_ = builder.BuildAndStart();
    std::cout << "Server listening on " << server_address << std::endl;

    // Proceed to the server's main loop.
    HandleRpcs();
  }
 void test1() {
   printf("----------will sleep 10\n");
   sleep(30);
   printf("------------sleep over, tag: %p\n", cd[12]);
   CallData *d = cd[12];
   d->Proceed();
 }
 void test(){
   tt = std::thread(&ServerImpl::test1, this);
 }

 private:
  // Class encompasing the state and logic needed to serve a request.
  class CallData {
   public:
    // Take in the "service" instance (in this case representing an asynchronous
    // server) and the completion queue "cq" used for asynchronous communication
    // with the gRPC runtime.
    CallData(Greeter::AsyncService* service, ServerCompletionQueue* cq)
        : service_(service), cq_(cq), responder_(&ctx_), status_(CREATE) {
      // Invoke the serving logic right away.
      Proceed();
    }

    void Proceed() {
      if (status_ == CREATE) {
        // Make this instance progress to the PROCESS state.
        status_ = PROCESS;

        // As part of the initial CREATE state, we *request* that the system
        // start processing SayHello requests. In this request, "this" acts are
        // the tag uniquely identifying the request (so that different CallData
        // instances can serve different requests concurrently), in this case
        // the memory address of this CallData instance.
        printf("now Create\n");
        service_->RequestSayHello(&ctx_, &request_, &responder_, cq_, cq_,
                                  this);
      } else if (status_ == PROCESS) {
        // Spawn a new CallData instance to serve new clients while we process
        // the one for this CallData. The instance will deallocate itself as
        // part of its FINISH state.
        
        //new CallData(service_, cq_);
        printf("now PROCESS %s\n", request_.name().c_str());
        // The actual processing.
        std::string prefix("Hello ");
        reply_.set_message(prefix + request_.name());

        // And we are done! Let the gRPC runtime know we've finished, using the
        // memory address of this instance as the uniquely identifying tag for
        // the event.
        status_ = FINISH;
        responder_.Finish(reply_, Status::OK, this);
      } else {
        printf("now finish, %p\n", this);
        GPR_ASSERT(status_ == FINISH);
        // Once in the FINISH state, deallocate ourselves (CallData).
        delete this;
      }
    }

   private:
    // The means of communication with the gRPC runtime for an asynchronous
    // server.
    Greeter::AsyncService* service_;
    // The producer-consumer queue where for asynchronous server notifications.
    ServerCompletionQueue* cq_;
    // Context for the rpc, allowing to tweak aspects of it such as the use
    // of compression, authentication, as well as to send metadata back to the
    // client.
    ServerContext ctx_;

    // What we get from the client.
    HelloRequest request_;
    // What we send back to the client.
    HelloReply reply_;

    // The means to get back to the client.
    ServerAsyncResponseWriter<HelloReply> responder_;

    // Let's implement a tiny state machine with the following states.
    enum CallStatus { CREATE, PROCESS, FINISH };
    CallStatus status_;  // The current serving state.
  };

  // This can be run in multiple threads if needed.
  void HandleRpcs() {
    // Spawn a new CallData instance to serve new clients.
    for(int i = 0; i < 100; i++) {
        cd[i] = new CallData(&service_, cq_.get());
    }
    
    void* tag;  // uniquely identifies a request.
    bool ok;
    ServerCompletionQueue::NextStatus nextStatus;
    gpr_timespec time;
    count = 0;
    while (true) {
      // Block waiting to read the next event from the completion queue. The
      // event is uniquely identified by its tag, which in this case is the
      // memory address of a CallData instance.
      // The return value of Next should always be checked. This return value
      // tells us whether there is any kind of event or cq_ is shutting down.
      time.tv_sec = 1;//设置1秒超时
      time.tv_nsec = 0;
      time.clock_type = GPR_TIMESPAN;
      tag = NULL;
      nextStatus = cq_->AsyncNext(&tag, &ok, time);
      printf("nextStatus: %d, ok:%d\n", nextStatus, ok);
      if(tag) {printf("tag %p\n", tag);}else{printf("tag is NULL\n");}
      if(nextStatus == ServerCompletionQueue::GOT_EVENT) {
        printf("now count: %d\n", count);
        if(count++ > 10) { printf("jujue\n");continue;}
        static_cast<CallData*>(tag)->Proceed();
      }
      //GPR_ASSERT(cq_->Next(&tag, &ok));
      //GPR_ASSERT(ok);

      //static_cast<CallData*>(tag)->Proceed();
    }
  }



  std::unique_ptr<ServerCompletionQueue> cq_;
  Greeter::AsyncService service_;
  std::unique_ptr<Server> server_;
  int count;
  CallData *cd[100];
  std::thread tt;
};

class GreeterServiceImpl :public Greeter::AsyncService {
public:
    Status SayHello(ServerContext* context, const HelloRequest* request, HelloReply* reply) {
        std::string prefix("Hello ");
        reply->set_message(prefix + request->name());
        printf("GreeterServiceImpl\n");
        //return Status::CANCELLED;
        return Status::OK;
    }  

    //Greeter::AsyncService greeter;
};


int main(int argc, char** argv) {
  //ServerImpl server;
  //server.test();
  //server.Run();

    AsyncServer server("0.0.0.0:50051");
    GreeterServiceImpl impl;
    server.RegisterService(&impl);
    server.start();

    //requestfunc must after start()
    RegisterFuncImpl(&server, HelloRequest, HelloReply, GreeterServiceImpl, &impl, SayHello);    

    while(1) {
        sleep(2);
        //break;
    }
    //server.stop();

    return 0;
}
