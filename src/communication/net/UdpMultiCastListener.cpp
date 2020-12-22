/******************************************************************************
 * File name     : UdpMultiCastListener.cpp
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

#include "Types.h"

#include "UdpMultiCastListener.h"

using namespace toyBasket;

UdpMultiCastListener::UdpMultiCastListener(EventLoop* loop, const InetAddress& groupAddr, const InetAddress& localAddr,
                                           const std::string& nameArg)
    : loop_(CHECK_NOTNULL(loop))
    , name_(nameArg)
    , groupAddr_(groupAddr)
    , localAddr_(localAddr)
    , socket_(::socket(groupAddr.family(), SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0))
    , channel_(loop, socket_.fd())
{
    channel_.setReadCallback(std::bind(&UdpMultiCastListener::handleRead, this));
    channel_.enableReading();
    socket_.setReuseAddr(true);
    socket_.bindAddress(InetAddress(groupAddr_.toPort()));

    struct ip_mreq mreq       = {};
    mreq.imr_multiaddr.s_addr = inet_addr(groupAddr_.address().c_str());
    mreq.imr_interface.s_addr = inet_addr(localAddr_.address().c_str());
    socket_.setAddMemberShip(mreq);
}

UdpMultiCastListener::~UdpMultiCastListener()
{
    this->stop();
}

void UdpMultiCastListener::stop()
{
    channel_.disableAll();
    channel_.remove();
}

void UdpMultiCastListener::handleRead()
{
    std::string buf;
    InetAddress peerAddr;
    ssize_t n = socket_.recvfrom(buf, peerAddr);
    if (n > 0)
    {
        messageCallback_(peerAddr, buf.data(), static_cast<int>(buf.size()));
    }
    else if (n == 0)
    {
        LOG_WARNING << name_ << "udp server close the connect \n";
    }
    else
    {
        LOG_ERROR << name_ << "recv errno: " << strerror(errno);
    }
}
