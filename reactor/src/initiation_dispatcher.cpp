#include "initiation_dispatcher.h"

#include <unistd.h>
namespace reactor {

InitiationDispatcher::InitiationDispatcher(
    EventDemultiplexer* event_demultiplexer)
    : event_demultiplexer_(event_demultiplexer),
      pipe_event_handler_(new PipeEventHandler()) {
  int ret = pipe(wakeup_pipe_.data());
  if (ret != 0) {
    std::runtime_error("pipe");
  }
  // register for waking up
  register_event_handler(wakeup_pipe_[0], pipe_event_handler_.get(), READ_EVENT,
                         false);
}

InitiationDispatcher::~InitiationDispatcher() {
  // pipe_event_handler_->handle_write(wakeup_pipe_[1]);
  /*close pipe*/
  delete event_demultiplexer_;
  for (auto it = event_handlers_.begin(); it != event_handlers_.end(); it++) {
    delete it->second;
  }
  ::close(wakeup_pipe_[0]);
  ::close(wakeup_pipe_[1]);
}

// 注册事件
void InitiationDispatcher::register_event_handler(Handle handle,
                                                  EventHandler* event_handler,
                                                  uint16_t event_type,
                                                  bool one_shot) {
  std::unique_lock<std::mutex> lock(event_handlers_mutex);
  if (EVENT_MASK & event_type) {
    if (event_handlers_.count(handle)) {
      event_handlers_[handle]->events_ |= event_type;
    } else {
      EventHandlerMapEntry* entry = new EventHandlerMapEntry;
      entry->event_handler_ = event_handler;
      entry->events_ = event_type;
      entry->one_shot_ = one_shot;
      entry->revents_ = 0U;
      event_handlers_.insert(
          std::pair<Handle, PEventHandlerMapEntry>(handle, entry));
    }
    lock.unlock();
  }

  // wakeup handle_events
  if (!dispatching_) {
    pipe_event_handler_->handle_write(wakeup_pipe_[1]);
  }
}

void InitiationDispatcher::unregister_event_handler(Handle handle,
                                                    uint16_t event_type) {
  std::unique_lock<std::mutex> lock(event_handlers_mutex);
  if (event_handlers_.count(handle)) {
    event_handlers_[handle]->events_ &= ~event_type;
    event_handlers_[handle]->revents_ &= ~event_type;

    if (event_handlers_[handle]->events_ & (~EVENT_MASK)) {
      event_handlers_.erase(handle);
    }
  }
  lock.unlock();
  // wakeup handle_events
  if (!dispatching_) {
    pipe_event_handler_->handle_write(wakeup_pipe_[1]);
  }
}

void InitiationDispatcher::handle_events(const struct timeval* timeout) {
  struct timeval tv;
  struct timeval* tv_ptr = nullptr;
  if (timeout != nullptr) {
    tv = *timeout;
    tv_ptr = &tv;
  }
  int count = event_demultiplexer_->awaiting(event_handlers_, tv_ptr);
  if (count > 0) {
    dispatch_event_handlers();
  }
}
void InitiationDispatcher::unblock() {
  pipe_event_handler_->handle_write(wakeup_pipe_[1]);
}
void InitiationDispatcher::dispatch_event_handlers() {
  dispatching_ = true;
  std::lock_guard<std::mutex> lock(event_handlers_mutex);
  for (auto it = event_handlers_.begin(); it != event_handlers_.end();) {
    Handle handle = it->first;
    auto entry = it->second;

    if (READ_EVENT & entry->revents_) {  // 读事件
      if (entry->one_shot_) {
        entry->events_ &= ~READ_EVENT;
      }

      entry->event_handler_->handle_read(handle);
    }

    if (WRITE_EVENT & entry->revents_) {  // 写事件
      if (entry->one_shot_) {
        entry->events_ &= ~WRITE_EVENT;
      }

      entry->event_handler_->handle_write(handle);
    }

    if (EXCEPTION_EVENT & entry->revents_) {  // 异常事件
      if (entry->one_shot_) {
        entry->events_ &= ~EXCEPTION_EVENT;
      }

      entry->event_handler_->handle_exception(handle);
    }

    if (ACCEPT_EVENT & entry->revents_) {  // accept
      if (entry->one_shot_) {
        entry->events_ &= ~ACCEPT_EVENT;
      }

      entry->event_handler_->handle_accept(handle);
    }

    if (!entry->events_) {
      if (entry->one_shot_) {
        it = event_handlers_.erase(it);

      } else {
        it++;
      }
    } else {
      it++;
    }
  }
  dispatching_ = false;
}

}  // namespace reactor