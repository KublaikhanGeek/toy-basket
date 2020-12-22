/******************************************************************************
 * File name     : EPollPoller.h
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

#ifndef _POLLER_EPOLLPOLLER_H
#define _POLLER_EPOLLPOLLER_H

#include "Poller.h"

#include <vector>

struct epoll_event;

namespace toyBasket {

///
/// IO Multiplexing with epoll(4).
///
class EPollPoller : public Poller {
public:
  EPollPoller(EventLoop *loop);
  ~EPollPoller() override;

  void poll(int timeoutMs, ChannelList *activeChannels) override;
  void updateChannel(Channel *channel) override;
  void removeChannel(Channel *channel) override;

private:
  static const int kInitEventListSize = 16;

  static const char *operationToString(int op);

  void fillActiveChannels(int numEvents, ChannelList *activeChannels) const;
  void update(int operation, Channel *channel);

  typedef std::vector<struct epoll_event> EventList;

  int epollfd_;
  EventList events_;
};

} // namespace toyBasket
#endif // _POLLER_EPOLLPOLLER_H
