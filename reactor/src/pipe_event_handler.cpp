#include "pipe_event_handler.h"

#include <unistd.h>

#include <stdexcept>
namespace reactor {
void PipeEventHandler::handle_read(Handle handle) {
  int ret;
  char dummy;

  ret = ::read(handle.GetFileDescriptor(), &dummy, sizeof(dummy));
  if (ret != sizeof(dummy)) {
    std::runtime_error("read");
  }
}

void PipeEventHandler::handle_write(Handle handle) {
  int ret;
  char dummy;

  ret = ::write(handle.GetFileDescriptor(), &dummy, sizeof(dummy));
  if (ret != sizeof(dummy)) {
    std::runtime_error("write");
  }
}

}  // namespace reactor