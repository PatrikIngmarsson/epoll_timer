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
constexpr unsigned int MAXIMUM_NUMBER_OF_EVENTS_HANDLED = 20;

EventDispatcher::EventDispatcher() :
    epoll_fd_(epoll_create1(EPOLL_CLOEXEC)),
    event_fd_(eventfd(0, EFD_CLOEXEC | EFD_SEMAPHORE)),
    is_to_run_(false) {
        epoll_event event;
        event.events = EPOLLIN;
        event.data.fd = -1;
        if(-1 == epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, event_fd_, &event)) {
            throw std::runtime_error("Unable to initialize Event Dispatcher");
        }
}

EventDispatcher::~EventDispatcher() {
}

bool EventDispatcher::GetRunCondition() { return is_to_run_; }

void EventDispatcher::Start() {

    worker_thread_ = std::thread([this](){
        is_to_run_ = true;
        epoll_event expired_events[MAXIMUM_NUMBER_OF_EVENTS_HANDLED];
        while(GetRunCondition()) {
            int number_of_expired_events = epoll_wait(epoll_fd_, expired_events,
                    MAXIMUM_NUMBER_OF_EVENTS_HANDLED, EPOLL_WAIT_ETERNAL);
            if(-1 == number_of_expired_events) {
                std::cerr << "Failed while waiting: " << strerror(errno) << std::endl;
                continue;
            }

            for(int i = 0; i < number_of_expired_events; i++) {
                const epoll_event& event = expired_events[i];
                if(-1 == event.data.fd) {
                    uint64_t dummy;
                    if(-1 == read(event_fd_, &dummy, sizeof(dummy))) {
                        std::string error_string("Failed to read event: ");
                        error_string += strerror(errno);
                        throw std::runtime_error(error_string);
                    } else {
                        Chore chore;
                        {
                            std::lock_guard<std::mutex> lock(chores_guard_);
                            chore = chores_.front();
                            chores_.erase(chores_.begin());
                        }
                        chore();
                    }
                } else {
                    Chore chore;
                    {
                        std::lock_guard<std::mutex> lock(chores_guard_);
                        chore = delayed_chores_.at(event.data.fd);
                    }
                    uint64_t dummy;
                    if(-1 != read(epoll_fd_, &dummy, sizeof(dummy))) {
                        if(-1 != read(event.data.fd, &dummy, sizeof(dummy))) {
                            close(event.data.fd);
                        }
                    }
                    chore();
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
    std::lock_guard<std::mutex> lock(chores_guard_);
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
    timer.it_value.tv_sec = ms/1000;
    timer.it_value.tv_nsec = (ms%1000) * 1000;
    if(-1 == timerfd_settime(timed_fd, 0, &timer, nullptr)) {
        std::cerr << "Couldn't set the timer: " << strerror(errno) << std::endl;
        close(timed_fd);
        return;
    }
    {
        std::lock_guard<std::mutex> lock(chores_guard_);
        delayed_chores_[timed_fd] = chore;
    }
}

void EventDispatcher::AddChoreWithFd(Chore&& chore, int fd, int events) {

    std::cout << __FUNCTION__ << std::endl;

    {
        std::lock_guard<std::mutex> lock(chores_guard_);
        RemoveChore(fd);
    }

    epoll_event event;
    event.events = events;
    event.data.fd = fd;

    if(-1 == epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &event)) {
        std::cerr << "Couldn't add fd to monitor: " << strerror(errno) << std::endl;
        return;
    }
    {
        std::lock_guard<std::mutex> lock(chores_guard_);
        delayed_chores_[fd] = chore;
    }
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
