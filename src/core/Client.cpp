#include "../../include/core/Client.hpp"
#include "../../include/utils/Utils.hpp"

Client::Client() {
    this->contentLength = 0;
    this->client_fd = -1;
    this->event.events = EPOLLIN;
    this->isHeadersReceived = false;
    this->isHeadersParsed = false;
};

Client::~Client() {
    close(this->client_fd);
};

Client::Client(const int& client_fd) {

    this->client_fd = client_fd;
    this->event.data.fd = client_fd;
    this->event.events = EPOLLIN;
    this->isHeadersReceived = false;
    this->isHeadersParsed = false;
};

Client::Client(const Client& client) {

    *this = client;
    
};

Client& Client::operator=(const Client& other) {

    if (this != &other) {
        this->client_fd = other.client_fd;
        this->contentLength = other.contentLength;
        this->isHeadersReceived = other.isHeadersReceived;
        this->isHeadersParsed = other.isHeadersParsed;
        this->event = other.event;
        this->request = other.request;
    }
    return *this;
};

epoll_event& Client::getDataEvent() {
    return this->event;
};

void Client::addBuffer(const std::string& request) {

    this->request.appendBuffer(request);
    if (!this->isHeadersReceived) {
        if (this->request.getBuffer().find("\r\n\r\n") != std::string::npos || this->request.getBuffer().find("\n\n") != std::string::npos) {

            this->isHeadersReceived = true;
            this->request.parser();
            this->contentLength = this->request.getContentLength();
            this->isHeadersParsed = true;
        }
    }
};

void Client::addBody(const std::string& body) {

    this->request.appendBody(body);
};



bool Client::isAllHeaders() {
    return this->isHeadersReceived;
}

int Client::getLenBody() {

    return this->contentLength;
}

HttpRequest& Client::getRequest() {
    return this->request;
};

void Client::cleanData() {

    this->contentLength = 0;
    this->isHeadersReceived = false;
    this->isHeadersParsed = false;
    this->request.clearAllData();
}