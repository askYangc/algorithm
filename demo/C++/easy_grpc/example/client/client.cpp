#include "grpc_client.h"
#include "helloworld.grpc.pb.h"

using helloworld::HelloRequest;
using helloworld::HelloReply;
using helloworld::Greeter;

void local_getHello(AsyncClientTask *task, std::string name) {
    printf("local_getHello\n");
    ClientTask<HelloReply> clientTask(task);
    //AsyncClientTaskT<HelloReply> *t = static_cast<AsyncClientTaskT<HelloReply> *>(task);
    //printf("local_getHello %s\n", clientTask.getReply().message().c_str());
    if(clientTask.getStatus().ok()) {
        printf("local_getHello %s\n", clientTask.getReply().message().c_str());
    }else {
        printf("local_getHello RPC failed, name: %s\n", name.c_str());
        printf("local_getHello1 %s\n", clientTask.getReply().message().c_str());
    }
}


class GreeterImpl : public AsyncService<Greeter> {
public:
    GreeterImpl(AsyncClient &client):AsyncService<Greeter>(client) {}
    int SayHello(std::string name) {
        HelloRequest req;
        req.set_name(name);

        //client_.doRpc<HelloRequest, HelloReply>(req, SetPrepareFunc(Greeter, PrepareAsyncSayHello), local_getHello);
        client_.doRpc<HelloRequest, HelloReply>(req, SetPrepareFunc(Greeter, PrepareAsyncSayHello), SetCbFuncN(local_getHello, name));
        //client_.doRpc<HelloRequest, HelloReply>(req, SetPrepareFunc(Greeter, PrepareAsyncSayHello), SetClassCbFuncN(GreeterImpl::getHello, this));
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
};

void async_test()
{
    AsyncClient client("127.0.0.1:50051");
    GreeterImpl impl(client);  
    char buf[10] = {0};
    
    for(int i = 0; i < 20; i++) {
        sprintf(buf, "test%d", i);
        impl.SayHello(buf);
        sleep(1);
    }
    
    //impl.SayHello("yangchuan1");
    sleep(50);
    
}

class GreeterImpl2 : public SyncService<Greeter> {
public:
    GreeterImpl2(SyncClient &client):SyncService<Greeter>(client) {}

    int SayHello(std::string name) {
        HelloRequest req;
        req.set_name(name);

        SyncClientTask<HelloReply> *task = doRpc<HelloRequest, HelloReply>(req, SetSyncRpcFunc(Greeter, SayHello));
        if(task->getStatus().ok()) {
            printf("Sync get message: %s\n", task->getReply().message().c_str());
        }else {
            printf("Sync do RPC failed\n");
        }
        delete task;
        return 0;
    }
};

void sync_test()
{
    SyncClient client("127.0.0.1:50051");
    GreeterImpl2 impl(client);  
    impl.SayHello("test");

    
}

int main()
{   
    async_test();
    //sync_test();

    return 0;
}
