#ifndef EPOLL_DEMULTIPLEXER_H
#define EPOLL_DEMULTIPLEXER_H
#include <sys/epoll.h>

#include <unordered_set>
#include <vector>

#include "event_demultiplexer.h"
namespace reactor {

class EPollDemultiplexer : public EventDemultiplexer {
 public:
  virtual int awaiting(std::map<Handle, EventHandlerMapEntry*>& event_handlers,
                       const struct timeval* timeout) override;
  EPollDemultiplexer();
  virtual ~EPollDemultiplexer() {}

 private:
  std::unordered_set<int> fd_set_;
  std::vector<epoll_event> events_;
  int epoll_instance_;
  int setup(std::map<Handle, EventHandlerMapEntry*>& event_handlers);
  void update(std::map<Handle, EventHandlerMapEntry*>& event_handlers);
};
}  // namespace reactor

#endif