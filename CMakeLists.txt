cmake_minimum_required(VERSION 3.5.1)

project(epoll_timer)

option(EPOLL_TIMER_UT "Unit tests for epoll timer" OFF)
option(EPOLL_TIMER_TEST_EXECUTABLE "Executable to test functionality" OFF)
option(EPOLL_TIMER_INSTALL "Install epoll_time" OFF)

set (CMAKE_CXX_STANDARD 14)

find_package(Threads REQUIRED)

add_library(event_dispatcher
    src/event_dispatcher.cpp
    src/file_descriptor.cpp)
target_include_directories(event_dispatcher
    PUBLIC include
)
target_link_libraries(event_dispatcher Threads::Threads)

if(${EPOLL_TIMER_TEST_EXECUTABLE})
    add_executable(epoll_timer src/main.cpp)
    target_include_directories(epoll_timer PUBLIC include)
    target_link_libraries(epoll_timer
        event_dispatcher)
endif()

if(${EPOLL_TIMER_UT})
    # Google Test
    add_subdirectory(lib/googletest/googletest)
    enable_testing()
    include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

    # Unit tests
    add_executable(test_epoll_timer test/test_suite.cpp)
    target_link_libraries(test_epoll_timer
        event_dispatcher 
        gtest
        gtest_main)
    add_test(test_epoll_timer_ut test_epoll_timer)
endif()

if(${EPOLL_TIMER_INSTALL})
    install(TARGETS event_dispatcher
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        PUBLIC_HEADER DESTINATION include
        ARCHIVE DESTINATION lib
    )
endif()
