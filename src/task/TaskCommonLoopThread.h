/******************************************************************************
 * File name     : TaskCommonLoopThread.h
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

#ifndef _TASKCOMMONLOOPTHREAD_H
#define _TASKCOMMONLOOPTHREAD_H

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <string>
#include <thread>

#include "noncopyable.h"

namespace toyBasket {

class EventLoop;

class TaskCommonLoopThread : noncopyable {
public:
  typedef std::function<void()> ThreadLoopCallback;

  TaskCommonLoopThread(const ThreadLoopCallback &cb = ThreadLoopCallback());
  ~TaskCommonLoopThread();
  void startLoop();
  void stopLoop();

private:
  void threadFunc();

  bool exiting_;
  std::atomic<bool> started_;
  std::thread thread_;
  ThreadLoopCallback callback_;
};

} // namespace toyBasket

#endif // _TASKCOMMONLOOPTHREAD_H
