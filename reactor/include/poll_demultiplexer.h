#ifndef POLLDEMULTIPLEXER_H
#define POLLDEMULTIPLEXER_H
#include <poll.h>

#include <vector>

#include "event_demultiplexer.h"
namespace reactor {

class PollDemultiplexer : public EventDemultiplexer {
 public:
  virtual int awaiting(std::map<Handle, EventHandlerMapEntry*>& event_handlers,
                       const struct timeval* timeout) override;
  PollDemultiplexer() = default;
  virtual ~PollDemultiplexer();

 private:
  std::vector<pollfd> fds_;
  int setup(std::map<Handle, EventHandlerMapEntry*>& event_handlers);
  void update(std::map<Handle, EventHandlerMapEntry*>& event_handlers);
};

}  // namespace reactor
#endif