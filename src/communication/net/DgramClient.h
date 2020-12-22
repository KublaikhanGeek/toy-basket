/******************************************************************************
 * File name     : DgramClient.h
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
#ifndef _DGRAMCLIENT_H
#define _DGRAMCLIENT_H

#include "Buffer.h"
#include "Callbacks.h"
#include "Channel.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "Socket.h"

namespace toyBasket {

class DgramClient : noncopyable {
public:
  // DgramClient(EventLoop* loop);
  // DgramClient(EventLoop* loop, const string& host, unsigned short port);
  DgramClient(EventLoop *loop, const InetAddress &serverAddr,
              const std::string &nameArg,
              const InetAddress &clientAddr = InetAddress(""));

  ~DgramClient(); // force out-line dtor, for std::unique_ptr members.

  EventLoop *getLoop() const { return loop_; }

  const std::string &name() const { return name_; }

  /// Set message callback.
  /// Not thread safe.
  void setMessageCallback(DgramEventCallback cb) {
    messageCallback_ = std::move(cb);
  }

  void send(const void *message, int len);
  void connect(const InetAddress &serverAddr);

private:
  void handleRead();
  void stop();

private:
  EventLoop *loop_;
  const std::string name_;
  const InetAddress serverAddr_;
  bool connected_;
  Socket socket_;
  Channel channel_;
  DgramEventCallback messageCallback_;
  Buffer inputBuffer_;
};

} // namespace toyBasket

#endif // _DGRAMCLIENT_H
