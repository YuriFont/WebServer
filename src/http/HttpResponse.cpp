#include "../../include/http/HttpResponse.hpp"
#include "../../include/utils/Utils.hpp"

HttpResponse::HttpResponse(): contentLength(0), isNotAllow(false), connectionClose(false) {
    this->setStatus(501);
    this->setHttpVersion("HTTP/1.1");
};

HttpResponse::HttpResponse(const HttpResponse& other) {
    *this = other;
};

HttpResponse& HttpResponse::operator=(const HttpResponse& other) {

    if (this != &other) {

        this->status = other.status;
        this->httpVersion = other.httpVersion;
        this->contentType = other.contentType;
        this->contentLength = other.contentLength;
        this->body = other.body;
        this->allowedMethods = other.allowedMethods;
        this->isNotAllow = other.isNotAllow;
        this->connectionClose = other.connectionClose;
        this->_headers = other._headers;
    }
    return *this;
};
HttpResponse::~HttpResponse() {};

void HttpResponse::setStatus(const int& statusCode) {

    this->status.setCodeStatus(statusCode);
};

void HttpResponse::setHttpVersion(const std::string& httpVersion) {
    this->httpVersion = httpVersion;
    if (httpVersion == "HTTP/1.0")
        this->setConnectionClose(true);
};

void HttpResponse::setContentType(const std::string& contentType) {
    this->contentType = contentType;
};

void HttpResponse::setContentLength(const int& contentLength){
    this->contentLength = contentLength;
};

void HttpResponse::setBody(const std::string& body) {
    this->body = body;
    setContentLength(body.size());
};

void HttpResponse::setConnectionClose(bool connectionClose) {
    this->connectionClose = connectionClose;
}

bool HttpResponse::isConnectionClose() {
    return this->connectionClose;
};

int HttpResponse::getContentLength() {
    return this->contentLength;
}

std::string joinMethods(const std::vector<std::string> &methods) {
    std::string result;
    for (size_t i = 0; i < methods.size(); ++i) {
        result += methods[i];
        if (i + 1 < methods.size())
            result += ", ";
    }
    return result;
}

void HttpResponse::setAllowedMethods(const std::vector<std::string>& methods) {

    this->allowedMethods += "Allow: ";
    this->allowedMethods += (joinMethods(methods) + "\r\n");
};

// Melhor passar o location depois, pois há paginas de erros setadas no arquivo de configuração
HttpResponse HttpResponse::methodNotAllowed(const std::vector<std::string>& methods) {

    HttpResponse response;

    response.isNotAllow = true;
    response.setHttpVersion("HTTP/1.1");
    response.setContentLength(0);
    response.setStatus(405);
    response.setAllowedMethods(methods);

    return (response);    
};

void HttpResponse::setHeader(const std::string &key, const std::string &value){
    _headers[key] = value;
}

const std::string HttpResponse::getStatusResponse() {
    return status.makeResponseStatus();
};

int HttpResponse::getStatusCode() {
    return status.getCodeStatus();
}

std::string HttpResponse::toString() {

    std::string response = this->httpVersion;
    response += (" " + status.makeResponseStatus() + "\r\n");
    if (this->contentType.size() > 0)
        response += ("Content-Type: " + this->contentType + "\r\n");
    response += "Content-Length: " + Utils::toString(this->contentLength) + "\r\n";
    for (std::map<std::string, std::string>::const_iterator it = _headers.begin();
         it != _headers.end(); ++it){
        response += it->first + ": " + it->second + "\r\n";
    }
    if (this->isNotAllow)
        response += allowedMethods;
    if (this->connectionClose)
        response += "Connection: close\r\n";
    response += "\r\n";
    if (this->contentLength > 0)
        response += this->body;
    return response;
}
