/******************************************************************************
 * File name     : StreamServer.cpp
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

#include "StreamServer.h"

#include "Acceptor.h"
#include "EventLoop.h"

#include <cstdio> // snprintf

using namespace toyBasket;

StreamServer::StreamServer(EventLoop* loop, const InetAddress& listenAddr, const std::string& nameArg, Option option)
    : loop_(CHECK_NOTNULL(loop))
    , ipPort_(listenAddr.toString())
    , name_(nameArg)
    , acceptor_(new Acceptor(loop, listenAddr, option == kReusePort))
    , connectionCallback_(defaultConnectionCallback)
    , messageCallback_(defaultMessageCallback)
    , started_(0)
    , nextConnId_(1)
{
    acceptor_->setNewConnectionCallback(std::bind(&StreamServer::newConnection, this, _1, _2));
}

StreamServer::~StreamServer()
{
    loop_->assertInLoopThread();
    LOG_INFO << "StreamServer::~StreamServer [" << name_ << "] destructing";

    for (auto& item : connections_)
    {
        StreamConnectionPtr conn(item.second);
        item.second.reset();
        conn->getLoop()->runInLoop(std::bind(&StreamConnection::connectDestroyed, conn));
    }
}

void StreamServer::start()
{
    if (started_.exchange(1) == 0)
    {
        assert(!acceptor_->listenning());
        loop_->runInLoop(std::bind(&Acceptor::listen, get_pointer(acceptor_)));
    }
}

void StreamServer::newConnection(int sockfd, const InetAddress& peerAddr)
{
    loop_->assertInLoopThread();
    char buf[64];
    snprintf(buf, sizeof buf, "-%s#%d", ipPort_.c_str(), nextConnId_);
    ++nextConnId_;
    std::string connName = name_ + buf;

    LOG_INFO << "StreamServer::newConnection [" << name_ << "] - new connection [" << connName << "] from "
             << peerAddr.toString();

    struct sockaddr_un localaddr = {};
    memset(&localaddr, 0, sizeof(localaddr));
    socklen_t addrlen = static_cast<socklen_t>(sizeof(localaddr));
    if (::getsockname(sockfd, reinterpret_cast<sockaddr*>(&localaddr), &addrlen) < 0)
    {
        LOG_ERROR << "get LocalAddr error: " << strerror(errno);
    }

    InetAddress localAddr(localaddr);
    // FIXME poll with zero timeout to double confirm the new connection
    // FIXME use make_shared if necessary
    StreamConnectionPtr conn(new StreamConnection(loop_, connName, sockfd, localAddr, peerAddr));
    connections_[connName] = conn;
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallback(std::bind(&StreamServer::removeConnection, this, _1)); // FIXME: unsafe
    loop_->runInLoop(std::bind(&StreamConnection::connectEstablished, conn));
}

void StreamServer::removeConnection(const StreamConnectionPtr& conn)
{
    // FIXME: unsafe
    loop_->runInLoop(std::bind(&StreamServer::removeConnectionInLoop, this, conn));
}

void StreamServer::removeConnectionInLoop(const StreamConnectionPtr& conn)
{
    loop_->assertInLoopThread();
    LOG_INFO << "StreamServer::removeConnectionInLoop [" << name_ << "] - connection " << conn->name();
    size_t n = connections_.erase(conn->name());
    (void)n;
    assert(n == 1);
    EventLoop* ioLoop = conn->getLoop();
    ioLoop->queueInLoop(std::bind(&StreamConnection::connectDestroyed, conn));
}
