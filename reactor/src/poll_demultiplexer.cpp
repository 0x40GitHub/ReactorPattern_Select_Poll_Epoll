#include "poll_demultiplexer.h"

#include "initiation_dispatcher.h"
namespace reactor {

PollDemultiplexer::~PollDemultiplexer() {}

int PollDemultiplexer::awaiting(
    std::map<Handle, EventHandlerMapEntry*>& event_handlers,
    const timeval* timeout) {
  int timeout_msec = timeout->tv_sec * 1e3 + timeout->tv_usec * 1e-3;

  setup(event_handlers);

  int ready = poll(fds_.data(), fds_.size(), -1);
  if (ready > 0) {
    update(event_handlers);
  }
  return ready;
}

// set pollfd
int PollDemultiplexer::setup(
    std::map<Handle, EventHandlerMapEntry*>& event_handlers) {
  std::lock_guard<std::mutex> lock(event_handlers_muptex);
  for (auto it = event_handlers.begin(); it != event_handlers.end(); it++) {
    it->second->revents_ = 0U;

    pollfd poll_fd;
    poll_fd.fd = it->first.GetFileDescriptor();
    if (it->second->events_ & (READ_EVENT | ACCEPT_EVENT))
      poll_fd.events = POLLIN;
    if (it->second->events_ & WRITE_EVENT) poll_fd.events = POLLOUT;
    if (it->second->events_ & EXCEPTION_EVENT) poll_fd.events = POLLERR;
    poll_fd.revents = 0;
    fds_.push_back(poll_fd);
  }
  return 0;
}

// update event_handlers
void PollDemultiplexer::update(
    std::map<Handle, EventHandlerMapEntry*>& event_handlers) {
  std::lock_guard<std::mutex> lock(event_handlers_muptex);
  for (auto poll_fd : fds_) {
    if (poll_fd.revents & POLLIN) {
      // read,accept,or read and accept
      event_handlers[Handle(poll_fd.fd)]->revents_ |=
          (event_handlers[Handle(poll_fd.fd)]->events_ &
           (READ_EVENT | ACCEPT_EVENT));
    }
    if (poll_fd.revents & POLLOUT) {
      event_handlers[Handle(poll_fd.fd)]->revents_ |= WRITE_EVENT;
    }
    if (poll_fd.revents & POLLERR) {
      event_handlers[Handle(poll_fd.fd)]->revents_ |= EXCEPTION_EVENT;
    }
    poll_fd.revents = 0;
  }
  // clear fds_
  fds_.clear();
}

}  // namespace reactor
