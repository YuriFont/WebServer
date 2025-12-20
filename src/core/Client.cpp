#include "../../include/core/Client.hpp"
#include "../../include/utils/Utils.hpp"

Client::Client(): handler(NULL)  {
    this->contentLength = 0;
    this->client_fd = -1;
    this->event.events = EPOLLIN;
    this->isHeadersReceived = false;
    this->isHeadersParsed = false;
    this->_isChunked = false;
    this->_bodyDelivered = false;
};

Client::~Client() {
    if (this->handler != NULL) {
        delete this->handler;
    };
};

Client::Client(const int& client_fd): handler(NULL) {
    this->client_fd = client_fd;
    this->event.data.fd = client_fd;
    this->event.events = EPOLLIN;
    this->isHeadersReceived = false;
    this->isHeadersParsed = false;
    this->_isChunked = false;
    this->_bodyDelivered = false;
};

Client::Client(const Client& client): handler(NULL) {
    *this = client;
};

Client& Client::operator=(const Client& other) {
    if (this->handler != NULL) {
        delete this->handler;
        this->handler = NULL;
    }

    if (this != &other) {
        this->client_fd = other.client_fd;
        this->contentLength = other.contentLength;
        this->isHeadersReceived = other.isHeadersReceived;
        this->isHeadersParsed = other.isHeadersParsed;
        this->event = other.event;
        this->_isChunked = other._isChunked;
        this->_bodyDelivered = other._bodyDelivered;
        this->request = other.request;
        if (other.handler != NULL) {
            this->handler = other.handler->clone();
        }
    }
    return *this;
};

void Client::eraseBody() {
    this->request.eraseBody();
};

epoll_event& Client::getDataEvent() {
    return this->event;
};

const int& Client::getClienteFd() {
    return this->client_fd;
};

void Client::addBuffer(const std::string& request) {
    this->request.appendBuffer(request);
    if (!this->isHeadersReceived) {
        if (this->request.getBuffer().find("\r\n\r\n") != std::string::npos || this->request.getBuffer().find("\n\n") != std::string::npos) {

            this->isHeadersReceived = true;
            this->request.parser();
            this->contentLength = this->request.getContentLength();
            // std::cout << this->request.getBuffer() << std::endl;
            // std::cout << this->request.getMethod() << " " << this->request.getPath() << " " << this->request.getHttpVersion() << std::endl;
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
    this->_isChunked = false;
    this->_bodyDelivered = false;
    _chunkedDecoder.reset();
    this->request.clearAllData();
}

void Client::setChunked(bool value){
    _isChunked = value;
}

bool Client::isChunked() const {
    return _isChunked;
}

void Client::initChunkedDecoder() {
    _chunkedDecoder.reset();
}

void Client::feedChunked(const char* data, size_t len) {
    _chunkedDecoder.feed(data, len);
}

bool Client::isChunkedFinished() const {
    return _chunkedDecoder.isFinished();
}

const std::string& Client::getChunkedBody() const {
    return _chunkedDecoder.getBody();
}

std::string Client::extractBodyAfterHeaders() {
        return request.extractBodyAfterHeaders();
    }

bool Client::hasDeliveredBody() const {
    return _bodyDelivered;
}

void Client::markBodyDelivered() {
    _bodyDelivered = true;
}