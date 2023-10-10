#include "select_demultiplexer.h"

#include "initiation_dispatcher.h"
namespace reactor {

int SelectDemultiplexer::awaiting(
    std::map<Handle, EventHandlerMapEntry*>& event_handlers,
    const struct timeval* timeout) {
  struct timeval tv;
  struct timeval* tv_ptr = nullptr;
  if (timeout != nullptr) {
    tv = *timeout;
    tv_ptr = &tv;
  }

  // 设置select需要监视的文件描述符集
  int maxfd = setup(event_handlers);
  int count = select(maxfd + 1, &readfds, &writefds, &exceptfds, tv_ptr);

  if (count > 0) {
    // 更新
    update(event_handlers);
  }
  return count;
}

int SelectDemultiplexer::setup(
    std::map<Handle, EventHandlerMapEntry*>& event_handlers) {
  FD_ZERO(&readfds);
  FD_ZERO(&writefds);
  FD_ZERO(&exceptfds);
  std::lock_guard<std::mutex> lock(event_handlers_muptex);
  for (auto it = event_handlers.begin(); it != event_handlers.end(); it++) {
    if (it->second->events_ & (READ_EVENT | ACCEPT_EVENT)) {
      FD_SET(it->first.GetFileDescriptor(), &readfds);
      it->second->revents_ = 0U;
    }
    if (it->second->events_ & WRITE_EVENT) {
      FD_SET(it->first.GetFileDescriptor(), &writefds);
      it->second->revents_ = 0U;
    }
    if (it->second->events_ & EXCEPTION_EVENT) {
      FD_SET(it->first.GetFileDescriptor(), &exceptfds);
      it->second->revents_ = 0U;
    }
  }
  // max fd
  return event_handlers.cbegin()->first.GetFileDescriptor();
}

void SelectDemultiplexer::update(
    std::map<Handle, EventHandlerMapEntry*>& event_handlers) {
  std::lock_guard<std::mutex> lock(event_handlers_muptex);
  for (auto it = event_handlers.begin(); it != event_handlers.end(); it++) {
    if (FD_ISSET(it->first.GetFileDescriptor(), &readfds)) {
      // read,accept,or read and accept
      it->second->revents_ |=
          (it->second->events_ & (READ_EVENT | ACCEPT_EVENT));
    }
    if (FD_ISSET(it->first.GetFileDescriptor(), &writefds)) {
      it->second->revents_ |= WRITE_EVENT;
    }
    if (FD_ISSET(it->first.GetFileDescriptor(), &exceptfds)) {
      it->second->revents_ |= EXCEPTION_EVENT;
    }
  }
}

}  // namespace reactor
