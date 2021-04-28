/******************************************************************************
 * File name     : TaskCommonLoopThread.cpp
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

#include "TaskCommonLoopThread.h"
#include "EventLoop.h"

using namespace toyBasket;

TaskCommonLoopThread::TaskCommonLoopThread(const ThreadLoopCallback& cb)
    : exiting_(false)
    , started_(false)
    , callback_(cb)
{
}

TaskCommonLoopThread::~TaskCommonLoopThread()
{
    this->stopLoop();
}

void TaskCommonLoopThread::startLoop()
{
    if (started_.load())
    {
        return;
    }

    thread_ = std::thread(std::bind(&TaskCommonLoopThread::threadFunc, this));
}

void TaskCommonLoopThread::stopLoop()
{
    exiting_ = true;
    started_.exchange(false);
    if (thread_.joinable())
    {
        thread_.join();
    }
}

void TaskCommonLoopThread::threadFunc()
{
    started_.exchange(true);

    while (!exiting_)
    {
        if (callback_)
        {
            callback_();
        }
        else
        {
            break;
        }
    }

    started_.exchange(false);
}
