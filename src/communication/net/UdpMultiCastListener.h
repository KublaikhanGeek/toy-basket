/******************************************************************************
 * File name     : UdpMultiCastListener.h
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
#ifndef _GUDPMULTICASTLISTENER_H
#define _GUDPMULTICASTLISTENER_H

#include "Buffer.h"
#include "Callbacks.h"
#include "Channel.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "Socket.h"

namespace toyBasket
{

class UdpMultiCastListener : noncopyable
{
public:
    // UdpMultiCastListener(EventLoop* loop);
    // UdpMultiCastListener(EventLoop* loop, const string& host, unsigned short
    // port);
    UdpMultiCastListener(EventLoop* loop, const InetAddress& groupAddr, const InetAddress& localAddr,
                         const std::string& nameArg);

    ~UdpMultiCastListener(); // force out-line dtor, for std::unique_ptr members.

    EventLoop* getLoop() const
    {
        return loop_;
    }

    const std::string& name() const
    {
        return name_;
    }

    /// Set message callback.
    /// Not thread safe.
    void setMessageCallback(DgramEventCallback cb)
    {
        messageCallback_ = std::move(cb);
    }

private:
    void handleRead();
    void stop();

private:
    EventLoop* loop_;
    const std::string name_;
    const InetAddress groupAddr_;
    const InetAddress localAddr_;
    Socket socket_;
    Channel channel_;
    DgramEventCallback messageCallback_;
    Buffer inputBuffer_;
};

} // namespace toyBasket

#endif // _GUDPMULTICASTLISTENER_H
