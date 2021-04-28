/******************************************************************************
 * File name     : Socket.cpp
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
#include <arpa/inet.h>
#include <cstdio> // snprintf
#include <cstring>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/un.h>

#include "Socket.h"

#include "InetAddress.h"

using namespace toyBasket;

Socket::~Socket()
{
    close();
    if (!unDomain_.empty())
    {
        ::unlink(unDomain_.c_str());
    }
}

bool Socket::getTcpInfo(struct tcp_info* tcpi) const
{
    socklen_t len = sizeof(*tcpi);
    memset(tcpi, 0, len);
    return ::getsockopt(sockfd_, SOL_TCP, TCP_INFO, tcpi, &len) == 0;
}

bool Socket::getTcpInfoString(char* buf, int len) const
{
    struct tcp_info tcpi = {};
    bool ok              = getTcpInfo(&tcpi);
    if (ok)
    {
        snprintf(buf, static_cast<unsigned long long>(len),
                 "unrecovered=%u "
                 "rto=%u ato=%u snd_mss=%u rcv_mss=%u "
                 "lost=%u retrans=%u rtt=%u rttvar=%u "
                 "sshthresh=%u cwnd=%u total_retrans=%u",
                 tcpi.tcpi_retransmits, // Number of unrecovered [RTO] timeouts
                 tcpi.tcpi_rto,         // Retransmit timeout in usec
                 tcpi.tcpi_ato,         // Predicted tick of soft clock in usec
                 tcpi.tcpi_snd_mss, tcpi.tcpi_rcv_mss,
                 tcpi.tcpi_lost,    // Lost packets
                 tcpi.tcpi_retrans, // Retransmitted packets out
                 tcpi.tcpi_rtt,     // Smoothed round trip time in usec
                 tcpi.tcpi_rttvar,  // Medium deviation
                 tcpi.tcpi_snd_ssthresh, tcpi.tcpi_snd_cwnd,
                 tcpi.tcpi_total_retrans); // Total retransmits for entire connection
    }
    return ok;
}

void Socket::bindAddress(const InetAddress& localaddr)
{
    if (localaddr.family() == AF_UNIX)
    {
        unDomain_ = localaddr.address();
        ::unlink(unDomain_.c_str());
    }

    int ret = ::bind(sockfd_, localaddr.getSockAddr(), localaddr.getSockLen());
    if (ret < 0)
    {
        if (localaddr.family() == AF_UNIX)
        {
            ::unlink(unDomain_.c_str());
        }
        LOG_FATAL << "bind address error";
    }
}

void Socket::bindAddress(const struct sockaddr* localaddr)
{
    if (localaddr->sa_family == AF_UNIX)
    {
        const struct sockaddr_un* addrun = reinterpret_cast<const struct sockaddr_un*>(localaddr);
        unDomain_                        = addrun->sun_path;
        ::unlink(unDomain_.c_str());
    }

    socklen_t addrlen = 0;
    if (localaddr->sa_family == AF_INET)
    {
        addrlen = static_cast<socklen_t>(sizeof(struct sockaddr_in));
    }
    else if (localaddr->sa_family == AF_INET6)
    {
        addrlen = static_cast<socklen_t>(sizeof(struct sockaddr_in6));
    }
    else if (localaddr->sa_family == AF_UNIX)
    {
        addrlen = static_cast<socklen_t>(sizeof(struct sockaddr_un));
    }

    int ret = ::bind(sockfd_, localaddr, addrlen);
    if (ret < 0)
    {
        if (localaddr->sa_family == AF_UNIX)
        {
            ::unlink(unDomain_.c_str());
        }
        LOG_FATAL << "bind address error";
    }
}

void Socket::listen()
{
    int ret = ::listen(sockfd_, SOMAXCONN);
    if (ret < 0)
    {
        LOG_FATAL << "listen socket error";
    }
}

int Socket::accept(InetAddress& peeraddr)
{
    struct sockaddr_un addr = {};
    memset(&addr, 0, sizeof(addr));

    socklen_t addrlen = static_cast<socklen_t>(sizeof(addr));
#if 0
    int connfd = ::accept(sockfd_, reinterpret_cast<struct sockaddr *>(&addr), &addrlen);
    // non-block
    int flags = ::fcntl(connfd, F_GETFL, 0);
    flags |= O_NONBLOCK;
    int ret = ::fcntl(connfd, F_SETFL, flags);
    // FIXME check

    // close-on-exec
    flags = ::fcntl(connfd, F_GETFD, 0);
    flags |= FD_CLOEXEC;
    ret = ::fcntl(connfd, F_SETFD, flags);
    // FIXME check
#else
    int connfd = ::accept4(sockfd_, reinterpret_cast<struct sockaddr*>(&addr), &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
#endif
    if (connfd < 0)
    {
        int savedErrno = errno;
        LOG_ERROR << "Socket::accept";
        switch (savedErrno)
        {
        case EAGAIN:
        case ECONNABORTED:
        case EINTR:
        case EPROTO: // ???
        case EPERM:
        case EMFILE: // per-process lmit of open file desctiptor ???
            // expected errors
            errno = savedErrno;
            break;
        case EBADF:
        case EFAULT:
        case EINVAL:
        case ENFILE:
        case ENOBUFS:
        case ENOMEM:
        case ENOTSOCK:
        case EOPNOTSUPP:
            // unexpected errors
            LOG_FATAL << "unexpected error of ::accept : " << strerror(savedErrno);
            break;
        default:
            LOG_FATAL << "unknown error of ::accept : " << strerror(savedErrno);
            break;
        }
    }

    if (connfd >= 0)
    {
        peeraddr.setSockAddr(addr);
    }
    return connfd;
}

int Socket::accept(struct sockaddr* peeraddr)
{
    socklen_t addrlen;
    if (peeraddr->sa_family == AF_INET)
    {
        addrlen = static_cast<socklen_t>(sizeof(struct sockaddr_in));
    }
    else if (peeraddr->sa_family == AF_INET6)
    {
        addrlen = static_cast<socklen_t>(sizeof(struct sockaddr_in6));
    }
    else if (peeraddr->sa_family == AF_UNIX)
    {
        addrlen = static_cast<socklen_t>(sizeof(struct sockaddr_un));
    }

    int connfd = ::accept4(sockfd_, peeraddr, &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (connfd < 0)
    {
        int savedErrno = errno;
        LOG_ERROR << "Socket::accept";
        switch (savedErrno)
        {
        case EAGAIN:
        case ECONNABORTED:
        case EINTR:
        case EPROTO: // ???
        case EPERM:
        case EMFILE: // per-process lmit of open file desctiptor ???
            // expected errors
            errno = savedErrno;
            break;
        case EBADF:
        case EFAULT:
        case EINVAL:
        case ENFILE:
        case ENOBUFS:
        case ENOMEM:
        case ENOTSOCK:
        case EOPNOTSUPP:
            // unexpected errors
            LOG_FATAL << "unexpected error of ::accept : " << strerror(savedErrno);
            break;
        default:
            LOG_FATAL << "unknown error of ::accept : " << strerror(savedErrno);
            break;
        }
    }

    return connfd;
}

ssize_t Socket::read(void* buf, size_t len)
{
    if (sockfd_ == -1)
    {
        LOG_ERROR << "Socket not connected!";
        return -1;
    }

    if (buf == NULL || len == 0)
    {
        LOG_ERROR << "Buffer or length is null!";
        return -1;
    }

    ssize_t readBytes;
    readBytes = ::read(sockfd_, buf, len);

    return readBytes;
}

ssize_t Socket::readv(const struct iovec* iov, int iovcnt)
{
    return ::readv(sockfd_, iov, iovcnt);
}

ssize_t Socket::write(const void* buf, size_t len)
{
    if (shutWR_ == true)
    {
        LOG_ERROR << "Socket has already been shut down!";
        return -1;
    }

    if (sockfd_ == -1)
    {
        LOG_ERROR << "Socket not connected!";
        return -1;
    }

    if (buf == NULL || len == 0)
    {
        LOG_ERROR << "Buffer or length is null!";
        return -1;
    }

    ssize_t writeBytes;
    writeBytes = ::write(sockfd_, buf, len);

    return writeBytes;
}

ssize_t Socket::send(const void* buf, size_t len, int flags) // flags: send()
{
    if (shutWR_ == true)
    {
        LOG_ERROR << "Socket has already been shut down!";
        return -1;
    }

    if (sockfd_ == -1)
    {
        LOG_ERROR << "Socket not connected!";
        return -1;
    }

    if (buf == NULL || len == 0)
    {
        LOG_ERROR << "Buffer or length is null!";
        return -1;
    }

    ssize_t writeBytes;
    writeBytes = ::send(sockfd_, buf, len, flags);

    return writeBytes;
}

ssize_t Socket::recv(void* buf, size_t len, int flags) // flags: recv()
{
    if (sockfd_ == -1)
    {
        LOG_ERROR << "Socket not connected!";
        return -1;
    }

    if (buf == NULL || len == 0)
    {
        LOG_ERROR << "Buffer or length is null!";
        return -1;
    }

    ssize_t readBytes;
    readBytes = ::recv(sockfd_, buf, len, flags);

    return readBytes;
}

void Socket::close()
{
    if (::close(sockfd_) < 0)
    {
        LOG_ERROR << "::close error : " << strerror(errno);
    }
}

///
/// UNIX datagram (DGRAM) sockets I/O
///
ssize_t Socket::sendto(const void* buf, size_t len, const char* path, int sendto_flags)
{
    if (buf == NULL)
    {
        LOG_ERROR << "sendto: Buffer is NULL!";
        return -1;
    }

    ssize_t bytes;
    struct sockaddr_un saddr = {};

    if (strlen(path) > sizeof(saddr.sun_path) - 1)
    {
        LOG_ERROR << "sendto_unix_dgram_socket: UNIX destination socket path too long";
        return -1;
    }

    memset(&saddr, 0, sizeof(struct sockaddr_un));

    saddr.sun_family = AF_UNIX;
    strncpy(saddr.sun_path, path, sizeof(saddr.sun_path) - 1);

    bytes = ::sendto(sockfd_, buf, len, sendto_flags, reinterpret_cast<struct sockaddr*>(&saddr),
                     sizeof(struct sockaddr_un));

    return bytes;
}

ssize_t Socket::sendto(const void* buf, size_t length, const std::string& path, int sendto_flags)
{
    return sendto(buf, length, path.c_str(), sendto_flags);
}

ssize_t Socket::sendto(const std::string& buf, const std::string& path, int sendto_flags)
{
    return sendto(static_cast<const void*>(buf.c_str()), buf.size(), path.c_str(), sendto_flags);
}

ssize_t Socket::recvfrom(void* buf, size_t len, char* source, size_t source_len, int recvfrom_flags)
{
    if (buf == NULL)
    {
        LOG_ERROR << "rcvfrom: Buffer is NULL!";
        return -1;
    }

    ssize_t bytes;
    struct sockaddr_un saddr = {};
    socklen_t socksize       = sizeof(struct sockaddr_un);

    memset(buf, 0, len);
    memset(source, 0, source_len);

    bytes = ::recvfrom(sockfd_, buf, len, recvfrom_flags, reinterpret_cast<struct sockaddr*>(&saddr), &socksize);

    if (source != NULL && source_len > 0)
    {
        memcpy(source, saddr.sun_path, source_len < sizeof(saddr.sun_path) ? source_len : sizeof(saddr.sun_path));
    }

    return bytes;
}

ssize_t Socket::recvfrom(void* buf, size_t len, std::string& source, int recvfrom_flags)
{
    if (buf == NULL)
    {
        LOG_ERROR << "rcvfrom: Buffer is NULL!";
        return -1;
    }

    ssize_t bytes;
    using std::unique_ptr;
    unique_ptr<char[]> source_cstr(new char[108]); // AFAIK, the address field in struct sockaddr_un is
    // only 108 bytes long...
    size_t source_cstr_len;
    struct sockaddr_un saddr = {};
    socklen_t socksize       = sizeof(struct sockaddr_un);

    memset(buf, 0, len);
    memset(source_cstr.get(), 0, 108);

    bytes = ::recvfrom(sockfd_, buf, len, recvfrom_flags, reinterpret_cast<struct sockaddr*>(&saddr), &socksize);

    memcpy(source_cstr.get(), saddr.sun_path, 107 < sizeof(saddr.sun_path) ? 107 : sizeof(saddr.sun_path));

    source_cstr_len = strlen(source_cstr.get());
    source.resize(source_cstr_len);
    source = source_cstr.get();

    return bytes;
}

ssize_t Socket::recvfrom(std::string& buf, std::string& source, int recvfrom_flags)
{
    if (buf.empty())
    {
        LOG_ERROR << "rcvfrom: Buffer is NULL!";
        return -1;
    }

    ssize_t bytes;
    using std::unique_ptr;
    unique_ptr<char[]> source_cstr(new char[108]); // AFAIK, the address field in struct sockaddr_un is
    // only 108 bytes...
    unique_ptr<char[]> cbuf(new char[buf.size()]);
    size_t source_cstr_len;
    struct sockaddr_un saddr = {};
    socklen_t socksize       = sizeof(struct sockaddr_un);

    memset(source_cstr.get(), 0, 108);
    memset(cbuf.get(), 0, buf.size());

    bytes = ::recvfrom(sockfd_, cbuf.get(), buf.size(), recvfrom_flags, reinterpret_cast<struct sockaddr*>(&saddr),
                       &socksize);

    memcpy(source_cstr.get(), saddr.sun_path, 107 < sizeof(saddr.sun_path) ? 107 : sizeof(saddr.sun_path));

    source_cstr_len = strlen(source_cstr.get());

    source.resize(source_cstr_len);
    buf.resize(static_cast<unsigned long long>(bytes));

    buf.assign(cbuf.get(), static_cast<unsigned long long>(bytes));
    source.assign(source_cstr.get(), source_cstr_len);

    return bytes;
}

///
/// UDP/IP sockets I/O
///
ssize_t Socket::sendto(const void* buf, size_t len, const InetAddress& dst,
                       int sndto_flags) // flags: sendto()
{
    if (-1 == sockfd_)
    {
        LOG_ERROR << "Socket already closed!";
    }

    ssize_t bytes;
    bytes = ::sendto(sockfd_, buf, len, sndto_flags, dst.getSockAddr(), dst.getSockLen());

    return bytes;
}

ssize_t Socket::sendto(const std::string& buf, const InetAddress& dst, int sndto_flags)
{
    return sendto(buf.c_str(), buf.size(), dst, sndto_flags);
}

ssize_t Socket::recvfrom(void* buf, size_t len, InetAddress& src, int rcvfrom_flags)
{
    if (-1 == sockfd_)
    {
        LOG_ERROR << "Socket already closed!";
    }

    ssize_t bytes;
    struct sockaddr peerAddr = {};
    memset(&peerAddr, 0, sizeof(peerAddr));
    socklen_t addrLen = sizeof(peerAddr);
    bytes             = ::recvfrom(sockfd_, buf, len, rcvfrom_flags, &peerAddr, &addrLen);
    src.setSockAddr(reinterpret_cast<struct sockaddr_un&>(peerAddr));

    return bytes;
}

ssize_t Socket::recvfrom(std::string& buf, InetAddress& src, int rcvfrom_flags)
{
    ssize_t bytes;
    using std::unique_ptr;
    char cbuf[MSG_BUF_SIZE];
    memset(cbuf, 0, MSG_BUF_SIZE);
    // unique_ptr<char[]> cbuf(new char[buf.size()]);
    // memset(cbuf.get(), 0, buf.size());
    // bytes = recvfrom(cbuf.get(), static_cast<size_t>(buf.size()), src,
    // rcvfrom_flags);
    bytes = recvfrom(cbuf, MSG_BUF_SIZE, src, rcvfrom_flags);

    buf.resize(static_cast<unsigned long long>(bytes));
    buf.assign(cbuf, static_cast<unsigned long long>(bytes));

    return bytes;
}

void Socket::shutdownWrite()
{
    if (::shutdown(sockfd_, SHUT_WR) < 0)
    {
        LOG_ERROR << "sockets::shutdownWrite";
    }
}

void Socket::setTcpNoDelay(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &optval, static_cast<socklen_t>(sizeof optval));
    // FIXME CHECK
}

void Socket::setReuseAddr(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &optval, static_cast<socklen_t>(sizeof optval));
    // FIXME CHECK
}

void Socket::setReusePort(bool on)
{
#ifdef SO_REUSEPORT
    int optval = on ? 1 : 0;
    int ret    = ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &optval, static_cast<socklen_t>(sizeof optval));
    if (ret < 0 && on)
    {
        LOG_ERROR << "SO_REUSEPORT failed.";
    }
#else
    if (on)
    {
        LOG_ERROR << "SO_REUSEPORT is not supported.";
    }
#endif
}

void Socket::setKeepAlive(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &optval, static_cast<socklen_t>(sizeof optval));
    // FIXME CHECK
}

void Socket::setMulticastIF(const std::string& address)
{
    struct in_addr addr = {};
    addr.s_addr         = inet_addr(address.c_str());
    ::setsockopt(sockfd_, IPPROTO_IP, IP_MULTICAST_IF, &addr, static_cast<socklen_t>(sizeof addr));
}

void Socket::setLoopBack(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, IPPROTO_IP, IP_MULTICAST_LOOP, &optval, static_cast<socklen_t>(sizeof optval));
    // FIXME CHECK
}

void Socket::setTTL(const unsigned char& ttl)
{
    ::setsockopt(sockfd_, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, static_cast<socklen_t>(sizeof ttl));
}

void Socket::setAddMemberShip(struct ip_mreq& mreq)
{
    ::setsockopt(sockfd_, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, static_cast<socklen_t>(sizeof mreq));
}