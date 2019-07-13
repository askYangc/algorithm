将grpc的C++教程简单封装，没有处理流式rpc。

client需要注意的地方


1，需要实现一个继承service的类，如
同步为
class GreeterImpl2 : public SyncService<Greeter> {}


异步为
class GreeterImpl : public AsyncService<Greeter> {}

2，调用同步rpc的时候，调用方式为
    
    
    SyncClientTask<HelloReply> *task = doRpc<HelloRequest, HelloReply>(req, SetSyncRpcFunc(Greeter, SayHello));
    根据task->getStatus().ok()得到返回结果

3，调用异步rpc的时候，有三种调用方式.
        
        //client_.doRpc<HelloRequest, HelloReply>(req, SetPrepareFunc(Greeter, PrepareAsyncSayHello), local_getHello);
        client_.doRpc<HelloRequest, HelloReply>(req, SetPrepareFunc(Greeter, PrepareAsyncSayHello), SetCbFuncN(local_getHello, name));
        //client_.doRpc<HelloRequest, HelloReply>(req, SetPrepareFunc(Greeter, PrepareAsyncSayHello), SetClassCbFuncN(GreeterImpl::getHello, this));
    第一种，设置回调函数为local_hello函数。local_hello函数的参数为void local_getHello(AsyncClientTask *task);
    第二种，设置回调函数用SetCbFuncN宏设置，local_getHello函数的声明为void local_getHello(AsyncClientTask *task, std::string name);
    第三种，设置回调函数用SetClassCbFuncN宏设置，GreeterImpl::getHello的函数声明为void getHello(AsyncClientTask *task);

4，ClientTask<HelloReply> task
    ClientTask用于封装一下应答，数据都存在该应答之中。
    

5，cq_.AsyncNext(&got_tag, &ok, time);
    //客户端的ok为false，自测出来一种情况
    //服务器收到请求后，不执行finish，表明收到了，但是没处理，一直持有。
    //如果出现ok为false的情况，应该提示一下。这种表明服务器忙之类的。
    //及时time超时了，ok也是1，不会出现其他错误。
   
   
    
    
