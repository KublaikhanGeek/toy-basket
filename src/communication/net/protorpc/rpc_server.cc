#include "rpc_server.h"
#include "rpc_meta.pb.h"
#include <arpa/inet.h>

namespace toyBasket {
RpcServer::RpcServer(EventLoop *loop, const InetAddress &listenAddr)
    : server_(new StreamServer(loop, listenAddr, "RpcServer")) {
  server_->setMessageCallback(std::bind(&RpcServer::onMessage, this, _1, _2));
  server_->setConnectionCallback(std::bind(&RpcServer::onConnection, this, _1));
}
RpcServer::~RpcServer() {}

bool RpcServer::Start() {
  // basic server
  server_->start();
  return true;
}

bool RpcServer::RegisterService(google::protobuf::Service *service,
                                bool ownership) {
  ServiceInfo service_info;
  std::string method_id;
  const google::protobuf::ServiceDescriptor *sdescriptor =
      service->GetDescriptor();
  for (int i = 0; i < sdescriptor->method_count(); ++i) {
    method_id = sdescriptor->method(i)->name();
    service_info.mdescriptor_[method_id] = sdescriptor->method(i);
  }

  service_info.service_ = service;
  services_[sdescriptor->name()] = service_info;
  return true;
}

void RpcServer::onMessage(const StreamConnectionPtr &conn, Buffer *buff) {
  while (buff->readableBytes() > sizeof(int32_t)) {
    const int32_t len = buff->peekInt32();
    if (buff->readableBytes() > (len + sizeof(int32_t))) {
      std::string meta_data(buff->peek() + sizeof(int32_t), len);
      RpcMeta meta_proto;
      meta_proto.ParseFromString(meta_data);
      int length = sizeof(int32_t) + len + meta_proto.data_size();
      if (buff->readableBytes() >= length) {
        ProcRpcData(meta_proto.id(), meta_proto.server(), meta_proto.method(),
                    std::string(buff->peek() + len + sizeof(int32_t),
                                meta_proto.data_size()));
        buff->retrieve(length);
        continue;
      } else {
        break;
      }
    } else {
      break;
    }
  }
}

void RpcServer::onConnection(const StreamConnectionPtr &conn) {
  if (conn->connected()) {
    basicConn_ = conn;
  }
}

void RpcServer::ProcRpcData(const int64_t id, const std::string &service_id,
                            const std::string &method_id,
                            const std::string &serialzied_data) {
  auto service = services_[service_id].service_;
  auto mdescriptor = services_[service_id].mdescriptor_[method_id];
  auto recv_msg = service->GetRequestPrototype(mdescriptor).New();
  auto resp_msg = service->GetResponsePrototype(mdescriptor).New();
  recv_msg->ParseFromString(serialzied_data);
  auto done = google::protobuf::NewCallback(this, &RpcServer::OnCallbackDone,
                                            id, resp_msg);

  service->CallMethod(mdescriptor, NULL, recv_msg, resp_msg, done);
}

void RpcServer::OnCallbackDone(int64_t id,
                               ::google::protobuf::Message *resp_msg) {
  int serialized_size = resp_msg->ByteSize();
  RpcMeta rpc_meta;
  rpc_meta.set_id(id);
  rpc_meta.set_data_size(serialized_size);

  std::string rpc_meta_str;
  rpc_meta.SerializeToString(&rpc_meta_str);
  int32_t meta_size = htonl(rpc_meta_str.size());
  std::string resp_data;
  resp_data.append(reinterpret_cast<const char *>(&meta_size), sizeof(int32_t));
  resp_data += rpc_meta_str;
  resp_msg->AppendToString(&resp_data);

  // resp_msg->SerializeToString(&resp_data);
  StreamConnectionPtr basicPtr = basicConn_.lock();
  if (basicPtr) {
    basicPtr->send(resp_data);
  }
}

} // namespace toyBasket