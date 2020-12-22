/******************************************************************************
 * File name     : UdpMultiCastSender.cpp
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

#include "UdpMultiCastSender.h"
#include "Types.h"

using namespace toyBasket;

UdpMultiCastSender::UdpMultiCastSender(const InetAddress &GroupAddr,
                                       const std::string &nameArg)
    : socket_(::socket(GroupAddr.family(),
                       SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0)),
      groupAddr_(GroupAddr), name_(nameArg) {}

void UdpMultiCastSender::setMulticastIF(const std::string &address) {
  socket_.setMulticastIF(address);
}

void UdpMultiCastSender::setLoopBack(bool on) { socket_.setLoopBack(on); }

void UdpMultiCastSender::setTTL(const unsigned char &ttl) {
  socket_.setTTL(ttl);
}

void UdpMultiCastSender::send(const void *message, int len) {
  ssize_t ret =
      ::sendto(socket_.fd(), message, static_cast<unsigned long long>(len), 0,
               groupAddr_.getSockAddr(), groupAddr_.getSockLen());
  if (ret < 0) {
    LOG_ERROR << name_
              << " UDP MultiCastSener sendto error: " << strerror(errno);
  }
}
