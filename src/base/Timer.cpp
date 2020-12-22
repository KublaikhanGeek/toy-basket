/******************************************************************************
 * File name     : timer.cpp
 * Description   :
 * Version       : v1.0
 * Create Time   : 2019/7/30
 * Author        : andy
 * Modify history:
 *******************************************************************************
 * Modify Time   Modify person  Modification
 * ------------------------------------------------------------------------------
 *
 *******************************************************************************/
#include "Timer.h"
#include <algorithm>
#include <iostream>
#include <thread>

using namespace toyBasket;

TimerManager::TimerManager()
    : tick_(std::chrono::milliseconds(1)), isStart_(false), timeline_(0),
      autoIncrementId_(1) {}

TimerManager::~TimerManager() { workStop(); }

TimerManager *TimerManager::getInstance() {
  static TimerManager timerManager;
  return &timerManager;
}

unsigned long long TimerManager::addTimer(unsigned int interval,
                                          std::function<void()> action,
                                          bool isRepeat, bool isNoDelay,
                                          int id) {
  if (isInLoopThread()) {
    Timer event(interval, this->timeline_, std::move(action), isRepeat,
                isNoDelay);
    event.m_id = (id != -1) ? static_cast<unsigned long long>(id)
                            : this->autoIncrementId_++;

    timer_.push_back(event);
    std::push_heap(timer_.begin(), timer_.end(), std::greater<Timer>());

    return event.m_id;
  } else {
    std::unique_lock<std::mutex> lock(mutex_);
    Timer event(interval, this->timeline_, std::move(action), isRepeat,
                isNoDelay);
    event.m_id = (id != -1) ? static_cast<unsigned long long>(id)
                            : this->autoIncrementId_++;

    timer_.push_back(event);
    std::push_heap(timer_.begin(), timer_.end(), std::greater<Timer>());

    return event.m_id;
  }
}

void TimerManager::deleteTimer(unsigned long long &timerId) {
  if (isInLoopThread()) {
    if (timerId < 1) {
      return;
    }

    for (auto it = timer_.begin(); it != timer_.end();) {
      if (it->m_id == timerId) {
        it = timer_.erase(it);
        break;
      } else {
        ++it;
      }
    }

    timerId = 0;
    // 重新调整堆
    std::make_heap(timer_.begin(), timer_.end(), std::greater<Timer>());
  } else {
    if (timerId < 1) {
      return;
    }

    std::unique_lock<std::mutex> lock(mutex_);
    for (auto it = timer_.begin(); it != timer_.end();) {
      if (it->m_id == timerId) {
        it = timer_.erase(it);
        break;
      } else {
        ++it;
      }
    }

    timerId = 0;
    // 重新调整堆
    std::make_heap(timer_.begin(), timer_.end(), std::greater<Timer>());
  }
}

void TimerManager::loopForExecute() {
  std::unique_lock<std::mutex> lock(mutex_, std::defer_lock);
  lock.lock();
  if (!timer_.empty()) {
    //取出堆顶元素
    std::pop_heap(timer_.begin(), timer_.end(), std::greater<Timer>());
    Timer top(this->timer_.back());
    while (this->isStart_.load() && top.m_deadline <= this->timeline_) {
      //执行到时函数
      top.m_action(); // 可能调用stopTimer

      // 是否删除计时器
      auto it = std::find(timer_.begin(), timer_.end(), top);
      if (it == timer_.end()) {
        break;
      }

      if (timer_.empty()) {
        break;
      }

      //从堆中删除
      this->timer_.pop_back();

      if (top.m_isRepeat) {
        //如果是重复事件,则重新添加
        this->addTimer(top.m_interval, top.m_action, top.m_isRepeat, false,
                       static_cast<int>(top.m_id));
      }

      if (timer_.empty()) {
        break;
      }
      std::pop_heap(timer_.begin(), timer_.end(), std::greater<Timer>());
      top = this->timer_.back();
    }
    // 重新调整堆
    std::make_heap(timer_.begin(), timer_.end(), std::greater<Timer>());
  }
  lock.unlock();
  //执行一次后等待一个周期
  if (this->isStart_.load()) {
    std::this_thread::sleep_for(this->tick_);
  }
  //周期增1
  this->timeline_++;
  // LOG_INFO << "+++++++ circle: " << this->timeline_;
}

void TimerManager::asyncWorkStart() {
  if (!this->isStart_.load()) {
    std::thread work_thread(&TimerManager::syncWorkStart, this);
    work_thread.detach();
  }
}

void TimerManager::syncWorkStart() {
  threadId_ = std::this_thread::get_id();
  if (!this->isStart_.load()) {
    this->isStart_.exchange(true);
    while (this->isStart_.load()) {
      this->loopForExecute();
    }
  }
}

void TimerManager::workStop() { this->isStart_.exchange(false); }
