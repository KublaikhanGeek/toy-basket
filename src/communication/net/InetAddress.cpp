/******************************************************************************
 * File name     : InetAddress.cpp
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

#include "InetAddress.h"
#include "Types.h"

#include <arpa/inet.h>
#include <cassert>
#include <cstddef>
#include <cstring>
#include <endian.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/un.h>

// INADDR_ANY use (type)value casting.
#pragma GCC diagnostic ignored "-Wold-style-cast"
static const in_addr_t kInaddrAny      = INADDR_ANY;
static const in_addr_t kInaddrLoopback = INADDR_LOOPBACK;
#pragma GCC diagnostic error "-Wold-style-cast"

//      /* Structure describing a generic socket address.  */
//      struct sockaddr
//        {
//          __SOCKADDR_COMMON (sa_);    /* Common data: address family and
//          length.  */ char sa_data[14];           /* Address data.  */
//        };

//     /* Structure describing an Internet socket address.  */
//     struct sockaddr_in {
//         sa_family_t    sin_family; /* address family: AF_INET */
//         unsigned short       sin_port;   /* port in network byte order */
//         struct in_addr sin_addr;   /* internet address */
//     };

//     /* Internet address. */
//     typedef unsigned int in_addr_t;
//     struct in_addr {
//         in_addr_t       s_addr;     /* address in network byte order */
//     };

//     struct sockaddr_in6 {
//         sa_family_t     sin6_family;   /* address family: AF_INET6 */
//         unsigned short        sin6_port;     /* port in network byte order */
//         unsigned int        sin6_flowinfo; /* IPv6 flow information */
//         struct in6_addr sin6_addr;     /* IPv6 address */
//         unsigned int        sin6_scope_id; /* IPv6 scope-id */
//     };

//     /* IPv6 address */
//     struct in6_addr
//     {
//         union
//         {
//             unsigned char u6_addr8[16];
//             unsigned short u6_addr16[8];
//             unsigned int u6_addr32[4];
//         } in6_u;
//     #define s6_addr                 in6_u.u6_addr8
//     #define s6_addr16               in6_u.u6_addr16
//     #define s6_addr32               in6_u.u6_addr32
//     };

//     /* Structure describing the address of an AF_LOCAL (aka AF_UNIX) socket.
//     */ struct sockaddr_un
//       {
//         __SOCKADDR_COMMON (sun_);
//         char sun_path[108];         /* Path name.  */
//       };

using namespace toyBasket;

static_assert(sizeof(InetAddress) == (sizeof(struct sockaddr_un) + 2), "InetAddress is same size as sockaddr_un + 2");
static_assert(offsetof(sockaddr_in, sin_family) == 0, "sin_family offset 0");
static_assert(offsetof(sockaddr_in, sin_port) == 2, "sin_port offset 2");
static_assert(offsetof(sockaddr_in6, sin6_family) == 0, "sin6_family offset 0");
static_assert(offsetof(sockaddr_in6, sin6_port) == 2, "sin6_port offset 2");
static_assert(offsetof(sockaddr_un, sun_family) == 0, "sun_family offset 0");
static_assert(offsetof(sockaddr_un, sun_path) == 2, "sun_path offset 2");

InetAddress::InetAddress(unsigned short port, bool loopbackOnly,
                         bool ipv6) // NOLINT
{
    static_assert(offsetof(InetAddress, addr_in6_) == 0, "addr_in6_ offset 0");
    static_assert(offsetof(InetAddress, addr_in_) == 0, "addr_in_ offset 0");
    unsigned short port16 = htobe16(port);
    if (ipv6)
    {
        memset(&addr_in6_, 0, sizeof(addr_in6_));
        addr_in6_.sin6_family = AF_INET6;
        in6_addr ip           = loopbackOnly ? in6addr_loopback : in6addr_any;
        addr_in6_.sin6_addr   = ip;
        addr_in6_.sin6_port   = port16;
    }
    else
    {
        memset(&addr_in_, 0, sizeof(addr_in_));
        addr_in_.sin_family      = AF_INET;
        in_addr_t ip             = loopbackOnly ? kInaddrLoopback : kInaddrAny;
        addr_in_.sin_addr.s_addr = htobe32(ip);
        addr_in_.sin_port        = port16;
    }
}

InetAddress::InetAddress(StringArg ip, unsigned short port, bool ipv6) // NOLINT
{
    if (ipv6)
    {
        memset(&addr_in6_, 0, sizeof(addr_in6_));
        addr_in6_.sin6_family = AF_INET6;
        addr_in6_.sin6_port   = htobe16(port);
        if (::inet_pton(AF_INET6, ip.c_str(), &(addr_in6_.sin6_addr)) <= 0)
        {
            LOG_ERROR << "inet_pton error: " << strerror(errno);
        }
    }
    else
    {
        memset(&addr_in_, 0, sizeof(addr_in_));
        addr_in_.sin_family = AF_INET;
        addr_in_.sin_port   = htobe16(port);
        if (::inet_pton(AF_INET, ip.c_str(), &(addr_in_.sin_addr)) <= 0)
        {
            LOG_ERROR << "inet_pton error: " << strerror(errno);
        }
    }
}

InetAddress::InetAddress(const char* un_domain) // NOLINT
{
    memset(&addr_un_, 0, sizeof(addr_un_));
    addr_un_.sun_family = AF_UNIX;
    strncpy(addr_un_.sun_path, un_domain, sizeof(addr_un_.sun_path) - 1);
}

InetAddress::InetAddress(const StringArg& un_domain) // NOLINT
{
    memset(&addr_un_, 0, sizeof(addr_un_));
    addr_un_.sun_family = AF_UNIX;
    strncpy(addr_un_.sun_path, un_domain.c_str(), sizeof(addr_un_.sun_path) - 1);
}

socklen_t InetAddress::getSockLen() const
{
    socklen_t len = 0;

    switch (family())
    {
    case AF_INET:
    {
        len = static_cast<socklen_t>(sizeof(addr_in_));
        break;
    }
    case AF_INET6:
    {
        len = static_cast<socklen_t>(sizeof(addr_in6_));
        break;
    }
    case AF_UNIX:
    {
        len = static_cast<socklen_t>(sizeof(addr_un_));
        break;
    }
    default:
        assert(0);
    };

    return len;
}

std::string InetAddress::toString() const
{
    char buf[128] = "";
    int size      = sizeof(buf);

    std::string ip = this->address();
    snprintf(buf, static_cast<size_t>(size), "%s", ip.c_str());

    const struct sockaddr* addr = getSockAddr();
    if (addr->sa_family != AF_UNIX)
    {
        const struct sockaddr_in* addr4 = reinterpret_cast<const struct sockaddr_in*>(addr);
        unsigned short port             = be16toh(addr4->sin_port);

        size_t end = ::strlen(buf);
        assert(static_cast<size_t>(size) > end);
        snprintf(buf + end, static_cast<size_t>(size) - end, ":%u", port);
    }

    return buf;
}

std::string InetAddress::address() const
{
    char buf[128]               = "";
    const struct sockaddr* addr = getSockAddr();
    int size                    = sizeof(buf);
    if (addr->sa_family == AF_INET)
    {
        assert(size >= INET_ADDRSTRLEN);
        const struct sockaddr_in* addr4 = reinterpret_cast<const struct sockaddr_in*>(addr);
        ::inet_ntop(AF_INET, &addr4->sin_addr, buf, static_cast<socklen_t>(size));
    }
    else if (addr->sa_family == AF_INET6)
    {
        assert(size >= INET6_ADDRSTRLEN);
        const struct sockaddr_in6* addr6 = reinterpret_cast<const struct sockaddr_in6*>(addr);
        ::inet_ntop(AF_INET6, &addr6->sin6_addr, buf, static_cast<socklen_t>(size));
    }
    else if (addr->sa_family == AF_UNIX)
    {
        assert(size >= 107);
        const struct sockaddr_un* addrun = reinterpret_cast<const struct sockaddr_un*>(addr);
        strncpy(buf, addrun->sun_path, 108);
    }
    return buf;
}

unsigned int InetAddress::ipNetEndian() const
{
    assert(family() == AF_INET);
    return addr_in_.sin_addr.s_addr;
}

unsigned short InetAddress::portNetEndian() const
{
    assert(family() != AF_UNIX);
    return addr_in_.sin_port;
}

unsigned short InetAddress::toPort() const
{
    return be16toh(this->portNetEndian());
}

static __thread char t_resolveBuffer[64 * 1024];

bool InetAddress::resolve(const std::string& hostname, InetAddress* out)
{
    assert(out != NULL);
    struct hostent hent = {};
    struct hostent* he  = NULL;
    int herrno          = 0;
    memset(&hent, 0, sizeof(hent));

    int ret = gethostbyname_r(hostname.c_str(), &hent, t_resolveBuffer, sizeof(t_resolveBuffer), &he, &herrno);
    if (ret == 0 && he != NULL)
    {
        if (he->h_addrtype == AF_INET)
        {
            assert(he->h_length == (sizeof(unsigned int)));
            out->addr_in_.sin_addr = *reinterpret_cast<struct in_addr*>(he->h_addr);

            return true;
        }
        else if (he->h_addrtype == AF_INET6)
        {
            assert(he->h_length == (4 * sizeof(unsigned int)));
            out->addr_in6_.sin6_addr = *reinterpret_cast<struct in6_addr*>(he->h_addr);

            return true;
        }
        else
        {
            assert(0);
        }
    }
    else
    {
        if (ret)
        {
            LOG_ERROR << "InetAddress::resolve";
        }
    }

    return false;
}

void InetAddress::setScopeId(unsigned int scope_id)
{
    if (family() == AF_INET6)
    {
        addr_in6_.sin6_scope_id = scope_id;
    }
}