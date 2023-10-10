#ifndef EVENT_HANDLER_H
#define EVENT_HANDLER_H

#include <cstdint>  // uint16_t

#include "handle.h"
namespace reactor {

class EventHandler {
 public:
  virtual ~EventHandler() = default;

  virtual void handle_read(Handle handle) {}

  virtual void handle_write(Handle handle) {}

  virtual void handle_exception(Handle handle) {}

  virtual void handle_accept(Handle handle) {}
};

typedef struct EventHandlerMapEntry {
  EventHandler* event_handler_;  // event handler
  bool one_shot_;     // The event is unregistered after being processed once
  uint16_t events_;   // register event
  uint16_t revents_;  // ready event
}* PEventHandlerMapEntry;

}  // namespace reactor
#endif