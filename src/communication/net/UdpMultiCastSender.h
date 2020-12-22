/******************************************************************************
 * File name     : UdpMultiCastSender.h
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

#ifndef _UDPMULTICASTSENDER_H
#define _UDPMULTICASTSENDER_H

#include "InetAddress.h"
#include "Socket.h"
#include "noncopyable.h"


namespace toyBasket {

class UdpMultiCastSender : noncopyable {
public:
  UdpMultiCastSender(const InetAddress &GroupAddr, const std::string &nameArg);
  ~UdpMultiCastSender() = default;

  const std::string &name() const { return name_; }

  void setMulticastIF(const std::string &address);
  void setLoopBack(bool on);
  void setTTL(const unsigned char &ttl);

  void send(const void *message, int len);

private:
  Socket socket_;
  InetAddress groupAddr_;
  const std::string name_;
};

} // namespace toyBasket

#endif // _UDPMULTICASTSENDER_H
