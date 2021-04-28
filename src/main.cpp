/******************************************************************************
 * File name     : main.cpp
 * Description   : main func
 * Version       : v1.0
 * Create Time   : 2018/2/8
 * Author        : andy
 * Modify history:
 *******************************************************************************
 * Modify Time   Modify person  Modification
 * ------------------------------------------------------------------------------
 *
 *******************************************************************************/
#include "base/Timer.h"
#include "component/ComponentBase.h"
#include "config.h"

#include <fstream>
#include <iomanip>
#include <sstream>
#include <sys/stat.h>
#include <vector>

using namespace toyBasket;

/*******************************************************************************
 * function name : SplitString()
 * description   : split String
 * param[in]     : string
 * param[out]    : none
 * return        : 0: success -1: failure
 *******************************************************************************/
static void SplitString(const std::string& src, std::vector<std::string>& ret, const std::string& c)
{
    std::string::size_type pos1, pos2;
    pos1 = 0;
    pos2 = src.find(c);
    while (std::string::npos != pos2)
    {
        ret.push_back(src.substr(pos1, pos2 - pos1));
        pos1 = pos2 + c.size();
        pos2 = src.find(c, pos1);
    }
    if (pos1 != src.length())
    {
        ret.push_back(src.substr(pos1));
    }
}

int main(int argc, char** argv)
{
    google::LogSeverity severity = static_cast<int>(LOG_LEVEL);
    if (severity < 0 || severity > 3)
    {
        severity = google::WARNING;
    }
    google::InitGoogleLogging(argv[0]);
    google::SetLogDestination(severity, "running.log");
    FLAGS_logbufsecs = 0; //实时输出日志
    // FLAGS_max_log_size = 50; //最大日志大小（MB）
    FLAGS_stderrthreshold  = severity;
    FLAGS_colorlogtostderr = true;

    // 创建并运行计时器
    TimerManager::getInstance()->asyncWorkStart();

    // 注册组件
    std::vector<ComponentBase*> components;
    // components.push_back(TestComponent::getInstance());

    // 初始化组件
    for (auto& component : components)
    {
        component->init();
    }

    // 运行组件
    for (auto& component : components)
    {
        component->run();
    }

    sleep(2);

    while (1)
    {
        sleep(10);
    }

    google::ShutdownGoogleLogging();
    return 0;
}