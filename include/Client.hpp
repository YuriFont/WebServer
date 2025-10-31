#pragma once

#include <string>
#include <sys/epoll.h>

#include "./HttpRequest.hpp"

class Client {

    private:

        int client_fd;
        int contentLength;
        bool isHeadersReceived;
        epoll_event event;
        HttpRequest request;

    public:

        Client(const int& client_fd);
        ~Client();
        epoll_event& getDataEvent();
        void addBody(const std::string& request);
        bool isAllHeaders();
        void cleanData();
};