#include "../../include/core/Client.hpp"
#include "../../include/utils/Utils.hpp"

Client::Client(): handler(NULL)  {
    this->contentLength = 0;
    this->client_fd = -1;
    this->event.events = EPOLLIN;
    this->isHeadersReceived = false;
    this->isHeadersParsed = false;
    this->_isChunked = false;
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


void Client::addBuffer(const std::string& data) {
    request.appendBuffer(data);

    // Ainda lendo headers
    if (!isHeadersReceived) {
        if (request.getBuffer().find("\r\n\r\n") != std::string::npos ||
            request.getBuffer().find("\n\n")     != std::string::npos) {

            isHeadersReceived = true;
            request.parser();
            isHeadersParsed = true;

            // 🔹 Detecta Transfer-Encoding: chunked
            std::string te = request.getHeader("Transfer-Encoding");
            if (!te.empty() && te.find("chunked") != std::string::npos) {
                _isChunked = true;
                contentLength = 0; // chunked NÃO usa Content-Length
            } else {
                _isChunked = false;
                contentLength = request.getContentLength();
            }

            // 🔹 Remove qualquer body que veio colado nos headers
            request.eraseBody();
        }
    }
}


void Client::addBody(const std::string& body) {
    // Não guarda chunked no HttpRequest
    if (!_isChunked) {
        this->request.appendBody(body);
    }
}

bool Client::isAllHeaders() {
    return this->isHeadersReceived;
}

int Client::getLenBody() {
    if (_isChunked)
        return -1; // indica tamanho desconhecido
    return this->contentLength;
}

HttpRequest& Client::getRequest() {
    return this->request;
};

void Client::cleanData() {

    // 🔹 LIMPA ARQUIVO TEMPORÁRIO DO BODY (chunked / multipart / CGI)
    if (!request.bodyTempPath.empty()) {

        // garante que não existe handler usando o arquivo
        if (handler != NULL) {
            delete handler;
            handler = NULL;
        }

        std::remove(request.bodyTempPath.c_str());
        request.bodyTempPath.clear();
    }

    contentLength = 0;
    isHeadersReceived = false;
    isHeadersParsed = false;
    _isChunked = false;

    request.clearAllData();
}

bool Client::isChunked() const {
    return _isChunked;
}