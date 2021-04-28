/******************************************************************************
 * File name     : StreamConnection.h
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

#ifndef _STREAMCONNECTION_H
#define _STREAMCONNECTION_H

#include "Buffer.h"
#include "Callbacks.h"
#include "InetAddress.h"
#include "Types.h"
#include "noncopyable.h"

#include <memory>

// struct tcp_info is in <netinet/tcp.h>
struct tcp_info;

namespace toyBasket
{

class Channel;
class EventLoop;
class Socket;

///
/// STREAM connection, for both client and server usage.
///
/// This is an interface class, so don't expose too much details.
class StreamConnection : noncopyable, public std::enable_shared_from_this<StreamConnection>
{
public:
    /// Constructs a StreamConnection with a connected sockfd
    ///
    /// User should not create this object.
    StreamConnection(EventLoop* loop, const std::string& name, int sockfd, const InetAddress& localAddr,
                     const InetAddress& peerAddr);
    ~StreamConnection();

    EventLoop* getLoop() const
    {
        return loop_;
    }
    const std::string& name() const
    {
        return name_;
    }
    const InetAddress& localAddress() const
    {
        return localAddr_;
    }
    const InetAddress& peerAddress() const
    {
        return peerAddr_;
    }
    bool connected() const
    {
        return state_ == kConnected;
    }
    bool disconnected() const
    {
        return state_ == kDisconnected;
    }
    // return true if success.
    bool getTcpInfo(struct tcp_info*) const;
    std::string getTcpInfoString() const;

    // void send(string&& message); // C++11
    void send(const void* data, int len);
    void send(const StringPiece& message);
    // void send(Buffer&& message); // C++11
    void send(Buffer* buf); // this one will swap data
    void shutdown();        // NOT thread safe, no simultaneous calling
    // void shutdownAndForceCloseAfter(double seconds); // NOT thread safe, no
    // simultaneous calling
    void forceClose();
    void forceCloseWithDelay(double seconds);
    void setTcpNoDelay(bool on);
    // reading or not
    void startRead();
    void stopRead();
    bool isReading() const
    {
        return reading_;
    }; // NOT thread safe, may race with start/stopReadInLoop

    void setConnectionCallback(const ConnectionCallback& cb)
    {
        connectionCallback_ = cb;
    }

    void setMessageCallback(const MessageCallback& cb)
    {
        messageCallback_ = cb;
    }

    void setWriteCompleteCallback(const WriteCompleteCallback& cb)
    {
        writeCompleteCallback_ = cb;
    }

    void setHighWaterMarkCallback(const HighWaterMarkCallback& cb, size_t highWaterMark)
    {
        highWaterMarkCallback_ = cb;
        highWaterMark_         = highWaterMark;
    }

    /// Advanced interface
    Buffer* inputBuffer()
    {
        return &inputBuffer_;
    }

    Buffer* outputBuffer()
    {
        return &outputBuffer_;
    }

    /// Internal use only.
    void setCloseCallback(const CloseCallback& cb)
    {
        closeCallback_ = cb;
    }

    // called when StreamServer accepts a new connection
    void connectEstablished(); // should be called only once
    // called when StreamServer has removed me from its map
    void connectDestroyed(); // should be called only once

private:
    enum StateE
    {
        kDisconnected,
        kConnecting,
        kConnected,
        kDisconnecting
    };
    void handleRead();
    void handleWrite();
    void handleClose();
    void handleError();
    // void sendInLoop(string&& message);
    void sendInLoop(const StringPiece& message);
    void sendInLoop(const void* data, size_t len);
    void shutdownInLoop();
    // void shutdownAndForceCloseInLoop(double seconds);
    void forceCloseInLoop();
    void setState(StateE s)
    {
        state_ = s;
    }
    const char* stateToString() const;
    void startReadInLoop();
    void stopReadInLoop();

    EventLoop* loop_;
    const std::string name_;
    StateE state_; // FIXME: use atomic variable
    bool reading_;
    // we don't expose those classes to client.
    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;
    const InetAddress localAddr_;
    const InetAddress peerAddr_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    HighWaterMarkCallback highWaterMarkCallback_;
    CloseCallback closeCallback_;
    size_t highWaterMark_;
    Buffer inputBuffer_;
    Buffer outputBuffer_; // FIXME: use list<Buffer> as output buffer.
                          // FIXME: creationTime_, lastReceiveTime_
                          //        bytesReceived_, bytesSent_
};

typedef std::shared_ptr<StreamConnection> StreamConnectionPtr;

} // namespace toyBasket

#endif // _NET_STREAMCONNECTION_H
