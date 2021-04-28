/******************************************************************************
 * File name     : TaskEventLoopThread.cpp
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

#include "TaskEventLoopThread.h"
#include "EventLoop.h"

using namespace toyBasket;

TaskEventLoopThread::TaskEventLoopThread(const ThreadInitCallback& cb)
    : loop_(NULL)
    , exiting_(false)
    , inited_(false)
    , callback_(cb)
{
}

TaskEventLoopThread::~TaskEventLoopThread()
{
    this->stopLoop();
}

EventLoop* TaskEventLoopThread::startLoop()
{
    if (inited_.load())
    {
        return loop_;
    }

    thread_ = std::thread(std::bind(&TaskEventLoopThread::threadFunc, this));

    EventLoop* loop = NULL;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while (loop_ == NULL)
        {
            cond_.wait(lock, [this] { return inited_.load(); });
        }
        loop = loop_;
    }

    return loop;
}

void TaskEventLoopThread::stopLoop()
{
    exiting_ = true;
    if (loop_ != NULL)
    { // not 100% race-free, eg. threadFunc could be running callback_.
        // still a tiny chance to call destructed object, if threadFunc exits just
        // now. but when TaskEventLoopThread destructs, usually programming is
        // exiting anyway.
        loop_->quit();
        thread_.join();
    }
}

void TaskEventLoopThread::threadFunc()
{
    EventLoop loop;

    if (callback_)
    {
        callback_(&loop);
    }

    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_ = &loop;
        inited_.exchange(true);
        cond_.notify_one();
    }

    loop.loop();
    // assert(exiting_);
    std::unique_lock<std::mutex> lock(mutex_);
    loop_ = NULL;
}
