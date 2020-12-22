/******************************************************************************
 * File name     : Acceptor.h
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

#ifndef _ACCEPTOR_H
#define _ACCEPTOR_H

#include <functional>

#include "Channel.h"
#include "Socket.h"

namespace toyBasket {

class EventLoop;
class InetAddress;

///
/// Acceptor of incoming TCP connections.
///
class Acceptor : noncopyable {
public:
  typedef std::function<void(int sockfd, const InetAddress &)>
      NewConnectionCallback;

  Acceptor(EventLoop *loop, const InetAddress &listenAddr, bool reuseport);
  ~Acceptor();

  void setNewConnectionCallback(const NewConnectionCallback &cb) {
    newConnectionCallback_ = cb;
  }

  bool listenning() const { return listenning_; }
  void listen();

private:
  void handleRead();

  EventLoop *loop_;
  Socket acceptSocket_;
  Channel acceptChannel_;
  NewConnectionCallback newConnectionCallback_;
  bool listenning_;
  int idleFd_;
};

} // namespace toyBasket

#endif // _ACCEPTOR_H
