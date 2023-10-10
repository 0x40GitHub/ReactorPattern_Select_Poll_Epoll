#ifndef TCP_SERVER_H
#define TCP_SERVER_H
#include <atomic>
#include <cstdint>
#include <memory>
#include <string>

#include "event_handler.h"
#include "initiation_dispatcher.h"
namespace tcp_server {
class TcpServer : public reactor::EventHandler {
 public:
  TcpServer(std::string ip, uint16_t port,
            reactor::InitiationDispatcher* initiation_dispatcher);
  ~TcpServer();

  void do_something_else();

  void start();

  void stop();

  void handle_accept(reactor::Handle handle);

  void handle_read(reactor::Handle handle);

 private:
  std::string ip_;
  uint16_t port_;
  std::atomic<bool> tcpserver_done_;
  std::atomic<int> conn_amount_;
  reactor::Handle listening_socket_;
  std::unique_ptr<reactor::InitiationDispatcher> initiation_dispatcher_;
};
}  // namespace tcp_server

#endif