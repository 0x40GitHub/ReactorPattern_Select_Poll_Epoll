#ifndef EVENT_DEMULTIPLEXER_H
#define EVENT_DEMULTIPLEXER_H

#include <sys/time.h>

#include <map>
#include <mutex>

#include "event_handler.h"
#include "handle.h"

namespace reactor {

class EventDemultiplexer {
 public:
  ~EventDemultiplexer() = default;
  virtual int awaiting(std::map<Handle, EventHandlerMapEntry*>& event_handlers,
                       const struct timeval* timeout) = 0;

 protected:
  std::mutex event_handlers_muptex;
};

}  // namespace reactor

#endif