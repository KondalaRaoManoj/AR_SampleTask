cmake_minimum_required(VERSION 3.10)
project(AR_UDP_Receiver)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(AR_UDP_Receiver AR_UDP_Receiver.cpp)


find_package(Threads REQUIRED)
target_link_libraries(AR_UDP_Receiver PRIVATE Threads::Threads)