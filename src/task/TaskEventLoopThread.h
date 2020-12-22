/******************************************************************************
 * File name     : TaskEventLoopThread.h
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

#ifndef _TASKEVENTLOOPTHREAD_H
#define _TASKEVENTLOOPTHREAD_H

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <string>
#include <thread>

#include "noncopyable.h"

namespace toyBasket {

class EventLoop;

class TaskEventLoopThread : noncopyable {
public:
  typedef std::function<void(EventLoop *)> ThreadInitCallback;

  TaskEventLoopThread(const ThreadInitCallback &cb = ThreadInitCallback());
  ~TaskEventLoopThread();
  EventLoop *startLoop();
  void stopLoop();

private:
  void threadFunc();

  EventLoop *loop_;
  bool exiting_;
  std::atomic<bool> inited_;
  std::thread thread_;
  std::mutex mutex_;
  std::condition_variable cond_;
  ThreadInitCallback callback_;
};

} // namespace toyBasket

#endif // _TASKEVENTLOOPTHREAD_H
