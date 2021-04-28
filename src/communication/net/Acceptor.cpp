/******************************************************************************
 * File name     : Acceptor.cpp
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

#include "Acceptor.h"

#include "EventLoop.h"
#include "InetAddress.h"
#include "Types.h"

#include <cassert>
#include <cerrno>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

using namespace toyBasket;

Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport)
    : loop_(loop)
    , acceptSocket_(::socket(listenAddr.family(), SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0))
    , acceptChannel_(loop, acceptSocket_.fd())
    , listenning_(false)
    , idleFd_(::open("/dev/null", O_RDONLY | O_CLOEXEC))
{
    assert(idleFd_ >= 0);
    if (listenAddr.family() != AF_UNIX)
    {
        acceptSocket_.setReuseAddr(true);
        acceptSocket_.setReusePort(reuseport);
    }
    acceptSocket_.bindAddress(listenAddr);
    acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor()
{
    acceptChannel_.disableAll();
    acceptChannel_.remove();
    ::close(idleFd_);
}

void Acceptor::listen()
{
    loop_->assertInLoopThread();
    listenning_ = true;
    acceptSocket_.listen();
    acceptChannel_.enableReading();
}

void Acceptor::handleRead()
{
    loop_->assertInLoopThread();
    InetAddress peerAddr;
    // FIXME loop until no more
    int connfd = acceptSocket_.accept(peerAddr);
    if (connfd >= 0)
    {
        // string hostport = peerAddr.toString();
        // LOG_INFO << "Accepts of " << hostport;
        if (newConnectionCallback_)
        {
            newConnectionCallback_(connfd, peerAddr);
        }
        else
        {
            if (::close(connfd) < 0)
            {
                LOG_ERROR << "::close error: " << strerror(errno);
            }
        }
    }
    else
    {
        LOG_ERROR << "in Acceptor::handleRead";
        // Read the section named "The special problem of
        // accept()ing when you can't" in libev's doc.
        // By Marc Lehmann, author of libev.

        // 这样做，只是为了优雅的断开客户端。但服务端的fd耗尽时，客户端的链接还是在的。
        //
        // 准备一个空闲的文件描述符。遇到这种情况，先关闭这个空闲文件，获得一个文件描述符的名额；
        // 再accept(2)拿到新socket连接的描述符；随后立刻close(2)它，这样就优雅地断开了客户端连接；
        // 最后重新打开一个空闲文件，把“坑”占住，以备再次出现这种情况时使用。
        if (errno == EMFILE)
        {
            ::close(idleFd_);
            idleFd_ = ::accept(acceptSocket_.fd(), NULL, NULL);
            ::close(idleFd_);
            idleFd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
        }
    }
}
