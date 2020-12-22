#ifndef _RPC_SERVER_H
#define _RPC_SERVER_H

#include <string>
#include "StreamServer.h"
#include "google/protobuf/service.h"

namespace toyBasket
{
class RpcServer
{
public:
    RpcServer(EventLoop* loop, const InetAddress& listenAddr);
    virtual ~RpcServer();

    bool Start();
    bool RegisterService(google::protobuf::Service* service, bool ownership);

private:
    void onMessage(const StreamConnectionPtr& conn, Buffer* buff);
    void onConnection(const StreamConnectionPtr& conn);
    void ProcRpcData(const int64_t id, const std::string& service_id, const std::string& method_id,
                     const std::string& serialzied_data);
    void OnCallbackDone(int64_t id, ::google::protobuf::Message* resp_msg);

private:
    struct ServiceInfo
    {
        ::google::protobuf::Service* service_;
        std::map<std::string, const ::google::protobuf::MethodDescriptor*> mdescriptor_;
    };
    std::map<std::string, ServiceInfo> services_;
    std::unique_ptr<StreamServer> server_;
    std::weak_ptr<StreamConnection> basicConn_;
};

}

#endif
