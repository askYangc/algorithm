#include "rpc_service_client.h"
#include "helloworld.grpc.pb.h"

using helloworld::HelloRequest;
using helloworld::HelloReply;
using helloworld::Greeter;

void local_getHello(AsyncClientTask *task) {
    printf("local_getHello\n");
    ClientTask<HelloReply> clientTask(task);
    //AsyncClientTaskT<HelloReply> *t = static_cast<AsyncClientTaskT<HelloReply> *>(task);
    //printf("local_getHello %s\n", clientTask.getReply().message().c_str());
    if(clientTask.getStatus().ok()) {
        printf("local_getHello %s\n", clientTask.getReply().message().c_str());
    }else {
        printf("RPC failed\n");
    }
}


class GreeterImpl : public AsyncService<Greeter> {
public:
    GreeterImpl(AsyncClient &client):AsyncService<Greeter>(client) {}
    int SayHello(std::string name) {
        HelloRequest req;
        req.set_name(name);

        //client_.doRpc<HelloRequest, HelloReply>(req, SetPrepareFunc(Greeter, PrepareAsyncSayHello), SetCbFunc(GreeterImpl::getHello));
        client_.doRpc<HelloRequest, HelloReply>(req, SetPrepareFunc(Greeter, PrepareAsyncSayHello), local_getHello);
        return 0;
    }
  

    void getHello(AsyncClientTask *task) {
        printf("getHello\n");
        ClientTask<HelloReply> clientTask(task);
        //AsyncClientTaskT<HelloReply> *t = static_cast<AsyncClientTaskT<HelloReply> *>(task);
        //printf("local_getHello %s\n", clientTask.getReply().message().c_str());
        if(clientTask.getStatus().ok()) {
            printf("getHello %s\n", clientTask.getReply().message().c_str());
        }else {
            printf("RPC failed\n");
        }
    }
  
private:
    
};

int main()
{   
    AsyncClient client("127.0.0.1:50051");
    GreeterImpl impl(client);    
    impl.SayHello("yangchuan");
    impl.SayHello("yangchuan1");

    sleep(2);

    return 0;
}
