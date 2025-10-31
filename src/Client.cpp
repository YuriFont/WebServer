#include "../include/Client.hpp"
#include "../include/Utils.hpp"



Client::~Client() {

};

Client::Client(const int& client_fd) {

    this->client_fd = client_fd;
    this->event.data.fd = client_fd;
    this->event.events = EPOLLIN;
    this->isHeadersReceived = false;
    this->isHeadersParsed = false;
};

epoll_event& Client::getDataEvent() {
    return this->event;
};

void Client::addBody(const std::string& request) {

    this->request.appendBuffer(request);
    if (!this->isHeadersReceived) {
        if (this->request.getBody().find("\r\n\r\n") != std::string::npos) {
            this->isHeadersReceived = true;
            this->request.parser();
            this->contentLength = this->request.getContentLength();
            this->isHeadersParsed = true;
        }
    }
};

bool Client::isAllHeaders() {
    return this->isHeadersReceived;
}

int Client::getLenBody() {

    return this->contentLength;
}

void Client::cleanData() {

    this->contentLength = 0;
    this->isHeadersReceived = false;
    this->isHeadersParsed = false;
    this->request.clearAllData();
}