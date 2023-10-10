#include "epoll_demultiplexer.h"

#include "initiation_dispatcher.h"

namespace reactor {

EPollDemultiplexer::EPollDemultiplexer() : epoll_instance_(epoll_create1(0)) {}

int EPollDemultiplexer::setup(
    std::map<Handle, EventHandlerMapEntry*>& event_handlers) {
  std::lock_guard<std::mutex> lock(event_handlers_muptex);
  for (auto it = event_handlers.begin(); it != event_handlers.end(); it++) {
    it->second->revents_ = 0;
    epoll_event event{0};
    if (it->second->events_ & (READ_EVENT | ACCEPT_EVENT))
      event.events |= EPOLLIN;
    if (it->second->events_ & WRITE_EVENT) event.events |= EPOLLOUT;
    if (it->second->events_ & EXCEPTION_EVENT) event.events |= EPOLLERR;
    event.data.fd = it->first.GetFileDescriptor();

    if (fd_set_.count(it->first.GetFileDescriptor())) {
      // just modify
      epoll_ctl(epoll_instance_, EPOLL_CTL_MOD, it->first.GetFileDescriptor(),
                &event);
    } else {
      fd_set_.insert(
          it->first.GetFileDescriptor());  // trace the fd in the epoll instance
      // add
      epoll_ctl(epoll_instance_, EPOLL_CTL_ADD, it->first.GetFileDescriptor(),
                &event);
    }
  }
  // clear
  for (auto it = fd_set_.begin(); it != fd_set_.end();) {
    if (!event_handlers.count(Handle(*it))) {
      epoll_event remove_event;
      remove_event.data.fd = *it;
      remove_event.events = 0;
      epoll_ctl(epoll_instance_, EPOLL_CTL_DEL, *it, &remove_event);
      it = fd_set_.erase(it);
    } else {
      it++;
    }
  }
  events_.clear();
  events_.resize(fd_set_.size());
  return 0;
}

void EPollDemultiplexer::update(
    std::map<Handle, EventHandlerMapEntry*>& event_handlers) {
  std::lock_guard<std::mutex> lock(event_handlers_muptex);
  for (auto it = events_.begin(); it != events_.end(); it++) {
    Handle handle(it->data.fd);
    if (it->events & EPOLLIN) {
      event_handlers[handle]->revents_ |=
          (event_handlers[handle]->events_ & (READ_EVENT | ACCEPT_EVENT));
    }
    if (it->events & EPOLLOUT) {
      event_handlers[handle]->revents_ |= WRITE_EVENT;
    }
    if (it->events & EPOLLERR) {
      event_handlers[handle]->revents_ |= EXCEPTION_EVENT;
    }
  }
  return;
}

int EPollDemultiplexer::awaiting(
    std::map<Handle, EventHandlerMapEntry*>& event_handlers,
    const timeval* timeout) {
  int timeout_msec = timeout->tv_sec * 1e3 + timeout->tv_usec * 1e-3;
  setup(event_handlers);
  int ready =
      epoll_wait(epoll_instance_, events_.data(), events_.size(), timeout_msec);
  if (ready > 0) {
    update(event_handlers);
  }
  return ready;
}

}  // namespace reactor