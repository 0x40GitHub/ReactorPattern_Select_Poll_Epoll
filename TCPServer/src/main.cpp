#include <thread>

#include "epoll_demultiplexer.h"
#include "poll_demultiplexer.h"
#include "select_demultiplexer.h"
#include "tcp_server.h"
int main() {
  reactor::SelectDemultiplexer* select_demultiplexer =
      new reactor::SelectDemultiplexer();
  reactor::PollDemultiplexer* poll_demultiplexer =
      new reactor::PollDemultiplexer();
  reactor::EPollDemultiplexer* epoll_demultiplexer =
      new reactor::EPollDemultiplexer();
  // reactor::InitiationDispatcher* dispatcher =
  //     new reactor::InitiationDispatcher(poll_demultiplexer);
  // reactor::InitiationDispatcher* dispatcher =
  //     new reactor::InitiationDispatcher(select_demultiplexr);
  reactor::InitiationDispatcher* dispatcher =
      new reactor::InitiationDispatcher(epoll_demultiplexer);
  std::atomic<bool> reactor_done(false);
  std::thread reactor_thread = std::thread([&dispatcher, &reactor_done]() {
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    while (!reactor_done) {
      dispatcher->handle_events(&timeout);
    }
  });
  // TODO:对于每个tcp通信,目前采用分离线程处理,需要确保在主线程在这些线程之前结束
  // tcp server
  tcp_server::TcpServer server("192.168.78.130", 9999, dispatcher);
  server.start();
  // server.stop();
  // reactor_done = true;

  { /* code */
  }

  // dispatcher->unblock();
  reactor_thread.join();
  return 0;
}