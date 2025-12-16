#include "../../include/core/Client.hpp"
#include "../../include/utils/Utils.hpp"

Client::Client(): contentLength(0), bytesSend(0), handler(NULL)  {
    this->client_fd = -1;
    this->event.events = EPOLLIN;
    this->isHeadersReceived = false;
    this->isHeadersParsed = false;
    this->_responseStatus = -1;
    this->closeConnection = false;
};

Client::~Client() {
    
    if (this->handler != NULL) {
        delete this->handler;
    };
};

void Client::addBytesSend(ssize_t& bytes) {
    this->bytesSend += bytes;
}

ssize_t& Client::getBytesSend() {
    return this->bytesSend;
}

std::string& Client::getResponse() {
    return this->_response;
};

void Client::setCloseConnection(const bool& connection) {
    this->closeConnection = connection;
};
const bool& Client::getCloseConnection() {
    return this->closeConnection;
};

void Client::setResponse(const std::string& resp) {
    this->_response = resp;
    this->contentLength = resp.size();
};

Client::Client(const int& client_fd): contentLength(0), bytesSend(0), handler(NULL) {

    this->client_fd = client_fd;
    this->event.data.fd = client_fd;
    this->event.events = EPOLLIN;
    this->isHeadersReceived = false;
    this->isHeadersParsed = false;

    this->_responseStatus = -1;
    this->closeConnection = false;
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
        this->request = other.request;
        this->bytesSend = other.bytesSend;
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

size_t Client::getLenBody() {

    return this->contentLength;
}

void Client::setCodeResponseStatus(const std::string& status) {
    this->_responseStatus = status;
};
const std::string& Client::getCodeResponseStatus() {

    return this->_responseStatus;
};

HttpRequest& Client::getRequest() {
    return this->request;
};

void Client::cleanData() {

    this->contentLength = 0;
    this->isHeadersReceived = false;
    this->isHeadersParsed = false;
    this->_response.erase();
    this->closeConnection = false;
    this->_responseStatus = -1;
    this->bytesSend = 0;
    this->request.clearAllData();
}
