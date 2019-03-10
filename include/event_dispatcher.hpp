#pragma once

#include<functional>
#include<map>
#include<sys/epoll.h>
#include<sys/timerfd.h>
#include<thread>
#include<vector>
#include<mutex>

#include"file_descriptor.hpp"

namespace aztrixiania {
namespace base {
namespace dispatcher {

using Chore = std::function<void()>;

class EventDispatcher{
public:
    EventDispatcher();
    ~EventDispatcher();

    void Start();
    void Stop();
    bool GetRunCondition();

    void AddChore(Chore&& chore);
    void AddChoreWithDelay(Chore&& chore, int ms);
    void AddChoreWithFd(Chore&& chore, int fd, int events);

    void RemoveChore(int fd);

private:
    const FileDescriptor epoll_fd_;
    const FileDescriptor event_fd_;

    std::mutex chores_guard_;
    std::vector<Chore> chores_;
    std::map<int, Chore> delayed_chores_;

    std::thread worker_thread_;
    bool is_to_run_;
};

} // namespace dispatcher
} // namespace base 
} // namespace aztrixiania
