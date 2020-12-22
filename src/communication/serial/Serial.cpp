/******************************************************************************
 * File name     : Serial.cpp
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
#include <cerrno>
#include <cstdlib>
#include <fcntl.h>
#include <iomanip>
#include <sstream>
#include <unistd.h>

#include "Serial.h"

using namespace toyBasket;

Serial::Serial(EventLoop *loop, const std::string &nameArg,
               const std::string &path)
    : loop_(loop), devicePath_(path), fd_(-1), name_(nameArg), buffer_(),
      pos_(0), inputBuffer_(MSG_BUF_SIZE), highWaterMark_(16 * 1024 * 1024),
      writing_(false) {
  memset(buffer_, 0, MSG_BUF_SIZE);
}

Serial::~Serial() {
  while (writing_) {
    usleep(1000);
  }

  if (channel_) {
    channel_->disableAll();
    channel_->remove();
    while (!channel_->isIdle()) {
      usleep(10000);
    }
  }

  if (fd_ > 0) {
    close(fd_);
    fd_ = -1;
  }
}

int Serial::openPort() {
  LOG_INFO << name_ << " devName: " << devicePath_;
  /*1.open dev*/
  fd_ = open(devicePath_.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
  if (fd_ < 0) {
    LOG_ERROR << name_ << " serial: open device failed!: " << strerror(errno);
    return -1;
  }

  /*2.*/
  if (0 == isatty(STDIN_FILENO)) {
    LOG_INFO << name_ << " serial: isatty failed!";
  }

  return fd_;
}

int Serial::setPortParam(int speed, int databits, int stopbits, char parity) {
  struct termios newtio = {};
  struct termios oldtio = {};
  if (tcgetattr(fd_, &oldtio) != 0) {
    close(fd_);
    return -1;
  }

  memset(&newtio, 0, sizeof(newtio));
  newtio.c_cflag |= CLOCAL | CREAD;
  newtio.c_cflag &= static_cast<unsigned int>(~CSIZE);

  switch (databits) {
  case 7:
    newtio.c_cflag |= CS7;
    break;
  case 8:
    newtio.c_cflag |= CS8;
    break;
  default:
    break;
  }

  switch (parity) {
  case 'O': {
    newtio.c_cflag |= PARENB;
    newtio.c_cflag |= PARODD;
    newtio.c_iflag |= (INPCK | ISTRIP);
    break;
  }
  case 'E': {
    newtio.c_iflag |= (INPCK | ISTRIP);
    newtio.c_cflag |= PARENB;
    newtio.c_cflag &= static_cast<unsigned int>(~PARODD);
    break;
  }
  case 'N':
    newtio.c_cflag &= static_cast<unsigned int>(~PARENB);
    break;
  default:
    break;
  }

  speed_t rate = getSerialBaudrate(speed);
  cfsetispeed(&newtio, rate);
  cfsetospeed(&newtio, rate);

  if (1 == stopbits) {
    newtio.c_cflag &= static_cast<unsigned int>(~CSTOPB);
  } else if (2 == stopbits) {
    newtio.c_cflag |= CSTOPB;
  }

  newtio.c_cc[VTIME] = 0; // x100ms timeout
  newtio.c_cc[VMIN] = 1;  // read len depend read's third param
  tcflush(fd_, TCIOFLUSH);

  if ((tcsetattr(fd_, TCSANOW, &newtio)) != 0) {
    close(fd_);
    return -1;
  }

  return 0;
}

void Serial::showPortParam() {
  struct termios Npt = {};
  if (tcgetattr(fd_, &Npt) != 0) {
    LOG_ERROR << name_ << " tcgetattr fd_ New termios: " << strerror(errno);
    return;
  }

  LOG_INFO << name_ << " baud rate is "
           << (Npt.c_cflag & CBAUD); // band rate B115200 is 4098
  LOG_INFO << name_ << " data bit is "
           << (Npt.c_cflag & CSIZE); // data bits SC8 is 48
  LOG_INFO << name_ << " stop bit is "
           << (Npt.c_cflag & CSTOPB); // stop bit is 0
  LOG_INFO << name_ << " parity is "
           << (Npt.c_cflag & PARENB); // no parity is 0
  LOG_INFO << name_ << " flow control is "
           << (Npt.c_cflag & CRTSCTS); // no flow contral is 0
}

void Serial::start() {
  if (fd_ < 0) {
    LOG_ERROR << name_ << " serial file descriptor is not available!";
    return;
  }

  if (!channel_) {
    channel_.reset(new Channel(loop_, fd_));
    channel_->setWriteCallback(std::bind(&Serial::handleWrite, this));
    channel_->setReadCallback(std::bind(&Serial::handleRead, this));
    channel_->enableReading();
  } else {
    LOG_ERROR << name_ << " Please do not repeat";
  }
}

void Serial::send(const void *data, int len) {
  writing_.exchange(true);
  send(StringPiece(static_cast<const char *>(data), len));
}

void Serial::send(const StringPiece &message) {
  if (fd_ < 0) {
    LOG_ERROR << name_ << " serial file descriptor is not available!";
    return;
  }

  if (loop_->isInLoopThread()) {
    sendInLoop(message);
  } else {
    void (Serial::*fp)(const StringPiece &message) = &Serial::sendInLoop;
    loop_->runInLoop(std::bind(fp,
                               this, // FIXME
                               message.as_string()));
    // std::forward<string>(message)));
  }
}

void Serial::sendInLoop(const StringPiece &message) {
  sendInLoop(message.data(), static_cast<unsigned long long>(message.size()));
}

void Serial::sendInLoop(const void *data, size_t len) {
  loop_->assertInLoopThread();
  ssize_t nwrote = 0;
  size_t remaining = len;
  bool faultError = false;
  // if no thing in output queue, try writing directly
  if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0) {
    nwrote = ::write(channel_->fd(), data, len);
    if (nwrote >= 0) {
      remaining = len - static_cast<size_t>(nwrote);
      if (remaining == 0 && writeCompleteCallback_) {
        loop_->queueInLoop(std::bind(writeCompleteCallback_, this));
      }
    } else { // nwrote < 0
      nwrote = 0;
      if (errno != EWOULDBLOCK) {
        LOG_ERROR << "Serial::sendInLoop";
        if (errno == EPIPE || errno == ECONNRESET) { // FIXME: any others?
          faultError = true;
        }
      }
    }
  }

  assert(remaining <= len);
  if (!faultError && remaining > 0) {
    size_t oldLen = outputBuffer_.readableBytes();
    if (oldLen + remaining >= highWaterMark_ && oldLen < highWaterMark_ &&
        highWaterMarkCallback_) {
      loop_->queueInLoop(
          std::bind(highWaterMarkCallback_, this, oldLen + remaining));
    }
    outputBuffer_.append(static_cast<const char *>(data) + nwrote, remaining);
    if (!channel_->isWriting()) {
      channel_->enableWriting();
    }
  }

  writing_.exchange(false);
}

void Serial::handleWrite() {
  loop_->assertInLoopThread();
  if (channel_->isWriting()) {
    ssize_t n = ::write(channel_->fd(), outputBuffer_.peek(),
                        outputBuffer_.readableBytes());
    if (n > 0) {
      outputBuffer_.retrieve(static_cast<unsigned long long>(n));
      if (outputBuffer_.readableBytes() == 0) {
        channel_->disableWriting();
        if (writeCompleteCallback_) {
          loop_->queueInLoop(std::bind(writeCompleteCallback_, this));
        }
      }
    } else {
      LOG_ERROR << "Serial::handleWrite: " << strerror(errno);
    }
  } else {
    LOG_INFO << "fd = " << channel_->fd() << " is down, no more writing";
  }
}

void Serial::handleRead() {
  //
  // Gets an entire frame of data
  //
  int savedErrno = 0;
  ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedErrno);
  if (n > 0) {

    if (dataCallback_) {
      dataCallback_(this, messageCallback_);
    } else {
      inputBuffer_.retrieveAll();
    }
  }
}

void Serial::printData(const void *data, int len, const LogLevel &loglevel) {
  std::ostringstream oss;
  for (int i = 0; i < len; ++i) {
    oss << " " << std::setfill('0') << std::setw(2) << std::hex
        << ((static_cast<const char *>(data)[i]) & 0xff);
  }

  switch (loglevel) {
  case LOG_LEVEL_INFO:
    LOG_INFO << name_ << oss.str();
    break;

  case LOG_LEVEL_WARNING:
    LOG_WARNING << name_ << oss.str();
    break;

  case LOG_LEVEL_ERROR:
    LOG_ERROR << name_ << oss.str();
    break;

  case LOG_LEVEL_FATAL:
    LOG_FATAL << name_ << oss.str();
    break;

  default:
    break;
  }
}

unsigned int Serial::getCrcCheck(unsigned char *buf, unsigned int len) {
  unsigned int reCrc = 0xFFFFFFFF;
  unsigned int temp = 0;
  unsigned char cnt = 0;
  unsigned char *ptr = buf;

  const unsigned int xCode = 0x04C11DB7;
  unsigned int xbit = 0x80000000;
  while (len > 0) {
    xbit = 0x80000000;
    temp = 0;
    cnt = static_cast<unsigned char>((len > 4) ? 4 : len);
    for (int j = 0; j < cnt; j++) {
      temp |= static_cast<unsigned int>(ptr[j] << (8 * j));
    }

    for (int bit = 0; bit < 32; bit++) {
      if ((reCrc & 0x80000000) != 0) {
        reCrc <<= 1;
        reCrc ^= xCode;
      } else {
        reCrc <<= 1;
      }
      if ((temp & xbit) != 0) {
        reCrc ^= xCode;
      }
      xbit >>= 1;
    }
    ptr += cnt;
    len -= cnt;
  }

  return reCrc;
}

speed_t Serial::getSerialBaudrate(int rate) {
  switch (rate) {
  case 4800:
    return B4800;
  case 9600:
    return B9600;
  case 38400:
    return B38400;
  case 57600:
    return B57600;
  case 115200:
    return B115200;
  case 460800:
    return B460800;
  default:
    return B115200;
  }
}