/******************************************************************************
 * File name     : ComponentBase.h
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
#ifndef _COMPONENTBASE_H
#define _COMPONENTBASE_H

#include <atomic>

#include "BlockingQueue.h"
#include "Types.h"

namespace toyBasket {

class ComponentBase {
protected:
  ComponentBase();

public:
  virtual ~ComponentBase();

public:
  virtual void init() = 0;
  virtual void run() = 0;
  virtual void stop() = 0;
  virtual void uninit() = 0;

  void addMsgToQueue(const std::string &msg);

protected:
  unsigned int getCrcCheck(char *buf, unsigned int len);
  void printMessage(const std::string &msg,
                    const LogLevel &loglevel = LOG_LEVEL_INFO);
  void printMessage(const void *data, int len,
                    const LogLevel &loglevel = LOG_LEVEL_INFO);

protected:
  std::atomic<bool> inited_;
  BlockingQueue<std::string> msgQueue_;
};

} // namespace toyBasket

#endif //_COMPONENTBASE_H