/******************************************************************************
 * File name     : EPollPoller.cpp
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
#include "EPollPoller.h"

#include "Channel.h"
#include "Types.h"

#include <cassert>
#include <cerrno>
#include <cstring>
#include <poll.h>
#include <sys/epoll.h>
#include <unistd.h>

using namespace toyBasket;

// On Linux, the constants of poll(2) and epoll(4)
// are expected to be the same.
static_assert(EPOLLIN == POLLIN, "epoll uses same flag values as poll");
static_assert(EPOLLPRI == POLLPRI, "epoll uses same flag values as poll");
static_assert(EPOLLOUT == POLLOUT, "epoll uses same flag values as poll");
static_assert(EPOLLRDHUP == POLLRDHUP, "epoll uses same flag values as poll");
static_assert(EPOLLERR == POLLERR, "epoll uses same flag values as poll");
static_assert(EPOLLHUP == POLLHUP, "epoll uses same flag values as poll");

namespace {
const int kNew = -1;
const int kAdded = 1;
const int kDeleted = 2;
} // namespace

EPollPoller::EPollPoller(EventLoop *loop)
    : Poller(loop), epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
      events_(kInitEventListSize) {
  if (epollfd_ < 0) {
    LOG_FATAL << "EPollPoller::EPollPoller";
  }
}

EPollPoller::~EPollPoller() { ::close(epollfd_); }

void EPollPoller::poll(int timeoutMs, ChannelList *activeChannels) {
  LOG_INFO << "fd total count " << m_channels.size();
  int numEvents = ::epoll_wait(epollfd_, &*events_.begin(),
                               static_cast<int>(events_.size()), timeoutMs);
  int savedErrno = errno;
  if (numEvents > 0) {
    LOG_INFO << numEvents << " events happened";
    fillActiveChannels(numEvents, activeChannels);
    if (static_cast<size_t>(numEvents) == events_.size()) {
      events_.resize(events_.size() * 2);
    }
  } else if (numEvents == 0) {
    LOG_INFO << "nothing happened";
  } else {
    // error happens, log uncommon ones
    if (savedErrno != EINTR) {
      errno = savedErrno;
      LOG_ERROR << "EPollPoller::poll()";
    }
  }
}

void EPollPoller::fillActiveChannels(int numEvents,
                                     ChannelList *activeChannels) const {
  assert(static_cast<size_t>(numEvents) <= events_.size());
  for (int i = 0; i < numEvents; ++i) {
    Channel *channel = static_cast<Channel *>(
        events_[static_cast<unsigned long long>(i)].data.ptr);
#if 0
        int fd = channel->fd();
        ChannelMap::const_iterator it = m_channels.find(fd);
        assert(it != m_channels.end());
        assert(it->second == channel);
#endif
    channel->set_revents(
        static_cast<int>(events_[static_cast<unsigned long long>(i)].events));
    activeChannels->push_back(channel);
  }
}

void EPollPoller::updateChannel(Channel *channel) {
  Poller::assertInLoopThread();
  const int index = channel->index();
  LOG_INFO << "fd = " << channel->fd() << " events = " << channel->events()
           << " index = " << index;
  if (index == kNew || index == kDeleted) {
    // a new one, add with EPOLL_CTL_ADD
    int fd = channel->fd();
    if (index == kNew) {
      assert(m_channels.find(fd) == m_channels.end());
      m_channels[fd] = channel;
    } else { // index == kDeleted
      assert(m_channels.find(fd) != m_channels.end());
      assert(m_channels[fd] == channel);
    }

    channel->set_index(kAdded);
    update(EPOLL_CTL_ADD, channel);
  } else {
    // update existing one with EPOLL_CTL_MOD/DEL
    int fd = channel->fd();
    (void)fd;
    assert(m_channels.find(fd) != m_channels.end());
    assert(m_channels[fd] == channel);
    assert(index == kAdded);
    if (channel->isNoneEvent()) {
      update(EPOLL_CTL_DEL, channel);
      channel->set_index(kDeleted);
    } else {
      update(EPOLL_CTL_MOD, channel);
    }
  }
}

void EPollPoller::removeChannel(Channel *channel) {
  Poller::assertInLoopThread();
  int fd = channel->fd();
  LOG_INFO << "fd = " << fd;
  assert(m_channels.find(fd) != m_channels.end());
  assert(m_channels[fd] == channel);
  assert(channel->isNoneEvent());
  int index = channel->index();
  assert(index == kAdded || index == kDeleted);
  size_t n = m_channels.erase(fd);
  (void)n;
  assert(n == 1);

  if (index == kAdded) {
    update(EPOLL_CTL_DEL, channel);
  }
  channel->set_index(kNew);
}

void EPollPoller::update(int operation, Channel *channel) {
  struct epoll_event event = {};
  event.events = static_cast<unsigned int>(channel->events());
  event.data.ptr = channel;
  int fd = channel->fd();
  LOG_INFO << "epoll_ctl op = " << operationToString(operation)
           << " fd = " << fd << " event = { " << channel->eventsToString()
           << " }";
  if (::epoll_ctl(epollfd_, operation, fd, &event) < 0) {
    if (operation == EPOLL_CTL_DEL) {
      LOG_ERROR << "epoll_ctl op =" << operationToString(operation)
                << " fd =" << fd;
    } else {
      LOG_FATAL << "epoll_ctl op =" << operationToString(operation)
                << " fd =" << fd;
    }
  }
}

const char *EPollPoller::operationToString(int op) {
  switch (op) {
  case EPOLL_CTL_ADD:
    return "ADD";
  case EPOLL_CTL_DEL:
    return "DEL";
  case EPOLL_CTL_MOD:
    return "MOD";
  default:
    assert(false && "ERROR op");
    return "Unknown Operation";
  }
}
