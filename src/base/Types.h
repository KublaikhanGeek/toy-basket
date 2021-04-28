/******************************************************************************
 * File name     : Types.h
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
#ifndef _TYPES_H
#define _TYPES_H

#include <functional>
#include <glog/logging.h>
#include <memory>
#include <thread>

/******************************** Defines ***********************************/
#define LOG_INFO    LOG(INFO) << "[" << std::this_thread::get_id() << "] "
#define LOG_WARNING LOG(WARNING) << "[" << std::this_thread::get_id() << "] "
#define LOG_ERROR   LOG(ERROR) << "[" << std::this_thread::get_id() << "] "
#define LOG_FATAL   LOG(FATAL) << "[" << std::this_thread::get_id() << "] "

#define MSG_BUF_SIZE 4096

/******************************** Typdef ************************************/
enum LogLevel
{
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_FATAL
};

#endif /* _TYPES_H */