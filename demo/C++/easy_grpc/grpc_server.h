#ifndef _GRPC_SERVER_H_
#define _GRPC_SERVER_H_

#include <stdio.h>
#include <grpcpp/grpcpp.h>
#include <grpc/support/log.h>

#include <boost/bind.hpp>
#include <boost/function.hpp>

using grpc::Server;
using grpc::ServerAsyncResponseWriter;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::CompletionQueue;
using grpc::ServerCompletionQueue;
using grpc::Status;

template<typename K, typename T>
using RequestFunc = boost::function<void (ServerContext*, K*, ServerAsyncResponseWriter<T>*, CompletionQueue*, ServerCompletionQueue*, void*)>;


template<typename K, typename T>
using CallRpcFunc = boost::function<Status (ServerContext*, const K*, T*)>;

#define JoinRequestFunc(Service, FB) &Service::AsyncService::Request##FB

#define RegisterFuncImpl(asyncServer_, K, T, ClassService, asyncService, ClassRpcServiceImpl, func, this) do {\
    RequestFunc<K, T> rb = boost::bind(JoinRequestFunc(ClassService, func), asyncService, _1, _2, _3, _4, _5, _6);\
    CallRpcFunc<K, T> cb = boost::bind(&ClassRpcServiceImpl::func, this, _1, _2, _3);\
    (asyncServer_)->RegisterFunc<K,T>(rb, cb);\
}while(0)

#define RegisterFuncImplN(asyncServer_, K, T, ClassService, asyncService, ClassRpcServiceImpl, func, this, count) do {\
    RequestFunc<K, T> rb = boost::bind(JoinRequestFunc(ClassService, func), asyncService, _1, _2, _3, _4, _5, _6);\
    CallRpcFunc<K, T> cb = boost::bind(&ClassRpcServiceImpl::func, this, _1, _2, _3);\
    (asyncServer_)->RegisterFunc<K,T>(rb, cb, count);\
}while(0)


class FuncImplBase {
public:
    virtual void callRequestFunc() {}
    virtual void callRpcFunc() {}
};

template<typename K, typename T>
class FuncImpl : public FuncImplBase{
public:
    explicit FuncImpl(ServerCompletionQueue *q):cq_(q) {
        init();
    }
    ~FuncImpl() {
        clear();
    }

    void init() {
        ctx_ = new ServerContext();
        responder_ = new ServerAsyncResponseWriter<T>(ctx_);
    }

    void clear() {
        delete responder_;
        delete ctx_;
    }
    
    void recycling() {
        clear();
        init();
    }

    void setRequestFunc(RequestFunc<K,T> f) {
        requestFunc_ = f;
    }

    void callRequestFunc() {
        if(!requestFunc_.empty()) {
            requestFunc_(ctx_, &request_, responder_, cq_, cq_, this);
        }
        status_ = PROCESS;
    }

    void setCallRpcFunc(CallRpcFunc<K,T> f) {
        callRpcFunc_ = f;
    }

    void callRpcFunc() {
        if(status_ == PROCESS) {
            if(!callRpcFunc_.empty()) {
                Status st = callRpcFunc_(ctx_, &request_, &reply_);
                responder_->Finish(reply_, st, this);
            }
            status_ = FINISH;
        }else {
            recycling();
            callRequestFunc();
        }
    }    
    
public:
    RequestFunc<K,T> requestFunc_;
    CallRpcFunc<K,T> callRpcFunc_;
    ServerCompletionQueue *cq_;
    ServerContext *ctx_;
    K request_;
    T reply_;
    ServerAsyncResponseWriter<T> *responder_;
    enum CallStatus { PROCESS = 1, FINISH = 2, };
    CallStatus status_;  // The current serving state.
};

class AsyncServer {
public:
    explicit AsyncServer(std::string addr, int threads_count = 0):server_addr(addr),threads_count_(threads_count) { init();}
    ~AsyncServer() {
        if(running) {
            server_->Shutdown();
            cq_->Shutdown();
            for(int i = 0; i < threads_count_; i++) {
                thread_[i].join();
            }
            running = false;
        }
    }

    void init() {
        if(threads_count_ == 0) {
            threads_count_ = 1;
        }
        builder.AddListeningPort(server_addr, grpc::InsecureServerCredentials());
        cq_ = builder.AddCompletionQueue();
    }

    void RegisterService(grpc::Service *service) {
        builder.RegisterService(service);
    }

    template<typename K, typename T>
    void RegisterFunc(RequestFunc<K, T> rb, CallRpcFunc<K, T> cb, int count = 10) {
        for(int i = 0; i < count; i++) {
            FuncImpl<K, T> *impl = new FuncImpl<K, T>(cq_.get());
            impl->setRequestFunc(rb);
            impl->setCallRpcFunc(cb);
            impl->callRequestFunc();
        }
    }

    void start() {
        server_ = builder.BuildAndStart();
        running = startThread();
    }


    bool startThread() {
        for(int i = 0; i < threads_count_; i++) {
           thread_[i] = std::thread(&AsyncServer::rpcProc, this);
        }
        return true;
    }
    

    void stop() {
        running = false;
    }

    void rpcProc() {
        void* tag;  // uniquely identifies a request.
        bool ok;
        ServerCompletionQueue::NextStatus nextStatus;
        gpr_timespec time;
        while(running) {
            time.tv_sec = 1;//设置1秒超时
            time.tv_nsec = 0;
            time.clock_type = GPR_TIMESPAN;
            tag = NULL;
            ok = false;
            nextStatus = cq_->AsyncNext(&tag, &ok, time);
            if(nextStatus == ServerCompletionQueue::SHUTDOWN) {
                break;
            }
            if(!ok || nextStatus == ServerCompletionQueue::TIMEOUT) {
                continue;
            }

            if(nextStatus == ServerCompletionQueue::GOT_EVENT) {
                static_cast<FuncImplBase*>(tag)->callRpcFunc();
            }
        }
    }

private:
    std::string server_addr;
    bool running;
    int threads_count_;

    std::thread thread_[16];
    ServerBuilder builder;
    std::unique_ptr<ServerCompletionQueue> cq_;
    std::unique_ptr<Server> server_;
};


#endif
