#pragma once

#include<functional>
#include<map>
#include<sys/epoll.h>
#include<sys/timerfd.h>
#include<thread>
#include<vector>

#include"file_descriptor.h"

namespace aztrixiania {
namespace base {
namespace dispatcher {

using Chore = std::function<void()>;

class EpollFd {
public:
    EpollFd();
    ~EpollFd();

    void Start();
    void Stop();
    bool GetRunCondition();

    void AddChore(Chore&& chore);
    void AddChoreWithDelay(Chore&& chore, int sec);
    //void AddChoreWithFd(Chore&& chore, int fd);

private:
    const FileDescriptor epoll_fd_;
    const FileDescriptor event_fd_;

    std::vector<Chore> chores_;
    std::map<int, Chore> delayed_chores_;

    std::thread worker_thread_;
    bool is_to_run_;
};

} // namespace dispatcher
} // namespace base 
} // namespace aztrixiania
