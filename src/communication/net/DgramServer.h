/******************************************************************************
 * File name     : DgramServer.h
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

#ifndef _DGRAMSERVER_H
#define _DGRAMSERVER_H

#include "noncopyable.h"
#include "InetAddress.h"
#include "EventLoop.h"
#include "Socket.h"
#include "Channel.h"
#include "Callbacks.h"

#include <atomic>

namespace toyBasket
{

/// This is an interface class, so don't expose too much details.
class DgramServer : noncopyable
{
public:
    // DgramServer(EventLoop* loop, const InetAddress& listenAddr);
    DgramServer(EventLoop* loop, const InetAddress& listenAddr, const std::string& nameArg);
    ~DgramServer(); // force out-line dtor, for std::unique_ptr members.

    const std::string& ipPort() const
    {
        return ipPort_;
    }
    const std::string& name() const
    {
        return name_;
    }
    EventLoop* getLoop() const
    {
        return loop_;
    }

    /// Starts the server if it's not listenning.
    ///
    /// It's harmless to call it multiple times.
    /// Thread safe.
    void start();

    void stop();

    /// Set message callback.
    /// Not thread safe.
    void setMessageCallback(const DgramEventCallback& cb)
    {
        messageCallback_ = cb;
    }

    void send(const InetAddress& clientAddr, const void* message, int len);

private:
    void handleRead();

private:
    EventLoop* loop_; // the acceptor loop
    Socket socket_;
    Channel channel_;
    InetAddress serverAddr_;
    const std::string ipPort_;
    const std::string name_;
    std::atomic<int> started_;
    DgramEventCallback messageCallback_;
};

} // namespace toyBasket

#endif // _DGRAMSERVER_H
