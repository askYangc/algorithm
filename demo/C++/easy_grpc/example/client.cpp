#include "rpc_service_client.h"
#include "helloworld.grpc.pb.h"

using helloworld::HelloRequest;
using helloworld::HelloReply;
using helloworld::Greeter;




HelloReply *PrepareAsyncSayHello1(::grpc::ClientContext* context, const HelloRequest& request, ::grpc::CompletionQueue* cq) 
{
    return NULL;
}


class GreeterImpl : public AsyncService<Greeter> {
public:
    GreeterImpl(AsyncClient &client):AsyncService<Greeter>(client) {}
    int SayHello(std::string name) {
        HelloRequest req;
        req.set_name(name);

        //client_.doRpc1<HelloReply>(req, SayHelloFb());
        client_.doRpc<HelloRequest, HelloReply>(req, SayHelloFb());
        return 0;
    }

    PrepareFunc<HelloRequest, HelloReply> SayHelloFb() {
        return boost::bind(&Greeter::Stub::PrepareAsyncSayHello, stub_.get(), _1, _2, _3);
        //return boost::bind(PrepareAsyncSayHello, _1, _2, _3);
    }
  
private:
    
};

int main()
{   
    AsyncClient client("127.0.0.1:5051");
    GreeterImpl impl(client);    


    return 0;
}
