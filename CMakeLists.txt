cmake_minimum_required(VERSION 3.11.0)

project(epoll_timer)

find_package(Threads)

set (CMAKE_CXX_STANDARD 14)

set( SOURCES
    src/epoll_fd.cpp
    src/file_descriptor.cpp
)

add_library(epoll_timer_lib ${SOURCES})
target_include_directories(epoll_timer_lib PUBLIC include)
target_include_directories(epoll_timer_lib PUBLIC src)
add_executable(epoll_timer src/main.cpp)

target_include_directories(epoll_timer PUBLIC include src)
target_link_libraries(epoll_timer epoll_timer_lib ${CMAKE_THREAD_LIBS_INIT})



# This adds another subdirectory, which has 'project(gtest)'.
add_subdirectory(lib/googletest/googletest)

enable_testing()

# Include the gtest library. gtest_SOURCE_DIR is available due to
# 'project(gtest)' above.
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

##############
# Unit Tests
##############
add_executable(runUnitTests test/test_suite.cpp)

# Standard linking to gtest stuff.
target_link_libraries(runUnitTests gtest gtest_main)

# Extra linking for the project.
target_link_libraries(runUnitTests epoll_timer_lib)

# This is so you can do 'make test' to see all your tests run, instead of
# manually running the executable runUnitTests to see those specific tests.
# add_test(NAME TestOne COMMAND runUnitTests)

# You can also omit NAME and COMMAND. The second argument could be some other
# test executable.
add_test(TestTwo runUnitTests)
