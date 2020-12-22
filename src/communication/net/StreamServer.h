/******************************************************************************
 * File name     : StreamServer.h
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

#ifndef _STREAMSERVER_H
#define _STREAMSERVER_H

#include "StreamConnection.h"

#include <map>
#include <atomic>

namespace toyBasket
{

class Acceptor;
class EventLoop;

///
/// STREAM server, supports single-threaded and thread-pool models.
///
/// This is an interface class, so don't expose too much details.
class StreamServer : noncopyable
{
public:
    typedef std::function<void(EventLoop*)> ThreadInitCallback;
    enum Option
    {
        kNoReusePort,
        kReusePort,
    };

    // StreamServer(EventLoop* loop, const InetAddress& listenAddr);
    StreamServer(EventLoop* loop, const InetAddress& listenAddr, const std::string& nameArg,
                 Option option = kNoReusePort);
    ~StreamServer(); // force out-line dtor, for std::unique_ptr members.

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

    /// Set connection callback.
    /// Not thread safe.
    void setConnectionCallback(const ConnectionCallback& cb)
    {
        connectionCallback_ = cb;
    }

    /// Set message callback.
    /// Not thread safe.
    void setMessageCallback(const MessageCallback& cb)
    {
        messageCallback_ = cb;
    }

    /// Set write complete callback.
    /// Not thread safe.
    void setWriteCompleteCallback(const WriteCompleteCallback& cb)
    {
        writeCompleteCallback_ = cb;
    }

private:
    /// Not thread safe, but in loop
    void newConnection(int sockfd, const InetAddress& peerAddr);
    /// Thread safe.
    void removeConnection(const StreamConnectionPtr& conn);
    /// Not thread safe, but in loop
    void removeConnectionInLoop(const StreamConnectionPtr& conn);

    typedef std::map<std::string, StreamConnectionPtr> ConnectionMap;

    EventLoop* loop_; // the acceptor loop
    const std::string ipPort_;
    const std::string name_;
    std::unique_ptr<Acceptor> acceptor_; // avoid revealing Acceptor
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    std::atomic<int> started_;
    // always in loop thread
    int nextConnId_;
    ConnectionMap connections_;
};

} // namespace toyBasket

#endif // _STREAMSERVER_H
