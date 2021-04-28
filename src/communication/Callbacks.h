/******************************************************************************
 * File name     : Callbacks.h
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

#ifndef _CALLBACKS_H
#define _CALLBACKS_H

#include <functional>
#include <memory>
#include <sys/socket.h>

namespace toyBasket
{

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

// should really belong to base/Types.h, but <memory> is not included there.

template <typename T>
inline T* get_pointer(const std::shared_ptr<T>& ptr)
{
    return ptr.get();
}

template <typename T>
inline T* get_pointer(const std::unique_ptr<T>& ptr)
{
    return ptr.get();
}

// All client visible callbacks go here.
class InetAddress;
class Buffer;
class StreamConnection;
class Serial;
typedef std::shared_ptr<StreamConnection> StreamConnectionPtr;
typedef std::function<void()> TimerCallback;
typedef std::function<void(const StreamConnectionPtr&)> ConnectionCallback;
typedef std::function<void(const StreamConnectionPtr&)> CloseCallback;
typedef std::function<void(const StreamConnectionPtr&)> WriteCompleteCallback;
typedef std::function<void(const StreamConnectionPtr&, size_t)> HighWaterMarkCallback;

// the data has been read to (buf, len)
typedef std::function<void(const StreamConnectionPtr&, Buffer*)> MessageCallback;

// for udp
typedef std::function<void(const InetAddress&, const void*, int)> DgramEventCallback;

typedef std::function<void(const struct sockaddr&, const void*, int)> InetEventCallback;

// for serial
typedef std::function<void(const void*, int)> MsgHandleCallback;
typedef std::function<void(Serial*, const MsgHandleCallback&)> DataCallback;

void defaultConnectionCallback(const StreamConnectionPtr& conn);
void defaultMessageCallback(const StreamConnectionPtr& conn, Buffer* buffer);

} // namespace toyBasket

#endif // _CALLBACKS_H
