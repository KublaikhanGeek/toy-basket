#include "EventLoop.h"
#include "InetAddress.h"
#include "Timer.h"
#include "echo_service.pb.h"
#include "rpc_channel.h"
#include <iostream>

using namespace toyBasket;

void print_usage() {
  std::cout << "Use:         echo_client ip port" << std::endl;
  std::cout << "for example: 127.0.0.1 12321" << std::endl;
}

void echoDoneCallback(toyBasket::EchoResponse *response) {
  std::cout << "echo: " << response->DebugString() << std::endl;
}

void PrintDoneCallback(toyBasket::EchoResponse *response) {
  std::cout << "Print: " << response->DebugString() << std::endl;
}

int main(int argc, char *argv[]) {
  if (argc < 3) {
    print_usage();
    return -1;
  }

  TimerManager::getInstance()->asyncWorkStart();

  toyBasket::EchoRequest request;
  toyBasket::EchoResponse response;
  request.set_message("hello tonull, from client");

  char *ip = argv[1];
  char *port = argv[2];
  toyBasket::EventLoop loop;
  InetAddress client(ip, atoi(port));
  toyBasket::RpcChannel rpc_channel(&loop, client);
  rpc_channel.connect();
  toyBasket::EchoServer_Stub stub(&rpc_channel);

  TimerManager::getInstance()->addTimer(
      1000,
      [&] {
        std::cout << "[Echo] echo client call!!!" << std::endl;
        stub.Echo(
            NULL, &request, &response,
            ::google::protobuf::NewCallback(&echoDoneCallback, &response));
      },
      true, false);

  TimerManager::getInstance()->addTimer(
      1000,
      [&] {
        std::cout << "[Print] echo client call!!!" << std::endl;
        stub.Print(
            NULL, &request, &response,
            ::google::protobuf::NewCallback(&PrintDoneCallback, &response));
      },
      true, false);

  loop.loop();

  return 0;
}
