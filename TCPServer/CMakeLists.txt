cmake_minimum_required(VERSION 3.10)

project(tcpserver)

include_directories("../reactor/include" "./include")
link_directories("../reactor")
add_executable(tcpserver "./src/main.cpp" "./src/tcp_server.cpp")
target_link_libraries(tcpserver reactor)