#include "event_dispatcher.hpp"

#include <cerrno>
#include <cstring>
#include<sys/epoll.h>
#include<sys/eventfd.h>
#include<sys/timerfd.h>
#include<unistd.h>
#include<iostream>

namespace aztrixiania {
namespace base {
namespace dispatcher {

constexpr int EPOLL_WAIT_ETERNAL = -1;
constexpr unsigned int MAXIMUM_NUMBER_OF_EVENTS_HANDLED = 8;

EventDispatcher::EventDispatcher() :
    epoll_fd_(epoll_create1(EPOLL_CLOEXEC)),
    event_fd_(eventfd(0, EFD_CLOEXEC | EFD_SEMAPHORE)),
    is_to_run_(false) {
        epoll_event event;
        event.events = EPOLLIN;
        event.data.fd = -1;
        if(-1 == epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, event_fd_, &event)) {
            throw std::runtime_error("Errrrrorrrrr");
        }
}

EventDispatcher::~EventDispatcher() {
}

bool EventDispatcher::GetRunCondition() { return is_to_run_; }

void EventDispatcher::Start() {

    worker_thread_ = std::thread([this](){
        is_to_run_ = true;
        epoll_event events[MAXIMUM_NUMBER_OF_EVENTS_HANDLED];
        while(GetRunCondition()) {
            int r = epoll_wait(epoll_fd_, events, MAXIMUM_NUMBER_OF_EVENTS_HANDLED, EPOLL_WAIT_ETERNAL);
            if(-1 == r) {
                std::cerr << "Failed while waiting: " << strerror(errno) << std::endl;
                continue;
            }

            for(int i = 0; i < r; i++) {
                const epoll_event& event = events[i];
                if(-1 == event.data.fd) {
                    unsigned int chore_count = 0;
                    for(auto c : chores_) {
                        c();
                        chore_count++;
                    }
                    // Guard this maybeeee?
                    chores_.clear();
                    uint64_t dummy;
                    for(unsigned int i = 0; i < chore_count; i++) {
                        if(-1 == read(event_fd_, &dummy, sizeof(dummy))) {
                            std::cerr << "Failed to read event: " << strerror(errno) << std::endl;
                        }
                    }
                } else {
                    auto iter = delayed_chores_.find(event.data.fd);
                    if(iter != delayed_chores_.end()) {
                        size_t s = 0;
                        if(-1 != read(iter->first, &s, sizeof(s))) {
                            iter->second();
                            close(iter->first);
                        }
                    }
                }
            }
        }});
}

void EventDispatcher::Stop() {
    is_to_run_ = false;
    AddChore([](){});
    worker_thread_.join();
    std::cout << "Closed.." << std::endl;
}
void EventDispatcher::AddChore(Chore&& chore) {
    chores_.push_back(chore);
    const uint64_t dummy = 1;
    if(-1 == write(event_fd_, &dummy, sizeof(dummy))) {
        std::cerr << "Unable to send event for chore: " << strerror(errno) << std::endl;
    }
}
void EventDispatcher::AddChoreWithDelay(Chore&& chore, int ms) {
    int timed_fd(timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC));
    if(0 > timed_fd) {
        std::cerr << "Failed to create timed fd: " << strerror(errno) << std::endl;
        return;
    }

    epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = timed_fd;
    if(-1 == epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, timed_fd, &event)) {
        std::cerr << "Couldn't add timed_fd to epoll fd: " << strerror(errno) << std::endl;
        close(timed_fd);
        return;
    }

    struct itimerspec timer;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_nsec = 0;
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_nsec = ms * 1000;
    if(-1 == timerfd_settime(timed_fd, 0, &timer, nullptr)) {
        std::cerr << "Couldn't set the timer: " << strerror(errno) << std::endl;
        close(timed_fd);
        return;
    }
    delayed_chores_[timed_fd] = chore;
}

void EventDispatcher::AddChoreWithFd(Chore&& chore, int fd, int events) {

    std::cout << __FUNCTION__ << std::endl;

    RemoveChore(fd);

    epoll_event event;
    event.events = events;
    event.data.fd = fd;

    if(-1 == epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &event)) {
        std::cerr << "Couldn't add fd to monitor: " << strerror(errno) << std::endl;
        return;
    }
    delayed_chores_[fd] = chore;
}

void EventDispatcher::RemoveChore(int fd) {

    auto chore = delayed_chores_.find(fd);
    if(chore != delayed_chores_.end()) {
        if(-1 == epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr)) {
            std::cerr << "Couldn't remove fd to monitor" << std::endl;
            return;
        }
        delayed_chores_.erase(fd);
    }
}


} // namespace dispatcher
} // namespace base
} // namespace aztrixiania
