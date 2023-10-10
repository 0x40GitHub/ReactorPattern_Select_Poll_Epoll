#include "tcp_server.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <thread>
namespace tcp_server {

TcpServer::TcpServer(std::string ip, uint16_t port,
                     reactor::InitiationDispatcher* initiation_dispatcher)
    : ip_(ip), port_(port), initiation_dispatcher_(initiation_dispatcher) {
  tcpserver_done_ = false;
  conn_amount_ = 0;
  listening_socket_ = -1;
}

TcpServer::~TcpServer() {}

void TcpServer::do_something_else() {
  std::thread thread = std::thread([this]() {
    while (!tcpserver_done_) {
      std::cout << "处理其他事务..." << std::endl;
      std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    }
  });
  thread.detach();
}

void TcpServer::start() {
  // socket for listening
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0) {
    perror("socket");
    exit(1);
  }

  // reuse addr
  int optval = 1;
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
    perror("setsockopt");
    exit(1);
  }

  struct sockaddr_in saddr;
  memset(&saddr, 0, sizeof(saddr));
  saddr.sin_family = AF_INET;
  saddr.sin_port = htons(port_);
  saddr.sin_addr.s_addr = inet_addr(ip_.c_str());

  // bind
  if (bind(fd, (struct sockaddr*)&saddr, sizeof(saddr)) == -1) {
    perror("bind");
    exit(1);
  }
  // listen
  if (listen(fd, 3) == -1) {
    perror("listen");
    exit(1);
  }
  // set listening socket
  listening_socket_.SetFileDescriptor(fd);
  // register socket
  initiation_dispatcher_->register_event_handler(listening_socket_, this,
                                                 ACCEPT_EVENT, false);
  puts("waiting for client...\n");
  // do_something_else();
}

void TcpServer::stop() { tcpserver_done_ = true; }

// handle accept event
void TcpServer::handle_accept(reactor::Handle handle) {
  std::thread thread = std::thread([this, handle]() {
    int conn_socket;
    sockaddr_in client_addr;
    socklen_t addr_size = sizeof(client_addr);
    if ((conn_socket = accept(handle.GetFileDescriptor(),
                              (struct sockaddr*)&client_addr, &addr_size)) ==
        -1) {
      perror("accept");
      exit(1);
    }
    if (conn_amount_ < 3) {  // max connection
      conn_amount_++;
      std::cout << "New connection accepted Client [" << conn_amount_ << "]"
                << inet_ntoa(client_addr.sin_addr) << ":"
                << ntohs(client_addr.sin_port) << std::endl;
      // register for reading
      initiation_dispatcher_->register_event_handler(
          reactor::Handle(conn_socket), this, READ_EVENT, true);
      send(conn_socket, "Hi,Client.\n", 12, 0);
    } else {
      send(conn_socket, "bye.\n", 4, 0);
      close(conn_socket);
    }
  });
  thread.detach();
}
// handle read event
void TcpServer::handle_read(reactor::Handle handle) {
  std::thread thread = std::thread([this, handle]() {
    char recv_buf[1024];
    char send_buf[1024];
    int ret;
    while (true) {
      ret = recv(handle.GetFileDescriptor(), recv_buf, sizeof(recv_buf), 0);
      if (ret > 0) {
        std::cout << "receive " << ret << " bytes data:" << recv_buf
                  << std::endl;
        snprintf(send_buf, 1056, "Server has received your data(%s).",
                 recv_buf);
        send(handle.GetFileDescriptor(), send_buf, strlen(send_buf), 0);
      } else if (ret == 0) {
        std::cout << "Client closed.\n";
        conn_amount_--;
        break;
      } else {
        perror("recv");
        // Check if the file descriptor is valid
        int flags = fcntl(handle.GetFileDescriptor(), F_GETFL);
        if (flags == -1) {
          perror("fcntl");
          close(handle.GetFileDescriptor());
        }
        this->initiation_dispatcher_->unregister_event_handler(handle,
                                                               READ_EVENT);
        exit(1);
      }
      memset(recv_buf, 0, sizeof(recv_buf));
      memset(recv_buf, 0, sizeof(send_buf));
    }
    // Check if the file descriptor is valid
    int flags = fcntl(handle.GetFileDescriptor(), F_GETFL);
    if (flags == -1) {
      perror("fcntl");
      close(handle.GetFileDescriptor());
    }
    this->initiation_dispatcher_->unregister_event_handler(handle, READ_EVENT);
  });
  thread.detach();
}

}  // namespace tcp_server