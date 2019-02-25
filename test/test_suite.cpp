#include <iostream>
#include "gtest/gtest.h"
#include <mutex>

#include "event_dispatcher.hpp"

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
    aztrixiania::base::dispatcher::EventDispatcher epoll_queue_;
};

TEST_F(TestSuite, TestOne) {
    std::mutex guard_;
    int i = 0;
    guard_.lock();
    epoll_queue_.Start();
    epoll_queue_.AddChore([&](){i++; guard_.unlock();});
    guard_.lock();

    EXPECT_EQ(1, i);
    epoll_queue_.Stop();
}

