#include <iostream>
#include "gtest/gtest.h"
#include <mutex>

#include "epoll_fd.hpp"

class TestSuite: public ::testing::Test {
protected:

    TestSuite() {
    }

    virtual ~TestSuite() override {
    }

    virtual void SetUp() override {
    }

    virtual void TearDown() override {
    }
    aztrixiania::base::dispatcher::EpollFd epoll_queue_;
};

TEST_F(TestSuite, TestOne) {

    std::mutex guard_;
    int i = 0;
    std::cerr << "dsfdsf" << std::endl;
    guard_.lock();
    epoll_queue_.Start();
    epoll_queue_.AddChore([&](){std::cerr << "hej " << i++ << std::endl; guard_.unlock();});
    
    guard_.lock();

    EXPECT_EQ(1, 1);
    epoll_queue_.Stop();
}

