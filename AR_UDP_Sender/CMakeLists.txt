cmake_minimum_required(VERSION 3.10)
project(AR_UDP_Sender)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(AR_UDP_Sender AR_UDP_Sender.cpp)

find_package(Threads REQUIRED)
target_link_libraries(AR_UDP_Sender PRIVATE Threads::Threads)