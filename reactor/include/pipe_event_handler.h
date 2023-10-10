#ifndef PIPE_EVENT_HANDLER_H
#define PIPE_EVENT_HANDLER_H

#include "event_handler.h"
namespace reactor {

class PipeEventHandler : public EventHandler {
 public:
  virtual ~PipeEventHandler() = default;
  /**
   * 读事件处理程序
   * handle: 文件描述符
   */
  virtual void handle_read(Handle handle);

  /**
   * 写事件处理程序
   * handle: 文件描述符
   */
  virtual void handle_write(Handle handle);

  /**
   * 异常事件处理程序
   * handle: 文件描述符
   */
  virtual void handle_exception(Handle handle){};
};

}  // namespace reactor
#endif