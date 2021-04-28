/******************************************************************************
 * File name     : timer.h
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
#ifndef _TIMER_H
#define _TIMER_H
#include <atomic>
#include <chrono>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

namespace toyBasket
{

class Timer
{
public:
    Timer(unsigned int interval, unsigned long long timeline, std::function<void()> action, bool isRepeat,
          bool isNoDelay)
    {
        this->m_interval             = interval;
        isNoDelay ? this->m_deadline = timeline : this->m_deadline = interval + timeline;
        this->m_action                                             = action;
        this->m_isRepeat                                           = isRepeat;
    }
    bool operator>(const Timer& timer) const
    {
        return m_deadline > timer.m_deadline;
    }
    bool operator==(const Timer& timer) const
    {
        return m_id == timer.m_id;
    }

public:
    friend class TimerManager;

private:
    unsigned long long m_id;        //定时事件的唯一标示id
    unsigned int m_interval;        //事件的触发间隔，在重复事件中会用到这个属性
    unsigned long long m_deadline;  //定时事件的触发时间
    std::function<void()> m_action; //触发的事件
    bool m_isRepeat;                //是否是重复执行事件
};

class TimerManager
{
public:
    ~TimerManager();

private:
    TimerManager();

public:
    static TimerManager* getInstance();

    /**
     * @description  添加定时器事件
     * @param interval 定时间隔
     * @param action 定时执行的动作
     * @param isRepeat 是否重复执行,默认不重复执行
     * @param isNoDelay 是否立即执行
     * @return unsigned int 定时器的id,可以根据这个id执行删除操作
     */
    unsigned long long addTimer(unsigned int interval, std::function<void()> action, bool isRepeat = false,
                                bool isNoDelay = false, int id = -1);

    /**
     * 删除定时器
     * @param timerId 定时器id
     */
    void deleteTimer(unsigned long long& timerId);

    /**
     * 同步执行启动定时器
     */
    void syncWorkStart();

    /**
     * 异步执行启动定时器
     */
    void asyncWorkStart();

    /**
     * 停止定时器
     */
    void workStop();

private:
    /**
     * 定时器处理
     */
    void loopForExecute();
    bool isInLoopThread() const
    {
        return threadId_ == std::this_thread::get_id();
    }

private:
    std::chrono::milliseconds tick_;     //每次tick间隔
    std::atomic<bool> isStart_;          //标志当前定时器的启动状态
    unsigned long long timeline_;        //当前时间线
    unsigned long long autoIncrementId_; //当前id
    // std::thread threadId_;            //工作线程

    std::mutex mutex_;
    std::vector<Timer> timer_;
    std::thread::id threadId_;
};

} // namespace toyBasket
#endif /* _TIMER_H */
