#ifndef SELECT_DEMULTIPLEXER_H
#define SELECT_DEMULTIPLEXER_H

#include <sys/select.h>

#include "event_demultiplexer.h"

namespace reactor {

class SelectDemultiplexer : public EventDemultiplexer {
 public:
  virtual int awaiting(std::map<Handle, EventHandlerMapEntry*>& event_handlers,
                       const struct timeval* timeout) override;

 private:
  fd_set readfds, writefds, exceptfds;
  int setup(std::map<Handle, EventHandlerMapEntry*>& event_handlers);
  void update(std::map<Handle, EventHandlerMapEntry*>& event_handlers);
};

}  // namespace reactor
#endif