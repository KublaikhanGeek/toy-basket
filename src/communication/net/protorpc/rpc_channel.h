#ifndef _RPC_CHANNLE_H_
#define _RPC_CHANNLE_H
#include "StreamClient.h"
#include <mutex>
#include <map>
#include "InetAddress.h"
#include "google/protobuf/service.h"

namespace toyBasket
{
class RpcChannel : public google::protobuf::RpcChannel
{
public:
    RpcChannel(EventLoop* loop, const InetAddress& serverAddr);
    virtual ~RpcChannel();

    void connect();
    void CallMethod(const ::google::protobuf::MethodDescriptor* method, ::google::protobuf::RpcController* controller,
                    const ::google::protobuf::Message* request, ::google::protobuf::Message* response,
                    ::google::protobuf::Closure* done) override;

private:
    void onMessage(const StreamConnectionPtr& conn, Buffer* buff);
    void onConnection(const StreamConnectionPtr& conn);

private:
    struct OutstandingCall
    {
        ::google::protobuf::Message* response;
        ::google::protobuf::Closure* done;
    };
    std::unique_ptr<StreamClient> client_;
    std::weak_ptr<StreamConnection> conn_;
    std::mutex mutex_;
    int64_t index_;
    std::map<int64_t, OutstandingCall> out_;
};
}

#endif