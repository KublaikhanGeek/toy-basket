/******************************************************************************
 * File name     : Serial.h
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

#ifndef _SERIAL_H
#define _SERIAL_H

#include <termios.h>

#include "Buffer.h"
#include "Callbacks.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Types.h"
#include "noncopyable.h"

#include <atomic>

namespace toyBasket
{

class Serial;
typedef std::function<void(const Serial*)> SerialWriteCompleteCallback;
typedef std::function<void(const Serial*, size_t)> SerialHighWaterMarkCallback;

class Serial : noncopyable
{
public:
    Serial(EventLoop* loop, const std::string& nameArg, const std::string& path);
    ~Serial();

    void start();
    int openPort();

    /*******************************************************************************
     * function name : setPortParam
     * description   : 设置串口终端参数
     * param[in]     : speed 波特率
     *                 databits 数据位
     *                 stopbits 停止位
     *                 parity 奇偶校验
     * param[out]    : none
     * return        : 0:success,-1:failed
     *******************************************************************************/
    int setPortParam(int speed, int databits, int stopbits, char parity);
    void showPortParam();

    void send(const void* data, int len);

    void setMessageCallback(const DataCallback& cb, const MsgHandleCallback& callback)
    {
        dataCallback_    = cb;
        messageCallback_ = callback;
    }

    void setWriteCompleteCallback(const SerialWriteCompleteCallback& cb)
    {
        writeCompleteCallback_ = cb;
    }

    void setHighWaterMarkCallback(const SerialHighWaterMarkCallback& cb, size_t highWaterMark)
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

    const std::string& name() const
    {
        return name_;
    }

private:
    void handleWrite();
    void handleRead();
    unsigned int getCrcCheck(unsigned char* buf, unsigned int len);
    void printData(const void* data, int len, const LogLevel& loglevel = LOG_LEVEL_INFO);
    speed_t getSerialBaudrate(int rate);
    void send(const StringPiece& message);
    void sendInLoop(const StringPiece& message);
    void sendInLoop(const void* data, size_t len);

private:
    EventLoop* loop_;
    std::unique_ptr<Channel> channel_;
    const std::string devicePath_;
    int fd_;
    const std::string name_;
    unsigned char buffer_[MSG_BUF_SIZE];
    int pos_;
    Buffer inputBuffer_;
    DataCallback dataCallback_;
    MsgHandleCallback messageCallback_;
    SerialWriteCompleteCallback writeCompleteCallback_;
    SerialHighWaterMarkCallback highWaterMarkCallback_;
    size_t highWaterMark_;
    Buffer outputBuffer_;
    std::atomic<bool> writing_;
};

} // namespace toyBasket

#endif // _SERIAL_H
