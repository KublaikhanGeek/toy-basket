/******************************************************************************
 * File name     : StreamClient.h
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
#ifndef _STREAMCLIENT_H
#define _STREAMCLIENT_H

#include "StreamConnection.h"
#include <mutex>

namespace toyBasket
{

class Connector;
typedef std::shared_ptr<Connector> ConnectorPtr;

class StreamClient : noncopyable
{
public:
    // StreamClient(EventLoop* loop);
    // StreamClient(EventLoop* loop, const string& host, unsigned short port);
    StreamClient(EventLoop* loop, const InetAddress& serverAddr, const std::string& nameArg);
    ~StreamClient(); // force out-line dtor, for std::unique_ptr members.

    void connect();
    void disconnect();
    void stop();

    StreamConnectionPtr connection() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return connection_;
    }

    EventLoop* getLoop() const
    {
        return loop_;
    }
    bool retry() const
    {
        return retry_;
    }
    void enableRetry()
    {
        retry_ = true;
    }

    const std::string& name() const
    {
        return name_;
    }

    /// Set connection callback.
    /// Not thread safe.
    void setConnectionCallback(ConnectionCallback cb)
    {
        connectionCallback_ = std::move(cb);
    }

    /// Set message callback.
    /// Not thread safe.
    void setMessageCallback(MessageCallback cb)
    {
        messageCallback_ = std::move(cb);
    }

    /// Set write complete callback.
    /// Not thread safe.
    void setWriteCompleteCallback(WriteCompleteCallback cb)
    {
        writeCompleteCallback_ = std::move(cb);
    }

private:
    /// Not thread safe, but in loop
    void newConnection(int sockfd);
    /// Not thread safe, but in loop
    void removeConnection(const StreamConnectionPtr& conn);

    EventLoop* loop_;
    ConnectorPtr connector_; // avoid revealing Connector
    const std::string name_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    bool retry_;   // atomic
    bool connect_; // atomic
    // always in loop thread
    int nextConnId_;
    mutable std::mutex mutex_;
    StreamConnectionPtr connection_;
};

} // namespace toyBasket

#endif // _STREAMCLIENT_H
