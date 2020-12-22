/******************************************************************************
 * File name     : StreamClient.cpp
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

#include "StreamClient.h"

#include "Connector.h"
#include "EventLoop.h"
#include "Timer.h"

#include <cstdio> // snprintf

using namespace toyBasket;

// StreamClient::StreamClient(EventLoop* loop)
//   : loop_(loop)
// {
// }

// StreamClient::StreamClient(EventLoop* loop, const std::string& host, unsigned
// short port)
//   : loop_(CHECK_NOTNULL(loop)),
//     serverAddr_(host, port)
// {
// }

namespace detail {
void removeConnection(EventLoop *loop, const StreamConnectionPtr &conn) {
  loop->queueInLoop(std::bind(&StreamConnection::connectDestroyed, conn));
}

void removeConnector(const ConnectorPtr &connector) {
  // connector->
}

} // namespace detail

StreamClient::StreamClient(EventLoop *loop, const InetAddress &serverAddr,
                           const std::string &nameArg)
    : loop_(CHECK_NOTNULL(loop)), connector_(new Connector(loop, serverAddr)),
      name_(nameArg), connectionCallback_(defaultConnectionCallback),
      messageCallback_(defaultMessageCallback), retry_(false), connect_(true),
      nextConnId_(1) {
  connector_->setNewConnectionCallback(
      std::bind(&StreamClient::newConnection, this, _1));
  // FIXME setConnectFailedCallback
  LOG_INFO << "StreamClient::StreamClient[" << name_ << "] - connector "
           << get_pointer(connector_);
}

StreamClient::~StreamClient() {
  LOG_INFO << "StreamClient::~StreamClient[" << name_ << "] - connector "
           << get_pointer(connector_);
  StreamConnectionPtr conn;
  bool unique = false;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    unique = connection_.unique();
    conn = connection_;
  }
  if (conn) {
    assert(loop_ == conn->getLoop());
    // FIXME: not 100% safe, if we are in different thread
    CloseCallback cb = std::bind(&detail::removeConnection, loop_, _1);
    loop_->runInLoop(std::bind(&StreamConnection::setCloseCallback, conn, cb));
    if (unique) {
      conn->forceClose();
    }
  } else {
    connector_->stop();
    // FIXME: HACK
    TimerManager::getInstance()->addTimer(
        1000, std::bind(&detail::removeConnector, connector_));
  }
}

void StreamClient::connect() {
  // FIXME: check state
  LOG_INFO << "StreamClient::connect[" << name_ << "] - connecting to "
           << connector_->serverAddress().toString();
  connect_ = true;
  connector_->start();
}

void StreamClient::disconnect() {
  connect_ = false;

  {
    std::lock_guard<std::mutex> lock(mutex_);
    if (connection_) {
      connection_->shutdown();
    }
  }
}

void StreamClient::stop() {
  connect_ = false;
  connector_->stop();
}

void StreamClient::newConnection(int sockfd) {
  loop_->assertInLoopThread();

  struct sockaddr_un addr = {};
  memset(&addr, 0, sizeof(addr));
  socklen_t addrlen = static_cast<socklen_t>(sizeof(addr));
  if (::getpeername(sockfd, reinterpret_cast<sockaddr *>(&addr), &addrlen) <
      0) {
    LOG_ERROR << "get PeerAddr error: " << strerror(errno);
  }

  InetAddress peerAddr(addr);
  char buf[32];
  snprintf(buf, sizeof(buf), ":%s#%d", peerAddr.toString().c_str(),
           nextConnId_);
  ++nextConnId_;
  std::string connName = name_ + buf;

  memset(&addr, 0, sizeof(addr));
  if (::getsockname(sockfd, reinterpret_cast<sockaddr *>(&addr), &addrlen) <
      0) {
    LOG_ERROR << "get LocalAddr error: " << strerror(errno);
  }

  InetAddress localAddr(addr);
  // FIXME poll with zero timeout to double confirm the new connection
  // FIXME use make_shared if necessary
  StreamConnectionPtr conn(
      new StreamConnection(loop_, connName, sockfd, localAddr, peerAddr));

  conn->setConnectionCallback(connectionCallback_);
  conn->setMessageCallback(messageCallback_);
  conn->setWriteCompleteCallback(writeCompleteCallback_);
  conn->setCloseCallback(
      std::bind(&StreamClient::removeConnection, this, _1)); // FIXME: unsafe
  {
    std::lock_guard<std::mutex> lock(mutex_);
    connection_ = conn;
  }
  conn->connectEstablished();
}

void StreamClient::removeConnection(const StreamConnectionPtr &conn) {
  loop_->assertInLoopThread();
  assert(loop_ == conn->getLoop());

  {
    std::lock_guard<std::mutex> lock(mutex_);
    assert(connection_ == conn);
    connection_.reset();
  }

  loop_->queueInLoop(std::bind(&StreamConnection::connectDestroyed, conn));
  if (retry_ && connect_) {
    LOG_INFO << "StreamClient::connect[" << name_ << "] - Reconnecting to "
             << connector_->serverAddress().toString();
    connector_->restart();
  }
}
