/******************************************************************************
 * File name     : InetAddress.h
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

#ifndef _INETADDRESS_H
#define _INETADDRESS_H

#include "StringPiece.h"
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>

namespace toyBasket {
///
/// Wrapper of sockaddr_in.
///
/// This is an POD interface class.
class InetAddress {
public:
  /// Constructs an endpoint with given port number.
  /// Mostly used in TcpServer listening.
  explicit InetAddress(unsigned short port = 0, bool loopbackOnly = false,
                       bool ipv6 = false);

  /// Constructs an endpoint with given ip and port.
  /// @c ip should be "1.2.3.4"
  InetAddress(StringArg ip, unsigned short port, bool ipv6 = false);

  explicit InetAddress(const char *un_domain);
  InetAddress(const StringArg &un_domain);

  /// Constructs an endpoint with given struct @c sockaddr_in
  /// Mostly used when accepting new connections
  explicit InetAddress(const struct sockaddr_in &addr) : addr_in_(addr) {}

  explicit InetAddress(const struct sockaddr_in6 &addr) : addr_in6_(addr) {}

  explicit InetAddress(const struct sockaddr_un &addr) : addr_un_(addr) {}

  sa_family_t family() const { return addr_in_.sin_family; }
  std::string address() const;
  std::string toString() const;
  unsigned short toPort() const;

  // default copy/assignment are Okay

  const struct sockaddr *getSockAddr() const {
    return reinterpret_cast<const struct sockaddr *>(&addr_un_);
  }
  socklen_t getSockLen() const;
  void setSockAddr(const struct sockaddr_un &addr) { addr_un_ = addr; }

  unsigned int ipNetEndian() const;
  unsigned short portNetEndian() const;

  // resolve hostname to IP address, not changing port or sin_family
  // return true on success.
  // thread safe
  static bool resolve(const std::string &hostname, InetAddress *out);
  // static std::vector<InetAddress> resolveAll(const char* hostname, unsigned
  // short port = 0);

  // set IPv6 ScopeID
  void setScopeId(unsigned int scope_id);

  bool operator<(const InetAddress &other) const {
    return toString() < other.toString();
  }

private:
  union {
    struct sockaddr_in addr_in_;
    struct sockaddr_in6 addr_in6_;
    struct sockaddr_un addr_un_;
  };
};

} // namespace toyBasket

#endif // _INETADDRESS_H
