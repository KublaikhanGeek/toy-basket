/******************************************************************************
 * File name     : DgramClient.cpp
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
#include "Types.h"

#include "DgramClient.h"

using namespace toyBasket;

DgramClient::DgramClient(EventLoop* loop, const InetAddress& serverAddr, const std::string& nameArg,
                         const InetAddress& clientAddr)
    : loop_(CHECK_NOTNULL(loop))
    , name_(nameArg)
    , serverAddr_(serverAddr)
    , connected_(false)
    , socket_(::socket(serverAddr.family(), SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0))
    , channel_(loop, socket_.fd())
{
    channel_.setReadCallback(std::bind(&DgramClient::handleRead, this));
    channel_.enableReading();
    if (serverAddr.family() == AF_UNIX)
    {
        socket_.bindAddress(clientAddr);
    }
    connect(serverAddr_);
}

DgramClient::~DgramClient()
{
    this->stop();
}

void DgramClient::stop()
{
    channel_.disableAll();
    channel_.remove();
}

void DgramClient::connect(const InetAddress& serverAddr)
{
    int ret = ::connect(socket_.fd(), serverAddr_.getSockAddr(), serverAddr_.getSockLen());
    if (ret < 0)
    {
        LOG_ERROR << "UDP client connect error: " << strerror(errno);
    }
    connected_ = true;
}

void DgramClient::send(const void* message, int len)
{
    if (connected_)
    {
        ssize_t ret = ::write(socket_.fd(), message, static_cast<size_t>(len));
        if (ret < 0)
        {
            LOG_ERROR << "UDP client send content error: " << strerror(errno);
        }
    }
    else
    {
        LOG_ERROR << "UDP client did not establish a connection";
    }
}

void DgramClient::handleRead()
{
    int savedErrno = 0;
    ssize_t n      = inputBuffer_.readFd(channel_.fd(), &savedErrno);
    if (n > 0)
    {
        messageCallback_(serverAddr_, inputBuffer_.toStringPiece().data(), inputBuffer_.toStringPiece().size());
        inputBuffer_.retrieveAll();
    }
    else if (n == 0)
    {
        LOG_WARNING << "udp server close the connect \n";
    }
    else
    {
        errno = savedErrno;
        LOG_ERROR << "recv errno: " << strerror(errno);
    }
}
