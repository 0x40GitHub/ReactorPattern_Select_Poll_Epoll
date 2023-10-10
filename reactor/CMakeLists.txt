cmake_minimum_required(VERSION 3.10)

project(reactor.so)
include_directories("./include")

add_library(reactor SHARED "./src/initiation_dispatcher.cpp" "./src/pipe_event_handler.cpp" "./src/select_demultiplexer.cpp" "./src/poll_demultiplexer.cpp" "./src/epoll_demultiplexer.cpp")