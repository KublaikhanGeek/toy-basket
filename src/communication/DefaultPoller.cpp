/******************************************************************************
 * File name     : DefaultPoller.cpp
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
#include "EPollPoller.h"

#include <cstdlib>

using namespace toyBasket;

Poller* Poller::newDefaultPoller(EventLoop* loop)
{
#if 0
    if (::getenv("USE_POLL")) {
        return new PollPoller(loop);
    } else {
        return new EPollPoller(loop);
    }
#endif
    return new EPollPoller(loop);
}
