/******************************************************************************
 * File name     : EventLoop.h
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

#ifndef _EVENTLOOP_H
#define _EVENTLOOP_H

#include "noncopyable.h"

#include <atomic>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

namespace toyBasket {

class Channel;
class Poller;

///
/// Reactor, at most one per thread.
///
/// This is an interface class, so don't expose too much details.
class EventLoop : noncopyable {
public:
  typedef std::function<void()> Functor;
  EventLoop();
  ~EventLoop(); // force out-line dtor, for std::unique_ptr members.

  ///
  /// Loops forever.
  ///
  /// Must be called in the same thread as creation of the object.
  ///
  void loop();

  /// Quits loop.
  ///
  /// This is not 100% thread safe, if you call through a raw pointer,
  /// better to call through shared_ptr<EventLoop> for 100% safety.
  void quit();

  /// Runs callback immediately in the loop thread.
  /// It wakes up the loop, and run the cb.
  /// If in the same loop thread, cb is run within the function.
  /// Safe to call from other threads.
  void runInLoop(Functor cb);
  /// Queues callback in the loop thread.
  /// Runs after finish pooling.
  /// Safe to call from other threads.
  void queueInLoop(Functor cb);

  size_t queueSize() const;

  // internal usage
  void wakeup();
  void updateChannel(Channel *channel);
  void removeChannel(Channel *channel);
  bool hasChannel(Channel *channel);

  void assertInLoopThread() {
    if (!isInLoopThread()) {
      abortNotInLoopThread();
    }
  }
  bool isInLoopThread() const {
    return threadId_ == std::this_thread::get_id();
  }
  // bool callingPendingFunctors() const { return callingPendingFunctors_; }
  bool eventHandling() const { return eventHandling_; }

  static EventLoop *getEventLoopOfCurrentThread();

private:
  void abortNotInLoopThread();
  void handleRead(); // waked up
  void doPendingFunctors();

  void printActiveChannels() const; // DEBUG

  typedef std::vector<Channel *> ChannelList;

  bool looping_; /* atomic */
  std::atomic<bool> quit_;
  bool eventHandling_;          /* atomic */
  bool callingPendingFunctors_; /* atomic */
  const std::thread::id threadId_;
  std::unique_ptr<Poller> poller_;
  int wakeupFd_;
  // we don't expose Channel to client.
  std::unique_ptr<Channel> wakeupChannel_;

  // scratch variables
  ChannelList activeChannels_;
  Channel *currentActiveChannel_;
  mutable std::mutex mutex_;
  std::vector<Functor> pendingFunctors_;
};
} // namespace toyBasket

#endif // _EVENTLOOP_H
