#pragma once

#include <string>
#include <sys/epoll.h>

#include "../http/HttpRequest.hpp"
#include "../interfaces/IMethodHandler.hpp"

class Client {

    private:

        int client_fd;
        int contentLength;
        bool isHeadersReceived;
        bool isHeadersParsed;
        epoll_event event;
        HttpRequest request;
        
        
    public:
        
        IMethodHandler* handler;
        Client();
        Client(const int& client_fd);
        Client(const Client& client);
        Client& operator=(const Client& other);
        ~Client();
        epoll_event& getDataEvent();
        const int& getClienteFd();
        void addBuffer(const std::string& request);
        void addBody(const std::string& body);
        HttpRequest& getRequest();
        bool isAllHeaders();
        int getLenBody();
        void eraseBody();
        void cleanData();
};