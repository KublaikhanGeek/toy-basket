/******************************************************************************
 * File name     : Poller.cpp
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

#include "Poller.h"

#include "Channel.h"

using namespace toyBasket;

Poller::Poller(EventLoop* loop)
    : m_ownerLoop(loop)
{
}

Poller::~Poller() = default;

bool Poller::hasChannel(Channel* channel) const
{
    assertInLoopThread();
    ChannelMap::const_iterator it = m_channels.find(channel->fd());
    return it != m_channels.end() && it->second == channel;
}
