#pragma once

#include <string>
#include <sys/epoll.h>


class Client {

    private:

        int client_fd;
        std::string bodyRequest;
        epoll_event event;

    public:

        Client(const int& client_fd);
        epoll_event& getDataEvent();
        void addBody(const std::string& request);
};