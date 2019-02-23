cmake_minimum_required(VERSION 3.5.1)

project(epoll_timer)

option(EPOLL_TIMER_UT "Unit tests for epoll timer" OFF)
option(EPOLL_TIMER_TEST_EXECUTABLE "Executable to test functionality" OFF)

set (CMAKE_CXX_STANDARD 14)

add_library(epoll_timer_lib
    src/epoll_fd.cpp
    src/file_descriptor.cpp)
target_include_directories(epoll_timer_lib PUBLIC include)
target_include_directories(epoll_timer_lib PUBLIC src)

if( ${EPOLL_TIMER_TEST_EXECUTABLE})
    find_package(Threads REQUIRED)
    add_executable(epoll_timer src/main.cpp)
    target_include_directories(epoll_timer PUBLIC include src)
    target_link_libraries(epoll_timer
         epoll_timer_lib
        Threads::Threads)
endif()

if (${EPOLL_TIMER_UT})
    # Google Test
    add_subdirectory(lib/googletest/googletest)
    enable_testing()
    include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

    # Unit tests
    add_executable(test_epoll_timer test/test_suite.cpp)
    target_link_libraries(test_epoll_timer
        epoll_timer_lib
        gtest
        gtest_main)
    add_test(test_epoll_timer_ut test_epoll_timer)

endif()
