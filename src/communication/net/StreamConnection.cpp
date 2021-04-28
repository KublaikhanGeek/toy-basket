/******************************************************************************
 * File name     : StreamConnection.cpp
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

#include "StreamConnection.h"

#include "Channel.h"
#include "EventLoop.h"
#include "Socket.h"
#include "Timer.h"
#include "WeakCallback.h"

#include <cerrno>

using namespace toyBasket;

void toyBasket::defaultConnectionCallback(const StreamConnectionPtr& conn)
{
    LOG_INFO << conn->localAddress().toString() << " -> " << conn->peerAddress().toString() << " is "
             << (conn->connected() ? "UP" : "DOWN");
    // do not call conn->forceClose(), because some users want to register message
    // callback only.
}

void toyBasket::defaultMessageCallback(const StreamConnectionPtr&, Buffer* buf)
{
    buf->retrieveAll();
}

StreamConnection::StreamConnection(EventLoop* loop, const std::string& nameArg, int sockfd,
                                   const InetAddress& localAddr, const InetAddress& peerAddr)
    : loop_(CHECK_NOTNULL(loop))
    , name_(nameArg)
    , state_(kConnecting)
    , reading_(true)
    , socket_(new Socket(sockfd))
    , channel_(new Channel(loop, sockfd))
    , localAddr_(localAddr)
    , peerAddr_(peerAddr)
    , highWaterMark_(64 * 1024 * 1024)
{
    channel_->setReadCallback(std::bind(&StreamConnection::handleRead, this));
    channel_->setWriteCallback(std::bind(&StreamConnection::handleWrite, this));
    channel_->setCloseCallback(std::bind(&StreamConnection::handleClose, this));
    channel_->setErrorCallback(std::bind(&StreamConnection::handleError, this));
    LOG_INFO << "StreamConnection::ctor[" << name_ << "] at " << this << " fd=" << sockfd;
    socket_->setKeepAlive(true);
}

StreamConnection::~StreamConnection()
{
    LOG_INFO << "StreamConnection::dtor[" << name_ << "] at " << this << " fd=" << channel_->fd()
             << " state=" << stateToString();
    assert(state_ == kDisconnected);
}

bool StreamConnection::getTcpInfo(struct tcp_info* tcpi) const
{
    if (localAddr_.family() == AF_UNIX)
    {
        return false;
    }
    return socket_->getTcpInfo(tcpi);
}

std::string StreamConnection::getTcpInfoString() const
{
    if (localAddr_.family() == AF_UNIX)
    {
        return "";
    }
    char buf[1024];
    buf[0] = '\0';
    socket_->getTcpInfoString(buf, sizeof buf);
    return buf;
}

void StreamConnection::send(const void* data, int len)
{
    send(StringPiece(static_cast<const char*>(data), len));
}

void StreamConnection::send(const StringPiece& message)
{
    if (state_ == kConnected)
    {
        if (loop_->isInLoopThread())
        {
            sendInLoop(message);
        }
        else
        {
            void (StreamConnection::*fp)(const StringPiece& message) = &StreamConnection::sendInLoop;
            loop_->runInLoop(std::bind(fp,
                                       this, // FIXME
                                       message.as_string()));
            // std::forward<string>(message)));
        }
    }
}

// FIXME efficiency!!!
void StreamConnection::send(Buffer* buf)
{
    if (state_ == kConnected)
    {
        if (loop_->isInLoopThread())
        {
            sendInLoop(buf->peek(), buf->readableBytes());
            buf->retrieveAll();
        }
        else
        {
            void (StreamConnection::*fp)(const StringPiece& message) = &StreamConnection::sendInLoop;
            loop_->runInLoop(std::bind(fp,
                                       this, // FIXME
                                       buf->retrieveAllAsString()));
            // std::forward<string>(message)));
        }
    }
}

void StreamConnection::sendInLoop(const StringPiece& message)
{
    sendInLoop(message.data(), static_cast<unsigned long long>(message.size()));
}

void StreamConnection::sendInLoop(const void* data, size_t len)
{
    loop_->assertInLoopThread();
    ssize_t nwrote   = 0;
    size_t remaining = len;
    bool faultError  = false;
    if (state_ == kDisconnected)
    {
        LOG_WARNING << "disconnected, give up writing";
        return;
    }
    // if no thing in output queue, try writing directly
    if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0)
    {
        nwrote = ::write(channel_->fd(), data, len);
        if (nwrote >= 0)
        {
            remaining = len - static_cast<size_t>(nwrote);
            if (remaining == 0 && writeCompleteCallback_)
            {
                loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
            }
        }
        else
        { // nwrote < 0
            nwrote = 0;
            if (errno != EWOULDBLOCK)
            {
                LOG_ERROR << "StreamConnection::sendInLoop";
                if (errno == EPIPE || errno == ECONNRESET)
                { // FIXME: any others?
                    faultError = true;
                }
            }
        }
    }

    assert(remaining <= len);
    if (!faultError && remaining > 0)
    {
        size_t oldLen = outputBuffer_.readableBytes();
        if (oldLen + remaining >= highWaterMark_ && oldLen < highWaterMark_ && highWaterMarkCallback_)
        {
            loop_->queueInLoop(std::bind(highWaterMarkCallback_, shared_from_this(), oldLen + remaining));
        }
        outputBuffer_.append(static_cast<const char*>(data) + nwrote, remaining);
        if (!channel_->isWriting())
        {
            channel_->enableWriting();
        }
    }
}

void StreamConnection::shutdown()
{
    // FIXME: use compare and swap
    if (state_ == kConnected)
    {
        setState(kDisconnecting);
        // FIXME: shared_from_this()?
        loop_->runInLoop(std::bind(&StreamConnection::shutdownInLoop, this));
    }
}

void StreamConnection::shutdownInLoop()
{
    loop_->assertInLoopThread();
    if (!channel_->isWriting())
    {
        // we are not writing
        socket_->shutdownWrite();
    }
}

void StreamConnection::forceClose()
{
    // FIXME: use compare and swap
    if (state_ == kConnected || state_ == kDisconnecting)
    {
        setState(kDisconnecting);
        loop_->queueInLoop(std::bind(&StreamConnection::forceCloseInLoop, shared_from_this()));
    }
}

void StreamConnection::forceCloseWithDelay(double seconds)
{
    if (state_ == kConnected || state_ == kDisconnecting)
    {
        setState(kDisconnecting);
        TimerManager::getInstance()->addTimer(
            static_cast<unsigned int>(seconds * 1000),
            makeWeakCallback(shared_from_this(),
                             &StreamConnection::forceClose)); // not forceCloseInLoop to avoid
                                                              // race condition
    }
}

void StreamConnection::forceCloseInLoop()
{
    loop_->assertInLoopThread();
    if (state_ == kConnected || state_ == kDisconnecting)
    {
        // as if we received 0 byte in handleRead();
        handleClose();
    }
}

const char* StreamConnection::stateToString() const
{
    switch (state_)
    {
    case kDisconnected:
        return "kDisconnected";
    case kConnecting:
        return "kConnecting";
    case kConnected:
        return "kConnected";
    case kDisconnecting:
        return "kDisconnecting";
    default:
        return "unknown state";
    }
}

void StreamConnection::setTcpNoDelay(bool on)
{
    if (localAddr_.family() != AF_UNIX)
    {
        socket_->setTcpNoDelay(on);
    }
}

void StreamConnection::startRead()
{
    loop_->runInLoop(std::bind(&StreamConnection::startReadInLoop, this));
}

void StreamConnection::startReadInLoop()
{
    loop_->assertInLoopThread();
    if (!reading_ || !channel_->isReading())
    {
        channel_->enableReading();
        reading_ = true;
    }
}

void StreamConnection::stopRead()
{
    loop_->runInLoop(std::bind(&StreamConnection::stopReadInLoop, this));
}

void StreamConnection::stopReadInLoop()
{
    loop_->assertInLoopThread();
    if (reading_ || channel_->isReading())
    {
        channel_->disableReading();
        reading_ = false;
    }
}

void StreamConnection::connectEstablished()
{
    loop_->assertInLoopThread();
    assert(state_ == kConnecting);
    setState(kConnected);
    channel_->tie(shared_from_this());
    channel_->enableReading();

    connectionCallback_(shared_from_this());
}

void StreamConnection::connectDestroyed()
{
    loop_->assertInLoopThread();
    if (state_ == kConnected)
    {
        setState(kDisconnected);
        channel_->disableAll();

        connectionCallback_(shared_from_this());
    }
    channel_->remove();
}

void StreamConnection::handleRead()
{
    loop_->assertInLoopThread();
    int savedErrno = 0;
    ssize_t n      = inputBuffer_.readFd(channel_->fd(), &savedErrno);
    if (n > 0)
    {
        messageCallback_(shared_from_this(), &inputBuffer_);
    }
    else if (n == 0)
    {
        handleClose();
    }
    else
    {
        errno = savedErrno;
        LOG_ERROR << "StreamConnection::handleRead: " << strerror(errno);
        handleError();
    }
}

void StreamConnection::handleWrite()
{
    loop_->assertInLoopThread();
    if (channel_->isWriting())
    {
        ssize_t n = ::write(channel_->fd(), outputBuffer_.peek(), outputBuffer_.readableBytes());
        if (n > 0)
        {
            outputBuffer_.retrieve(static_cast<unsigned long long>(n));
            if (outputBuffer_.readableBytes() == 0)
            {
                channel_->disableWriting();
                if (writeCompleteCallback_)
                {
                    loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
                }
                if (state_ == kDisconnecting)
                {
                    shutdownInLoop();
                }
            }
        }
        else
        {
            LOG_ERROR << "StreamConnection::handleWrite: " << strerror(errno);
            // if (state_ == kDisconnecting)
            // {
            //   shutdownInLoop();
            // }
        }
    }
    else
    {
        LOG_INFO << "Connection fd = " << channel_->fd() << " is down, no more writing";
    }
}

void StreamConnection::handleClose()
{
    loop_->assertInLoopThread();
    LOG_INFO << "fd = " << channel_->fd() << " state = " << stateToString();
    assert(state_ == kConnected || state_ == kDisconnecting);
    // we don't close fd, leave it to dtor, so we can find leaks easily.
    setState(kDisconnected);
    channel_->disableAll();

    StreamConnectionPtr guardThis(shared_from_this());
    connectionCallback_(guardThis);
    // must be the last line
    closeCallback_(guardThis);
}

void StreamConnection::handleError()
{
    int err          = 0;
    socklen_t optlen = static_cast<socklen_t>(sizeof(err));
    if (::getsockopt(channel_->fd(), SOL_SOCKET, SO_ERROR, &err, &optlen) < 0)
    {
        err = errno;
    }

    char t_errnobuf[512];
    LOG_ERROR << "StreamConnection::handleError [" << name_ << "] - SO_ERROR = " << err << " "
              << strerror_r(err, t_errnobuf, sizeof(t_errnobuf));
}
