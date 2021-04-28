/******************************************************************************
 * File name     : ComponentBase.cpp
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

#include <iomanip>
#include <sstream>

#include "ComponentBase.h"

using namespace toyBasket;

ComponentBase::ComponentBase()
    : inited_(false)
{
}

ComponentBase::~ComponentBase() = default;

void ComponentBase::addMsgToQueue(const std::string& msg)
{
    if (inited_.load())
    {
        msgQueue_.put(msg);
    }
    else
    {
        LOG_INFO << "Module not initialized";
    }
}

void ComponentBase::printMessage(const std::string& msg, const LogLevel& loglevel)
{
    std::ostringstream oss;
    for (auto val : msg)
    {
        oss << std::setfill('0') << std::setw(2) << std::hex << (val & 0xff) << " ";
    }

    switch (loglevel)
    {
    case LOG_LEVEL_INFO:
        LOG_INFO << oss.str();
        break;

    case LOG_LEVEL_WARNING:
        LOG_WARNING << oss.str();
        break;

    case LOG_LEVEL_ERROR:
        LOG_ERROR << oss.str();
        break;

    case LOG_LEVEL_FATAL:
        LOG_FATAL << oss.str();
        break;

    default:
        break;
    }
}

void ComponentBase::printMessage(const void* data, int len, const LogLevel& loglevel)
{
    std::ostringstream oss;
    for (int i = 0; i < len; ++i)
    {
        oss << std::setfill('0') << std::setw(2) << std::hex << ((static_cast<const char*>(data)[i]) & 0xff) << " ";
    }

    switch (loglevel)
    {
    case LOG_LEVEL_INFO:
        LOG_INFO << oss.str();
        break;

    case LOG_LEVEL_WARNING:
        LOG_WARNING << oss.str();
        break;

    case LOG_LEVEL_ERROR:
        LOG_ERROR << oss.str();
        break;

    case LOG_LEVEL_FATAL:
        LOG_FATAL << oss.str();
        break;

    default:
        break;
    }
}

unsigned int ComponentBase::getCrcCheck(char* buf, unsigned int len)
{
    unsigned int reCrc = 0xFFFFFFFF;
    unsigned int temp  = 0;
    char cnt           = 0;
    char* ptr          = buf;

    const unsigned int xCode = 0x04C11DB7;
    unsigned int xbit        = 0x80000000;
    while (len > 0)
    {
        xbit = 0x80000000;
        temp = 0;
        cnt  = static_cast<char>((len > 4) ? 4 : len);
        for (int j = 0; j < cnt; j++)
        {
            temp |= static_cast<unsigned int>(ptr[j] << (8 * j));
        }

        for (int bit = 0; bit < 32; bit++)
        {
            if ((reCrc & 0x80000000) != 0)
            {
                reCrc <<= 1;
                reCrc ^= xCode;
            }
            else
            {
                reCrc <<= 1;
            }
            if ((temp & xbit) != 0)
            {
                reCrc ^= xCode;
            }
            xbit >>= 1;
        }
        ptr += cnt;
        len -= cnt;
    }

    return reCrc;
}
