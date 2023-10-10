#ifndef INITIATION_DISPATCHER_H
#define Initiation Dispatcher

#include <atomic>
#include <map>
#include <memory>
#include <mutex>

#include "event_demultiplexer.h"
#include "event_handler.h"
#include "handle.h"
#include "pipe_event_handler.h"
namespace reactor {

// event type
#define READ_EVENT 01
#define WRITE_EVENT 02
#define EXCEPTION_EVENT 04
#define ACCEPT_EVENT 010
#define EVENT_MASK READ_EVENT | WRITE_EVENT | EXCEPTION_EVENT | ACCEPT_EVENT

class InitiationDispatcher {
 public:
  InitiationDispatcher(EventDemultiplexer* event_demultiplexer);
  ~InitiationDispatcher();
  void register_event_handler(Handle handle, EventHandler* event_handler,
                              uint16_t event_type, bool one_shot = false);
  void unregister_event_handler(Handle handle, uint16_t event_type);
  void handle_events(const struct timeval* timeout);
  void unblock();

 private:
  std::mutex event_handlers_mutex;
  // registered event
  std::map<Handle, EventHandlerMapEntry*> event_handlers_;
  // select,poll, or epoll...
  EventDemultiplexer* event_demultiplexer_;

  void dispatch_event_handlers();

  // pipe for wakeup bolcking call
  std::array<int, 2> wakeup_pipe_;

  // pipe event handler
  std::unique_ptr<PipeEventHandler> pipe_event_handler_;

  // is dispatching event handler
  std::atomic_bool dispatching_;
};

}  // namespace reactor
#endif