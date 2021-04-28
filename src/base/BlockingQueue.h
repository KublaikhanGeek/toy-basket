/******************************************************************************
 * File name     : BlockingQueue.h
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
#ifndef _BLOCKINGQUEUE_H
#define _BLOCKINGQUEUE_H

#include "noncopyable.h"

#include <assert.h>
#include <condition_variable>
#include <deque>
#include <iostream>
#include <mutex>

namespace toyBasket
{

template <typename T>
class BlockingQueue : noncopyable
{
public:
    BlockingQueue()
        : queue_()
    {
    }

    void put(const T& x)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        queue_.push_back(x);
        notEmpty_.notify_one(); // wait morphing saves us
                                // http://www.domaigne.com/blog/computing/condvars-signal-with-mutex-locked-or-not/
    }

    void put(T&& x)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        queue_.push_back(std::move(x));
        notEmpty_.notify_one();
    }

    T take()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        // always use a while-loop, due to spurious wakeup
        while (queue_.empty())
        {
            notEmpty_.wait(lock);
        }
        assert(!queue_.empty());
        T front(std::move(queue_.front()));
        queue_.pop_front();
        return std::move(front);
    }

    size_t size() const
    {
        std::unique_lock<std::mutex> lock(mutex_);
        return queue_.size();
    }

private:
    mutable std::mutex mutex_;
    std::condition_variable notEmpty_;
    std::deque<T> queue_;
};

} // namespace toyBasket

#endif // _BLOCKINGQUEUE_H
