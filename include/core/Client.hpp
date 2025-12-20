#pragma once

#include <string>
#include <sys/epoll.h>

#include "../http/HttpRequest.hpp"
#include "../interfaces/IMethodHandler.hpp"
#include "../bodyProcessor/ChunkedDecoder.hpp"

class Client {

    private:

        int client_fd;
        int contentLength;
        bool isHeadersReceived;
        bool isHeadersParsed;
        bool _isChunked;
        bool _bodyDelivered;
        epoll_event event;
        HttpRequest request;
        ChunkedDecoder _chunkedDecoder;
        
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
        void setChunked(bool value);
        bool isChunked() const;
        void initChunkedDecoder();
        void feedChunked(const char* data, size_t len);
        bool isChunkedFinished() const;
        const std::string& getChunkedBody() const;
        std::string extractBodyAfterHeaders();
        bool hasDeliveredBody() const;
        void markBodyDelivered();
};