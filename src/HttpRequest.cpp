#include "../include/HttpRequest.hpp"

#include <sstream>
#include <vector>
#include "../include/Utils.hpp"
#include <exception>

HttpRequest::HttpRequest() {

};

HttpRequest::HttpRequest(const char *buffer): _buffer(buffer) {
    std::string req(buffer);
    
    // encontrar o fim dos headers
    size_t headerEnd = req.find("\r\n\r\n");
    std::string headersPart = req.substr(0, headerEnd);
    _body = (headerEnd != std::string::npos) ? req.substr(headerEnd + 4) : "";

    std::vector<std::string> lines;
    std::stringstream ss(headersPart);
    std::string aux;

    while (getline(ss, aux, '\n')) {
        lines.push_back(Utils::trim(aux));
    }

    // primeira linha → método, caminho e versão
    std::stringstream firstLine(lines[0]);
    firstLine >> _method >> _path >> _httpVersion;

    // headers
    for (size_t i = 1; i < lines.size(); i++) {
        size_t pos = lines[i].find(':');
        if (pos == std::string::npos) continue;

        std::string key = Utils::trim(lines[i].substr(0, pos));
        std::string value = Utils::trim(lines[i].substr(pos + 1));
        _headers[key] = value;
    }
}

void HttpRequest::parser() {
    
    // encontrar o fim dos headers
    size_t headerEnd = this->_buffer.find("\r\n\r\n");
    std::string headersPart = this->_buffer.substr(0, headerEnd);

    std::vector<std::string> lines;
    std::stringstream ss(headersPart);
    std::string aux;

    while (getline(ss, aux, '\n')) {
        lines.push_back(Utils::trim(aux));
    }

    // primeira linha → método, caminho e versão
    std::stringstream firstLine(lines[0]);
    firstLine >> _method >> _path >> _httpVersion;

    // separar path e query string
    size_t qpos = _path.find('?');
    if (qpos != std::string::npos) {
        _queryString = _path.substr(qpos + 1);
        _path = _path.substr(0, qpos);
    } else {
        _queryString = "";
    }

    // headers
    for (size_t i = 1; i < lines.size(); i++) {
        size_t pos = lines[i].find(':');
        if (pos == std::string::npos) continue;

        std::string key = Utils::trim(lines[i].substr(0, pos));
        std::string value = Utils::trim(lines[i].substr(pos + 1));
        _headers[key] = value;
    }

    if (this->getContentLength() > 0)
        _body = (headerEnd != std::string::npos) ? this->_buffer.substr(headerEnd + 4) : "";
};

HttpRequest::~HttpRequest() {};

const std::string& HttpRequest::getHttpVersion() const {
    return this->_httpVersion;
};

const std::string& HttpRequest::getMethod() const {
    return this->_method;
};

const std::string& HttpRequest::getPath() const {
    return this->_path;
};

const std::string& HttpRequest::getHeader(const std::string &key) const {
    const std::map<std::string, std::string>::const_iterator it = this->_headers.find(key);

    if (it != this->_headers.end()) {
        return it->second;
    }

    static const std::string empty = "";
    return empty;
};

const std::string& HttpRequest::getBuffer() const {
    return this->_buffer;
};

const std::string& HttpRequest::getBody() const {
    return this->_body;
};

const std::string& HttpRequest::getQueryString() const {
    return this->_queryString;
};

int HttpRequest::getContentLength() const {
    std::map<std::string, std::string>::const_iterator it = _headers.find("Content-Length");
    if (it == _headers.end())
        return 0;
    std::string value = it->second;
    return std::atoi(value.c_str());
}


void HttpRequest::appendBuffer(const std::string& buffer) {
    this->_buffer.append(buffer);
};

void HttpRequest::appendBody(const std::string& body) {

    this->_body.append(body);
};

void HttpRequest::clearAllData() {
    
    this->_buffer.erase();
    this->_method.erase();
    this->_path.erase();
    this->_httpVersion.erase();
    this->_headers.erase(this->_headers.begin(), this->_headers.end());
    this->_body.erase();
    this->_queryString.erase();
};