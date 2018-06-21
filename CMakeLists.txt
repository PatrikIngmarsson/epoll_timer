cmake_minimum_required(VERSION 3.11.0)

project(epoll_timer)

find_package(Threads)

set (CMAKE_CXX_STANDARD 14)

set( SOURCES
    src/main.cpp
    src/epoll_fd.cpp
    src/file_descriptor.cpp
)

add_executable(epoll_timer ${SOURCES})

target_include_directories(epoll_timer PRIVATE include src)
target_link_libraries(epoll_timer ${CMAKE_THREAD_LIBS_INIT})
