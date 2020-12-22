#include "rpc_channel.h"
#include "rpc_meta.pb.h"
#include <arpa/inet.h>

namespace toyBasket {

RpcChannel::RpcChannel(EventLoop *loop, const InetAddress &serverAddr)
    : client_(new StreamClient(loop, serverAddr, "rpc_channel")), index_(0) {
  client_->setMessageCallback(std::bind(&RpcChannel::onMessage, this, _1, _2));
  client_->setConnectionCallback(
      std::bind(&RpcChannel::onConnection, this, _1));
}

RpcChannel::~RpcChannel() {}

void RpcChannel::connect() { client_->connect(); }

void RpcChannel::CallMethod(const ::google::protobuf::MethodDescriptor *method,
                            ::google::protobuf::RpcController *controller,
                            const ::google::protobuf::Message *request,
                            ::google::protobuf::Message *response,
                            ::google::protobuf::Closure *done) {
  std::string request_data_str;
  request->SerializeToString(&request_data_str);
  // 发送格式: meta_size + meta_data + request_data
  RpcMeta rpc_meta;
  ++index_;
  rpc_meta.set_id(index_);
  rpc_meta.set_server(method->service()->name());
  rpc_meta.set_method(method->name());
  rpc_meta.set_data_size(request_data_str.size());

  std::string rpc_meta_str;
  rpc_meta.SerializeToString(&rpc_meta_str);
  int32_t meta_size = htonl(rpc_meta_str.size());
  std::string serialzied_str;
  serialzied_str.append(reinterpret_cast<const char *>(&meta_size),
                        sizeof(int32_t));
  serialzied_str += rpc_meta_str;
  serialzied_str += request_data_str;

  OutstandingCall out = {response, done};
  {
    std::unique_lock<std::mutex> lock(mutex_);
    out_[index_] = out;
  }

  StreamConnectionPtr conn = conn_.lock();
  if (conn) {
    conn->send(serialzied_str);
  }
}

void RpcChannel::onMessage(const StreamConnectionPtr &conn, Buffer *buff) {
  while (buff->readableBytes() > sizeof(int32_t)) {
    const int32_t len = buff->peekInt32();
    if (buff->readableBytes() > (len + sizeof(int32_t))) {
      std::string meta_data(buff->peek() + sizeof(int32_t), len);
      RpcMeta meta_proto;
      meta_proto.ParseFromString(meta_data);
      int length = sizeof(int32_t) + len + meta_proto.data_size();
      if (buff->readableBytes() >= length) {
        int64_t id = meta_proto.id();
        OutstandingCall out = {NULL, NULL};
        {
          std::unique_lock<std::mutex> lock(mutex_);
          std::map<int64_t, OutstandingCall>::iterator it = out_.find(id);
          if (it != out_.end()) {
            out = it->second;
            out_.erase(it);
          }
        }

        if (out.response) {
          out.response->ParseFromString(std::string(
              buff->peek() + len + sizeof(int32_t), meta_proto.data_size()));
          if (out.done) {
            out.done->Run();
          }
        }

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

void RpcChannel::onConnection(const StreamConnectionPtr &conn) {
  if (conn->connected()) {
    conn_ = conn;
  }
}

} // namespace toyBasket
