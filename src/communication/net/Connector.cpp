/******************************************************************************
 * File name     : Connector.cpp
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

#include "Connector.h"

#include "Channel.h"
#include "EventLoop.h"
#include "Timer.h"
#include "Types.h"

#include <cassert>
#include <cerrno>

using namespace toyBasket;

const int Connector::kMaxRetryDelayMs;

Connector::Connector(EventLoop* loop, const InetAddress& serverAddr)
    : loop_(loop)
    , serverAddr_(serverAddr)
    , connect_(false)
    , state_(kDisconnected)
    , retryDelayMs_(kInitRetryDelayMs)
{
    LOG_INFO << "ctor[" << this << "]";
}

Connector::~Connector()
{
    LOG_INFO << "dtor[" << this << "]";
    assert(!channel_);
}

void Connector::start()
{
    connect_ = true;
    loop_->runInLoop(std::bind(&Connector::startInLoop, this)); // FIXME: unsafe
}

void Connector::startInLoop()
{
    loop_->assertInLoopThread();
    assert(state_ == kDisconnected);
    if (connect_)
    {
        connect();
    }
    else
    {
        LOG_INFO << "do not connect";
    }
}

void Connector::stop()
{
    connect_ = false;
    loop_->queueInLoop(std::bind(&Connector::stopInLoop, this)); // FIXME: unsafe
                                                                 // FIXME: cancel timer
}

void Connector::stopInLoop()
{
    loop_->assertInLoopThread();
    if (state_ == kConnecting)
    {
        setState(kDisconnected);
        int sockfd = removeAndResetChannel();
        retry(sockfd);
    }
}

void Connector::connect()
{
    int sockfd = ::socket(serverAddr_.family(), SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (sockfd < 0)
    {
        LOG_FATAL << "create socket failed!";
    }
    int ret        = ::connect(sockfd, serverAddr_.getSockAddr(), serverAddr_.getSockLen());
    int savedErrno = (ret == 0) ? 0 : errno;
    switch (savedErrno)
    {
    case 0:
    case EINPROGRESS:
    case EINTR:
    case EISCONN:
        connecting(sockfd);
        break;

    case EAGAIN:
    case EADDRINUSE:
    case EADDRNOTAVAIL:
    case ECONNREFUSED:
    case ENETUNREACH:
        retry(sockfd);
        break;

    case EACCES:
    case EPERM:
    case EAFNOSUPPORT:
    case EALREADY:
    case EBADF:
    case EFAULT:
    case ENOTSOCK:
        LOG_ERROR << "connect error in Connector::startInLoop " << savedErrno;
        if (::close(sockfd) < 0)
        {
            LOG_ERROR << "close socket error: " << strerror(errno);
        }
        break;

    default:
        LOG_ERROR << "Unexpected error in Connector::startInLoop " << savedErrno;
        if (::close(sockfd) < 0)
        {
            LOG_ERROR << "close socket error: " << strerror(errno);
        }
        // connectErrorCallback_();
        break;
    }
}

void Connector::restart()
{
    loop_->assertInLoopThread();
    setState(kDisconnected);
    retryDelayMs_ = kInitRetryDelayMs;
    connect_      = true;
    startInLoop();
}

void Connector::connecting(int sockfd)
{
    setState(kConnecting);
    assert(!channel_);
    channel_.reset(new Channel(loop_, sockfd));
    channel_->setWriteCallback(std::bind(&Connector::handleWrite, this)); // FIXME: unsafe
    channel_->setErrorCallback(std::bind(&Connector::handleError, this)); // FIXME: unsafe

    // channel_->tie(shared_from_this()); is not working,
    // as channel_ is not managed by shared_ptr
    channel_->enableWriting();
}

int Connector::removeAndResetChannel()
{
    channel_->disableAll();
    channel_->remove();
    int sockfd = channel_->fd();
    // Can't reset channel_ here, because we are inside Channel::handleEvent
    loop_->queueInLoop(std::bind(&Connector::resetChannel, this)); // FIXME: unsafe
    return sockfd;
}

void Connector::resetChannel()
{
    channel_.reset();
}

void Connector::handleWrite()
{
    LOG_INFO << "Connector::handleWrite " << state_;

    if (state_ == kConnecting)
    {
        int err;
        int sockfd = removeAndResetChannel();

        socklen_t errlen = static_cast<socklen_t>(sizeof(err));
        if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &err, &errlen) < 0)
        {
            err = errno;
        }

        if (err)
        {
            char t_errnobuf[512];
            LOG_WARNING << "Connector::handleWrite - SO_ERROR = " << err << " "
                        << strerror_r(err, t_errnobuf, sizeof(t_errnobuf));
            retry(sockfd);
        }
        else if (this->isSelfConnect(sockfd))
        {
            LOG_WARNING << "Connector::handleWrite - Self connect";
            retry(sockfd);
        }
        else
        {
            setState(kConnected);
            if (connect_)
            {
                newConnectionCallback_(sockfd);
            }
            else
            {
                if (::close(sockfd) < 0)
                {
                    LOG_ERROR << "close socket error!";
                }
            }
        }
    }
    else
    {
        // what happened?
        assert(state_ == kDisconnected);
    }
}

void Connector::handleError()
{
    LOG_INFO << "Connector::handleError state=" << state_;
    if (state_ == kConnecting)
    {
        int err;
        int sockfd = removeAndResetChannel();

        socklen_t errlen = static_cast<socklen_t>(sizeof(err));
        if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &err, &errlen) < 0)
        {
            err = errno;
        }

        char t_errnobuf[512];
        LOG_INFO << "SO_ERROR = " << err << " " << strerror_r(err, t_errnobuf, sizeof(t_errnobuf));
        retry(sockfd);
    }
}

void Connector::retry(int sockfd)
{
    if (::close(sockfd) < 0)
    {
        LOG_ERROR << "close socket error!";
    }

    setState(kDisconnected);
    if (connect_)
    {
        LOG_INFO << "Connector::retry - Retry connecting to " << serverAddr_.toString() << " in " << retryDelayMs_
                 << " milliseconds. ";
        TimerManager::getInstance()->addTimer(static_cast<unsigned int>(retryDelayMs_),
                                              std::bind(&Connector::startInLoop, shared_from_this()));
        retryDelayMs_ = std::min(retryDelayMs_ * 2, kMaxRetryDelayMs);
    }
    else
    {
        LOG_INFO << "do not connect";
    }
}

bool Connector::isSelfConnect(int sockfd)
{
    struct sockaddr_un localaddr = {};
    memset(&localaddr, 0, sizeof(localaddr));
    socklen_t addrlen = static_cast<socklen_t>(sizeof localaddr);
    if (::getsockname(sockfd, reinterpret_cast<struct sockaddr*>(&localaddr), &addrlen) < 0)
    {
        LOG_ERROR << "get LocalAddr error";
    }

    struct sockaddr_un peeraddr = {};
    memset(&peeraddr, 0, sizeof(peeraddr));
    if (::getpeername(sockfd, reinterpret_cast<struct sockaddr*>(&peeraddr), &addrlen) < 0)
    {
        LOG_ERROR << "get PeerAddr error";
    }

    if (localaddr.sun_family == AF_INET)
    {
        const struct sockaddr_in* laddr4 = reinterpret_cast<struct sockaddr_in*>(&localaddr);
        const struct sockaddr_in* raddr4 = reinterpret_cast<struct sockaddr_in*>(&peeraddr);
        return laddr4->sin_port == raddr4->sin_port && laddr4->sin_addr.s_addr == raddr4->sin_addr.s_addr;
    }
    else if (localaddr.sun_family == AF_INET6)
    {
        const struct sockaddr_in6* laddr6 = reinterpret_cast<struct sockaddr_in6*>(&localaddr);
        const struct sockaddr_in6* raddr6 = reinterpret_cast<struct sockaddr_in6*>(&peeraddr);
        return laddr6->sin6_port == raddr6->sin6_port
            && memcmp(&laddr6->sin6_addr, &raddr6->sin6_addr, sizeof(laddr6->sin6_addr)) == 0;
    }
    else
    {
        return false;
    }
}