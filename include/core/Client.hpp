#pragma once

#include <string>
#include <sys/epoll.h>

#include "../http/HttpRequest.hpp"
#include "../interfaces/IMethodHandler.hpp"
#include "../bodyProcessor/ChunkedDecoder.hpp"

class Client {

    private:

        int client_fd;
        size_t contentLength;
        bool isHeadersReceived;
        bool isHeadersParsed;
        bool _isChunked;
        bool _bodyDelivered;
        epoll_event event;
        HttpRequest request;
        ChunkedDecoder _chunkedDecoder;
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
        const int& getClienteFd() const;
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
        void setChunked(bool value);
        bool isChunked() const;
        void initChunkedDecoder();
        void feedChunked(const char* data, size_t len);
        bool isChunkedFinished() const;
        const std::string& getChunkedBody() const;
        std::string extractBodyAfterHeaders();
        bool hasDeliveredBody() const;
        void markBodyDelivered();
        void addBytesSend(ssize_t& bytes);
        ssize_t& getBytesSend();
        void setCloseConnection(const bool& connection);
        const bool& getCloseConnection();
};