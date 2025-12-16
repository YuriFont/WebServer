#pragma once

#include <string>
#include <sys/epoll.h>

#include "../http/HttpRequest.hpp"
#include "../interfaces/IMethodHandler.hpp"

class Client {

    private:

        int client_fd;
        size_t contentLength;
        bool isHeadersReceived;
        bool isHeadersParsed;
        epoll_event event;
        HttpRequest request;
        std::string _response;
        ssize_t bytesSend;
        std::string _responseStatus;
        bool closeConnection;
        
        
    public:
        
        IMethodHandler* handler;
        Client();
        Client(const int& client_fd);
        Client(const Client& client);
        Client& operator=(const Client& other);
        ~Client();
        epoll_event& getDataEvent();
        const int& getClienteFd();
        void setCodeResponseStatus(const std::string& status);
        const std::string& getCodeResponseStatus();
        void addBuffer(const std::string& request);
        void addBody(const std::string& body);
        HttpRequest& getRequest();
        std::string& getResponse();
        void setResponse(const std::string& resp);
        bool isAllHeaders();
        size_t getLenBody();
        void eraseBody();
        void cleanData();
        void addBytesSend(ssize_t& bytes);
        ssize_t& getBytesSend();
        void setCloseConnection(const bool& connection);
        const bool& getCloseConnection();
};