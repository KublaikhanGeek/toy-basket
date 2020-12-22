/******************************************************************************
 * File name     : DgramServer.cpp
 * Description   :
 * Version       : v1.0
 * Create Time   : 2019/8/29
 * Author        : andy
 * Modify history:
 *******************************************************************************
 * Modify Time   Modify person  Modification
 * ------------------------------------------------------------------------------
 *
 *******************************************************************************/

#include "DgramServer.h"
#include "Types.h"

using namespace toyBasket;

DgramServer::DgramServer(EventLoop *loop, const InetAddress &listenAddr,
                         const std::string &nameArg)
    : loop_(CHECK_NOTNULL(loop)),
      socket_(::socket(listenAddr.family(),
                       SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0)),
      channel_(loop, socket_.fd()), serverAddr_(listenAddr),
      ipPort_(listenAddr.toString()), name_(nameArg), started_(0) {
  socket_.bindAddress(serverAddr_);
  channel_.setReadCallback(std::bind(&DgramServer::handleRead, this));
}

DgramServer::~DgramServer() { this->stop(); }

void DgramServer::start() {
  if (started_.exchange(1) == 0) {
    channel_.enableReading();
  }
}

void DgramServer::stop() {
  if (started_.exchange(0) == 1) {
    channel_.disableAll();
    channel_.remove();
  }
}

void DgramServer::send(const InetAddress &clientAddr, const void *message,
                       int len) {
  ssize_t ret =
      ::sendto(socket_.fd(), message, static_cast<unsigned long long>(len), 0,
               clientAddr.getSockAddr(), clientAddr.getSockLen());
  if (ret < 0) {
    LOG_ERROR << "UDP server send to client error: " << strerror(errno);
  }
}

void DgramServer::handleRead() {
  char message[MSG_BUF_SIZE];
  struct sockaddr_un peerAddr = {};
  socklen_t addrLen = sizeof(peerAddr);
  memset(&peerAddr, 0, addrLen);
  ssize_t nr =
      ::recvfrom(socket_.fd(), message, sizeof(message), 0,
                 reinterpret_cast<struct sockaddr *>(&peerAddr), &addrLen);

  if (nr > 0) {
    messageCallback_(InetAddress(peerAddr), message, static_cast<int>(nr));
  } else {
    LOG_ERROR << "recv error: " << strerror(errno);
  }
}
