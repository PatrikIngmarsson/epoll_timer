/*
 * Author: Patrik Ingmarsson
 */

#include<iostream>

#include<sys/timerfd.h>
#include<sys/epoll.h>
#include<errno.h>
#include<unistd.h>

#include<thread>
#include<chrono>

#include "epoll_fd.h"

using namespace aztrixiania::base::dispatcher;

int main(int argc, char * argv[]) {
    std::cout << "Starting Epoll Timer..." << std::endl;

    EpollFd epoller;

    epoller.Start();
    std::cout << "Started.." << std::endl;
    epoller.AddChoreWithDelay([](){
            std::cout << "A chore with one second delay" << std::endl;
        }, 1);
    std::cout << "Timer Set" << std::endl;
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(10s);
    epoller.Stop();

    std::cout << "Exiting..." << std::endl;
}
