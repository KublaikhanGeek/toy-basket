/******************************************************************************
 * File name     : Socket.h
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

#ifndef _SOCKET_H
#define _SOCKET_H

#include <string>
#include <sys/types.h>


#include "Types.h"
#include "noncopyable.h"


// struct tcp_info is in <netinet/tcp.h>
struct tcp_info;

namespace toyBasket {
///
/// TCP networking.
///
class InetAddress;

///
/// Wrapper of socket file descriptor.
///
/// It closes the sockfd when desctructs.
/// It's thread safe, all operations are delagated to OS.
class Socket : noncopyable {
public:
  explicit Socket(int sockfd)
      : sockfd_(sockfd), shutWR_(false), unDomain_("") {}

  // Socket(Socket&&) // move constructor in C++11
  ~Socket();

  int fd() const { return sockfd_; }
  // return true if success.
  bool getTcpInfo(struct tcp_info *) const;
  bool getTcpInfoString(char *buf, int len) const;

  /// abort if address in use
  void bindAddress(const InetAddress &localaddr);
  void bindAddress(const struct sockaddr *localaddr);
  /// abort if address in use
  void listen();

  /// On success, returns a non-negative integer that is
  /// a descriptor for the accepted socket, which has been
  /// set to non-blocking and close-on-exec. *peeraddr is assigned.
  /// On error, -1 is returned, and *peeraddr is untouched.
  int accept(InetAddress &peeraddr);
  int accept(struct sockaddr *peeraddr);

  ssize_t read(void *buf, size_t len);
  ssize_t readv(const struct iovec *iov, int iovcnt);
  ssize_t write(const void *buf, size_t len);

  ssize_t send(const void *buf, size_t len, int flags = 0); // flags: send()
  ssize_t recv(void *buf, size_t len, int flags = 0);       // flags: recv()

  void close();

  ///
  /// UNIX datagram (DGRAM) sockets I/O
  ///
  ssize_t sendto(const void *buf, size_t len, const char *path,
                 int sendto_flags = 0);
  ssize_t sendto(const void *buf, size_t len, const std::string &path,
                 int sendto_flags = 0);
  ssize_t sendto(const std::string &buf, const std::string &path,
                 int sendto_flags = 0);

  ssize_t recvfrom(void *buf, size_t len, char *source, size_t source_len,
                   int recvfrom_flags = 0);
  ssize_t recvfrom(void *buf, size_t len, std::string &source,
                   int recvfrom_flags = 0);
  ssize_t recvfrom(std::string &buf, std::string &source,
                   int recvfrom_flags = 0);

  ///
  /// UDP/IP sockets I/O
  ///
  ssize_t sendto(const void *buf, size_t len, const InetAddress &dst,
                 int sndto_flags = 0); // flags: sendto()
  ssize_t sendto(const std::string &buf, const InetAddress &dst,
                 int sndto_flags = 0);

  ssize_t recvfrom(void *buf, size_t len, InetAddress &src,
                   int rcvfrom_flags = 0);

  ssize_t recvfrom(std::string &buf, InetAddress &src, int rcvfrom_flags = 0);

  void shutdownWrite();

  ///
  /// Enable/disable TCP_NODELAY (disable/enable Nagle's algorithm).
  ///
  void setTcpNoDelay(bool on);

  ///
  /// Enable/disable SO_REUSEADDR
  ///
  void setReuseAddr(bool on);

  ///
  /// Enable/disable SO_REUSEPORT
  ///
  void setReusePort(bool on);

  ///
  /// Enable/disable SO_KEEPALIVE
  ///
  void setKeepAlive(bool on);

  ///
  /// set IP_MULTICAST_IF
  ///
  void setMulticastIF(const std::string &address);

  ///
  /// Enable/disable IP_MULTICAST_LOOP
  ///
  void setLoopBack(bool on);

  ///
  /// set IP_MULTICAST_TTL
  ///
  void setTTL(const unsigned char &ttl);

  ///
  /// set IP_ADD_MEMBERSHIP
  ///
  void setAddMemberShip(struct ip_mreq &mreq);

private:
  const int sockfd_;
  bool shutWR_;
  std::string unDomain_;
};

} // namespace toyBasket

#endif // _SOCKET_H
