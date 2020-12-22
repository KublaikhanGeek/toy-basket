#include "EventLoop.h"
#include "InetAddress.h"
#include "echo_service.pb.h"
#include "rpc_server.h"
#include <iostream>

using namespace toyBasket;

class EchoServerImpl : public toyBasket::EchoServer {
public:
  EchoServerImpl() {}
  virtual ~EchoServerImpl() {}

private:
  virtual void Echo(google::protobuf::RpcController *controller,
                    const toyBasket::EchoRequest *request,
                    toyBasket::EchoResponse *response,
                    google::protobuf::Closure *done) {
    std::cout << "[Echo] server received client msg: " << request->message()
              << std::endl;
    response->set_message("[Echo] server say: received msg: ***" +
                          request->message() + std::string("***"));
    done->Run();
  }
  virtual void Print(google::protobuf::RpcController *controller,
                     const toyBasket::EchoRequest *request,
                     toyBasket::EchoResponse *response,
                     google::protobuf::Closure *done) {
    std::cout << "[Print] server received client msg: " << request->message()
              << std::endl;
    response->set_message("[Print] server say: received msg: ***" +
                          request->message() + std::string("***"));
    done->Run();
  }
};

int main(int argc, char *argv[]) {
  toyBasket::EventLoop loop;
  InetAddress server("127.0.0.1", 12321);
  toyBasket::RpcServer rpc_server(&loop, server);

  toyBasket::EchoServer *echo_service = new EchoServerImpl();
  if (!rpc_server.RegisterService(echo_service, false)) {
    std::cout << "register service failed" << std::endl;
    return -1;
  }

  rpc_server.Start();

  loop.loop();

  return 0;
}
